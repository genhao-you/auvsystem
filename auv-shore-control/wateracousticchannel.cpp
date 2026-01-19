#include "wateracousticchannel.h"
#include"framebuilder.h"
#include "frameparser.h"
#include <QJsonDocument>
#include <QDebug>
#include <QSerialPortInfo>
#include<QThread>
// 辅助函数：指令类型转字符串（静态函数实现）
QString WaterAcousticChannel::waUplinkCmdToString(WaterAcousticUplinkCmd cmd) {
    switch (cmd) {
    case WaterAcousticUplinkCmd::Unknown:      return "Unknown";
    case WaterAcousticUplinkCmd::SendFinish:  return "SendFinish";
    case WaterAcousticUplinkCmd::RecvNotify:  return "RecvNotify";
    case WaterAcousticUplinkCmd::DataReceived:return "DataReceived";
    default: return "Unsupported";
    }
}

WaterAcousticChannel::WaterAcousticChannel(QObject* parent)
    : CommunicationChannelBase(CommunicationChannel::WaterAcoustic, parent)
    , m_waTimer(new QTimer(this)) // 初始化超时定时器
    , m_portCheckTimer(new QTimer(this)) // 初始化兜底定时器
    , m_recvNotifyTimer(new QTimer(this)) // 新数据通知定时器
    , m_hasRecvNotify(false)

{
    // 初始化串口
    m_serialPort = new QSerialPort(this);
    // 初始化兜底检测定时器（2秒检测一次，可调整）
    m_portCheckTimer->setInterval(2000);
    connect(m_portCheckTimer, &QTimer::timeout, this, &WaterAcousticChannel::checkPortAvailability);
    // 绑定基类信号到业务槽（QueuedConnection保证线程安全）
    connect(this, &CommunicationChannelBase::sendDataRequested,
        this, [this](const QVariant& var) {
            // 仅处理QByteArray类型（RadioChannel只需要原始字节）
            if (var.canConvert<QByteArray>()) {
                this->sendDataImpl(var.value<QByteArray>());
            }
            else {
                m_lastError = "WaterAcousticChannel通道接收非法发送参数";
                emit errorOccurred(m_lastError);
                emit sendDataFinished(false);
            }
        }, Qt::DirectConnection);
    connect(this, &CommunicationChannelBase::setConfigRequested,
        this, &WaterAcousticChannel::setConfigImpl, Qt::QueuedConnection);
    connect(this, &CommunicationChannelBase::startRequested,
        this, &WaterAcousticChannel::startImpl, Qt::QueuedConnection);
    connect(this, &CommunicationChannelBase::stopRequested,
        this, &WaterAcousticChannel::stopImpl, Qt::QueuedConnection);

    // 绑定串口信号
    connect(m_serialPort, &QSerialPort::readyRead, this, &WaterAcousticChannel::onDataReady);
    connect(m_serialPort, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
        this, &WaterAcousticChannel::onError);

    m_waTimer->setSingleShot(true); // 单次触发
    connect(m_waTimer, &QTimer::timeout, this, &WaterAcousticChannel::onWaDataTimeout);

    // 初始化“新数据通知”超时定时器
    m_recvNotifyTimer->setSingleShot(true);
    connect(m_recvNotifyTimer, &QTimer::timeout, this, &WaterAcousticChannel::onRecvNotifyTimeout);
}

WaterAcousticChannel::~WaterAcousticChannel() {
    stop();
    clearWaDataCache(); // 析构时清理缓存
    if (m_portCheckTimer) {
        m_portCheckTimer->stop();
        delete m_portCheckTimer;
        m_portCheckTimer = nullptr;
    }
}
bool WaterAcousticChannel::setConfig(const ChannelConfigPtr& config) {
    emit setConfigRequested(config); // 触发配置更新（异步处理）
    return true;
}

ChannelConfigPtr WaterAcousticChannel::config() const {
    return m_config;
}

bool WaterAcousticChannel::start() {
    emit startRequested(); // 触发启动（异步处理）
    return true;
}

