#include "BDSchannel.h"
#include"frameparser.h"
#include<QJsonDocument>
#include<qDebug>
#include<QJsonArray>
#include<QSerialPortInfo>
// AT反馈与指令码的映射（核心：反向匹配）
static QMap<QString, CommandCode> AT_RESPONSE_CMD_MAP = {
    {"CSQ:", CommandCode::BeidouComm_GetSignalQuality},    // 信号质量
    {"LNO:", CommandCode::BeidouComm_GetIdentity},          // 身份查询
    {"PNO:", CommandCode::BeidouComm_GetTarget},            // 目标查询
    {"White List:", CommandCode::BeidouComm_GetWhitelist},  // 白名单查询
    {"DEF:OK", CommandCode::BeidouComm_FactoryReset},       // 恢复出厂
    {"RESET:OK", CommandCode::BeidouComm_Reboot},           // 重启
    {"STACNT", CommandCode::BeidouComm_GetConnectStatus},   // 连接状态
    {"GNRMC:", CommandCode::BeidouComm_GetPosition}         // 定位信息
};
BDSChannel::BDSChannel(QObject* parent)
    : CommunicationChannelBase(CommunicationChannel::BDS, parent)
    , m_shortMsgTimer(new QTimer(this)) // 初始化超时定时器
    , m_portCheckTimer(new QTimer(this)) // 初始化兜底定时器
{
    m_serialPort = new QSerialPort(this);
    // 初始化兜底检测定时器（2秒检测一次，可调整）
    m_portCheckTimer->setInterval(2000);

    connect(m_portCheckTimer, &QTimer::timeout, this, &BDSChannel::checkPortAvailability);
    //连接基类信号到当前的槽函数（QueuedConnection确保在所属线程执行）
    connect(this, &CommunicationChannelBase::sendDataRequested,
        this, &BDSChannel::sendDataImpl, Qt::QueuedConnection);
    connect(this, &BDSChannel::sendDataRequestedWithType,
        this, &BDSChannel::sendDataImplWithType, Qt::QueuedConnection);
    connect(this, &CommunicationChannelBase::setConfigRequested,
        this, &BDSChannel::setConfigImpl, Qt::QueuedConnection);
    connect(this, &CommunicationChannelBase::startRequested,
        this, &BDSChannel::startImpl, Qt::QueuedConnection);
    connect(this, &CommunicationChannelBase::stopRequested,
        this, &BDSChannel::stopImpl, Qt::QueuedConnection);

    //串口自身的信号
    connect(m_serialPort, &QSerialPort::readyRead, this, &BDSChannel::onDataReady);
    connect(m_serialPort, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
        this, &BDSChannel::onError);
    //短报文超时定时器配置
    m_shortMsgTimer->setSingleShot(true); // 单次触发
    connect(m_shortMsgTimer, &QTimer::timeout, this, &BDSChannel::onShortMessageTimeout);
}

BDSChannel::~BDSChannel() {
    stop();
    clearShortMessageCache(); // 析构时清理缓存
    if (m_portCheckTimer) {
        m_portCheckTimer->stop();
        delete m_portCheckTimer;
        m_portCheckTimer = nullptr;
    }
}

bool BDSChannel::setConfig(const ChannelConfigPtr& config)
{
    emit setConfigRequested(config);//触发配置信号(由setConfigImp处理)
    return true;//此处返回值仅表示成功触发，实际结果通过setConfigFinished获取
}

ChannelConfigPtr BDSChannel::config() const {
    return m_config;
}

bool BDSChannel::start()
{
    emit startRequested();//触发启动信号(由startImpl处理)
    return true;

}

void BDSChannel::stop()
{
    emit stopRequested();//触发停止信号(由stopImpl处理)

}

bool BDSChannel::isRunning() const {
    return m_running;
}

QString BDSChannel::lastError() const
{
    return m_lastError;
}


QString BDSChannel::statusInfo() const {
    return m_running ? "已连接到串口: " + m_config->m_portName : "已停止";
}

bool BDSChannel::sendShortMessage(const QByteArray& data)
{
    if (data.isEmpty()) {
        m_lastError = "短报文数据为空";
        emit errorOccurred(m_lastError);
        return false;
    }
    // 触发带类型的发送实现
    QMetaObject::invokeMethod(this, "sendDataImplWithType",
        Qt::QueuedConnection, Q_ARG(QByteArray, data), Q_ARG(BDSDataType, BDSDataType::ShortMessage));
    return true;
}

