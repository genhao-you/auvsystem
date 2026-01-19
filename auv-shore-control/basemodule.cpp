#include "basemodule.h"
#include<QJsonDocument>
#include<QVariant>



BaseModule::BaseModule(ModuleType type, const QString& moduleName, QObject* parent)
	: QObject(parent),
	m_type(type),
	m_sName(moduleName)

{
	//注册策略到分发器
	MessageDispatcher::instance().registerStrategy(
		MessageDispatcher::CommandRouting,
		[&](const DataMessage& msg)
		{CommandRoutingStrategy::instance().handleMessage(msg); }
	);
	MessageDispatcher::instance().registerStrategy(
		MessageDispatcher::PublishSubscribe,
		[&](const DataMessage& msg)
		{PublishSubscribeStrategy::instance().handlerMessage(msg); }
	);
	MessageDispatcher::instance().registerStrategy(
		MessageDispatcher::Broadcast,
		[&](const DataMessage& msg)
		{BroadcastStrategy::instance().handleMessage(msg);
		}
	);
	MessageDispatcher::instance().registerStrategy(
		MessageDispatcher::DirectTransfer,
		[&](const DataMessage& msg)
		{
			DirectTransferStrategy::instance().handleMessage(msg); }
	);
}

BaseModule::~BaseModule()
{
	shutdown();
}

bool BaseModule::initialize()
{
	return true;
}

void BaseModule::shutdown()
{
	m_observers.clear();
}

void BaseModule::sendCommand(const QString& targetModule, CommunicationChannel channelType,const QString& command, const QJsonObject& params)
{
	QJsonObject json;
	json["target"] = targetModule;
	json["commandType"] = command;
	json["params"] = params;
	DataMessage message(MessageType::Command, channelType, QJsonDocument(json).toJson(), m_type);
	MessageDispatcher::instance().send(message);
}
void BaseModule::publishStatus(CommunicationChannel channel, const QString& topic, const QJsonObject& data)
{
	QJsonObject json;
	json["topic"] = topic;
	json["data"] = data;
	DataMessage message(
		MessageType::Status,
		channel,
		QJsonDocument(json).toJson(),
		m_type
	);
	MessageDispatcher::instance().send(message);
}

void BaseModule::publishEvent(CommunicationChannel channel, const QString& eventType, const QJsonObject& data)
{
	QJsonObject json;
	json["eventType"] = eventType;
	json["data"] = data;
	DataMessage message(
		MessageType::Event,
		channel,
		QJsonDocument(json).toJson(),
		m_type
	);
	MessageDispatcher::instance().send(message);
}

void BaseModule::transferData(CommunicationChannel channel, const QString& target, const QByteArray& data)
{
	QJsonObject json;
	json["target"] = target;
	json["data"] = QJsonValue::fromVariant(QVariant::fromValue(data.toBase64()));
	DataMessage message(
		MessageType::DataTransfer,
		channel,
		QJsonDocument(json).toJson(),
		m_type
	);
	MessageDispatcher::instance().send(message);
}



void BaseModule::registerCommandHandler(const QString& command, const MessageHandler& handler)
{
	CommandRoutingStrategy::instance().registerCommandHandler(name(), command, handler);
}

void BaseModule::subscribeStatus(const QString& topic, const MessageHandler& handler)
{
	PublishSubscribeStrategy::instance().subscribe(topic, handler);
}

void BaseModule::subscribeEvent(const QString& eventType, const MessageHandler& handler)
{
	BroadcastStrategy::instance().addListener([eventType, handler](const DataMessage& message)
		{
			QJsonDocument doc = QJsonDocument::fromJson(message.data());
			QJsonObject json = doc.object();

			if (json["eventType"].toString() == eventType)
			{
				handler(message);
			}
		});
}

void BaseModule::registerDataReceiver(const QString& target, const MessageHandler& handler)
{
	DirectTransferStrategy::instance().registerReceiver(target, handler);
}

//void BaseModule::registerCommunicationChannel(CommunicationChannel channelType)
//{
//	if (!m_registeredChannels.contains(channelType))
//	{
//		//创建回调函数
//		SubscriberCallback callback = [this](const DataMessage& message)
//		{
//			this->handleMessage(message);
//		};
//
//		//订阅通道
//		PubSubCenter::instance().subscribe(channelType, callback);
//		//保存回调用于后续取消订阅
//		m_callbacks.append(callback);
//		m_registeredChannels.append(channelType);
//	}
//}

//void BaseModule::unregisterCommunicationChannel(CommunicationChannel channelType)
//{
//	if (m_registeredChannels.contains(channelType))
//	{
//		//找到对应的回调并取消订阅
//		for (const auto& callback : m_callbacks)
//		{
//			PubSubCenter::instance().unsubscribe(channelType, callback);
//		}
//		m_registeredChannels.removeOne(channelType);
//	}
//}

ModuleType BaseModule::type() const
{
	return m_type;
}

QString BaseModule::name() const
{
	return m_sName;
}

void BaseModule::addObserver(IResultObserver* observer)
{
	
	if (observer && !m_observers.contains(observer))
	{
		m_observers.append(observer);
	}
	
}

void BaseModule::removeObserver(IResultObserver* observer)
{
	m_observers.removeOne(observer);
}

void BaseModule::notifyObservers(const BaseResult* result)
{
	if (!result)
	{
		return;
	}
	// 通知所有观察者
	for (auto& observer : m_observers) {
		if (observer) {
			observer->onResultUpdated(result);
		}
	}
}

CommunicationChannel BaseModule::defaultCommunicationChannel() const
{
	return CommunicationChannel::Radio;
}
//void BaseModule::sendMessage(CommunicationChannel channel, const QByteArray& data)
//{
//	DataMessage message(channel, data, m_type);
//	PubSubCenter::instance().publish(message);
//}
//void BaseModule::onChannelDataReceived(const DataMessage& message)
//{
//	//检查消息通道是否是模块注册的通道
//	if (m_registeredChannels.contains(message.channel()))
//	{
//		//调用子类实现的消息处理函数
//		handleMessage(message);
//	}
//}