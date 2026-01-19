#pragma once

#include <QObject>
#include"MapModule/s57mapmodule.h"
#include"mission_monitor_types.h"
class CS57CanvasImage;

class AuvRenderer  : public QObject
{
	Q_OBJECT

public:
	explicit AuvRenderer(CMapCanvas*mapCanvas,QObject *parent = nullptr);
	~AuvRenderer() override;

	//--------------------
	//AUV管理接口
	//--------------------
	void addAuv(const AuvRealTimeData& auvData);

	void updateAuv(const QString& auvId, const AuvRealTimeData& auvData);

	//--------------------
	//轨迹管理接口
	//--------------------
	//void addTrajectory(const QString& auvId);

	//void updateMissionTrajectory(const QString& auvId, const QVector<MissionWaypoint>& waypoints);

	void addActualTrajectoryPoint(const QString& auvId, const ActualTrajectoryPoint& point);


	//-------------------
	//目标管理接口
	//------------------
	void addTarget(const DetectedTarget& target);

	void updateTarget(const QString& targetId, const DetectedTarget& targetData);



	//-------------------
	//高亮控制
	//------------------
	//void clearAllHighlights();

	//清除所有选中状态
	void clearAllSelection();

	//使命轨迹管理接口
	void setAuvMissionTrajectory(const QString& auvId, const MissionTrajectory& mission);

	void setMissionActive(const QString& auvId, bool isActive);

	void clearAuvMissionTrajectory(const QString& auvId);

	void setWaypointHighlighted(const QString& auvId, const QString& waypointId, bool highlighted);
	
	//在CS57画布上叠加绘制AUV
	void paintAuvOnCs57Canvas(CS57CanvasImage* canvasImage);

	void clearAllData();

	//获取AUV列表
	const QMap<QString, AuvRealTimeData>& getAuvList() const;
	//获取目标列表
	const QMap<QString, DetectedTarget>& getTargetList()const;
	//获取轨迹列表
	const QMap<QString, QVector<ActualTrajectoryPoint>>& getTrajectoryList()const;
	//获取使命航点列表
	const QMap<QString, MissionTrajectory>& getMissionTrajectoryList()const;

	//设置Auv选中状态
	void setAuvSelected(const QString& auvId, bool selected);

	//设置目标选中状态
	void setTargetSelected(const QString& targetId, bool selected);

	//设置轨迹点选中状态
	void setActualTrajPointSelected(const QString& auvId,int pointIndex, bool selected);
	bool isTrajectoryPointSelected(const QString& auvId, int pointIndex) const;
	

	void setMissionWaypointSelected(const QString& auvId, int waypointIndex, bool selected);
	bool isMissionWaypointSelected(const QString& auvId, int waypointIndex)const;

	double getElementSize(double fixedSize, double baseSize)const;
	//地理坐标->屏幕坐标
	QPointF geoToScreen(const GeoCoord& geo)const;


	// 统一尺寸定义：类内静态常量，替代全局变量
	static const double FixAuvSize;         // AUV主体固定像素尺寸
	static const double FixTrajPointSize;   // 实际轨迹点固定像素尺寸
	static const double FixMissionWpSize;   // 使命航点固定像素尺寸
	static const double FixTargetSize;      // 探测目标固定像素尺寸
	static const double FixTextSize;        // 文本固定像素尺寸
	static const double FixLineWidth;       // 固定线宽像素尺寸
private:
	

	//动态计算尺寸
	double scaledSize(double baseSize)const;

	//绘制单个AUV
	void paintSingleAuv(QPainter* painter, const AuvRealTimeData& data);
	//绘制所有AUV的轨迹
	void paintAllTrajectories(QPainter* painter);
	//绘制所有探测目标
	void paintAllTargets(QPainter* painter);

	//使命轨迹绘制
	void paintAllMissionTrajectories(QPainter* painter);
	void paintMissionWaypoints(QPainter* painter, const MissionTrajectory& mission, const QString& auvId);
	//根据航点类型获取对应图标
	QPolygonF getWaypointShape(WaypointType type, double size);
	//根据航点类型获取对应颜色
	QColor getWaypointColor(WaypointType type, bool isActive, bool isSelected);
	//绘制礁石图标
	void drawRockIcon(QPainter* painter, QPointF pos, double size);

	//绘制沉船图标
	void drawWreckIcon(QPainter* painter, QPointF pos, double size);

	//绘制生物群图标
	void drawBiologicalIcon(QPainter* painter, QPointF pos, double size);

	//绘制障碍物图标
	void drawObstacleIcon(QPainter* painter, QPointF pos, double size);

	//绘制默认图标
	void drawDefaultIcon(QPainter* painter, QPointF pos, double size);



	bool isScreenVisible(const QPointF& screenPos)const;
	bool isGeoVisible(const GeoCoord& geo)const;

	
private:
	CMapCanvas* m_mapCanvas;

	//AUV数据缓存
	QMap<QString, AuvRealTimeData>m_auvMap;
	//轨迹数据缓存
	QMap<QString, QVector<ActualTrajectoryPoint>> m_actualTrajMap;

	QMap<QString, MissionTrajectory>m_missionTrajMap;  //保存使命轨迹数据（Auv ID->航点）
	
	QMap<QString,DetectedTarget>m_targetMap;    //保存目标数据
	
	//实际轨迹点选中状态（key：auvId，value：每个轨迹点的选中状态）
	QMap<QString, QVector<bool>>m_trajPointSelected;

	//使命航点选中状态：key:auvId，value：每个航点的选中状态
	QMap<QString, QVector<bool>>m_missionWaypointSelected;


	//配置参数
	const double m_dMinScale = 500.0;
	const double m_dMaxScale = 100000000.0;
	const int m_iMaxTrajPoints = 2000;  //轨迹点最大数量
	const double m_dBaseScale = 100000.0; // 尺寸计算基准比例尺

	//---------------固定尺寸配置

	const double m_dFixTextSize = 5.0;       // 文本固定像素尺寸
	const double m_dFixLineWidth = 1.5;    //固定线宽像素尺寸
	const bool   m_bUseFixedSize = true;        //开关：是否启用固定尺寸
};
