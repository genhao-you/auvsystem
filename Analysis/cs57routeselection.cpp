#include "cs57routeselection.h"
#include "cpolylineclip.h"

using namespace Analysis;
CS57RouteSelection::CS57RouteSelection()
	: CS57ChartSelectionL()
{}

CS57RouteSelection::~CS57RouteSelection()
{}
void CS57RouteSelection::routeRegion(CS57Route* route)
{
	CS57McovrRegionProvider* provider = m_pDoc->getMcovrRegionProviderPtr();

	GpcPolygon* gpcI = provider->m_pResultGpcPolygonI;
	if (gpcI)
	{
		routeRegion(route, gpcI, m_vecRoutesI);
		translate2GeoResult(m_vecRoutesI);
		//QMessageBox::information(nullptr, "提示", "I：" + QString::number(m_vecRoutesI.size()));
	}

	GpcPolygon* gpcII = provider->m_pResultGpcPolygonII;
	if (gpcII)
	{
		routeRegion(route, gpcII, m_vecRoutesII);
		translate2GeoResult(m_vecRoutesII);
		//QMessageBox::information(nullptr, "提示", "II：" + QString::number(m_vecRoutesII.size()));
	}

	GpcPolygon* gpcIII = provider->m_pResultGpcPolygonIII;
	if (gpcIII)
	{
		routeRegion(route, gpcIII, m_vecRoutesIII);
		translate2GeoResult(m_vecRoutesIII);
		//QMessageBox::information(nullptr, "提示", "III：" + QString::number(m_vecRoutesIII.size()));
	}

	GpcPolygon* gpcIV = provider->m_pResultGpcPolygonIV;
	if (gpcIV)
	{
		routeRegion(route, gpcIV, m_vecRoutesIV);
		translate2GeoResult(m_vecRoutesIV);
		//QMessageBox::information(nullptr, "提示", "IV：" + QString::number(m_vecRoutesIV.size()));
	}

	GpcPolygon* gpcV = provider->m_pResultGpcPolygonV;
	if (gpcV)
	{
		routeRegion(route, gpcV, m_vecRoutesV);
		translate2GeoResult(m_vecRoutesV);
		//QMessageBox::information(nullptr, "提示", "V：" + QString::number(m_vecRoutesV.size()));
	}
}

void CS57RouteSelection::routeRegion(CS57Route* route, GpcPolygon* gpc, CPolylineFs& result)
{
	result.clear();
	CPolylineF geoRoute = route->m_vecGeoPolyline;//内部经纬度坐标

	CPolylineF vpRoute;//转换坐标到投影坐标
	for (int i = 0; i < geoRoute.size(); i++)
	{
		QPointF ptf = CS57Transform::instance()->geo2Vp(geoRoute[i]);
		vpRoute.push_back(ptf);
	}

	CPolylineFs vpOutPolygons;
	CPolylineFs vpInPolygons;
	QPainterPath path;
	int num_contours = gpc->m_gpcPolygon.num_contours;
	int *hole = gpc->m_gpcPolygon.hole;
	//获取所有外环
	for (int i = 0; i < num_contours; i++)
	{
		if (hole[i] == 0)
		{
			CPolylineF vpPolygon;
			int num_vertices = gpc->m_gpcPolygon.contour[i].num_vertices;
			for (int j = 0; j < num_vertices; j++)
			{
				QPointF ptf = CS57Transform::instance()->geo2Vp(gpc->m_gpcPolygon.contour[i].vertex[j].x,
					gpc->m_gpcPolygon.contour[i].vertex[j].y);
				vpPolygon.append(ptf);
			}
			vpOutPolygons.push_back(vpPolygon);
		}
	}
	//获取所有内环
	for (int i = 0; i < num_contours; i++)
	{
		if (hole[i] == 1)
		{
			CPolylineF vpPolygon;
			int num_vertices = gpc->m_gpcPolygon.contour[i].num_vertices;
			for (int j = 0; j < num_vertices; j++)
			{
				QPointF ptf = CS57Transform::instance()->geo2Vp(gpc->m_gpcPolygon.contour[i].vertex[j].x,
					gpc->m_gpcPolygon.contour[i].vertex[j].y);
				vpPolygon.append(ptf);
			}
			vpInPolygons.push_back(vpPolygon);
		}
	}

	//处理所有外环
	for (int i = 0; i < vpOutPolygons.size(); i++)
	{
		CPolylineClip clip;
		CPolylineFs in = clip.GetInterPolylines(vpRoute, vpOutPolygons[i], false);
		for (int j = 0; j < in.size(); j++)
		{
			result.push_back(in[j]);
		}
	}
	holeClip(vpInPolygons, result);
}

