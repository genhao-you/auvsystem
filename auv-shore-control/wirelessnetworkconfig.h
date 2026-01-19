#pragma once
#include"channelconfig.h"
class WirelessNetworkConfig  : public ChannelConfig
{
public:
    QString name() const override;
    QJsonObject toJson() const override;
    void fromJson(const QJsonObject& json) override;

    QString m_host;
    int m_port;
    int m_timeout;

};
