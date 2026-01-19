#pragma once
#include"channelconfig.h"
#include<QSerialPort>

class BDSConfig : public ChannelConfig
{
public:
    QString name() const override;
    QJsonObject toJson() const override;
    void fromJson(const QJsonObject& json) override;

    QString m_portName;
    int m_baudRate;
    QSerialPort::DataBits m_dataBits;
    QSerialPort::Parity m_parity;
    QSerialPort::StopBits m_stopBits;
    QSerialPort::FlowControl m_flowControl;
};
