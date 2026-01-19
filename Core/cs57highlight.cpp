#include "stdafx.h"
#include "cs57highlight.h"
#include "cs57recfeature.h"
#include "cs57cell.h"
#include "cs57recfeature.h"
#include "cs57recvector.h"

using namespace Core;
CS57FeatureHighlight::CS57FeatureHighlight()
	: m_pHighlightFeature(nullptr)
	, m_nLineWidth(2)
	, m_LineColor(QColor(255,0,0))
	, m_FillColor(QColor(255,0,0,100))
{}

CS57FeatureHighlight::~CS57FeatureHighlight()
{}


void CS57FeatureHighlight::setOperateMode(XT::OperateMode mode)
{
	m_eCurOperateMode = mode;
}

void CS57FeatureHighlight::setQueryMode(XT::QueryMode mode)
{
	m_eCurQueryMode = mode;
}
void CS57FeatureHighlight::setDataSource(CS57RecFeature* pFeature)
{
	m_pHighlightFeature = pFeature;
}
//************************************
// Method:    doRender
// Brief:	  开始渲染高亮任务
// Returns:   void
// Author:    cl
// DateTime:  2021/07/23
//************************************
void CS57FeatureHighlight::doRender(QPainter* p)
{
	if (m_pHighlightFeature == nullptr || 
		m_eCurOperateMode == XT::NORMAL)
		return;

	m_pPainter = p;

	switch (m_eCurQueryMode)
	{
	case XT::P_QUERY:
		pointHighlight(m_pHighlightFeature);
		break;
	case XT::L_QUERY:
		lineHighlight(m_pHighlightFeature);
		break;
	case XT::A_QUERY:
		areaHighlight(m_pHighlightFeature);
		break;
	}
}

