#include "stdafx.h"
#include "cs57symbol.h"
#include "cs57projection.h"
#include "cline.h"
#include <qmath.h>
#include <QtGlobal>
#include <QDebug>

using namespace Core;
CS57AbstractSymbol::CS57AbstractSymbol()
{}

CS57AbstractSymbol::~CS57AbstractSymbol()
{}

CS57PointSymbol::CS57PointSymbol(CS57PresLib* preslib)
{
	m_pPresLib = preslib;
}

CS57PointSymbol::~CS57PointSymbol()
{}

void CS57PointSymbol::renderSY(QPainter* painter, QPoint& pt, const QString& symbName, float scale, float rotation)
{
	SYMB* pCurSymb = m_pPresLib->getSymb(symbName);
	if (!pCurSymb) return;

	QPointF pointPivot;		// 符号的定位点
	pointPivot.setX(pCurSymb->m_fieldSymd.SYCL);
	pointPivot.setY(pCurSymb->m_fieldSymd.SYRW);

	float scaleX = painter->device()->physicalDpiX() / 2540.;
	float scaleY = painter->device()->physicalDpiX() / 2540.;

	painter->save();

	// 按输入坐标、比例、角度设置转换
	painter->translate(pt);
	painter->scale(scale, scale);
	painter->rotate(rotation);

	painter->scale(scaleX, scaleY);
	painter->translate(-pointPivot.x(), -pointPivot.y());

	for (int i = 0; i < pCurSymb->m_vecS52Layers.size(); i++)
	{
		QColor curColor;
		S52Color* pColor = m_pPresLib->getS52Color(pCurSymb->m_vecS52Layers[i]->strColorName);
		curColor.setRgb(pColor->r, pColor->g, pColor->b);
		curColor.setAlphaF(1. - pCurSymb->m_vecS52Layers[i]->uTransparent * 0.25);

		QPen curPen;
		curPen.setWidth(pCurSymb->m_vecS52Layers[i]->nPenWidth * 15);
		curPen.setColor(curColor);
		painter->setPen(curPen);

		QBrush curBrush(curColor);
		curBrush.setStyle(Qt::SolidPattern);

		painter->setBrush(curBrush);
		if (pCurSymb->m_vecS52Layers[i]->vecPolygon.size() > 0)
		{
			for (int j = 0; j < pCurSymb->m_vecS52Layers[i]->vecPolygon.size(); j++)
			{
				QPolygon polygon;
				polygon.reserve(pCurSymb->m_vecS52Layers[i]->vecPolygon[j]->vecLine.size());
				for (int k = 0; k < pCurSymb->m_vecS52Layers[i]->vecPolygon[j]->vecLine.size(); k++)
				{
					polygon.push_back(*pCurSymb->m_vecS52Layers[i]->vecPolygon[j]->vecLine[k]);
				}
				QPen pen = painter->pen();
				QBrush brush = painter->brush();
				switch (pCurSymb->m_vecS52Layers[i]->vecPolygon[j]->modeFill)
				{
				case 0:
					painter->setBrush(Qt::NoBrush);
					break;
				case 1:
					painter->setPen(Qt::NoPen);
					break;
				case 2:
					break;
				}
				painter->drawPolygon(polygon);
				polygon.clear();
				painter->setPen(pen);
				painter->setBrush(brush);
			}
		}

		for (int j = 0; j < pCurSymb->m_vecS52Layers[i]->vecLines.size(); j++)
		{
			CPolyline vecLines;
			vecLines.reserve(pCurSymb->m_vecS52Layers[i]->vecLines[j]->vecLine.size());
			for (int k = 0; k < pCurSymb->m_vecS52Layers[i]->vecLines[j]->vecLine.size(); k++)
			{
				vecLines.push_back(*pCurSymb->m_vecS52Layers[i]->vecLines[j]->vecLine[k]);
			}
			painter->drawPolyline(vecLines);
			vecLines.clear();
		}

		for (int j = 0; j < pCurSymb->m_vecS52Layers[i]->vecCircle.size(); j++)
		{
			QRect circleRect;
			S52Circle* pCircle = pCurSymb->m_vecS52Layers[i]->vecCircle[j];
			circleRect.setLeft(pCircle->cenPoint.x() - pCircle->radius);
			circleRect.setTop(pCircle->cenPoint.y() - pCircle->radius);
			circleRect.setWidth(2 * pCircle->radius);
			circleRect.setHeight(2 * pCircle->radius);
			QPen pen = painter->pen();
			QBrush brush = painter->brush();
			switch (pCurSymb->m_vecS52Layers[i]->vecCircle[j]->modeFill)
			{
			case 0:
				painter->setBrush(Qt::NoBrush);
				break;
			case 1:
				painter->setPen(Qt::NoPen);
				break;
			case 2:

				painter->drawArc(circleRect, 0, 5760);
				break;
			}
			painter->drawEllipse(circleRect);
			painter->setPen(pen);
			painter->setBrush(brush);
		}
	}
	painter->restore();
}

