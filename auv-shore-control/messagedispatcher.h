#pragma once

#include <QObject>
#include<QMap>
#include<QMutex>
#include"datamessage.h"

//消息处理函数类型
using MessageHandler = std::function<void(const DataMessage&)>;
class MessageDispatcher  : public QObject
{
	Q_OBJECT

public:

	explicit	MessageDispatcher(QObject* parent = nullptr);
	~MessageDispatcher();

	enum DispatchStrategy
	{
		PublishSubscribe,//发布-订阅策略（处理status类型消息）
		CommandRouting,//命令路由器策略（处理control类型消息）
		Broadcast,//广播策略（处理event类型消息）
		DirectTransfer//直接传输策略（处理DataTransfer类型i消息）
	};
	static MessageDispatcher& instance();

	//注册分发策略
	void registerStrategy(DispatchStrategy strategy, const MessageHandler& handler);

	//分发消息
	void dispatch(const  DataMessage& message);

	//订阅特定类型的消息
	void subscribe(MessageType type, const MessageHandler& handler);

	//发送消息
	void send(const DataMessage& message);

private:
	QMutex m_mutex;
	QMap<DispatchStrategy, MessageHandler>m_strategies;
	QMap<MessageType, QList<MessageHandler>>m_subscribers;
};
