#pragma once
#include"messagedispatcher.h"
#include<QMap>

//命令路由策略实现
class CommandRoutingStrategy
{
public:
	static CommandRoutingStrategy& instance();
	//注册模块命令处理器
	void registerCommandHandler(const QString& moduleName, const QString& command, const MessageHandler& handler);

	//处理命令消息
	void handleMessage(const DataMessage& message);

private:
	CommandRoutingStrategy() = default;
	~CommandRoutingStrategy() = default;
	//命令路由表：模块名->(命令->处理函数)
	QMap<QString, QMap<QString, QList<MessageHandler>>>m_routeTable;
};

//发布-订阅策略实现
class PublishSubscribeStrategy
{
public:
	static PublishSubscribeStrategy& instance();
	//订阅主题
	void subscribe(const QString& topic, const MessageHandler& handler);

	//处理发布的消息
	void handlerMessage(const DataMessage& messsage);

private:
	PublishSubscribeStrategy() = default;
	~PublishSubscribeStrategy()=default;
	//主题订阅表：主题+订阅者
	QMap<QString, QList<MessageHandler>>m_subscribers;
};

//广播策略实现
class BroadcastStrategy
{
public:
	static BroadcastStrategy& instance();
	//添加全局监听器
	void addListener(const MessageHandler& listener);
	
	//处理广播消息
	void handleMessage(const DataMessage& message);
private:
	BroadcastStrategy() = default;
	~BroadcastStrategy() = default;
	QList<MessageHandler> m_listeners;
};

//直接传输策略
class DirectTransferStrategy
{
public :
	static DirectTransferStrategy& instance();
	//注册数据接收者
	void registerReceiver(const QString& target, const MessageHandler& handler);
	//处理数据传输
	void handleMessage(const DataMessage& message);
private:
	DirectTransferStrategy() = default;
	~DirectTransferStrategy() = default;

	//数据接收者：目标->处理函数
	QMap<QString, QList<MessageHandler>>m_receivers;
};