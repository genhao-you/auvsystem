#include "stdafx.h"
#include "cs57cell.h"
#include "dpi.h"
#include "cs57annotation.h"
#include "cs57renderer.h"
#include "cs57recfeature.h"
#include "cs57recvector.h"
#include "cs57symbolrules.h"
#include "cs57auxiliaryrenderer.h"
#include <QDebug>

using namespace Core;
#pragma execution_character_set("utf-8")
CS57AbstractRenderer::CS57AbstractRenderer()
	: m_pPainter(nullptr)
	, m_pTransform(CS57Transform::instance())
{}

CS57AbstractRenderer::~CS57AbstractRenderer()
{}

void CS57AbstractRenderer::setPresLib(CS57PresLib* prelib)
{
	m_pPresLib = prelib;
}

CS57GlobalMapRenderer::CS57GlobalMapRenderer()
	: m_pProvider(nullptr)
	, m_nLineWidth(1)
	, m_LineColor(QColor(71, 71, 61))
	, m_FillColor(QColor(207, 210, 181))
	, m_bVisible(true)
{}

CS57GlobalMapRenderer::~CS57GlobalMapRenderer()
{}
void CS57GlobalMapRenderer::setVisible(bool visible)
{
	m_bVisible = visible;
}
void CS57GlobalMapRenderer::setLineWidth(int w)
{
	m_nLineWidth = w;
}

int CS57GlobalMapRenderer::getLineWidth() const
{
	return m_nLineWidth;
}

void CS57GlobalMapRenderer::setLineColor(const QColor& color)
{
	m_LineColor = color;
}

QColor CS57GlobalMapRenderer::getLineColor() const
{
	return m_LineColor;
}
void CS57GlobalMapRenderer::setFillColor(const QColor& color)
{
	m_FillColor = color;
}

QColor CS57GlobalMapRenderer::getFillColor() const
{
	return m_FillColor;
}
void CS57GlobalMapRenderer::setDataSource(CS57GlobalMapProvider* pGlobalMapProvider)
{
	m_pProvider = pGlobalMapProvider;
}
void CS57GlobalMapRenderer::doRender(QPainter* p)
{
	if (!m_pProvider || m_pProvider->m_vecRenderPts.size() == 0 || !m_bVisible) return;

	p->save();
	p->setPen(QPen(m_LineColor, m_nLineWidth));
	p->setBrush(QBrush(m_FillColor));

	for (int i = 0; i < m_pProvider->m_vecRenderPts.size(); i++)
	{
		QPoint* pts = new QPoint[m_pProvider->m_vecRenderPts[i]->size()];
		for (int j = 0; j < m_pProvider->m_vecRenderPts[i]->size(); j++)
		{
			QPointF fpt = m_pProvider->m_vecRenderPts[i]->at(j);
			double lon_rad = fpt.x() / RO;
			double lat_rad = fpt.y() / RO;

			double vpx, vpy;
			int dpx, dpy;
			m_pTransform->m_Proj.getXY(lat_rad, lon_rad, &vpy, &vpx);
			m_pTransform->m_Viewport.convertVp2Dp(vpx, vpy, &dpx, &dpy);
			QPoint pt(dpx, dpy);
			pts[j] = pt;
		}
		p->drawPolygon(pts, m_pProvider->m_vecRenderPts[i]->size());

		delete[] pts;
		pts = nullptr;
	}
	//QString strNum = QString::number(m_pGlobalMap->m_vecRenderPts.size());
	//p->drawText(QPoint(50, 200), strNum);
	p->restore();
}

CS57McovrRenderer::CS57McovrRenderer()
	: m_pProvider(nullptr)
	, m_nLineWidth(1)
	, m_bVisible(true)
	, m_LineColor(QColor(255, 0, 0))
	, m_FillColor(QColor(255, 255, 255, 0))
{}

CS57McovrRenderer::~CS57McovrRenderer()
{}
void CS57McovrRenderer::setLineWidth(int w)
{
	m_nLineWidth = w;
}

int CS57McovrRenderer::getLineWidth() const
{
	return m_nLineWidth;
}

void CS57McovrRenderer::setLineColor(const QColor& color)
{
	m_LineColor = color;
}

QColor CS57McovrRenderer::getLineColor() const
{
	return m_LineColor;
}
void CS57McovrRenderer::setFillColor(const QColor& color)
{
	m_FillColor = color;
}

QColor CS57McovrRenderer::getFillColor() const
{
	return m_FillColor;
}

void CS57McovrRenderer::setVisible(bool visible)
{
	m_bVisible = visible;
}

void CS57McovrRenderer::setDataSource(CS57McovrProvider* pMcovrProvider)
{
	m_pProvider = pMcovrProvider;
}

void CS57McovrRenderer::doRender(QPainter* p)
{
	if (!m_pProvider ||
		m_pProvider->m_vecRenderMcovr.size() == 0 ||
		!m_bVisible) return;

	p->save();
	p->setPen(QPen(m_LineColor,m_nLineWidth));
	p->setBrush(QBrush(m_FillColor));

	//开始绘制
	for (int i = 0; i < m_pProvider->m_vecRenderMcovr.size(); i++)
	{
		CS57CellMessage* pCellMsg = m_pProvider->m_vecRenderMcovr[i];
		for (int j = 0; j < pCellMsg->m_vecS57RecFE.size(); j++)
		{
			CS57RecFeature* pFE = pCellMsg->m_vecS57RecFE[j];
			QVector<QVector<QPointF>*> pRings;
			pCellMsg->genFeaturePolygon(pFE, &pRings);
			if (pRings.size() == 0)
			{
				qDeleteAll(pRings);
				pRings.clear();
				continue;
			}
			for (int k = 0; k < pRings.size(); k++)
			{
				QPoint* pts = new QPoint[pRings[k]->size()];
				for (int l = 0; l < pRings[k]->size(); l++)
				{
					QPointF p = pRings[k]->at(l);
					double lon_rad = (double)p.x()/ RO;
					double lat_rad = (double)p.y()/ RO;

					double vpx, vpy;
					int dpx, dpy;
					m_pTransform->m_Proj.getXY(lat_rad, lon_rad, &vpy, &vpx);
					m_pTransform->m_Viewport.convertVp2Dp(vpx, vpy, &dpx, &dpy);
					QPoint pt(dpx, dpy);
					pts[l] = pt;
				}
				p->drawPolygon(pts, pRings[k]->size());

				delete[] pts;
				pts = nullptr;

				qDeleteAll(pRings);
				pRings.clear();
			}
		}
	}
	//QString strNum = QString::number(m_pProvider->m_vecRenderMcovr.size());
	//p->drawText(QPoint(5, 15), strNum);
	p->restore();
}

CS57CellRenderer::CS57CellRenderer()
	: m_pProvider(nullptr)
	, m_eRenderType(XT::RENDER_DISPLAY)
	, m_pPrintParameters(nullptr)
{}

CS57CellRenderer::~CS57CellRenderer()
{}

void CS57CellRenderer::setSymbolMode(QString strSymbolMode)
{
	QStringList strs = strSymbolMode.split(",");
	m_strPSymbolMode = strs[0];
	m_strLSymbolMode = strs[1];
	m_strASymbolMode = strs[2];
}

void CS57CellRenderer::setDataSource(CS57CellProvider* pCellProvider)
{
	m_pProvider = pCellProvider;
}

void CS57CellRenderer::doRender(QPainter* p)
{
	if (!m_pProvider ||
		m_pProvider->m_vecCellDisplayContext.size() == 0)	
		return;

	m_pPainter = p;

	for (int i = 0; i < m_pProvider->m_vecCellDisplayContext.size(); i++)
	{
		renderArea(m_pProvider->m_vecCellDisplayContext[i]);
		if(m_bBorderVisible)
			renderBorder(m_pProvider->m_vecCellDisplayContext[0]);
		renderLine(m_pProvider->m_vecCellDisplayContext[i]);
		renderPoint(m_pProvider->m_vecCellDisplayContext[i]);
		renderAnnotation(m_pProvider->m_vecCellDisplayContext[i]);
	}
}

void CS57CellRenderer::renderArea(CellDisplayContext* pContext)
{
	CS57AreaSymbol	areaSymbol(m_pPresLib);

	CS57Cell* pCell = pContext->pS57Cell;
	m_pPainter->save();
	m_pPainter->setPen(QPen(QColor(0, 0, 0), 1));
	double scale = m_pTransform->m_Viewport.getDisplayScale();
	double factor = getSymbolScaleFactor(pCell, scale);
	for (int i = 0; i < pContext->vecAFeatures.size(); i++)
	{
		CS57RecFeature* pFE = pContext->vecAFeatures[i];

		if(pFE->m_Frid.objl >= 300 &&
		   pFE->m_Frid.objl <= 312)
			continue;

		if(pFE->m_Frid.objl == 63)
			continue;

		bool flag = false;
		for (int j = 0; j < pFE->m_vecInst.size();j++)
		{
			if (pFE->m_vecInst[j]->fieldINST.vecAC.size() > 0 ||
				pFE->m_vecInst[j]->fieldINST.vecAP.size() > 0 ||
				pFE->m_vecInst[j]->fieldINST.vecCS.size() > 0)
			{
				flag = true;
				break;
			}
		}
		if(!flag) continue;

		QVector<QPolygon> renderPts;
		QVector<QVector<QPointF>*> vecRings;
		pCell->genFeaturePolygon(pFE, &vecRings);
		if (vecRings.size() == 0)	return;
		for (int j = 0; j < vecRings.size(); j++)
		{
			QPolygon polygon;
			for (int k = 0; k < vecRings[j]->size(); k++)
			{
				double lon_rad = vecRings[j]->at(k).x() / RO;
				double lat_rad = vecRings[j]->at(k).y() / RO;

				double vpx, vpy;
				int dpx, dpy;
				m_pTransform->m_Proj.getXY(lat_rad, lon_rad, &vpy, &vpx);
				m_pTransform->m_Viewport.convertVp2Dp(vpx, vpy, &dpx, &dpy);
				QPoint pt(dpx, dpy);
				polygon.push_back(pt);
			}
			renderPts.push_back(polygon);
		}
		qDeleteAll(vecRings);
		vecRings.clear();

		for (int j = 0; j < pFE->m_vecInst.size(); j++)
		{
			if (pFE->m_vecInst[j]->tnam == m_strASymbolMode)
			{
				for (int k = 0; k < pFE->m_vecInst[j]->fieldINST.vecAC.size(); k++)
				{
					QString strAreaColor = pFE->m_vecInst[j]->fieldINST.vecAC[k]->areacolor;
					unsigned short uTransparent = pFE->m_vecInst[j]->fieldINST.vecAC[k]->transparence;
					areaSymbol.renderAC(m_pPainter, renderPts, uTransparent, strAreaColor);
				}
				for (int k = 0; k < pFE->m_vecInst[j]->fieldINST.vecCS.size(); k++)
				{
					if (pFE->m_vecInst[j]->fieldINST.vecCS[k]->procName =="DEPARE03")
					{
						CS57FieldINST fieldInst;
						CS57SymbolRules rulesSymbol(pFE);
						string str = rulesSymbol.DEPARE03();

						m_pPresLib->parseCS(fieldInst.fieldINST, str);
						for (int l = 0; l < fieldInst.fieldINST.vecAC.size(); l++)
						{
							QString strAreaColor = fieldInst.fieldINST.vecAC[k]->areacolor;
							unsigned short uTransparent = fieldInst.fieldINST.vecAC[k]->transparence;
							areaSymbol.renderAC(m_pPainter, renderPts, uTransparent, strAreaColor);
						}
					}
				}
				for (int k = 0; k < pFE->m_vecInst[j]->fieldINST.vecAP.size(); k++)
				{
					QString strPatName = pFE->m_vecInst[j]->fieldINST.vecAP[k]->patname;
					float rotation = pFE->m_vecInst[j]->fieldINST.vecAP[k]->rotation;
					areaSymbol.renderAP(m_pPainter, renderPts, strPatName, factor, rotation);
				}
			}
		}
	}
	for (int i = 0; i < pContext->vecAFeatures.size(); i++)
	{
		CS57RecFeature* pFE = pContext->vecAFeatures[i];

		if (pFE->m_Frid.objl >= 300 &&
			pFE->m_Frid.objl <= 312)
			continue;

		if (pFE->m_Frid.objl == 63)
			continue;

		if (pFE->m_Frid.objl == 1) continue;	// 行政区边线不绘
		if (pFE->m_Frid.objl == 27) continue;	// 警告区边线不绘
		if (pFE->m_Frid.objl == 91) continue;	// 引航站边线不绘
		if (pFE->m_Frid.objl == 42) continue;	// 已知深度区边线不绘
		if (pFE->m_Frid.objl == 73) continue;	// 陆地自然区边线不绘
		if (pFE->m_Frid.objl == 119) continue;	// 命名水域边线不绘
		if (pFE->m_Frid.objl == 71) continue;	// 陆地区边线不绘
		if (pFE->m_Frid.objl == 122) continue;	// 人工海岸边线不绘
		if (pFE->m_Frid.objl == 113) continue;	// 海床区边线

		bool flag = false;
		for (int j = 0; j < pFE->m_vecInst.size();j++)
		{
			if (pFE->m_vecInst[j]->fieldINST.vecLS.size() > 0 ||
				pFE->m_vecInst[j]->fieldINST.vecLC.size() > 0)
			{
				flag = true;
				break;
			}
		}
		if (!flag) continue;

		CPolylines Polylines;
		QVector<QVector<QPointF>*> vecPolyline;
		pCell->genFeaturePolyline(pFE, &vecPolyline);
		if (vecPolyline.size() == 0)	
			return;
		for (int j = 0; j < vecPolyline.size(); j++)
		{
			CPolyline polyline;
			for (int k = 0; k < vecPolyline[j]->size(); k++)
			{
				double lon_rad = vecPolyline[j]->at(k).x() / RO;
				double lat_rad = vecPolyline[j]->at(k).y() / RO;

				double vpx, vpy;
				int dpx, dpy;
				m_pTransform->m_Proj.getXY(lat_rad, lon_rad, &vpy, &vpx);
				m_pTransform->m_Viewport.convertVp2Dp(vpx, vpy, &dpx, &dpy);
				QPoint pt(dpx, dpy);
				polyline.push_back(pt);
			}
			Polylines.push_back(polyline);
		}
		qDeleteAll(vecPolyline);
		vecPolyline.clear();

		for (int j = 0; j < pFE->m_vecInst.size(); j++)
		{
			if (pFE->m_vecInst[j]->tnam == m_strASymbolMode)
			{
				//绘制颜色填充面
				for (int k = 0; k < pFE->m_vecInst[j]->fieldINST.vecLS.size(); k++)
				{
					QString strLineColor = pFE->m_vecInst[j]->fieldINST.vecLS[k]->lcolor;
					int nLineWidth = pFE->m_vecInst[j]->fieldINST.vecLS[k]->lwidth;
					QString strLineStyle = pFE->m_vecInst[j]->fieldINST.vecLS[k]->lstyle;

					for (int l = 0; l < Polylines.size(); l++)
					{
						areaSymbol.renderLS(m_pPainter, Polylines[l], factor, nLineWidth, strLineStyle, strLineColor);
					}
				}
				for (int k = 0; k < pFE->m_vecInst[j]->fieldINST.vecLC.size(); k++)
				{
					QString strLnstName = pFE->m_vecInst[j]->fieldINST.vecLC[k]->Cline;

					for (int l = 0; l < Polylines.size(); l++)
					{
						areaSymbol.renderLC(m_pPainter, Polylines[l], factor, strLnstName);
					}
				}
			}
		}
	}
	m_pPainter->restore();
}

