#pragma once
#include "basemodule.h"
#include "communicationchannel.h"
#include"communicationmanager.h"
#include"selfcheckenum.h"
#include "framebuilder.h"
#include "frameparser.h"
#include"communicationchannelstatusresult.h"
#include <QMap>
class CommunicationModule : public BaseModule {
    Q_OBJECT

public:
    static CommunicationModule& instance();
    explicit CommunicationModule(QObject* parent = nullptr);
    ~CommunicationModule()override;
    // 初始化模块
    bool initialize() override;
    // 清理资源
    void shutdown() override;

private slots:
    // 接收CommunicationManager的状态更新
    void onChannelStatusChanged(CommunicationChannel channel, const QString& status);
    // 接收CommunicationManager的错误信息
    void onChannelError(CommunicationChannel channel, const QString& error);
    //接收硬件回复数据
    void onHardwareRespond(const DataMessage& message);
    //接收FTP文件列表回复
    void onFTPFileListRespond(CommunicationChannel channel, bool success,
        const QStringList& files, const QString& msg);
    // 处理FTP上传进度
    void onFtpUploadProgress(CommunicationChannel channel, int progress);
    // 处理FTP下载进度
    void onFtpDownloadProgress(CommunicationChannel channel, int progress);

    //通信状态处理
    void onChannelStatusRespond(const DataMessage& msg);

private:
    // 注册命令处理器(外部模块可调用的命令)
    void registerCommandHandlers();

    //注册数据接收者
    void registerDataReceivers();

    //注册状态处理器
    void subscribeStatuses();

    // 通用JSON解析函数
    bool parseOuterJson(const QByteArray& data, QJsonObject& outJson);
    
    void publishConfigResult(CommunicationChannel channel, bool success, 
        const QString& msg);
    void publishStopResult(CommunicationChannel channel, bool success,
        const QString& msg);
    // 控制类命令结果发布（自检等，带元信息）
    void publishControlResult(CommunicationChannel channel, bool success,
        const QString& msg,
        DeviceId deviceId, CommandCode cmdCode, uint8_t packetSeq,
        ModuleType sourceModule, uint64_t requestId);

    void publishCommandError(const DataMessage& msg, const QString& errorMsg,
        DeviceId deviceId, CommandCode cmdCode, uint8_t packetSeq,
        ModuleType sourceModule);

    // 数据传输类命令结果发布（FTP文件下发等，带业务标识）
    void publishTransferResult(CommunicationChannel channel, bool success,
        const QString& msg,ModuleType sourceModule, uint64_t requestId,
        const QString& operationType);

    // 异步发送函数（复用线程逻辑）
    void sendFrameAsyncForControl(CommunicationChannel channel, 
        const QByteArray& frame,
        DeviceId deviceId, CommandCode cmdCode, uint8_t packetSeq,
        ModuleType sourceModule, uint64_t requestId, BDSDataType bdsDataType = BDSDataType::ATCommand);
   
    // 命令处理函数

    //统一控制命令入口解析来源并分发
    void handleControlCommand(const DataMessage& msg);

    void handleSelfCheckSendCmd(CommunicationChannel channelType, 
        const QJsonObject& params);  // 处理自检发送命令
    
    void handleControlParamSendCmd(CommunicationChannel channelType,
        const QJsonObject& params);

    void handleSailControlSendCmd(CommunicationChannel channelType,
        const QJsonObject& params);

    void handleSwitchChannel(const DataMessage& msg);    // 切换通信通道
    void handleSetChannelConfig(const DataMessage& msg); // 配置通道参数
    void handleChannelClose(const DataMessage& msg);     // 关闭通道
    void handleQueryChannelStatus(const DataMessage& msg);// 查询通道状态

    //使命文件处理函数
    void handleMissionIssueTransfer(const DataMessage& msg);

    //文件下载处理
    void handleFileDownloadTransfer(const DataMessage& msg);

    //文件上传处理
    void handleFileUploadTransfer(const DataMessage& msg);

    //文件列表获取处理
    void handleFileListGetTransfer(const DataMessage& msg);

    //删除文件处理
    void handleFileDeleteTransfer(const DataMessage& msg);

    //断开通道连接
    void handleStopChannel(const DataMessage& msg);

private:
    CommunicationManager* m_commManager; //指向物理层管理器
      // 缓存各通道当前状态
    QMap<CommunicationChannel, CommunicationChannelStatus> m_channelStatusCache;

};
