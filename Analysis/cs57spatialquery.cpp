#include "stdafx.h"
#include "cs57spatialquery.h"
#include "cs57recfeature.h"
#include "cs57recvector.h"
#include "cs57cootranshelper.h"
#include <QTextCodec>

using namespace Analysis;
#pragma execution_character_set("utf-8")
CS57SpatialQuery::CS57SpatialQuery()
	: m_pFeatureQuery(new CS57FeatureQuery())
{}

CS57SpatialQuery::~CS57SpatialQuery()
{
	if (m_pFeatureQuery != nullptr)
		delete m_pFeatureQuery;
	m_pFeatureQuery = nullptr;
}

void CS57SpatialQuery::setProj(CS57Projection* proj)
{
	m_pFeatureQuery->setProj(proj);
}

void CS57SpatialQuery::setViewport(CS57Viewport* viewport)
{
	m_pFeatureQuery->setViewport(viewport);
}

void CS57SpatialQuery::setProvider(CS57CellProvider* provider)
{
	m_pFeatureQuery->setProvider(provider);
}

void CS57SpatialQuery::setPresLib(CS57PresLib* prelib)
{
	m_pFeatureQuery->setPresLib(prelib);
}

void CS57SpatialQuery::startHighlight()
{
	m_pFeatureQuery->startHighlight();
}

void CS57SpatialQuery::stopHighlight()
{
	m_pFeatureQuery->stopHighlight();
}

void Analysis::CS57SpatialQuery::closeFeatureQuery()
{
	m_pFeatureQuery->closeFeatureQuery();
}

void CS57SpatialQuery::setHighlightRow(int row)
{
	m_pFeatureQuery->setHighlightRow(row);
}

void CS57SpatialQuery::queryFeatureInfo(XT::QueryMode mode, QPoint pt)
{
	m_pFeatureQuery->queryFeatureInfo(mode,pt);
}

void CS57SpatialQuery::releaseHighlightFeature()
{
	m_pFeatureQuery->releaseHighlightFeature();
}

CS57RecFeature* CS57SpatialQuery::getHighlightFeature() const
{
	return m_pFeatureQuery->getHighlightFeature();
}


QMultiMap<QString, QVector<QString>>* CS57SpatialQuery::getObjAttributesMapPtr()
{
	return m_pFeatureQuery->getObjAttributesMapPtr();
}

CS57FeatureQuery::CS57FeatureQuery()
	: m_pPresLib(nullptr)
	, m_pProjection(nullptr)
	, m_pViewport(nullptr)
	, m_bHighlight(false)
	, m_pHighlightFeature(nullptr)
	, m_pProvider(nullptr)
{}

CS57FeatureQuery::~CS57FeatureQuery()
{}
void CS57FeatureQuery::setProj(CS57Projection* proj)
{
	m_pProjection = proj;
	m_CooTransHelper.setProj(proj);
}

void CS57FeatureQuery::setViewport(CS57Viewport* viewport)
{
	m_pViewport = viewport;
	m_CooTransHelper.setViewport(viewport);
}

void CS57FeatureQuery::queryFeatureInfo(XT::QueryMode mode, QPoint pt)
{
	if (!m_pProvider)	
		return;

	m_vecQueryResult.clear();
	switch (mode)
	{
	case XT::NO_QUERY:
		break;
	case XT::P_QUERY:
		queryPointInfo(pt);
		break;
	case XT::L_QUERY:
		queryLineInfo(pt);
		break;
	case XT::A_QUERY:
		queryAreaInfo(pt);
		break;
	}
	genDisplayContext();
}

