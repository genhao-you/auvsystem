#include "auvrenderer.h"
#include"cs57canvasimage.h"
#include<qDebug>
#pragma execution_character_set("utf-8")

// 初始化类内静态尺寸常量
const double AuvRenderer::FixAuvSize = 15.0;
const double AuvRenderer::FixTrajPointSize = 5.0;
const double AuvRenderer::FixMissionWpSize = 8.0;
const double AuvRenderer::FixTargetSize = 10.0;
const double AuvRenderer::FixTextSize = 10.0;
const double AuvRenderer::FixLineWidth = 1.5;


AuvRenderer::AuvRenderer(CMapCanvas* mapCanvas, QObject* parent)
	: QObject(parent)
	,m_mapCanvas(mapCanvas)
{

}

AuvRenderer::~AuvRenderer()
{
	//clearAllData();
}

void AuvRenderer::addAuv(const AuvRealTimeData& auvData)
{
	if (!auvData.isValid() || m_auvMap.contains(auvData.auvId))
		return;
	m_auvMap.insert(auvData.auvId, auvData);
	if (m_mapCanvas)
		m_mapCanvas->refreshCanvas();
}

void AuvRenderer::updateAuv(const QString& auvId, const AuvRealTimeData& auvData)
{
	if (!auvData.isValid() || !m_auvMap.contains(auvId))
		return;
	m_auvMap[auvId] = auvData;
	
	if (m_mapCanvas)
		m_mapCanvas->refreshCanvas();
}

void AuvRenderer::addActualTrajectoryPoint(const QString& auvId, const ActualTrajectoryPoint& point)
{
	if (auvId.isEmpty() || !point.isValid() ||!m_auvMap.contains(auvId))
		return;
	
	//m_actualTrajMap.insert(auvId, points);
	auto& points = m_actualTrajMap[auvId];
	points.append(point);
	if (m_actualTrajMap.contains(auvId))
	{
		m_trajPointSelected[auvId].append(false);
	}
	//限制轨迹点数量，避免性能问题
	if (points.size() > m_iMaxTrajPoints)
	{
		points.removeFirst();
	}
	if (m_mapCanvas)
		m_mapCanvas->refreshCanvas();
}

void AuvRenderer::addTarget(const DetectedTarget& target)
{
	if (!target.isValid() || m_targetMap.contains(target.targetId))
		return;
	m_targetMap.insert(target.targetId, target);
	if (m_mapCanvas)
		m_mapCanvas->refreshCanvas();
	
}

void AuvRenderer::updateTarget(const QString& targetId, const DetectedTarget& targetData)
{
	if (targetId.isEmpty() || !targetData.isValid() || !m_targetMap.contains(targetId))
		return;
	m_targetMap[targetId] = targetData;
	if (m_mapCanvas)
		m_mapCanvas->refreshCanvas();
}


void AuvRenderer::clearAllSelection()
{
	for (auto& auv : m_auvMap)auv.isSelected = false;
	for (auto& target : m_targetMap)target.isSelected = false;
	m_trajPointSelected.clear();
	m_missionWaypointSelected.clear();
	if (m_mapCanvas)
		m_mapCanvas->refreshCanvas();

}

void AuvRenderer::setAuvMissionTrajectory(const QString& auvId, const MissionTrajectory& mission)
{
	if (!mission.isValid() || !m_auvMap.contains(auvId))
		return;
	//m_missionTrajMap.insert(auvId, mission);
	m_missionTrajMap[auvId] = mission;
	//初始化选中状态容器
	m_missionWaypointSelected[auvId]= QVector<bool>(mission.waypoints.size(),false);
	if (m_mapCanvas)
		m_mapCanvas->refreshCanvas();
}

void AuvRenderer::setMissionActive(const QString& auvId, bool isActive)
{
	if (!m_missionTrajMap.contains(auvId))
		return;
	m_missionTrajMap[auvId].isActive = isActive;
}

