#pragma once

#include"basemodule.h"
#include"missionmonitorresult.h"
#include "frameglobal.h"
class MissionMonitorModule  : public BaseModule
{
	Q_OBJECT

public:
	explicit MissionMonitorModule(QObject *parent = nullptr);
	~MissionMonitorModule();

    /**
     * @brief 初始化模块
     * @return 初始化是否成功
     */
    bool initialize() override;

    /**
     * @brief 清理模块资源
     */
    void shutdown() override;

    CheckError downloadFile(const QString& localPath, const QString& remotePath);

private slots:
    //处理接收到的auv位置信息
    void onAUVPositionReceived(const DataMessage& data);

private:
    void notifyObservers(const MissionMonitorResult& result);
};
