#pragma once
#include"channelconfig.h"
// 有线网络配置
class WiredNetworkConfig : public ChannelConfig
{
public:
    QString name() const override;
    QJsonObject toJson() const override;
    void fromJson(const QJsonObject& json) override;

    QString m_host;
    int m_port;
    int m_timeout;

    // ---------------------- FTP专属配置字段 ----------------------
    QString m_ftpUsername;  // FTP登录用户名（匿名登录留空）
    QString m_ftpPassword;  // FTP登录密码（匿名登录留空）
    bool m_ftpPassiveMode;  // FTP被动模式开关（默认开启，适配多数服务器）
};
