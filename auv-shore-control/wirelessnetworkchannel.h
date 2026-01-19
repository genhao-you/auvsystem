#pragma once

#include <QObject>
#include"communicationchannelbase.h"
#include"wirelessnetworkconfig.h"
#include<QUdpSocket>
class WirelessNetworkChannel  : public CommunicationChannelBase
{
	Q_OBJECT

 public :
    explicit WirelessNetworkChannel(QObject* parent = nullptr);
    ~WirelessNetworkChannel() override;

    bool setConfig(const ChannelConfigPtr& config) override;
    ChannelConfigPtr config() const override;

    bool start() override;
    void stop() override;
    bool isRunning() const override;
    QString lastError()const override;
    //bool sendData(const QByteArray& data) override;
    QString statusInfo() const override;

private slots:
    void onDataReady();

private:
    QSharedPointer<WirelessNetworkConfig> m_config;
    QUdpSocket* m_socket;
    QString m_lastError;
};