void CS57PointSymbol::renderCA(QPainter* painter,
									QPoint& pt,
									float sectr1,
									float sectr2, 
									float arc_radius, 
									float sector_radius, 
									unsigned short outline_width, 
									unsigned short arc_width, 
									char arc_color[6],
									char outline_color[6])
{
	float dpiX = painter->device()->physicalDpiX();
	unsigned short r = 0 , g = 0, b = 0;
	m_pPresLib->getS52Color(arc_color, r, g, b);
	float sb;
	float se;

	QPen pen(QColor(255, 0, 0, 255), 1);
	int rad = (int)(arc_radius * dpiX / 25.4);

	int width = (rad * 2) + 28;
	int height = (rad * 2) + 28;
	int flag = -1;
	if (sectr2 > sectr1)
	{
		flag = 0;
		sb = sectr1 - 90;
		se = sectr2 - 90;
	}
	else
	{
		flag = 1;
		sb = sectr2 - 90;
		se = sectr1 - 90;
	}
	float symbolScale = 1.0;
	if (fabs(sectr2 - sectr1) != 360)
	{
		painter->save();
		QRect rect;
		rect.setX(pt.x() - rad * symbolScale);
		rect.setY(pt.y() - rad * symbolScale);
		rect.setWidth(rad * 2 * symbolScale);
		rect.setHeight(rad * 2 * symbolScale);
		m_pPresLib->getS52Color(outline_color, r, g, b);
		pen.setColor(QColor(r, g, b, 255));
		pen.setWidth(outline_width * symbolScale);
		painter->setPen(pen);
		if (flag == 0)
			painter->drawArc(rect, -(sb - 2) * 16, -(se - sb + 4) * 16);
		else
			painter->drawArc(rect, -sb * 16, (360 - (se - sb)) * 16);
		if (arc_width)
		{
			m_pPresLib->getS52Color(arc_color, r, g, b);
			pen.setColor(QColor(r, g, b, 255));
			pen.setWidth(arc_width * symbolScale);
			painter->setPen(pen);
			if (flag == 0)
				painter->drawArc(rect, -sb * 16, -(se - sb) * 16);
			else
				painter->drawArc(rect, -(sb - 2) * 16, (360 - (se - sb + 4)) * 16);
		}
		painter->restore();
	}
	else
	{
		painter->save();
		QRect rect;
		rect.setX(pt.x() - rad * symbolScale);
		rect.setY(pt.y() - rad * symbolScale);
		rect.setWidth(rad * 2 * symbolScale);
		rect.setHeight(rad * 2 * symbolScale);

		m_pPresLib->getS52Color(outline_color, r, g, b);
		pen.setColor(QColor(r, g, b, 255));
		pen.setWidth(outline_width * symbolScale);
		painter->setPen(pen);
		painter->drawArc(rect, -sb * 16, -360 * 16);
		if (arc_width)
		{
			m_pPresLib->getS52Color(arc_color, r, g, b);
			pen.setColor(QColor(r, g, b, 255));
			pen.setWidth(arc_width * symbolScale);
			painter->setPen(pen);
			painter->drawArc(rect, -sb * 16, -360 * 16);
		}
		painter->restore();
	}

	if (arc_radius > 0)
	{
		if (sectr1 == 0)
			return;

		painter->save();
		int leg_len = (int)(sector_radius * dpiX / 25.4);

		m_pPresLib->getS52Color("CHBLK", r, g, b);
		QPen pen(QColor(r, g, b, 125), 1);
		pen.setStyle(Qt::DashLine);
		QVector<qreal> vecPattern;
		vecPattern.push_back(8);
		vecPattern.push_back(4);
		pen.setDashPattern(vecPattern);
		painter->setPen(pen);
		float a = (sectr1 - 90) * PI / 180;

		int x = pt.x() + (int)(leg_len * cosf(a));
		int y = pt.y() + (int)(leg_len * sinf(a));
		painter->drawLine(pt.x(), pt.y(), x, y);
		a = (sectr2 - 90) * PI / 180;

		x = pt.x() + (int)(leg_len * cosf(a));
		y = pt.y() + (int)(leg_len * sinf(a));
		painter->drawLine(pt.x(), pt.y(), x, y);
		painter->restore();
	}
}

void CS57PointSymbol::renderSounding(QPainter* painter, QPoint& pt, const QString& symbName, float scale, float rotation)
{
	int devType = painter->device()->devType();
	SYMB* pCurSymb = m_pPresLib->getSymb(symbName);
	if (!pCurSymb) return;

	float dpiX = painter->device()->physicalDpiX();
	float dpiY = painter->device()->physicalDpiX();
	float scaleX = dpiX / 2540.;
	float scaleY = dpiY / 2540.;

	QPoint pointPivot;	// 符号的定位点
	pointPivot.setX(pCurSymb->m_fieldSymd.SYCL);
	pointPivot.setY(pCurSymb->m_fieldSymd.SYRW);

	QPoint pointUperLeft; // 符号左上角点坐标
	pointUperLeft.setX(pCurSymb->m_fieldSymd.SBXC);
	pointUperLeft.setY(pCurSymb->m_fieldSymd.SBXR);

	QRect symRect;
	symRect.setTopLeft(pointUperLeft);
	symRect.setWidth(pCurSymb->m_fieldSymd.SYHL);
	symRect.setHeight(pCurSymb->m_fieldSymd.SYVL);

	painter->save();

	// 按输入坐标、比例、角度设置转换
	painter->translate(pt.x(), pt.y());
	painter->scale(scale, scale);
	painter->rotate(rotation);

	painter->scale(scaleX, scaleY);
	painter->translate(-pointPivot.x(), -pointPivot.y());

	for (int i = 0; i < pCurSymb->m_vecS52Layers.size(); i++)
	{
		QColor curColor;
		S52Color* pColor = m_pPresLib->getS52Color(pCurSymb->m_vecS52Layers[i]->strColorName);
		curColor.setRgb(pColor->r, pColor->g, pColor->b);
		if (devType == QInternal::Printer)
		{
			curColor.setRgb(7, 7, 7);
		}
		curColor.setAlphaF(1. - pCurSymb->m_vecS52Layers[i]->uTransparent * 0.25);

		QPen curPen;
		curPen.setWidth(pCurSymb->m_vecS52Layers[i]->nPenWidth/ scaleY);	// modify by wb 2021.04.29

		curPen.setColor(curColor);
		painter->setPen(curPen);

		QBrush curBrush(curColor);
		curBrush.setStyle(Qt::SolidPattern);

		painter->setBrush(curBrush);
		if (pCurSymb->m_vecS52Layers[i]->vecPolygon.size() > 0)
		{
			for (int j = 0; j < pCurSymb->m_vecS52Layers[i]->vecPolygon.size(); j++)
			{
				QPolygon polygon;
				polygon.reserve(pCurSymb->m_vecS52Layers[i]->vecPolygon[j]->vecLine.size());
				for (int k = 0; k < pCurSymb->m_vecS52Layers[i]->vecPolygon[j]->vecLine.size(); k++)
				{
					polygon.push_back(*pCurSymb->m_vecS52Layers[i]->vecPolygon[j]->vecLine[k]);
				}
				QPen pen = painter->pen();
				QBrush brush = painter->brush();
				switch (pCurSymb->m_vecS52Layers[i]->vecPolygon[j]->modeFill)
				{
				case 0:
					painter->setBrush(Qt::NoBrush);
					break;
				case 1:
					painter->setPen(Qt::NoPen);
					break;
				case 2:
					break;
				}
				painter->drawPolygon(polygon);
				polygon.clear();
				painter->setPen(pen);
				painter->setBrush(brush);
			}
		}

		for (int j = 0; j < pCurSymb->m_vecS52Layers[i]->vecLines.size(); j++)
		{
			CPolyline vecLines;
			vecLines.reserve(pCurSymb->m_vecS52Layers[i]->vecLines[j]->vecLine.size());
			for (int k = 0; k < pCurSymb->m_vecS52Layers[i]->vecLines[j]->vecLine.size(); k++)
			{
				vecLines.push_back(*pCurSymb->m_vecS52Layers[i]->vecLines[j]->vecLine[k]);
			}
			painter->drawPolyline(vecLines);
			vecLines.clear();
		}

		for (int j = 0; j < pCurSymb->m_vecS52Layers[i]->vecCircle.size(); j++)
		{
			QRect circleRect;
			S52Circle* pCircle = pCurSymb->m_vecS52Layers[i]->vecCircle[j];
			circleRect.setLeft(pCircle->cenPoint.x() - pCircle->radius);
			circleRect.setTop(pCircle->cenPoint.y() - pCircle->radius);
			circleRect.setWidth(2 * pCircle->radius);
			circleRect.setHeight(2 * pCircle->radius);
			QPen pen = painter->pen();
			QBrush brush = painter->brush();
			switch (pCurSymb->m_vecS52Layers[i]->vecCircle[j]->modeFill)
			{
			case 0:
				painter->setBrush(Qt::NoBrush);
				break;
			case 1:
				painter->setPen(Qt::NoPen);
				break;
			case 2:

				painter->drawArc(circleRect, 0, 5760);
				break;
			}
			painter->drawEllipse(circleRect);
			painter->setPen(pen);
			painter->setBrush(brush);
		}
	}

	painter->restore();
}