void AuvRenderer::clearAuvMissionTrajectory(const QString& auvId)
{
	if (m_missionTrajMap.contains(auvId))
	{
		m_missionTrajMap.remove(auvId);
	}
}

void AuvRenderer::setWaypointHighlighted(const QString& auvId, const QString& waypointId, bool highlighted)
{
	if (!m_missionTrajMap.contains(auvId))
		return;

	auto& mission = m_missionTrajMap[auvId];
	for (auto& wp : mission.waypoints)
	{
		if (wp.waypointId == waypointId)
		{
			wp.isSelected = highlighted;
			break;
		}
	}
}

void AuvRenderer::paintAuvOnCs57Canvas(CS57CanvasImage* canvasImage)
{
	if (!canvasImage)
		return;

	//1.获取CS57画布的painter
	QPainter* painter = canvasImage->getCanvasImagePainter();

	//若painter不可访问，基于画布QImage创建临时Painter
	bool isTempPainter = false;
	if (!painter)
	{
		QImage* canvasQImage = canvasImage->canvasImage();
		if (!canvasQImage)
			return;
		painter = new QPainter(canvasQImage);
		isTempPainter = true;
	}

	//2.初始化画笔
	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);
	painter->setRenderHint(QPainter::TextAntialiasing, true);

	//3.绘制顺序：使命轨迹->实际轨迹->目标->AUV
	paintAllMissionTrajectories(painter);
	paintAllTrajectories(painter);
	paintAllTargets(painter);
	for (const auto& auv : m_auvMap)
	{
		paintSingleAuv(painter, auv);
	}

	//4.释放资源
	painter->restore();
	if (isTempPainter)
	{
		delete painter;
	}
}

void AuvRenderer::clearAllData()
{
	m_auvMap.clear();
	m_actualTrajMap.clear();
	m_targetMap.clear();
	m_missionTrajMap.clear();
}

const QMap<QString, AuvRealTimeData>& AuvRenderer::getAuvList() const
{
	// TODO: 在此处插入 return 语句
	return m_auvMap;
}

const QMap<QString, DetectedTarget>& AuvRenderer::getTargetList() const
{
	// TODO: 在此处插入 return 语句
	return m_targetMap;
}

const QMap<QString, QVector<ActualTrajectoryPoint>>& AuvRenderer::getTrajectoryList() const
{
	// TODO: 在此处插入 return 语句
	return m_actualTrajMap;
}

const QMap<QString, MissionTrajectory>& AuvRenderer::getMissionTrajectoryList() const
{
	// TODO: 在此处插入 return 语句
	return m_missionTrajMap;
}

void AuvRenderer::setAuvSelected(const QString& auvId, bool selected)
{
	if (m_auvMap.contains(auvId))
	{
		m_auvMap[auvId].isSelected = selected;
	}
}

void AuvRenderer::setTargetSelected(const QString& targetId, bool selected)
{
	if (m_targetMap.contains(targetId))
	{
		m_targetMap[targetId].isSelected = selected;
	}
}

void AuvRenderer::setActualTrajPointSelected(const QString& auvId, int pointIndex, bool selected)
{
	//检查auv是否存在轨迹数据
	if (!m_actualTrajMap.contains(auvId))
		return;
	const auto& points = m_actualTrajMap[auvId];

	//检查索引有效性
	if (pointIndex < 0 || pointIndex >= points.size())
		return;

	//初始化选中状态容器
	if (!m_trajPointSelected.contains(auvId))
	{
		m_trajPointSelected[auvId] = QVector<bool>(points.size(), false);
	}

	//更新选中状态
	m_trajPointSelected[auvId][pointIndex] = selected;
}

bool AuvRenderer::isTrajectoryPointSelected(const QString& auvId, int pointIndex) const
{
	//检查容器和索引有效性
	if (!m_trajPointSelected.contains(auvId))
		return false;
	const auto& selected = m_trajPointSelected[auvId];
	return (pointIndex >= 0 && pointIndex < selected.size()) ? selected[pointIndex]:false;
}

