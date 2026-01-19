#pragma once
#include "channelconfig.h"
#include <QSerialPort>
class WaterAcousticConfig : public ChannelConfig
{
public:
    QString name() const override;
    QJsonObject toJson() const override;
    void fromJson(const QJsonObject& json) override;

    // 串口参数（与RadioConfig完全一致）
    QString m_portName;
    int m_baudRate = QSerialPort::Baud115200;
    QSerialPort::DataBits m_dataBits = QSerialPort::Data8;
    QSerialPort::Parity m_parity = QSerialPort::NoParity;
    QSerialPort::StopBits m_stopBits = QSerialPort::OneStop;
    QSerialPort::FlowControl m_flowControl = QSerialPort::NoFlowControl;
};
// 智能指针别名
using WaterAcousticConfigPtr = QSharedPointer<WaterAcousticConfig>;