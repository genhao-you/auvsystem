#include "wirednetworkconfig.h"

QString WiredNetworkConfig::name() const {
    return "WiredNetwork";
}

QJsonObject WiredNetworkConfig::toJson() const {
    QJsonObject json;
    json["host"] = m_host;
    json["port"] = m_port;
    json["timeout"] = m_timeout;
    // FTP配置字段（适配FTP协议）
    json["ftpUsername"] = m_ftpUsername;    // FTP用户名
    json["ftpPassword"] = m_ftpPassword;    // FTP密码
    json["ftpPassiveMode"] = m_ftpPassiveMode; // FTP被动模式
    return json;
}

void WiredNetworkConfig::fromJson(const QJsonObject& json) {
    m_host = json["host"].toString();
    m_port = json["port"].toInt();
    m_timeout = json["timeout"].toInt();
    // FTP配置字段（设置默认值，避免未配置时异常）
    m_ftpUsername = json["ftpUsername"].toString(); // 默认空（匿名登录）
    m_ftpPassword = json["ftpPassword"].toString(); // 默认空（匿名登录）
    m_ftpPassiveMode = json["ftpPassiveMode"].toBool(true); // 默认开启被动模式
}