void BDSChannel::onDataReady() {
    //1.读取硬件(串口)原始数据
    QByteArray rawData = m_serialPort->readAll();
    if (rawData.isEmpty())
        return;

    //2.拼接数据到缓冲区
    m_recvBuffer += rawData;
    qDebug() << "BDSChannel接收原始数据：" << rawData.toHex(' ');

    //先提取短报文回复数据并校验一致性
    QByteArray recvShortMsg = extractShortMessageData(m_recvBuffer);
    if (!recvShortMsg.isEmpty()) {
        QMutexLocker locker(&m_shortMsgMutex);
        if (!m_sentShortMsg.isEmpty()) {
            DataMessage msg;
            //逐字节对比发送和回复的短报文数据
            if (recvShortMsg == m_sentShortMsg) {
                qDebug() << "[BDS] 短报文数据一致 - 发送：" << m_sentShortMsg.toHex(' ')
                    << "回复：" << recvShortMsg.toHex(' ');
                msg = createShortMessageDataMsg(BDSShortMsgResult::Success, m_sentShortMsg,
                    recvShortMsg);
            }
            else {
                qWarning() << "[BDS] 短报文数据不一致 - 发送：" << m_sentShortMsg.toHex(' ')
                    << "回复：" << recvShortMsg.toHex(' ');
                // 短报文不一致：生成Failed结果的DataMessage
                msg = createShortMessageDataMsg(BDSShortMsgResult::Failed, m_sentShortMsg,
                    recvShortMsg);
            }
            msg.setTimestamp(QDateTime::currentDateTime());
            emit dataReceived(msg); // 发送短报文的DataMessage
            // 对比完成后清理缓存+停止定时器
            clearShortMessageCache();
            m_shortMsgTimer->stop();
        }
    }

    // 3. 解析AT反馈
    parseATResponse();
}

void BDSChannel::onError(QSerialPort::SerialPortError error) {
    if (error != QSerialPort::NoError) {
        QString errorMsg = "串口错误: " + m_serialPort->errorString();
        qWarning() << "[北斗通道] 串口异常：" << errorMsg;

        // 重点：捕获硬件意外移除错误（ResourceError）
        if (error == QSerialPort::ResourceError) {
            errorMsg = QString("北斗串口硬件已意外移除（端口：%1）").arg(m_serialPort->portName());
            m_lastError = errorMsg;

            // 1. 强制标记通道为未运行状态
            if (m_running) {
                m_running = false;
            }

            // 2. 清理北斗专属缓存和定时器
            clearShortMessageCache();
            m_shortMsgTimer->stop();

            // 3. 重置白名单相关状态
            QMutexLocker whitelistLocker(&m_whitelistMutex);
            m_isParsingWhitelist = false;
            m_whitelistCardCache.clear();
            QMutexLocker cmdLocker(&m_whitelistCmdMutex);
            m_lastWhitelistCmd = CommandCode::Invalid;

            // 4. 主动调用停止实现，清理串口
            stopImpl();

            // 5. 兜底关闭串口
            if (m_serialPort->isOpen()) {
                m_serialPort->close();
            }
        }
        else {
            // 其他普通错误，保留原有逻辑
            m_lastError = errorMsg;
        }

        // 发送错误信号和状态变更信号，同步UI
        emit errorOccurred(m_lastError);
        emit statusChanged(m_running ? "运行中" : "已断开（硬件移除）");
    }
}

void BDSChannel::sendDataImpl(const QByteArray& data)
{
    sendDataImplWithType(data, BDSDataType::ATCommand);
}