void WaterAcousticChannel::stop() {
    emit stopRequested(); // 触发停止（异步处理）
}

bool WaterAcousticChannel::isRunning() const {
    return m_running;
}

QString WaterAcousticChannel::lastError() const {
    return m_lastError;
}

QString WaterAcousticChannel::statusInfo() const {
    if (m_running && m_config) {
        return QString("水声通信已连接：串口%1（%2波特率）").arg(m_config->m_portName).arg(m_config->m_baudRate);
    }
    return "水声通信已停止";
}

void WaterAcousticChannel::onDataReady()
{
    // 1. 读取原始数据（循环读取，确保读完所有缓存）
    QByteArray rawData;
    while (m_serialPort->bytesAvailable() > 0) {
        rawData += m_serialPort->readAll();
    }
    if (rawData.isEmpty()) return;

    // 2. 追加到缓冲区（关键：不截断，完整保留）
    m_recvBuffer += rawData;
    qDebug() << "水声通信接收原始数据：" << rawData.toHex(' ');
   

    // 3. 立即解析（无需等待，确保连帧实时解析）
    parseFrames();
}
void WaterAcousticChannel::onError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError) {
        QString errorMsg = "串口错误: " + m_serialPort->errorString();
        qWarning() << "[水声通道] 串口异常：" << errorMsg;

        // 重点：捕获硬件意外移除错误（ResourceError）
        if (error == QSerialPort::ResourceError) {
            errorMsg = QString("水声串口硬件已意外移除（端口：%1）").arg(m_serialPort->portName());
            m_lastError = errorMsg;

            // 1. 强制标记通道为未运行状态（核心：同步UI判断依据）
            if (m_running) {
                m_running = false;
            }

            // 2. 停止专属定时器（避免定时器残留触发无效逻辑）
            m_waTimer->stop();
            m_recvNotifyTimer->stop();
            m_hasRecvNotify = false;

            // 3. 主动调用停止实现，清理串口+缓存（复用现有逻辑）
            stopImpl(); // 直接同步调用，无需信号异步触发

            // 4. 额外兜底：确保串口关闭
            if (m_serialPort->isOpen()) {
                m_serialPort->close();
            }
        }
        else {
            // 其他普通错误，保留原有逻辑
            m_lastError = errorMsg;
        }

        // 发送错误信号和状态变更信号，让UI感知
        emit errorOccurred(m_lastError);
        emit statusChanged(m_running ? "运行中" : "已断开（硬件移除）");
    }
}

