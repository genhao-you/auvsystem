#include "sailcontrolmodule.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QPointer>

SailControlModule::SailControlModule(QObject* parent)
    : BaseModule(ModuleType::SailControl, "SailControl", parent)
    , m_nextPacketSeq(1)
{
    initialize();
}

SailControlModule::~SailControlModule()
{
    shutdown();
}

bool SailControlModule::initialize()
{
    qDebug() << "初始化航行控制模块...";

    // 1. 订阅通信模块事件
    subscribeEvent("sailcontrol_send_result",
        [this](const DataMessage& msg) {
            QPointer<SailControlModule> weakThis = this;
            if (weakThis) {
                weakThis->onSendResultReceived(msg);
            }
        });

    subscribeEvent("sailcontrol_receiveresult",
        [this](const DataMessage& msg) {
            QPointer<SailControlModule> weakThis = this;
            if (weakThis) {
                weakThis->onSailResultReceived(msg);
            }
        });

    // 2. 初始化定期清理定时器（30秒）
    m_cleanupTimer = new QTimer(this);
    m_cleanupTimer->setInterval(30000);
    m_cleanupTimer->setSingleShot(false);
    connect(m_cleanupTimer, &QTimer::timeout, this, &SailControlModule::cleanupStaleStates);
    m_cleanupTimer->start();

    qDebug() << "航行控制模块初始化完成";
    return true;
}

void SailControlModule::shutdown()
{
    qDebug() << "清理航行控制模块...";

    // 停止清理定时器
    if (m_cleanupTimer) {
        m_cleanupTimer->stop();
        m_cleanupTimer->deleteLater();
        m_cleanupTimer = nullptr;
    }

    // 清理命令状态和定时器（加锁保护）
    QMutexLocker locker(&m_mutex);
    for (auto& state : m_commandStates) {
        if (state.timer) {
            state.timer->stop();
            state.timer->disconnect();
            state.timer->deleteLater();
            state.timer = nullptr;
        }
    }

    m_commandStates.clear();
    m_cmdTimerMap.clear();

    qDebug() << "航行控制模块清理完成";
}

CheckError SailControlModule::sendSailCommand(SailCommandCode cmdCode, CommunicationChannel channel)
{
    qDebug() << "发送航行控制命令：" << static_cast<int>(cmdCode)
        << " (" << SailControlParam(cmdCode).toString() << ")"
        << " 信道：" << static_cast<int>(channel);

    // 1. 基础校验
    if (cmdCode != SailCommandCode::Sail_Start && cmdCode != SailCommandCode::Sail_Stop) {
        return CheckError(ErrorCode::CommandNotSupported,
            "不支持的航行控制命令：" + QString::number(static_cast<int>(cmdCode)));
    }

    // 2. 生成包序号和全局请求ID
    uint8_t packetSeq = getNextPacketSeq();
    uint64_t requestId = CommandIdGenerator::instance().nextId();

    // 3. 构建数据体：只有命令码（1字节）
    SailControlParam param(cmdCode);
    QByteArray dataBody = param.toBytes();

    qDebug() << "航行控制数据体（十六进制）：" << dataBody.toHex().toUpper();

    // 4. 超时管理
    QTimer* timer = new QTimer(this);
    timer->setSingleShot(true);
    int timeoutMs = getDynamicTimeoutMs(cmdCode);
    timer->setInterval(timeoutMs);

    connect(timer, &QTimer::timeout, this, [this, packetSeq, cmdCode, timer]() {
        // 锁外停止定时器，避免死锁
        timer->stop();
        timer->disconnect();

        // 加锁处理超时逻辑
        QMutexLocker locker(&m_mutex);
        if (m_commandStates.contains(packetSeq)) {
            CommandState& state = m_commandStates[packetSeq];
            if (state.cmdCode == cmdCode && !state.isProcessed) {
                qDebug() << "[超时触发] 航行控制命令 - 类型：" << static_cast<int>(cmdCode)
                    << " 包序号：" << static_cast<int>(packetSeq);
                this->onCommandTimeout(cmdCode, packetSeq);
            }

            // 清理状态
            m_cmdTimerMap.remove(packetSeq);
            m_commandStates.remove(packetSeq);
        }

        // 异步删除定时器
        if (timer) timer->deleteLater();
        });
    timer->start();

    // 5. 存储命令状态
    {
        QMutexLocker locker(&m_mutex);
        m_commandStates[packetSeq] = {
            cmdCode,
            timer,
            false,
            packetSeq,
            QDateTime::currentDateTime(),
            1  // refCount
        };
        m_cmdTimerMap[packetSeq] = QPointer<QTimer>(timer);
    }

    // 6. 发送到通信模块
    QJsonObject cmdParams;
    cmdParams["source_module"] = static_cast<int>(ModuleType::SailControl);
    cmdParams["request_id"] = QString::number(requestId);
    cmdParams["packet_seq"] = static_cast<int>(packetSeq);
    cmdParams["work_phase"] = static_cast<int>(WorkPhase::ControlCommand);
    cmdParams["data_body"] = QString(dataBody.toBase64());
    cmdParams["timestamp"] = QDateTime::currentDateTime().toString("yyMMddhhmmss");
    cmdParams["sail_cmd_code"] = static_cast<int>(cmdCode);  // 航行控制专用命令码
    cmdParams["is_sail_control"] = true;  // 标记为航行控制命令
    cmdParams["cmd_type"] = static_cast<int>(CommandType::External);  // 外部指令

    sendCommand(
        "CommunicationModule",
        channel,
        "send_control_command",
        cmdParams
    );

    // 7. 更新航行状态
    updateSailStatus(cmdCode == SailCommandCode::Sail_Start);

    return CheckError();
}

