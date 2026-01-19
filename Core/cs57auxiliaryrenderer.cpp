#include "stdafx.h"
#include "cs57auxiliaryrenderer.h"

using namespace Core;
CS57AuxiliaryRenderer::CS57AuxiliaryRenderer()
	: m_bEnabled(false)
{}

CS57AuxiliaryRenderer::~CS57AuxiliaryRenderer()
{}

void CS57AuxiliaryRenderer::setEnabled(bool enable)
{
	m_bEnabled = enable;
}

void CS57AuxiliaryRenderer::renderCrossLine(QPainter* painter, QPoint pt)
{
	int nLength = 30;
	QPen pen(QColor(255, 0, 0, 255), 1);
	painter->save();
	QPoint centerPt = pt;

	//左侧点
	int x_left = centerPt.x() - nLength / 2;
	int y_left = centerPt.y();
	QPoint leftPt(x_left, y_left);
	//右侧点
	int x_right = centerPt.x() + nLength / 2;
	int y_right = centerPt.y();
	QPoint rightPt(x_right, y_right);
	//横线
	QLine hLine(leftPt, rightPt);

	//上侧点
	int x_top = centerPt.x();
	int y_top = centerPt.y() - nLength / 2;
	QPoint topPt(x_top, y_top);
	//下侧点
	int x_bottom = centerPt.x();
	int y_bottom = centerPt.y() + nLength / 2;
	QPoint bottomPt(x_bottom, y_bottom);
	//竖线
	QLine vLine(topPt, bottomPt);

	painter->setPen(pen);
	painter->drawLine(hLine);
	painter->drawLine(vLine);

	painter->restore();
}

void CS57AuxiliaryRenderer::renderCrossLine(QPainter* painter, QPoint pt, QColor color,int penWidth)
{
	int nLength = 30;
	QPen pen(color, penWidth);
	painter->save();
	QPoint centerPt = pt;

	//左侧点
	int x_left = centerPt.x() - nLength / 2;
	int y_left = centerPt.y();
	QPoint leftPt(x_left, y_left);
	//右侧点
	int x_right = centerPt.x() + nLength / 2;
	int y_right = centerPt.y();
	QPoint rightPt(x_right, y_right);
	//横线
	QLine hLine(leftPt, rightPt);

	//上侧点
	int x_top = centerPt.x();
	int y_top = centerPt.y() - nLength / 2;
	QPoint topPt(x_top, y_top);
	//下侧点
	int x_bottom = centerPt.x();
	int y_bottom = centerPt.y() + nLength / 2;
	QPoint bottomPt(x_bottom, y_bottom);
	//竖线
	QLine vLine(topPt, bottomPt);

	painter->setPen(pen);
	painter->drawLine(hLine);
	painter->drawLine(vLine);

	painter->restore();
}

void CS57AuxiliaryRenderer::doRender(QPainter* painter)
{
	if (!m_bEnabled)
		return;

	int nLength = 30;
	QPen pen(QColor(255, 0, 0, 255), 1);
	painter->save();
	QPoint centerPt = m_CenterPt;

	//左侧点
	int x_left = centerPt.x() - nLength / 2;
	int y_left = centerPt.y();
	QPoint leftPt(x_left, y_left);
	//右侧点
	int x_right = centerPt.x() + nLength / 2;
	int y_right = centerPt.y();
	QPoint rightPt(x_right, y_right);
	//横线
	QLine hLine(leftPt, rightPt);

	//上侧点
	int x_top = centerPt.x();
	int y_top = centerPt.y() - nLength / 2;
	QPoint topPt(x_top, y_top);
	//下侧点
	int x_bottom = centerPt.x();
	int y_bottom = centerPt.y() + nLength / 2;
	QPoint bottomPt(x_bottom, y_bottom);
	//竖线
	QLine vLine(topPt, bottomPt);

	painter->setPen(pen);
	painter->drawLine(hLine);
	painter->drawLine(vLine);

	painter->restore();
}

void CS57AuxiliaryRenderer::setCenterPt(QPoint pt)
{
	m_CenterPt = pt;
}
