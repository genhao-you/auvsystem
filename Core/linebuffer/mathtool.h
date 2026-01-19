#pragma once
#include "coordinate.h"

class MathTool
{
public:
	MathTool();
	~MathTool();

	///
	/// 获取由两个点所形成的向量的象限角度
	///
	/// 第一个点的坐标
	/// 第二个点的坐标
	///
	static double GetQuadrantAngle(Coordinate preCoord, Coordinate nextCoord);
	///
	/// 由增量X和增量Y所形成的向量的象限角度
	///
	/// 增量X
	/// 增量Y
	/// 象限角
	static double GetQuadrantAngle(double x, double y);
	///
	/// 获取由相邻的三个点所形成的两个向量之间的夹角
	///
	///
	///
	///
	///
	static double GetIncludedAngle(Coordinate preCoord, Coordinate midCoord, Coordinate nextCoord);
	///
	/// 获取由两个点所形成的向量的模(长度)
	///
	/// 第一个点
	/// 第二个点
	/// 由两个点所形成的向量的模(长度)
	double GetDistance(Coordinate preCoord, Coordinate nextCoord);

};