void WaterAcousticChannel::sendDataImpl(const QByteArray& data)
{
    // 1. 前置检查
    if (!m_running || !m_serialPort->isOpen()) {
        m_lastError = "水声通道未连接，无法发送数据";
        emit errorOccurred(m_lastError);
        emit sendDataFinished(false);
        return;
    }
    if (data.isEmpty()) {
        m_lastError = "水声发送数据为空";
        emit errorOccurred(m_lastError);
        emit sendDataFinished(false);
        return;
    }

    // 2. 检查发送间隔（≥3s）
    qint64 elapsedMs = m_lastSendTime.msecsTo(QDateTime::currentDateTime());
    if (m_lastSendTime.isValid() && elapsedMs < FrameConstants::WATER_ACOUSTIC_SEND_INTERVAL_MS) {
        m_lastError = QString("发送间隔不足3s（需等待%1ms）").arg(FrameConstants::WATER_ACOUSTIC_SEND_INTERVAL_MS - elapsedMs);
        emit errorOccurred(m_lastError);
        emit sendDataFinished(false);
        return;
    }
    // 3. 加锁前打印日志（确认执行到这一步）
    qInfo() << "[水声] 准备加锁m_waMutex，当前线程ID：" << (quintptr)QThread::currentThreadId();
    // 加锁行（修改为递归锁后，若死锁则会正常加锁，不会退出）
    QMutexLocker locker(&m_waMutex);
    qInfo() << "[水声] 成功加锁m_waMutex，开始执行后续逻辑"; // 若打印这行，说明锁没问题
    if (m_waTimer->isActive()) {
        m_waTimer->stop();
        qInfo() << "[水声] 发送前停止旧的超时定时器";
    }
    // 4. 提取水声帧的coreData（帧头后、帧尾前，包含校验位）
    int waHeaderLen = FrameConstants::WATER_ACOUSTIC_HEADER_LENGTH;
    int waFooterLen = FrameConstants::WATER_ACOUSTIC_FOOTER_LENGTH;
    QByteArray coreData = data.mid(waHeaderLen, data.size() - waHeaderLen - waFooterLen);
    // 校验coreData最小长度（协议要求：数据长度2+时间戳6+包序号1+信道1+发送设备2+接收设备2+阶段1+数据体1+校验位1=17）
    const int MIN_CORE_DATA_LEN = 17;
    if (coreData.size() < MIN_CORE_DATA_LEN) {
        m_lastError = "发送的水声帧中核心数据长度不足";
        emit errorOccurred(m_lastError);
        emit sendDataFinished(false);
        return;
    }

    const int OFFSET_DATA_LEN = 0;        // 数据长度（2字节，大端）
    const int OFFSET_TIMESTAMP = 2;       // 时间戳（6字节：2-7）
    const int OFFSET_PACKET_SEQ = 8;      // 包序号（1字节）
    const int OFFSET_CHANNEL = 9;         // 通信信道（1字节）
    const int OFFSET_SENDER_DEV = 10;     // 发送设备（2字节，大端）
    const int OFFSET_RECEIVER_DEV = 12;   // 接收设备（2字节，大端）
    const int OFFSET_PHASE = 14;          // 使用阶段（1字节）
    const int OFFSET_DATA_BODY = 15;      // 数据体起始
    const int OFFSET_CHECKSUM = coreData.size() - 1; // 校验位（最后1字节）

    // 提取字段（严格按协议）
    m_sentFrameField.packetSeq = static_cast<uint8_t>(coreData[OFFSET_PACKET_SEQ]);
    m_sentFrameField.channel = static_cast<uint8_t>(coreData[OFFSET_CHANNEL]);
    // 发送设备：大端序
    uint16_t senderDev = (static_cast<uint16_t>(coreData[OFFSET_SENDER_DEV]) << 8) | coreData[OFFSET_SENDER_DEV + 1];
    // 接收设备：大端序
    uint16_t receiverDev = (static_cast<uint16_t>(coreData[OFFSET_RECEIVER_DEV]) << 8) | coreData[OFFSET_RECEIVER_DEV + 1];

    // 关键：发送侧也需要互换（和接收侧逻辑一致）
    m_sentFrameField.senderDevice = receiverDev; // 上位机发送设备 = 下位机接收设备
    m_sentFrameField.receiverDevice = senderDev; // 上位机接收设备 = 下位机发送设备

    m_sentFrameField.phase = static_cast<uint8_t>(coreData[OFFSET_PHASE]);
    // 数据体：从15字节到校验位前
    m_sentFrameField.dataBody = coreData.mid(OFFSET_DATA_BODY, OFFSET_CHECKSUM - OFFSET_DATA_BODY);
    m_sentFrameField.fullWaterFrame = data; // 完整水声帧

    // 打印缓存日志（验证提取的字段是否正确）
    qDebug() << "[水声] 缓存发送通用帧字段：";
    qDebug() << "  - 包序号：0x" << QString::number(m_sentFrameField.packetSeq, 16);
    qDebug() << "  - 通信信道：0x" << QString::number(m_sentFrameField.channel, 16);
    qDebug() << "  - 发送设备（互换后）：0x" << QString::number(m_sentFrameField.senderDevice, 16);
    qDebug() << "  - 接收设备（互换后）：0x" << QString::number(m_sentFrameField.receiverDevice, 16);
    qDebug() << "  - 使用阶段：0x" << QString::number(m_sentFrameField.phase, 16);
    qDebug() << "  - 数据体：" << m_sentFrameField.dataBody.toHex(' ');

    // 5. 发送帧
    qint64 bytesWritten = m_serialPort->write(data);
    qDebug() << "水声发送帧（" << bytesWritten << "字节）：" << data.toHex(' ');

    bool success = (bytesWritten == data.size());
    if (success) {
        m_serialPort->flush(); // 强制刷出缓冲区
        m_lastSendTime = QDateTime::currentDateTime(); // 更新发送时间
        m_waTimer->start(WA_TIMEOUT_MS);
        qInfo() << "[水声] 发送成功，启动超时定时器（" << WA_TIMEOUT_MS << "ms）";
    }
    else {
        m_lastError = QString("水声发送失败（实际发送%1字节）：%2").arg(bytesWritten).arg(m_serialPort->errorString());
        emit errorOccurred(m_lastError);
        // 发送失败：清理缓存+停止定时器
        clearWaDataCache();
        m_waTimer->stop();
    }

    emit sendDataFinished(success);
}

