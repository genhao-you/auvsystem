#pragma once

#include <QObject>
#include "xt_global.h"
#include "cs57canvas.h"
#include <QBrush>
#include <QPen>

class CS57RubberBand : public QObject, public CS57CanvasItem
{
	Q_OBJECT

public:
	CS57RubberBand(CS57Control* control, XT::GeometryType geometryType);
	~CS57RubberBand();

	void addPoint(QPoint pt);//像素坐标，不会跟着图动
	void movePoint(QPoint pt);
	void calcBoundingBox();
	void setColor(const QColor &color);
	void setFillColor(const QColor &color);
	void setStrokeColor(const QColor &color);
	void setSecondaryStrokeColor(const QColor &color);
	void setWidth(int width);
	void setLineStyle(Qt::PenStyle penStyle);
	void setBrushStyle(Qt::BrushStyle brushStyle);
	void setToCanvasRectangle(QRect rect);
protected:
	void paint(QPainter *p) override;
private:
	XT::GeometryType m_GeometryType = XT::PolygonGeometry;
	QBrush m_Brush;
	QPen m_Pen;
	QPen m_SecondaryPen;
	QRect m_Rect;
	QVector<QPoint> m_vecPoint;
};
