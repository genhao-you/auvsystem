#include "wateracousticconfig.h"

QString WaterAcousticConfig::name() const {
    return "WaterAcoustic"; // 配置名称，与Radio的"Radio"对应
}

QJsonObject WaterAcousticConfig::toJson() const {
    QJsonObject json;
    json["portName"] = m_portName;
    json["baudRate"] = m_baudRate;
    json["dataBits"] = m_dataBits;
    json["parity"] = m_parity;
    json["stopBits"] = m_stopBits;
    json["flowControl"] = m_flowControl;
    return json;
}

void WaterAcousticConfig::fromJson(const QJsonObject& json) {
    m_portName = json["portName"].toString();
    m_baudRate = json["baudRate"].toInt(QSerialPort::Baud115200);
    m_dataBits = static_cast<QSerialPort::DataBits>(json["dataBits"].toInt(QSerialPort::Data8));
    m_parity = static_cast<QSerialPort::Parity>(json["parity"].toInt(QSerialPort::NoParity));
    m_stopBits = static_cast<QSerialPort::StopBits>(json["stopBits"].toInt(QSerialPort::OneStop));
    m_flowControl = static_cast<QSerialPort::FlowControl>(json["flowControl"].toInt(QSerialPort::NoFlowControl));
}