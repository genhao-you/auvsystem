#include "radiochannel.h"
#include"frameparser.h"
#include<QJsonDocument>
#include<qDebug>
#include<QEventLoop>
#include<QTimer>
#include<QSerialPortInfo>
RadioChannel::RadioChannel(QObject* parent)
    : CommunicationChannelBase(CommunicationChannel::Radio, parent)
    , m_portCheckTimer(new QTimer(this)) // 初始化兜底定时器
{
    m_serialPort = new QSerialPort(this);
    //初始化硬件检测定时器（每隔2秒检测一次）
    m_portCheckTimer->setInterval(2000); // 2秒检测一次，可调整
    connect(m_portCheckTimer, &QTimer::timeout, this, &RadioChannel::checkPortAvailability);
    //连接基类信号到当前的槽函数（QueuedConnection确保在所属线程执行）
    connect(this, &CommunicationChannelBase::sendDataRequested,
        this, &RadioChannel::sendDataImpl, Qt::QueuedConnection);
    connect(this, &CommunicationChannelBase::setConfigRequested,
        this, &RadioChannel::setConfigImpl, Qt::QueuedConnection);
    connect(this, &CommunicationChannelBase::startRequested,
        this, &RadioChannel::startImpl, Qt::QueuedConnection);
    connect(this, &CommunicationChannelBase::stopRequested,
        this, &RadioChannel::stopImpl, Qt::QueuedConnection);

    //串口自身的信号
    connect(m_serialPort, &QSerialPort::readyRead, this, &RadioChannel::onDataReady);
    connect(m_serialPort, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
        this, &RadioChannel::onError);
}

RadioChannel::~RadioChannel() {
    // 析构时强制关闭（线程安全）
    if (m_running) {
        QEventLoop loop;
        // 连接关闭完成信号，触发事件循环退出
        QMetaObject::Connection conn = connect(this, &CommunicationChannelBase::stopFinished,
            &loop, &QEventLoop::quit);
        // 触发关闭请求
        stop();
        // 设置超时（1000ms），避免事件循环卡死
        QTimer::singleShot(1000, &loop, &QEventLoop::quit);
        // 进入事件循环，直到 stopFinished 触发或超时
        loop.exec();
        // 断开临时连接
        disconnect(conn);
    }
    // 释放串口资源
    if (m_serialPort) {
        if (m_serialPort->isOpen()) {
            m_serialPort->close();
        }
        delete m_serialPort;
        m_serialPort = nullptr;
    }
    if (m_portCheckTimer) {
        m_portCheckTimer->stop();
        delete m_portCheckTimer;
        m_portCheckTimer = nullptr;
    }
}

bool RadioChannel::setConfig(const ChannelConfigPtr& config)
{
    emit setConfigRequested(config);//触发配置信号(由setConfigImp处理)
    return true;//此处返回值仅表示成功触发，实际结果通过setConfigFinished获取
}

ChannelConfigPtr RadioChannel::config() const {
    return m_config;
}

bool RadioChannel::start()
{
    emit startRequested();//触发启动信号(由startImpl处理)
    return true;
    
}

void RadioChannel::stop()
{
    emit stopRequested();//触发停止信号(由stopImpl处理)
    
}

bool RadioChannel::isRunning() const {
    return m_running;
}

QString RadioChannel::lastError() const
{
    return m_lastError;
}


QString RadioChannel::statusInfo() const {
    return m_running ? "已连接到串口: " + m_config->m_portName : "已停止";
}

void RadioChannel::onDataReady() {
    //1.读取硬件(串口)原始数据
    QByteArray rawData = m_serialPort->readAll();
    if (rawData.isEmpty())
        return;

    //2.拼接数据到缓冲区
    m_recvBuffer += rawData;
    qDebug() << "RadioChannel接收原始数据：" << rawData.toHex(' ');

    //3.解析缓冲区中的完整帧
    parseFrames();

}