void AuvRenderer::setMissionWaypointSelected(const QString& auvId, int waypointIndex, bool selected)
{
	//检查auv是否存在使命数据
	if (!m_missionTrajMap.contains(auvId))
		return;
	const auto& mission = m_missionTrajMap[auvId];

	//检查索引有效性
	if (waypointIndex < 0 || waypointIndex >= mission.waypoints.size())
		return;

	//初始化选中状态容器
	if (!m_missionWaypointSelected.contains(auvId))
	{
		m_missionWaypointSelected[auvId] = QVector<bool>(mission.waypoints.size(), false);
	}
	//更新选中状态
	m_missionWaypointSelected[auvId][waypointIndex] = selected;

}

bool AuvRenderer::isMissionWaypointSelected(const QString& auvId, int waypointIndex) const
{
	//检查容器和索引有效性
	if (!m_missionWaypointSelected.contains(auvId))
		return false;
	const auto& selected = m_missionWaypointSelected[auvId];
	return (waypointIndex >= 0 && waypointIndex < selected.size()) ? selected[waypointIndex]:false;
}

QPointF AuvRenderer::geoToScreen(const GeoCoord& geo) const
{
	int screen_x, screen_y;
	m_mapCanvas->transformGeoToScrn(geo.lon, geo.lat, &screen_x, &screen_y);
	return QPointF(screen_x, screen_y);
}

double AuvRenderer::scaledSize(double baseSize) const
{
	//获取当前比例尺
	double currentScale = qBound(m_dMinScale, m_mapCanvas->getDisplayScale(), m_dMaxScale);
	//计算相对比例尺
	double relativeScale = currentScale / m_dBaseScale;
	//对数映射
	double scaleFactor = log10(relativeScale * 9 + 1);
	const double minFactor = 0.3;
	const double maxFactor = 3.0;
	double sizeFactor = minFactor + (maxFactor - minFactor) * scaleFactor;
	return baseSize * sizeFactor;
}

void AuvRenderer::paintSingleAuv(QPainter* painter, const AuvRealTimeData& data)
{
	QPointF screenPos = geoToScreen(data.geoPos);

	if (!isScreenVisible(screenPos))
	{
		return;
	}
	double auvSize = getElementSize(FixAuvSize,10.0); //AUV基准尺寸10px

	painter->save();

	//1.高亮光环(选中时显示)
	if (data.isSelected)
	{
		painter->setPen(QPen(Qt::NoPen));
		painter->setBrush(QColor(255, 255, 0, 80)); //半透明黄色
		painter->drawEllipse(screenPos, auvSize * 2, auvSize * 2);
	}

	//2.AUV主体(箭头)
	painter->translate(screenPos); // 画笔原点移至AUV位置
	painter->rotate(data.heading);

	QPolygonF arrow;
	arrow << QPointF(0, -auvSize)
		<< QPointF(auvSize / 2, auvSize)
		<< QPointF(-auvSize / 2, auvSize);

	//根据连接状态和高亮状态设置颜色
	QColor bodyColor = data.isConnected ? Qt::blue : Qt::gray;
	if (data.isSelected)
	{
		bodyColor = Qt::red;//选中时变红
	}
	painter->setPen(QPen(Qt::darkGray, getElementSize(m_dFixLineWidth,1.5)));
	painter->setBrush(bodyColor);
	painter->drawPolygon(arrow);

	painter->restore();

	//4.绘制auv id标签
	painter->setPen(bodyColor);
	QFont infoFont("Arial", getElementSize(m_dFixTextSize,8.0));
	painter->setFont(infoFont);
	QString info = QString("%1 (%2 m/s)").arg(data.auvId).arg(data.speed);
	painter->drawText(screenPos + QPointF(auvSize,-auvSize), info);
}

