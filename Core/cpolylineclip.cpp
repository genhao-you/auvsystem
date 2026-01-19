#include "cpolylineclip.h"
#include <qmath.h>
#define EPSILON 0.000001

CPolylineClip::CPolylineClip()
	: eps(1e-8)
{}

CPolylineClip::~CPolylineClip()
{}

int CPolylineClip::Sgn(double a)
{
	if (a == 0)
	{
		return 0;
	}
	else if (a < 0)
	{
		return -1;
	}
	else
	{
		return 1;
	}
}

QPointF CPolylineClip::GetCenter(QPointF p1, QPointF p2)
{
	return QPointF((p1.x() + p2.x()) / 2, (p1.y() + p2.y()) / 2);
}

bool CPolylineClip::zero(double x)
{
	return (((x) > 0 ? (x) : -(x)) < eps);
}

double CPolylineClip::xmult(QPointF p1, QPointF p2, QPointF p0)
{
	return (p1.x() - p0.x()) * (p2.y() - p0.y()) - (p2.x() - p0.x()) * (p1.y() - p0.y());
}

bool CPolylineClip::dot_online_in(QPointF p, QPointF l1, QPointF l2)
{
	return zero(xmult(p, l1, l2)) && (l1.x() - p.x()) * (l2.x() - p.x()) < eps && (l1.y() - p.y()) * (l2.y() - p.y()) < eps;
}

bool CPolylineClip::same_side(QPointF p1, QPointF p2, QPointF l1, QPointF l2)
{
	return xmult(l1, p1, l2) * xmult(l1, p2, l2) > eps;
}

bool CPolylineClip::dots_inline(QPointF p1, QPointF p2, QPointF p3)
{
	return zero(xmult(p1, p2, p3));
}

bool CPolylineClip::parallel(QPointF u1, QPointF u2, QPointF v1, QPointF v2)
{
	return zero((u1.x() - u2.x()) * (v1.y() - v2.y()) - (v1.x() - v2.x()) * (u1.y() - u2.y()));
}

bool CPolylineClip::intersect_in(QPointF u1, QPointF u2, QPointF v1, QPointF v2)
{
	if (!dots_inline(u1, u2, v1) || !dots_inline(u1, u2, v2))
		return !same_side(u1, u2, v1, v2) && !same_side(v1, v2, u1, u2);

	return dot_online_in(u1, v1, v2) || dot_online_in(u2, v1, v2) || dot_online_in(v1, u1, u2) || dot_online_in(v2, u1, u2);
}

int CPolylineClip::GetIntersectionPoint(QPointF u1, QPointF u2, QPointF v1, QPointF v2, QPointF& ret)
{
	ret = u1;
	if (parallel(u1, u2, v1, v2) || !intersect_in(u1, u2, v1, v2))
	{
		return 0;
	}
	double t = ((u1.x() - v1.x()) * (v1.y() - v2.y()) - (u1.y() - v1.y()) * (v1.x() - v2.x()))
				/ ((u1.x() - u2.x()) * (v1.y() - v2.y()) - (u1.y() - u2.y()) * (v1.x() - v2.x()));
	ret.setX(ret.x() + (u2.x() - u1.x()) * t);
	ret.setY(ret.y() + (u2.y() - u1.y()) * t);
	return 1;
}

QVector<QPointF> CPolylineClip::GetInterPoints(QPointF p1, QPointF p2, QVector<QPointF> polygon)
{
	QVector<QPointF> interPoints;
	QVector<QPointF> vecPolygonPts;
	for (int i = 0; i < polygon.size(); i++)
	{
		vecPolygonPts.push_back(polygon[i]);
	}
	for (int i = 0; i < vecPolygonPts.size(); i++)
	{
		QPointF polygon1 = vecPolygonPts[i];
		QPointF polygon2;
		if (i == vecPolygonPts.size() - 1)
		{
			polygon2 = vecPolygonPts[0];
		}
		else
		{
			polygon2 = vecPolygonPts[i + 1];

		}
		QPointF inter;
		int interType = GetIntersectionPoint(p1, p2, polygon1, polygon2, inter);
		switch (interType)
		{
		case 1:
			if (inter != p1 && inter != p2)
			{
				interPoints.push_back(inter);
			}
			break;
		case 0:
		default:
			break;
		}
	}
	return interPoints;
}