void CS57RouteSelection::holeClip(CPolylineFs& vpInPolygons, CPolylineFs& result)
{
	//处理所有内环
	CPolylineFs vecTempRoutes;
	bool needBreak = false;
	for (int i = 0; i < vpInPolygons.size(); i++)
	{
		for (int j = 0; j < result.size(); j++)
		{
			//首先判断内环是否与航线相交，不相交不用继续了
			CSpatialPolygon spatialPolygon;
			QVector<QVector<QPointF>*> vpRings;
			CPolylineF vpRing;
			for (int k = 0; k < vpInPolygons[i].size(); k++)
			{
				vpRing.push_back(vpInPolygons[i][k]);
			}
			vpRing.push_back(vpInPolygons[i][0]);
			vpRings.push_back(&vpRing);
			spatialPolygon.setPolygon(vpRings);
			if (!spatialPolygon.isIntersectPolyline(&result[j]))
			{
				spatialPolygon.relPolygon();
				continue;
			}

			CPolylineClip clip;
			CPolylineFs out = clip.GetInterPolylines(result[j], vpInPolygons[i], true);
			if (out.size() > 0)
			{
				vecTempRoutes.clear();
				for (int k = 0; k < result.size(); k++)
				{
					if (result[j] != result[k])
					{
						vecTempRoutes.push_back(result[k]);
					}
				}
				for (int k = 0; k < out.size(); k++)
				{
					vecTempRoutes.push_back(out[k]);
				}
				needBreak = true;
				break;
			}
		}
		if (vecTempRoutes.size() > 0)
		{
			result.clear();
			for (int j = 0; j < vecTempRoutes.size(); j++)
			{
				result.push_back(vecTempRoutes[j]);
			}
			vecTempRoutes.clear();
		}
	}
}

void CS57RouteSelection::startSelectI()
{
	if (m_vecRoutesI.size() == 0)
		return;

	for (int i = 0; i < m_vecRoutesI.size(); i++)
	{
		CS57ChartSelectionL::startSelect(&m_vecRoutesI[i]);
		for (int j = 0; j < m_vecSelectedMcovrs.size(); j++)
		{
			double cscl = m_vecSelectedMcovrs[j]->getCellCscl();
			if (cscl < 100000)
			{
				m_vecSelectedMcovrsI.push_back(m_vecSelectedMcovrs[j]);
			}
		}
		m_vecSelectedMcovrs.clear();
	}
	//排序
	qSort(m_vecSelectedMcovrsI.begin(), m_vecSelectedMcovrsI.end());
	//去重
	m_vecSelectedMcovrsI.erase(unique(m_vecSelectedMcovrsI.begin(), m_vecSelectedMcovrsI.end()), m_vecSelectedMcovrsI.end());
}

void CS57RouteSelection::startSelectII()
{
	if (m_vecRoutesII.size() == 0)
		return;

	for (int i = 0; i < m_vecRoutesII.size(); i++)
	{
		CS57ChartSelectionL::startSelect(&m_vecRoutesII[i]);
		for (int j = 0; j < m_vecSelectedMcovrs.size(); j++)
		{
			double cscl = m_vecSelectedMcovrs[j]->getCellCscl();
			if (cscl >= 100000 && cscl < 200000)
			{
				m_vecSelectedMcovrsII.push_back(m_vecSelectedMcovrs[j]);
			}
		}
		m_vecSelectedMcovrs.clear();
	}
	//排序
	qSort(m_vecSelectedMcovrsII.begin(), m_vecSelectedMcovrsII.end());
	//去重
	m_vecSelectedMcovrsII.erase(unique(m_vecSelectedMcovrsII.begin(), m_vecSelectedMcovrsII.end()), m_vecSelectedMcovrsII.end());
}

void CS57RouteSelection::startSelectIII()
{
	if (m_vecRoutesIII.size() == 0)
		return;

	for (int i = 0; i < m_vecRoutesIII.size(); i++)
	{
		CS57ChartSelectionL::startSelect(&m_vecRoutesIII[i]);
		for (int j = 0; j < m_vecSelectedMcovrs.size(); j++)
		{
			double cscl = m_vecSelectedMcovrs[j]->getCellCscl();
			if (cscl >= 200000 && cscl < 1000000)
			{
				m_vecSelectedMcovrsIII.push_back(m_vecSelectedMcovrs[j]);
			}
		}
		m_vecSelectedMcovrs.clear();
	}
	//排序
	qSort(m_vecSelectedMcovrsIII.begin(), m_vecSelectedMcovrsIII.end());
	//去重
	m_vecSelectedMcovrsIII.erase(unique(m_vecSelectedMcovrsIII.begin(), m_vecSelectedMcovrsIII.end()), m_vecSelectedMcovrsIII.end());
}