CS57LineSymbol::CS57LineSymbol(CS57PresLib* preslib)
{
	m_pPresLib = preslib;
}

CS57LineSymbol::~CS57LineSymbol()
{}

void CS57LineSymbol::renderLS(QPainter* pPainter, CPolyline& pts, float scale, int lineWidth, QString lineStyle, QString lineColor)
{
	if (lineStyle == "DOTT")
	{
		renderDotLine(pPainter, pts, scale, lineColor);
		return;
	}

	float dpi = pPainter->device()->physicalDpiX();
	int devType = pPainter->device()->devType();

	unsigned short r = 0, g = 0, b = 0;
	m_pPresLib->getS52Color(lineColor, r, g, b);

	float penWidth = 0.3;
	if (devType == QInternal::Printer)
	{
		penWidth = lineWidth * 0.1 / 25.4 * dpi * scale;
	}
	else
	{
		penWidth = lineWidth * 0.3 / 25.4 * dpi * scale;
	}
	QPen pen;
	pen.setColor(QColor(r,g,b));
	pen.setWidth(penWidth);
	if (lineStyle == "DASH")
	{
		QVector<qreal> dashes;
		if (devType == QInternal::Printer)//打印模式
		{
			dashes.push_back(36);
			dashes.push_back(18);
		}
		else
		{
			dashes.push_back(12);
			dashes.push_back(6);
		}
		pen.setStyle(Qt::CustomDashLine);
		pen.setDashPattern(dashes);
	}
	else
	{
		pen.setStyle(Qt::SolidLine);
	}
	pPainter->save();
	pPainter->setPen(pen);
	CPolylineF vecPoints;
	vecPoints.reserve(pts.size());
	for (int i = 0; i < pts.size(); i++)
		vecPoints.push_back(pts[i]);

	pPainter->drawPolyline(vecPoints);
	pPainter->restore();
}

void CS57LineSymbol::renderDotLine(QPainter* pPainter, CPolyline& pts, float scale, QString lineColor)
{
	int size = pts.size();
	if (size < 2)
		return;
	float dpi = pPainter->device()->physicalDpiX();

	unsigned short r = 0, g = 0, b = 0;
	m_pPresLib->getS52Color(lineColor, r, g, b);

	QBrush brush(QColor(r, g, b));
	brush.setStyle(Qt::SolidPattern);
	pPainter->save();
	pPainter->setPen(Qt::NoPen);
	pPainter->setBrush(brush);

	qreal radius = 0.6 * dpi / 25.4 * scale;
	qreal snap = 1.2 * dpi / 25.4 * scale;

	QPointF curPoint, nextPoint, p0;
	QRectF circleRect;

	float runingSegLength = 0.0;	// distance to the current segment
	float measure = 0.0;			// total distance to the start position

	for (int i = 0; i < size - 1; i++)
	{
		curPoint = pts[i];
		nextPoint = pts[i + 1];
		CLine<qreal> seg(curPoint.x(),curPoint.y(), nextPoint.x(),nextPoint.y());
		float lengthSeg = seg.length();
		float ang = seg.angle();
		double sina = sin(ang);
		double cosa = cos(ang);

		while (1)
		{
			float curLength = measure - runingSegLength;
			p0.setX(seg._x1 + curLength * cosa);
			p0.setY(seg._y1 + curLength * sina);
			circleRect.setTopLeft(QPointF(p0.x() - 0.5 * radius, p0.y() - 0.5 * radius));
			circleRect.setBottomRight(QPointF(p0.x() + 0.5 * radius, p0.y() + 0.5 * radius));
			pPainter->drawEllipse(circleRect);
			measure = measure + snap;
			if (measure > runingSegLength + lengthSeg)
				break;
		}
		runingSegLength += lengthSeg;
	}
	pPainter->restore();
}