void WaterAcousticChannel::setConfigImpl(const ChannelConfigPtr& config)
{
    auto waConfig = qSharedPointerDynamicCast<WaterAcousticConfig>(config);
    if (!waConfig) {
        m_lastError = "水声配置类型错误（非WaterAcousticConfig）";
        emit errorOccurred(m_lastError);
        emit setConfigFinished(false);
        return;
    }
    m_config = waConfig;
    emit setConfigFinished(true);
}

void WaterAcousticChannel::startImpl()
{
    if (m_running) {
        emit startFinished(true);
        return;
    }
    if (!m_config) {
        m_lastError = "水声通信未配置串口参数";
        emit errorOccurred(m_lastError);
        emit startFinished(false);
        return;
    }

    // 应用串口配置
    m_serialPort->setPortName(m_config->m_portName);
    m_serialPort->setBaudRate(m_config->m_baudRate);
    m_serialPort->setDataBits(m_config->m_dataBits);
    m_serialPort->setParity(m_config->m_parity);
    m_serialPort->setStopBits(m_config->m_stopBits);
    m_serialPort->setFlowControl(m_config->m_flowControl);

    // 打开串口
    if (m_serialPort->open(QIODevice::ReadWrite)) {
        m_running = true;
        m_recvBuffer.clear();
        m_lastSendTime = QDateTime();
        m_portCheckTimer->start();
        emit statusChanged("运行中");
        emit startFinished(true);
    }
    else {
        m_lastError = QString("无法打开水声串口%1：%2").arg(m_config->m_portName).arg(m_serialPort->errorString());
        emit errorOccurred(m_lastError);
        emit startFinished(false);
    }
}

void WaterAcousticChannel::stopImpl()
{
    if (!m_running) {
        emit stopFinished();
        return;
    }
    m_portCheckTimer->stop();

    // 停止所有定时器+清理状态
    clearWaDataCache();
    m_waTimer->stop();
    m_recvNotifyTimer->stop();
    m_hasRecvNotify = false;

    m_running = false;
    m_serialPort->close();
    m_recvBuffer.clear();
    emit statusChanged("已停止");
    emit stopFinished();
}

void WaterAcousticChannel::onWaDataTimeout()
{
    QMutexLocker locker(&m_waMutex);
    QByteArray sentData = m_sentFrameField.fullWaterFrame;
    clearWaDataCache(); // 超时后清理缓存

    qWarning() << "[水声] 数据回复超时 - 发送数据：" << sentData.toHex(' ');
    // 生成超时结果的DataMessage
    DataMessage msg = createWaDataMsg(WaterAcousticResult::Timeout,
        WaterAcousticUplinkCmd::DataReceived,
        sentData, QByteArray());
    msg.setTimestamp(QDateTime::currentDateTime());
    emit dataReceived(msg);
}