void AuvRenderer::paintAllTrajectories(QPainter* painter)
{
	for (auto it = m_actualTrajMap.begin(); it != m_actualTrajMap.end(); ++it)
	{
		const QString& auvId = it.key();
		const auto& points = it.value();
		if (points.size() < 2)
			continue;

		//1.轨迹实线(蓝色实线，抗锯齿)
		QPen trajPen(Qt::blue, getElementSize(m_dFixLineWidth,1.0));
		trajPen.setCapStyle(Qt::RoundCap);
		painter->setPen(trajPen);

		QPainterPath trajPath;
		trajPath.moveTo(geoToScreen(points.first().coord));
		for (const auto& point : points)
		{
			trajPath.lineTo(geoToScreen(point.coord));
		}
		painter->drawPath(trajPath);

		//2.轨迹点(每隔10个点画一个，平衡细节与性能)
		double pointSize = getElementSize(FixTrajPointSize,2.0);
		double selectedSize = pointSize * 2;
		
		for (int i = 0; i < points.size(); i ++)
		{
			QPointF pos = geoToScreen(points[i].coord);
			bool isSelected = isTrajectoryPointSelected(auvId, i);
			//选中点：红色实心+黄色边框
			if (isSelected)
			{
				qDebug() << QString("轨迹点%1被选中").arg(i);
				painter->setBrush(Qt::red);
				painter->drawEllipse(pos, selectedSize, selectedSize);
				//外边框增强视觉效果
				/*painter->setBrush(Qt::NoBrush);
				painter->setPen(QPen(Qt::yellow, pointSize));
				painter->drawEllipse(pos, selectedSize + pointSize, selectedSize + pointSize);*/
			}
			else
			{
				//普通点
				painter->setBrush(Qt::blue);
				painter->drawEllipse(pos, pointSize, pointSize);
			}
			
		}
	}
}

void AuvRenderer::paintAllTargets(QPainter* painter)
{
	for (auto& target : m_targetMap)
	{
		QPointF screenPos = geoToScreen(target.coord);
		//目标尺寸
		double targetSize = getElementSize(FixTargetSize,8.0) * (0.5 + target.confidence * 0.5);

		painter->save();

		//1.选中光环
		if (target.isSelected)
		{
			painter->setPen(Qt::NoPen);
			painter->setBrush(QColor(255, 0, 0, 80));
			painter->drawEllipse(screenPos, targetSize * 2, targetSize * 2);
		}

		//2.目标图标
		painter->setPen(QPen(Qt::darkGray, getElementSize(m_dFixLineWidth,0.5)));
		if (target.type == "礁石")
		{
			drawRockIcon(painter, screenPos, targetSize);
		}
		else if (target.type == "沉船")
		{
			drawWreckIcon(painter, screenPos, targetSize);
		}
		else if (target.type == "生物群")
		{
			drawBiologicalIcon(painter, screenPos, targetSize);
		}
		else if (target.type == "障碍物")
		{
			drawObstacleIcon(painter, screenPos, targetSize);
		}
		else
		{
			drawDefaultIcon(painter, screenPos, targetSize);
		}

		//目标信息
		painter->setPen(Qt::darkRed);
		QFont infoFont("Arial", getElementSize(m_dFixTextSize,7.0));
		painter->setFont(infoFont);
		QString info = QString("%1(%2.0f)").arg(target.targetId).arg(target.confidence * 100);
		painter->drawText(screenPos + QPointF(targetSize, -targetSize), info);
		painter->restore();
	}
}

