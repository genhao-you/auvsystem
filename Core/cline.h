#pragma once
#include <qmath.h>
template<class T>
class  CLine
{
public:
	CLine(T x1, T y1, T x2, T y2)
		: _x1(x1)
		, _y1(y1)
		, _x2(x2)
		, _y2(y2)
	{}
	~CLine(){}

	T length()
	{
		return sqrt((_x1 - _x2)*(_x1 - _x2) + (_y1 - _y2)*(_y1 - _y2));
	}
	T angle()
	{
		return atan2((_y2 - _y1), (_x2 - _x1));
	}

	T _x1,_y1,_x2,_y2;
};


struct Line 
{
	float A, B, C;
	float X1, Y1, X2, Y2;
	float  x, y, dx, ym;     //(x,y):第一个接触到扫描线的点，y和ym可以判断是否为横直线，dx：1/k，ym:最大y  
	Line() {}
	Line(float x1, float y1, float x2, float y2) :
		X1(x1), Y1(y1), X2(x2), Y2(y2) 
	{
		A = y2 - y1;
		B = x1 - x2;
		C = x2 * y1 - x1 * y2;
		if (y1 == y2) 
		{      //横直线dx不能按正常算
			this->y = y1;
			this->ym = y1;
			if (x1 < x2) 
			{ 
				dx = x1;
				x = x2;
			}
			else 
			{ 
				dx = x2; x = x1; 
			}
		}
		else if (y2 < y1) 
		{
			this->x = x2;                     //记录上方的x值一方便处理关键时刻（用于插入AET排序）
			this->y = y2;                    //记录上方的y值，用于排序
			this->ym = y1;                  //靠下者ym
			dx = (x2 - x1) / (y2 - y1);
		}
		else 
		{
			this->x = x1;
			this->y = y1;
			this->ym = y2;
			dx = (x2 - x1) / (y2 - y1);
		}
	}
	void init(float x1, float y1, float x2, float y2) 
	{
		X1 = x1, Y1 = y1, X2 = x2, Y2 = y2;
		A = y2 - y1;
		B = x1 - x2;
		C = x2 * y1 - x1 * y2;
	}
	bool inTheLine(float x, float y) 
	{
		float minx = std::min(X1, X2);
		float maxx = std::max(X1, X2);
		float miny = std::min(Y1, Y2);
		float maxy = std::max(Y1, Y2);
		if (minx <= x && x <= maxx && miny <= y && y <= maxy)
			return 1;
		else 
		{ 
			return 0; 
		}
	}
	bool onSide(float x, float y) 
	{
		//返回在直线的某一侧
		if (A * x + B * y + C > 0)
			return 1;
		else 
			return 0;
	}
};