void CS57CellRenderer::renderLine(CellDisplayContext* pContext)
{
	CS57LineSymbol	lineSymbol(m_pPresLib);
	CS57Cell* pCell = pContext->pS57Cell;
	m_pPainter->save();
	double scale = m_pTransform->m_Viewport.getDisplayScale();
	double factor = getSymbolScaleFactor(pCell, scale);
	for (int i = 0; i < pContext->vecLFeatures.size(); i++)
	{
		CS57RecFeature* pFE = pContext->vecLFeatures[i];

		CPolylines Polylines;
		QVector<QVector<QPointF>*> vecPolyline;
		pCell->genFeaturePolyline(pFE, &vecPolyline);
		if (vecPolyline.size() == 0)	return;
		for (int j = 0; j < vecPolyline.size(); j++)
		{
			CPolyline polyline;
			for (int k = 0; k < vecPolyline[j]->size(); k++)
			{
				double lon_rad = vecPolyline[j]->at(k).x() / RO;
				double lat_rad = vecPolyline[j]->at(k).y() / RO;

				double vpx, vpy;
				int dpx, dpy;
				m_pTransform->m_Proj.getXY(lat_rad, lon_rad, &vpy, &vpx);
				m_pTransform->m_Viewport.convertVp2Dp(vpx, vpy, &dpx, &dpy);
				QPoint pt(dpx, dpy);
				polyline.push_back(pt);
			}
			Polylines.push_back(polyline);
		}
		qDeleteAll(vecPolyline);
		vecPolyline.clear();

		for (int j = 0; j < pFE->m_vecInst.size(); j++)
		{
			if (pFE->m_vecInst[j]->tnam == m_strLSymbolMode)
			{
				for (int k = 0; k < pFE->m_vecInst[j]->fieldINST.vecLS.size(); k++)
				{
					QString strLineColor = pFE->m_vecInst[j]->fieldINST.vecLS[k]->lcolor;
					int nLineWidth = pFE->m_vecInst[j]->fieldINST.vecLS[k]->lwidth;
					QString strLineStyle = pFE->m_vecInst[j]->fieldINST.vecLS[k]->lstyle;

					for (int l = 0; l < Polylines.size(); l++)
					{
						lineSymbol.renderLS(m_pPainter, Polylines[l], factor, nLineWidth, strLineStyle, strLineColor);
					}
				}
				for (int k = 0; k < pFE->m_vecInst[j]->fieldINST.vecLC.size(); k++)
				{
					QString strLnstName = pFE->m_vecInst[j]->fieldINST.vecLC[k]->Cline;

					for (int l = 0; l < Polylines.size(); l++)
					{
						lineSymbol.renderLC(m_pPainter, Polylines[l], factor, strLnstName);
					}
				}
				for (int k = 0; k < pFE->m_vecInst[j]->fieldINST.vecCS.size(); k++)
				{
					if (pFE->m_vecInst[j]->fieldINST.vecCS[k]->procName == "DEPCNT03")
					{
						CS57FieldINST fieldInst;
						CS57SymbolRules symbolRules(pFE);
						string str = symbolRules.DEPCNT03();
						m_pPresLib->parseCS(fieldInst.fieldINST, str);
						for (int l = 0; l < fieldInst.fieldINST.vecLS.size(); l++)
						{
							QString strLineColor = fieldInst.fieldINST.vecLS[l]->lcolor;
							int nLineWidth = fieldInst.fieldINST.vecLS[l]->lwidth;
							QString strLineStyle = fieldInst.fieldINST.vecLS[l]->lstyle;

							for (int l = 0; l < Polylines.size(); l++)
							{
								lineSymbol.renderLS(m_pPainter, Polylines[l], factor, nLineWidth, strLineStyle, strLineColor);
							}
						}
					}
				}
			}
		}
	}
	m_pPainter->restore();
}

