#include "stdafx.h"
#include "spatialpolygon.h"
#include "gpcpolygon.h"
#include <qmath.h>
#include <QtGlobal>

CSpatialPolygon::CSpatialPolygon()
	: m_dMaxX(0.0)
	, m_dMaxY(0.0)
	, m_dMinX(0.0)
	, m_dMinY(0.0)
{}

CSpatialPolygon::CSpatialPolygon(QVector<QVector<QPointF>*> vecRings)
	: m_dMaxX(0.0)
	, m_dMaxY(0.0)
	, m_dMinX(0.0)
	, m_dMinY(0.0)
{
	for (int i = 0; i < vecRings.size(); i++)
	{
		CPolylineF* pvecRing = new CPolylineF();
		for (int j = 0; j < vecRings[i]->size(); j++)
		{
			QPointF pt;
			pt.setX(vecRings[i]->at(j).x());
			pt.setY(vecRings[i]->at(j).y());
			pvecRing->push_back(pt);
		}
		m_vecRings.push_back(pvecRing);
	}
	if (m_vecRings.size() == 0)
		return;

	m_dMinX = vecRings[0]->at(0).x();
	m_dMinY = vecRings[0]->at(0).y();
	m_dMaxX = vecRings[0]->at(0).x();
	m_dMaxY = vecRings[0]->at(0).y();

	for (int i = 0; i < vecRings.size(); i++)
	{
		for (int j = 0; j < vecRings.at(i)->size(); j++)
		{
			m_dMinX = qMin(m_dMinX, vecRings[i]->at(j).x());
			m_dMinY = qMin(m_dMinY, vecRings[i]->at(j).y());
			m_dMaxX = qMax(m_dMaxX, vecRings[i]->at(j).x());
			m_dMaxY = qMax(m_dMaxY, vecRings[i]->at(j).y());
		}
	}
}

CSpatialPolygon::~CSpatialPolygon()
{
	relPolygon();
}
void CSpatialPolygon::setPolygon(QVector<QVector<QPointF>*> vecRings)
{
	for (int i = 0; i < vecRings.size(); i++)
	{
		CPolylineF* pvecRing = new CPolylineF();
		for (int j = 0; j < vecRings[i]->size(); j++)
		{
			QPointF pt;
			pt.setX(vecRings[i]->at(j).x());
			pt.setY(vecRings[i]->at(j).y());
			pvecRing->push_back(pt);
		}
		m_vecRings.push_back(pvecRing);
	}
	if (m_vecRings.size() == 0)
		return;

	m_dMinX = vecRings[0]->at(0).x();
	m_dMinY = vecRings[0]->at(0).y();
	m_dMaxX = vecRings[0]->at(0).x();
	m_dMaxY = vecRings[0]->at(0).y();

	for (int i = 0; i < vecRings.size(); i++)
	{
		for (int j = 0; j < vecRings[i]->size(); j++)
		{
			m_dMinX = qMin(m_dMinX,vecRings[i]->at(j).x());
			m_dMinY = qMin(m_dMinY,vecRings[i]->at(j).y());
			m_dMaxX = qMax(m_dMaxX,vecRings[i]->at(j).x());
			m_dMaxY = qMax(m_dMaxY,vecRings[i]->at(j).y());
		}
	}
}

//************************************
// Method:    ptInPolygon
// Brief:	  点在多边形内
// Returns:   bool
// Author:    cl
// DateTime:  2022/07/21
// Parameter: double x
// Parameter: double y
//************************************
bool CSpatialPolygon::ptInPolygon(double x, double y)
{
	bool result = false;
	if (x < m_dMinX || y < m_dMinY || x> m_dMaxX || y> m_dMaxY) 
		return result;	//  点在最小外接矩形外	

	for (int i = 0; i < m_vecRings.size(); i++)
	{
		int k = m_vecRings[i]->size() - 1;
		for (int j = 0; j < m_vecRings[i]->size(); j++)
		{
			double x1 = m_vecRings[i]->at(j).x();
			double x2 = m_vecRings[i]->at(k).x();
			double y1 = m_vecRings[i]->at(j).y();
			double y2 = m_vecRings[i]->at(k).y();
			if ((y1 <  y && y2 >= y || y2 <  y && y1 >= y) && (x1 <= x || x2 <= x))
			{
				result ^= ( x1 + ( y - y1 ) / ( y2 - y1 ) * ( x2 - x1 ) < x );
			}
			k = j;
		}
	}
	return result;
}

