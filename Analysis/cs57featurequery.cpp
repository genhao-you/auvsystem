#include "stdafx.h"
#include "cs57featurequery.h"
#include "cs57recfeature.h"
#include "cs57recvector.h"
#include "cs57transform.h"
#include <QTextCodec>

using namespace Analysis;
#pragma execution_character_set("utf-8")

CS57FeatureQuery::CS57FeatureQuery()
	: m_pPresLib(nullptr)
	, m_bHighlight(false)
	, m_pHighlightFeature(nullptr)
	, m_pDoc(nullptr)
	, m_pTransform(CS57Transform::instance())
{}

CS57FeatureQuery::~CS57FeatureQuery()
{}

void CS57FeatureQuery::queryMouse(XT::QueryMode mode, QPoint pt)
{
	CS57CellProvider* pProvider = m_pDoc->getCellProviderPtr();
	if (!pProvider)	
		return;

	m_mapQueryResult.clear();
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
	CS57CellProvider* pProvider = m_pDoc->getCellProviderPtr();
	for (int i = 0; i < pProvider->m_vecCellDisplayContext.size(); i++)
	{
		CS57Cell* pCell = pProvider->m_vecCellDisplayContext[i]->pS57Cell;
		double comf = pCell->getCellComf();

		double dis = (double)5 / (double)1000 * (double)pCell->getCellCscl();
		QPointF vpt = m_pTransform->pixel2Vp(pt.x(), pt.y());
		double l, t, r, b;
		l = vpt.x() - dis;
		r = vpt.x() + dis;
		t = vpt.y() + dis;
		b = vpt.y() - dis;

		for (int j = 0; j < pProvider->m_vecCellDisplayContext[i]->vecPFeatures.size(); j++)
		{
			CS57RecFeature* pFE = pProvider->m_vecCellDisplayContext[i]->vecPFeatures[j];
			if (pFE->m_Frid.objl == 129)
			{
				for (int k = 0;k < pCell->m_vecS57RecVI[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg3d.m_vecSg3d.size();k++)
				{
					lat = pCell->m_vecS57RecVI[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg3d.m_vecSg3d[k].ycoo / comf / RO;
					lon = pCell->m_vecS57RecVI[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg3d.m_vecSg3d[k].xcoo / comf / RO;
					QPointF vpt = m_pTransform->bl2Vp(lon, lat);

					if (vpt.x() > l && vpt.x() < r && vpt.y() > b && vpt.y() < t)
					{
						m_mapQueryResult.insert(pFE->m_Frid.rcid, pFE);
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
			QPointF vpt = m_pTransform->bl2Vp(lon, lat);
			if (vpt.x() > l && vpt.x() < r && vpt.y() > b && vpt.y() < t)
				m_mapQueryResult.insert(pFE->m_Frid.rcid, pFE);
		}
	}
}

void CS57FeatureQuery::queryLineInfo(QPoint pt)
{
	CS57CellProvider* pProvider = m_pDoc->getCellProviderPtr();
	for (int i = 0; i < pProvider->m_vecCellDisplayContext.size(); i++)
	{
		CS57Cell* pCell = pProvider->m_vecCellDisplayContext[i]->pS57Cell;
		double dis = (double)5 / (double)1000 * (double)pCell->getCellCscl();
		QPointF vpt = m_pTransform->pixel2Vp(pt.x(), pt.y());
		double l, t, r, b;
		l = vpt.x() - dis;
		r = vpt.x() + dis;
		t = vpt.y() + dis;
		b = vpt.y() - dis;
		QVector<QVector<QPointF>*> vecRings;
		CPolylineF* pvecRing = new CPolylineF();
		QPointF pt(l, t);
		QPointF pt1(l, b);
		QPointF pt2(r, b);
		QPointF pt3(r, t);
		QPointF geoPt = m_pTransform->vp2Geo(pt);
		QPointF geoPt1 = m_pTransform->vp2Geo(pt1);
		QPointF geoPt2 = m_pTransform->vp2Geo(pt2);
		QPointF geoPt3 = m_pTransform->vp2Geo(pt3);
		pvecRing->push_back(geoPt);
		pvecRing->push_back(geoPt1);
		pvecRing->push_back(geoPt2);
		pvecRing->push_back(geoPt3);
		pvecRing->push_back(geoPt);
		vecRings.push_back(pvecRing);
		CSpatialPolygon filterPolygon(vecRings);

		for (int j = 0; j < pProvider->m_vecCellDisplayContext[i]->vecLFeatures.size(); j++)
		{
			CS57RecFeature* pFE = pProvider->m_vecCellDisplayContext[i]->vecLFeatures[j];

			QVector<QVector<QPointF>*> vecPolylines;
			pCell->genFeaturePolyline(pFE, &vecPolylines);
			if (filterPolygon.isIntersectPolyline(&vecPolylines))
			{
				m_mapQueryResult.insert(pFE->m_Frid.rcid, pFE);
			}
			qDeleteAll(vecPolylines);
			vecPolylines.clear();
		}
		//面中线
		for (int j = 0; j < pProvider->m_vecCellDisplayContext[i]->vecAFeatures.size(); j++)
		{
			CS57RecFeature* pFE = pProvider->m_vecCellDisplayContext[i]->vecAFeatures[j];

			QVector<QVector<QPointF>*> vecPolylines;
			pCell->genFeaturePolyline(pFE, &vecPolylines);
			if (filterPolygon.isIntersectPolyline(&vecPolylines))
			{
				m_mapQueryResult.insert(pFE->m_Frid.rcid, pFE);
			}
			qDeleteAll(vecPolylines);
			vecPolylines.clear();
		}

		qDeleteAll(vecRings);
		vecRings.clear();
	}
}

void CS57FeatureQuery::queryAreaInfo(QPoint pt)
{
	CS57CellProvider* pProvider = m_pDoc->getCellProviderPtr();
	for (int i = 0; i < pProvider->m_vecCellDisplayContext.size(); i++)
	{
		CS57Cell* pCell = pProvider->m_vecCellDisplayContext[i]->pS57Cell;

		for (int j = 0; j < pProvider->m_vecCellDisplayContext[i]->vecAFeatures.size(); j++)
		{
			CS57RecFeature* pFE = pProvider->m_vecCellDisplayContext[i]->vecAFeatures[j];
			if (pFE->m_Frid.objl == 306 ||	//航标系统
				pFE->m_Frid.objl == 308 ||	//数据质量
				pFE->m_Frid.objl == 81  ||	//磁差
				pFE->m_Frid.objl == 302)	//地理范围
				continue;

			// 获取要素多边形数据
			QVector<QVector<QPointF>*> vecRings;
			pCell->genFeaturePolygon(pFE, &vecRings);
			CSpatialPolygon featurePolygon(vecRings);
			QPointF geoPt = m_pTransform->pixel2Geo(pt);
			if (featurePolygon.ptInPolygon(geoPt.x(), geoPt.y()))
			{
				m_mapQueryResult.insert(pFE->m_Frid.rcid, pFE);
			}
			qDeleteAll(vecRings);
			vecRings.clear();
		}
	}
}
void CS57FeatureQuery::setDoc(CS57DocManager* doc)
{
	m_pDoc = doc;
}
void CS57FeatureQuery::setPresLib(CS57PresLib* prelib)
{
	m_pPresLib = prelib;
}

void CS57FeatureQuery::genDisplayContext()
{
	m_mulmapObjAttributes.clear();
	int highlightId = 0;
	QMap<int, CS57RecFeature*>::iterator it = m_mapQueryResult.begin();
	for (; it != m_mapQueryResult.end(); it++)
	{
		int objl = it.value()->m_Frid.objl;
		QString strCName = m_pPresLib->getFeatureInfo(objl)->strFeaObjectCname;

		QVector<QString> vecAttributes;
		QString strResult, strName, strValue;
		if (it.value()->m_pCell != nullptr)
		{
			strName = "所属图幅";
			strValue = it.value()->m_pCell->getCellNameWithoutSuffix();
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
			if (it.value()->m_Frid.prim == 1)		strValue = "点";
			else if (it.value()->m_Frid.prim == 2) 	strValue = "线";
			else if (it.value()->m_Frid.prim == 3) 	strValue = "面";
			strResult = strName + "|" + strValue;
			vecAttributes.push_back(strResult);

			strName = "标识号";
			strValue = QString::number(it.value()->m_Frid.rcid);
			strResult = strName + "|" + strValue;
			vecAttributes.push_back(strResult);

			int size = it.value()->m_Attf.m_vecAttf.size();
			for (int i = 0; i < size; i++)
			{
				strName = "---(分割线)---";
				strValue = "---(分割线)---";
				strResult = strName + "|" + strValue;
				vecAttributes.push_back(strResult);

				int attl = it.value()->m_Attf.m_vecAttf[i].attl;
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
				strValue = it.value()->m_Attf.m_vecAttf[i].atvl;
				strResult = strName + "|" + strValue;
				vecAttributes.push_back(strResult);

				QStringList lstValues = strValue.split(",");
				for (int j = 0; j < lstValues.size(); j++)
				{
					unsigned short expertValue = lstValues[j].toUShort();
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

			size = it.value()->m_Natf.m_vecNatf.size();
			for (int i = 0; i < size; i++)
			{
				strName = "---(分割线)---";
				strValue = "---(分割线)---";
				strResult = strName + "|" + strValue;
				vecAttributes.push_back(strResult);

				int attl = it.value()->m_Natf.m_vecNatf[i].attl;
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
				strValue = codec_16->toUnicode(it.value()->m_Natf.m_vecNatf[i].atvl);
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
	m_mapQueryResult.clear();
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

	if (m_mapQueryResult.size() == 0 || row < 0)
		m_pHighlightFeature = nullptr;

	int n = 0;
	QMap<int, CS57RecFeature*>::iterator it = m_mapQueryResult.begin();
	for (; it != m_mapQueryResult.end(); it++)
	{
		if (n == row)
		{
			m_pHighlightFeature = it.value();
			break;
		}
		n++;
	}
}

CS57RecFeature* CS57FeatureQuery::getHighlightFeature() const
{
	return m_pHighlightFeature;
}