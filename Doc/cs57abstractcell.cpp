#include "stdafx.h"
#include "cs57abstractcell.h"
#include "cs57recfeature.h"
#include "cs57recvector.h"
#include <QMessageBox>
#include <QDebug>

using namespace Doc;
#pragma execution_character_set("utf-8")
CS57AbstractCell::CS57AbstractCell()
	: m_pRecDsgi(nullptr)
	, m_pRecDsgr(nullptr)
{}

CS57AbstractCell::~CS57AbstractCell()
{}

//************************************
// Method:    getCellName
// Brief:	  获取图幅名称
// Returns:   QString
// Author:    cl
// DateTime:  2022/07/21
//************************************
QString CS57AbstractCell::getCellName() const
{
	return m_pRecDsgi->m_Dsid.dsnm;
}

//************************************
// Method:    getCellNameWithoutSuffix
// Brief:	  获取图幅名称不含扩展后缀
// Returns:   QString
// Author:    cl
// DateTime:  2022/07/21
//************************************
QString CS57AbstractCell::getCellNameWithoutSuffix() const
{
	QString strCellNo = m_pRecDsgi->m_Dsid.dsnm;

	return strCellNo.replace(".000", "");
}

//************************************
// Method:    getCellEdtn
// Brief:	  获取图幅Edtn
// Returns:   QString
// Author:    cl
// DateTime:  2022/07/21
//************************************
QString CS57AbstractCell::getCellEdtn() const
{
	return m_pRecDsgi->m_Dsid.edtn;
}

//************************************
// Method:    getCellUpdn
// Brief:	  获取图幅Updn
// Returns:   QString
// Author:    cl
// DateTime:  2022/07/21
//************************************
QString CS57AbstractCell::getCellUpdn() const
{
	return m_pRecDsgi->m_Dsid.updn;
}

//************************************
// Method:    getCellCscl
// Brief:	  获取图幅Cscl
// Returns:   double
// Author:    cl
// DateTime:  2022/07/21
//************************************
double CS57AbstractCell::getCellCscl() const
{
	return m_pRecDsgr->m_Dspm.cscl;
}

//************************************
// Method:    getCellComf
// Brief:	  获取图幅Comf
// Returns:   double
// Author:    cl
// DateTime:  2022/07/21
//************************************
double CS57AbstractCell::getCellComf() const
{
	return m_pRecDsgr->m_Dspm.comf;
}

//************************************
// Method:    getCellSomf
// Brief:	  获取图幅Somf
// Returns:   double
// Author:    cl
// DateTime:  2022/07/21
//************************************
double CS57AbstractCell::getCellSomf() const
{
	return m_pRecDsgr->m_Dspm.somf;
}

//************************************
// Method:    getFeatureSize
// Brief:	  获取要素总数
// Returns:   int
// Author:    cl
// DateTime:  2022/07/21
//************************************
int CS57AbstractCell::getFeatureSize() const
{
	return m_vecS57RecFE.size();
}

