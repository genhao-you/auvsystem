#include "radioconfig.h"

QString RadioConfig::name() const {
    return "Radio";
}

QJsonObject RadioConfig::toJson() const {
    QJsonObject json;
    json["portName"] = m_portName;
    json["baudRate"] = m_baudRate;
    json["dataBits"] = m_dataBits;
    json["parity"] = m_parity;
    json["stopBits"] = m_stopBits;
    json["flowControl"] = m_flowControl;
    return json;
}

void RadioConfig::fromJson(const QJsonObject& json) {
    m_portName = json["portName"].toString();
    m_baudRate = json["baudRate"].toInt();
    m_dataBits = static_cast<QSerialPort::DataBits>(json["dataBits"].toInt());
    m_parity = static_cast<QSerialPort::Parity>(json["parity"].toInt());
    m_stopBits = static_cast<QSerialPort::StopBits>(json["stopBits"].toInt());
    m_flowControl = static_cast<QSerialPort::FlowControl>(json["flowControl"].toInt());
}