//QVector<QVector<QPointF>> CPolylineClip::GetInterPolylines(QVector<QPointF> polyline, QVector<QPointF> polygon, bool hole)
//{
//	QVector<QVector<QPointF>> list;
//	//TODO： 1.遍历折线上的每相邻的两个个点，组成线段，与多边形的每一条边计算，求出此线段与多边形的边的交点
//	//TODO:  2.对此线段的上的交点进行排序，组成连续点的折线，判断这些线段在多边形内部的部分，加入集合
//	QVector<QPointF> vecPolylinePts, vecPolygonPts;
//	for (int i = 0; i < polyline.size(); i++)
//	{
//		vecPolylinePts.push_back(polyline[i]);
//	}
//	for (int i = 0; i < polygon.size(); i++)
//	{
//		vecPolygonPts.push_back(polygon[i]);
//	}
//
//	QVector<QPointF> interPolyline;
//
//
//
//	for (int i = 0; i < vecPolylinePts.size() - 1; i++)
//	{
//		QPointF one = vecPolylinePts[i];
//		QPointF two;
//		if (i != vecPolylinePts.size() - 1)
//		{
//			two = vecPolylinePts[i + 1];
//		}
//
//		QVector<QPointF> inters = GetInterPoints(one, two, polygon);
//		QVector<QPointF> sortInters = SortPointsBySlopeOfLine(one, two, inters);
//
//
//		bool in = false;
//
//		for (int j = 0; j < sortInters.size(); j++)
//		{
//			if (j < sortInters.size() - 1)
//			{
//				if (!hole)
//				{
//					if (InPoly(polygon, GetCenter(sortInters[j], sortInters[j + 1])))
//					{
//						if (!in)
//						{
//							if (interPolyline.size() >= 2)
//							{
//								list.push_back(interPolyline);
//							}
//							interPolyline.clear();
//							interPolyline.push_back(sortInters[j]);
//							interPolyline.push_back(sortInters[j + 1]);
//							in = true;
//						}
//						else
//							interPolyline.push_back(sortInters[j + 1]);
//					}
//					else
//						in = false;
//				}
//				else
//				{
//					if (!InPoly(polygon, GetCenter(sortInters[j], sortInters[j + 1])))
//					{
//						if (!in)
//						{
//							if (interPolyline.size() >= 2)
//							{
//								list.push_back(interPolyline);
//							}
//							interPolyline.clear();
//							interPolyline.push_back(sortInters[j]);
//							interPolyline.push_back(sortInters[j + 1]);
//							in = true;
//						}
//						else
//							interPolyline.push_back(sortInters[j + 1]);
//					}
//					else
//						in = false;
//
//				}
//			}
//
//		}
//
//	}
//	if (interPolyline.size() >= 2)
//		list.push_back(interPolyline);
//	interPolyline.clear();
//	return list;
//}

QVector<QVector<QPointF>> CPolylineClip::GetInterPolylines(QVector<QPointF> polyline, QVector<QPointF> polygon, bool hole)
{
	QVector<QVector<QPointF>> list;
	//TODO： 1.遍历折线上的每相邻的两个个点，组成线段，与多边形的每一条边计算，求出此线段与多边形的边的交点
	//TODO:  2.对此线段的上的交点进行排序，组成连续点的折线，判断这些线段在多边形内部的部分，加入集合
	QVector<QPointF> vecPolylinePts, vecPolygonPts;
	for (int i = 0; i < polyline.size(); i++)
	{
		vecPolylinePts.push_back(polyline[i]);
	}
	for (int i = 0; i < polygon.size(); i++)
	{
		vecPolygonPts.push_back(polygon[i]);
	}

	QVector<QPointF> interPolyline;

	QVector<QPointF> tmpPolylinePts;
	for (int i = 0; i < vecPolylinePts.size() - 1; i++)
	{
		QPointF one = vecPolylinePts[i];
		QPointF two;
		if (i != vecPolylinePts.size() - 1)
		{
			two = vecPolylinePts[i + 1];
		}
		QVector<QPointF> inters = GetInterPoints(one, two, polygon);
		QVector<QPointF> sortInters = SortPointsBySlopeOfLine(one, two, inters);
		if (i == 0)
			tmpPolylinePts.append(sortInters);
		else
		{
			for (int j = 1; j < sortInters.size(); j++)
				tmpPolylinePts.push_back(sortInters[j]);
		}
	}

	bool in = false;
	for (int i = 0; i < tmpPolylinePts.size() - 1; i++)
	{
		if (!hole)
		{
			if (InPoly(polygon, GetCenter(tmpPolylinePts[i], tmpPolylinePts[i + 1])))
			{
				if (!in)
				{
					if (interPolyline.size() >= 2)
					{
						list.push_back(interPolyline);
					}
					interPolyline.clear();
					interPolyline.push_back(tmpPolylinePts[i]);
					interPolyline.push_back(tmpPolylinePts[i + 1]);
					in = true;
				}
				else
					interPolyline.push_back(tmpPolylinePts[i + 1]);
			}
			else
			{
				if (interPolyline.size() >= 2)
				{
					list.push_back(interPolyline);
					interPolyline.clear();

				}
				in = false;
			}


		}
		else
		{
			if (!InPoly(polygon, GetCenter(tmpPolylinePts[i], tmpPolylinePts[i + 1])))
			{
				if (!in)
				{
					if (interPolyline.size() >= 2)
					{
						list.push_back(interPolyline);
					}
					interPolyline.clear();
					interPolyline.push_back(tmpPolylinePts[i]);
					interPolyline.push_back(tmpPolylinePts[i + 1]);
					in = true;
				}
				else
					interPolyline.push_back(tmpPolylinePts[i + 1]);
			}
			else
			{
				if (interPolyline.size() >= 2)
				{
					list.push_back(interPolyline);
					interPolyline.clear();
				}
				in = false;
			}

		}
	}
	if (interPolyline.size() >= 2)
		list.push_back(interPolyline);
	interPolyline.clear();
	return list;
}


