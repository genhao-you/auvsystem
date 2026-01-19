#include "missionmonitorresult.h"

MissionMonitorResult::MissionMonitorResult(MissionMonitorResultType resultType, QString auvId)
	:BaseResult(ModuleType::Monitoring, ResultStatus::Success),
	m_resultType(resultType),
	m_sAuvId (auvId)
{

}

MissionMonitorResult::~MissionMonitorResult()
{}

MissionMonitorResultType MissionMonitorResult::resultType() const
{
	return m_resultType;
}

void MissionMonitorResult::setResultType(MissionMonitorResultType type)
{
	m_resultType = type;
}

void MissionMonitorResult::setAuvId(QString id)
{
	m_sAuvId = id;
}

QString MissionMonitorResult::auvId() const
{
	return m_sAuvId;
}

void MissionMonitorResult::setRealTimeData(const AuvRealTimeData& coord)
{
	m_realTimeData = coord;
}

AuvRealTimeData MissionMonitorResult::realTimeData() const
{
	return m_realTimeData;
}

void MissionMonitorResult::addHistoricalPoint(const GeoCoord& point)
{
	m_historicalTrail.append(point);
}

QList<GeoCoord> MissionMonitorResult::historicalTrail() const
{
	return m_historicalTrail;
}

void MissionMonitorResult::clearHistoricalTrail()
{
	m_historicalTrail.clear();
}

void MissionMonitorResult::setPlannedWaypoints(const QList<MissionWaypoint>& waypoints)
{
	m_plannedWaypoints = waypoints;
}

QList<MissionWaypoint> MissionMonitorResult::plannedWaypoins() const
{
	return m_plannedWaypoints;
}

void MissionMonitorResult::addEnvironmentTarget(const DetectedTarget& target)
{
	m_environmentTargets.append(target);
}

QList<DetectedTarget> MissionMonitorResult::environmentTargets() const
{
	return m_environmentTargets;
}

void MissionMonitorResult::removeEnvironmentTarget(int targetId)
{
	for (auto it = m_environmentTargets.begin(); it != m_environmentTargets.end(); ++it)
	{
		if (it->targetId == targetId)
		{
			m_environmentTargets.erase(it);
			break;
		}
	}
}

QString MissionMonitorResult::description() const
{
	switch (m_resultType)
	{
	case MissionMonitorResultType::RealTimePosition:
		return QString("AUV%1实时定位：纬度%2，经度%3")
			.arg(m_sAuvId)
			.arg(m_realTimeData.geoPos.lat , 0, 'f', 6)
			.arg(m_realTimeData.geoPos.lon, 0, 'f', 6);
	case MissionMonitorResultType::HistoricalTrail:
		return QString("AUV%1历史轨迹更新，累计%2个点")
			.arg(m_sAuvId)
			.arg(m_historicalTrail.size());
	case MissionMonitorResultType::PlannedTrail:
		return QString("使命计划轨迹更新，含%1个航点")
			.arg(m_plannedWaypoints.size());
	case MissionMonitorResultType::DetectedTarget:
		return QString("环境态势更新，探测到%1个目标")
			.arg(m_environmentTargets.size());
	default:
		return QString("未知使命监控结果");
		break;
	}
	return QString();
}
