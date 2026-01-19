#include "wirelessnetworkconfig.h"

QString WirelessNetworkConfig::name() const {
    return "WirelessNetwork";
}

QJsonObject WirelessNetworkConfig::toJson() const {
    QJsonObject json;
    json["host"] = m_host;
    json["port"] = m_port;
    json["timeout"] = m_timeout;
    return json;
}

void WirelessNetworkConfig::fromJson(const QJsonObject& json) {
    m_host = json["host"].toString();
    m_port = json["port"].toInt();
    m_timeout = json["timeout"].toInt();
}