void CS57LineSymbol::renderLC(QPainter* pPainter, CPolyline& pts, float scale, const QString& lnstName)
{
	float dpi = scale * pPainter->device()->physicalDpiY();

	QPointF prePoint, curPoint, nextPoint;
	unsigned short numCurPoint;
	float preCross = 0.0;	// 当前线段与上一条线段叉积，确定当前线段另一端点在上一条线的左、右侧或线上
	float nextCross = 0.0;	// 当前线段与下一条线段叉积，确定下条线段的另一端点在当前线的左、右侧或线上

	float preAng = 0.0;		// 上一条线段方向角	
	float curAng = 0.0;		// 当前线段方向角
	float nextAng = 0.0;	// 下一条线段方向角
	float oldAng = 0.0;		// 线闭合时，
	float oldCross = 0.0;

	float radius;			// 宽度
	QPointF bufferPts[2];	// 当前线段

	float runingSegLength = 0.0;	// distance to the current segment
	float measure = 0.0;			// total distance to the start position
	float snap = 0.0;
	bool closeid = false;

	int size = pts.size();
	if (size < 2)
		return;

	LNST* curLNST = m_pPresLib->getLnst(lnstName);
	if (curLNST == nullptr)
		return;

	radius = 5. * dpi / 25.40;

	float tt = curLNST->m_fieldLind.LBXC			// 线符号单元左上角（原点）横坐标
		- curLNST->m_fieldLind.LICL;				// 线符号单元定位点 横坐标
	float tt1 = curLNST->m_fieldLind.LICL - curLNST->m_fieldLind.LBXC - curLNST->m_fieldLind.LIHL;
	if (tt1 > 0) tt = tt1;
	// 定位点是否在符号框内,确定线符号循环长度
	if (tt > 0)
		snap = (curLNST->m_fieldLind.LIHL + tt) * 0.01 *dpi / 25.4 /**scale*/;
	else
		snap = curLNST->m_fieldLind.LIHL * 0.01 * dpi / 25.4 /**scale*/;

	curPoint = pts[0];
	nextPoint = pts[1];
	if (pts[0].x() == pts[size - 1].x() &&
		pts[0].y() == pts[size - 1].y() &&
		size > 2)
	{
		closeid = true;
		prePoint = pts[size - 2];
		//前后线段所形成的向量的叉积
		preCross = LocOfPointAndLine(prePoint, curPoint, nextPoint);
		//两线段夹角对角线的方位角
		preAng = atan2(curPoint.y() - prePoint.y(), curPoint.x() - prePoint.x());
		curAng = atan2(nextPoint.y() - curPoint.y(), nextPoint.x() - curPoint.x());
		oldCross = preCross;
		oldAng = curAng;
		nextCross = preCross;
	}
	else
	{
		closeid = false;
		// 前后线段所形成的向量的叉积
		preCross = 0;
		// 两线段夹角对角线的方位角
		curAng = atan2(nextPoint.y() - curPoint.y(), nextPoint.x() - curPoint.x());
		preAng = curAng;
	}

	for (int i = 0; i < size - 1; i++)
	{
		curPoint = pts[i];
		nextPoint = pts[i + 1];
		CLine<qreal> seg(curPoint.x(),curPoint.y(), nextPoint.x(),nextPoint.y());
		float lengthSeg = seg.length();
		float ang = seg.angle();
		double sina = sin(ang);
		double cosa = cos(ang);
		ang = ang * 180. / PI;
		bufferPts[0] = curPoint;
		bufferPts[1] = nextPoint;
		preCross = nextCross;
		if (i + 2 == size)
		{
			if (closeid)
			{
				nextCross = oldCross;
				nextAng = oldAng;
			}
			else
			{
				nextCross = 0;
				nextAng = curAng;
			}
		}
		else
		{
			prePoint = pts[i];
			curPoint = pts[i + 1];
			nextPoint = pts[i + 2];
			nextCross = LocOfPointAndLine(prePoint, curPoint, nextPoint);
			nextAng = atan2(nextPoint.y() - curPoint.y(), nextPoint.x() - curPoint.x());
		}
		QPainterPath path;
		setClipPath(preCross, nextCross, preAng, curAng, nextAng, radius, bufferPts, path);
		m_fPreAng = preAng;
		m_fCurAng = curAng;
		m_fNextAng = nextAng;
		m_StartPt = bufferPts[0];
		m_EndPt = bufferPts[1];
		m_ClipPath = path;
		//pPainter->drawPath(path);

		preCross = nextCross;
		preAng = curAng;
		curAng = nextAng;
		QPoint p0;

		//pPainter->save();
		//QRectF clipRect = pPainter->clipBoundingRect();

		while (1)
		{
			float curLength = measure - runingSegLength;
			p0.setX(seg._x1 + curLength * cosa);
			p0.setY(seg._y1 + curLength * sina);
			//if (clipRect.contains(p0))
			renderLnst(pPainter, p0, curLNST, scale, ang);
			if (measure + snap > runingSegLength + lengthSeg)
				break;
			measure = measure + snap;
		}
		runingSegLength += lengthSeg;
		//pPainter->restore();
	}
}