void CS57FeatureQuery::queryPointInfo(QPoint pt)
{
	double lon, lat;
	for (int i = 0; i < m_pProvider->m_vecCellDisplayContext.size(); i++)
	{
		CS57Cell* pCell = m_pProvider->m_vecCellDisplayContext[i]->pS57Cell;
		double comf = pCell->getCellComf();

		double dis = (double)5 / (double)1000 * (double)pCell->getCellCscl();
		QPointF vpt = m_CooTransHelper.pixel2Vp(pt.x(), pt.y());
		double l, t, r, b;
		l = vpt.x() - dis;
		r = vpt.x() + dis;
		t = vpt.y() + dis;
		b = vpt.y() - dis;

		for (int j = 0; j < m_pProvider->m_vecCellDisplayContext[i]->vecPFeatures.size(); j++)
		{
			CS57RecFeature* pFE = m_pProvider->m_vecCellDisplayContext[i]->vecPFeatures[j];
			if (pFE->m_Frid.objl == 129)
			{
				for (int k = 0;k < pCell->m_vecS57RecVI[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg3d.m_vecSg3d.size();k++)
				{
					lat = pCell->m_vecS57RecVI[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg3d.m_vecSg3d[k].ycoo / comf / RO;
					lon = pCell->m_vecS57RecVI[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg3d.m_vecSg3d[k].xcoo / comf / RO;
					QPointF vpt = m_CooTransHelper.bl2Vp(lon, lat);

					if (vpt.x() > l && vpt.x() < r && vpt.y() > b && vpt.y() < t)
					{
						m_vecQueryResult.push_back(pFE);
					}
				}
				continue;
			}

			if (pFE->m_Fspt.m_vecFspt[0].rcnm == 110)
			{
				lat = pCell->m_vecS57RecVI[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d[0].ycoo / comf / RO;
				lon = pCell->m_vecS57RecVI[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d[0].xcoo / comf / RO;
			}
			else if (pFE->m_Fspt.m_vecFspt[0].rcnm == 120)
			{
				lat = pCell->m_vecS57RecVC[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d[0].ycoo / comf / RO;
				lon = pCell->m_vecS57RecVC[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d[0].xcoo / comf / RO;
			}
			else
				continue;
			QPointF vpt = m_CooTransHelper.bl2Vp(lon, lat);
			if (vpt.x() > l && vpt.x() < r && vpt.y() > b && vpt.y() < t)
				m_vecQueryResult.push_back(pFE);
		}
	}
	//排序
	qSort(m_vecQueryResult.begin(), m_vecQueryResult.end());
	//去重
	m_vecQueryResult.erase(unique(m_vecQueryResult.begin(), m_vecQueryResult.end()),m_vecQueryResult.end());
}

void CS57FeatureQuery::queryLineInfo(QPoint pt)
{
	for (int i = 0; i < m_pProvider->m_vecCellDisplayContext.size(); i++)
	{
		CS57Cell* pCell = m_pProvider->m_vecCellDisplayContext[i]->pS57Cell;
		double dis = (double)5 / (double)1000 * (double)pCell->getCellCscl();
		QPointF vpt = m_CooTransHelper.pixel2Vp(pt.x(), pt.y());
		double l, t, r, b;
		l = vpt.x() - dis;
		r = vpt.x() + dis;
		t = vpt.y() + dis;
		b = vpt.y() - dis;
		QVector<QVector<QPointF>*> vecRings;
		QVector<QPointF>* pvecRing = new QVector<QPointF>();
		QPointF pt(l, t);
		QPointF pt1(l, b);
		QPointF pt2(r, b);
		QPointF pt3(r, t);
		QPointF geoPt = m_CooTransHelper.vp2Geo(pt);
		QPointF geoPt1 = m_CooTransHelper.vp2Geo(pt1);
		QPointF geoPt2 = m_CooTransHelper.vp2Geo(pt2);
		QPointF geoPt3 = m_CooTransHelper.vp2Geo(pt3);
		pvecRing->push_back(geoPt);
		pvecRing->push_back(geoPt1);
		pvecRing->push_back(geoPt2);
		pvecRing->push_back(geoPt3);
		pvecRing->push_back(geoPt);
		vecRings.push_back(pvecRing);
		CS57FilterPolygon filterPolygon(vecRings);

		for (int j = 0; j < m_pProvider->m_vecCellDisplayContext[i]->vecLFeatures.size(); j++)
		{
			CS57RecFeature* pFE = m_pProvider->m_vecCellDisplayContext[i]->vecLFeatures[j];

			QVector<QVector<QPointF>*> vecPolylines;
			pCell->genFeaturePolyline(pFE, &vecPolylines);
			if (filterPolygon.isIntersectPolyline(&vecPolylines))
			{
				m_vecQueryResult.push_back(pFE);
			}
			qDeleteAll(vecPolylines);
			vecPolylines.clear();
		}
		//面中线
		for (int j = 0; j < m_pProvider->m_vecCellDisplayContext[i]->vecAFeatures.size(); j++)
		{
			CS57RecFeature* pFE = m_pProvider->m_vecCellDisplayContext[i]->vecAFeatures[j];

			QVector<QVector<QPointF>*> vecPolylines;
			pCell->genFeaturePolyline(pFE, &vecPolylines);
			if (filterPolygon.isIntersectPolyline(&vecPolylines))
			{
				m_vecQueryResult.push_back(pFE);
			}
			qDeleteAll(vecPolylines);
			vecPolylines.clear();
		}

		qDeleteAll(vecRings);
		vecRings.clear();
	}

	//排序
	qSort(m_vecQueryResult.begin(), m_vecQueryResult.end());
	//去重
	m_vecQueryResult.erase(unique(m_vecQueryResult.begin(), m_vecQueryResult.end()),m_vecQueryResult.end());
}

void CS57FeatureQuery::queryAreaInfo(QPoint pt)
{
	for (int i = 0; i < m_pProvider->m_vecCellDisplayContext.size(); i++)
	{
		CS57Cell* pCell = m_pProvider->m_vecCellDisplayContext[i]->pS57Cell;

		for (int j = 0; j < m_pProvider->m_vecCellDisplayContext[i]->vecAFeatures.size(); j++)
		{
			CS57RecFeature* pFE = m_pProvider->m_vecCellDisplayContext[i]->vecAFeatures[j];
			if (pFE->m_Frid.objl == 306 ||	//航标系统
				pFE->m_Frid.objl == 308 ||	//数据质量
				pFE->m_Frid.objl == 81  ||	//磁差
				pFE->m_Frid.objl == 302)	//地理范围
				continue;

			// 获取要素多边形数据
			QVector<QVector<QPointF>*> vecRings;
			pCell->genFeaturePolygon(pFE, &vecRings);
			CS57FilterPolygon featurePolygon(vecRings);
			QPointF geoPt = m_CooTransHelper.pixel2Geo(pt);
			if (featurePolygon.ptInPolygon(geoPt.x(), geoPt.y()))
			{
				m_vecQueryResult.push_back(pFE);
			}
			qDeleteAll(vecRings);
			vecRings.clear();
		}
	}

	//排序
	qSort(m_vecQueryResult.begin(), m_vecQueryResult.end());
	//去重
	m_vecQueryResult.erase(
		unique(m_vecQueryResult.begin(), m_vecQueryResult.end()),
		m_vecQueryResult.end());
}
void CS57FeatureQuery::setProvider(CS57CellProvider* provider)
{
	m_pProvider = provider;
}


void CS57FeatureQuery::setPresLib(CS57PresLib* prelib)
{
	m_pPresLib = prelib;
}

void CS57FeatureQuery::genDisplayContext()
{
	m_mulmapObjAttributes.clear();
	int highlightId = 0;
	QVector<CS57RecFeature*>::iterator it = m_vecQueryResult.begin();
	for (;it != m_vecQueryResult.end(); it++)
	{
		int objl = (*it)->m_Frid.objl;
		QString strCName = m_pPresLib->getFeatureInfo(objl)->strFeaObjectCname;

		QVector<QString> vecAttributes;
		QString strResult,strName, strValue;
		if ((*it)->m_pCell != nullptr)
		{
			strName = "所属图幅";	
			strValue = (*it)->m_pCell->getCellName();
			strResult = strName + "|" + strValue;
			vecAttributes.push_back(strResult);

			strName = "要素名称";
			strValue = strCName;
			strResult = strName + "|" + strValue;
			vecAttributes.push_back(strResult);

			strName = "要素编码";
			strValue = m_pPresLib->getFeatureInfo(objl)->strFeaObjectAcronym;
			strResult = strName + "|" + strValue;
			vecAttributes.push_back(strResult);

			strName = "要素代码";
			strValue = QString::number(m_pPresLib->getFeatureInfo(objl)->nFeaObjectCode);
			strResult = strName + "|" + strValue;
			vecAttributes.push_back(strResult);

			strName = "几何特征";
			if ((*it)->m_Frid.prim == 1)		strValue = "点";
			else if ((*it)->m_Frid.prim == 2) 	strValue = "线";
			else if ((*it)->m_Frid.prim == 3) 	strValue = "面";
			strResult = strName + "|" + strValue;
			vecAttributes.push_back(strResult);

			strName = "标识号";
			strValue = QString::number((*it)->m_Frid.rcid);
			strResult = strName + "|" + strValue;
			vecAttributes.push_back(strResult);

			int size = (*it)->m_Attf.m_vecAttf.size();
			for (int i = 0; i < size; i++)
			{
				strName = "---(分割线)---";
				strValue = "---(分割线)---";
				strResult = strName + "|" + strValue;
				vecAttributes.push_back(strResult);

				int attl = (*it)->m_Attf.m_vecAttf[i].attl;
				strName = "属性名称";
				strValue = m_pPresLib->getAttributeInfo(attl)->strCName;
				strResult = strName + "|" + strValue;
				vecAttributes.push_back(strResult);

				strName = "属性编码";
				strValue = m_pPresLib->getAttributeInfo(attl)->strFeaAttrAcronym;
				strResult = strName + "|" + strValue;
				vecAttributes.push_back(strResult);

				strName = "属性代码";
				strValue = QString::number(m_pPresLib->getAttributeInfo(attl)->nFeaAttrCode);
				strResult = strName + "|" + strValue;
				vecAttributes.push_back(strResult);

				strName = "属性值";
				strValue = (*it)->m_Attf.m_vecAttf[i].atvl;
				strResult = strName + "|" + strValue;
				vecAttributes.push_back(strResult);

				QStringList lstValues = strValue.split(",");
				for (int j = 0; j < lstValues.size(); j++)
				{
					unsigned short expertValue = lstValues[0].toUShort();
					QString cName, eName;
					if (m_pPresLib->getAttrExpInfo(m_pPresLib->getAttributeInfo(attl)->nFeaAttrCode, expertValue, cName, eName))
					{
						if (cName != "")
						{
							strName = "中文说明";
							strValue = QString::number(expertValue) + "," + cName;
							strResult = strName + "|" + strValue;
							vecAttributes.push_back(strResult);
						}
						if (eName != "")
						{
							strName = "英文说明";
							strValue = QString::number(expertValue) + "," + eName;
							strResult = strName + "|" + strValue;
							vecAttributes.push_back(strResult);
						}
					}
				}
			}

			size = (*it)->m_Natf.m_vecNatf.size();
			for (int i = 0; i < size; i++)
			{
				strName = "---(分割线)---";
				strValue = "---(分割线)---";
				strResult = strName + "|" + strValue;
				vecAttributes.push_back(strResult);

				int attl = (*it)->m_Natf.m_vecNatf[i].attl;
				strName = "属性名称";
				strValue = m_pPresLib->getAttributeInfo(attl)->strCName;
				strResult = strName + "|" + strValue;
				vecAttributes.push_back(strResult);

				strName = "属性编码";
				strValue = m_pPresLib->getAttributeInfo(attl)->strFeaAttrAcronym;
				strResult = strName + "|" + strValue;
				vecAttributes.push_back(strResult);

				strName = "属性代码";
				strValue = QString::number(m_pPresLib->getAttributeInfo(attl)->nFeaAttrCode);
				strResult = strName + "|" + strValue;
				vecAttributes.push_back(strResult);

				strName = "属性值";
				QTextCodec* codec_16 = QTextCodec::codecForName("UTF-16");
				strValue = codec_16->toUnicode((*it)->m_Natf.m_vecNatf[i].atvl);
				strResult = strName + "|" + strValue;
				vecAttributes.push_back(strResult);

				QStringList lstValues = strValue.split(",");
				for (int j = 0; j < lstValues.size(); j++)
				{
					unsigned short expertValue = lstValues[0].toUShort();
					QString cName, eName;
					if (m_pPresLib->getAttrExpInfo(m_pPresLib->getAttributeInfo(attl)->nFeaAttrCode, expertValue, cName, eName))
					{
						if (cName != "")
						{
							strName = "中文说明";
							strValue = QString::number(expertValue) + "," + cName;
							strResult = strName + "|" + strValue;
							vecAttributes.push_back(strResult);
						}
						if (eName != "")
						{
							strName = "英文说明";
							strValue = QString::number(expertValue) + "," + eName;
							strResult = strName + "|" + strValue;
							vecAttributes.push_back(strResult);
						}
					}
				}
			}
		}
		QString strKey = strCName + " " + QString::number(highlightId);
		m_mulmapObjAttributes.insert(strKey, vecAttributes);
		highlightId++;
	}
}

QMultiMap<QString, QVector<QString>>* CS57FeatureQuery::getObjAttributesMapPtr()
{
	return &m_mulmapObjAttributes;
}

void CS57FeatureQuery::startHighlight()
{
	m_bHighlight = true;
}

void CS57FeatureQuery::stopHighlight()
{
	m_bHighlight = false;
}

void Analysis::CS57FeatureQuery::closeFeatureQuery()
{
	m_vecQueryResult.clear();
	m_pProvider = nullptr;
	m_pHighlightFeature = nullptr;
	m_bHighlight = false;
}

void Analysis::CS57FeatureQuery::releaseHighlightFeature()
{
	delete m_pHighlightFeature;
	m_pHighlightFeature = nullptr;
}

void CS57FeatureQuery::setHighlightRow(int row)
{
	m_pHighlightFeature = nullptr;
	if (!m_bHighlight)
		return;

	if (m_vecQueryResult.size() == 0 || row < 0)
		m_pHighlightFeature = nullptr;

	m_pHighlightFeature = m_vecQueryResult[row];
}

CS57RecFeature* CS57FeatureQuery::getHighlightFeature() const
{
	return m_pHighlightFeature;
}