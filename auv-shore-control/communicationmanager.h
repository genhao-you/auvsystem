#pragma once

#include <QObject>
#include"communicationchannel.h"
#include"communicationchannelbase.h"
#include"timestampprocessor.h"
#include"datasourceselector.h"
#include"channelconfig.h"
#include"radiochannel.h"
#include"BDSchannel.h"
#include"wirednetworkchannel.h"
#include"wirelessnetworkchannel.h"
#include"wateracousticchannel.h"
#include"basemodule.h"
#include"ftpsupportinterface.h"
#include<QMutex>
#include<QMap>
#include<QList>
#include<memory>
#include<QThread>
#include"frame.h"
class CommunicationManager  : public QObject 
{
	Q_OBJECT
    Q_DISABLE_COPY(CommunicationManager)//禁止拷贝
public:
    static CommunicationManager& instance();
	CommunicationManager(QObject *parent = nullptr);
	~CommunicationManager() override;

    // 启动所有通信通道
    void startAllChannels();

    // 停止所有通信通道
    void stopAllChannels();

    bool startChannel(CommunicationChannel channel);
    bool stopChannel(CommunicationChannel channel);

    //判断通道是否在运行
    bool isChannelRunning(CommunicationChannel channel)const;
    // 物理层发送数据（供CommunicationModule调用）
    bool sendData(CommunicationChannel channel, const QByteArray& data);

    //辅助函数：获取通道实例
    CommunicationChannelBase* getChannelInstance(CommunicationChannel channel)const;

    // 获取通道状态
    QString channelStatus(CommunicationChannel channel) const;

    // 设置通道配置(如波特率、频率)
    bool setChannelConfig(CommunicationChannel channel, const ChannelConfigPtr& config);


    // FTP 操作接口（通过动态判断通道是否支持）
    bool sendFtpFile(CommunicationChannel channel, const QVariantMap& params);
    bool downloadFtpFile(CommunicationChannel channel, const QVariantMap& params);
    bool listFtpFiles(CommunicationChannel channel, const QVariantMap& params);
    // 删除远程文件/目录
    bool deleteFtpItem(CommunicationChannel channel, const QVariantMap& params);

    template<typename T>
    T* getTypedChannelInstance(CommunicationChannel channel) const
    {
        CommunicationChannelBase* base = getChannelInstance(channel);
        if (!base) return nullptr;

        return qobject_cast<T*>(base);
    }
signals:
    // 通道状态变化信号
    void channelStatusChanged(CommunicationChannel channel, const QString& status);

    // 通道错误信号
    void channelError(CommunicationChannel channel, const QString& error);  

    //硬件数据接收信号
    void responseReceived(const DataMessage& message);

    // FTP 操作结果转发信号
    void ftpFileSent(CommunicationChannel channel, bool success);
    void ftpFileDownloaded(CommunicationChannel channel, bool success);
    void ftpFileListFetched(CommunicationChannel channel, bool success, 
        const QStringList& files, const QString& msg);

    // 上传进度（通道类型 + 进度值）
    void ftpUploadProgress(CommunicationChannel channel, int progress);
    // 下载进度（通道类型 + 进度值）
    void ftpDownloadProgress(CommunicationChannel channel, int progress);

private:
    // 初始化各通信通道
    void initChannels();

    //检查通道是否有效
    bool isChannelValid(CommunicationChannel channel) const;

    // 辅助方法：获取并验证FTP通道
    WiredNetworkChannel* getValidFtpChannel(CommunicationChannel channel);
private:
    mutable QMutex m_mutex;
    QList<CommunicationChannelBase*> m_channels;
    QMap<CommunicationChannel, CommunicationChannelBase*> m_channelMap;
    QMap<CommunicationChannel, bool>m_channelConfigured;//记录通道是否被配置
    QList<QThread*> m_threads;
    TimestampProcessor* m_timestampProcessor;
    DataSourceSelector* m_dataSelector;
};
