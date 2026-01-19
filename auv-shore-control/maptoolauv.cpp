#include "maptoolauv.h"
#include<QMenu>
#include<QToolTip>
#include<QTimer>
#include"auvrenderer.h"
#include"auvinfopanel.h"
#pragma execution_character_set("utf-8")
MapToolAuv::MapToolAuv(CS57Control* parent, AuvRenderer* renderer, AuvInfoPanel* infoWidget)
	:CS57MapTool(parent)
	, m_auvRenderer(renderer)
	, m_infoPanel(infoWidget)
	, m_sSelectedAuvId("")
	, m_sSelectedTargetId("")
	, m_sSelectedTrajAuvId("")
	, m_iSelectedTrajPointIndex(-1)
	, m_bIsDragging(false)
	

{
	//设置默认光标
	//setCursor(Qt::ArrowCursor);

	//创建右键菜单
	m_auvContextMenu = new QMenu(parent);

	//添加菜单项
	QAction* selfCheckAction = new QAction("设备自检", m_auvContextMenu);
	QAction* deviceStatusAction = new QAction("设备状态信息", m_auvContextMenu);
	QAction* missionInfoAction = new QAction("使命信息", m_auvContextMenu);

	//连接菜单信号
	connect(selfCheckAction, &QAction::triggered, this, &MapToolAuv::on_selfCheck_triggered);
	connect(deviceStatusAction, &QAction::triggered, this, &MapToolAuv::on_deviceStatus_triggered);
	connect(missionInfoAction, &QAction::triggered, this, &MapToolAuv::on_missionInfo_triggered);

	//添加到菜单
	m_auvContextMenu->addAction(selfCheckAction);
	m_auvContextMenu->addAction(deviceStatusAction);
	m_auvContextMenu->addAction(missionInfoAction);


}

MapToolAuv::~MapToolAuv()
{
	if (m_auvContextMenu != nullptr)
		delete m_auvContextMenu;
	m_auvContextMenu = nullptr;
	
}



/**
 * @brief 鼠标移动事件（处理悬停提示）
 * @param event 
*/
void MapToolAuv::canvasMoveEvent(QMouseEvent * event)
{
	if (!m_auvRenderer)
		return;
	
}

void MapToolAuv::canvasDoubleClickEvent(QMouseEvent* event)
{
	Q_UNUSED(event);
	if (!m_sSelectedAuvId.isEmpty())
	{
		//获取选中AUV的地理坐标，设置海图中心
		auto it = m_auvRenderer->getAuvList().find(m_sSelectedAuvId);
		if (it != m_auvRenderer->getAuvList().end())
		{
			const GeoCoord& centerGeo = it->geoPos;
			m_pS57Control->setViewportCenterPt(centerGeo.lon, centerGeo.lat);
			m_pS57Control->refreshCanvas();
		}
	}
	
}

void MapToolAuv::canvasPressEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton)
	{
		//清除之前的状态
		clearSelection();
		//检查点击是否命中元素
		bool isAuvHit = checkAuvHit(e->pos());
		if (!isAuvHit)
		{
			bool isTargetHit = checkTargetHit(e->pos());
			if (!isTargetHit)
			{
				bool isMissionPointHit = checkMissionWaypointHit(e->pos());
				if (!isMissionPointHit)
				{
					checkTrajectoryHit(e->pos());
				}
			}
		}

		//刷新画布显示新的选择状态
		m_pS57Control->refreshCanvas();

	}
	//右键点击
	else if (e->button() == Qt::RightButton && !m_sSelectedAuvId.isEmpty())
	{
		m_auvContextMenu->exec(e->globalPos());
	}
}

void MapToolAuv::canvasReleaseEvent(QMouseEvent* event)
{	
	Q_UNUSED(event);
}

void MapToolAuv::wheelEvent(QWheelEvent* e)
{
}

void MapToolAuv::on_selfCheck_triggered()
{

}
void MapToolAuv::on_deviceStatus_triggered()
{
}

void MapToolAuv::on_missionInfo_triggered()
{
}

/**
 * @brief 检查是否命中AUV
 * @param screenPos 屏幕坐标
 * @param isHover 是否悬停
 * @return 
*/
bool MapToolAuv::checkAuvHit(const QPointF& screenPos)
{
	//命中半径
	double auvSize = m_auvRenderer->getElementSize(AuvRenderer::FixAuvSize, 15);
	double hitRadius = auvSize * 1.5;
	double hitRadiusSquared = hitRadius * hitRadius;

	//遍历所有AUV检查命中
	for (const auto& auv : m_auvRenderer->getAuvList())
	{
		QPointF auvPos = m_auvRenderer->geoToScreen(auv.geoPos);
		if (distanceSquared(screenPos, auvPos) <= hitRadiusSquared)
		{
			
			//选中状态：显示详细信息
			m_sSelectedAuvId = auv.auvId;
			m_auvRenderer->setAuvSelected(auv.auvId, true);
			
			//在dockwidget中显示详细信息
			if (m_infoPanel)
			{
				m_infoPanel->setVisible(true);
				m_infoPanel->showAuvInfo(auv);

			}
			return true;
		}
	}
	return false;
}

