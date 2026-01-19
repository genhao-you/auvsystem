#pragma once

#include <QObject>
#include "basemodule.h"
#include "frameglobal.h"
#include"communicationchannelstatusresult.h"
#include"wirednetworkconfig.h"
#include"ftploginlistresult.h"
#include"ftptransferresult.h"
#include"ftpprogressresult.h"
#include"ftpdeleteresult.h"
class DataModule : public BaseModule
{
	Q_OBJECT

public:
	DataModule(QObject *parent);
	~DataModule() override;
	// 初始化模块
	bool initialize() override;
	// 清理资源
	void shutdown() override;

	CheckError logIn(const QString& host, const QString& port,
		const QString& user, const QString& pwd);
	CheckError logOut();
	CheckError downloadFile(const QString& localPath, const QString& remotePath);
	CheckError uploadFile(const QString& localPath, const QString& remotePath);
	CheckError getRemoteFileList(const QString& remotePath);
	CheckError deleteRemoteItem(const QString& remotePath, FtpDeleteResult::DeleteType delType);
private:
	//通信状态处理
	void onChannelStatusReceived(const DataMessage& msg);

	//数据传输结果分类
	void onFtpTransferReceived(const DataMessage& msg);
	void onFtpFileListReceived(const DataMessage& msg);
	void onFtpTransferProgressReceived(const DataMessage& msg);
};
