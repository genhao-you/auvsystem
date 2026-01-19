#pragma once
#include"baseresult.h"
#include"mission_monitor_types.h"
/**
 * @brief 使命监控模块结果类
 *封装使命监控的各类信息：实时定位、历史轨迹、计划轨迹、环境态势
 */
class MissionMonitorResult :public BaseResult
{
public:
	MissionMonitorResult(MissionMonitorResultType resultType, QString auvId);
	~MissionMonitorResult();

	//-------------------
	//通用字段(所有结果类型共享)
	//-------------------
	MissionMonitorResultType resultType()const;
	void setResultType(MissionMonitorResultType type);


	//设置/获取AUVID(多auv场景下区分)
	void setAuvId(QString id);
	QString auvId()const;


	//-------------------
	//实时定位信息(对应RealTimePosition类型)
	//-------------------
	void setRealTimeData(const AuvRealTimeData& coord);
	AuvRealTimeData realTimeData()const;

	//-------------------
	//历史轨迹信息(对应HistoricalTrail类型)
	//-------------------
	void addHistoricalPoint(const GeoCoord& point);
	QList<GeoCoord> historicalTrail()const;
	void clearHistoricalTrail();

	//-------------------
	//计划轨迹信息(对应PlannedTrail类型)
	//------------------
	void setPlannedWaypoints(const QList<MissionWaypoint>& waypoints);
	QList<MissionWaypoint>plannedWaypoins()const;

	//------------------
	//环境态势信息(对应EnvironmmentTarget类型)
	//------------------
	void addEnvironmentTarget(const DetectedTarget& target);
	QList<DetectedTarget>environmentTargets()const;
	void removeEnvironmentTarget(int targetId);//移除消失目标

	//重写描述方法(根据结果类型生成描述)
	QString description()const ;
private:
	MissionMonitorResultType m_resultType;//结果类型
	QString m_sAuvId = 1;

	//实时定位
	AuvRealTimeData m_realTimeData;

	//历史轨迹(累计点集)
	QList<GeoCoord>m_historicalTrail;

	//计划轨迹(航点列表)
	QList<MissionWaypoint>m_plannedWaypoints;
	//环境目标
	QList<DetectedTarget>m_environmentTargets;

};
