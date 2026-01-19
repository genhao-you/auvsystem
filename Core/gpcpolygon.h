#pragma once
#include "core_global.h"
extern "C" {
#include "gpc.h"
}
#include "geometry.h"
class GpcPolygon
{
public:
	GpcPolygon();
	GpcPolygon(GpcPolygon& polygon);
	~GpcPolygon();

	void createPolygon(CPolylineF &vecPtFs);
	void createPolygon(CPolyline &vecPts);
	void add_contour(CPolylineF &vecPtFs, int hole);
	void add_contour(CPolyline &vecPts, int hole);
	void polygon_clip(gpc_op op, GpcPolygon *clip, GpcPolygon *result);
	void freePolygon();

	void toPolygonF(QVector<QVector<QPointF>*>* pRings);//转换成存在经纬度的坐标多边形容器
	gpc_polygon m_gpcPolygon;
};

