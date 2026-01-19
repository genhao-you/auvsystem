#include "missionmonitormodule.h"
#include<QJsonDocument>
MissionMonitorModule::MissionMonitorModule(QObject *parent)
	: BaseModule(ModuleType::Monitoring,"Monitoring", parent)
{
    //注册结果处理器：接收CommunicationModule返回的auv位置
    subscribeEvent("missionmonitor_result",
        [this](const DataMessage& msg)
        {
            onAUVPositionReceived(msg);
        }
    );
  
}


MissionMonitorModule::~MissionMonitorModule()
{}

bool MissionMonitorModule::initialize()
{
	return false;
}

void MissionMonitorModule::shutdown()
{

}

CheckError MissionMonitorModule::downloadFile(const QString& localPath, const QString& remotePath)
{
    // 1. 生成全局唯一request_id
    uint64_t requestId = CommandIdGenerator::instance().nextId();
    // 2. 封装元数据（本地路径、远程路径）和JSON数据
    QJsonObject meta;
    meta["localPath"] = localPath;
    meta["remotePath"] = remotePath;
    meta["request_id"] = QString::number(requestId); // 携带request_id
    meta["source_module"] = static_cast<int>(ModuleType::Monitoring); // 携带发送模块标识
   // 直接传递元数据JSON，无需组合JSON内容
    QByteArray metaData = QJsonDocument(meta).toJson(QJsonDocument::Compact);

    //传输数据到通信模块
    // 3. 调用BaseModule的transferData，指定目标和通道
    transferData(
        CommunicationChannel::WiredNetwork, // 通信通道（如WiredNetwork）
        "fileDownload_transfer",       // 目标接收器标识（唯一）
        metaData                   // 直接传输的二进制数据
    );

    return CheckError();
}
void MissionMonitorModule::onAUVPositionReceived(const DataMessage& data)
{
	AuvRealTimeData auv;
	auv.auvId = "AUV-001";
	//QPointF point =  m_s57MapModule->getMapCanvas()->getViewportCenterPt();  { x = 108.87541665000001, y = 18.383194399847842 }   108°52′31″，18°22′54″
	auv.geoPos = { 108.8754,18.38319 };
	auv.heading = 0;
	auv.speed = 1.2;
	auv.isConnected = true;
	MissionMonitorResult result (MissionMonitorResultType::RealTimePosition, auv.auvId);
	result.setRealTimeData(auv);
    notifyObservers(result);

}
void MissionMonitorModule::notifyObservers(const MissionMonitorResult& result)
{
    BaseModule::notifyObservers(&result);
}
