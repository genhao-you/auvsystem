#include "stdafx.h"
#include "cs57viewport.h"

CS57Viewport::CS57Viewport()
	: m_bLockDisplayScale(false)
{
	m_dAngle = 0.0;
	m_dOffSetX = 0.0;
	m_dOffSetY = 0.0;
	m_dDisplayScale = 1.0;
}

CS57Viewport::CS57Viewport(int xdpi, int ydpi, QRectF& rect_in)
	: m_dOffSetX(0.0)
	, m_dOffSetY(0.0)
	, m_bLockDisplayScale(false)
{ 
	m_dXdpi = xdpi;
	m_dYdpi = ydpi;
	setViewPortRect(rect_in);
}

CS57Viewport::~CS57Viewport()
{}


void CS57Viewport::setCenterPtXY(double x, double y)				// 设置窗口中心点对应的用户数据坐标（矢量坐标）
{
	m_dCenterPt[0] = x;
	m_dCenterPt[1] = y;
}

//************************************
// Method:    lockDisplayScale
// Brief:	  锁定比例尺
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: bool lock
//************************************
void CS57Viewport::lockDisplayScale(bool lock)
{
	m_bLockDisplayScale = lock;
}

//************************************
// Method:    setViewPort
// Brief:	  设置视口
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: double xdpi
// Parameter: double ydpi
// Parameter: QRectF rect_in
//************************************
void CS57Viewport::setViewPort(double xdpi, double ydpi, QRectF rect_in)
{
	m_dXdpi = xdpi;
	m_dYdpi = ydpi;
	setViewPortRect(rect_in);
}

//************************************
// Method:    setDisplayScale
// Brief:	  设置显示比例尺
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: double scale
//************************************
void  CS57Viewport::setDisplayScale(double scale)
{
	if (m_bLockDisplayScale)
		return;

	m_dDisplayScale = scale;
}

void CS57Viewport::setAngle(double angle)
{
	m_dAngle = angle;
}

//************************************
// Method:    setViewPortRect
// Brief:	  设置视口矩形
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QRectF rect_in
//************************************
void CS57Viewport::setViewPortRect(QRectF rect_in)					// 设置视口范围，单位为设备逻辑单位
{
	m_rcViewPort.setBottom(rect_in.bottom());
	m_rcViewPort.setLeft(rect_in.left());
	m_rcViewPort.setRight(rect_in.right());
	m_rcViewPort.setTop(rect_in.top());
	m_nCenterPt[0] = (rect_in.right() - rect_in.left()) / 2;
	m_nCenterPt[1] = (rect_in.bottom() - rect_in.top()) / 2;
}

void CS57Viewport::setViewPortOrg(int x, int y)						// 设置视口原点的设备坐标
{
	m_nCenterPt[0] = x;
	m_nCenterPt[1] = y;
}

//************************************
// Method:    setViewPortOffset
// Brief:	  设置视口偏移
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: double offsetx
// Parameter: double offsety
//************************************
void CS57Viewport::setViewPortOffset(double offsetx, double offsety)
{
	m_dOffSetX = offsetx;
	m_dOffSetY = offsety;
}

//************************************
// Method:    getBufferRange
// Brief:	  获取视口四至范围坐标
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QPointF pt[4]
//************************************
void CS57Viewport::getBufferRange(QPointF pt[4])
{
	// TODO: 在此处添加实现代码.
	double dx, dy;
	convertDp2Vp(m_rcViewPort.left(), m_rcViewPort.bottom(), &dx, &dy);			//左下角
	pt[0].setX(dx);
	pt[0].setY(dy);

	convertDp2Vp(m_rcViewPort.right(), m_rcViewPort.bottom(), &dx, &dy);			//右下角
	pt[1].setX(dx);
	pt[1].setY(dy);

	convertDp2Vp(m_rcViewPort.right(), m_rcViewPort.top(), &dx, &dy);				//右上角
	pt[2].setX(dx);
	pt[2].setY(dy);

	convertDp2Vp(m_rcViewPort.left(), m_rcViewPort.top(), &dx, &dy);				//左上角
	pt[3].setX(dx);
	pt[3].setY(dy);
}