bool MapToolAuv::checkTargetHit(const QPointF& screenPos)
{
	//命中半径：目标渲染尺寸的2倍
	double targetSize = m_auvRenderer->getElementSize(AuvRenderer::FixTargetSize, 10.0);
	double hitRadius = targetSize * 2.0;
	double hitRadiusSquared = hitRadius * hitRadius;

	//遍历所有目标检查命中
	for (const auto& target : m_auvRenderer->getTargetList())
	{
		QPointF targetPos = m_auvRenderer->geoToScreen(target.coord);
		if (distanceSquared(screenPos, targetPos) <= hitRadiusSquared)
		{
			//更新选中状态
			m_sSelectedTargetId = target.targetId;
			m_auvRenderer->setTargetSelected(target.targetId, true);
			if (m_infoPanel)
			{
				m_infoPanel->setVisible(true);
				m_infoPanel->showTargetInfo(target);
			}
			return true;
		}
	}
	return false;
}

/**
 * @brief 检查是否命中轨迹点
 * @param screenPos 屏幕坐标 
 * @return 命中结果
*/
bool MapToolAuv::checkTrajectoryHit(const QPointF& screenPos)
{
	//命中半径：轨迹点渲染尺寸的3倍
	double trajPointSize = m_auvRenderer->getElementSize(AuvRenderer::FixTrajPointSize, 5.0);
	double hitRadius = trajPointSize * 3.0;
	double hitRadiusSquared = hitRadius * hitRadius;

	const auto& trajectoryMap = m_auvRenderer->getTrajectoryList();
	QMap<QString, QVector<ActualTrajectoryPoint>>::const_iterator it;

	//遍历所有轨迹点检查命中
	for (it = trajectoryMap.begin(); it != trajectoryMap.end(); ++it )
	{
		const QString& auvId = it.key();
		const auto& points = it.value();
		for (int i = 0; i < points.size(); ++i)
		{
			QPointF pointPos = m_auvRenderer->geoToScreen(points[i].coord);
			if (distanceSquared(screenPos, pointPos) <= hitRadiusSquared)
			{
				
				m_sSelectedTrajAuvId = auvId;
				m_iSelectedTrajPointIndex = i;
				m_auvRenderer->setActualTrajPointSelected(auvId, m_iSelectedTrajPointIndex, true);
				//在dockwidget中显示详细信息
				if (m_infoPanel)
				{
					const auto& point = points[i];
					m_infoPanel->setVisible(true);
					m_infoPanel->showTrajectoryPointInfo(auvId,point);
				}
				
				
				return true;
			}
		}
	}
	return false;
}

bool MapToolAuv::checkMissionWaypointHit(const QPointF& screenPos)
{
	//遍历所有AUV的使命轨迹
	const auto& missionMap = m_auvRenderer->getMissionTrajectoryList();
	for (auto it = missionMap.begin(); it != missionMap.end(); ++it)
	{
		const QString& auvId = it.key();
		const auto& mission = it.value();

		//计算点击检测半径
		double hitRadius = m_auvRenderer->getElementSize(AuvRenderer::FixMissionWpSize, 8.0);
		double hitRadiusSquared = hitRadius * hitRadius;

		for (int i = 0; i < mission.waypoints.size(); ++i)
		{
			const auto& waypoint = mission.waypoints[i];
			QPointF wpPos = m_auvRenderer->geoToScreen(waypoint.coord);
			if (distanceSquared(screenPos, wpPos) <= hitRadiusSquared)
			{

				m_sSelectedMissionAuvId = auvId;
				m_iSelectedWaypointIndex = i;
				m_auvRenderer->setMissionWaypointSelected(auvId, i, true);
				//在dockwidget中显示详细信息
				if (m_infoPanel)
				{
					m_infoPanel->setVisible(true);
					m_infoPanel->showMissionPointInfo(auvId, waypoint);
				}


				return true;
			}
		}
	}
	return false;
}



inline int MapToolAuv::distanceSquared(const QPointF& p1, const QPointF& p2)
{
	int dx = p1.x() - p2.x();
	int dy = p1.y() - p2.y();
	return dx * dx + dy * dy;
}

/**
 * @brief 清除选中状态
 *
*/
void MapToolAuv::clearSelection()
{
	m_auvRenderer->clearAllSelection();
	m_sSelectedAuvId.clear();
	m_sSelectedTargetId.clear();
	m_sSelectedTrajAuvId.clear();
	m_iSelectedTrajPointIndex = -1; 
	m_sSelectedMissionAuvId.clear();
	m_iSelectedWaypointIndex = -1;

	if (m_infoPanel)
	{
		m_infoPanel->clear();
	}
}
