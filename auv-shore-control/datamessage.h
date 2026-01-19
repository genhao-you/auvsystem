#pragma once
#include"communicationchannel.h"
#include<QByteArray>
#include<QDateTime>
#include"ModuleEnum.h"
#include"messagetype.h"
#include"selfcheckenum.h"
class DataMessage 
{
  
public:
    DataMessage() = default;
    DataMessage(MessageType type,CommunicationChannel channel, const QByteArray& data,ModuleType sender = ModuleType::Unknown);

    CommunicationChannel channel() const;
    QByteArray data()const;

    QDateTime timestamp()const;
    void setTimestamp(QDateTime time);

    MessageType type()const;
    void setMessageType(MessageType type);

    void setCommandType(CommandType cmdType);
    CommandType commandType() const;

    // 设置和获取消息ID
    void setMessageId(int id);
    int messageId() const;

    // 设置和获取消息优先级
    void setPriority(int priority);
    int priority() const;

    // 设置和获取消息来源地址
    void setSourceAddress(const QString& address);
    QString sourceAddress() const;

    //添加拷贝构造函数和赋值运算符
    DataMessage(const DataMessage& other);
    DataMessage& operator=(const DataMessage& other);

    //设置发送者(用于系统内部消息)
    void setSender(ModuleType sender);
    ModuleType sender() const;

    // 判断消息是否为空（无效消息）
    bool isNull() const;
private:
    MessageType m_messageType;
    CommunicationChannel m_channel;  // 数据来源通道
    QByteArray m_data;               // 数据内容
    QDateTime m_timestamp;           // 接收时间戳
    int m_messageId = 0;             // 消息ID
    int m_priority = 0;              // 消息优先级
    QString m_sourceAddress;         // 消息来源地址
    ModuleType m_sender;
    CommandType m_cmdType = CommandType::External;
};