void BDSChannel::sendDataImplWithType(const QByteArray& data, BDSDataType type)
{
    if (!m_running || !m_serialPort->isOpen())
    {
        m_lastError = "通道未连接，无法发送数据";
        emit errorOccurred(m_lastError);
        emit sendDataFinished(false);
        return;
    }
    if (m_serialPort->error() != QSerialPort::NoError) {
        m_lastError = QString("串口存在未处理错误：%1").arg(m_serialPort->errorString());
        emit errorOccurred(m_lastError);
        emit sendDataFinished(false);
        return;
    }
    // 2. 数据合法性检查
    if (data.isEmpty()) {
        m_lastError = "发送数据为空";
        emit errorOccurred(m_lastError);
        emit sendDataFinished(false);
        return;
    }
    if (data.size() > 4096) { // 假设最大缓冲区为4096字节，可根据硬件调整
        m_lastError = QString("数据过长（%1字节），超过最大限制（4096字节）").arg(data.size());
        emit errorOccurred(m_lastError);
        emit sendDataFinished(false);
        return;
    }

    // ========== 识别白名单指令，更新缓存 ==========
    QString atCmdStr = QString::fromUtf8(data).trimmed();
    QMutexLocker cmdLocker(&m_whitelistCmdMutex);
    if (type == BDSDataType::ATCommand) { // 仅AT指令需要缓存
        if (atCmdStr.startsWith("AT+ADDPD")) {
            // 添加白名单指令
            m_lastWhitelistCmd = CommandCode::BeidouComm_AddWhitelist;
            qDebug() << "[BDS] 缓存白名单指令类型：AddWhitelist";
        }
        else if (atCmdStr.startsWith("AT+DELPD")) {
            // 删除白名单指令
            m_lastWhitelistCmd = CommandCode::BeidouComm_DelWhitelist;
            qDebug() << "[BDS] 缓存白名单指令类型：DelWhitelist";
        }
        else if (atCmdStr.startsWith("AT+LISTPD")) {
            // 查询白名单指令
            m_lastWhitelistCmd = CommandCode::BeidouComm_GetWhitelist;
            qDebug() << "[BDS] 缓存白名单指令类型：GetWhitelist";
        }
       
    }

    // ==========分离「发送数据」和「对比缓存数据」 ==========
    QByteArray pureFrameData; // 纯帧数据（用于对比）
    if (type == BDSDataType::ShortMessage) {
        // 从完整AT指令中提取纯帧数据（剥离AT+SND前缀和\r\n后缀）
        // 提取规则：AT+SND 后、\r\n 前的内容即为纯帧数据
        QString fullAtCmd = QString::fromUtf8(data);
        int sndSuffixIndex = fullAtCmd.indexOf("AT+SND ") + 6; // 跳过"AT+SND "
        int lineEndIndex = fullAtCmd.lastIndexOf("\r\n");
        if (sndSuffixIndex < lineEndIndex) {
            pureFrameData = fullAtCmd.mid(sndSuffixIndex, lineEndIndex - sndSuffixIndex).toUtf8();
        }

        QMutexLocker locker(&m_shortMsgMutex);
        // 缓存纯帧数据（不是完整AT指令），用于后续与接收端纯帧数据对比
        m_sentShortMsg = pureFrameData;
        m_shortMsgTimer->start(SHORT_MSG_TIMEOUT);
        qDebug() << "[BDS] 发送完整AT指令：" << data.trimmed();
        qDebug() << "[BDS] 缓存纯帧数据（用于对比）：" << pureFrameData.toHex(' ');
    }
    else {
        qDebug() << "[BDS] 发送普通AT指令：" << data.trimmed();
    }


    qint64 bytesWritten = m_serialPort->write(data);
    qDebug() << "BDS 发送数据（" << bytesWritten << "字节）：" << data.toHex(' ');
    // 关键修正：success判断逻辑（原代码逻辑颠倒）
    bool success = (bytesWritten == data.size());
    if (!success) {
        // 详细错误信息
        QSerialPort::SerialPortError error = m_serialPort->error();
        QString errorDetail;
        switch (error) {
        case QSerialPort::WriteError: errorDetail = "写入失败"; break;
        case QSerialPort::NotOpenError: errorDetail = "串口未打开"; break;
        case QSerialPort::ResourceError: errorDetail = "硬件资源错误（可能已断开）"; break;
        default: errorDetail = m_serialPort->errorString();
        }
        m_lastError = QString("发送失败（实际发送%1字节）：%2").arg(bytesWritten).arg(errorDetail);
        qDebug() << "BDS 错误:" << m_lastError;
        emit errorOccurred(m_lastError);

        // 发送失败时清理短报文缓存+停止定时器
        if (type == BDSDataType::ShortMessage) {
            clearShortMessageCache();
            m_shortMsgTimer->stop();
        }
    }
    else {
        // 确保数据被立即发送（而非缓存）
        m_serialPort->flush();
    }
    emit sendDataFinished(success);
}

void BDSChannel::setConfigImpl(const ChannelConfigPtr& config)
{
    auto bdsConfig = qSharedPointerDynamicCast<BDSConfig>(config);
    if (!bdsConfig)
    {
        emit setConfigFinished(false);
        return;
    }
    m_config = bdsConfig;
    emit setConfigFinished(true);//配置更新成功(生效需启动通道)
}