void RadioChannel::onError(QSerialPort::SerialPortError error) {
    if (error != QSerialPort::NoError) {
        QString errorMsg = "串口错误: " + m_serialPort->errorString();
        qWarning() << "RadioChannel 串口异常：" << errorMsg;

        // 重点：捕获硬件移除错误（ResourceError是硬件意外断开的核心标识）
        if (error == QSerialPort::ResourceError) {
            errorMsg = "串口硬件已意外移除（端口：" + m_serialPort->portName() + "）";
            m_lastError = errorMsg;

            // 1. 强制标记通道为未运行状态（核心：同步m_running）
            if (m_running) {
                m_running = false;
            }

            // 2. 主动触发停止流程，清理串口资源
            stopImpl(); // 直接调用停止实现，无需通过信号（紧急场景同步执行）

            // 3. 额外标记串口无效，防止后续误操作
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

void RadioChannel::sendDataImpl(const QByteArray& data)
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
    qint64 bytesWritten = m_serialPort->write(data);
    qDebug() << "radio 发送数据（" << bytesWritten << "字节）：" << data.toHex(' ');
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
        qDebug() << "radio 错误:" << m_lastError;
        emit errorOccurred(m_lastError);
    }
    else {
        // 确保数据被立即发送（而非缓存）
        m_serialPort->flush();
    }
    emit sendDataFinished(success);
}

void RadioChannel::setConfigImpl(const ChannelConfigPtr& config)
{
    auto radioConfig = qSharedPointerDynamicCast<RadioConfig>(config);
    if (!radioConfig)
    {
        emit setConfigFinished(false);
        return;
    }
    m_config = radioConfig;
    emit setConfigFinished(true);//配置更新成功(生效需启动通道)
}

void RadioChannel::startImpl()
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
        // 启动硬件检测定时器
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

void RadioChannel::stopImpl()
{
    // 边界处理：已停止则直接返回
    if (!m_running) {
        qInfo() << "RadioChannel: 通道已停止，无需重复关闭";
        emit stopFinished(); // 通知上层关闭完成
        emit statusChanged("已停止");
        return;
    }
    // 停止硬件检测定时器
    m_portCheckTimer->stop();
    //区分主动停止和被动停止（硬件移除）
    qInfo() << "RadioChannel: 开始关闭串口通道（"
        << (m_lastError.contains("硬件移除") ? "被动：硬件移除" : "主动：用户操作")
        << "），端口：" << (m_config ? m_config->m_portName : "未知");

    // 标记为停止中，防止并发操作（如发送数据）
    m_running = false;

    // 安全关闭串口（处理硬件异常）
    bool closeSuccess = true;
    if (m_serialPort->isOpen()) {
        // 1. 清空串口缓冲区（clear返回bool，但仅执行，无需判断）
        m_serialPort->clear(QSerialPort::AllDirections);
        // 2. 调用close()（void类型，直接执行）
        m_serialPort->close();
        // 3. 检查是否真的关闭成功
        if (m_serialPort->isOpen()) { // 关键：通过isOpen()判断关闭结果
            closeSuccess = false;
            m_lastError = QString("串口关闭失败：%1（端口：%2）")
                .arg(m_serialPort->errorString())
                .arg(m_serialPort->portName());
            qWarning() << m_lastError;
            emit errorOccurred(m_lastError);
        }
        else {
            qInfo() << "RadioChannel: 串口关闭成功，端口：" << m_serialPort->portName();
        }
    }

    // 重置通道状态
    resetChannelState();

    // 步骤4：通知上层关闭结果（扩展 stopFinished 为带成功状态，可选）
    emit statusChanged(closeSuccess ? "已停止" : "关闭失败");
    emit stopFinished(); // 基类信号（若需精准反馈，可修改基类为 stopFinished(bool)）

    //超时保护（防止串口卡死）
    if (!closeSuccess) {
        QTimer::singleShot(500, this, [this]() {
            if (m_serialPort->isOpen()) {
                m_serialPort->close(); // 强制关闭
                qWarning() << "RadioChannel: 强制关闭串口";
            }
            });
    }
}

void RadioChannel::checkPortAvailability()
{
    // 仅在通道运行时检测
    if (!m_running || !m_serialPort || !m_serialPort->isOpen()) {
        return;
    }

    // 检测方式1：检查串口错误状态
    if (m_serialPort->error() != QSerialPort::NoError) {
        QString errorMsg = "串口异常（定时检测）：" + m_serialPort->errorString();
        m_lastError = errorMsg;
        emit errorOccurred(errorMsg);
        // 触发停止流程
        m_running = false;
        m_portCheckTimer->stop();
        stopImpl();
        return;
    }

    // 检测方式2：（Windows/Linux通用）验证端口是否仍存在
    QSerialPortInfo portInfo(*m_serialPort);
    if (!portInfo.isValid()) { // 端口无效（已移除）
        m_lastError = "串口硬件已移除（定时检测确认）：" + m_serialPort->portName();
        emit errorOccurred(m_lastError);
        // 触发停止流程
        m_running = false;
        m_portCheckTimer->stop();
        stopImpl();
        emit statusChanged("已断开（硬件移除）");
        return;
    }
}

void RadioChannel::parseFrames()
{
    while (true) 
    {
        // 3.1 查找帧头（固定为"@@@"）
        int headIdx = m_recvBuffer.indexOf(FrameConstants::FRAME_HEADER);
        if (headIdx == -1)
        {
            // 无完整帧头：保留可能组成帧头的尾部数据（如最后2字节）
            if (m_recvBuffer.size() > 2)
            {
                m_recvBuffer = m_recvBuffer.right(2);
            }
            break;
        }

        // 3.2 移除帧头前的无效数据
        if (headIdx > 0) 
        {
            m_recvBuffer.remove(0, headIdx);
            headIdx = 0;
        }

        // 3.3 检查帧尾是否存在（至少保留帧头+最小帧长）
        if (m_recvBuffer.size() < FrameConstants::FRAME_HEADER_LENGTH + 
            FrameConstants::FRAME_FOOTER_LENGTH) 
        {
            break; // 数据不足，等待后续
        }

        // 3.4 查找帧尾（固定为"$$$"）
        int footIdx = m_recvBuffer.indexOf(FrameConstants::FRAME_FOOTER, 
            headIdx);
        if (footIdx == -1) 
        {
            break; // 未找到帧尾，等待后续数据
        }

        // 3.5 提取完整帧（从帧头到帧尾）
        QByteArray frameData = m_recvBuffer.mid(headIdx, footIdx +
            FrameConstants::FRAME_FOOTER_LENGTH - headIdx);

        // 3.6 调用FrameParser解析帧（含时间戳）
        Frame frame;
        CheckError error;
        if (FrameParser::parseFrame(frameData, frame, error)) {
            qDebug() << "解析成功，时间戳：" << frame.timestamp.
                toString("yyMMddhhmmss");
            // 将frame 转为Datamessage
            DataMessage msg(
                MessageType::Status, // 消息类型：响应
                CommunicationChannel::Radio,               // 接收通道
                QJsonDocument(frame.toJson()).toJson(), // Frame转换为JSON（含时间戳）
                ModuleType::SelfCheck
            );
            // 用Frame自带的timestamp覆盖消息默认时间戳（确保与硬件时间一致）
            msg.setTimestamp(frame.timestamp);
            msg.setCommandType(CommandType::External);
            emit dataReceived(msg);
        }
        else
        {
            qWarning() << "帧解析失败：" << error.description();
        }

        // 3.7 移除已处理的帧
        m_recvBuffer.remove(0, footIdx + FrameConstants::FRAME_FOOTER_LENGTH);
    }
}

void RadioChannel::resetChannelState()
{
    m_recvBuffer.clear(); // 清空接收缓冲区（避免下次启动解析旧数据）
    m_lastError.clear();  // 清空错误信息
    m_running = false;    // 标记为未运行
}
