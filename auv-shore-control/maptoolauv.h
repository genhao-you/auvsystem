#pragma once

#include <QObject>
#include"cs57maptool.h"
class AuvRenderer;
class AuvInfoPanel;
class MapToolAuv  : public CS57MapTool
{
	Q_OBJECT

public:
	MapToolAuv(CS57Control *parent,AuvRenderer* renderer,AuvInfoPanel* infoWidget);
	~MapToolAuv();
	void canvasMoveEvent(QMouseEvent* e) override;

	void canvasDoubleClickEvent(QMouseEvent* e) override;

	void canvasPressEvent(QMouseEvent* e) override;

	void canvasReleaseEvent(QMouseEvent* e) override;

	void wheelEvent(QWheelEvent* e) override;
private slots:
	//菜单动作响应
	void on_selfCheck_triggered();
	void on_deviceStatus_triggered();
	void on_missionInfo_triggered();

private:
	//检查点击位置是否命中元素
	bool checkAuvHit(const QPointF& screenPos);
	bool checkTargetHit(const QPointF& screenPos);
	bool checkTrajectoryHit(const QPointF& screenPos);
	bool checkMissionWaypointHit(const QPointF& screenPos);
	//辅助计算：两点间距离平方（避免开方计算，提高效率）
	inline int distanceSquared(const QPointF& p1, const QPointF& p2);


	void clearSelection();
private:
	AuvRenderer* m_auvRenderer;        //AUV渲染器指针
	QMenu* m_auvContextMenu;		   //AUV右键菜单
	AuvInfoPanel* m_infoPanel;

	//选中状态跟踪
	QString m_sSelectedAuvId;		   //当前选中的AUVid
	QString m_sSelectedTargetId;	   //当前选中的目标id
	QString m_sSelectedTrajAuvId;	   //当前选中的轨迹所属AUV id
	int m_iSelectedTrajPointIndex;	   //当前选中的轨迹点索引

	QString m_sSelectedMissionAuvId;   //选中航点所属的auv id
	int m_iSelectedWaypointIndex;      //选中的航点索引
	bool m_bIsDragging;    			   //是否正在拖拽
};									   