//************************************
// Method:    genFeaturePolyline
// Brief:	  从图幅中获取线状要素的坐标数据
// Returns:   void
// Author:    cl
// DateTime:  2021/08/20
// Parameter: CS57RecFeature * pFE
// Parameter: QVector<CPolylineF * > * pPolylines
//************************************
void CS57AbstractCell::genFeaturePolyline(CS57RecFeature* pFE, QVector<QVector<QPointF>*>* pPolylines)
{
	if (pFE->m_Frid.prim != 3 && pFE->m_Frid.prim != 2)
		return;

	int firstID = -1, endId = -1;
	double x, y;

	double comf = getCellComf();
	for (int i = 0; i < pFE->m_Fspt.m_vecFspt.size(); i++)
	{
		if (pFE->m_Fspt.m_vecFspt[i].mask == 1)
			continue;

		CPolylineF* pvecPolyline = new CPolylineF();
		CS57RecVector* pRecVE = m_vecS57RecVE[pFE->m_Fspt.m_vecFspt[i].idx];
		if (pFE->m_Fspt.m_vecFspt[i].ornt != 2)	// 正向
		{
			QPointF pt0;
			x = (double)m_vecS57RecVC[pRecVE->m_Vrpt.m_vecVrpt[0].idx]->m_Sg2d.m_vecSg2d[0].xcoo / comf;
			y = (double)m_vecS57RecVC[pRecVE->m_Vrpt.m_vecVrpt[0].idx]->m_Sg2d.m_vecSg2d[0].ycoo / comf;
			pt0.setX(x);
			pt0.setY(y);
			pvecPolyline->push_back(pt0);
			for (int j = 0; j < pRecVE->m_Sg2d.m_vecSg2d.size(); j++)
			{
				QPointF pt;
				x = (double)pRecVE->m_Sg2d.m_vecSg2d[j].xcoo / comf;
				y = (double)pRecVE->m_Sg2d.m_vecSg2d[j].ycoo / comf;
				pt.setX(x);
				pt.setY(y);
				pvecPolyline->push_back(pt);
			}

			QPointF pt1;
			x = (double)m_vecS57RecVC[pRecVE->m_Vrpt.m_vecVrpt[1].idx]->m_Sg2d.m_vecSg2d[0].xcoo / comf;
			y = (double)m_vecS57RecVC[pRecVE->m_Vrpt.m_vecVrpt[1].idx]->m_Sg2d.m_vecSg2d[0].ycoo / comf;
			pt1.setX(x);
			pt1.setY(y);
			pvecPolyline->push_back(pt1);
		}
		else
		{
			QPointF pt0;
			x = (double)m_vecS57RecVC[pRecVE->m_Vrpt.m_vecVrpt[1].idx]->m_Sg2d.m_vecSg2d[0].xcoo / comf;
			y = (double)m_vecS57RecVC[pRecVE->m_Vrpt.m_vecVrpt[1].idx]->m_Sg2d.m_vecSg2d[0].ycoo / comf;
			pt0.setX(x);
			pt0.setY(y);
			pvecPolyline->push_back(pt0);
			for (int j = pRecVE->m_Sg2d.m_vecSg2d.size() - 1; j >= 0; j--)
			{
				QPointF pt;
				x = (double)pRecVE->m_Sg2d.m_vecSg2d[j].xcoo / comf;
				y = (double)pRecVE->m_Sg2d.m_vecSg2d[j].ycoo / comf;
				pt.setX(x);
				pt.setY(y);
				pvecPolyline->push_back(pt);
			}
			QPointF pt1;
			x = (double)m_vecS57RecVC[pRecVE->m_Vrpt.m_vecVrpt[0].idx]->m_Sg2d.m_vecSg2d[0].xcoo / comf;
			y = (double)m_vecS57RecVC[pRecVE->m_Vrpt.m_vecVrpt[0].idx]->m_Sg2d.m_vecSg2d[0].ycoo / comf;
			pt1.setX(x);
			pt1.setY(y);
			pvecPolyline->push_back(pt1);
		}
		pPolylines->push_back(pvecPolyline);
	}
}

