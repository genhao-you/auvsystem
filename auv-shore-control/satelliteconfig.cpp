#include "satelliteconfig.h"

QString SatelliteConfig::name() const {
    return "Satellite";
}

QJsonObject SatelliteConfig::toJson() const {
    QJsonObject json;
    json["apiEndpoint"] = m_apiEndpoint;
    json["apiKey"] = m_apiKey;
    json["timeout"] = m_timeout;
    return json;
}

void SatelliteConfig::fromJson(const QJsonObject& json) {
    m_apiEndpoint = json["apiEndpoint"].toString();
    m_apiKey = json["apiKey"].toString();
    m_timeout = json["timeout"].toInt();
}