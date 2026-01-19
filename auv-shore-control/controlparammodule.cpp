#include "controlparammodule.h"
#include "controlparamresult.h"
#include <QPointer>
#include <qDebug>
#include <QJsonDocument>
ControlParamModule::ControlParamModule(QObject* parent)
	: BaseModule(ModuleType::ControlParamDebugging, "ControlParamDebugging", parent)
	, m_nextPacketSeq(1) {
	initialize();
}

ControlParamModule::~ControlParamModule()
{}

bool ControlParamModule::initialize()
{
    // 1. 订阅通信模块事件
    subscribeEvent("controlparam_send_result",
        [this](const DataMessage& msg) {
            QPointer<ControlParamModule> weakThis = this;
            if (weakThis) {
                weakThis->onSendResultReceived(msg);
            }
        });

    subscribeEvent("controlparam_receiveresult",
        [this](const DataMessage& msg) {
            QPointer<ControlParamModule> weakThis = this;
            if (weakThis) {
                weakThis->onControlResultReceived(msg);
            }
        });

    // 2. 初始化定期清理定时器（30秒）
    m_cleanupTimer = new QTimer(this);
    m_cleanupTimer->setInterval(30000);
    m_cleanupTimer->setSingleShot(false);
    connect(m_cleanupTimer, &QTimer::timeout, this, &ControlParamModule::cleanupStaleStates);
    m_cleanupTimer->start();

    qDebug() << "控制参数模块初始化完成（无命令码）";
    return true;
}

void ControlParamModule::shutdown()
{
    qDebug() << "清理控制参数模块";

    // 停止清理定时器（对齐SelfCheckModule）
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

    for (auto& state : m_internalCmdStates) {
        if (state.timer) {
            state.timer->stop();
            state.timer->disconnect();
            state.timer->deleteLater();
            state.timer = nullptr;
        }
    }

    m_commandStates.clear();
    m_internalCmdStates.clear();
    m_cmdTimerMap.clear();
    m_observers.clear();
}

