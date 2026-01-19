#pragma once
#include"channelconfig.h"
class SatelliteConfig : public ChannelConfig
{
public:
    QString name() const override;
    QJsonObject toJson() const override;
    void fromJson(const QJsonObject& json) override;

    QString m_apiEndpoint;
    QString m_apiKey;
    int m_timeout;
};