void CS57LineSymbol::renderLnst(QPainter* pPainter, QPoint& posPoint, LNST* pLnst, float scale, float rotation)
{
	float scaleX = pPainter->device()->physicalDpiX() / 2540.;
	float scaleY = pPainter->device()->physicalDpiX() / 2540.;

	QPoint pointPivot;	// 符号的定位点
	pointPivot.setX(pLnst->m_fieldLind.LICL);
	pointPivot.setY(pLnst->m_fieldLind.LIRW);

	//QPoint pointUperLeft; // 符号左上角点坐标
	//pointUperLeft.setX(p_CurLnst->fieldLIND.LBXC);
	//pointUperLeft.setY(p_CurLnst->fieldLIND.LBXR);

	//QRect symRect;
	//symRect.setTopLeft(pointUperLeft);
	//symRect.setWidth(p_CurLnst->fieldLIND.LIHL);
	//symRect.setHeight(p_CurLnst->fieldLIND.LIVL);

	QRect clipRect = pPainter->clipBoundingRect().toRect();
	QPainterPath curClipPath;
	if (clipRect.width() > 0 && clipRect.height() > 0)
	{
		QPainterPath tmpPath;
		tmpPath.addRect(clipRect);
		curClipPath = tmpPath.intersected(m_ClipPath);
	}
	else
		curClipPath = m_ClipPath;
	pPainter->save();
	pPainter->setClipPath(curClipPath);
	// 按输入坐标、比例、角度设置转换
	pPainter->translate(posPoint.x(), posPoint.y());
	pPainter->scale(scale, scale);
	pPainter->rotate(rotation);

	// 进入符号坐标系统
	pPainter->scale(scaleX, scaleY);
	pPainter->translate(-pointPivot.x(), -pointPivot.y());

	//QTransform curTrans = pPainter->transform();
	//QPointF p1 = curTrans.map(startP);
	//QPointF p2 = curTrans.map(endP);

	for (int i = 0; i < pLnst->m_vecS52Layers.size(); i++)
	{
		QColor curColor;
		unsigned short r = 0, g = 0, b = 0;
		m_pPresLib->getS52Color(pLnst->m_vecS52Layers[i]->strColorName, r, g, b);
		curColor.setRgb(r, g, b);
		curColor.setAlphaF(1.0 - pLnst->m_vecS52Layers[i]->uTransparent * 0.25);

		QPen curPen;
		curPen.setWidth(pLnst->m_vecS52Layers[i]->nPenWidth / scaleY/* * 20*/);	// modify by wb 2021.04.29
		curPen.setColor(curColor);
		pPainter->setPen(curPen);

		QBrush curBrush(curColor);
		curBrush.setStyle(Qt::SolidPattern);

		pPainter->setBrush(curBrush);

		for (int j = 0; j < pLnst->m_vecS52Layers[i]->vecPolygon.size(); j++)
		{
			QPolygon polygon;
			polygon.reserve(pLnst->m_vecS52Layers[i]->vecPolygon[j]->vecLine.size());
			for (int k = 0; k < pLnst->m_vecS52Layers[i]->vecPolygon[j]->vecLine.size(); k++)
			{
				polygon.push_back(*pLnst->m_vecS52Layers[i]->vecPolygon[j]->vecLine[k]);
			}
			QPen pen = pPainter->pen();
			QBrush brush = pPainter->brush();
			switch (pLnst->m_vecS52Layers[i]->vecPolygon[j]->modeFill)
			{
			case 0:
				pPainter->setBrush(Qt::NoBrush);
				break;
			case 1:
				pPainter->setPen(Qt::NoPen);
				break;
			case 2:
				break;
			}
			pPainter->drawPolygon(polygon);
			polygon.clear();
			pPainter->setPen(pen);
			pPainter->setBrush(brush);
		}

		for (int j = 0; j < pLnst->m_vecS52Layers[i]->vecLines.size(); j++)
		{
			CPolyline vecLines;
			vecLines.reserve(pLnst->m_vecS52Layers[i]->vecLines[j]->vecLine.size());
			for (int k = 0; k < pLnst->m_vecS52Layers[i]->vecLines[j]->vecLine.size(); k++)
			{
				vecLines.push_back(*pLnst->m_vecS52Layers[i]->vecLines[j]->vecLine[k]);
			}
			pPainter->drawPolyline(vecLines);
			vecLines.clear();
		}

		for (int j = 0; j < pLnst->m_vecS52Layers[i]->vecCircle.size(); j++)
		{
			QRect circleRect;
			S52Circle* pCircle = pLnst->m_vecS52Layers[i]->vecCircle[j];
			circleRect.setLeft(pCircle->cenPoint.x() - pCircle->radius);
			circleRect.setTop(pCircle->cenPoint.y() - pCircle->radius);
			circleRect.setWidth(2 * pCircle->radius);
			circleRect.setHeight(2 * pCircle->radius);
			QPen tmpPen = pPainter->pen();
			QBrush tmpBrush = pPainter->brush();
			switch (pLnst->m_vecS52Layers[i]->vecCircle[j]->modeFill)
			{
			case 0:
				pPainter->setBrush(Qt::NoBrush);
				break;
			case 1:
				pPainter->setPen(Qt::NoPen);
				break;
			case 2:

				pPainter->drawArc(circleRect, 0, 5760);
				break;
			}
			pPainter->drawEllipse(circleRect);
			pPainter->setPen(tmpPen);
			pPainter->setBrush(tmpBrush);
		}
	}

	for (int i = 0; i < pLnst->m_vecInstSC.size(); i++)
	{
		SYMB* pCurSymb = m_pPresLib->getSymb(pLnst->m_vecInstSC[i]->symbName);
		if (pCurSymb == nullptr)
			continue;

		pPainter->save();
		pPainter->translate(pLnst->m_vecInstSC[i]->piovtPoint);
		pPainter->rotate(pLnst->m_vecInstSC[i]->orient);

		QPoint pointPivot;	// 符号的定位点
		pointPivot.setX(pCurSymb->m_fieldSymd.SYCL);
		pointPivot.setY(pCurSymb->m_fieldSymd.SYRW);

		QPoint pointUperLeft; // 符号左上角点坐标
		pointUperLeft.setX(pCurSymb->m_fieldSymd.SBXC);
		pointUperLeft.setY(pCurSymb->m_fieldSymd.SBXR);

		QRect symRect; // 符号外接矩形
		symRect.setTopLeft(pointUperLeft);
		symRect.setWidth(pCurSymb->m_fieldSymd.SYHL);
		symRect.setHeight(pCurSymb->m_fieldSymd.SYVL);

		//pPainter->save();
		// 按输入坐标、比例、角度设置转换
		pPainter->translate(-pointPivot.x(), -pointPivot.y());

		// 判断符号是否完全在剪切多边形内
		QPainterPath clipPath = pPainter->clipPath();
		QRectF rectClip = clipPath.boundingRect();

		if (!clipPath.contains(symRect) && !clipPath.isEmpty()/*&&!tmpClipPath.contains(pointPivot)*/)
		{
			// 符号有一半在剪切矩形外
			if ((symRect.x() + 0.5 * symRect.width()) < rectClip.x() || (symRect.x() + 0.5*symRect.width()) > (rectClip.x() + rectClip.width()))
			{
				pPainter->restore();
				continue;
			}
		};
		//pPainter->setClipping(false);
		renderSymbol(pPainter, pCurSymb, 0);
		pPainter->restore();
	}

	pPainter->restore();
}

