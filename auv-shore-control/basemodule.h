#pragma once

#include <QObject>
#include<QByteArray>
#include<QJsonObject>
#include"moduleenum.h"
#include"communicationchannel.h"
#include"dispatchstrategies.h"
#include"datamessage.h"
#include"basemodule.h"
#include "observer.h"
class BaseModule  : public QObject
{
	Q_OBJECT

public:
	explicit BaseModule(ModuleType type,const QString& moduleName, QObject *parent = nullptr);
	virtual ~BaseModule();
	//模块初始化与反初始化
	virtual bool initialize();
	virtual void shutdown();
	//发送消息
	void sendCommand(const QString& targetModule, CommunicationChannel channelType,const QString& command, const QJsonObject& params = QJsonObject());
	void publishStatus(CommunicationChannel channelType,const QString& topic, const QJsonObject& data);
	void publishEvent(CommunicationChannel channelType,const QString& eventType, const QJsonObject& data);
	void transferData(CommunicationChannel channel, const QString& target, const QByteArray& data);

	//注册命令处理器
	void registerCommandHandler(const QString& command, const MessageHandler& handler);

	//订阅状态更新
	void subscribeStatus(const QString& topic, const MessageHandler& handler);

	//订阅事件
	void subscribeEvent(const QString& eventType, const MessageHandler& handler);

	//注册数据接收者
	void registerDataReceiver(const QString& target, const MessageHandler& handler);

	

	//获取模块信息
	ModuleType type()const;
	QString name() const;

	/**
   * @brief 添加结果观察者
   * @param observer 观察者对象
   */
	void addObserver(IResultObserver* observer);

	/**
	 * @brief 移除结果观察者
	 * @param observer 观察者对象
	 */
	void removeObserver(IResultObserver* observer);

	/**
	 * @brief 通知所有观察者结果更新
	 * @param result 自检结果
	 */
	void notifyObservers(const BaseResult* result);
protected:
	//处理接收到的数据，由子类实现
	//virtual void handleData(CommunicationChannel channelType, const QByteArray& data) = 0;
	//virtual void handleMessage(const DataMessage& message) = 0;
	//获取默认通信方式（子类可重写）
	virtual CommunicationChannel defaultCommunicationChannel()const;
private slots:
	//void onChannelDataReceived(const DataMessage& message) ;
private:
	ModuleType m_type;
	QString m_sName;
	QList<IResultObserver*>m_observers;//观察者列表(使用QPointer避免悬空指针)
};
// 全局唯一ID生成器（确保并发安全）
class CommandIdGenerator {
public:
	static CommandIdGenerator& instance() {
		static CommandIdGenerator gen;
		return gen;
	}
	uint64_t nextId() {
		QMutexLocker locker(&m_mutex);
		return m_nextId++;
	}
private:
	CommandIdGenerator() : m_nextId(1) {}
	QMutex m_mutex;
	uint64_t m_nextId;
};