void CS57RouteSelection::startSelectIV()
{
	if (m_vecRoutesIV.size() == 0)
		return;

	for (int i = 0; i < m_vecRoutesIV.size(); i++)
	{
		CS57ChartSelectionL::startSelect(&m_vecRoutesIV[i]);
		for (int j = 0; j < m_vecSelectedMcovrs.size(); j++)
		{
			double cscl = m_vecSelectedMcovrs[j]->getCellCscl();
			if (cscl >= 1000000 && cscl < 3000000)
			{
				m_vecSelectedMcovrsIV.push_back(m_vecSelectedMcovrs[j]);
			}
		}
		m_vecSelectedMcovrs.clear();
	}
	//排序
	qSort(m_vecSelectedMcovrsIV.begin(), m_vecSelectedMcovrsIV.end());
	//去重
	m_vecSelectedMcovrsIV.erase(unique(m_vecSelectedMcovrsIV.begin(), m_vecSelectedMcovrsIV.end()), m_vecSelectedMcovrsIV.end());
}

void CS57RouteSelection::startSelectV()
{
	if (m_vecRoutesV.size() == 0)
		return;

	for (int i = 0; i < m_vecRoutesV.size(); i++)
	{
		CS57ChartSelectionL::startSelect(&m_vecRoutesV[i]);
		for (int j = 0; j < m_vecSelectedMcovrs.size(); j++)
		{
			double cscl = m_vecSelectedMcovrs[j]->getCellCscl();
			if (cscl >= 3000000)
			{
				m_vecSelectedMcovrsV.push_back(m_vecSelectedMcovrs[j]);
			}
		}
		m_vecSelectedMcovrs.clear();
	}
	//排序
	qSort(m_vecSelectedMcovrsV.begin(), m_vecSelectedMcovrsV.end());
	//去重
	m_vecSelectedMcovrsV.erase(unique(m_vecSelectedMcovrsV.begin(), m_vecSelectedMcovrsV.end()), m_vecSelectedMcovrsV.end());
}

void CS57RouteSelection::startSelect()
{
	startSelectI();
	startSelectII();
	startSelectIII();
	startSelectIV();
	startSelectV();

	m_vecSelectedMcovrs.clear();
	for (int i = 0; i < m_vecSelectedMcovrsI.size(); i++)
	{
		m_vecSelectedMcovrs.push_back(m_vecSelectedMcovrsI[i]);
	}

	for (int i = 0; i < m_vecSelectedMcovrsII.size(); i++)
	{
		m_vecSelectedMcovrs.push_back(m_vecSelectedMcovrsII[i]);
	}

	for (int i = 0; i < m_vecSelectedMcovrsIII.size(); i++)
	{
		m_vecSelectedMcovrs.push_back(m_vecSelectedMcovrsIII[i]);
	}

	for (int i = 0; i < m_vecSelectedMcovrsIV.size(); i++)
	{
		m_vecSelectedMcovrs.push_back(m_vecSelectedMcovrsIV[i]);
	}

	for (int i = 0; i < m_vecSelectedMcovrsV.size(); i++)
	{
		m_vecSelectedMcovrs.push_back(m_vecSelectedMcovrsV[i]);
	}

	//排序
	qSort(m_vecSelectedMcovrs.begin(), m_vecSelectedMcovrs.end());
	//去重
	m_vecSelectedMcovrs.erase(unique(m_vecSelectedMcovrs.begin(), m_vecSelectedMcovrs.end()), m_vecSelectedMcovrs.end());
}

void CS57RouteSelection::translate2GeoResult(CPolylineFs& vecRoutes)
{
	for (int i = 0; i < vecRoutes.size(); i++)
	{
		for (int j = 0; j < vecRoutes[i].size(); j++)
		{
			QPointF ptf = CS57Transform::instance()->vp2Geo(vecRoutes[i][j]);
			vecRoutes[i][j] = ptf;
		}
	}
}

CPolylineFs* CS57RouteSelection::getRoutePtsI()
{
	return &m_vecRoutesI;
}

CPolylineFs* CS57RouteSelection::getRoutePtsII()
{
	return &m_vecRoutesII;
}

CPolylineFs* CS57RouteSelection::getRoutePtsIII()
{
	return &m_vecRoutesIII;
}

CPolylineFs* CS57RouteSelection::getRoutePtsIV()
{
	return &m_vecRoutesIV;
}

CPolylineFs* CS57RouteSelection::getRoutePtsV()
{
	return &m_vecRoutesV;
}