void AuvRenderer::paintAllMissionTrajectories(QPainter* painter)
{
	for (auto it = m_missionTrajMap.begin(); it != m_missionTrajMap.end(); ++it)
	{
		const QString& auvId = it.key();
		const auto& mission = it.value();
		if (!mission.isValid())
			continue;

		//绘制使命轨迹线
		QPen missionPen;
		if (mission.isActive)
		{
			double lineWidth = getElementSize(1.2, 1.2);
			missionPen = QPen(QColor(255, 106, 0),lineWidth );
			missionPen.setStyle(Qt::DashLine);
		}
		else
		{
			double lineWidth = getElementSize(1.0, 1.0);
			missionPen = QPen(Qt::darkGray, lineWidth);
			missionPen.setStyle(Qt::DotLine);
		}
		missionPen.setCapStyle(Qt::RoundCap);
		painter->setPen(missionPen);

		//连接所有航点
		QPainterPath missionPath;
		missionPath.moveTo(geoToScreen(mission.waypoints.first().coord));
		for (const auto& wp : mission.waypoints)
		{
			missionPath.lineTo(geoToScreen(wp.coord));
		}
		painter->drawPath(missionPath);

		//绘制航点
		paintMissionWaypoints(painter, mission,auvId);
	}
}

void AuvRenderer::paintMissionWaypoints(QPainter* painter, const MissionTrajectory& mission, const QString& auvId)
{
	double wpSize = getElementSize(FixMissionWpSize,4.0);
	bool isMissionActive = mission.isActive;

	for (int i =0;i<mission.waypoints.size();i++)
	{
		const auto& wp = mission.waypoints[i];
		QPointF wpPos = geoToScreen(wp.coord);
		bool isSelected = isMissionWaypointSelected(auvId, i);
		QColor color = getWaypointColor(wp.type, isMissionActive, wp.isSelected);

		//1.绘制到达半径(半透明圆环)
		painter->setPen(Qt::NoPen);
		painter->setBrush(QColor(color.red(), color.green(), color.blue(), 30));
		double radiusPixel = wp.radius * 1000.0 / m_mapCanvas->getDisplayScale();
		painter->drawEllipse(wpPos, radiusPixel, radiusPixel);



		//2.绘制航点核心（不同类型不同形状）
		painter->save();
		painter->translate(wpPos);
		double lineWidth = getElementSize(0.8, 0.8);
		painter->setPen(QPen(Qt::darkGray, lineWidth));
		if (isSelected)
		{
			color = color.lighter(150);
			painter->setBrush(color);
			painter->setPen(QPen(Qt::darkGray, lineWidth * 1.2));
			painter->drawPolygon(getWaypointShape(wp.type, wpSize * 1.5));
			painter->restore();
		}
		else
		{
			color = color.darker(120);
			painter->setBrush(color);
			painter->setPen(QPen(Qt::darkGray, lineWidth));
			painter->drawPolygon(getWaypointShape(wp.type, wpSize));
			painter->restore();
		}

		//3.绘制航点id和任务描述
		painter->setPen(color);
		QFont idFont("Arial", getElementSize (m_dFixTextSize,5.0));
		painter->setFont(idFont);
		painter->drawText(wpPos + QPointF(wpSize, -wpSize), wp.waypointId);

		//4.绘制任务描述（仅在比例尺足够大时显示）

	}
}

QPolygonF AuvRenderer::getWaypointShape(WaypointType type, double size)
{
	QPolygonF shape;
	switch (type)
	{
	case WaypointType::Start:
		//起点：圆形
		for (int i = 0; i < 12; i++)
		{
			double angle = 2 * M_PI * i / 12;
			shape << QPointF(size * cos(angle), size * sin(angle));
		}
		break;
	case WaypointType::Turning:
		//拐点：正方形
		shape << QPointF(-size, -size) << QPointF(size, -size)
			<< QPointF(size, size) << QPointF(-size, size);
		break;
	case WaypointType::End:
		//终点：三角形
		shape << QPointF(0, -size) << QPointF(size, size) << QPointF(-size, size);
		break;
	case WaypointType::Mission:
		//任务点：六边形
		for (int i = 0; i < 6; ++i)
		{
			double angle = 2 * M_PI * i / 6;
			shape << QPointF(size * cos(angle), size * sin(angle));
		}
		break;
	default:
		break;
	}
	return shape;
}