void WaterAcousticChannel::onRecvNotifyTimeout()
{
    if (m_hasRecvNotify) {
        DataMessage msg = createWaStatusMsg(
            "通信机收到数据但校验失败，未转发数据",
            WaterAcousticResult::Failed,
            WaterAcousticUplinkCmd::DataReceived
        );
        msg.setTimestamp(QDateTime::currentDateTime());
        emit dataReceived(msg);
        m_hasRecvNotify = false; // 重置状态
    }
}

void WaterAcousticChannel::checkPortAvailability()
{
    // 仅在通道运行时执行检测
    if (!m_running || !m_serialPort || !m_serialPort->isOpen()) {
        return;
    }

    // 检测方式1：检查串口错误状态
    if (m_serialPort->error() != QSerialPort::NoError) {
        QString errorMsg = "[水声通道] 定时检测：串口异常 - " + m_serialPort->errorString();
        m_lastError = errorMsg;
        emit errorOccurred(errorMsg);
        // 触发停止流程
        m_running = false;
        m_portCheckTimer->stop();
        stopImpl();
        return;
    }

    // 检测方式2：验证串口是否仍有效（Windows/Linux通用）
    QSerialPortInfo portInfo(*m_serialPort);
    if (!portInfo.isValid()) {
        m_lastError = QString("[水声通道] 定时检测：串口硬件已移除（端口：%1）").arg(m_serialPort->portName());
        emit errorOccurred(m_lastError);
        // 触发停止流程
        m_running = false;
        m_portCheckTimer->stop();
        stopImpl();
        emit statusChanged("已断开（硬件移除）");
        return;
    }
}