//************************************
// Method:    ptInPolygon
// Brief:	  点在多边形内
// Returns:   bool
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QPointF pt
//************************************
bool CSpatialPolygon::ptInPolygon(QPointF pt)
{
	double x = pt.x();
	double y = pt.y();

	bool result = false;
	if (x < m_dMinX || y < m_dMinY || x> m_dMaxX || y> m_dMaxY)
		return result;	//  点在最小外接矩形外	

	for (int i = 0; i < m_vecRings.size(); i++)
	{
		int k = m_vecRings[i]->size() - 1;
		for (int j = 0; j < m_vecRings[i]->size(); j++)
		{
			double x1 = m_vecRings[i]->at(j).x();
			double x2 = m_vecRings[i]->at(k).x();
			double y1 = m_vecRings[i]->at(j).y();
			double y2 = m_vecRings[i]->at(k).y();
			if ((y1 < y && y2 >= y || y2 < y && y1 >= y) && (x1 <= x || x2 <= x))
			{
				result ^= (x1 + (y - y1) / (y2 - y1) * (x2 - x1) < x);
			}
			k = j;
		}
	}
	return result;
}
//支持多环
bool CSpatialPolygon::isIntersectPolyline(QVector<QVector<QPointF>*>* pRings)
{
	// 如果折线上任一顶点在多边形内，说明线与多边形相关
	double dMinX = pRings->at(0)->at(0).x();
	double dMinY = pRings->at(0)->at(0).y();
	double dMaxX = pRings->at(0)->at(0).x();
	double dMaxY = pRings->at(0)->at(0).y();
	for (int i = 0; i < pRings->size(); i++)
	{
		for (int j = 0; j < pRings->at(i)->size(); j++)
		{
			double dx = pRings->at(i)->at(j).x();
			double dy = pRings->at(i)->at(j).y();
			if (ptInPolygon(dx, dy))
				return true;

			dMinX = qMin(dx, dMinX);
			dMinY = qMin(dy, dMinY);
			dMaxX = qMax(dx, dMaxX);
			dMaxY = qMax(dy, dMaxY);
		}
	}

	QPointF segment1[2];
	QPointF segment2[2];

	if (dMinX > m_dMaxX || dMinY > m_dMaxY || dMaxX < m_dMinX || dMaxY < m_dMinY)
		return false;	//线的最小外接矩形与多边形的最小外接矩形不相交

	for (int i = 0; i < pRings->size(); i++)
	{
		for (int j = 0; j < pRings->at(i)->size() - 1; j++)
		{
			segment1[0].setX( pRings->at(i)->at(j).x());
			segment1[0].setY( pRings->at(i)->at(j).y());
			segment1[1].setX( pRings->at(i)->at(j + 1).x());
			segment1[1].setY( pRings->at(i)->at(j + 1).y());
			for (int m = 0; m < m_vecRings.size(); m++)
			{
				for (int n = 0; n < m_vecRings[m]->size() - 1; n++)
				{
					segment2[0].setX(m_vecRings[m]->at(n).x());
					segment2[0].setY(m_vecRings[m]->at(n).y());
					segment2[1].setX(m_vecRings[m]->at(n + 1).x());
					segment2[1].setY(m_vecRings[m]->at(n + 1).y());
					if (isIntersectLinesegment(segment1, segment2))
						return true;
				}
			}
		}
	}
	return false;
}
//不支持多环
bool CSpatialPolygon::isIntersectPolyline(QVector<QPointF>* pLine)
{
	// 如果折线上任一顶点在多边形内，说明线与多边形相关
	double dMinX = pLine->at(0).x();
	double dMinY = pLine->at(0).y();
	double dMaxX = pLine->at(0).x();
	double dMaxY = pLine->at(0).y();
	for (int i = 0; i < pLine->size(); i++)
	{
		double dx = pLine->at(i).x();
		double dy = pLine->at(i).y();
		if (ptInPolygon(dx, dy))
			return true;

		dMinX = qMin(dx, dMinX);
		dMinY = qMin(dy, dMinY);
		dMaxX = qMax(dx, dMaxX);
		dMaxY = qMax(dy, dMaxY);
	}

	QPointF segment1[2];
	QPointF segment2[2];

	if (dMinX > m_dMaxX || dMinY > m_dMaxY || dMaxX < m_dMinX || dMaxY < m_dMinY)
		return false;	//线的最小外接矩形与多边形的最小外接矩形不相交

	for (int i = 0; i < pLine->size() - 1; i++)
	{
		segment1[0].setX(pLine->at(i).x());
		segment1[0].setY(pLine->at(i).y());
		segment1[1].setX(pLine->at(i + 1).x());
		segment1[1].setY(pLine->at(i + 1).y());
		for (int m = 0; m < m_vecRings.size(); m++)
		{
			for (int n = 0; n < m_vecRings[m]->size() - 1; n++)
			{
				segment2[0].setX(m_vecRings[m]->at(n).x());
				segment2[0].setY(m_vecRings[m]->at(n).y());
				segment2[1].setX(m_vecRings[m]->at(n + 1).x());
				segment2[1].setY(m_vecRings[m]->at(n + 1).y());
				if (isIntersectLinesegment(segment1, segment2))
					return true;
			}
		}
	}
	return false;
}
//************************************
// Method:    isIntersect
// Brief:	  视口多边形与面物标判断空间关系
//			  注意：外部传进来的数据外部自己释放
// Returns:   bool
// Author:    cl
// DateTime:  2021/08/23
// Parameter: QVector<QVector<QPointF>*> * pRings
//************************************
bool CSpatialPolygon::isIntersect(QVector<QVector<QPointF>*>* pRings)
{
	double dMinX = pRings->at(0)->at(0).x();
	double dMinY = pRings->at(0)->at(0).y();
	double dMaxX = pRings->at(0)->at(0).x();
	double dMaxY = pRings->at(0)->at(0).y();
	for (int i = 0; i < pRings->size(); i++)
	{
		for (int j = 0; j < pRings->at(i)->size(); j++)
		{
			double dx = pRings->at(i)->at(j).x();
			double dy = pRings->at(i)->at(j).y();
			if (ptInPolygon(dx, dy))
				return true;

			dMinX = qMin(dx, dMinX);
			dMinY = qMin(dy, dMinY);
			dMaxX = qMax(dx, dMaxX);
			dMaxY = qMax(dy, dMaxY);
		}
	}

	if (dMinX > m_dMaxX || dMinY > m_dMaxY || dMaxX < m_dMinX || dMaxY < m_dMinY)
		return false;	//线的最小外接矩形与多边形的最小外接矩形不相交

	CSpatialPolygon* pFilterPolygon = new CSpatialPolygon(*pRings);

	for (int i = 0;i < m_vecRings.size();i++)
	{
		for (int j = 0;j < m_vecRings[i]->size();j++)
		{
			double dx = m_vecRings[i]->at(j).x();
			double dy = m_vecRings[i]->at(j).y();
			if (pFilterPolygon->ptInPolygon(dx, dy))
			{
				if (pFilterPolygon != nullptr)
				{
					delete pFilterPolygon;
					pFilterPolygon = nullptr;
				}
				return true;
			}
		}
	}
	if (pFilterPolygon != nullptr)
	{
		delete pFilterPolygon;
		pFilterPolygon = nullptr;
	}

	QPointF segment1[2];
	QPointF segment2[2];
	for (int i = 0; i < pRings->size(); i++)
	{
		int numPoints = pRings->at(i)->size();
		int nextj;
		for (int j = 0; j < numPoints; j++)
		{
			nextj = (j + 1) % numPoints;
			segment1[0].setX(pRings->at(i)->at(j).x());
			segment1[0].setY(pRings->at(i)->at(j).y());
			segment1[1].setX(pRings->at(i)->at(nextj).x());
			segment1[1].setY(pRings->at(i)->at(nextj).y());
			for (int m = 0; m < m_vecRings.size(); m++)
			{
				int numPartPoint = m_vecRings[m]->size();
				int nextn;
				for (int n = 0; n < numPartPoint; n++)
				{
					nextn = (n + 1) % numPartPoint;
					segment2[0].setX(m_vecRings[m]->at(n).x());
					segment2[0].setY(m_vecRings[m]->at(n).y());
					segment2[1].setX(m_vecRings[m]->at(nextn).x());
					segment2[1].setY(m_vecRings[m]->at(nextn).y());
					if (isIntersectLinesegment(segment1, segment2))
						return true;
				}
			}
		}
	}
	return false;
}
//视口与M_COVR判断空间关系
bool CSpatialPolygon::isIntersectPolygon(CPolylineF* pRing)
{
	// 如果折线上任一顶点在多边形内，说明线与多边形相关
	double dMinX = pRing->at(0).x();
	double dMinY = pRing->at(0).y();
	double dMaxX = pRing->at(0).x();
	double dMaxY = pRing->at(0).y();
	for (int i = 0; i < pRing->size(); i++)
	{
		double dx = pRing->at(i).x();
		double dy = pRing->at(i).y();
		if (ptInPolygon(dx, dy))
			return true;

		dMinX = qMin(dx, dMinX);
		dMinY = qMin(dy, dMinY);
		dMaxX = qMax(dx, dMaxX);
		dMaxY = qMax(dy, dMaxY);
	}

	if (dMinX < m_dMinX && dMinY < m_dMinY && dMaxX > m_dMaxX && dMaxY > m_dMaxY)
		return true;	//当前筛选多边形完全在线环内部

	QPointF segment1[2];
	QPointF segment2[2];

	if (dMinX > m_dMaxX || dMinY > m_dMaxY || dMaxX < m_dMinX || dMaxY < m_dMinY)
		return false;	//线的最小外接矩形与多边形的最小外接矩形不相交

	for (int i = 0; i < pRing->size() - 1; i++)
	{
		segment1[0].setX(pRing->at(i).x());
		segment1[0].setY(pRing->at(i).y());
		segment1[1].setX(pRing->at(i + 1).x());
		segment1[1].setY(pRing->at(i + 1).y());
		for (int m = 0; m < m_vecRings.size(); m++)
		{
			for (int n = 0; n < m_vecRings[m]->size() - 1; n++)
			{
				segment2[0].setX(m_vecRings[m]->at(n).x());
				segment2[0].setY(m_vecRings[m]->at(n).y());
				segment2[1].setX(m_vecRings[m]->at(n + 1).x());
				segment2[1].setY(m_vecRings[m]->at(n + 1).y());
				if (isIntersectLinesegment(segment1, segment2))
					return true;
			}
		}
	}
	return false;
}
//多边形线段与线段是否相交判断
bool CSpatialPolygon::isIntersectLinesegment(QPointF linesegment1[2], QPointF linesegment2[2])
{
	double x11, y11, x12, y12, x21, y21, x22, y22;
	x11 = linesegment1[0].x();
	y11 = linesegment1[0].y();
	x12 = linesegment1[1].x();
	y12 = linesegment1[1].y();

	x21 = linesegment2[0].x();
	y21 = linesegment2[0].y();
	x22 = linesegment2[1].x();
	y22 = linesegment2[1].y();

	if (x11 == x12 && y11 == y12 || x21 == x22 && y21 == y22)
		return false;
	if (x11 == x21 && y11 == y21 || x12 == x21 && y12 == y21)
		return true;
	if (x11 == x22 && y11 == y22 || x12 == x22 && y12 == y22)
		return true;


	if (qMin(x11, x12) > qMax(x21, x22) || qMin(y11, y12) > qMax(y21, y22) || 
		qMax(x11, x12) < qMin(x21, x22) || qMax(y11, y12) < qMin(y21, y22))
		return false;
	//	两线段整体平移（注意是整体），让线段1的A端点与原点（0, 0）重合，
	x12 -= x11;
	y12 -= y11;
	x21 -= x11;
	y21 -= y11;
	x22 -= x11;
	y22 -= y11;

	//  Discover the length of segment A-B.
	double distAB = sqrt(x12*x12 + y12 * y12);

	//  (2) Rotate the system so that point B is on the positive X axis.

	double theCos = x12 / distAB;
	double theSin = y12 / distAB;
	double newX = x21 * theCos + y21 * theSin;
	y21 = (float)(y21 * theCos - x21 * theSin);
	x21 = (float)newX;
	newX = x22 * theCos + y22 * theSin;
	y22 = (float)(y22 * theCos - x22 * theSin);
	x22 = (float)newX;

	//  Fail if segment 2 doesn't cross line 1.
	if (y21 < 0 && y22 < 0 || y21 >= 0 && y22 >= 0)
		return false;

	//  (3) Discover the position of the intersection point along line A-B.
	double posAB = x22 + (x21 - x22) * y22 / (y22 - y21);

	//  Fail if segment C-D crosses line A-B outside of segment A-B.
	if (posAB < 0 || posAB > distAB)
		return false;

	//  (4) Apply the discovered position to line A-B in the original coordinate system.
	return true;
}
//清空处理
void CSpatialPolygon::relPolygon()
{
	for (int i = 0; i < m_vecRings.size(); i++)
	{
		if (m_vecRings[i] != nullptr)
			delete m_vecRings[i];
		m_vecRings[i] = nullptr;
	}
	m_vecRings.clear();

	m_dMaxX = 0.0;
	m_dMaxY = 0.0;
	m_dMinX = 0.0;
	m_dMinY = 0.0;
}
//转换成GPC Polygon内存未释放,外部释放
GpcPolygon* CSpatialPolygon::toGpcPolygon()
{
	if (m_vecRings.size() == 0)
		return nullptr;
	GpcPolygon* pGpcPolygon = new GpcPolygon();
	CPolylineF polygon;
	CPolylineF* ring = m_vecRings[0];
	for (int i = 0; i < ring->size(); i++)
	{
		QPointF ptf = ring->at(i);
		polygon.push_back(ptf);
	}
	pGpcPolygon->createPolygon(polygon);//目前只转换第一个环


	return pGpcPolygon;
}