void CS57CellRenderer::renderPoint(CellDisplayContext* pContext)
{
	CS57PointSymbol	pointSymbol(m_pPresLib);

	CS57Cell* pCell = pContext->pS57Cell;
	double comf = pCell->getCellComf();
	double scale = m_pTransform->m_Viewport.getDisplayScale();
	double factor = getSymbolScaleFactor(pCell, scale);
	for (int i = 0; i < pContext->vecPFeatures.size(); i++)
	{
		QPoint renderPt;
		CS57RecFeature* pFE = pContext->vecPFeatures[i];
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
				float ve3d = (float)pCell->m_vecS57RecVI[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg3d.m_vecSg3d[j].ve3d;
				float somf = (float)pCell->getCellSomf();
				float z = ve3d / somf;
				renderSounding(m_pPainter, renderPt, factor, 0.0, z, false, false);

				//m_pPainter->save();
				//m_pPainter->setPen(QPen(QColor(0, 0, 0), 1));
				//m_pPainter->setBrush(QBrush(QColor(255, 0, 0)));
				//QRect rect(dpx - 2, dpy - 2, 4, 4);
				//m_pPainter->drawEllipse(rect);
				//m_pPainter->drawLine(pt.x() - 10, pt.y(), pt.x() + 10, pt.y());
				//m_pPainter->drawLine(pt.x(), pt.y() + 10, pt.x(), pt.y() - 10);
				//m_pPainter->restore();
			}
			continue;
		}
		if (pFE->m_Frid.prim == 1 && pFE->m_Fspt.m_vecFspt[0].rcnm == 110)
		{
			if (pCell->m_vecS57RecVI[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d.size() != 1)
				continue;

			double lat_rad = pCell->m_vecS57RecVI[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d[0].ycoo / comf / RO;
			double lon_rad = pCell->m_vecS57RecVI[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d[0].xcoo / comf / RO;
			int dpx, dpy;
			double vpx, vpy;
			m_pTransform->m_Proj.getXY(lat_rad, lon_rad, &vpy, &vpx);
			m_pTransform->m_Viewport.convertVp2Dp(vpx, vpy, &dpx, &dpy);
			renderPt.setX(dpx);
			renderPt.setY(dpy);

			if (pFE->m_Frid.objl == 81)
			{
				//磁差 绘制方位圈
				sCompassInfo compassInfo;
				compassInfo.centerPt = QPoint(renderPt);
				compassInfo.dCscl = pCell->getCellCscl();
				for (int j = 0; j < pFE->m_Attf.m_vecAttf.size(); j++)
				{
					int attl = pFE->m_Attf.m_vecAttf[j].attl;
					if (attl == 176)
					{
						//磁差值
						//QString strMagnetic = ;
						compassInfo.strMagnetic = pFE->m_Attf.m_vecAttf[j].atvl;
					}
					else if (attl == 173)
					{
						//年变率
						//QString strAnnual ;
						compassInfo.strAnnual = pFE->m_Attf.m_vecAttf[j].atvl;
					}
					else if (attl == 130)
					{
						//参考年份
						compassInfo.strYear = pFE->m_Attf.m_vecAttf[j].atvl;
					}
				}
				m_pPainter->save();
				renderCompass(compassInfo);
				m_pPainter->restore();

				continue;
			}
		}
		else if (pFE->m_Frid.prim == 1 && pFE->m_Fspt.m_vecFspt[0].rcnm == 120)
		{
			if (pCell->m_vecS57RecVC[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d.size() != 1)
				continue;

			double lat_rad = pCell->m_vecS57RecVC[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d[0].ycoo / comf / RO;
			double lon_rad = pCell->m_vecS57RecVC[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d[0].xcoo / comf / RO;
			int dpx, dpy;
			double vpx, vpy;
			m_pTransform->m_Proj.getXY(lat_rad, lon_rad, &vpy, &vpx);
			m_pTransform->m_Viewport.convertVp2Dp(vpx, vpy, &dpx, &dpy);
			renderPt.setX(dpx);
			renderPt.setY(dpy);
			//m_pPainter->save();
			//m_pPainter->setPen(QPen(QColor(0, 0, 0), 1));
			//m_pPainter->setBrush(QBrush(QColor(0, 0, 255)));
			//QRect rect(dpx - 3, dpy - 3, 6, 6);
			//m_pPainter->drawEllipse(rect);
			//m_pPainter->restore();
		}
		else
		{
			continue;
		}

		m_pPainter->save();
		for (int j = 0; j < pFE->m_vecInst.size(); j++)
		{
			if (pFE->m_vecInst[j]->tnam == m_strPSymbolMode)
			{
				bool offset = false;
				if (pFE->m_Frid.objl == 144)
				{
					for (int k = 0; k < pFE->m_vecInst[j]->fieldINST.vecSY.size(); k++)
					{
						if (pFE->m_vecInst[j]->fieldINST.vecSY[k]->offset)
						{
							offset = true;
							break;
						}
					}
				}
				for (int k = 0; k < pFE->m_vecInst[j]->fieldINST.vecSY.size(); k++)
				{
					float rotation = 0;
					QString strSymbName = pFE->m_vecInst[j]->fieldINST.vecSY[k]->syname;
					QString strSymbAttr = pFE->m_vecInst[j]->fieldINST.vecSY[k]->attr;
					if (strSymbAttr.length() == 6)
					{
						unsigned short attr = m_pPresLib->getAttrCode(strSymbAttr);
						rotation = pFE->getAttrValue(attr).toFloat();
					}
					else
					{
						rotation = pFE->m_vecInst[j]->fieldINST.vecSY[k]->rotation;
					}
					pointSymbol.renderSY(m_pPainter, renderPt, strSymbName, factor, rotation);
				}
				for (int k = 0; k < pFE->m_vecInst[j]->fieldINST.vecCA.size();k++)
				{
					float sectr1 = pFE->m_vecInst[j]->fieldINST.vecCA[k]->sectr1;
					float sectr2 = pFE->m_vecInst[j]->fieldINST.vecCA[k]->sectr2;
					float arc_radius = pFE->m_vecInst[j]->fieldINST.vecCA[k]->arc_radius;
					float sector_radius = pFE->m_vecInst[j]->fieldINST.vecCA[k]->sector_radius;
					unsigned short outline_width = pFE->m_vecInst[j]->fieldINST.vecCA[k]->outline_width;
					unsigned short arc_width = pFE->m_vecInst[j]->fieldINST.vecCA[k]->arc_width;
					char arc_color[6] = { 0 };
					char outline_color[6] = { 0 };
					strcpy(arc_color, pFE->m_vecInst[j]->fieldINST.vecCA[k]->arc_color);
					strcpy(outline_color, pFE->m_vecInst[j]->fieldINST.vecCA[k]->outline_color);
					pointSymbol.renderCA(m_pPainter, renderPt, sectr1, sectr2, arc_radius, sector_radius, outline_width, arc_width, arc_color, outline_color);	
				}
			}
		}
		//绘制十字线
		//CS57AuxiliaryRenderer auxiliary;
		//auxiliary.renderCrossLine(m_pPainter,renderPt);

		m_pPainter->restore();
	}
}

void CS57CellRenderer::renderSounding(QPainter* painter,QPoint& point, float scale, float rotation,float z, bool shallow,bool swept)
{
	CS57PointSymbol	pointSymbol(m_pPresLib);
	QString strSoundName;
	float dpi = painter->device()->physicalDpiX();
	// 根据水深是否浅于安全水深，设置水深符号名前缀
	if (shallow)
		strSoundName = "SOUNDS";  // 浅于安全水深
	else
		strSoundName = "SOUNDG";

	float uz = abs(z);	// 水深取绝对值，将水深值加上0.001，防止取整误差
	uz += 0.001;

	QString strSound = QString("%1").arg(uz);	// 将水深值转换成字符串
	QStringList lstSound = strSound.split('.');		// 分割整数与小数部分

	QString strSoundMain = lstSound.at(0);			// 水深整数部分
	unsigned short lenMain = strSoundMain.size();	// 

	QStringList lstName;
	switch (lenMain)
	{
	case 1:
		strSoundName = strSoundName.left(6).append('1').append(strSoundMain);
		lstName.append(strSoundName);
		point.setX(point.x() + 1.0 / 25.4 * dpi * scale);
		break;
	case 2:
		strSoundName = strSoundName.left(6).append('1').append(strSoundMain.at(0));
		lstName.append(strSoundName);
		strSoundName = strSoundName.left(6).append('0').append(strSoundMain.at(1));
		lstName.append(strSoundName);
		break;
	case 3:
		strSoundName = strSoundName.left(6).append('2').append(strSoundMain.at(0));
		lstName.append(strSoundName);
		strSoundName = strSoundName.left(6).append('1').append(strSoundMain.at(1));
		lstName.append(strSoundName);
		strSoundName = strSoundName.left(6).append('0').append(strSoundMain.at(2));
		lstName.append(strSoundName);
		point.setX(point.x() + 1.0 / 25.4 * dpi * scale);
		break;
	case 4:
		strSoundName = strSoundName.left(6).append('2').append(strSoundMain.at(0));
		lstName.append(strSoundName);
		strSoundName = strSoundName.left(6).append('1').append(strSoundMain.at(1));
		lstName.append(strSoundName);
		strSoundName = strSoundName.left(6).append('0').append(strSoundMain.at(2));
		lstName.append(strSoundName);
		strSoundName = strSoundName.left(6).append('4').append(strSoundMain.at(3));
		lstName.append(strSoundName);
		break;
	case 5:
		strSoundName = strSoundName.left(6).append('3').append(strSoundMain.at(0));
		lstName.append(strSoundName);
		strSoundName = strSoundName.left(6).append('2').append(strSoundMain.at(1));
		lstName.append(strSoundName);
		strSoundName = strSoundName.left(6).append('1').append(strSoundMain.at(2));
		lstName.append(strSoundName);
		strSoundName = strSoundName.left(6).append('0').append(strSoundMain.at(3));
		lstName.append(strSoundName);
		strSoundName = strSoundName.left(6).append('4').append(strSoundMain.at(4));
		lstName.append(strSoundName);
		point.setX(point.x() + 1.0 / 25.4 * dpi * scale);
		break;
	default:
		lstName.clear();
		break;
	}

	// 写水深整数部分

	for (int i = 0;i < lstName.size();i++)
	{
		pointSymbol.renderSounding(painter, point, lstName.at(i), scale, rotation);
	}

	// 写水深小数部分
	if (uz < 31 && lstSound.at(1).at(0) != '0')
	{
		strSoundName = strSoundName.left(6);
		strSoundName.append('5').append(lstSound.at(1).at(0));
		if (uz >= 10)
		{
			int offsetx = 2. / 25.4 * dpi * scale;
			QPoint pt(point.x() + offsetx, point.y());
			pointSymbol.renderSounding(painter, pt, strSoundName, scale, rotation);
		}
		else
		{
			int offsetx = 1. / 25.4 * dpi * scale;
			QPoint pt(point.x() + offsetx, point.y());
			pointSymbol.renderSounding(painter, pt, strSoundName, scale, rotation);
		}
	}

	if (z < 0) // 干出水深，绘制下划线
	{
		pointSymbol.renderSounding( painter, point, "SOUNDSA1", scale, rotation);
	}

	if (swept) // 扫海水深，绘制下方线
	{
		pointSymbol.renderSounding(painter, point, "SOUNDSB1", scale, rotation);
	}
}

void CS57CellRenderer::renderAnnotation(CellDisplayContext* pContext)
{
	renderAreaAnnotation(pContext);
	renderLineAnnotation(pContext);
	renderPointAnnotation(pContext);
}

void CS57CellRenderer::renderPointAnnotation(CellDisplayContext* pContext)
{
	CS57PointAnnotation PointAnnotation(m_pPresLib);

	CS57Cell* pCell = pContext->pS57Cell;
	double comf = pCell->getCellComf();
	double scale = m_pTransform->m_Viewport.getDisplayScale();
	double factor = getSymbolScaleFactor(pCell, scale);
	for (int i = 0; i < pContext->vecPFeatures.size(); i++)
	{
		QPoint renderPt;
		CS57RecFeature* pFE = pContext->vecPFeatures[i];
		if (pFE->m_Frid.objl == 129)
		{
			continue;

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
				float ve3d = (float)pCell->m_vecS57RecVI[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg3d.m_vecSg3d[j].ve3d;
				float somf = (float)pCell->getCellSomf();
				float z = ve3d / somf;
				renderSounding(m_pPainter, renderPt, factor, 0.0, z, false, false);

			}
			continue;
		}
		if (pFE->m_Frid.prim == 1 && pFE->m_Fspt.m_vecFspt[0].rcnm == 110)
		{
			if (pCell->m_vecS57RecVI[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d.size() != 1)
				continue;

			double lat_rad = pCell->m_vecS57RecVI[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d[0].ycoo / comf / RO;
			double lon_rad = pCell->m_vecS57RecVI[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d[0].xcoo / comf / RO;
			int dpx, dpy;
			double vpx, vpy;
			m_pTransform->m_Proj.getXY(lat_rad, lon_rad, &vpy, &vpx);
			m_pTransform->m_Viewport.convertVp2Dp(vpx, vpy, &dpx, &dpy);
			renderPt.setX(dpx);
			renderPt.setY(dpy);

			if(pFE->m_Frid.objl == 81)
				continue;
		}
		else if (pFE->m_Frid.prim == 1 && pFE->m_Fspt.m_vecFspt[0].rcnm == 120)
		{
			if (pCell->m_vecS57RecVC[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d.size() != 1)
				continue;

			double lat_rad = pCell->m_vecS57RecVC[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d[0].ycoo / comf / RO;
			double lon_rad = pCell->m_vecS57RecVC[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d[0].xcoo / comf / RO;
			int dpx, dpy;
			double vpx, vpy;
			m_pTransform->m_Proj.getXY(lat_rad, lon_rad, &vpy, &vpx);
			m_pTransform->m_Viewport.convertVp2Dp(vpx, vpy, &dpx, &dpy);
			renderPt.setX(dpx);
			renderPt.setY(dpy);
		}
		else
		{
			continue;
		}

		m_pPainter->save();
		for (int j = 0; j < pFE->m_vecInst.size(); j++)
		{
			if (pFE->m_vecInst[j]->tnam == m_strPSymbolMode)
			{
				PointAnnotation.setAnnoFeature(pFE);
				for (int k = 0; k < pFE->m_vecInst[j]->fieldINST.vecTE.size(); k++)
				{
					PointAnnotation.setTE(pFE->m_vecInst[j]->fieldINST.vecTE[k]);
					PointAnnotation.renderTE(m_pPainter, renderPt, factor);
				}
				for (int k = 0; k < pFE->m_vecInst[j]->fieldINST.vecTX.size();k++)
				{
					PointAnnotation.setTX(pFE->m_vecInst[j]->fieldINST.vecTX[k]);
					PointAnnotation.renderTX(m_pPainter, renderPt, factor);
				}
			}
		}
		m_pPainter->restore();
	}
}

void CS57CellRenderer::renderLineAnnotation(CellDisplayContext* pContext)
{
	CS57LineAnnotation LineAnnotation(m_pPresLib);

	CS57Cell* pCell = pContext->pS57Cell;
	double comf = pCell->getCellComf();
	double scale = m_pTransform->m_Viewport.getDisplayScale();
	double factor = getSymbolScaleFactor(pCell, scale);
	for (int i = 0; i < pContext->vecLFeatures.size(); i++)
	{
		CS57RecFeature* pFE = pContext->vecLFeatures[i];

		CPolylines Polylines;
		QVector<QVector<QPointF>*> vecPolyline;
		pCell->genFeaturePolyline(pFE, &vecPolyline);
		if (vecPolyline.size() == 0)
			return;

		for (int j = 0; j < vecPolyline.size(); j++)
		{
			CPolyline polyline;
			for (int k = 0; k < vecPolyline[j]->size(); k++)
			{
				double lon_rad = vecPolyline[j]->at(k).x() / RO;
				double lat_rad = vecPolyline[j]->at(k).y() / RO;

				double vpx, vpy;
				int dpx, dpy;
				m_pTransform->m_Proj.getXY(lat_rad, lon_rad, &vpy, &vpx);
				m_pTransform->m_Viewport.convertVp2Dp(vpx, vpy, &dpx, &dpy);
				QPoint pt(dpx, dpy);
				polyline.push_back(pt);
			}
			Polylines.push_back(polyline);
		}
		qDeleteAll(vecPolyline);
		vecPolyline.clear();

		for (int j = 0; j < pFE->m_vecInst.size(); j++)
		{
			if (pFE->m_vecInst[j]->tnam == m_strLSymbolMode)
			{
				LineAnnotation.setAnnoFeature(pFE);
				for (int k = 0; k < pFE->m_vecInst[j]->fieldINST.vecTE.size(); k++)
				{
					float angle = 0.0;
					QString strResultAnno = "";
					showTextTE* pTE = pFE->m_vecInst[j]->fieldINST.vecTE[k];
					QString formatstr = pTE->formatstr;
					QString strAttrb = pTE->attrb;
					S57AttributeValue* pAttrValue = m_pPresLib->getAttributeInfo(strAttrb);
					for (int m = 0; m < pFE->m_Attf.m_vecAttf.size(); m++)
					{
						if (pFE->m_Attf.m_vecAttf[m].attl == 117)
						{
							angle = pFE->m_Attf.m_vecAttf[m].atvl.toFloat();
						}

						char szText[100] = { '\0' };
						if (pFE->m_Attf.m_vecAttf[m].attl == pAttrValue->nFeaAttrCode)
						{
							int nVal = 0;
							float fVal = 0.0;
							QString strVal = "";
							switch (pAttrValue->eAttrType)
							{
							case E:		// enumerated 预期输入是从预定义属性值列表中选择的数字。 必须选择一个值。 此类型的缩写是“ E”。
								break;
							case L:		// list 预期的输入是一个从一个预定义属性值列表中选择的一个或多个数字的列表。 如果使用多个值，则通常必须用逗号分隔，但在特殊情况下，可以使用斜杠（“ /”）。 此类型的缩写是“ L”。
								break;
							case F:		// float 预期的输入是具有定义范围，分辨率，单位和格式的浮点数值。 此类型的缩写是“ F”。
								fVal = pFE->m_Attf.m_vecAttf[m].atvl.toFloat();
								sprintf(szText, (const char*)formatstr.toLocal8Bit(), fVal);
								break;
							case I:		// integer 预期的输入是具有定义范围，单位和格式的整数值。 此类型的缩写是“I”。
								nVal = pFE->m_Attf.m_vecAttf[m].atvl.toInt();
								sprintf(szText, (const char*)formatstr.toLocal8Bit(), nVal);
								break;
							case A:		// coded string 预期的输入是预定义格式的ASCII字符字符串。 信息是根据已定义的编码系统进行编码的，例如：国籍将由ISO 3166“国家名称表示代码”指定的两个字符字段进行编码。 加拿大=>“ CA”（请参阅S-57附录A附录A）。 此类型的缩写是“ A”。
							case S:		// free text 预期的输入是自由格式的字母数字字符串。 它可以是指向文本或图形文件的文件名。 此类型的缩写是“ S”。
								strVal = pFE->m_Attf.m_vecAttf[m].atvl;
								sprintf(szText, (const char*)formatstr.toLocal8Bit(), (const char*)strVal.toLocal8Bit());
								break;
							}
							int len = strlen(szText);
							szText[len] = '\0';
							string text = szText;
							QString strText = QString::fromLocal8Bit(text.c_str());
							strResultAnno = strText;
						}
					}
					LineAnnotation.setTE(pFE->m_vecInst[j]->fieldINST.vecTE[k]);
					LineAnnotation.renderTE(m_pPainter, Polylines, strResultAnno, angle, factor);
				}
				for (int k = 0; k < pFE->m_vecInst[j]->fieldINST.vecTX.size(); k++)
				{
					float angle = 0.0;
					QString strResultAnno = "";
					showTextTX* pTX = pFE->m_vecInst[j]->fieldINST.vecTX[k];
					if (pTX->str.length() == 6)
					{
						S57AttributeValue* pAttrValue = m_pPresLib->getAttributeInfo(pTX->str);
						if (pAttrValue == nullptr)
							break;

						for (int m = 0; m < pFE->m_Attf.m_vecAttf.size(); m++)
						{
							if (pFE->m_Attf.m_vecAttf[m].attl == pAttrValue->nFeaAttrCode)
							{
								int nVal = 0;
								float fVal = 0.0;
								QString strVal = "";
								switch (pAttrValue->eAttrType)
								{
								case E:		// enumerated 预期输入是从预定义属性值列表中选择的数字。 必须选择一个值。 此类型的缩写是“ E”。
									break;
								case L:		// list 预期的输入是一个从一个预定义属性值列表中选择的一个或多个数字的列表。 如果使用多个值，则通常必须用逗号分隔，但在特殊情况下，可以使用斜杠（“ /”）。 此类型的缩写是“ L”。
								{
									QString strCName = "";
									QString strSource = pFE->m_Attf.m_vecAttf[m].atvl;
									QStringList lstVal = strSource.split(",");
									if (lstVal.size() == 0)
										continue;

									if (lstVal.size() == 1)
									{
										strCName = m_pPresLib->getS57AttrExpValueInfo(pAttrValue->nFeaAttrCode, lstVal[0].toUShort())->strCName;
									}
									else
									{
										strCName = m_pPresLib->getS57AttrExpValueInfo(pAttrValue->nFeaAttrCode, lstVal[0].toUShort())->strCName;
										for (int j = 1; j < lstVal.size(); j++)
										{
											strCName = m_pPresLib->getS57AttrExpValueInfo(pAttrValue->nFeaAttrCode, lstVal[j].toUShort())->strCName;
											strCName += "," + strCName;
										}
									}
									strResultAnno = strCName;
									lstVal.clear();
									break;
								}
								case F:		// float 预期的输入是具有定义范围，分辨率，单位和格式的浮点数值。 此类型的缩写是“ F”。
									fVal = pFE->m_Attf.m_vecAttf[m].atvl.toFloat();
									strResultAnno = QString("%1").arg(fVal);
									break;
								case I:		// integer 预期的输入是具有定义范围，单位和格式的整数值。 此类型的缩写是“I”。
									nVal = pFE->m_Attf.m_vecAttf[m].atvl.toInt();
									strResultAnno = QString("%1").arg(nVal);
									break;
								case A:		// coded string 预期的输入是预定义格式的ASCII字符字符串。 信息是根据已定义的编码系统进行编码的，例如：国籍将由ISO 3166“国家名称表示代码”指定的两个字符字段进行编码。 加拿大=>“ CA”（请参阅S-57附录A附录A）。 此类型的缩写是“ A”。
								case S:		// free text 预期的输入是自由格式的字母数字字符串。 它可以是指向文本或图形文件的文件名。 此类型的缩写是“ S”。
									strVal = pFE->m_Attf.m_vecAttf[m].atvl;
									strResultAnno = strVal;
									break;
								}
							}
						}
					}
					else
					{
						strResultAnno = pTX->str;
						strResultAnno = strResultAnno.mid(1, strResultAnno.length() - 2);
					}
					LineAnnotation.setTX(pFE->m_vecInst[j]->fieldINST.vecTX[k]);
					LineAnnotation.renderTX(m_pPainter, Polylines, strResultAnno, angle, factor);
				}
			}
		}
	}
}

void CS57CellRenderer::renderAreaAnnotation(CellDisplayContext* pContext)
{
	CS57AreaAnnotation AreaAnnotation(m_pPresLib);

	CS57Cell* pCell = pContext->pS57Cell;
	double comf = pCell->getCellComf();
	double scale = m_pTransform->m_Viewport.getDisplayScale();
	double factor = getSymbolScaleFactor(pCell, scale);
	for (int i = 0; i < pContext->vecAFeatures.size(); i++)
	{
		CS57RecFeature* pFE = pContext->vecAFeatures[i];

		if (pFE->m_Frid.objl >= 300 && pFE->m_Frid.objl <= 312)
			continue;
		if (pFE->m_Frid.prim != 3)
			continue;
		if (pFE->m_vecInst.size() == 0)
			continue;
		CPolylines Polygons;
		QVector<QVector<QPointF>*> vecPolgyon;
		pCell->genFeaturePolygon(pFE, &vecPolgyon);
		if (vecPolgyon.size() == 0)
			return;

		for (int j = 0; j < vecPolgyon.size(); j++)
		{
			CPolyline polygon;
			for (int k = 0; k < vecPolgyon[j]->size(); k++)
			{
				double lon_rad = vecPolgyon[j]->at(k).x() / RO;
				double lat_rad = vecPolgyon[j]->at(k).y() / RO;

				double vpx, vpy;
				int dpx, dpy;
				m_pTransform->m_Proj.getXY(lat_rad, lon_rad, &vpy, &vpx);
				m_pTransform->m_Viewport.convertVp2Dp(vpx, vpy, &dpx, &dpy);
				QPoint pt(dpx, dpy);
				polygon.push_back(pt);
			}
			Polygons.push_back(polygon);
		}
		qDeleteAll(vecPolgyon);
		vecPolgyon.clear();

		for (int j = 0; j < pFE->m_vecInst.size(); j++)
		{
			if (pFE->m_vecInst[j]->tnam == m_strASymbolMode)
			{
				for (int k = 0; k < pFE->m_vecInst[j]->fieldINST.vecTE.size(); k++)
				{
					float angle = 0.0;
					QString strResultAnno = "";
					showTextTE* pTE = pFE->m_vecInst[j]->fieldINST.vecTE[k];
					QString formatstr = pTE->formatstr;
					QString strAttrb = pTE->attrb;
					S57AttributeValue* pAttrValue = m_pPresLib->getAttributeInfo(strAttrb);
					if (pAttrValue == nullptr) 
						continue;

					QString str = pFE->getAttrValue(pAttrValue->nFeaAttrCode);

					unsigned short code;
					QString str1 = "";
					switch (pAttrValue->nFeaAttrCode)
					{
					case 102://INFORM
						code = 300;
						str1 = pFE->getAttrValue(code);
						if (str1.length() > 0)
							str = str1;
						break;
					case 116://OBJNAM
						code = 301;
						str1 = pFE->getAttrValue(code);
						if (str1.length() > 0)
							str = str1;
						break;
					}
					int nVal = 0;
					float fVal = 0.0;
					QString strVal = "";
					char szText[100] = { '\0' };
					switch (pAttrValue->eAttrType)
					{
					case E:		// enumerated 预期输入是从预定义属性值列表中选择的数字。 必须选择一个值。 此类型的缩写是“ E”。
						break;
					case L:		// list 预期的输入是一个从一个预定义属性值列表中选择的一个或多个数字的列表。 如果使用多个值，则通常必须用逗号分隔，但在特殊情况下，可以使用斜杠（“ /”）。 此类型的缩写是“ L”。
						break;
					case F:		// float 预期的输入是具有定义范围，分辨率，单位和格式的浮点数值。 此类型的缩写是“ F”。
						if (str.length() == 0) 
							break;
						fVal = str.toFloat();
						sprintf(szText, (const char*)formatstr.toLocal8Bit(), fVal);
						strResultAnno.append(szText);
						break;
					case I:		// integer 预期的输入是具有定义范围，单位和格式的整数值。 此类型的缩写是“I”。
						nVal = str.toInt();
						sprintf(szText, (const char*)formatstr.toLocal8Bit(), nVal);
						strResultAnno.append(szText);
						break;
					case A:		// coded string 预期的输入是预定义格式的ASCII字符字符串。 信息是根据已定义的编码系统进行编码的，例如：国籍将由ISO 3166“国家名称表示代码”指定的两个字符字段进行编码。 加拿大=>“ CA”（请参阅S-57附录A附录A）。 此类型的缩写是“ A”。
					case S:		// free text 预期的输入是自由格式的字母数字字符串。 它可以是指向文本或图形文件的文件名。 此类型的缩写是“ S”。
						sprintf(szText, (const char*)formatstr.toLocal8Bit(), (const char*)str.toLocal8Bit());
						strResultAnno.append(szText);
						break;
					}
					if (strResultAnno.length() == 0)
						continue;

					unsigned short attl = 117;
					str1 = pFE->getAttrValue(attl);
					if (str1.length() > 0)
						angle = str1.toFloat();

					unsigned short objl = pFE->m_Frid.objl;
					AreaAnnotation.setTE(pFE->m_vecInst[j]->fieldINST.vecTE[k]);
					AreaAnnotation.renderTE(m_pPainter, Polygons, objl, strResultAnno, angle, factor);
				}
				for (int k = 0; k < pFE->m_vecInst[j]->fieldINST.vecTX.size(); k++)
				{
					
					float angle = 0.0;
					QString strResultAnno = "";
					showTextTX* pTX = pFE->m_vecInst[j]->fieldINST.vecTX[k];
					S57AttributeValue* pAttrValue = m_pPresLib->getAttributeInfo(pTX->str);
					if (pAttrValue == nullptr)
						continue;

					QString str = pFE->getAttrValue(pAttrValue->nFeaAttrCode);

					int pos = 0;
					QString str1 = "";
					unsigned short code;
					switch (pAttrValue->nFeaAttrCode)
					{
					case 113:
						do
						{
							int value = str.toInt();
							switch (value)
							{
							case 1:
								str1.append("泥");
								break;
							case 3:
								str1.append("淤泥");
								break;
							case 4:
								str1.append("沙");
								break;
							case 6:
								str1.append("砾");
								break;
							case 7:
								str1.append("磊石");
								break;
							case 8:
								str1.append("中石");
								break;
							case 9:
								str1.append("岩");
								break;
							case 11:
								str1.append("熔岩");
								break;
							case 14:
								str1.append("珊瑚");
								break;
							case 17:
								str1.append("贝壳");
								break;
							case 18:
								str1.append("圆石");
								break;
							case 19:
								str1.append("岩礁");
								break;
							}
							pos = str.indexOf(",");
							str = str.mid(1, str.length() - pos);

						} while (pos != -1);
						str = str1;
						str1 = "";
						break;
					case 102://INFORM
						code = 300;
						str1 = pFE->getAttrValue(code);
						if (str1.length() > 0)
							str = str1;
						break;
					case 116://OBJNAM
						code = 301;
						str1 = pFE->getAttrValue(code);
						if (str1.length() > 0)
							str = str1;
						break;
					}
					strResultAnno = str;
					if (strResultAnno.length() == 0)
						continue;

					unsigned short attl = 117;
					str1 = pFE->getAttrValue(attl);
					if (str1.length() > 0)
						angle = str1.toFloat();

					unsigned short objl = pFE->m_Frid.objl;
					AreaAnnotation.setTX(pFE->m_vecInst[j]->fieldINST.vecTX[k]);
					AreaAnnotation.renderTX(m_pPainter, Polygons, objl, strResultAnno, angle, factor);
				}
			}
		}
	}
}

void CS57CellRenderer::renderCompass(sCompassInfo& compassInfo)
{
	int centerX = compassInfo.centerPt.x();
	int centerY = compassInfo.centerPt.y();
	//绘制方位圈
	m_pPainter->save();
	float dpi = m_pPainter->device()->physicalDpiX();
	double scale = m_pTransform->m_Viewport.getDisplayScale();
	double relScale = compassInfo.dCscl / scale;

	double mmMin = 1 / 25.4 * dpi * relScale;
	float penWidth = 1.7 * relScale;//0.1 / 25.4 * dpi / mmMin;
	float radius = 69 * mmMin;;//方位圈半径
	float minus = radius / 34.5;
	float minScaleL, scale5L, scale10L;
	minScaleL = minus;      //方位圈最小刻度长 
	scale5L = 1.5 * minus;  //方位圈5度刻度长
	scale10L = 2 * minus;   //方位圈10度刻度长
	float centerCmm = 0.5 * minus;    //方位圈中心圆半径长
	float centerCCmm = 0.05 * mmMin;  //方位圈圆心半径

	//设置经纬度绘制方位圈
	QPen normalPen;
	normalPen.setColor(QColor(255, 0, 255));
	normalPen.setWidthF(0.1 * mmMin);
	m_pPainter->setPen(normalPen);

	m_pPainter->save();
	QFont fontMagnetic;
	fontMagnetic.setFamily("黑体");
	fontMagnetic.setPixelSize(minus);
	m_pPainter->setBrush(Qt::white);
	m_pPainter->drawEllipse(QPointF(centerX, centerY), centerCmm, centerCmm);
	m_pPainter->setBrush(QColor(255, 0, 255));
	m_pPainter->drawEllipse(QPointF(centerX, centerY), centerCCmm, centerCCmm);
	m_pPainter->translate(centerX, centerY);
	for (int i = 0; i < 360; i++)
	{
		m_pPainter->save();
		if (i % 5 != 0)
		{
			//小刻度
			m_pPainter->rotate(i);
			m_pPainter->drawLine(QPointF(0, -radius), QPointF(0, -(radius - minScaleL)));
		}
		else if (i % 5 == 0 && i % 10 != 0)
		{		
			//5度刻度
			m_pPainter->rotate(i);
			m_pPainter->drawLine(QPointF(0, -radius), QPointF(0, -(radius - scale5L)));
		}
		else
		{	
			//10度刻度，并添加标签
			m_pPainter->rotate(i);
			m_pPainter->drawLine(QPointF(0, -radius), QPointF(0, -(radius - scale10L)));
			m_pPainter->setFont(fontMagnetic);
			QFontMetricsF fmCompass(fontMagnetic);
			float wCompassTag = fmCompass.width(QString::number(i));
			QString strTags = QString::number(i);
			if (i < 100 && i != 0)
			{
				strTags = "0" + strTags;
			}
			m_pPainter->drawText(QPointF(0 - wCompassTag / 2, -(radius + 0.5)), strTags);
		}
		m_pPainter->restore();
		//绘制四条长线
		m_pPainter->save();
		if (i % 90 == 0)
		{
			m_pPainter->rotate(i);
			m_pPainter->drawLine(QPointF(0, -centerCmm), QPointF(0, -31 * minScaleL));
		}
		m_pPainter->restore();
	}
	m_pPainter->restore();

	//绘制磁差标记与磁差指针
	m_pPainter->save();
	QFont font;
	font.setFamily("黑体");
	font.setPixelSize(2 * minus);
	m_pPainter->translate(centerX, centerY);

	//年变率
	m_pPainter->save();
	QString strAnnual = compassInfo.strAnnual;
	float fAnnual = compassInfo.strAnnual.toFloat();
	if (fAnnual > 0)
	{
		strAnnual += "′E";
	}
	else
	{
		strAnnual += "′W";
	}
	QString strADiff = "(" + strAnnual + ")";
	QFontMetricsF fmADiff(font);
	float wADiff = fmADiff.width(strADiff);
	float hADiff = fmADiff.height();
	m_pPainter->drawText(QPointF((centerCmm + 31 * minScaleL) / 2 - wADiff / 2, -hADiff / 3), strADiff);
	m_pPainter->restore();

	//推算年份、磁差
	m_pPainter->save();
	QString strMag = "";
	float fMag = compassInfo.strMagnetic.toFloat();
	if (fMag > 0)
	{
		if (compassInfo.strMagnetic.contains("."))
		{
			double dMag = compassInfo.strMagnetic.toDouble();
			int magD = dMag;
			int magM = (dMag - magD) * 60;
			strMag = QString("%1°%2′E").arg(QString::number(magD)).arg(QString::number(magM));
		}
		else
		{
			double dMag = compassInfo.strMagnetic.toDouble();
			int magD = dMag;
			strMag = QString("%1°E").arg(QString::number(magD));
		}
	}
	else
	{
		if (compassInfo.strMagnetic.contains("."))
		{
			double dMag = abs(compassInfo.strMagnetic.toDouble());
			int magD = dMag;
			int magM = (dMag - magD) * 60;
			strMag = QString("%1°%2′W").arg(QString::number(magD)).arg(QString::number(magM));
		}
		else
		{
			double dMag = compassInfo.strMagnetic.toDouble();
			int magD = dMag;
			strMag = QString("%1°W").arg(QString::number(magD));
		}
	}

	QString strMandY = strMag + "(" + compassInfo.strYear + ")";
	QFontMetricsF fmMandY(font);
	float wMandY = fmMandY.width(strMandY);
	float hMandY = fmMandY.height();
	m_pPainter->drawText(QPointF(-(centerCmm + 31 * minScaleL) / 2 - wMandY / 2, -hMandY / 3), strMandY);
	m_pPainter->restore();

	//指针
	m_pPainter->save();
	//int dnum = compassInfo.strMagnetic.indexOf("°");
	//int fnum = compassInfo.strMagnetic.indexOf("′");
	//int mnum = compassInfo.strMagnetic.indexOf("″");
	//int d = compassInfo.strMagnetic.mid(0, dnum).toInt();
	//int f = compassInfo.strMagnetic.mid(dnum + 1, fnum - dnum - 1).toInt();
	//float m = compassInfo.strMagnetic.mid(fnum + 1, mnum - fnum - 1).toFloat();
	//float magAngle = d + f / 60. + m / 3600.;
	//if (compassInfo.strMagnetic.contains("W"))
	//{
	//	magAngle = -magAngle;
	//}
	float magAngle = compassInfo.strMagnetic.toFloat();
	m_pPainter->rotate(magAngle);
	m_pPainter->drawLine(QPointF(0, -centerCmm), QPointF(0, -32 * minScaleL));
	if (magAngle < 0)
	{
		QPointF points[4] = {
			QPointF(0, -(32 * minScaleL - 4 * minus)),
			QPointF(-0.5*minus, -(32 * minScaleL - 3.75 * minus)),
			QPointF(-1 * minus, -(32 * minScaleL - 3.75 * minus)),
			QPointF(0, -32 * minScaleL),
		};
		m_pPainter->drawPolyline(points, 4);
	}
	else
	{
		QPointF points[4] = {
			QPointF(0, -(32 * minScaleL - 4 * minus)),
			QPointF(0.5*minus, -(32 * minScaleL - 3.75 * minus)),
			QPointF(1 * minus, -(32 * minScaleL - 3.75 * minus)),
			QPointF(0, -32 * minScaleL),
		};
		m_pPainter->drawPolyline(points, 4);
	}
	m_pPainter->restore();

	m_pPainter->restore();
	//绘制上方五角星（半径0.25cm，距离圆0.9cm）和竖线
	//竖线
	m_pPainter->save();
	m_pPainter->translate(centerX, centerY);
	m_pPainter->drawLine(QPointF(0, -(radius + 2 * minus)), QPointF(0, -(radius + 4.5*minus)));
	m_pPainter->restore();
	//五角星
	double largeCircleR, smallCircleR;//大圆小圆半径
	largeCircleR = 1.25 * minus;
	smallCircleR = (cos(72.0 * 2.0 * PI / 360.0) / cos(36.0 * 2.0 * PI / 360.0)) * largeCircleR;
	//绘制黑色填充部分
	m_pPainter->save();
	m_pPainter->translate(centerX, centerY - (radius + 4.5*minus));
	for (int i = 0; i < 360; i = i + 360 / 5)
	{
		m_pPainter->save();
		m_pPainter->rotate(i);
		QPainterPath path;
		QPolygonF polygon;
		polygon.push_back(QPointF(0, 0));
		polygon.push_back(QPointF(0, -largeCircleR));
		polygon.push_back(QPointF(smallCircleR*cos((90 - 36)*PI / 180), -smallCircleR * sin((90 - 36)*PI / 180)));
		path.addPolygon(polygon);
		m_pPainter->setBrush(QColor(255, 0, 255));
		m_pPainter->drawPath(path);
		m_pPainter->restore();
	}
	//绘制白色部分
	for (int i = 0; i < 360; i = i + 360 / 5)
	{
		m_pPainter->save();
		m_pPainter->rotate(i);
		QPainterPath path;
		QPolygonF polygon;
		polygon.push_back(QPointF(0, 0));
		polygon.push_back(QPointF(largeCircleR*cos((90 - 72)*PI / 180), -largeCircleR * sin((90 - 72)*PI / 180)));
		polygon.push_back(QPointF(smallCircleR*cos((90 - 36)*PI / 180), -smallCircleR * sin((90 - 36)*PI / 180)));
		path.addPolygon(polygon);
		QPen pStar(QColor(255, 0, 255));
		pStar.setWidthF(0.1 * mmMin);
		m_pPainter->setPen(pStar);
		m_pPainter->setBrush(Qt::white);
		m_pPainter->drawPath(path);
		m_pPainter->restore();
	}
	m_pPainter->restore();
	m_pPainter->restore();
}

void CS57CellRenderer::renderBorder(CellDisplayContext* pContext)
{
	CS57Cell* pCell = pContext->pS57Cell;
	switch (m_eRenderType)
	{
	case XT::RENDER_DISPLAY:
	{
		CS57CellBorderRenderer* pRender = new CS57CellBorderRenderer(pCell);
		pRender->doRender(m_pPainter);

		delete pRender;
		pRender = nullptr;

		m_eRenderType = XT::RENDER_DISPLAY;
		break;
	}
	case XT::RENDER_PRINT:
	{
		if (m_pPrintParameters == nullptr)
			return;

		m_pPainter->setClipping(false);
		CS57CellBorderRenderer* pRender = new CS57CellBorderRenderer(m_pPrintParameters);
		pRender->doRender(m_pPainter);
		m_pPainter->setClipping(true);

		delete pRender;
		pRender = nullptr;

		m_eRenderType = XT::RENDER_DISPLAY;
		break;
	}
	default:
	{
		CS57CellBorderRenderer* pRender = new CS57CellBorderRenderer(pCell);
		pRender->doRender(m_pPainter);

		delete pRender;
		pRender = nullptr;

		m_eRenderType = XT::RENDER_DISPLAY;
		break;
	}
	}

}

void CS57CellRenderer::setBorderVisible(bool visible)
{
	m_bBorderVisible = visible;
}

void CS57CellRenderer::setSymbolScaleMode(XT::SymbolScaleMode mode)
{
	m_eCurSymbolScaleMode = mode;
}

float CS57CellRenderer::getSymbolScaleFactor(CS57Cell* pCell, double displayScale)
{
	float factor = 1.0;
	switch (m_eCurSymbolScaleMode)
	{
	case XT::ABS_SCALE:
		factor = 1;
		break;
	case XT::REL_SCALE:
		factor = (float)pCell->getCellCscl() / (float)displayScale;
		break;
	}

	return factor;
}

void CS57CellRenderer::setRenderType(XT::RenderType type)
{
	m_eRenderType = type;
}

void CS57CellRenderer::setPrintParameters(XT::sExportParameters* pPara)
{
	m_pPrintParameters = pPara;
}

CS57McovrRegionRenderer::CS57McovrRegionRenderer()
{}

CS57McovrRegionRenderer::~CS57McovrRegionRenderer()
{}

void CS57McovrRegionRenderer::setVisible(McovrRegion r, bool visible)
{
	switch (r)
	{
	case Core::CS57McovrRegionRenderer::Region_I:
		m_bVisibleI = visible;
		break;
	case Core::CS57McovrRegionRenderer::Region_II:
		m_bVisibleII = visible;
		break;
	case Core::CS57McovrRegionRenderer::Region_III:
		m_bVisibleIII = visible;
		break;
	case Core::CS57McovrRegionRenderer::Region_IV:
		m_bVisibleIV = visible;
		break;
	case Core::CS57McovrRegionRenderer::Region_V:
		m_bVisibleV = visible;
		break;
	}
}

void CS57McovrRegionRenderer::setLineWidth(McovrRegion r, int w)
{
	switch (r)
	{
	case Core::CS57McovrRegionRenderer::Region_I:
		m_nLineWidthI = w;
		break;
	case Core::CS57McovrRegionRenderer::Region_II:
		m_nLineWidthII = w;
		break;
	case Core::CS57McovrRegionRenderer::Region_III:
		m_nLineWidthIII = w;
		break;
	case Core::CS57McovrRegionRenderer::Region_IV:
		m_nLineWidthIV = w;
		break;
	case Core::CS57McovrRegionRenderer::Region_V:
		m_nLineWidthV = w;
		break;
	}
}

int CS57McovrRegionRenderer::getLineWidth(McovrRegion r) const
{
	switch (r)
	{
	case Core::CS57McovrRegionRenderer::Region_I:
		return m_nLineWidthI;
		break;
	case Core::CS57McovrRegionRenderer::Region_II:
		return m_nLineWidthII;
		break;
	case Core::CS57McovrRegionRenderer::Region_III:
		return m_nLineWidthIII;
		break;
	case Core::CS57McovrRegionRenderer::Region_IV:
		return m_nLineWidthIV;
		break;
	case Core::CS57McovrRegionRenderer::Region_V:
		return m_nLineWidthV;
		break;
	}

	return 0;
}

void CS57McovrRegionRenderer::setLineColor(McovrRegion r, const QColor& color)
{
	switch (r)
	{
	case Core::CS57McovrRegionRenderer::Region_I:
		m_LineColorI = color;
		break;
	case Core::CS57McovrRegionRenderer::Region_II:
		m_LineColorII = color;
		break;
	case Core::CS57McovrRegionRenderer::Region_III:
		m_LineColorIII = color;
		break;
	case Core::CS57McovrRegionRenderer::Region_IV:
		m_LineColorIV = color;
		break;
	case Core::CS57McovrRegionRenderer::Region_V:
		m_LineColorV = color;
		break;
	}
}

QColor CS57McovrRegionRenderer::getLineColor(McovrRegion r) const
{
	switch (r)
	{
	case Core::CS57McovrRegionRenderer::Region_I:
		return m_LineColorI;
		break;
	case Core::CS57McovrRegionRenderer::Region_II:
		return m_LineColorII;
		break;
	case Core::CS57McovrRegionRenderer::Region_III:
		return m_LineColorIII;
		break;
	case Core::CS57McovrRegionRenderer::Region_IV:
		return m_LineColorIV;
		break;
	case Core::CS57McovrRegionRenderer::Region_V:
		return m_LineColorV;
		break;
	}

	return QColor();
}

void CS57McovrRegionRenderer::setFillColor(McovrRegion r, const QColor& color)
{
	switch (r)
	{
	case Core::CS57McovrRegionRenderer::Region_I:
		m_FillColorI = color;
		break;
	case Core::CS57McovrRegionRenderer::Region_II:
		m_FillColorII = color;
		break;
	case Core::CS57McovrRegionRenderer::Region_III:
		m_FillColorIII = color;
		break;
	case Core::CS57McovrRegionRenderer::Region_IV:
		m_FillColorIV = color;
		break;
	case Core::CS57McovrRegionRenderer::Region_V:
		m_FillColorV = color;
		break;
	}
}

QColor CS57McovrRegionRenderer::getFillColor(McovrRegion r) const
{
	switch (r)
	{
	case Core::CS57McovrRegionRenderer::Region_I:
		return m_FillColorI;
		break;
	case Core::CS57McovrRegionRenderer::Region_II:
		return m_FillColorII;
		break;
	case Core::CS57McovrRegionRenderer::Region_III:
		return m_FillColorIII;
		break;
	case Core::CS57McovrRegionRenderer::Region_IV:
		return m_FillColorIV;
		break;
	case Core::CS57McovrRegionRenderer::Region_V:
		return m_FillColorV;
		break;
	}
	return QColor();
}

void CS57McovrRegionRenderer::setDataSource(CS57McovrRegionProvider* pMcovrRegionProvider)
{
	m_pProvider = pMcovrRegionProvider;
}

void CS57McovrRegionRenderer::doRender(QPainter* p)
{
	m_pPainter = p;

	m_pPainter->save();
	GpcPolygon* polygonV = m_pProvider->m_pResultGpcPolygonV;
	if (polygonV)
		renderMcovrRegion(Region_V, polygonV);

	GpcPolygon* polygonIV = m_pProvider->m_pResultGpcPolygonIV;
	if (polygonIV)
		renderMcovrRegion(Region_IV, polygonIV);

	GpcPolygon* polygonIII = m_pProvider->m_pResultGpcPolygonIII;
	if (polygonIII)
		renderMcovrRegion(Region_III, polygonIII);

	GpcPolygon* polygonII = m_pProvider->m_pResultGpcPolygonII;
	if (polygonII)
		renderMcovrRegion(Region_II, polygonII);

	GpcPolygon* polygonI = m_pProvider->m_pResultGpcPolygonI;
	if (polygonI)
		renderMcovrRegion(Region_I, polygonI);

	m_pPainter->restore();
}

void Core::CS57McovrRegionRenderer::renderMcovrRegion(McovrRegion r, GpcPolygon* polygon)
{
	QPen pen;
	QBrush brush;
	switch (r)
	{
	case Core::CS57McovrRegionRenderer::Region_I:
	{
		pen.setColor(m_LineColorI);
		pen.setWidth(m_nLineWidthI);
		m_pPainter->setPen(pen);
		brush.setStyle(Qt::SolidPattern);
		brush.setColor(m_FillColorI);
		break;
	}
	case Core::CS57McovrRegionRenderer::Region_II:
	{
		pen.setColor(m_LineColorII);
		pen.setWidth(m_nLineWidthII);
		m_pPainter->setPen(pen);
		brush.setStyle(Qt::SolidPattern);
		brush.setColor(m_FillColorII);
		break;
	}
	case Core::CS57McovrRegionRenderer::Region_III:
	{
		pen.setColor(m_LineColorIII);
		pen.setWidth(m_nLineWidthIII);
		m_pPainter->setPen(pen);
		brush.setStyle(Qt::SolidPattern);
		brush.setColor(m_FillColorIII);
		break;
	}
	case Core::CS57McovrRegionRenderer::Region_IV:
	{
		pen.setColor(m_LineColorIV);
		pen.setWidth(m_nLineWidthIV);
		m_pPainter->setPen(pen);
		brush.setStyle(Qt::SolidPattern);
		brush.setColor(m_FillColorIV);
		break;
	}
	case Core::CS57McovrRegionRenderer::Region_V:
	{
		pen.setColor(m_LineColorV);
		pen.setWidth(m_nLineWidthV);
		m_pPainter->setPen(pen);
		brush.setStyle(Qt::SolidPattern);
		brush.setColor(m_FillColorV);
		break;
	}
	}

	QPainterPath path;
	int num_contours = polygon->m_gpcPolygon.num_contours;
	int *hole = polygon->m_gpcPolygon.hole;
	for (int i = 0; i < num_contours; i++)
	{
		QPolygon pol;
		int num_vertices = polygon->m_gpcPolygon.contour[i].num_vertices;
		for (int j = 0; j < num_vertices; j++)
		{
			QPoint pt = m_pTransform->geo2Pixel(polygon->m_gpcPolygon.contour[i].vertex[j].x, polygon->m_gpcPolygon.contour[i].vertex[j].y);
			pol.append(pt);
		}
		path.addPolygon(pol);
		m_pPainter->drawPolygon(pol);
	}
	m_pPainter->fillPath(path, brush);
}

CS57CellBorderRenderer::CS57CellBorderRenderer(CS57Cell* pCell)
	: m_bVisible(true)
{
	sBorderInfo borderInfo;
	m_ChartInfo.chartName = pCell->getCellNameWithoutSuffix();		// 图名
	m_ChartInfo.chartNum = pCell->getCellNameWithoutSuffix();						// 图号
	CRectangle<double> rect = pCell->getCellExtent();
	m_ChartInfo.minLat = rect.yMinimum() / RO;									// 南图廓纬度　 （单位：弧度）
	m_ChartInfo.maxLat = rect.yMaximum() / RO;									// 北图廓纬度   （单位：弧度）
	m_ChartInfo.minLon = rect.xMinimum() / RO;									// 西图廓经度   （单位：弧度）
	m_ChartInfo.maxLon = rect.xMaximum() / RO;									// 东图廓经度   （单位：弧度）
	m_ChartInfo.csclScale = pCell->getCellCscl();								// 编辑比例尺   
	m_ChartInfo.projName = "MCT";												// 投影名称
	m_ChartInfo.baseLat = (rect.yMinimum() + rect.yMaximum()) / RO * 0.5;		// 基准纬线　	（单位：弧度）
	m_ChartInfo.midLon = (rect.xMinimum() + rect.xMaximum()) / RO * 0.5;		// 中央经线　	（单位：弧度）
	init();
}
CS57CellBorderRenderer::CS57CellBorderRenderer(XT::sExportParameters* pPara)
	: m_bVisible(true)
{
	m_ChartInfo.chartName = pPara->cellName;								// 图名
	m_ChartInfo.chartNum = pPara->cellNo;									// 图号
	//左下地理坐标点
	int lbLD = pPara->lbLD;
	int lbLM = pPara->lbLM;
	float lbLS = pPara->lbLS;
	int lbBD = pPara->lbBD;
	int lbBM = pPara->lbBM;
	float lbBS = pPara->lbBS;
	double lbLon = 0.0, lbLat = 0.0;
	m_pTransform->dms2Geo(lbLD, lbLM, lbLS, lbLon);
	m_pTransform->dms2Geo(lbBD, lbBM, lbBS, lbLat);
	QPointF lbPt(lbLon, lbLat);

	//右上地理坐标点
	int rtLD = pPara->rtLD;
	int rtLM = pPara->rtLM;
	float rtLS = pPara->rtLS;
	int rtBD = pPara->rtBD;
	int rtBM = pPara->rtBM;
	float rtBS = pPara->rtBS;
	double rtLon = 0.0, rtLat = 0.0;
	m_pTransform->dms2Geo(rtLD, rtLM, rtLS, rtLon);
	m_pTransform->dms2Geo(rtBD, rtBM, rtBS, rtLat);
	QPointF rtPt(rtLon, rtLat);

	m_ChartInfo.minLat = lbPt.y() / RO;									// 南图廓纬度　 （单位：弧度）
	m_ChartInfo.maxLat = rtPt.y() / RO;									// 北图廓纬度   （单位：弧度）
	m_ChartInfo.minLon = lbPt.x() / RO;									// 西图廓经度   （单位：弧度）
	m_ChartInfo.maxLon = rtPt.x() / RO;									// 东图廓经度   （单位：弧度）
	m_ChartInfo.csclScale = pPara->displayScale;								// 编辑比例尺   
	m_ChartInfo.projName = "MCT";												// 投影名称
	int baseLineBD = pPara->baseLineBD;
	int baseLineBM = pPara->baseLineBM;
	int baseLineBS = pPara->baseLineBS;
	QString strBaseLine = QString("%1.%2%3")
		.arg(QString::number(baseLineBD))
		.arg(QString::number(baseLineBM))
		.arg(QString::number(baseLineBS));
	m_ChartInfo.baseLat = fun_dmmssTorad(strBaseLine.toDouble());		// 基准纬线　	（单位：弧度）
	m_ChartInfo.midLon = (lbPt.x() + rtPt.x()) / RO * 0.5;		// 中央经线　	（单位：弧度）;
	init();
}
CS57CellBorderRenderer::~CS57CellBorderRenderer()
{}
void CS57CellBorderRenderer::init()
{
	double scale = m_ChartInfo.csclScale;
	double maxSubInterval = 0.0;
	double largerSubInterval = 0.0;
	double smallSubInterval = 0.0;
	double minSubInterval = 0.0;
	double lenBlackWhiteSection = 0.0;
	double lableIntercal = 0.0;
	double lineInterval = 0.0;

	if (scale < 30000)     // 比例尺大于1：30000
	{
		maxSubInterval = 1. / 60.;					// 最大细分间隔 (单位：度) １分
		largerSubInterval = 0.5;					// 较大细分间隔 (单位：分)　0.5分
		smallSubInterval = 6.;						// 较小细分间隔 (单位：秒)	6秒
		minSubInterval = 1.;						// 最小细分间隔 (单位：秒)	1秒
		lenBlackWhiteSection = 1.;					// 黑白节长度   (单位：分)
		lableIntercal = 1.;							// 标注间隔		(单位：分)
		lineInterval = (int)(scale / 10000. + 0.4);	// 经纬网间隔	(单位：分)
		if (lineInterval < 1)
		{
			lineInterval = 0.5;
			lableIntercal = 0.5;
		}
	}
	else if (scale < 50000)
	{
		largerSubInterval = 1.;						// 较大细分间隔 (单位：分)　1
		smallSubInterval = 30.;						// 较小细分间隔 (单位：秒)
		minSubInterval = 6.;						// 最小细分间隔 (单位：秒)
		lenBlackWhiteSection = 1.;					// 黑白节长度   (单位：分)
		lableIntercal = 1.;							// 标注间隔		(单位：分)
		lineInterval = (int)(scale / 10000.);		// 经纬网间隔	（单位：分）
		if (lineInterval == 4)
			lineInterval = 5;
		maxSubInterval = lineInterval / 60.;		// 最大细分间隔 (单位：度) １分

	}
	else if (scale < 100000)
	{
		//maxSubInterval = 1. / 60.;					// 最大细分间隔 (单位：度) １分
		largerSubInterval = 5.;						// 较大细分间隔 (单位：分)　1
		smallSubInterval = 30.;						// 较小细分间隔 (单位：秒)
		minSubInterval = 6.;						// 最小细分间隔 (单位：秒)
		lenBlackWhiteSection = 1.;					// 黑白节长度   (单位：分)
		lableIntercal = 5.;							// 标注间隔		(单位：分)
		lineInterval = 10;
		if (scale <= 60000) lineInterval = 5;
		maxSubInterval = lineInterval / 60.;		// 最大细分间隔 (单位：度) １分
	}
	else if (scale < 200000)
	{
		//maxSubInterval = 1. / 60.;					// 最大细分间隔 (单位：度) １分
		largerSubInterval = 5.;						// 较大细分间隔 (单位：分)　1
		smallSubInterval = 60.;						// 较小细分间隔 (单位：秒)
		minSubInterval = 12.;						// 最小细分间隔 (单位：秒)
		lenBlackWhiteSection = 1.;					// 黑白节长度   (单位：分)
		lableIntercal = 5.;							// 标注间隔		(单位：分)
		lineInterval = 20.;
		if (scale <= 160000) lineInterval = 10;
		maxSubInterval = lineInterval / 60.;		// 最大细分间隔 (单位：度) １分
	}
	else if (scale < 500000)
	{
		//maxSubInterval = 1. / 60.;					// 最大细分间隔 (单位：度) １分
		largerSubInterval = 5.;						// 较大细分间隔 (单位：分)　1
		smallSubInterval = 60.;						// 较小细分间隔 (单位：秒)
		minSubInterval = 30.;						// 最小细分间隔 (单位：秒)
		lenBlackWhiteSection = 1.;					// 黑白节长度   (单位：分)
		lableIntercal = 10.;							// 标注间隔		(单位：分)
		lineInterval = 30.;
		if (scale <= 300000) lineInterval = 20;
		maxSubInterval = lineInterval / 60.;		// 最大细分间隔 (单位：度) １分
	}
	else if (scale < 1500000)
	{
		maxSubInterval = 1.;					// 最大细分间隔 (单位：度) １分
		largerSubInterval = 10.;						// 较大细分间隔 (单位：分)　1
		smallSubInterval = 300.;						// 较小细分间隔 (单位：秒)
		minSubInterval = 60.;						// 最小细分间隔 (单位：秒)
		lenBlackWhiteSection = 5.;					// 黑白节长度   (单位：分)
		lableIntercal = 30.;							// 标注间隔		(单位：分)
		lineInterval = 60.;
	}
	else if (scale < 2250000)
	{
		maxSubInterval = 1.;					// 最大细分间隔 (单位：度) １分
		largerSubInterval = 30.;						// 较大细分间隔 (单位：分)　1
		smallSubInterval = 600.;						// 较小细分间隔 (单位：秒)
		minSubInterval = 120.;						// 最小细分间隔 (单位：秒)
		lenBlackWhiteSection = 10.;					// 黑白节长度   (单位：分)
		lableIntercal = 60.;							// 标注间隔		(单位：分)
		lineInterval = 300.;
	}
	else if (scale < 4750000)
	{
		maxSubInterval = 1.;					// 最大细分间隔 (单位：度) １分
		largerSubInterval = 60.;						// 较大细分间隔 (单位：分)　1
		smallSubInterval = 1800.;						// 较小细分间隔 (单位：秒)
		minSubInterval = 300.;						// 最小细分间隔 (单位：秒)
		lenBlackWhiteSection = 30.;					// 黑白节长度   (单位：分)
		lableIntercal = 60.;							// 标注间隔		(单位：分)
		lineInterval = 300.;
	}
	else if (scale <= 10000000)
	{
		maxSubInterval = 5.;					// 最大细分间隔 (单位：度) １分
		largerSubInterval = 300.;						// 较大细分间隔 (单位：分)　1
		smallSubInterval = 3600.;						// 较小细分间隔 (单位：秒)
		minSubInterval = 600.;						// 最小细分间隔 (单位：秒)
		lenBlackWhiteSection = 60.;					// 黑白节长度   (单位：分)
		lableIntercal = 300.;							// 标注间隔		(单位：分)
		lineInterval = 600.;
	}
	else if (scale <= 45000000)
	{
		maxSubInterval = 10.;					// 最大细分间隔 (单位：度) １分
		largerSubInterval = 600.;						// 较大细分间隔 (单位：分)　1
		smallSubInterval = 7200.;						// 较小细分间隔 (单位：秒)
		minSubInterval = 3600;						// 最小细分间隔 (单位：秒)
		lenBlackWhiteSection = 120.;					// 黑白节长度   (单位：分)
		lableIntercal = 600.;							// 标注间隔		(单位：分)
		lineInterval = 3600.;
	}
	else
	{
		// 该比例尺的图廓细分不在图式规定之中！！
	}

	m_BorderInfo.maxSubInterval = round(maxSubInterval * 3600.);
	m_BorderInfo.largerSubInterval = round(largerSubInterval * 60.);
	m_BorderInfo.smallSubInterval = round(smallSubInterval);
	m_BorderInfo.minSubInterval = round(minSubInterval);
	m_BorderInfo.blackWhiteSectionInterval = round(lenBlackWhiteSection * 60.);
	m_BorderInfo.lableIntercal = round(lableIntercal * 60.);
	m_BorderInfo.lineInterval = round(lineInterval * 60.);


	m_BorderInfo.lenin2Outline = 11.5;		// 内图廓距外图廓(内侧)的距离
	m_BorderInfo.lenin2labe = 4.25;
	m_BorderInfo.lenMaxSub = 3.5;
	m_BorderInfo.lenLargerSub = 2.0;
	m_BorderInfo.lenMinSub = 0.8;

	m_BorderInfo.widthBlackWhiteSection = 0.2;
	m_BorderInfo.widthOutLine = 1.0;

	if (scale < 30000)
	{
		m_BorderInfo.lenMaxSub = 3.0;
		m_BorderInfo.lenLargerSub = 1.5;
	}
	bool ismt = false;//　民图
	if (scale < 80000 && ismt)
	{
		if (scale < 12500)
		{
			m_BorderInfo.KmMaxInterval = 10;
			m_BorderInfo.KmMinInterval = 2;
			m_BorderInfo.KmMarkInterval = 50;
		}
		else if (scale < 30000)
		{
			m_BorderInfo.KmMaxInterval = 500;
			m_BorderInfo.KmMinInterval = 50;
			m_BorderInfo.KmMarkInterval = 1000;
		}
		else if (scale < 50000)
		{
			m_BorderInfo.KmMaxInterval = 1000;
			m_BorderInfo.KmMinInterval = 100;
			m_BorderInfo.KmMarkInterval = 1000;
		}
		else
		{
			m_BorderInfo.KmMaxInterval = 1000;
			m_BorderInfo.KmMinInterval = 200;
			m_BorderInfo.KmMarkInterval = 5000;
		}
	}
	else
	{
		if (scale < 1250)
		{
			m_BorderInfo.KmMaxInterval = 10;
			m_BorderInfo.KmMinInterval = 2;
			m_BorderInfo.KmMarkInterval = 50;
		}
		else if (scale < 3500)
		{
			m_BorderInfo.KmMaxInterval = 20;
			m_BorderInfo.KmMinInterval = 5;
			m_BorderInfo.KmMarkInterval = 100;
		}
		else if (scale < 6250)
		{
			m_BorderInfo.KmMaxInterval = 100;
			m_BorderInfo.KmMinInterval = 10;
			m_BorderInfo.KmMarkInterval = 200;
		}
		else if (scale < 12500)
		{
			m_BorderInfo.KmMaxInterval = 100;
			m_BorderInfo.KmMinInterval = 20;
			m_BorderInfo.KmMarkInterval = 500;
		}
		else if (scale < 35000)
		{
			m_BorderInfo.KmMaxInterval = 200;
			m_BorderInfo.KmMinInterval = 50;
			m_BorderInfo.KmMarkInterval = 1000;

		}
		else if (scale < 62500)
		{
			m_BorderInfo.KmMaxInterval = 1000;
			m_BorderInfo.KmMinInterval = 100;
			m_BorderInfo.KmMarkInterval = 2000;
		}
		else if (scale < 150000)
		{
			m_BorderInfo.KmMaxInterval = 1000;
			m_BorderInfo.KmMinInterval = 200;
			m_BorderInfo.KmMarkInterval = 5000;
		}
		else if (scale <= 350000)
		{
			m_BorderInfo.KmMaxInterval = 2000;
			m_BorderInfo.KmMinInterval = 500;
			m_BorderInfo.KmMarkInterval = 10000;
		}
		else if (scale <= 625000)
		{
			m_BorderInfo.KmMaxInterval = 10000;
			m_BorderInfo.KmMinInterval = 1000;
			m_BorderInfo.KmMarkInterval = 20000;
		}
		else if (scale <= 1500000)
		{
			m_BorderInfo.KmMaxInterval = 10000;
			m_BorderInfo.KmMinInterval = 2000;
			m_BorderInfo.KmMarkInterval = 50000;
		}
		else if (scale <= 4000000)
		{
			m_BorderInfo.KmMaxInterval = 20000;
			m_BorderInfo.KmMinInterval = 4000;
			m_BorderInfo.KmMarkInterval = 100000;
		}
		else if (scale > 4000000)
		{
			m_BorderInfo.KmMaxInterval = 100000;
			m_BorderInfo.KmMinInterval = 10000;
			m_BorderInfo.KmMarkInterval = 200000;
		}
		else
		{
			// 该比例尺的公里尺细分不在图式规定之中！！
		}

	}
	m_BorderInfo.wKmMax = 1.8;
	m_BorderInfo.wKmMid = 1.0;
	m_BorderInfo.wKmMin = 0.6;
}


void CS57CellBorderRenderer::setVisible(bool visible)
{
	m_bVisible = visible;
}

void CS57CellBorderRenderer::doRender(QPainter* p)
{
	if (!m_bVisible) return;

	m_pPainter = p;

	renderBorder(p);
}

void CS57CellBorderRenderer::getBorderInnerSize(double * Lmm, double *Vmm)
{
	calcInnerCoors();
	*Lmm = (PointRB.x() - PointLB.x()) / m_ChartInfo.csclScale *1000.;
	*Vmm = (PointLT.y() - PointLB.y()) / m_ChartInfo.csclScale *1000.;
}

void CS57CellBorderRenderer::splitDMMSS(double dmmss, QString &Degree, QString &Minute, QString &second)
{
	Degree.clear();
	Minute.clear();
	second.clear();
	QString strDMS = QString::number(dmmss, 'g.', 4);

	second = strDMS.right(2);
	Minute = strDMS.right(4).left(2);
	Degree = strDMS.left(strDMS.length() - 5);
}

void CS57CellBorderRenderer::renderBorder(QPainter* painter)
{
	double RelativeScale = m_ChartInfo.csclScale / m_pTransform->m_Viewport.getDisplayScale();
	double mm2dp = m_pTransform->m_Viewport.m_dXdpi * RelativeScale / 25.4;
	int dt0 = m_BorderInfo.lenin2Outline * mm2dp;
	int dt1 = m_BorderInfo.lenMaxSub * mm2dp;
	int dt2 = m_BorderInfo.lenLargerSub * mm2dp;
	int dt3 = m_BorderInfo.lenMinSub * mm2dp;
	int dtbw = 1.4  * mm2dp;

	int penWidth = 0.1 * mm2dp;
	QPen pen, pen1, pen2, pen3;
	pen.setWidth(penWidth);
	pen.setColor(QColor(0, 0, 0));
	pen1.setWidth(0.2 * mm2dp);
	pen1.setCapStyle(Qt::FlatCap);
	pen1.setColor(QColor(0, 0, 0));
	pen2.setWidth(1.0 *mm2dp);
	pen2.setCapStyle(Qt::FlatCap);
	pen2.setJoinStyle(Qt::MiterJoin);
	pen2.setColor(QColor(0, 0, 0));
	pen3.setWidth(0.5*mm2dp);
	pen3.setCapStyle(Qt::FlatCap);
	painter->save();
	painter->setPen(pen);

	QFont font1("汉仪中等线");		// 中等线体
	QFont font2("右斜中黑");		// 右斜等线体
	QFont font3("汉仪书宋二");		// 细宋
	QFont font4("汉仪中宋");		// 宋体
	QFont font5("长书宋二");		// 长细宋
	QFont font6("汉仪中黑");		// 等线体
	//QFont font6("右斜中黑");		// 右斜等线体
	//QFont font7("汉仪中等线");		// 中等线体
	//QFont font8("右斜中黑");		// 右斜等线体

	QFontMetricsF *fontMetrics;

	double xcoor, ycoor;
	{// 绘制内图廓线
		QVector<QPoint> polyline;
		polyline.clear();

		// 计算内图廓四个角点的设备坐标
		int x1, y1, x2, y2, x3, y3, x4, y4;
		double xcoor1, xcoor2, xcoor3, xcoor4, ycoor1, ycoor2, ycoor3, ycoor4;
		m_pTransform->m_Proj.getXY(m_ChartInfo.minLat, m_ChartInfo.minLon, &ycoor1, &xcoor1);
		m_pTransform->m_Viewport.convertVp2Dp(xcoor1, ycoor1, &x1, &y1);
		m_pTransform->m_Proj.getXY(m_ChartInfo.maxLat, m_ChartInfo.minLon, &ycoor2, &xcoor2);
		m_pTransform->m_Viewport.convertVp2Dp(xcoor2, ycoor2, &x2, &y2);
		m_pTransform->m_Proj.getXY(m_ChartInfo.maxLat, m_ChartInfo.maxLon, &ycoor3, &xcoor3);
		m_pTransform->m_Viewport.convertVp2Dp(xcoor3, ycoor3, &x3, &y3);
		m_pTransform->m_Proj.getXY(m_ChartInfo.minLat, m_ChartInfo.maxLon, &ycoor4, &xcoor4);
		m_pTransform->m_Viewport.convertVp2Dp(xcoor4, ycoor4, &x4, &y4);

		// 根据比例尺，选择内图廓绘制
		painter->save();
		painter->setPen(pen);

		painter->drawLine(x1, y1 + dt2, x2, y2 - dt2);
		painter->drawLine(x2 - dt2, y2, x3 + dt2, y3);
		painter->drawLine(x3, y3 - dt2, x4, y4 + dt2);
		painter->drawLine(x1 - dt2, y1, x4 + dt2, y4);
		if (m_ChartInfo.csclScale > 30000)
		{
			painter->drawLine(x1, y1, x1 - dt2, y1 + dt2);
			painter->drawLine(x2, y2, x2 - dt2, y2 - dt2);
			painter->drawLine(x3, y3, x3 + dt2, y3 - dt2);
			painter->drawLine(x4, y4, x4 + dt2, y4 + dt2);

			polyline.clear();
			polyline.push_back(QPoint(x1 - dt3, y1 + dt3));
			polyline.push_back(QPoint(x2 - dt3, y2 - dt3));
			polyline.push_back(QPoint(x3 + dt3, y3 - dt3));
			polyline.push_back(QPoint(x4 + dt3, y4 + dt3));
			polyline.push_back(QPoint(x1 - dt3, y1 + dt3));
			painter->drawPolyline(polyline);

			polyline.clear();
			polyline.push_back(QPoint(x1 - dt2, y1 + dt2));
			polyline.push_back(QPoint(x2 - dt2, y2 - dt2));
			polyline.push_back(QPoint(x3 + dt2, y3 - dt2));
			polyline.push_back(QPoint(x4 + dt2, y4 + dt2));
			polyline.push_back(QPoint(x1 - dt2, y1 + dt2));
			painter->drawPolyline(polyline);
		}
		painter->setPen(pen2);
		QPoint line[4];
		line[0] = QPoint(x1 - (dt0 + 0.5*mm2dp), y1 + dt2);
		line[1] = QPoint(x1 - (dt0 + 0.5*mm2dp), y1 + (dt0 + 0.5*mm2dp));
		line[2] = QPoint(x4 + (dt0 + 0.5*mm2dp), y4 + (dt0 + 0.5*mm2dp));
		line[3] = QPoint(x4 + (dt0 + 0.5*mm2dp), y4 + dt2);
		painter->drawPolyline(line, 4);
		line[0] = QPoint(x2 - (dt0 + 0.5*mm2dp), y2 - dt2);
		line[1] = QPoint(x2 - (dt0 + 0.5*mm2dp), y2 - (dt0 + 0.5*mm2dp));
		line[2] = QPoint(x3 + (dt0 + 0.5*mm2dp), y3 - (dt0 + 0.5*mm2dp));
		line[3] = QPoint(x4 + (dt0 + 0.5*mm2dp), y3 - dt2);
		painter->drawPolyline(line, 4);
		painter->restore();
		// 绘制图号
		{
			double annox;
			double annoy;
			QString cellNumber = m_ChartInfo.chartNum.left(8).right(5);
			font1.setPixelSize(6.5*mm2dp); // 28K
			fontMetrics = new QFontMetricsF(font1);
			QRectF rect = fontMetrics->boundingRect(cellNumber);
			delete fontMetrics;
			fontMetrics = nullptr;
			annox = x1 - dt0 - 6.9*mm2dp - rect.height();
			annoy = y1 + dt0 + 0.4*mm2dp - rect.width();
			renderAnno(painter, font1, annox, annoy, 90., cellNumber);

			annox = x2 - dt0 - 0.4 *mm2dp + rect.width();
			annoy = y2 - dt0 - 4.4 *mm2dp - rect.height();
			renderAnno(painter, font1, annox, annoy, 180., cellNumber);

			annox = x3 + dt0 + 6.9*mm2dp + rect.height();
			annoy = y3 - dt0 + 0.4*mm2dp + rect.width();
			renderAnno(painter, font1, annox, annoy, -90., cellNumber);

			annox = x4 + dt0 + 0.4 *mm2dp - rect.width();
			annoy = y4 + dt0 + 4.4 *mm2dp + rect.height();
			renderAnno(painter, font1, annox, annoy, 0., cellNumber);

			// 绘制内图廓左下角右上角坐标
			QString Degree, Minute, Second, str;
			QFont font0("Arial");
			font0.setPixelSize(1.75*mm2dp);
			splitDMMSS(fun_radTodmmss(m_ChartInfo.minLat), Degree, Minute, Second);
			if (fun_radTodmmss(m_ChartInfo.minLat < 0))
				str = Degree + "°" + Minute + "′" + Second + "”" + "S";
			else
				str = Degree + "°" + Minute + "’" + Second + "”" + "N";
			renderAnno(painter, font0, x1 + 1.75*mm2dp, y1 - 0.5*mm2dp, 0, str);

			splitDMMSS(fun_radTodmmss(m_ChartInfo.minLon), Degree, Minute, Second);
			if (fun_radTodmmss(m_ChartInfo.minLon < 0))
				str = Degree + "°" + Minute + "’" + Second + "”" + "W";
			else
				str = Degree + "°" + Minute + "’" + Second + "”" + "E";
			renderAnno(painter, font0, x1 + 1.75*mm2dp, y1 - 1.75*mm2dp, -90., str);

			splitDMMSS(fun_radTodmmss(m_ChartInfo.maxLat), Degree, Minute, Second);
			if (fun_radTodmmss(m_ChartInfo.minLat < 0))
				str = Degree + "°" + Minute + "′" + Second + "”" + "S";
			else
				str = Degree + "°" + Minute + "’" + Second + "”" + "N";
			fontMetrics = new QFontMetricsF(font0);
			rect = fontMetrics->boundingRect(str);
			delete fontMetrics;
			fontMetrics = nullptr;
			renderAnno(painter, font0, x3 - rect.width() - 1.75*mm2dp, y3 + 1.75*mm2dp, 0, str);

			splitDMMSS(fun_radTodmmss(m_ChartInfo.maxLon), Degree, Minute, Second);
			if (fun_radTodmmss(m_ChartInfo.minLon < 0))
				str = Degree + "°" + Minute + "’" + Second + "”" + "W";
			else
				str = Degree + "°" + Minute + "’" + Second + "”" + "E";
			renderAnno(painter, font0, x3 - 1.75*mm2dp, y3 + 1.75*mm2dp, 90., str);

			// 绘制图积
			double sizeX, sizeY;
			getBorderInnerSize(&sizeX, &sizeY);
			str = "(" + QString::number(sizeX, 'g.', 1) + "×" + QString::number(sizeY, 'g.', 1) + ")";
			font2.setPixelSize(2.25*mm2dp); // 10K
			fontMetrics = new QFontMetricsF(font2);
			rect = fontMetrics->boundingRect(str);
			delete fontMetrics;
			fontMetrics = nullptr;
			annox = x4 - rect.width();
			annoy = y4 + dt1 + rect.height();
			renderAnno(painter, font2, annox, annoy, 0., str);



			// 绘制出版单位
			str = "中国人民解放军海军参谋部航海保证局";
			font4.setPixelSize(5.5*mm2dp); // 24K
			fontMetrics = new QFontMetricsF(font4);
			rect = fontMetrics->boundingRect(str);
			delete fontMetrics;
			annox = (x1 + x4)*0.5 - rect.width()*0.5;
			annoy = y4 + dt0 + rect.height() + 4.4*mm2dp;
			renderAnno(painter, font4, annox, annoy, 0., str);
			double tmpH = rect.height();

			str = "CHINA NAVY HYDROGRAPHIC OFFICE";
			font5.setPixelSize(4.5*mm2dp); // 24K
			fontMetrics = new QFontMetricsF(font5);
			rect = fontMetrics->boundingRect(str);
			delete fontMetrics;
			fontMetrics = nullptr;
			annox = (x1 + x4)*0.5 - rect.width()*0.5;
			annoy = y4 + dt0 + tmpH + rect.height() + 6.4*mm2dp;
			renderAnno(painter, font5, annox, annoy, 0., str);

			// 绘制比例尺、基准纬线
			QString strScal = QString::number(m_ChartInfo.csclScale, 'g0', 0);
			if (strScal.length() < 4)
				strScal = "1︰" + strScal;
			else if (strScal.length() < 7)
				strScal = "1︰" + strScal.left(strScal.length() - 3) + " " + strScal.right(3);
			else
				strScal = "1︰" + strScal.left(strScal.length() - 6) + strScal.right(6).left(3) + " " + strScal.right(3);

			splitDMMSS(fun_radTodmmss(m_ChartInfo.baseLat), Degree, Minute, Second);
			//qDebug() << "baslat = " << fun_radTodmmss(m_ChartInfo.baseLat) << Degree << Minute << Second << endl;

			strScal.append("(").append(Degree).append("°").append(Minute).append("’").append(")");
			font4.setPixelSize(3.75*mm2dp);//16K
			fontMetrics = new QFontMetricsF(font4);
			rect = fontMetrics->boundingRect(strScal);
			delete fontMetrics;
			fontMetrics = nullptr;
			annox = (x2 + x3)*0.5 - rect.width()*0.5;
			annoy = y2 - dt0 - 5 * mm2dp;
			renderAnno(painter, font4, annox, annoy, 0, strScal);

			// 绘制图名
			font6.setPixelSize(7.5*mm2dp);//32K
			fontMetrics = new QFontMetricsF(font6);
			rect = fontMetrics->boundingRect(m_ChartInfo.chartName);
			double wChartName = rect.width();
			delete fontMetrics;
			fontMetrics = nullptr;
			annox = (x2 + x3)*0.5 - rect.width()*0.5;
			annoy = y2 - dt0 - 5 * mm2dp - 3.75*mm2dp - 3.*mm2dp;
			renderAnno(painter, font6, annox, annoy, 0, m_ChartInfo.chartName);

			// 绘制基准面
			QString strDep = "深度……米……理论深度基准面";
			QString strDev = "高程……米……1985国家高程基准";
			double annox1, annoy1;
			fontMetrics = new QFontMetricsF(font4);
			rect = fontMetrics->boundingRect(strDep);
			annox1 = annox - 12 * mm2dp - rect.width();
			annoy1 = annoy - (7.5 - 3.75)*mm2dp*0.5;
			renderAnno(painter, font4, annox1, annoy1, 0, strDep);
			rect = fontMetrics->boundingRect(strDev);
			annox1 = annox + wChartName + 12 * mm2dp;
			annoy1 = annoy - (7.5 - 3.75)*mm2dp*0.5;
			renderAnno(painter, font4, annox1, annoy1, 0, strDev);
			delete fontMetrics;
			fontMetrics = nullptr;
		}

		// 绘制公里尺
		painter->save();
		painter->setPen(pen);
		//　左侧公里尺边线
		painter->drawLine(x1 - dt0, y1, x2 - dt0, y2);
		painter->drawLine(x1 - dt0 - m_BorderInfo.wKmMin* mm2dp, y1, x2 - dt0 - m_BorderInfo.wKmMin* mm2dp, y2);
		painter->drawLine(x1 - dt0 - m_BorderInfo.wKmMid* mm2dp, y1, x2 - dt0 - m_BorderInfo.wKmMid* mm2dp, y2);

		painter->drawLine(x1 - dt0, y1, x2 - dt0 - m_BorderInfo.wKmMax* mm2dp, y1);
		painter->drawLine(x1 - dt0, y2, x2 - dt0 - m_BorderInfo.wKmMax* mm2dp, y2);

		painter->drawLine(x4 + dt0, y4, x3 + dt0, y3);
		painter->drawLine(x4 + dt0 + m_BorderInfo.wKmMin* mm2dp, y4, x3 + dt0 + m_BorderInfo.wKmMin* mm2dp, y3);
		painter->drawLine(x4 + dt0 + m_BorderInfo.wKmMid* mm2dp, y4, x3 + dt0 + m_BorderInfo.wKmMid* mm2dp, y3);

		painter->drawLine(x3 + dt0, y3, x4 + dt0 + m_BorderInfo.wKmMax* mm2dp, y3);
		painter->drawLine(x3 + dt0, y4, x4 + dt0 + m_BorderInfo.wKmMax* mm2dp, y4);

		QString str;
		double tmplat, tmplon;
		int tmpx, tmpy, tmpy0;
		double kmBottom = m_pTransform->m_Proj.getMeridianArcLength(m_ChartInfo.minLat);
		double kmTop = m_pTransform->m_Proj.getMeridianArcLength(m_ChartInfo.maxLat);
		double kmLen = kmTop - kmBottom;
		tmplat = m_ChartInfo.minLat;

		font1.setPixelSize(2.25*mm2dp);
		if (m_BorderInfo.KmMarkInterval >= 1000)
			str = "km";
		else
			str = "m";
		QFontMetricsF qm1(font1);

		renderAnno(painter, font1, x1 - dt0 - (m_BorderInfo.wKmMax) * mm2dp, y1 + qm1.height(), 0, str);
		renderAnno(painter, font1, x4 + dt0 + m_BorderInfo.wKmMid*mm2dp - qm1.width(str), y4 + qm1.height(), 0, str);
		renderAnno(painter, font1, x1 - dt0 - (m_BorderInfo.wKmMax) * mm2dp - qm1.width("0"), y1 + 2.25*mm2dp*0.5, 0, "0");
		renderAnno(painter, font1, x4 + dt0 + (m_BorderInfo.wKmMax) * mm2dp, y4 + 2.25*mm2dp*0.5, 0, "0");
		int  i = m_BorderInfo.KmMinInterval;
		while (i < kmLen)
		{
			i += m_BorderInfo.KmMinInterval;
			tmpy0 = kmBottom + i;
			tmplat = m_pTransform->m_Proj.getMeridianArcRad(tmpy0); // 子午线弧长反解
			if (tmplat > m_ChartInfo.maxLat)
				break;
			if (i%m_BorderInfo.KmMarkInterval == 0)
			{

				m_pTransform->m_Proj.getXY(tmplat, m_ChartInfo.minLon, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &tmpx, &tmpy);
				painter->drawLine(tmpx - dt0, tmpy, tmpx - dt0 - m_BorderInfo.wKmMax * mm2dp, tmpy);

				if (m_BorderInfo.KmMarkInterval >= 1000)
					str = QString::number(i / 1000);
				else
					str = QString::number(i);
				QFontMetricsF qm(font1);

				renderAnno(painter, font1, tmpx - dt0 - (m_BorderInfo.wKmMax) * mm2dp - qm.width(str), tmpy + 2.25*mm2dp*0.5, 0, str);

				m_pTransform->m_Proj.getXY(tmplat, m_ChartInfo.maxLon, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &tmpx, &tmpy);
				painter->drawLine(tmpx + dt0, tmpy, tmpx + dt0 + m_BorderInfo.wKmMax * mm2dp, tmpy);
				renderAnno(painter, font1, tmpx + dt0 + m_BorderInfo.wKmMax * mm2dp, tmpy + 2.25*mm2dp*0.5, 0, str);
			}
			else if (i%m_BorderInfo.KmMaxInterval == 0)
			{
				m_pTransform->m_Proj.getXY(tmplat, m_ChartInfo.minLon, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &tmpx, &tmpy);
				painter->drawLine(tmpx - dt0, tmpy, tmpx - dt0 - m_BorderInfo.wKmMid * mm2dp, tmpy);
				m_pTransform->m_Proj.getXY(tmplat, m_ChartInfo.maxLon, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &tmpx, &tmpy);
				painter->drawLine(tmpx + dt0, tmpy, tmpx + dt0 + m_BorderInfo.wKmMid * mm2dp, tmpy);
			}
			else
			{
				m_pTransform->m_Proj.getXY(tmplat, m_ChartInfo.minLon, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &tmpx, &tmpy);
				painter->drawLine(tmpx - dt0, tmpy, tmpx - dt0 - m_BorderInfo.wKmMin * mm2dp, tmpy);
				m_pTransform->m_Proj.getXY(tmplat, m_ChartInfo.maxLon, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &tmpx, &tmpy);
				painter->drawLine(tmpx + dt0, tmpy, tmpx + dt0 + m_BorderInfo.wKmMin * mm2dp, tmpy);
			}

			if (i%m_BorderInfo.KmMaxInterval == 0
				&& i / m_BorderInfo.KmMaxInterval % 2 == 1)
			{
				m_pTransform->m_Proj.getXY(tmplat, m_ChartInfo.minLon, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &tmpx, &tmpy);
				double nextY = tmpy0 + m_BorderInfo.KmMaxInterval;
				double nextLat = m_pTransform->m_Proj.getMeridianArcRad(nextY);
				int nextx, nexty;
				m_pTransform->m_Proj.getXY(nextLat, m_ChartInfo.minLon, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &nextx, &nexty);
				painter->save();

				if (nexty < y2)
					nexty = y2;
				painter->setPen(pen3);
				painter->drawLine(tmpx - dt0 - (m_BorderInfo.wKmMin + 0.2)*mm2dp, tmpy, tmpx - dt0 - (m_BorderInfo.wKmMin + 0.2)*mm2dp, nexty);
				m_pTransform->m_Proj.getXY(tmplat, m_ChartInfo.maxLon, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &tmpx, &tmpy);
				painter->drawLine(tmpx + dt0 + (m_BorderInfo.wKmMin + 0.2)*mm2dp, tmpy, tmpx + dt0 + (m_BorderInfo.wKmMin + 0.2)*mm2dp, nexty);
				painter->restore();
			}
		}
		painter->restore();
	}

	{//绘制细分和经纬线
		double lat, lat1, lat2, lon;
		double lon1 = m_ChartInfo.minLon;
		double lon2 = m_ChartInfo.maxLon;
		int x1, y1, x2, y2;
		//绘制东西图廓细分
		for (long i = m_ChartInfo.minLat*RO*3600. + 0.5; i < m_ChartInfo.maxLat*RO*3600.; i++)
		{
			lat = i / 3600. / RO;
			double dmmss = fun_radTodmmss(lat);
			QString strDegree, strMinute, strSecond;
			splitDMMSS(dmmss, strDegree, strMinute, strSecond);

			// 绘制纬线和标注
			if (i%m_BorderInfo.lineInterval == 0)
			{// 纬线间隔
				m_pTransform->m_Proj.getXY(lat, lon1, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &x1, &y1);
				m_pTransform->m_Proj.getXY(lat, lon2, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &x2, &y2);
				painter->drawLine(x1 - dt0, y1, x2 + dt0, y2);
				//painter->drawLine(x1 - dt0, y1, x1 - dt0 + dt2, y2);
				//painter->drawLine(x2 + dt0, y2, x2 - dt0 + dt2, y2);
				//　标注
				if (strMinute == "00" && strSecond == "00")
				{
					font1.setPixelSize(3.5*mm2dp);
					QFontMetricsF fm(font1);
					float width = fm.width(strDegree + '1');  // 加１只是为了计算宽度，
					float offsetY = fm.height() / 2.;

					painter->setFont(font1);
					QBrush brush(QColor(255, 255, 255));
					painter->drawText(QPointF(x1 - dt1 - width, y1), strDegree + "°");
					painter->drawText(QPointF(x2 + dt1, y1), strDegree + "°");
				}
				else if (strMinute != "00"&&strSecond == "00")
				{
					font1.setPixelSize(3.5*mm2dp);
					QFontMetricsF fm(font1);
					float width = fm.width(strDegree + '1');  // 加１只是为了计算宽度，
					float offsetY = fm.height() / 2.;
					painter->setFont(font1);
					QBrush brush(QColor(255, 255, 255));
					painter->drawText(QPointF(x1 - dt1 - width, y1), strDegree + "°");
					painter->drawText(QPointF(x2 + dt1, y1), strDegree + "°");
					font1.setPixelSize(2.75*mm2dp);
					QFontMetricsF fm1(font1);
					width = fm1.width(strMinute + '1');  // 加１只是为了计算宽度，
					offsetY = fm1.height() / 2.;
					painter->setFont(font1);
					painter->drawText(QPointF(x1 - dt1 - width, y1 + fm1.height()), strMinute + "’");
					painter->drawText(QPointF(x2 + dt1, y1 + fm1.height()), strMinute + "’");
				}

			}
			else if (i%m_BorderInfo.largerSubInterval == 0)
			{
				// 较大分划
				m_pTransform->m_Proj.getXY(lat, lon1, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &x1, &y1);
				m_pTransform->m_Proj.getXY(lat, lon2, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &x2, &y2);
				painter->drawLine(x1, y1, x1 - dt1, y1);
				painter->drawLine(x2, y2, x2 + dt1, y2);

				if (strMinute != "00"&&strSecond == "00")
				{
					font1.setPixelSize(2.75*mm2dp);
					QFontMetricsF fm1(font1);
					float width = fm1.width(strMinute + '1');  // 加１只是为了计算宽度，
					float offsetY = fm1.height() / 2.;
					painter->setFont(font1);
					QBrush brush(QColor(255, 255, 255));
					painter->drawText(QPointF(x1 - dt1 - width, y1 + offsetY), strMinute + "’");
					painter->drawText(QPointF(x2 + dt1, y1 + offsetY), strMinute + "’");
				}
				else if (strSecond != "00")
				{
					font1.setPixelSize(2.75*mm2dp);
					QFontMetricsF fm1(font1);
					float width = fm1.width(strSecond + '1');  // 加１只是为了计算宽度，
					float offsetY = fm1.height() / 2.;
					painter->setFont(font1);
					QBrush brush(QColor(255, 255, 255));
					painter->fillRect(x1 - dt1 - width, y1 - offsetY, width, fm1.height(), brush);
					painter->drawText(QPointF(x1 - dt1 - width, y1 + offsetY), strSecond + "”");
					painter->fillRect(x2 + dt1, y1 - offsetY, width, fm1.height(), brush);
					painter->drawText(QPointF(x2 + dt1, y1 + offsetY), strSecond + "”");
				}

			}
			else if (i%m_BorderInfo.smallSubInterval == 0)
			{
				// 较小分划
				m_pTransform->m_Proj.getXY(lat, lon1, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &x1, &y1);
				m_pTransform->m_Proj.getXY(lat, lon2, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &x2, &y2);
				painter->drawLine(x1, y1, x1 - dt2, y1);
				painter->drawLine(x2, y2, x2 + dt2, y2);
			}
			else if (i%m_BorderInfo.minSubInterval == 0)
			{
				m_pTransform->m_Proj.getXY(lat, lon1, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &x1, &y1);
				m_pTransform->m_Proj.getXY(lat, lon2, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &x2, &y2);
				painter->drawLine(x1, y1, x1 - dt3, y1);
				painter->drawLine(x2, y2, x2 + dt3, y2);
			}

			if (m_ChartInfo.csclScale < 30000)
				continue;

			// 绘制黑白节
			if (i%m_BorderInfo.blackWhiteSectionInterval == 0 && i / m_BorderInfo.blackWhiteSectionInterval % 2 == 0)
			{
				painter->save();
				painter->setPen(pen1);
				lat1 = (i + m_BorderInfo.blackWhiteSectionInterval) / 3600. / RO;
				if (lat1 > m_ChartInfo.maxLat)
					lat1 = m_ChartInfo.maxLat;
				m_pTransform->m_Proj.getXY(lat, lon1, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &x1, &y1);
				m_pTransform->m_Proj.getXY(lat1, lon1, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &x2, &y2);
				painter->drawLine(x1 - dtbw, y1, x2 - dtbw, y2);
				m_pTransform->m_Proj.getXY(lat, lon2, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &x1, &y1);
				m_pTransform->m_Proj.getXY(lat1, lon2, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &x2, &y2);
				painter->drawLine(x1 + dtbw, y1, x2 + dtbw, y2);
				painter->restore();
			}
			else if (i%m_BorderInfo.blackWhiteSectionInterval == 0 && i / m_BorderInfo.blackWhiteSectionInterval % 2 == 1)
			{
				if (i - m_ChartInfo.minLat*RO*3600. < m_BorderInfo.blackWhiteSectionInterval)
				{
					painter->save();
					painter->setPen(pen1);
					lat1 = m_ChartInfo.minLat;
					m_pTransform->m_Proj.getXY(lat, lon1, &ycoor, &xcoor);
					m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &x1, &y1);
					m_pTransform->m_Proj.getXY(lat1, lon1, &ycoor, &xcoor);
					m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &x2, &y2);
					painter->drawLine(x1 - dtbw, y1, x2 - dtbw, y2);
					m_pTransform->m_Proj.getXY(lat, lon2, &ycoor, &xcoor);
					m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &x1, &y1);
					m_pTransform->m_Proj.getXY(lat1, lon2, &ycoor, &xcoor);
					m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &x2, &y2);
					painter->drawLine(x1 + dtbw, y1, x2 + dtbw, y2);
					painter->restore();
				}
			}
		}

		// 绘制南北图廓细分
		lat1 = m_ChartInfo.minLat;
		lat2 = m_ChartInfo.maxLat;
		for (long i = m_ChartInfo.minLon*RO*3600. + 0.5; i < m_ChartInfo.maxLon*RO*3600.; i++)
		{
			lon = i / 3600. / RO;
			double dmmss = fun_radTodmmss(lon);
			QString strDegree, strMinute, strSecond;
			splitDMMSS(dmmss, strDegree, strMinute, strSecond);

			// 绘制经线和标注
			if (i%m_BorderInfo.lineInterval == 0)
			{
				// 纬制经线
				m_pTransform->m_Proj.getXY(lat1, lon, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &x1, &y1);
				m_pTransform->m_Proj.getXY(lat2, lon, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &x2, &y2);
				painter->drawLine(x1, y1 + dt1, x2, y2 - dt1);
				painter->drawLine(x1, y1 + dt0, x2, y1 + dt0 - dt1);
				painter->drawLine(x1, y2 - dt0, x2, y2 - dt0 + dt1);
				//　标注
				if (strMinute == "00" && strSecond == "00")
				{
					font1.setPixelSize(3.5*mm2dp);
					QFontMetricsF fm(font1);
					float width = fm.width(strDegree/* + '1'*/);  // 加１只是为了计算宽度，
					float offsetY = fm.height() / 2.;

					painter->setFont(font1);
					painter->drawText(QPointF(x1 - width * 0.5, y1 + dt1 + fm.height()), strDegree + "°");
					painter->drawText(QPointF(x2 - width * 0.5, y2 - dt1), strDegree + "°");
				}
				else if (strMinute != "00"&&strSecond == "00")
				{
					font1.setPixelSize(3.5*mm2dp);
					QFontMetricsF fm(font1);
					float width = fm.width(strDegree/* + '1'*/);  // 加１只是为了计算宽度，
					float offsetY = fm.height() / 2.;
					painter->setFont(font1);
					QBrush brush(QColor(255, 255, 255));
					painter->drawText(QPointF(x1 - width, y1 + dt1 + fm.height()), strDegree + "°");
					painter->drawText(QPointF(x2 - width, y2 - dt1), strDegree + "°");
					font1.setPixelSize(2.75*mm2dp);
					QFontMetricsF fm1(font1);
					width = fm1.width(strMinute + '1');  // 加１只是为了计算宽度，
					offsetY = fm1.height() / 2.;
					painter->setFont(font1);
					painter->drawText(QPointF(x1 + fm1.width("1"), y1 + dt1 + fm1.height() + offsetY * 0.5), strMinute + "’");
					painter->drawText(QPointF(x2 + fm1.width("1"), y2 - dt1), strMinute + "’");

				}

			}
			else if (i%m_BorderInfo.largerSubInterval == 0)
			{
				//较大分划
				m_pTransform->m_Proj.getXY(lat1, lon, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &x1, &y1);
				m_pTransform->m_Proj.getXY(lat2, lon, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &x2, &y2);
				painter->drawLine(x1, y1, x1, y1 + dt1);
				painter->drawLine(x2, y2, x2, y2 - dt1);

				if (strMinute != "00"&&strSecond == "00")
				{
					font1.setPixelSize(2.75*mm2dp);
					QFontMetricsF fm1(font1);
					float width = fm1.width(strMinute /*+ '1'*/);  // 加１只是为了计算宽度，
					float offsetY = fm1.height() / 2.;
					painter->setFont(font1);
					QBrush brush(QColor(255, 255, 255));
					painter->drawText(QPointF(x1 - width * 0.5, y1 + dt1 + fm1.height()), strMinute + "’");
					painter->drawText(QPointF(x2 - width * 0.5, y2 - dt1), strMinute + "’");
				}
				else if (strSecond != "00")
				{
					font1.setPixelSize(2.75*mm2dp);
					QFontMetricsF fm1(font1);
					float width = fm1.width(strSecond /*+ '1'*/);  // 加１只是为了计算宽度，
					float offsetY = fm1.height() / 2.;
					painter->setFont(font1);
					QBrush brush(QColor(255, 255, 255));
					painter->drawText(QPointF(x1 - width * 0.5, y1 + dt1 + fm1.height()), strSecond + "”");
					painter->drawText(QPointF(x2 - width * 0.5, y2 - dt1), strSecond + "”");
				}
			}
			else if (i%m_BorderInfo.smallSubInterval == 0)
			{
				m_pTransform->m_Proj.getXY(lat1, lon, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &x1, &y1);
				m_pTransform->m_Proj.getXY(lat2, lon, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &x2, &y2);
				painter->drawLine(x1, y1, x1, y1 + dt2);
				painter->drawLine(x2, y2, x2, y2 - dt2);
			}
			else if (i%m_BorderInfo.minSubInterval == 0)
			{
				m_pTransform->m_Proj.getXY(lat1, lon, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &x1, &y1);
				m_pTransform->m_Proj.getXY(lat2, lon, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &x2, &y2);
				painter->drawLine(x1, y1, x1, y1 + dt3);
				painter->drawLine(x2, y2, x2, y2 - dt3);
			}

			if (m_ChartInfo.csclScale < 30000)
				continue;
			if (i%m_BorderInfo.blackWhiteSectionInterval == 0 && i / m_BorderInfo.blackWhiteSectionInterval % 2 == 1)
			{
				painter->save();
				painter->setPen(pen1);
				lon1 = (i + m_BorderInfo.blackWhiteSectionInterval) / 3600. / RO;
				if (lon1 > m_ChartInfo.maxLon)
					lon1 = m_ChartInfo.maxLon;
				m_pTransform->m_Proj.getXY(lat1, lon, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &x1, &y1);
				m_pTransform->m_Proj.getXY(lat1, lon1, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &x2, &y2);
				painter->drawLine(x1, y1 + dtbw, x2, y2 + dtbw);


				m_pTransform->m_Proj.getXY(lat2, lon, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &x1, &y1);
				m_pTransform->m_Proj.getXY(lat2, lon1, &ycoor, &xcoor);
				m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &x2, &y2);
				painter->drawLine(x1, y1 - dtbw, x2, y2 - dtbw);
				painter->restore();
			}
			else if (i%m_BorderInfo.blackWhiteSectionInterval == 0 && i / m_BorderInfo.blackWhiteSectionInterval % 2 == 0)
			{
				if (i - int(m_ChartInfo.minLon*RO*3600. + 0.5) < m_BorderInfo.blackWhiteSectionInterval
					&& i - int(m_ChartInfo.minLon*RO*3600. + 0.5) > 0)
				{
					painter->save();
					painter->setPen(pen1);
					lon1 = m_ChartInfo.minLon;
					m_pTransform->m_Proj.getXY(lat1, lon, &ycoor, &xcoor);
					m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &x1, &y1);
					m_pTransform->m_Proj.getXY(lat1, lon1, &ycoor, &xcoor);
					m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &x2, &y2);
					painter->drawLine(x1, y1 + dtbw, x2, y2 + dtbw);


					m_pTransform->m_Proj.getXY(lat2, lon, &ycoor, &xcoor);
					m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &x1, &y1);
					m_pTransform->m_Proj.getXY(lat2, lon1, &ycoor, &xcoor);
					m_pTransform->m_Viewport.convertVp2Dp(xcoor, ycoor, &x2, &y2);
					painter->drawLine(x1, y1 - dtbw, x2, y2 - dtbw);
					painter->restore();
				}
			}
		}
	}
	painter->restore();
}

void CS57CellBorderRenderer::preCalcBorder()
{
	// 当前视口下
	double RelativeScale = m_ChartInfo.csclScale / m_pTransform->m_Viewport.getDisplayScale();
	double mm2dp = m_pTransform->m_Viewport.m_dXdpi * RelativeScale / 25.4; // 当前视口下，纸图上1mm对应的设备坐标长度
	double dt0 = m_BorderInfo.lenin2Outline * mm2dp;
	//dt1 = m_BorderInfo.lenMaxSub * mm2dp;
	//dt2 = m_BorderInfo.lenLargerSub * mm2dp;
	//dt3 = m_BorderInfo.lenMinSub * mm2dp;
	//dtbw = 1.4  * mm2dp;
	//pen.setWidth(0.1*mm2dp);
	//pen.setColor(QColor(0, 0, 0));
	//pen1.setWidth(0.2 * mm2dp);
	//pen1.setCapStyle(Qt::FlatCap);
	//pen1.setColor(QColor(0, 0, 0));
	//pen2.setWidth(1.0 *mm2dp);
	//pen2.setCapStyle(Qt::FlatCap);
	//pen2.setJoinStyle(Qt::MiterJoin);
	//pen2.setColor(QColor(0, 0, 0));
	//pen3.setWidth(0.4*mm2dp);
	//pen3.setCapStyle(Qt::FlatCap);
}

void CS57CellBorderRenderer::renderAnno(QPainter * &painter, QFont &font, double x, double y, float ang, QString str)
{
	m_pPainter->save();
	m_pPainter->setFont(font);
	m_pPainter->translate(x, y);
	m_pPainter->rotate(ang);
	m_pPainter->drawText(QPointF(0, 0), str);
	m_pPainter->restore();
}

bool Core::CS57CellBorderRenderer::calcInnerCoors()
{
	double coorx1, coorx2, coory1, coory2;
	m_pTransform->m_Proj.getXY(m_ChartInfo.minLat, m_ChartInfo.minLon, &coory1, &coorx1);
	m_pTransform->m_Proj.getXY(m_ChartInfo.maxLat, m_ChartInfo.maxLon, &coory2, &coorx2);
	PointLB = QPoint(coorx1, coory1);
	PointLT = QPoint(coorx1, coory2);
	pointRT = QPoint(coorx2, coory2);
	PointRB = QPoint(coorx2, coory1);
	return true;
}