QColor AuvRenderer::getWaypointColor(WaypointType type, bool isActive, bool isSelected)
{
	QColor baseColor;
	switch (type)
	{
	case WaypointType::Start:
		baseColor = Qt::green;
		break;
	case WaypointType::Turning:
		baseColor = Qt::blue;
		break;
	case WaypointType::End:
		baseColor = Qt::red;
		break;
	case WaypointType::Mission:
		baseColor = Qt::magenta;
		break;
	default:
		break;
	}
	//激活状态调整(未激活则降低饱和度)
	if (!isActive)
	{
		baseColor = baseColor.darker(150);
	}
	//高亮状态调整(高亮则增加亮度)
	if (isSelected)
	{
		baseColor = baseColor.lighter(130);
	}
	return baseColor;
}

void AuvRenderer::drawRockIcon(QPainter* painter, QPointF pos, double size)
{
	QPolygonF triangle;
	triangle << pos + QPointF(0, -size)
		     << pos + QPointF(size, size)
		     << pos + QPointF(-size, size);

	painter->setBrush(QColor(139, 69, 19)); // 棕色
	painter->drawPolygon(triangle);
}

void AuvRenderer::drawWreckIcon(QPainter* painter, QPointF pos, double size)
{
	painter->setBrush(QColor(105, 105, 105));//灰色
	painter->drawRect(pos.x() +( -size / 2), pos.y()+(-size / 2), size, size); //船体
	painter->drawLine(pos.x() + (-size / 2), pos.y() + 0, size / 2, 0);//甲板线
}

void AuvRenderer::drawBiologicalIcon(QPainter* painter, QPointF pos, double size)
{
	painter->setBrush(QColor(0, 255, 127, 180));//浅绿色
	painter->drawEllipse(pos, size, size);

	//内部点缀
	painter->setBrush(QColor(0, 255, 0));
	painter->drawEllipse(pos + QPointF(-size / 3, -size / 3), size / 5, size / 5);
	painter->drawEllipse(pos + QPointF(size / 3, size / 3), size / 5, size / 5);
	painter->drawEllipse(pos + QPointF(0, size / 2), size / 5, size / 5);
}

void AuvRenderer::drawObstacleIcon(QPainter* painter, QPointF pos, double size)
{
	painter->setBrush(QColor(178, 34, 34));  //火红色
	double lineWidth = getElementSize(0.5, 0.5);
	painter->setPen(QPen(Qt::black, lineWidth));
	painter->drawRect(pos.x() + (-size / 2), pos.y() + (-size / 2), size, size);
}

void AuvRenderer::drawDefaultIcon(QPainter* painter, QPointF pos, double size)
{
	QPolygonF hexagon;
	for (int i = 0; i < 6; i++)
	{
		double angle = 2 * M_PI * i / 6;
		hexagon << pos + QPointF(size * cos(angle), size * sin(angle));
	}
	painter->setBrush(QColor(128, 0, 128, 180)); // 紫色
	painter->drawPolygon(hexagon);
}

bool AuvRenderer::isScreenVisible(const QPointF& screenPos) const
{
	if (!m_mapCanvas)
		return false;

	//获取视图可见范围
	QRectF viewportRect = m_mapCanvas->viewport()->rect();
	int margin = 10;
	QRectF visibleRect = viewportRect.adjusted(-margin, -margin, margin, margin);

	return visibleRect.contains(screenPos);
	
}

bool AuvRenderer::isGeoVisible(const GeoCoord& geo) const
{
	if (!geo.isValid())
		return false;

	QPointF screenPos = geoToScreen(geo);
	return isScreenVisible(screenPos);
}



/**
 * @brief 统一获取元素尺寸：支持固定尺寸/动态缩放切换
 * @param fixedSize 固定像素尺寸
 * @param baseSize  动态缩放的基准尺寸
 * @return 最终元素尺寸（像素）
*/
double AuvRenderer::getElementSize(double fixedSize, double baseSize) const
{
	if (m_bUseFixedSize)
	{
		//启用固定尺寸
		return fixedSize;
	}
	else
	{
		//禁用固定尺寸
		return scaledSize(baseSize);
	}
	
}