CheckError ControlParamModule::sendControlParamCommand(ControlType controlType,
    const std::shared_ptr<ControlParamParameter>& param,
    WorkPhase phase,
    CommunicationChannel channel) {

    qDebug() << "发送控制参数命令（外部指令）- 类型：" << static_cast<int>(controlType)
        << " 信道：" << static_cast<int>(channel);

    // 1. 基础校验：控制类型有效性
    if (controlType < ControlType::DepthControl || controlType > ControlType::HeadingControl) {
        return CheckError(ErrorCode::DeviceNotFound, "控制类型非法：" + QString::number(static_cast<int>(controlType)));
    }
    if (!param) {
        return CheckError(ErrorCode::ParameterInvalid, "控制参数为空");
    }

    // 2. 生成包序号和全局请求ID
    uint8_t packetSeq = getNextPacketSeq();
    uint64_t requestId = CommandIdGenerator::instance().nextId();

    // 3. 构建数据体：仅 控制类型(1字节) + 控制参数（无内部指令分支）
    QByteArray dataBody;
    dataBody.append(static_cast<char>(static_cast<uint8_t>(controlType)));  // 控制类型
    dataBody.append(param->toBytes());                                      // 参数字节流
    qDebug() << "控制参数数据体（十六进制）：" << dataBody.toHex().toUpper();

    // 4. 超时管理（完全对齐自检模块外部指令逻辑）
    QTimer* timer = new QTimer(this);
    timer->setSingleShot(true);
    int timeoutMs = getDynamicTimeoutMs(controlType);
    timer->setInterval(timeoutMs);

    connect(timer, &QTimer::timeout, this, [this, packetSeq, controlType, timer]() {
        // 锁外停止定时器，避免死锁
        timer->stop();
        timer->disconnect();

        // 加锁处理超时逻辑
        QMutexLocker locker(&m_mutex);
        if (m_commandStates.contains(packetSeq)) {
            CommandState& state = m_commandStates[packetSeq];
            if (state.controlType == controlType && !state.isProcessed) {
                qDebug() << "[超时触发] 控制参数命令 - 类型：" << static_cast<int>(controlType)
                    << " 包序号：" << static_cast<int>(packetSeq);
                this->onCommandTimeout(controlType, packetSeq);
            }

            // 清理状态
            m_cmdTimerMap.remove(packetSeq);
            m_commandStates.remove(packetSeq);
        }

        // 异步删除定时器
        if (timer) timer->deleteLater();
        });
    timer->start();

    // 5. 存储命令状态（仅外部指令，按packetSeq存储）
    {
        QMutexLocker locker(&m_mutex);
        m_commandStates[packetSeq] = {
            controlType,
            timer,
            false,
            packetSeq,
            QDateTime::currentDateTime()
        };
        m_cmdTimerMap[packetSeq] = QPointer<QTimer>(timer);
    }

    // 6. 发送到通信模块（固定为外部指令）
    QJsonObject cmdParams;
    cmdParams["source_module"] = static_cast<int>(ModuleType::ControlParamDebugging);
    cmdParams["request_id"] = QString::number(requestId);
    cmdParams["packet_seq"] = packetSeq;
    cmdParams["work_phase"] = static_cast<int>(phase);
    cmdParams["data_body"] = QString(dataBody.toBase64());
    cmdParams["timestamp"] = QDateTime::currentDateTime().toString("yyMMddhhmmss");
    cmdParams["control_type"] = static_cast<int>(controlType);
    cmdParams["cmd_type"] = static_cast<int>(CommandType::External);  // 固定为外部指令

    sendCommand(
        "CommunicationModule",
        channel,
        "send_control_command",
        cmdParams
    );

    return CheckError();
}
void ControlParamModule::onControlResultReceived(const DataMessage& data) {
    // 解析通信模块返回的JSON结果
    QJsonObject outerJson = QJsonDocument::fromJson(data.data()).object();
    QJsonObject dataJson = outerJson["data"].toObject();

    // 仅处理外部指令的无线电/水声信道结果
    if (data.channel() == CommunicationChannel::Radio || data.channel() == CommunicationChannel::WaterAcoustic) {
        parseControlParamResult(dataJson);
    }
}


uint8_t ControlParamModule::getUniquePacketSeq()
{
    return getNextPacketSeq();
}

void ControlParamModule::onSendResultReceived(const DataMessage& data)
{
    QJsonObject receiveJson = QJsonDocument::fromJson(data.data()).object();
    QJsonObject dataJson = receiveJson["data"].toObject();

    // 提取核心字段（无CommandType）
    int controlTypeInt = dataJson["control_type"].toInt(-1);
    int packetSeqInt = dataJson["packet_seq"].toInt(-1);
    bool success = dataJson["success"].toBool(false);
    QString message = dataJson["message"].toString("未知原因");
    uint64_t requestId = dataJson["request_id"].toVariant().toULongLong();

    // 字段有效性校验
    QMutexLocker locker(&m_mutex);
    bool isValid = true;
    ControlType controlType = static_cast<ControlType>(controlTypeInt);
    uint8_t packetSeq = static_cast<uint8_t>(packetSeqInt);

    if (controlTypeInt < 0 || controlTypeInt > static_cast<int>(ControlType::HeadingControl)) {
        qWarning() << "[发送结果无效] 控制类型非法：" << controlTypeInt;
        isValid = false;
    }
    if (packetSeqInt < 0 || packetSeqInt > 255) {
        qWarning() << "[发送结果无效] 包序号超出范围：" << packetSeqInt;
        isValid = false;
    }

    // 构建发送结果对象并通知观察者
    ControlParamSendResult sendResult(controlType, success, packetSeq);
    sendResult.setMessage(message);
    sendResult.setRequestId(requestId);

    if (!isValid) {
        sendResult.setSuccess(false);
        sendResult.setMessage("发送结果字段非法");
        notifyObservers(&sendResult);
        return;
    }

    // 日志打印
    QString typeStr;
    switch (controlType) {
    case ControlType::DepthControl: typeStr = "深度控制"; break;
    case ControlType::HeightControl: typeStr = "高度控制"; break;
    case ControlType::HeadingControl: typeStr = "航向控制"; break;
    }

    if (success) {
        qDebug() << "[控制参数命令发送成功] " << typeStr << " 包序号：" << static_cast<int>(packetSeq);
        notifyObservers(&sendResult);

        // 通知Pending状态
        ControlParamSendResult pendingResult(controlType, true, packetSeq);
        pendingResult.setFeedbackDesc("命令已发送，等待响应...");
        notifyObservers(&pendingResult);
    }
    else {
        qWarning() << "[控制参数命令发送失败] " << typeStr << " 原因：" << message;
        notifyObservers(&sendResult);

        // 发送失败，立即清理状态
        cleanupCommandState(packetSeq);
    }
}