void WaterAcousticChannel::parseFrames()
{
    bool parsedOneFrame = false;
    while (!parsedOneFrame && !m_recvBuffer.isEmpty()) {
        // 1. 查找水声帧头（A5 A5 A5 D5，4字节）
        QByteArray waHeader = QByteArray(reinterpret_cast<const char*>(FrameConstants::WATER_ACOUSTIC_HEADER), FrameConstants::WATER_ACOUSTIC_HEADER_LENGTH);
        int headIdx = m_recvBuffer.indexOf(waHeader);
        if (headIdx == -1) {
            int maxFrameLen = 2 * (FrameConstants::WATER_ACOUSTIC_HEADER_LENGTH + FrameConstants::WATER_ACOUSTIC_MAX_CONTENT_LEN + FrameConstants::WATER_ACOUSTIC_FOOTER_LENGTH);
            if (m_recvBuffer.size() > maxFrameLen) {
                m_recvBuffer = m_recvBuffer.right(maxFrameLen);
                qDebug() << "[水声解析] 无帧头，清理超长缓冲区后：" << m_recvBuffer.toHex(' ');
            }
            break;
        }

        // 2. 移除帧头前无效数据（仅移除，不截断后续）
        if (headIdx > 0) {
            qDebug() << "[水声解析] 移除帧头前无效数据：" << m_recvBuffer.left(headIdx).toHex(' ');
            m_recvBuffer.remove(0, headIdx);
        }

        // 3. 查找水声帧尾（A5 D4，2字节）
        QByteArray waFooter = QByteArray(reinterpret_cast<const char*>(FrameConstants::WATER_ACOUSTIC_FOOTER), FrameConstants::WATER_ACOUSTIC_FOOTER_LENGTH);
        int footerIdx = m_recvBuffer.indexOf(waFooter, FrameConstants::WATER_ACOUSTIC_HEADER_LENGTH);
        if (footerIdx == -1) {
            
            break;
        }

        // 4. 提取完整水声帧
        int frameLen = footerIdx + FrameConstants::WATER_ACOUSTIC_FOOTER_LENGTH;
        QByteArray frameData = m_recvBuffer.mid(0, frameLen);
        // 移除已解析的帧，剩余字节继续循环解析（核心：连帧的关键）
        m_recvBuffer.remove(0, frameLen);
        qDebug() << "[水声解析] 提取完整帧：" << frameData.toHex(' ') << "，剩余缓冲区：" << m_recvBuffer.toHex(' ');

        // 5. 解析水声帧
        Frame frame;
        CheckError error;
        if (FrameParser::parseWaterAcousticFrame(frameData, frame, error)) {
            // 识别上行指令
            WaterAcousticUplinkCmd cmd = parseWaterAcousticUplinkCmd(frame.dataBody); // 替换原有parseWaterAcousticCmd
            qDebug() << "[水声] 解析到指令：" << static_cast<int>(cmd) << "，内容：" << frame.dataBody.toHex(' ');

            // ========== 分指令处理 ==========
            switch (cmd) {
                // 指令2：对方准备发送数据（A5 A5 A5 D5 02 02 00 A5 D4）
            case WaterAcousticUplinkCmd::RecvNotify: {
                qInfo() << "[水声] 解析到RecvNotify指令（对方通知发送）";
                m_hasRecvNotify = true;
                m_recvNotifyTimer->start(RECV_NOTIFY_TIMEOUT_MS); // 启动校验超时定时器
                // 调用createWaStatusMsg生成状态消息
                DataMessage msg = createWaStatusMsg(
                    "对方准备发送水声数据，等待接收...",
                    WaterAcousticResult::Pending,
                    cmd
                );
                msg.setTimestamp(frame.timestamp);
                emit dataReceived(msg);
                break;
            }
                // 指令1：我方发送完成（A5 A5 A5 D5 01 00 A5 D4）
            case WaterAcousticUplinkCmd::SendFinish: {
                // 调用createWaStatusMsg生成状态消息
                DataMessage msg = createWaStatusMsg(
                    "我方水声数据发送完成",
                    WaterAcousticResult::Success,
                    cmd
                );
                msg.setTimestamp(frame.timestamp);
                emit dataReceived(msg);
                QMutexLocker locker(&m_waMutex);
                qInfo() << "[水声] 处理SendFinish指令，当前缓存数据长度：" << m_sentFrameField.fullWaterFrame.size();
                m_waTimer->stop();  // 仅停止定时器，不清空缓存
                qInfo() << "[水声] SendFinish处理完成，定时器状态：" << (m_waTimer->isActive() ? "运行中" : "已停止");
                break;
            }
            
             // 指令3：对方发送的有效数据
            case WaterAcousticUplinkCmd::DataReceived: {
                if (m_recvNotifyTimer->isActive()) m_recvNotifyTimer->stop();
                m_hasRecvNotify = false;

                Frame coreFrame;
                CheckError coreError;
                if (FrameParser::parseWaterAcousticCoreData(frameData,frame.dataBody, coreFrame, coreError, cmd)) {
                    QMutexLocker locker(&m_waMutex);
                    // ========== 打印缓存的发送字段（验证是否被清空） ==========
                    qDebug() << "[水声] 准备对比字段 - 缓存的发送字段：";
                    qDebug() << "  - 包序号：0x" << QString::number(m_sentFrameField.packetSeq, 16);
                    qDebug() << "  - 通信信道：0x" << QString::number(m_sentFrameField.channel, 16);
                    qDebug() << "  - 发送设备：0x" << QString::number(m_sentFrameField.senderDevice, 16);
                    qDebug() << "  - 接收设备：0x" << QString::number(m_sentFrameField.receiverDevice, 16);
                    qDebug() << "  - 使用阶段：0x" << QString::number(m_sentFrameField.phase, 16);
                    qDebug() << "  - 数据体：" << m_sentFrameField.dataBody.toHex(' ');
                    // ========== 核心：字段对比（无通用帧解析） ==========
                    WaterAcousticResult compareResult = WaterAcousticResult::Success;
                    QStringList mismatchList;

                    // 对比包序号
                    if (coreFrame.packetSeq != m_sentFrameField.packetSeq) {
                        mismatchList.append(QString("包序号[0x%1≠0x%2]").arg(m_sentFrameField.packetSeq, 2, 16).arg(coreFrame.packetSeq, 2, 16));
                    }
                    // 对比通信信道
                    if (coreFrame.channel != m_sentFrameField.channel) {
                        mismatchList.append(QString("通信信道[0x%1≠0x%2]").arg(m_sentFrameField.channel, 2, 16).arg(coreFrame.channel, 2, 16));
                    }
                    // 修复：设备字段互逆对比（上位机发送的sender = 下位机回复的receiver；上位机发送的receiver = 下位机回复的sender）
                    if (m_sentFrameField.senderDevice != coreFrame.receiverDevice) {
                        mismatchList.append(QString("发送设备[0x%1≠0x%2]").arg(m_sentFrameField.senderDevice, 4, 16).arg(coreFrame.receiverDevice, 4, 16));
                    }
                    if (m_sentFrameField.receiverDevice != coreFrame.senderDevice) {
                        mismatchList.append(QString("接收设备[0x%1≠0x%2]").arg(m_sentFrameField.receiverDevice, 4, 16).arg(coreFrame.senderDevice, 4, 16));
                    }
                    // 对比使用阶段
                    if (static_cast<uint8_t>(coreFrame.phase) != m_sentFrameField.phase) {
                        mismatchList.append(QString("使用阶段[0x%1≠0x%2]").arg(m_sentFrameField.phase, 2, 16).arg(static_cast<uint8_t>(coreFrame.phase), 2, 16));
                    }
                    // 对比数据体
                    if (coreFrame.dataBody != m_sentFrameField.dataBody) {
                        mismatchList.append(QString("数据体[%1≠%2]")
                            .arg(QString(m_sentFrameField.dataBody.toHex(' ')))  // 显式转QString
                            .arg(QString(coreFrame.dataBody.toHex(' '))));       // 显式转QString
                    }

                    // 汇总对比结果
                    QString compareDesc;
                    if (mismatchList.isEmpty()) {
                        compareDesc = "所有字段校验通过（时间戳已替换）";
                    }
                    else {
                        compareResult = WaterAcousticResult::Failed;
                        compareDesc = "字段不匹配：" + mismatchList.join("；");
                    }
                    qDebug() << "[水声] 数据对比结果：" << compareDesc;

                    // 生成消息并发送
                    DataMessage msg = createWaDataMsg(compareResult, cmd, m_sentFrameField.dataBody, coreFrame.dataBody);
                    msg.setMessageType(MessageType::Status);
                    msg.setTimestamp(coreFrame.timestamp);
                    emit dataReceived(msg);
                    // 校验成功：清理缓存+停止定时器
                    if (compareResult == WaterAcousticResult::Success) {
                        clearWaDataCache();
                        m_waTimer->stop();
                    }
                }
                else {
                    // 核心数据解析失败 → 调用createWaStatusMsg
                    m_lastError = coreError.description();
                    emit errorOccurred(m_lastError);
                    DataMessage msg = createWaStatusMsg(
                        "解析对方水声数据失败：" + coreError.description(),
                        WaterAcousticResult::Failed,
                        cmd
                    );
                    emit dataReceived(msg);
                }
                break;
            }
            // 未知指令
            case WaterAcousticUplinkCmd::Unknown: {
                DataMessage msg = createWaStatusMsg(
                    QString("收到未知水声指令（内容：%1）").arg(QString(frame.dataBody.toHex(' '))),
                    WaterAcousticResult::Failed,
                    cmd
                );
                msg.setTimestamp(frame.timestamp);
                emit dataReceived(msg);
                break;
            }
            }
        }
        else {
        // 帧解析失败 → 调用createWaStatusMsg
        m_lastError = error.description();
        emit errorOccurred(m_lastError);
        DataMessage msg = createWaStatusMsg(
            "水声帧解析失败：" + error.description(),
            WaterAcousticResult::Failed,
            WaterAcousticUplinkCmd::Unknown
        );
        emit dataReceived(msg);
        }
        parsedOneFrame = true; // 只解析1帧，避免连帧干扰
    }
}

