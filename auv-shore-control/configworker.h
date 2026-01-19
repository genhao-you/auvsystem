#pragma once

#include <QObject>
#include"communicationchannel.h"
#include"channelconfig.h"
#include"communicationmanager.h"
#include "ftpdeleteresult.h"
class ConfigWorker  : public QObject
{
	Q_OBJECT

public:
	ConfigWorker(CommunicationChannel channel,
		CommunicationManager* commManager, QObject* parent = nullptr);
	ConfigWorker(CommunicationChannel channel,const ChannelConfigPtr&config,
		CommunicationManager* commManager,QObject *parent = nullptr);
	ConfigWorker(CommunicationChannel channel, QByteArray frame , CommunicationManager* commManager, 
		BDSDataType bdsDataType = BDSDataType::ATCommand, // 参数
		QObject* parent = nullptr);
	// 构造函数调整：接收路径而非data
	ConfigWorker(CommunicationChannel channel, const QString& localPath, const QString& remotePath,
		ModuleType sourceModule, uint64_t requestId, CommunicationManager* commManager, QObject* parent = nullptr);
	~ConfigWorker();

	void setDeleteType(FtpDeleteResult::DeleteType type);
signals:
		//配置完成信号
	void configFinished(bool configSuccess, CommunicationChannel channel);
	void dataSendFinished(bool sendSuccess, CommunicationChannel channel);
	// 任务文件下发结果信号（携带请求ID）
	void TaskFileSendFinished(bool sendSuccess, CommunicationChannel channel,
		uint64_t requestId);
	//文件下载结果信号
	void FileDownloadFinished(bool downloadSuccess, CommunicationChannel channel,
		uint64_t requestId);
	//文件上传信号
	void FileUploadFinished(bool uploadSuccess, CommunicationChannel channel,
		uint64_t requestId);

	void FileDeleteFinished(bool success, CommunicationChannel channel, uint64_t requestId);
	//文件列表结果信号
	void fileListFetched(bool success, const QStringList& files,
		CommunicationChannel channel, uint64_t requestId);
	// 停止通道结果信号
	void stopFinished(bool stopSuccess, CommunicationChannel channel);

public slots:
	//执行配置的槽函数
	void doConfig();

	//执行发送数据的槽函数
	void doSendData();

	//执行下发任务文件的槽函数
	void doSendTasksFile();

	//执行上传文件的槽函数
	void doUploadFile();
	
	//执行文件下载的槽函数
	void doDownloadFile();

	// 执行文件列表获取
	void doListFiles();

	// 执行停止通道的槽函数
	void doStop();

	void doDeleteFile();
private:
	CommunicationChannel m_channel;
	ChannelConfigPtr m_config;
	CommunicationManager* m_commManager;
	QByteArray m_frame;
	QString m_localPath; // 本地路径
	QString m_remotePath; // 远程路径
	ModuleType m_sourceModule;  // 来源模块
	uint64_t m_requestId;       // 任务请求ID
	QMetaObject::Connection m_currentConn; // 保存当前信号连接
	BDSDataType m_bdsDataType; // 保存BDS类型
	FtpDeleteResult::DeleteType m_deleteType = FtpDeleteResult::DeleteType::File;
};
