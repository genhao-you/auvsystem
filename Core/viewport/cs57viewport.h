#pragma once
#include <math.h>
#include <QRect>

#define FLOATMAX 3.40E+38
#define FLOATMIN -3.40E+38
using namespace std;

class CS57Viewport
{
public:
	CS57Viewport();
	CS57Viewport(int xdpi, int ydpi, QRectF& rect_in);
	~CS57Viewport();

public:
	QRectF		m_rcViewPort;												// 视口对应的窗口矩形，单位为设备坐标单位（像素）
	double		m_dXdpi;													// dpi参与计算的地方很多，为保证精度，改为双精度浮点数
	double		m_dYdpi;
	double		m_dAngle;													// 图形旋转角度，弧度；逆时针旋转为正
	double		m_dCenterPt[3];												// 窗口中心点对应的地图投影坐标，单位为米
	int			m_nCenterPt[2];												// 窗口中心点对应的设备坐标
	double		m_dOffSetX;
	double		m_dOffSetY;
public:
	void		lockDisplayScale(bool lock);
	void		setViewPort(double xdpi, double ydpi, QRectF rect_in);
	void		setDisplayScale(double scale);								// 设置投影显示比例尺
	void		setAngle(double angle);										// 设置用户数据旋转角度
	void		setCenterPtXY(double x, double y);						// 设置窗口中心点对应的逻辑坐标（矢量坐标）
	void		setViewPortRect(QRectF rect_in);								// 设置视口范围，单位为对应的设备坐标单位
	void		setViewPortOrg(int x, int y);								// 设置视口原点，单位为对应的设备坐标单位
	void		setViewPortOffset(double offsetx, double offsety);
	void		getRect(QRectF* rect);
	void		getAngle(double* angle);
	void		getBufferRange(QPointF pt[4]);							// 获取映射成视口数据范围所对应的用户数据范围
	void		getCenterPointXY(double& x, double& y);
	void		getViewPortOffset(double* offsetx, double* offsety);
	void		convertVp2Dp(double x_in, double y_in, int* xout, int* yout);//参数vpx,vpy  dpx,dpy
	void		convertDp2Vp(int x_in, int y_in, double* xout, double* yout);

	void		convertHimetric2Dp(double x, double y, int* xout, int* yout);
	void		convertMm2Dp(double x, double y, int* xout, int* yout);
	void		convertDp2Himetric(int x, int y, double* xout, double*yout);
	void		convertDp2Mm(int x, int y, double* xout, double* yout);
	double		getDisplayScale()const;

private:
	bool		m_bLockDisplayScale;
	double		m_dDisplayScale;											// 显示比例尺
};

