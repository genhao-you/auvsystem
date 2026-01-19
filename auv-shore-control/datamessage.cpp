#include "datamessage.h"

//DataMessage::DataMessage()
//	: m_channel(CommunicationChannel::WiredNetwork), m_data(), m_timestamp(QDateTime::currentDateTime())
//{
//}
DataMessage::DataMessage(MessageType type, CommunicationChannel channel, const QByteArray& data, ModuleType sender)
	:m_messageType(type), m_channel(channel), m_data(data), m_timestamp(QDateTime::currentDateTime()),m_sender(sender)
{

}

CommunicationChannel DataMessage::channel() const
{
	return m_channel;
}
QByteArray  DataMessage::data() const
{
	return m_data;
}
QDateTime  DataMessage::timestamp() const
{
	return m_timestamp;
}

void DataMessage::setTimestamp(QDateTime time)
{
	m_timestamp = time;
}

MessageType DataMessage::type() const
{
	return m_messageType;
}

void DataMessage::setMessageType(MessageType type)
{
	m_messageType = type;
}

void DataMessage::setCommandType(CommandType cmdType)
{
	m_cmdType = cmdType;
}

CommandType DataMessage::commandType() const
{
	return m_cmdType;
}

// 设置和获取消息ID
void DataMessage::setMessageId(int id) {
	m_messageId = id;
}
int DataMessage::messageId() const {
	return m_messageId;
}

// 设置和获取消息优先级
void DataMessage::setPriority(int priority) {
	m_priority = priority;
}
int DataMessage::priority() const {
	return m_priority;
}

// 设置和获取消息来源地址
void DataMessage::setSourceAddress(const QString& address)
{
	m_sourceAddress = address;
}
QString DataMessage::sourceAddress() const
{
	return m_sourceAddress;
}

DataMessage::DataMessage(const DataMessage& other)
	:m_messageType(other.m_messageType),
	m_channel(other.m_channel),
	m_data(other.m_data),
	m_timestamp(other.m_timestamp),
	m_messageId(other.m_messageId),
	m_priority(other.m_priority),
	m_sourceAddress(other.m_sourceAddress),
	m_sender(other.m_sender)
{

}

DataMessage& DataMessage::operator=(const DataMessage& other)
{
	// TODO: 在此处插入 return 语句
	if (this != &other)
	{
		m_messageType = other.m_messageType;
		m_channel = other.m_channel;
		m_data = other.m_data;
		m_timestamp = other.m_timestamp;
		m_messageId = other.m_messageId;
		m_priority = other.m_priority;
		m_sourceAddress = other.m_sourceAddress;
		m_sender = other.m_sender;
	}
	return *this;
}

void DataMessage::setSender(ModuleType sender)
{
	m_sender = sender;
}

ModuleType DataMessage::sender()const
{
	return m_sender;
}

bool DataMessage::isNull() const
{
	return m_data.isEmpty();
	
}
