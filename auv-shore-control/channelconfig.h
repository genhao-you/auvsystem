#pragma once
#include<QString>
#include<QJsonObject>
#include<memory>
#include<QSharedPointer>
//前向声明
class ChannelConfig;
//定义智能指针类型别名
using ChannelConfigPtr = QSharedPointer<ChannelConfig>;
// 通信通道配置基类
class ChannelConfig 
{
public:
    virtual ~ChannelConfig() {}

    // 获取配置名称
    virtual QString name() const = 0;

    // 转换为JSON对象
    virtual QJsonObject toJson() const = 0;

    // 从JSON对象加载配置
    virtual void fromJson(const QJsonObject& json) = 0;
};
