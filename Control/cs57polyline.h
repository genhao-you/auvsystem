#pragma once
#include "control_global.h"
#include "geometry.h"
#include "cs57geometry.h"

class CS57Transform;
class CS57Polyline : public CS57Geometry
{
	Q_OBJECT
public:
	CS57Polyline(CS57Control* control,const QString& name);
	~CS57Polyline();

	void setPenWidth(int w);
	void setColor(const QColor &color);
	void setLineStyle(Qt::PenStyle penStyle);
	void addPoint(double lon, double lat);
	void addPoint(QPointF pt);//经纬度坐标
	void addPts(CPolylineF pts);
	void modityPt(int idx, QPointF pt);
	void removePt(int idx);
	void clearPt();
	void calcBoundingBox();
	QString toString(CPolyline& vecPts);
protected:
	void paint(QPainter *p) override;

public:
	QPen		m_Pen;
	CPolylineF	m_vecGeoPolyline;//只存经纬度坐标
	CPolyline	m_vecPixelPolyline;//转换后像素坐标
	CS57Transform* m_pTransform;
};