void CS57LineSymbol::renderSymbol(QPainter* pPainter, SYMB* pSymbol, float rotation)
{
	pPainter->save();


	for (int i = 0; i < pSymbol->m_vecS52Layers.size(); i++)
	{
		QColor curColor;
		unsigned short r = 0, g = 0, b = 0;
		m_pPresLib->getS52Color(pSymbol->m_vecS52Layers[i]->strColorName, r, g, b);
		curColor.setRgb(r, g, b);
		curColor.setAlphaF(1. - pSymbol->m_vecS52Layers[i]->uTransparent * 0.25);

		QPen curPen;
		curPen.setWidth(pSymbol->m_vecS52Layers[i]->nPenWidth * 2540./ pPainter->device()->physicalDpiX()/* * 20*/);  // modify by wb 2021.04.29
		curPen.setColor(curColor);
		pPainter->setPen(curPen);

		QBrush curBrush(curColor);
		curBrush.setStyle(Qt::SolidPattern);

		pPainter->setBrush(curBrush);

		for (int j = 0; j < pSymbol->m_vecS52Layers[i]->vecPolygon.size(); j++)
		{
			QPolygon polygon;
			polygon.reserve(pSymbol->m_vecS52Layers[i]->vecPolygon[j]->vecLine.size());
			for (int k = 0; k < pSymbol->m_vecS52Layers[i]->vecPolygon[j]->vecLine.size(); k++)
			{
				polygon.push_back(*pSymbol->m_vecS52Layers[i]->vecPolygon[j]->vecLine[k]);
			}
			QPen pen = pPainter->pen();
			QBrush brush = pPainter->brush();
			switch (pSymbol->m_vecS52Layers[i]->vecPolygon[j]->modeFill)
			{
			case 0:
				pPainter->setBrush(Qt::NoBrush);
				break;
			case 1:
				pPainter->setPen(Qt::NoPen);
				break;
			case 2:
				break;
			}
			pPainter->drawPolygon(polygon);
			polygon.clear();
			pPainter->setPen(pen);
			pPainter->setBrush(brush);
		}

		for (int j = 0; j < pSymbol->m_vecS52Layers[i]->vecLines.size(); j++)
		{
			CPolyline vecLines;
			vecLines.reserve(pSymbol->m_vecS52Layers[i]->vecLines[j]->vecLine.size());
			for (int k = 0; k < pSymbol->m_vecS52Layers[i]->vecLines[j]->vecLine.size(); k++)
			{
				vecLines.push_back(*pSymbol->m_vecS52Layers[i]->vecLines[j]->vecLine[k]);
			}
			pPainter->drawPolyline(vecLines);
			vecLines.clear();
		}

		for (int j = 0; j < pSymbol->m_vecS52Layers[i]->vecCircle.size(); j++)
		{
			QRect circleRect;
			S52Circle* pCircle = pSymbol->m_vecS52Layers[i]->vecCircle[j];
			circleRect.setLeft(pCircle->cenPoint.x() - pCircle->radius);
			circleRect.setTop(pCircle->cenPoint.y() - pCircle->radius);
			circleRect.setWidth(2 * pCircle->radius);
			circleRect.setHeight(2 * pCircle->radius);
			QPen pen = pPainter->pen();
			QBrush brush = pPainter->brush();
			switch (pSymbol->m_vecS52Layers[i]->vecCircle[j]->modeFill)
			{
			case 0:
				pPainter->setBrush(Qt::NoBrush);
				break;
			case 1:
				pPainter->setPen(Qt::NoPen);
				break;
			case 2:

				pPainter->drawArc(circleRect, 0, 5760);
				break;
			}
			pPainter->drawEllipse(circleRect);
			pPainter->setPen(pen);
			pPainter->setBrush(brush);
		}
	}

	pPainter->restore();
}

int CS57LineSymbol::LocOfPointAndLine(QPointF p1, QPointF p2, QPointF p0)
{
	return (p2.x() - p1.x())*(p0.y() - p1.y()) - (p2.y() - p1.y())*(p0.x() - p1.x());
}

int CS57LineSymbol::LocOfPointAndLine(QPoint p1, QPoint p2, QPoint p0)
{
	return (p2.x() - p1.x())*(p0.y() - p1.y()) - (p2.y() - p1.y())*(p0.x() - p1.x());
}

void CS57LineSymbol::setClipPath(float preCross, float nextCross, float preAng, float curAng, float nextAng, float radius, QPointF points[2], QPainterPath& path)
{
	QPointF buffer[6];

	float k, r, d;
	float ang, sina, cosa;

	if (preCross > 0)
	{
		ang = curAng + PI * 1.5;
		sina = sin(ang);
		cosa = cos(ang);
		k = (curAng - preAng - PI) * 0.5;
		r = preAng + k;
		d = radius / sin(k);
		buffer[1].setX(points[0].x() + radius * cosa);
		buffer[1].setY(points[0].y() + radius * sina);
		buffer[5].setX(points[0].x() + d * cos(r));
		buffer[5].setY(points[0].y() + d * sin(r));

	}
	else if (preCross < 0)
	{
		ang = curAng + PI * 0.5;
		sina = sin(ang);
		cosa = cos(ang);
		k = (curAng - preAng - PI) * 0.5;
		r = preAng + k;
		d = -radius / sin(k);
		buffer[1].setX(points[0].x() + d * cos(r));
		buffer[1].setY(points[0].y() + d * sin(r));
		buffer[5].setX(points[0].x() + radius * cosa);
		buffer[5].setY(points[0].y() + radius * sina);
	}
	else
	{
		ang = curAng + PI * 1.5;
		sina = sin(ang);
		cosa = cos(ang);
		buffer[1].setX(points[0].x() + radius * cosa);
		buffer[1].setY(points[0].y() + radius * sina);
		buffer[5].setX(points[0].x() - radius * cosa);
		buffer[5].setY(points[0].y() - radius * sina);
	}

	if (nextCross > 0)
	{
		ang = curAng + PI * 1.5;
		sina = sin(ang);
		cosa = cos(ang);
		k = (nextAng - curAng - PI) * 0.5;
		r = curAng + k;
		d = radius / sin(k);
		buffer[4].setX(points[1].x() + d * cos(r));
		buffer[4].setY(points[1].y() + d * sin(r));
		buffer[2].setX(points[1].x() + radius * cosa);
		buffer[2].setY(points[1].y() + radius * sina);
	}
	else if (nextCross < 0)
	{
		ang = curAng + PI * 0.5;
		sina = sin(ang);
		cosa = cos(ang);
		k = (nextAng - curAng - PI) * 0.5;
		r = curAng + k;
		d = -radius / sin(k);
		buffer[2].setX(points[1].x() + d * cos(r));
		buffer[2].setY(points[1].y() + d * sin(r));
		buffer[4].setX(points[1].x() + radius * cosa);
		buffer[4].setY(points[1].y() + radius * sina);
	}
	else
	{
		ang = curAng + PI * 0.5;
		sina = sin(ang);
		cosa = cos(ang);
		buffer[2].setX(points[1].x() - radius * cosa);
		buffer[2].setY(points[1].y() - radius * sina);
		buffer[4].setX(points[1].x() + radius * cosa);
		buffer[4].setY(points[1].y() + radius * sina);
	}
	buffer[0].setX(points[0].x());
	buffer[0].setY(points[0].y());
	buffer[3].setX(points[1].x());
	buffer[3].setY(points[1].y());


	QPolygonF polygon;

	for (unsigned short i = 0; i <= 6; i++)
		polygon.push_back(buffer[i % 6]);
	path.addPolygon(polygon);
}

CS57AreaSymbol::CS57AreaSymbol(CS57PresLib* preslib)
{
	m_pPresLib = preslib;
	m_pCurPatt = nullptr;
	m_pLineSymbol = new CS57LineSymbol(preslib);
}