//************************************
// Method:    pointHighlight
// Brief:	  高亮点要素
// Returns:   void
// Author:    cl
// DateTime:  2021/07/23
// Parameter: CS57RecFeature * pFeature
//************************************
void CS57FeatureHighlight::pointHighlight(CS57RecFeature* pFeature)
{
	QPoint renderPt;
	CS57Cell* pCell = pFeature->getParentCell();
	double comf = pCell->getCellComf();
	CS57RecFeature* pFE = pFeature;
	if (pFE->m_Frid.objl == 129)
	{
		for (int j = 0;j < pCell->m_vecS57RecVI[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg3d.m_vecSg3d.size();j++)
		{
			double lat_rad = pCell->m_vecS57RecVI[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg3d.m_vecSg3d[j].ycoo / comf / RO;
			double lon_rad = pCell->m_vecS57RecVI[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg3d.m_vecSg3d[j].xcoo / comf / RO;
			int dpx, dpy;
			double vpx, vpy;
			m_pTransform->m_Proj.getXY(lat_rad, lon_rad, &vpy, &vpx);
			m_pTransform->m_Viewport.convertVp2Dp(vpx, vpy, &dpx, &dpy);
			renderPt.setX(dpx);
			renderPt.setY(dpy);
		}
	}
	if (pFE->m_Fspt.m_vecFspt[0].rcnm == 110)
	{
		if (pCell->m_vecS57RecVI[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d.size() != 1)
			return;

		double lat_rad = pCell->m_vecS57RecVI[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d[0].ycoo / comf / RO;
		double lon_rad = pCell->m_vecS57RecVI[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d[0].xcoo / comf / RO;
		int dpx, dpy;
		double vpx, vpy;
		m_pTransform->m_Proj.getXY(lat_rad, lon_rad, &vpy, &vpx);
		m_pTransform->m_Viewport.convertVp2Dp(vpx, vpy, &dpx, &dpy);
		renderPt.setX(dpx);
		renderPt.setY(dpy);
	}
	else if (pFE->m_Fspt.m_vecFspt[0].rcnm == 120)
	{
		if (pCell->m_vecS57RecVC[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d.size() != 1)
			return;

		double lat_rad = pCell->m_vecS57RecVC[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d[0].ycoo / comf / RO;
		double lon_rad = pCell->m_vecS57RecVC[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d[0].xcoo / comf / RO;
		int dpx, dpy;
		double vpx, vpy;
		m_pTransform->m_Proj.getXY(lat_rad, lon_rad, &vpy, &vpx);
		m_pTransform->m_Viewport.convertVp2Dp(vpx, vpy, &dpx, &dpy);
		renderPt.setX(dpx);
		renderPt.setY(dpy);
	}
	QPen pen;
	pen.setColor(m_LineColor);
	pen.setWidth(m_nLineWidth);
	m_pPainter->save();
	m_pPainter->setPen(pen);
	m_pPainter->setBrush(QBrush(m_FillColor));
	QRect rect(renderPt.x() - 10, renderPt.y() - 10, 20, 20);
	m_pPainter->drawEllipse(rect);
	m_pPainter->restore();
}

//************************************
// Method:    lineHighlight
// Brief:	  高亮线要素
// Returns:   void
// Author:    cl
// DateTime:  2021/07/23
// Parameter: CS57RecFeature * pFeature
//************************************
void CS57FeatureHighlight::lineHighlight(CS57RecFeature* pFeature)
{
	CS57Cell* pCell = pFeature->getParentCell();
	double comf = pCell->getCellComf();
	CS57RecFeature* pFE = pFeature;

	CPolylines Polylines;
	QVector<QVector<QPointF>*> vecPolyline;
	pCell->genFeaturePolyline(pFeature, &vecPolyline);
	if (vecPolyline.size() == 0)	return;
	for (int j = 0; j < vecPolyline.size(); j++)
	{
		CPolyline polyline;
		for (int k = 0; k < vecPolyline[j]->size(); k++)
		{
			double lon_rad = vecPolyline[j]->at(k).x() / RO;
			double lat_rad = vecPolyline[j]->at(k).y() / RO;

			int dpx, dpy;
			double vpx, vpy;
			m_pTransform->m_Proj.getXY(lat_rad, lon_rad, &vpy, &vpx);
			m_pTransform->m_Viewport.convertVp2Dp(vpx, vpy, &dpx, &dpy);
			QPoint pt(dpx, dpy);
			polyline.push_back(pt);
		}
		Polylines.push_back(polyline);
	}
	qDeleteAll(vecPolyline);
	vecPolyline.clear();

	QPen pen;
	pen.setColor(m_LineColor);
	pen.setWidth(m_nLineWidth);
	m_pPainter->save();
	m_pPainter->setPen(pen);
	for (int i = 0; i < Polylines.size(); i++)
	{
		QPoint* pPoint = new QPoint[Polylines[i].size()];
		for (int j = 0; j < Polylines[i].size(); j++)
		{
			pPoint[j].setX(Polylines[i][j].x());
			pPoint[j].setY(Polylines[i][j].y());
		}
		m_pPainter->drawPolyline(pPoint, Polylines[i].size());

		delete[] pPoint;
		pPoint = nullptr;
	}
	m_pPainter->restore();
}

//************************************
// Method:    areaHighlight
// Brief:	  高亮面要素
// Returns:   void
// Author:    cl
// DateTime:  2021/07/23
// Parameter: CS57RecFeature * pFeature
//************************************
void CS57FeatureHighlight::areaHighlight(CS57RecFeature* pFeature)
{
	CS57Cell* pCell = pFeature->getParentCell();
	double comf = pCell->getCellComf();
	CS57RecFeature* pFE = pFeature;

	CPolylines Polygons;
	QVector<QVector<QPointF>*> vecPolgyon;
	pCell->genFeaturePolygon(pFeature, &vecPolgyon);

	if (vecPolgyon.size() == 0)	
		return;

	for (int j = 0; j < vecPolgyon.size(); j++)
	{
		CPolyline polygon;
		for (int k = 0; k < vecPolgyon[j]->size(); k++)
		{
			double lon_rad = vecPolgyon[j]->at(k).x() / RO;
			double lat_rad = vecPolgyon[j]->at(k).y() / RO;

			int dpx, dpy;
			double vpx, vpy;
			m_pTransform->m_Proj.getXY(lat_rad, lon_rad, &vpy, &vpx);
			m_pTransform->m_Viewport.convertVp2Dp(vpx, vpy, &dpx, &dpy);
			QPoint pt(dpx, dpy);
			polygon.push_back(pt);
		}
		Polygons.push_back(polygon);
	}
	qDeleteAll(vecPolgyon);
	vecPolgyon.clear();

	QPen pen;
	pen.setColor(m_LineColor);
	pen.setWidth(m_nLineWidth);
	m_pPainter->save();
	m_pPainter->setPen(pen);
	m_pPainter->setBrush(QBrush(m_FillColor));
	QPainterPath path;
	for (int i = 0; i < Polygons.size(); i++)
	{
		QPolygon polygon;
		for (int j = 0; j < Polygons[i].size(); j++)
		{
			polygon.append(Polygons[i][j]);
		}
		path.addPolygon(polygon);
	}
	m_pPainter->fillPath(path, QBrush(m_FillColor));
	m_pPainter->restore();
}

void CS57FeatureHighlight::setLineWidth(int w)
{
	m_nLineWidth = w;
}

void CS57FeatureHighlight::setLineColor(QColor color)
{
	m_LineColor = color;
}

void CS57FeatureHighlight::setFillColor(QColor color)
{
	m_FillColor = color;
}

int CS57FeatureHighlight::getLineWidth() const
{
	return m_nLineWidth;
}

QColor CS57FeatureHighlight::getLineColor() const
{
	return m_LineColor;
}

QColor CS57FeatureHighlight::getFillColor() const
{
	return m_FillColor;
}


