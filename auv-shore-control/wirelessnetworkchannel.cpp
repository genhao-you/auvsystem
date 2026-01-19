#include "wirelessnetworkchannel.h"

// 无线网络通信通道实现
WirelessNetworkChannel::WirelessNetworkChannel(QObject* parent)
    : CommunicationChannelBase(CommunicationChannel::WirelessNetwork, parent) {
    //m_config = std::make_shared<WirelessNetworkConfig>();
    m_socket = new QUdpSocket(this);

    connect(m_socket, &QUdpSocket::readyRead, this, &WirelessNetworkChannel::onDataReady);
}

WirelessNetworkChannel::~WirelessNetworkChannel() {
    stop();
}

bool WirelessNetworkChannel::setConfig(const ChannelConfigPtr& config) {
   /* auto wirelessConfig = std::dynamic_pointer_cast<WirelessNetworkConfig>(config);
    if (wirelessConfig) {
        m_config = wirelessConfig;
        return true;
    }*/
    return false;
}

ChannelConfigPtr WirelessNetworkChannel::config() const {
    return m_config;
}

bool WirelessNetworkChannel::start() {
    if (m_running)
        return true;

    if (!m_config) {
        emit errorOccurred("配置未设置");
        return false;
    }

    if (!m_socket->bind(QHostAddress::Any, m_config->m_port)) {
        emit errorOccurred("无法绑定到端口: " + QString::number(m_config->m_port));
        return false;
    }

    m_running = true;
    emit statusChanged("已启动，监听端口: " + QString::number(m_config->m_port));
    return true;
}

void WirelessNetworkChannel::stop() {
    if (!m_running)
        return;

    m_running = false;
    m_socket->close();
    emit statusChanged("已停止");
}

bool WirelessNetworkChannel::isRunning() const {
    return m_running;
}

QString WirelessNetworkChannel::lastError() const
{
    return QString();
}

//bool WirelessNetworkChannel::sendData(const QByteArray& data) {
//    if (!m_running) {
//        emit errorOccurred("通道未运行，无法发送数据");
//        return false;
//    }
//
//    qint64 bytesWritten = m_socket->writeDatagram(data, QHostAddress(m_config->m_host), m_config->m_port);
//    return bytesWritten == data.size();
//}

QString WirelessNetworkChannel::statusInfo() const {
    return m_running ? "已启动，监听端口: " + QString::number(m_config->m_port) : "已停止";
}

void WirelessNetworkChannel::onDataReady() {
    while (m_socket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_socket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        m_socket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        DataMessage message(MessageType::Command,m_type, datagram);
        message.setSourceAddress(sender.toString() + ":" + QString::number(senderPort));
        emit dataReceived(message);
    }
}