void BDSChannel::startImpl()
{
    if (m_running)
    {
        emit startFinished(true);
        return;
    }
    //应用配置并打开串口
    if (!m_config)
    {
        m_lastError = "配置未设置";
        emit errorOccurred(m_lastError);
        return;
    }

    m_serialPort->setPortName(m_config->m_portName);
    m_serialPort->setBaudRate(m_config->m_baudRate);
    m_serialPort->setDataBits(m_config->m_dataBits);
    m_serialPort->setParity(m_config->m_parity);
    m_serialPort->setStopBits(m_config->m_stopBits);
    m_serialPort->setFlowControl(m_config->m_flowControl);
    if (m_serialPort->open(QIODevice::ReadWrite))
    {
        m_running = true;
        m_portCheckTimer->start();
        emit statusChanged("运行中");
        emit startFinished(true);
    }
    else
    {
        m_lastError = "无法打开串口: " + m_config->m_portName;
        emit errorOccurred(m_lastError);
        emit startFinished(false);

    }
}

void BDSChannel::stopImpl()
{
    if (!m_running)
    {
        emit stopFinished();
        return;
    }
    m_portCheckTimer->stop();
    //停止时清理短报文缓存+定时器
    clearShortMessageCache();
    m_shortMsgTimer->stop();

    // 重置白名单解析状态和缓存
    QMutexLocker whitelistLocker(&m_whitelistMutex);
    m_isParsingWhitelist = false;
    m_whitelistCardCache.clear();
    QMutexLocker cmdLocker(&m_whitelistCmdMutex);
    m_lastWhitelistCmd = CommandCode::Invalid;
    m_running = false;
    m_serialPort->close();
    emit statusChanged("已停止");
    emit stopFinished();
}

void BDSChannel::onShortMessageTimeout()
{
    QMutexLocker locker(&m_shortMsgMutex);
    QByteArray sentData = m_sentShortMsg;
    clearShortMessageCache(); // 超时后清理缓存

    qWarning() << "[BDS] 短报文回复超时 - 发送数据：" << sentData.toHex(' ');
    // 生成超时结果的DataMessage
    DataMessage msg = createShortMessageDataMsg(BDSShortMsgResult::Timeout, sentData, QByteArray());
    msg.setTimestamp(QDateTime::currentDateTime());
    msg.setCommandType(CommandType::Internal);
    emit dataReceived(msg);

    QMutexLocker cmdLocker(&m_whitelistCmdMutex);
    m_lastWhitelistCmd = CommandCode::Invalid;
}

void BDSChannel::checkPortAvailability()
{ // 仅在通道运行时执行检测
    if (!m_running || !m_serialPort || !m_serialPort->isOpen()) {
        return;
    }

    // 检测方式1：检查串口错误状态
    if (m_serialPort->error() != QSerialPort::NoError) {
        QString errorMsg = "[北斗通道] 定时检测：串口异常 - " + m_serialPort->errorString();
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
        m_lastError = QString("[北斗通道] 定时检测：串口硬件已移除（端口：%1）").arg(m_serialPort->portName());
        emit errorOccurred(m_lastError);
        // 触发停止流程
        m_running = false;
        m_portCheckTimer->stop();
        stopImpl();
        emit statusChanged("已断开（硬件移除）");
        return;
    }
}