void ControlParamModule::onCommandTimeout(ControlType controlType, uint8_t packetSeq)
{ // 构建超时结果并通知观察者
    ControlParamResult result(controlType, ResultStatus::Timeout);
    result.setFeedbackDesc("命令超时未响应（超时时间：" + QString::number(getDynamicTimeoutMs(controlType)) + "ms）");
    result.setPacketSeq(packetSeq);
    notifyObservers(&result);

    qDebug() << "[超时清理] 控制参数命令 - 类型：" << static_cast<int>(controlType)
        << " 包序号：" << static_cast<int>(packetSeq);
}

void ControlParamModule::cleanupStaleStates()
{
    QList<uint8_t> staleSeqs;
    {
        QMutexLocker locker(&m_mutex);
        QDateTime now = QDateTime::currentDateTime();

        // 仅清理外部指令的过期状态（超过2倍超时时间）
        for (auto it = m_commandStates.begin(); it != m_commandStates.end(); ++it) {
            CommandState& state = it.value();
            bool isStale = state.isProcessed
                || now > state.sendTime.addMSecs(getDynamicTimeoutMs(state.controlType) * 2)
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
        qDebug() << "清理控制参数过期状态：" << staleSeqs.size() << "个";
    }
}

uint8_t ControlParamModule::getNextPacketSeq()
{
    // 原子操作生成包序号（1-255循环）
    uint8_t seq = m_nextPacketSeq.fetchAndAddRelaxed(1);
    if (seq > 255) {
        m_nextPacketSeq.storeRelaxed(1);
        return 1;
    }
    return seq;
}

int ControlParamModule::getDynamicTimeoutMs(ControlType controlType)
{
    // 按控制类型设置不同超时时间
    switch (controlType) {
    case ControlType::DepthControl: return 5000;
    case ControlType::HeightControl: return 5000;
    case ControlType::HeadingControl: return 3000;
    default: return 5000;
    }
}

void ControlParamModule::parseControlParamResult(const QJsonObject& resultJson)
{  // 解析控制参数执行结果
    int controlTypeInt = resultJson["control_type"].toInt();
    bool success = resultJson["success"].toBool();
    QString feedback = resultJson["feedback"].toString();
    uint8_t packetSeq = static_cast<uint8_t>(resultJson["packet_seq"].toInt());

    ControlParamResult result(
        static_cast<ControlType>(controlTypeInt),
        success ? ResultStatus::Success : ResultStatus::Failed
    );
    result.setFeedbackDesc(feedback);
    result.setPacketSeq(packetSeq);

    // 清理命令状态
    cleanupCommandState(packetSeq);
    // 通知观察者
    notifyObservers(&result);
}

void ControlParamModule::cleanupCommandState(uint8_t packetSeq)
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