WaterAcousticUplinkCmd WaterAcousticChannel::parseWaterAcousticUplinkCmd(const QByteArray& content)
{
    qDebug() << "[指令匹配] content：" << content.toHex(' ') << "，长度：" << content.size();

    // 1. 优先匹配RecvNotify（对方通知发送：首字节0x02，任意长度）
    if (content.size() >= 1 && content.at(0) == 0x02) {
        qDebug() << "[指令匹配] 识别为RecvNotify";
        return WaterAcousticUplinkCmd::RecvNotify;
    }

    // 2. 匹配SendFinish（兼容所有变体：00 / 01 00 / 01 00 00）
    if (content.size() == 1 && content == QByteArray::fromHex("00")) {
        qDebug() << "[指令匹配] 识别为SendFinish（00）";
        return WaterAcousticUplinkCmd::SendFinish;
    }
    if ((content.size() == 2 && content == QByteArray::fromHex("0100")) ||
        (content.size() == 3 && content == QByteArray::fromHex("010000"))) { // 新增匹配01 00 00
        qDebug() << "[指令匹配] 识别为SendFinish（01 00/01 00 00）";
        return WaterAcousticUplinkCmd::SendFinish;
    }

    // 3. 匹配DataReceived（对方有效数据：长度>0且不是以上指令）
    if (content.size() > 0) {
        return WaterAcousticUplinkCmd::DataReceived;
    }

    // 4. 未知指令
    return WaterAcousticUplinkCmd::Unknown;
}