void BDSChannel::parseATResponse()
{
    // 1. 按行分割AT反馈
    QStringList lines = splitBufferByLine();
    if (lines.isEmpty()) return;

    // 白名单临时缓存（用于聚合多行数据）
    QMutexLocker whitelistLocker(&m_whitelistMutex);
    QStringList currentWhitelistCards; // 本次解析收集的白名单卡号

    // ========== 获取缓存的白名单指令类型 ==========
    QMutexLocker cmdLocker(&m_whitelistCmdMutex);
    CommandCode lastWhitelistCmd = m_lastWhitelistCmd;
    cmdLocker.unlock(); // 提前解锁，避免锁持有过久
    // ==================================================
   
    // 2. 遍历每行反馈，封装纯北斗数据（无Frame）
    for (const QString& line : lines) {
        QString trimmedLine = line.trimmed();
        if (trimmedLine.isEmpty()) continue;
        qDebug() << "[BDSChannel] 原始AT反馈：" << trimmedLine;
        // ========== ：处理白名单失败反馈（ID error!） ==========
        if (trimmedLine == "ID error!") {
            // 仅当缓存了白名单指令时，才处理为白名单失败反馈
            if (lastWhitelistCmd == CommandCode::BeidouComm_AddWhitelist ||
                lastWhitelistCmd == CommandCode::BeidouComm_DelWhitelist ||
                lastWhitelistCmd == CommandCode::BeidouComm_GetWhitelist) {

                // 构造失败反馈JSON
                QJsonObject bdsJson;
                bdsJson["msg_type"] = "at_response";
                bdsJson["channel_type"] = static_cast<int>(CommunicationChannel::BDS);
                bdsJson["device_id"] = static_cast<int>(DeviceId::BeidouComm);
                bdsJson["cmd_code"] = static_cast<int>(lastWhitelistCmd); // 关联对应的操作类型
                bdsJson["raw_at_response"] = trimmedLine;
                bdsJson["result"] = "failed"; // 标记失败
                bdsJson["error_msg"] = "卡号无效或操作失败";
                bdsJson["timestamp"] = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");

                // 发送失败DataMessage
                DataMessage msg(
                    MessageType::Status,
                    CommunicationChannel::BDS,
                    QJsonDocument(bdsJson).toJson(),
                    ModuleType::SelfCheck
                );
                msg.setCommandType(CommandType::Internal);
                msg.setTimestamp(QDateTime::currentDateTime());
                emit dataReceived(msg);

                // 清空白名单指令缓存（避免重复处理）
                QMutexLocker cmdLocker2(&m_whitelistCmdMutex);
                m_lastWhitelistCmd = CommandCode::Invalid;

                // 重置白名单解析状态
                m_isParsingWhitelist = false;
                m_whitelistCardCache.clear();
            }
            continue; // 跳过后续逻辑
        }
        // 3. 先处理白名单特殊逻辑（聚合标题与卡号）
        if (trimmedLine.startsWith("White List:")) {
            // 标记进入白名单解析状态，初始化缓存
            m_isParsingWhitelist = true;
            m_whitelistCardCache.clear();
            currentWhitelistCards.clear();
            continue; // 标题行不单独发送，等待后续卡号行
        }
        else if (m_isParsingWhitelist) {
            // 处于白名单解析状态，判断当前行是否为有效卡号（7位数字）
            bool isCardNumber = true;
            for (QChar c : trimmedLine) {
                if (!c.isDigit()) {
                    isCardNumber = false;
                    break;
                }
            }
            if (isCardNumber && trimmedLine.length() == 7) {
                // 有效卡号，加入缓存
                m_whitelistCardCache.append(trimmedLine);
                currentWhitelistCards.append(trimmedLine);
                continue; // 卡号行暂不发送，等待聚合完成
            }
            else {
                // 非卡号行，标记白名单解析结束（后续行恢复正常解析）
                m_isParsingWhitelist = false;
            }
        }

        // 4. 简单匹配cmdCode（仅用于分类，不解析内容）
        CommandCode cmdCode = CommandCode::Invalid;
        if (trimmedLine.startsWith("STACNT")) {
            cmdCode = CommandCode::BeidouComm_GetConnectStatus;
        }
        else if (trimmedLine.startsWith("CSQ:")) {
            cmdCode = CommandCode::BeidouComm_GetSignalQuality;
        }
        else if (trimmedLine.startsWith("LNO:")) {
            cmdCode = CommandCode::BeidouComm_GetIdentity;
        }
        else if (trimmedLine.startsWith("PNO:")) {
            cmdCode = CommandCode::BeidouComm_GetTarget; // 查/设反馈格式一致，后续由适配器区分
        }
        else if (trimmedLine.startsWith("$GNRMC")) {
            cmdCode = CommandCode::BeidouComm_GetPosition;
        }
        else if (trimmedLine.startsWith("VER:")) {
            cmdCode = CommandCode::BeidouComm_GetSysInfo;
        }
        else if (trimmedLine == "DEF:OK") {
            cmdCode = CommandCode::BeidouComm_FactoryReset;
        }
        else if (trimmedLine == "RESET:OK") {
            cmdCode = CommandCode::BeidouComm_Reboot;
        }
        else if (trimmedLine.isEmpty() || trimmedLine == "NO SIM!" || trimmedLine.startsWith("SND:ERROR")) {
            cmdCode = CommandCode::BeidouComm_Test;
        }
       
        else {
            cmdCode = CommandCode::Common_GetFault;
        }

        // 4. 构造北斗专用JSON（无Frame，仅核心字段）
        QJsonObject bdsJson;
        bdsJson["msg_type"] = "at_response"; // 标记为AT指令反馈
        bdsJson["channel_type"] = static_cast<int>(CommunicationChannel::BDS); // 标记为BDS通道
        bdsJson["device_id"] = static_cast<int>(DeviceId::BeidouComm);         // 固定北斗设备ID
        bdsJson["cmd_code"] = static_cast<int>(cmdCode);                       // 匹配的命令码
        bdsJson["raw_at _response"] = trimmedLine;                              // 原始AT反馈
        bdsJson["timestamp"] = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");

        // 区分普通AT反馈、白名单反馈、GNRMC反馈
        if (cmdCode == CommandCode::BeidouComm_GetWhitelist && !m_whitelistCardCache.isEmpty()) {
            // 白名单反馈
            bdsJson["raw_at_response"] = QString("White List:\n%1").arg(m_whitelistCardCache.join("\n"));
            bdsJson["whitelist_title"] = "White List";
            bdsJson["whitelist_cards"] = QJsonArray::fromStringList(m_whitelistCardCache);
        }
        else if (cmdCode == CommandCode::BeidouComm_GetPosition && trimmedLine.startsWith("$GNRMC")) {
            if (trimmedLine.length() < 10 || !trimmedLine.contains('*')) {
                qWarning() << "[GNRMC解析] 无效的GNRMC语句：" << trimmedLine;
                bdsJson["raw_at_response"] = trimmedLine;
                bdsJson["gnrmc_data"] = QJsonObject({ {"error", "无效的GNRMC语句格式"} });
            }
            else {
                // 正常解析
                bdsJson["raw_at_response"] = trimmedLine;
                bdsJson["gnrmc_data"] = parseGNRMC(trimmedLine);
            }
        }
        else {
            // 普通AT反馈
            bdsJson["raw_at_response"] = trimmedLine;
        }
        DataMessage msg(
            MessageType::Status,
            CommunicationChannel::BDS,
            QJsonDocument(bdsJson).toJson(),
            ModuleType::SelfCheck
        );
        msg.setTimestamp(QDateTime::currentDateTime());
        msg.setCommandType(CommandType::Internal);
        emit dataReceived(msg);
    }
    // 7. 处理解析结束后仍缓存有白名单卡号的情况（避免遗漏）
    if (m_isParsingWhitelist && !m_whitelistCardCache.isEmpty()) {
        // 构造完整的白名单AT反馈JSON（仍为at_response类型）
        QJsonObject bdsJson;
        bdsJson["msg_type"] = "at_response"; // 保持统一
        bdsJson["channel_type"] = static_cast<int>(CommunicationChannel::BDS);
        bdsJson["device_id"] = static_cast<int>(DeviceId::BeidouComm);
        // ========== 关键修改：不再固定为GetWhitelist，使用缓存的指令类型 ==========
        CommandCode targetCmdCode = CommandCode::BeidouComm_GetWhitelist; // 默认查询
        QMutexLocker cmdLocker2(&m_whitelistCmdMutex);
        if (m_lastWhitelistCmd == CommandCode::BeidouComm_AddWhitelist ||
            m_lastWhitelistCmd == CommandCode::BeidouComm_DelWhitelist ||
            m_lastWhitelistCmd == CommandCode::BeidouComm_GetWhitelist) {
            targetCmdCode = m_lastWhitelistCmd; // 替换为实际操作类型
            m_lastWhitelistCmd = CommandCode::Invalid; // 处理后清空缓存
        }
        cmdLocker2.unlock();
        bdsJson["cmd_code"] = static_cast<int>(targetCmdCode); // 关联Add/Del/Get
        bdsJson["raw_at_response"] = QString("White List:\n%1").arg(m_whitelistCardCache.join("\n"));
        // 白名单专属字段（仅此时存在）
        bdsJson["whitelist_title"] = "White List";
        bdsJson["whitelist_cards"] = QJsonArray::fromStringList(m_whitelistCardCache);
        bdsJson["result"] = "success"; // 标记成功
        bdsJson["timestamp"] = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");

        DataMessage msg(
            MessageType::Status,
            CommunicationChannel::BDS,
            QJsonDocument(bdsJson).toJson(),
            ModuleType::SelfCheck
        );
        msg.setCommandType(CommandType::Internal);
        msg.setTimestamp(QDateTime::currentDateTime());
        emit dataReceived(msg);

        // 重置白名单解析状态和缓存
        m_isParsingWhitelist = false;
        m_whitelistCardCache.clear();
    }
}

