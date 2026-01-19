#include "messagedispatcher.h"
#include<QCoreApplication>
MessageDispatcher::MessageDispatcher(QObject* parent)
	: QObject(parent)
{}

MessageDispatcher::~MessageDispatcher()
{}

MessageDispatcher& MessageDispatcher::instance()
{
	// TODO: 在此处插入 return 语句
	static MessageDispatcher instacne;
	return instacne;
}

void MessageDispatcher::registerStrategy(DispatchStrategy strategy, const MessageHandler& handler)
{
	QMutexLocker locker(&m_mutex);
	m_strategies[strategy] = handler;
}

void MessageDispatcher::dispatch(const DataMessage& message)
{
	QMutexLocker locker(&m_mutex);
	//根据消息类型选择分发策略
	DispatchStrategy strategy;

	switch (message.type())
	{
	case MessageType::Command:
			strategy = CommandRouting;
			break;
	case MessageType::Status:
		strategy = PublishSubscribe;
		break;
	case MessageType::Event:
		strategy = Broadcast;
		break;
	case MessageType::DataTransfer:
		strategy = DirectTransfer;
		break;
	default:
		strategy = PublishSubscribe;
		break;
	}
	//执行对应的分发策略
	if (m_strategies.contains(strategy))
	{
		// 使用异步调用避免阻塞
		auto handler = m_strategies[strategy];
		QMetaObject::invokeMethod(
			QCoreApplication::instance(),
			[handler, message]() { handler(message); },
			Qt::QueuedConnection
		);
	}
}

void MessageDispatcher::subscribe(MessageType type, const MessageHandler& handler)
{
	QMutexLocker locker(&m_mutex);
	m_subscribers[type].append(handler);

}

void MessageDispatcher::send(const DataMessage& message)
{
	//直接调用分发器
	dispatch(message);
}

