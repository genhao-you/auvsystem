#include "dispatchstrategies.h"
#include<QJsonDocument>
#include<QJsonObject>
CommandRoutingStrategy& CommandRoutingStrategy::instance()
{
	static CommandRoutingStrategy instance;
	return instance;
}
void CommandRoutingStrategy::registerCommandHandler(const QString& moduleName, const QString& command, const MessageHandler& handler)
{
	m_routeTable[moduleName][command].append(handler);
}

void CommandRoutingStrategy::handleMessage(const DataMessage& message)
{
	//解析消息中的目标模块和命令
	QJsonDocument doc = QJsonDocument::fromJson(message.data());
	QJsonObject json = doc.object();

	QString targetModule = json["target"].toString();
	QString command = json["commandType"].toString();

	if(m_routeTable.contains(targetModule)&& m_routeTable[targetModule].contains(command))
	{
		foreach  (const auto &handler,m_routeTable[targetModule][command])
		{
			handler(message);
		}
	}
}

PublishSubscribeStrategy& PublishSubscribeStrategy::instance()
{
	static PublishSubscribeStrategy instance;
	return instance;
}
//发布-订阅策略实现
void PublishSubscribeStrategy::subscribe(const QString& topic, const MessageHandler& handler)
{
	m_subscribers[topic].append(handler);
}

void PublishSubscribeStrategy::handlerMessage(const DataMessage& message)
{
	//解析消息中的主题
	QJsonDocument doc = QJsonDocument::fromJson(message.data());
	QJsonObject json = doc.object();

	QString topic = json["topic"].toString();

	if (m_subscribers.contains(topic))
	{
		foreach (const auto& subscriber,m_subscribers[topic])
		{
			subscriber(message);
		}
	}
}
BroadcastStrategy& BroadcastStrategy::instance()
{
	static BroadcastStrategy instance;
	return instance;
}
//广播策略实现
void BroadcastStrategy::addListener(const MessageHandler& listener)
{
	m_listeners.append(listener);
}

void BroadcastStrategy::handleMessage(const DataMessage& message)
{
	//广播给所有监听器
	foreach(const auto & listener, m_listeners)
	{
		listener(message);
	}
}
DirectTransferStrategy& DirectTransferStrategy::instance()
{
	static DirectTransferStrategy instance;
	return instance;
}

void DirectTransferStrategy::registerReceiver(const QString& target, const MessageHandler& handler)
{
	m_receivers[target].append(handler);
}

void DirectTransferStrategy::handleMessage(const DataMessage& message)
{
	//解析消息中的主题
	QJsonDocument doc = QJsonDocument::fromJson(message.data());
	QJsonObject json = doc.object();

	QString target = json["target"].toString();

	if (m_receivers.contains(target))
	{
		foreach(const auto & receiver, m_receivers[target])
		{
			receiver(message);
		}
	}
}