QJsonObject BDSChannel::parseGNRMC(const QString& gnrmcStr)
{
    QJsonObject gnrmcJson;
    // 按逗号分割GNRMC字段（忽略语句头的$和尾部的校验和分隔符*）
    QStringList fields = gnrmcStr.split(',', Qt::KeepEmptyParts);
    // 补全空字段（确保字段数量一致，即使部分为空）
    if (fields.size() > 14) {
        fields = fields.mid(0, 14);
    }
    // 安全补全字段：确保fields有14个元素，不足则在末尾补空（保持原始字段顺序）
    while (fields.size() < 14) {
        fields.append("");
    }
    // 1. 基础信息
    gnrmcJson["statement_type"] = "GNRMC";
    gnrmcJson["raw_gnrmc"] = gnrmcStr;

    // 2. 核心定位字段（按NMEA-0183标准映射）
    QString utcTime = fields[1]; // 索引1：UTC时间
    QString fixStatus = fields[2]; // 索引2：定位状态（A/V）
    QString latitude = fields[3]; // 索引3：纬度
    QString latHemisphere = fields[4]; // 索引4：纬度半球
    QString longitude = fields[5]; // 索引5：经度
    QString lonHemisphere = fields[6]; // 索引6：经度半球
    QString groundSpeed = fields[7]; // 索引7：地面速度（节）
    QString groundCourse = fields[8]; // 索引8：地面航向
    QString utcDate = fields[9]; // 索引9：UTC日期
    QString magneticVariation = fields[10]; // 索引10：磁偏角
    QString magVarDirection = fields[11]; // 索引11：磁偏角方向
    QString modeAndChecksum = fields[12]; // 索引12：模式指示+校验和（如"N,V*37"）

     // 关键修改3：安全提取模式指示和校验和，避免越界
    QString modeIndicator = "";
    QString checksum = "";
    if (!modeAndChecksum.isEmpty()) {
        QStringList modeChecksumList = modeAndChecksum.split('*');
        // 校验split后的数组长度，再访问对应索引
        if (modeChecksumList.size() >= 1) {
            modeIndicator = modeChecksumList[0];
        }
        if (modeChecksumList.size() >= 2) {
            checksum = modeChecksumList[1];
        }
    }
    // 3. 赋值到JSON，同时添加易理解的中文说明
    gnrmcJson["utc_time"] = utcTime;
    gnrmcJson["fix_status"] = fixStatus;
    gnrmcJson["fix_status_desc"] = (fixStatus == "A") ? "有效定位" : (fixStatus == "V") ? "无效定位" : "未知状态";
    gnrmcJson["latitude"] = latitude;
    gnrmcJson["latitude_hemisphere"] = latHemisphere;
    gnrmcJson["longitude"] = longitude;
    gnrmcJson["longitude_hemisphere"] = lonHemisphere;
    gnrmcJson["ground_speed_knot"] = groundSpeed;
    gnrmcJson["ground_speed_kmh"] = groundSpeed.isEmpty() ? 0.0 : (groundSpeed.toDouble() * 1.852); // 转换为km/h
    gnrmcJson["ground_course_degree"] = groundCourse;
    gnrmcJson["utc_date"] = utcDate;
    gnrmcJson["magnetic_variation"] = magneticVariation;
    gnrmcJson["magnetic_variation_direction"] = magVarDirection;
    gnrmcJson["mode_indicator"] = modeIndicator;
    gnrmcJson["mode_indicator_desc"] = modeIndicator == "A" ? "自主定位" : (modeIndicator == "D" ? "差分定位" : (modeIndicator == "N" ? "无效模式" : "未知模式"));
    gnrmcJson["checksum"] = checksum;

    // 4. 辅助判断：是否有效定位（增加字段非空校验）
    bool isValidFix = (fixStatus == "A") && !latitude.isEmpty() && !longitude.isEmpty() && !latHemisphere.isEmpty() && !lonHemisphere.isEmpty();
    gnrmcJson["is_valid_fix"] = isValidFix;

    // 5. 安全转换经纬度（增加异常防护）
    auto convertNMEAToDegree = [](const QString& nmeaStr, const QString& hemisphere) -> double {
        if (nmeaStr.isEmpty()) return 0.0;
        // 拆分度和分（纬度：ddmm.mmmm；经度：dddmm.mmmm）
        int degreeLen = (hemisphere == "N" || hemisphere == "S") ? 2 : 3;
        if (nmeaStr.length() < degreeLen) return 0.0; // 避免字符串长度不足引发异常

        QString degreeStr = nmeaStr.left(degreeLen);
        QString minuteStr = nmeaStr.mid(degreeLen);
        bool degreeOk = false;
        bool minuteOk = false;
        double degree = degreeStr.toDouble(&degreeOk);
        double minute = minuteStr.toDouble(&minuteOk);
        if (!degreeOk || !minuteOk) return 0.0;

        return degree + (minute / 60.0);
    };

    double latDegree = convertNMEAToDegree(latitude, latHemisphere);
    double lonDegree = convertNMEAToDegree(longitude, lonHemisphere);
    // 处理南北纬、东西经的正负
    if (latHemisphere == "S") latDegree = -latDegree;
    if (lonHemisphere == "W") lonDegree = -lonDegree;
    gnrmcJson["latitude_degree"] = latDegree; // 十进制纬度
    gnrmcJson["longitude_degree"] = lonDegree; // 十进制经度

    // 6. 日志打印：便于排查问题，不引发异常
    if (!isValidFix) {
        qWarning() << "[GNRMC解析] 当前无有效定位，状态：" << gnrmcJson["fix_status_desc"].toString();
    }
    else {
        qInfo() << "[GNRMC解析] 有效定位 - 纬度：" << latDegree << "°，经度：" << lonDegree << "°";
    }

    return gnrmcJson;
}

