#pragma once

#include <QObject>
#include"communicationchannel.h"
#include"channelconfig.h"
#include"datamessage.h"
#include<memory>
class CommunicationChannelBase  : public QObject
{
	Q_OBJECT

public:
    explicit CommunicationChannelBase(CommunicationChannel type, QObject* parent = nullptr);
    virtual ~CommunicationChannelBase();

    // 设置通道配置
    virtual bool setConfig(const ChannelConfigPtr& config) = 0;

    // 获取通道配置
    virtual ChannelConfigPtr config() const = 0;

    // 启动通信通道
    virtual bool start() = 0;

    // 停止通信通道
    virtual void stop() = 0;

    // 检查通道是否运行
    virtual bool isRunning() const = 0;

    //获取最后一次错误信息
    virtual QString lastError()const = 0;
    // 发送数据
   // virtual bool sendData(const QByteArray& data) = 0;

    // 获取通道类型
    CommunicationChannel type() const;

    // 获取通道状态信息
    virtual QString statusInfo() const = 0;

signals:
    // 数据接收信号(通道所属线程发射，外部线程接收)
    void dataReceived(const DataMessage& message);

    // 通道状态变化信号
    void statusChanged(const QString& status);

    // 错误发生信号
    void errorOccurred(const QString& error);

    //---------------------------
    //跨线程操作触发信号(外部线程发射，通道所属线程接收)
    //------------------------
    //触发发送数据
    void sendDataRequested(const QByteArray& data);
    //触发配置更新
    void setConfigRequested(const ChannelConfigPtr config);
  

    //明确“请求启动”，区分触发/结果
    void startRequested();
    void stopRequested();

    //---------------------------
    // 操作结果反馈信号(通道所属线程发射，外部线程接收)
    // ---------------------------
    //发送完成通知
    void sendDataFinished(bool success);//发送结果
  
    void setConfigFinished(bool success);
    void startFinished(bool success);
    void stopFinished();
   
protected:
    CommunicationChannel m_type;
    bool m_running = false;
    QString m_lastError;
};
;
