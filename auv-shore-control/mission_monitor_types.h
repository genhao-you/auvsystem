#pragma once
#include<QString>
#include<QDateTime>
//经纬度坐标结构体
struct GeoCoord
{
	double lon;//经度
	double lat;//纬度
	double depth;
	GeoCoord(double lon_ = 0, double lat_ = 0, double depth_ = 0) :lon(lon_), lat(lat_), depth(depth_) {}
	bool isValid()const { return (lat >= -90 && lat <= 90) && (lon >= -180 && lon <= 180); }
	QString toString()const { return QString("%1,%2").arg(lat, 0, 'f', 6).arg(lon, 0, 'f', 6).arg(depth, 0, 'f', 6); }
};

/*
* 使命航点类型
*/
enum class WaypointType
{
	Start,   //起点
	Turning, //拐点
	End,    //终点
	Mission  //任务点
};
//使命计划轨迹点
struct MissionWaypoint
{
	QString waypointId;//航点id
	GeoCoord coord;//坐标
	double stayTime; //停留时间
	double radius;  //到达半径
	QString taskDesc;//该航点任务描述
	WaypointType type;  //点类型(起点/拐点/终点/任务点)
	//QVariant missionData;  //任务数据
	bool isSelected =false;  //是否高亮

	MissionWaypoint(const QString& waypoint_id, const GeoCoord& coordinate):
		waypointId(waypoint_id), coord(coordinate), stayTime(0.0), radius(10.0),type(WaypointType::Turning), isSelected(false) {}
	MissionWaypoint() : stayTime(0), isSelected(false) {}
	bool isValid()const { return coord.isValid() && !waypointId.isEmpty(); }
};

//使命轨迹(预规划路径)
struct MissionTrajectory
{
	QString missionId;
	QList<MissionWaypoint>waypoints;
	bool isActive;
	MissionTrajectory(const QString& id = "") :missionId(id), isActive(false) {}
	bool isValid()const { return !missionId.isEmpty() && waypoints.size() >= 2; }
	void clear() { waypoints.clear(); missionId.clear(); isActive = false; }
};


//实际轨迹点结构
struct ActualTrajectoryPoint
{
	GeoCoord coord;   //位置坐标
	QDateTime timestamp;  //时间戳
	double heading;			//航向角
	double speed;			//速度
	bool isSelected; // 是否高亮

	ActualTrajectoryPoint(const GeoCoord& c = GeoCoord()) :coord(c), timestamp(QDateTime::currentDateTime()), isSelected(false) {}
	bool isValid()const { return coord.isValid(); }
};

//环境目标信息
struct DetectedTarget
{
	QString targetId;      //目标唯一id
	GeoCoord coord;    //目标经纬度
	double depth;      //目标深度
	QString type;      //目标类型(如"水面船只","水雷")
	double speed;      //目标航速
	double heading;    //目标航向
	double distance;   //距auv距离
	int threatLevel;   //威胁等级
	QString detectDevice;  //探测设备(如DC"桅杆"、"单波束声呐")
	double  confidence; //探测置信度（0-5）
	bool isSelected;    //选中状态
	QDateTime timestamp;	//时间戳
	DetectedTarget() :confidence(0.0),isSelected(false){}

	bool isValid()const { return !targetId.isEmpty() && coord.isValid() && (confidence >= 0.0 && confidence <= 1.0); }
};

//使命监控结果类型(区分更新不同内容)
enum class MissionMonitorResultType
{
	RealTimePosition,    //实时定位更新
	HistoricalTrail,     //历史轨迹点
	PlannedTrail,       //计划轨迹更新
	DetectedTarget    //环境目标更新
};

//AUV位置与状态信息
struct AuvRealTimeData
{
	QString auvId = "1";          //AUV编号
	GeoCoord geoPos;		    //经纬度坐标
	double heading;			    //航向角
	double speed;			    //速度
	QString status;			    //状态描述
	bool isConnected;		    //连接状态
	QDateTime timestamp;	    //时间戳
	bool isSelected;
	AuvRealTimeData() :heading(0), speed(0), isConnected(false), isSelected(false) {}
	bool isValid()const { return geoPos.isValid() && !auvId.isEmpty(); }
};