CS57AreaSymbol::~CS57AreaSymbol()
{
	if (m_pLineSymbol != nullptr)
		delete m_pLineSymbol;
	m_pLineSymbol = nullptr;
}

void CS57AreaSymbol::renderAC(QPainter* pPainter, QVector<QPolygon>& polygons, unsigned short transparent, QString areaColor)
{
	unsigned short r = 0, g = 0, b = 0, a = 255;
	m_pPresLib->getS52Color(areaColor, r, g, b);
	a = 255. * (1. - transparent * 0.25);
	QBrush brush(QColor(r, g, b, a));

	QPainterPath path;
	for (int i = 0;i < polygons.size(); i++)
	{
		path.addPolygon(polygons[i]);
	}
	pPainter->fillPath(path, brush);
}

void CS57AreaSymbol::renderAP(QPainter* pPainter, QVector<QPolygon>& polygons, QString patName, float scale, float rotation)
{
	QPainterPath path;
	m_pCurPatt = m_pPresLib->getPatt(patName);
	for (int i = 0; i < polygons.size(); i++)
	{
		path.addPolygon(polygons[i]);
	}
	QRectF boundRect = path.boundingRect();

	QPoint piovtPt;
	QPixmap* pixmap = getPattPixmap(pPainter,patName, piovtPt,1.0, rotation);
	if (pixmap == nullptr)
		return;

	float pixDpiX = pixmap->physicalDpiX();
	float painterDpiX = pPainter->device()->physicalDpiX();
	pPainter->save();
	if (m_pCurPatt->m_fieldPatd.PATP != "STG")
	{
		QBrush brush(*pixmap);
		pPainter->fillPath(path,brush);
	}
	else
	{
		int x, y, num, x0, y0;
		num = 0;
		int numRow = boundRect.height() / pixmap->height() + 2;
		int numColum = boundRect.width() / pixmap->width() + 2;

		x0 = boundRect.left() + pixmap->width() * 0.5;
		y0 = boundRect.top() + pixmap->height() * 0.5;
		y = y0;

		pPainter->setClipPath(path);
		for (int i = 0; i < numRow; i++)
		{
			x = boundRect.left() + pixmap->width() * 0.5;
			if (num % 2 == 1)
				x = boundRect.left();
			for (int j = 0; j < numColum; j++)
			{
				QPoint p0(x - piovtPt.x(), y - piovtPt.y());
				QRectF rect(p0.x(), p0.y(), pixmap->width(), pixmap->height());
				pPainter->drawImage(p0, pixmap->toImage());
				x += pixmap->width();
			}
			y += pixmap->height();
			num++;
		}
	}
	pPainter->restore();

	if (pixmap != nullptr)
	{
		delete pixmap;
		pixmap = nullptr;
	}
}

void CS57AreaSymbol::renderLS(QPainter* pPainter, CPolyline& pts, float scale, int lineWidth, QString lineStyle, QString lineColor)
{
	m_pLineSymbol->renderLS(pPainter, pts, scale, lineWidth, lineStyle, lineColor);
}

void CS57AreaSymbol::renderLC(QPainter* pPainter, CPolyline& pts, float scale, const QString& lnstName)
{
	m_pLineSymbol->renderLC(pPainter, pts, scale, lnstName);
}

void CS57AreaSymbol::initPatt(QPainter * pPainter, QString pattName, QPoint& posPoint, float scale, float rotation)
{
	m_CurPoint = posPoint;
	m_patPiovt.setX(m_pCurPatt->m_fieldPatd.PACL);
	m_patPiovt.setY(m_pCurPatt->m_fieldPatd.PARW);

	QPoint pointUperLeft; // 符号左上角点坐标
	pointUperLeft.setX(m_pCurPatt->m_fieldPatd.PBXC);
	pointUperLeft.setY(m_pCurPatt->m_fieldPatd.PBXR);

	QRect symRect;
	symRect.setTopLeft(pointUperLeft);
	symRect.setWidth(m_pCurPatt->m_fieldPatd.PAHL);
	symRect.setHeight(m_pCurPatt->m_fieldPatd.PAVL);

	float dpiX = pPainter->device()->physicalDpiX();
	float dpiY = pPainter->device()->physicalDpiY();
	m_fPainterScaleX = dpiX / 2540.;
	m_fPainterScaleY = dpiY / 2540.;

	QTransform trans;
	trans.translate(posPoint.x(), posPoint.y());
	trans.scale(scale, scale);
	trans.rotate(rotation);
	trans.scale(m_fPainterScaleX, m_fPainterScaleY);
	trans.translate(-m_patPiovt.x(), -m_patPiovt.y());
}

void CS57AreaSymbol::initPatt(float dpiX, float dpiY, QString pattName, QPoint& posPoint, float scale, float rotation)
{
	m_CurPoint = posPoint;
	m_patPiovt.setX(m_pCurPatt->m_fieldPatd.PACL);
	m_patPiovt.setY(m_pCurPatt->m_fieldPatd.PARW);

	QPoint pointUperLeft; // 符号左上角点坐标
	pointUperLeft.setX(m_pCurPatt->m_fieldPatd.PBXC);
	pointUperLeft.setY(m_pCurPatt->m_fieldPatd.PBXR);

	QRect symRect;
	symRect.setTopLeft(pointUperLeft);
	symRect.setWidth(m_pCurPatt->m_fieldPatd.PAHL);
	symRect.setHeight(m_pCurPatt->m_fieldPatd.PAVL);

	m_fPainterScaleX = dpiX / 2540.;
	m_fPainterScaleY = dpiY / 2540.;

	QTransform trans;
	trans.translate(posPoint.x(), posPoint.y());
	trans.scale(scale, scale);
	trans.rotate(rotation);
	trans.scale(m_fPainterScaleX, m_fPainterScaleY);
	trans.translate(-m_patPiovt.x(), -m_patPiovt.y());
}

