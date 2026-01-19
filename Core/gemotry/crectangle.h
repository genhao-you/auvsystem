#pragma once
/************************************************************************/
/* 矩形类        只处理像素坐标  不能用于笛卡尔坐标                     */
/************************************************************************/
template<class T>
class CRectangle
{
public:
	CRectangle() {}
	explicit CRectangle(T xMin, T yMin, T xMax, T yMax)
		: _Xmin(xMin)
		, _Ymin(yMin)
		, _Xmax(xMax)
		, _Ymax(yMax)
	{
		normalize();
	}
	~CRectangle() {}

	void set(T xMin, T yMin, T xMax, T yMax)
	{
		_Xmin = xMin;
		_Ymin = yMin;
		_Xmax = xMax;
		_Ymax = yMax;
		normalize();
	}

	double xMinimum() const { return _Xmin; }
	double yMinimum() const { return _Ymin; }
	double xMaximum() const { return _Xmax; }
	double yMaximum() const { return _Ymax; }
	double area() const { return (_Xmax - _Xmin) * (_Ymax - _Ymin); }
private:
	void normalize()
	{
		if (_Xmin > _Xmax)
		{
			std::swap(_Xmin, _Xmax);
		}
		if (_Ymin > _Ymax)
		{
			std::swap(_Ymin, _Ymax);
		}
	}
private:
	T _Xmin;
	T _Ymin;
	T _Xmax;
	T _Ymax;
};