//************************************
// Method:    genFeaturePolygon
// Brief:	  从图幅中获取面状要素的坐标数据
// Returns:   void
// Author:    cl
// DateTime:  2021/08/20
// Parameter: CS57RecFeature * pFE	要素
// Parameter: QVector<CPolylineF * > * pRings	环坐标数据，注意使用后释放
//************************************
void CS57AbstractCell::genFeaturePolygon(CS57RecFeature* pFE, QVector<QVector<QPointF>*>* pRings)
{
	if (pFE->m_Frid.prim != 3)
		return;

	int startId = -1;
	int endId = -1;
	int len = 0;
	double x, y;
	double comf = getCellComf();
	CPolylineF* pvecRing = nullptr;
	for (int i = 0; i < pFE->m_Fspt.m_vecFspt.size(); i++)
	{
		CS57RecVector* pVE = m_vecS57RecVE[pFE->m_Fspt.m_vecFspt[i].idx];
		if (pFE->m_Fspt.m_vecFspt[i].ornt != 2)	// 正向
		{
			if (len == 0)
			{
				pvecRing = new CPolylineF();
				startId = pVE->m_Vrpt.m_vecVrpt[0].idx;
			}
			if (endId != pVE->m_Vrpt.m_vecVrpt[0].idx)
			{
				QPointF pt;
				x = (double)m_vecS57RecVC[pVE->m_Vrpt.m_vecVrpt[0].idx]->m_Sg2d.m_vecSg2d[0].xcoo / comf;
				y = (double)m_vecS57RecVC[pVE->m_Vrpt.m_vecVrpt[0].idx]->m_Sg2d.m_vecSg2d[0].ycoo / comf;
				pt.setX(x);
				pt.setY(y);
				pvecRing->push_back(pt);
				len++;
			}
			for (int j = 0; j < pVE->m_Sg2d.m_vecSg2d.size(); j++)
			{
				QPointF pt;
				x = (double)pVE->m_Sg2d.m_vecSg2d[j].xcoo / comf;
				y = (double)pVE->m_Sg2d.m_vecSg2d[j].ycoo / comf;
				pt.setX(x);
				pt.setY(y);
				pvecRing->push_back(pt);
				len++;
			}
			endId = pVE->m_Vrpt.m_vecVrpt[1].idx;
			QPointF pt;
			x = (double)m_vecS57RecVC[endId]->m_Sg2d.m_vecSg2d[0].xcoo / comf;
			y = (double)m_vecS57RecVC[endId]->m_Sg2d.m_vecSg2d[0].ycoo / comf;
			pt.setX(x);
			pt.setY(y);
			pvecRing->push_back(pt);
			len++;
		}
		else
		{
			if (len == 0)
			{
				pvecRing = new CPolylineF();
				startId = pVE->m_Vrpt.m_vecVrpt[1].idx;
			}

			if (endId != pVE->m_Vrpt.m_vecVrpt[1].idx)
			{
				QPointF pt;
				x = (double)m_vecS57RecVC[pVE->m_Vrpt.m_vecVrpt[1].idx]->m_Sg2d.m_vecSg2d[0].xcoo / comf;
				y = (double)m_vecS57RecVC[pVE->m_Vrpt.m_vecVrpt[1].idx]->m_Sg2d.m_vecSg2d[0].ycoo / comf;
				pt.setX(x);
				pt.setY(y);
				pvecRing->push_back(pt);
				len++;
			}

			for (int j = pVE->m_Sg2d.m_vecSg2d.size() - 1; j >= 0; j--)
			{
				QPointF pt;
				x = (double)pVE->m_Sg2d.m_vecSg2d[j].xcoo / comf;
				y = (double)pVE->m_Sg2d.m_vecSg2d[j].ycoo / comf;
				pt.setX(x);
				pt.setY(y);
				pvecRing->push_back(pt);
				len++;
			}
			endId = pVE->m_Vrpt.m_vecVrpt[0].idx;
			QPointF pt;
			x = (double)m_vecS57RecVC[endId]->m_Sg2d.m_vecSg2d[0].xcoo / comf;
			y = (double)m_vecS57RecVC[endId]->m_Sg2d.m_vecSg2d[0].ycoo / comf;
			pt.setX(x);
			pt.setY(y);
			pvecRing->push_back(pt);
			len++;
		}
		if (startId == endId && len != 0)
		{
			pRings->push_back(pvecRing);
			endId = -1;
			len = 0;
		}
	}
	if (len != 0)
	{
		len = 0;
		QMessageBox::information(nullptr, "提示", "M_COVR数据未闭合");
	}
}