void CS57AreaSymbol::renderPatt(QPainter* pPainter, float scale, float rotation)
{
	pPainter->save();
	// 按输入坐标、比例、角度设置转换
	pPainter->translate(m_CurPoint.x(), m_CurPoint.y());
	pPainter->scale(scale, scale);
	pPainter->rotate(rotation);

	pPainter->scale(m_fPainterScaleX, m_fPainterScaleY);
	pPainter->translate(-m_patPiovt.x(), -m_patPiovt.y());

	for (int i = 0; i < m_pCurPatt->m_vecS52Layers.size(); i++)
	{
		QColor curColor;
		S52Color* pColor = m_pPresLib->getS52Color(m_pCurPatt->m_vecS52Layers[i]->strColorName);
		curColor.setRgb(pColor->r, pColor->g, pColor->b);
		curColor.setAlphaF(1. - m_pCurPatt->m_vecS52Layers[i]->uTransparent * 0.25);

		QPen curPen;
		curPen.setWidth(m_pCurPatt->m_vecS52Layers[i]->nPenWidth / m_fPainterScaleY);
		curPen.setColor(curColor);
		pPainter->setPen(curPen);

		QBrush curBrush(curColor);
		curBrush.setStyle(Qt::SolidPattern);

		pPainter->setBrush(curBrush);
		if (m_pCurPatt->m_vecS52Layers[i]->vecPolygon.size() > 0)
		{
			for (int j = 0; j < m_pCurPatt->m_vecS52Layers[i]->vecPolygon.size(); j++)
			{
				QPolygon polygon;
				polygon.reserve(m_pCurPatt->m_vecS52Layers[i]->vecPolygon[j]->vecLine.size());
				for (int k = 0; k < m_pCurPatt->m_vecS52Layers[i]->vecPolygon[j]->vecLine.size(); k++)
				{
					polygon.push_back(*m_pCurPatt->m_vecS52Layers[i]->vecPolygon[j]->vecLine[k]);
				}
				QPen tmpPen = pPainter->pen();
				QBrush tmpBrush = pPainter->brush();
				switch (m_pCurPatt->m_vecS52Layers[i]->vecPolygon[j]->modeFill)
				{
				case 0:
					pPainter->setBrush(Qt::NoBrush);
					break;
				case 1:
					pPainter->setPen(Qt::NoPen);
					break;
				case 2:
					break;
				}
				pPainter->drawPolygon(polygon);
				polygon.clear();
				pPainter->setPen(tmpPen);
				pPainter->setBrush(tmpBrush);
			}
		}

		for (int j = 0; j < m_pCurPatt->m_vecS52Layers[i]->vecLines.size(); j++)
		{
			CPolyline tmpLines;
			tmpLines.reserve(m_pCurPatt->m_vecS52Layers[i]->vecLines[j]->vecLine.size());
			for (int k = 0; k < m_pCurPatt->m_vecS52Layers[i]->vecLines[j]->vecLine.size(); k++)
			{
				tmpLines.push_back(*m_pCurPatt->m_vecS52Layers[i]->vecLines[j]->vecLine[k]);
			}
			pPainter->drawPolyline(tmpLines);
			tmpLines.clear();
		}

		for (int j = 0; j < m_pCurPatt->m_vecS52Layers[i]->vecCircle.size(); j++)
		{
			QRect circleRect;
			S52Circle* pCircle = m_pCurPatt->m_vecS52Layers[i]->vecCircle[j];
			circleRect.setLeft(pCircle->cenPoint.x() - pCircle->radius);
			circleRect.setTop(pCircle->cenPoint.y() - pCircle->radius);
			circleRect.setWidth(2 * pCircle->radius);
			circleRect.setHeight(2 * pCircle->radius);
			QPen tmpPen = pPainter->pen();
			QBrush tmpBrush = pPainter->brush();
			switch (m_pCurPatt->m_vecS52Layers[i]->vecCircle[j]->modeFill)
			{
			case 0:
				pPainter->setBrush(Qt::NoBrush);
				break;
			case 1:
				pPainter->setPen(Qt::NoPen);
				break;
			case 2:
				pPainter->drawArc(circleRect, 0, 5760);
				break;
			}
			pPainter->drawEllipse(circleRect);
			pPainter->setPen(tmpPen);
			pPainter->setBrush(tmpBrush);
		}
	}
	pPainter->restore();
}

QPixmap * CS57AreaSymbol::getPattPixmap(QPainter* pPainter, QString pattName, QPoint& piovtPt, float scale, float rotation)
{
	if (m_pCurPatt == nullptr)
		return nullptr;

	float dpiX = pPainter->device()->physicalDpiX();
	float dpiY = pPainter->device()->physicalDpiY();

	QPoint patPiovt;	// 符号的定位点
	patPiovt.setX(m_pCurPatt->m_fieldPatd.PACL);
	patPiovt.setY(m_pCurPatt->m_fieldPatd.PARW);

	// 符号左上角点坐标
	QPoint pointUperLeft;
	pointUperLeft.setX(m_pCurPatt->m_fieldPatd.PBXC);
	pointUperLeft.setY(m_pCurPatt->m_fieldPatd.PBXR);

	int symPAMI = m_pCurPatt->m_fieldPatd.PAMI;// 模板符号遮盖部分（边界框+中枢点）之间的最小距离PAMI I(5)(单位0.01 mm)其中 0 <= PAMI <= 32767 */
	int symPAMA = m_pCurPatt->m_fieldPatd.PAMA;// 模板符号遮盖部分（边界框+中枢点）之间的最大距离PAMA I(5) (单位0.01 mm)其中 0 <= PAMA <= 32767 若 PASP = ‘CON’，则PAMA无意义 * /
	int mWidth = m_pCurPatt->m_fieldPatd.PAHL;
	int mHeight = m_pCurPatt->m_fieldPatd.PAVL;
	int symWidth = std::max(symPAMI, mWidth);
	int symHeight = std::max(symPAMI, mHeight);

	QRect symRect;
	symRect.setTopLeft(pointUperLeft);
	symRect.setWidth(symWidth);
	symRect.setHeight(symHeight);

	float pixScaleX = dpiX / 2540.;
	float pixScaleY = dpiY / 2540.;

	QTransform trans;
	trans.scale(scale, scale);
	trans.scale(pixScaleX, pixScaleY);
	trans.translate(-pointUperLeft.x(), -pointUperLeft.y());
	symRect = trans.mapRect(symRect);
	pointUperLeft = trans.map(pointUperLeft);
	piovtPt = trans.map(patPiovt);

	int w = 30 * pixScaleY * scale;
	piovtPt = QPoint(piovtPt.x() + w, piovtPt.y() + w);

	QPixmap* pPixmap = new QPixmap(symRect.width() + 2 * w, symRect.height() + 2 * w);
	pPixmap->fill(QColor(255, 0, 0, 0));
	QPainter painter(pPixmap);
	initPatt(dpiX, dpiY, pattName, piovtPt, scale, rotation);
	renderPatt(&painter, scale, rotation);

	return pPixmap;
}