bool CPolylineClip::pointOrderAscendingX(QPointF pt1, QPointF pt2)
{
	return pt1.x() <= pt2.x();
}

bool CPolylineClip::pointOrderDescendingX(QPointF pt1, QPointF pt2)
{
	return pt1.x() > pt2.x();
}

bool CPolylineClip::pointOrderAscendingY(QPointF pt1, QPointF pt2)
{
	return pt1.y() <= pt2.y();
}

bool CPolylineClip::pointOrderDescendingY(QPointF pt1, QPointF pt2)
{
	return pt1.y() > pt2.y();
}

QVector<QPointF> CPolylineClip::SortPointsBySlopeOfLine(QPointF p1, QPointF p2, QVector<QPointF> interPoints)
{
	QVector<QPointF> points;
	QVector<QPointF> newInterPoints;
	points.push_back(p1);
	if (p1.x() == p2.x())//垂直线段
	{
		if (p1.y() > p2.y())
		{
			qSort(interPoints.begin(), interPoints.end(), pointOrderDescendingY);
			newInterPoints = interPoints;
		}
		else
		{
			qSort(interPoints.begin(), interPoints.end(), pointOrderAscendingY);
			newInterPoints = interPoints;
		}
	}
	else
	{
		if (p1.y() == p2.y())//水平线段
		{
			if (p1.x() > p2.x())
			{
				qSort(interPoints.begin(), interPoints.end(), pointOrderDescendingX);		
				newInterPoints = interPoints;
			}
			else
			{
				qSort(interPoints.begin(), interPoints.end(), pointOrderAscendingX);
				newInterPoints = interPoints;
			}

		}
		else//普通斜率线段，按x或y都行
		{
			if (p1.x() > p2.x())
			{
				qSort(interPoints.begin(), interPoints.end(), pointOrderDescendingX);
				newInterPoints = interPoints;
			}
			else
			{
				qSort(interPoints.begin(), interPoints.end(), pointOrderAscendingX);
				newInterPoints = interPoints;
			}

		}

	}
	for (int i = 0; i < newInterPoints.size(); i++)
	{
		points.push_back(newInterPoints[i]);
	}
	points.push_back(p2);

	return points;
}

bool CPolylineClip::InPoly(QVector<QPointF> polygon, QPointF p)
{
	// 交点个数 
	int nCross = 0;
	int nCount;
	int nexti;
	nCount = polygon.size();
	QPointF p1, p2;
	for (int i = 0; i < nCount; i++)
	{
		nexti = (i + 1) % nCount;
		p1.setX(polygon[i].x());
		p1.setY(polygon[i].y());
		p2.setX(polygon[nexti].x());
		p2.setY(polygon[nexti].y());

		if (fabs(p1.y() - p2.y()) < EPSILON)
			continue;
		if (p.y() <= std::min(p1.y(), p2.y()))
			continue;
		if (p.y() >= std::max(p1.y(), p2.y()))
			continue;
		// 求交点的x坐标（由直线两点式方程转化而来）   

		double x = (p.y() - p1.y()) * (p2.x() - p1.x()) / (p2.y() - p1.y()) + p1.x();

		// 只统计p1p2与p向右射线的交点  
		if (x > p.x())
		{
			nCross++;
		}
	}

	// 交点为偶数，点在多边形之外  交点为奇数，点在多边形之内
	if ((nCross % 2) == 1)
		return true;
	else
		return false;
}