QStringList BDSChannel::splitBufferByLine()
{
    QStringList lines;
    QString bufferStr = QString::fromUtf8(m_recvBuffer).replace("\r\n", "\n");
    int splitIdx = bufferStr.lastIndexOf('\n');
    if (splitIdx == -1) return lines; // 无完整行

    // 提取所有完整行
    QString completeLines = bufferStr.left(splitIdx);
    lines = completeLines.split('\n', Qt::SkipEmptyParts);

    // 保留未完成的行到缓冲区
    m_recvBuffer = bufferStr.mid(splitIdx + 1).toUtf8();
    return lines;
}

QByteArray BDSChannel::extractShortMessageData(const QByteArray& rawData)
{
    QMutexLocker locker(&m_shortMsgMutex);
    if (m_sentShortMsg.isEmpty()) {
        return QByteArray(); // 无待校验的发送数据，直接返回空
    }

    // 1. 【轻量化过滤】：仅移除无关控制字符和非帧数据关键字（接收端无AT+SND，无需过滤）
    QByteArray filteredData = rawData;
    // 过滤其他AT响应关键字（避免干扰纯帧数据提取）
    QString filterKeywords[] = { "CSQ:", "LNO:", "PNO:", "SND:", "NO SIM!", "White List:", "DEF:OK", "RESET:OK" };
    for (const QString& keyword : filterKeywords) {
        QByteArray keywordBytes = keyword.toUtf8();
        int keywordIndex = filteredData.indexOf(keywordBytes);
        while (keywordIndex != -1) {
            filteredData.remove(keywordIndex, keywordBytes.size());
            keywordIndex = filteredData.indexOf(keywordBytes);
        }
    }
    // 过滤控制字符（\r、\n）和空格（接收端可能附带的无效字符）
    filteredData = filteredData.replace('\r', QByteArray())
        .replace('\n', QByteArray())
        .replace(' ', QByteArray());

    // 2. 【精准匹配】：在过滤后的数据中查找发送的纯帧数据
    int frameIndex = filteredData.indexOf(m_sentShortMsg);
    if (frameIndex != -1) {
        // 提取匹配到的纯帧数据（与发送的纯帧数据长度一致）
        QByteArray matchedFrame = filteredData.mid(frameIndex, m_sentShortMsg.size());
        // 从接收缓冲区移除已匹配的帧数据（避免重复校验）
        m_recvBuffer = m_recvBuffer.remove(m_recvBuffer.indexOf(matchedFrame), matchedFrame.size());
        qDebug() << "[BDS] 接收端提取到纯帧数据：" << matchedFrame.toHex(' ');
        return matchedFrame;
    }

    return QByteArray();
}

void BDSChannel::clearShortMessageCache()
{
    QMutexLocker locker(&m_shortMsgMutex);
    m_sentShortMsg.clear();
}

DataMessage BDSChannel::createShortMessageDataMsg(BDSShortMsgResult result, const QByteArray& sentData, const QByteArray& recvData)
{
    QJsonObject shortMsgJson;
    shortMsgJson["msg_type"] = "short_message"; // 标记为短报文
    shortMsgJson["channel_type"] = static_cast<int>(CommunicationChannel::BDS);
    shortMsgJson["device_id"] = static_cast<int>(DeviceId::BeidouComm);
    shortMsgJson["result"] = static_cast<int>(result); // 0:Success, 1:Failed, 2:Timeout
    shortMsgJson["sent_data"] = QString(sentData.toHex()); // 发送数据（十六进制便于查看）
    shortMsgJson["recv_data"] = QString(recvData.toHex()); // 接收数据（超时为空）
    shortMsgJson["timestamp"] = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");

    DataMessage msg(
        MessageType::Status,
        CommunicationChannel::BDS,
        QJsonDocument(shortMsgJson).toJson(),
        ModuleType::SelfCheck
    );
    msg.setCommandType(CommandType::Internal);
    return msg;
}