//************************************
// Method:    getCenterPointXY
// Brief:	  获取视口中心点
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: double & x
// Parameter: double & y
//************************************
void CS57Viewport::getCenterPointXY(double& x, double& y)
{
	x = m_dCenterPt[0];
	y = m_dCenterPt[1];
}

//************************************
// Method:    getViewPortOffset
// Brief:	  获取视口偏移
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: double * offsetx
// Parameter: double * offsety
//************************************
void CS57Viewport::getViewPortOffset(double* offsetx, double* offsety)
{
	*offsetx = m_dOffSetX;
	*offsety = m_dOffSetY;
}

//************************************
// Method:    getDisplayScale
// Brief:	  获取显示比例尺
// Returns:   double
// Author:    cl
// DateTime:  2022/07/21
//************************************
double CS57Viewport::getDisplayScale() const
{
	return m_dDisplayScale;
}

//************************************
// Method:    getRect
// Brief:	  获取视口矩形
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QRectF * rect
//************************************
void CS57Viewport::getRect(QRectF* rect)
{
	*rect = m_rcViewPort;
}

void CS57Viewport::getAngle(double* angle)
{
	*angle = m_dAngle;
}

void CS57Viewport::convertHimetric2Dp(double x, double y, int* xout, int* yout)		// 输入坐标单位为0.01mm,输出为设备坐标
{
	*xout = round(x * m_dXdpi / 2540.);
	*yout = round(y * m_dYdpi / 2540.);
}

void CS57Viewport::convertMm2Dp(double x, double y, int* xout, int* yout)			// 输入坐标单位为mm,输出为设备坐标
{
	*xout = round(x * m_dXdpi / 25.40);
	*yout = round(y * m_dYdpi / 25.40);
}

void CS57Viewport::convertDp2Himetric(int x, int y, double* xout, double* yout)		// 输入设备坐标,输出坐标单位为0.01mm
{
	*xout = (double)x * 2540. / (double)m_dXdpi;
	*yout = (double)y * 2540. / (double)m_dYdpi;
}

void CS57Viewport::convertDp2Mm(int x, int y, double* xout, double* yout)			// 输入设备坐标,输出坐标单位为mm
{
	*xout = (double)x * 25.40 / (double)m_dXdpi;
	*yout = (double)y * 25.40 / (double)m_dYdpi;
}


void CS57Viewport::convertVp2Dp(double x_in, double y_in, int* xout, int* yout)
{
	double x1, y1, x2, y2;
	x1 = x_in - m_dCenterPt[0] + m_dOffSetY;					// 平移
	y1 = y_in - m_dCenterPt[1] + m_dOffSetY;
	x2 = x1 * cos(m_dAngle) - y1 * sin(m_dAngle);				// 旋转
	y2 = x1 * sin(m_dAngle) + y1 * cos(m_dAngle);
	x2 = x2 * 1000. / m_dDisplayScale;
	y2 = y2 * 1000. / m_dDisplayScale;
	convertMm2Dp(x2, y2, xout, yout);

	*xout += m_nCenterPt[0];
	*yout = m_nCenterPt[1] - *yout;
}

void CS57Viewport::convertDp2Vp(int x_in, int y_in, double* xout, double* yout)
{
	double x1, y1, x2, y2;
	int nx = x_in - m_nCenterPt[0];
	int ny = m_nCenterPt[1] - y_in;
	convertDp2Mm(nx, ny,&x1,&y1);

	x2 = x1 * cos(m_dAngle) + y1 * sin(m_dAngle);	// 旋转	x2 = x1 * cos(-m_fAngle) - y1 * sin(-m_fAngle);
	y2 = y1 * cos(m_dAngle) - x1 * sin(m_dAngle);	//		y2 = x1 * sin(-m_fAngle) + y1 * cos(-m_fAngle);

	*xout = x2 * m_dDisplayScale / 1000. + m_dCenterPt[0] - m_dOffSetY;
	*yout = y2 * m_dDisplayScale / 1000. + m_dCenterPt[1] - m_dOffSetY;
}