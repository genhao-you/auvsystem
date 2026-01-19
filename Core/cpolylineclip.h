#pragma once
#include <QVector>

class CPolylineClip
{
public:
	CPolylineClip();
	~CPolylineClip();
	int Sgn(double a);
	/// <summary>
	/// 取两个点的中点
	/// </summary>
	/// <param name="p1"></param>
	/// <param name="p2"></param>
	/// <returns></returns>
	QPointF GetCenter(QPointF p1, QPointF p2);
	/// <summary>
	/// 对一线段与多边形的交点集合，按从p1到p2的方向进行排序
	/// </summary>
	/// <param name="p1"></param>
	/// <param name="p2"></param>
	/// <param name="interPoints"></param>
	/// <returns></returns>
	 QVector<QPointF> SortPointsBySlopeOfLine(QPointF p1, QPointF p2, QVector<QPointF> interPoints);
	 /// <summary>
	 /// 判断多边形是否包含某个点
	 /// </summary>
	 /// <param name="poly">多边形边框上每个角的点坐标数组</param>
	 /// <param name="p">要进行判断的点</param>
	 /// <returns>true:包含; false:不包含</returns>
	 bool InPoly(QVector<QPointF> polygon, QPointF p);
	/// <summary>
	/// 判断一个数值是否在误差范围内
	/// </summary>
	/// <param name="x"></param>
	/// <returns></returns>
	bool zero(double x);
	/// <summary>
	/// 计算交叉乘积(P1-P0)x(P2-P0)
	/// </summary>
	/// <param name="p1"></param>
	/// <param name="p2"></param>
	/// <param name="p0"></param>
	/// <returns></returns>
	double xmult(QPointF p1, QPointF p2, QPointF p0);
	/// <summary>
	/// 判点是否在线段上,包括端点
	/// </summary>
	/// <param name="p"></param>
	/// <param name="l1"></param>
	/// <param name="l2"></param>
	/// <returns></returns>
	bool dot_online_in(QPointF p, QPointF l1, QPointF l2);
	/// <summary>
	/// 判两点在线段同侧,点在线段上返回0
	/// </summary>
	/// <param name="p1"></param>
	/// <param name="p2"></param>
	/// <param name="l1"></param>
	/// <param name="l2"></param>
	/// <returns></returns>
	bool same_side(QPointF p1, QPointF p2, QPointF l1, QPointF l2);
	/// <summary>
	/// 判三点共线
	/// </summary>
	/// <param name="p1"></param>
	/// <param name="p2"></param>
	/// <param name="p3"></param>
	/// <returns></returns>
	bool dots_inline(QPointF p1, QPointF p2, QPointF p3);
	/// <summary>
	/// 判断两直线平行
	/// </summary>
	/// <param name="u1"></param>
	/// <param name="u2"></param>
	/// <param name="v1"></param>
	/// <param name="v2"></param>
	/// <returns></returns>
	bool parallel(QPointF u1, QPointF u2, QPointF v1, QPointF v2);
	/// <summary>
	/// 判两线段相交,包括端点和部分重合
	/// </summary>
	/// <param name="u1"></param>
	/// <param name="u2"></param>
	/// <param name="v1"></param>
	/// <param name="v2"></param>
	/// <returns></returns>
	bool intersect_in(QPointF u1, QPointF u2, QPointF v1, QPointF v2);
	/// <summary>
	/// 计算两线段交点,请判线段是否相交(同时还是要判断是否平行!)
	/// </summary>
	/// <param name="u1"></param>
	/// <param name="u2"></param>
	/// <param name="v1"></param>
	/// <param name="v2"></param>
	/// <param name="ret"></param>
	/// <returns></returns>
	int GetIntersectionPoint(QPointF u1, QPointF u2, QPointF v1, QPointF v2, QPointF& ret);
	/// <summary>
	 /// 求出线段和多边形的交点,不包括p1p2
	 /// </summary>
	 /// <param name="p1"></param>
	 /// <param name="p2"></param>
	 /// <param name="polygon"></param>
	 /// <returns></returns>
	QVector<QPointF> GetInterPoints(QPointF p1, QPointF p2, QVector<QPointF> polygon);
	/// <summary>
	/// 获取多边形裁剪折线形成的线段集合（多边形必须首尾闭合）,hole true/内环  false/外环
	/// </summary>
	/// <param name="polyline"></param>
	/// <param name="polygon"></param>
	/// <param name="hole"></param>
	/// <returns></returns>
	QVector<QVector<QPointF>> GetInterPolylines(QVector<QPointF> polyline, QVector<QPointF> polygon,bool hole);

	//X升序
	static bool pointOrderAscendingX(QPointF pt1, QPointF pt2);
	//X降序
	static bool pointOrderDescendingX(QPointF pt1, QPointF pt2);

	//Y升序
	static bool pointOrderAscendingY(QPointF pt1, QPointF pt2);
	//Y降序
	static bool pointOrderDescendingY(QPointF pt1, QPointF pt2);
private:
	double eps;
};
