#pragma once
#include "geometry.h"
#include "cline.h"
#include <QVector>
#include <QPoint>
#include <QRect>
/************************************************************************/
/* 空间关系判断多边形                                                   */
/************************************************************************/
class GpcPolygon;
class CSpatialPolygon
{
public:
	CSpatialPolygon();
	CSpatialPolygon(QVector<QVector<QPointF>*> vecRings);
	~CSpatialPolygon();

	void setPolygon(QVector<QVector<QPointF>*> vecRings);
	bool ptInPolygon(QPointF pt);
	bool ptInPolygon(double x,double y);
	bool isIntersect(QVector<QVector<QPointF>*>* pRings);

	double minX() { return m_dMinX; }
	double minY() { return m_dMinY; }
	double maxX() { return m_dMaxX; }
	double maxY() { return m_dMaxY; }

	bool isIntersectPolyline(QVector<QVector<QPointF>*>* pRings);
	bool isIntersectPolyline(CPolylineF* pLine);
	bool isIntersectPolygon(CPolylineF* pRing);
	void relPolygon();
	//转换成GPC Polygon内存未释放,外部释放
	GpcPolygon* toGpcPolygon();//转换成gpcPolygon
private:
	bool isIntersectLinesegment(QPointF linesegment1[2], QPointF linesegment2[2]);
private:
	//第一层：环数
	//第二层：环点数
	//第三层：环点坐标
	QVector<QVector<QPointF>*> m_vecRings;
	GpcPolygon* m_pGpcPolygon;
	double m_dMinX, m_dMinY, m_dMaxX, m_dMaxY;
};
//多边形裁剪
class CS57PolygonClip
{
public:
	CS57PolygonClip();
	~CS57PolygonClip();

	void setRect(QRectF rectIn);
	void setCoorSysId(bool CoorSysID);
	bool qujiao(float& x, float& y, Line l1, Line l2);
	bool Inside(QPointF p, int i);
	void Intersect(QPointF S, QPointF P, int i, QPointF &ans);
	void Sutherland_Hodgman(QVector<QPointF> &cv);
	void PolypolylineClip(QVector<QVector<QPointF>> &cv);

private:
	bool m_bCoorSysID;
	QRectF m_ClipRect;
};