void SailControlModule::onSailResultReceived(const DataMessage& data)
{
    // 解析通信模块返回的JSON结果
    QJsonObject outerJson = QJsonDocument::fromJson(data.data()).object();
    QJsonObject dataJson = outerJson["data"].toObject();

    parseSailResult(dataJson);
}

void SailControlModule::onSendResultReceived(const DataMessage& data)
{
    QJsonObject receiveJson = QJsonDocument::fromJson(data.data()).object();
    QJsonObject dataJson = receiveJson["data"].toObject();

    // 提取核心字段
    int cmdCodeInt = dataJson["sail_cmd_code"].toInt(-1);
    int packetSeqInt = dataJson["packet_seq"].toInt(-1);
    bool success = dataJson["success"].toBool(false);
    QString message = dataJson["message"].toString("未知原因");
    uint64_t requestId = dataJson["request_id"].toVariant().toULongLong();

    // 字段有效性校验
    QMutexLocker locker(&m_mutex);
    bool isValid = true;
    SailCommandCode cmdCode = static_cast<SailCommandCode>(cmdCodeInt);
    uint8_t packetSeq = static_cast<uint8_t>(packetSeqInt);

    if (cmdCode != SailCommandCode::Sail_Start && cmdCode != SailCommandCode::Sail_Stop) {
        qWarning() << "[发送结果无效] 航行控制命令码非法：" << cmdCodeInt;
        isValid = false;
    }
    if (packetSeqInt < 0 || packetSeqInt > 255) {
        qWarning() << "[发送结果无效] 包序号超出范围：" << packetSeqInt;
        isValid = false;
    }

    QString cmdName = SailControlParam(cmdCode).toString();

    if (success) {
        qDebug() << "[航行控制命令发送成功] " << cmdName
            << " 包序号：" << static_cast<int>(packetSeq);
    }
    else {
        qWarning() << "[航行控制命令发送失败] " << cmdName
            << " 原因：" << message;

        // 发送失败，立即清理状态
        cleanupCommandState(packetSeq);

        // 如果发送失败，可能需要重置航行状态
        if (cmdCode == SailCommandCode::Sail_Start) {
            updateSailStatus(false);
        }
    }

    // 发射信号通知外部
    //emit commandResult(cmdCode, success, message);
}

void SailControlModule::onCommandTimeout(SailCommandCode cmdCode, uint8_t packetSeq)
{
    QString cmdName = SailControlParam(cmdCode).toString();
    QString timeoutMsg = QString("%1命令响应超时（超时时间：%2ms）")
        .arg(cmdName)
        .arg(getDynamicTimeoutMs(cmdCode));

    qWarning() << "[超时] " << timeoutMsg;

    // 发射信号通知外部
    //emit commandResult(cmdCode, false, timeoutMsg);

    // 超时时清理状态
    cleanupCommandState(packetSeq);

    // 如果开始命令超时，重置航行状态
    if (cmdCode == SailCommandCode::Sail_Start) {
        updateSailStatus(false);
    }
}