CS57PolygonClip::CS57PolygonClip()
	: m_bCoorSysID(true)
{}

CS57PolygonClip::~CS57PolygonClip()
{}

void CS57PolygonClip::setRect(QRectF rectIn)
{
	m_ClipRect = rectIn;
}

void CS57PolygonClip::setCoorSysId(bool b)
{
	m_bCoorSysID = b;
}

bool CS57PolygonClip::qujiao(float& x, float& y, Line l1, Line l2)
{
	//取交点
	float m = l1.A * l2.B - l1.B * l2.A;
	if (m == 0)
		return false;
	else 
	{
		x = (l2.C * l1.B - l1.C * l2.B) / m;
		y = (l1.C * l2.A - l2.C * l1.A) / m;
		return true;
	}
	return true;
}

bool CS57PolygonClip::Inside(QPointF p, int i)
{
	if (i == 0) 
	{//左边界
		if (p.x() >= m_ClipRect.left())
			return 1; 
	}
	else if (i == 1) 
	{//右边界
		if (p.x() <= m_ClipRect.right())
			return 1;
	}
	else if (i == 2) 
	{//上边界
		if (m_bCoorSysID)
		{
			if (p.y() >= m_ClipRect.top())
				return 1;
		}
		else
		{
			if (p.y() <= m_ClipRect.top())
				return 1;
		}
	}
	else if (i == 3) 
	{//下边界
		if (m_bCoorSysID)
		{
			if (p.y() <= m_ClipRect.bottom())
				return 1;
		}
		else
		{
			if (p.y() >= m_ClipRect.bottom())
				return 1;
		}
	}
	return 0;
}

