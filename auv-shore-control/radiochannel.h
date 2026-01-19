#pragma once

#include <QObject>
#include"communicationchannelbase.h"
#include"radioconfig.h"
class RadioChannel  : public CommunicationChannelBase
{
	Q_OBJECT

public:
    explicit RadioChannel(QObject* parent = nullptr);
    ~RadioChannel() override;

    bool setConfig(const ChannelConfigPtr& config) override;
    ChannelConfigPtr config() const override;

    bool start() override;
    void stop() override;
    bool isRunning() const override;
    QString lastError()const override;
    QString statusInfo() const override;

private slots:
    void onDataReady();
    void onError(QSerialPort::SerialPortError error);
    void sendDataImpl(const QByteArray& data);
    void setConfigImpl(const ChannelConfigPtr& config);
    void startImpl();
    void stopImpl();
    void checkPortAvailability(); // 串口可用性检测
private:
    void parseFrames();//解析缓冲区的完整帧
    void resetChannelState(); // 关闭后重置通道状态（清理缓冲区/错误信息）
private:
    QSharedPointer<RadioConfig> m_config;
    QSerialPort* m_serialPort;
    QString m_lastError;
    QByteArray m_recvBuffer;//接收缓冲区
    bool m_running = false; // 通道运行状态（核心标记）
    QTimer* m_portCheckTimer; // 串口可用性检测定时器
};