void SailControlModule::cleanupStaleStates()
{
    QList<uint8_t> staleSeqs;
    {
        QMutexLocker locker(&m_mutex);
        QDateTime now = QDateTime::currentDateTime();

        // 清理过期状态（超过2倍超时时间）
        for (auto it = m_commandStates.begin(); it != m_commandStates.end(); ++it) {
            CommandState& state = it.value();
            bool isStale = state.isProcessed
                || now > state.sendTime.addMSecs(getDynamicTimeoutMs(state.cmdCode) * 2)
                || (state.timer && !state.timer->isActive());

            if (isStale) staleSeqs.append(it.key());
        }

        // 批量移除过期状态
        for (uint8_t seq : staleSeqs) {
            m_commandStates.remove(seq);
            m_cmdTimerMap.remove(seq);
        }
    }

    // 锁外删除定时器
    if (!staleSeqs.isEmpty()) {
        for (uint8_t seq : staleSeqs) {
            QPointer<QTimer> timer = m_cmdTimerMap.value(seq);
            if (!timer.isNull()) {
                timer->stop();
                timer->deleteLater();
            }
        }
        qDebug() << "清理航行控制过期状态：" << staleSeqs.size() << "个";
    }
}

uint8_t SailControlModule::getNextPacketSeq()
{
    // 原子操作生成包序号（1-255循环）
    uint8_t seq = m_nextPacketSeq.fetchAndAddRelaxed(1);
    if (seq > 255) {
        m_nextPacketSeq.storeRelaxed(1);
        return 1;
    }
    return seq;
}

int SailControlModule::getDynamicTimeoutMs(SailCommandCode cmdCode)
{
    // 航行控制命令统一超时时间
    return SAIL_CMD_TIMEOUT_MS;
}

void SailControlModule::parseSailResult(const QJsonObject& resultJson)
{
    // 解析航行控制执行结果
    int cmdCodeInt = resultJson["sail_cmd_code"].toInt();
    bool success = resultJson["success"].toBool();
    QString feedback = resultJson["feedback"].toString();
    uint8_t packetSeq = static_cast<uint8_t>(resultJson["packet_seq"].toInt());

    SailCommandCode cmdCode = static_cast<SailCommandCode>(cmdCodeInt);
    QString cmdName = SailControlParam(cmdCode).toString();

    if (success) {
        qDebug() << "[航行控制命令执行成功] " << cmdName
            << " 反馈：" << feedback;

        // 根据命令结果更新航行状态
        if (cmdCode == SailCommandCode::Sail_Stop) {
            updateSailStatus(false);
        }
        else if (cmdCode == SailCommandCode::Sail_Start) {
            updateSailStatus(true);
        }
    }
    else {
        qWarning() << "[航行控制命令执行失败] " << cmdName
            << " 反馈：" << feedback;

        // 如果开始命令失败，重置航行状态
        if (cmdCode == SailCommandCode::Sail_Start) {
            updateSailStatus(false);
        }
    }

    // 清理命令状态
    cleanupCommandState(packetSeq);

    // 发射信号通知外部
    //emit commandResult(cmdCode, success, feedback);
}

void SailControlModule::cleanupCommandState(uint8_t packetSeq)
{
    QMutexLocker locker(&m_mutex);
    if (m_commandStates.contains(packetSeq)) {
        CommandState& state = m_commandStates[packetSeq];
        if (state.timer) {
            state.timer->stop();
            state.timer->deleteLater();
        }
        m_commandStates.remove(packetSeq);
        m_cmdTimerMap.remove(packetSeq);
    }
}

void SailControlModule::updateSailStatus(bool isSailing)
{
    if (m_isSailing != isSailing) {
        m_isSailing = isSailing;
        qDebug() << "航行状态更新：" << (isSailing ? "航行中" : "已停止");
        //emit sailStatusChanged(isSailing);
    }
}