void CS57PolygonClip::Intersect(QPointF S, QPointF P, int i, QPointF &ans)
{
	if (i < 2) 
	{//垂直裁剪边
		if (i == 0)
			ans.setX(m_ClipRect.left());
		else
			ans.setX(m_ClipRect.right());
		ans.setY(S.y() + (ans.x() - S.x()) * (P.y() - S.y()) / (P.x() - S.x()));
	}
	else 
	{//水平裁剪边
		if (i == 2)
			ans.setY(m_ClipRect.top());
		else
			ans.setY(m_ClipRect.bottom());
		ans.setX(S.x() + (ans.y() - S.y()) * (P.x() - S.x()) / (P.y() - S.y()));
	}
}

void CS57PolygonClip::Sutherland_Hodgman(QVector<QPointF> &cv)
{
	/*遍历矩形四个边界，每次都更新原来的点集*/
	QVector<QPointF>tmp;
	QPointF ip;

	for (int i = 0; i < 4; i++)
	{
		if (cv.size() == 0) return;
		QPointF S = cv[cv.size() - 1], P;
		for (int j = 0; j < cv.size(); j++)
		{
			P = cv[j];
			if (Inside(P, i))
			{
				if (Inside(S, i))
				{
					tmp.push_back(P);
				}
				else
				{
					Intersect(S, P, i, ip);
					tmp.push_back(ip);
					tmp.push_back(P);
				}
			}
			else if (Inside(S, i))
			{
				Intersect(S, P, i, ip);
				tmp.push_back(ip);
			}
			S = P;
		}
		cv.swap(tmp);
		tmp.clear();
	}
}

void CS57PolygonClip::PolypolylineClip(QVector<QVector<QPointF>> &cv)
{}