void WaterAcousticChannel::clearWaDataCache()
{
    QMutexLocker locker(&m_waMutex);
    m_sentFrameField = {};
}

// 数据校验消息：含发送/接收数据+指令类型
DataMessage WaterAcousticChannel::createWaDataMsg(WaterAcousticResult result, WaterAcousticUplinkCmd cmd,
    const QByteArray& sentData, const QByteArray& recvData)
{
    QJsonObject waJson;
    waJson["msg_type"] = "water_acoustic_data"; // 数据类消息标识
    waJson["channel_type"] = static_cast<int>(CommunicationChannel::WaterAcoustic);
    waJson["device_id"] = static_cast<int>(DeviceId::AcousticComm);
    waJson["result"] = static_cast<int>(result);
    waJson["sent_data"] = QString(sentData.toHex());
    waJson["recv_data"] = QString(recvData.toHex());
    waJson["timestamp"] = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
    waJson["cmd_type"] = static_cast<int>(cmd); // 指令枚举值
    waJson["cmd_type_desc"] = waUplinkCmdToString(cmd); // 指令字符串描述

    return DataMessage(
        MessageType::Status,
        CommunicationChannel::WaterAcoustic,
        QJsonDocument(waJson).toJson(),
        ModuleType::SelfCheck
    );
}
// 状态通知消息：含状态描述+指令类型（恢复的核心函数）
DataMessage WaterAcousticChannel::createWaStatusMsg(const QString& statusDesc, WaterAcousticResult result,
    WaterAcousticUplinkCmd cmd)
{
    QJsonObject waJson;
    waJson["msg_type"] = "water_acoustic_status"; // 状态类消息标识
    waJson["channel_type"] = static_cast<int>(CommunicationChannel::WaterAcoustic);
    waJson["device_id"] = static_cast<int>(DeviceId::AcousticComm);
    waJson["cmd_code"] = static_cast<int>(CommandCode::AcousticComm_Test);
    waJson["result"] = static_cast<int>(result);
    waJson["status_desc"] = statusDesc; // 状态描述文本
    waJson["timestamp"] = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
    waJson["cmd_type"] = static_cast<int>(cmd); // 指令枚举值
    waJson["cmd_type_desc"] = waUplinkCmdToString(cmd); // 指令字符串描述

    return DataMessage(
        MessageType::Status,
        CommunicationChannel::WaterAcoustic,
        QJsonDocument(waJson).toJson(),
        ModuleType::SelfCheck
    );
}

