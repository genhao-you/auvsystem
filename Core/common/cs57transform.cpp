#include "stdafx.h"
#include "cs57transform.h"

CS57Transform* CS57Transform::m_pInstance = nullptr;
CS57Transform::CS57Transform()
{
	m_pInstance = this;
	static Garbo garbo;
}

CS57Transform::~CS57Transform()
{}

//************************************
// Method:    pixel2Geo
// Brief:	  像素转地理坐标
// Returns:   QPointF
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QPoint pt
//************************************
QPointF CS57Transform::pixel2Geo(QPoint pt)
{
	double vpx = 0.0, vpy = 0.0;
	double lon = 0.0, lat = 0.0;
	m_Viewport.convertDp2Vp(pt.x(), pt.y(), &vpx, &vpy);
	m_Proj.getBL(vpy, vpx, &lat, &lon);
	lon *= RO;
	lat *= RO;

	return QPointF(lon, lat);
}

//************************************
// Method:    pixel2Geo
// Brief:	  像素转地理坐标
// Returns:   QPointF
// Author:    cl
// DateTime:  2022/07/21
// Parameter: int x
// Parameter: int y
//************************************
QPointF CS57Transform::pixel2Geo(int x, int y)
{
	double vpx = 0.0, vpy = 0.0;
	double lon = 0.0, lat = 0.0;
	m_Viewport.convertDp2Vp(x, y, &vpx, &vpy);
	m_Proj.getBL(vpy, vpx, &lat, &lon);
	lon *= RO;
	lat *= RO;

	return QPointF(lon, lat);
}

//************************************
// Method:    pixel2Bl
// Brief:	  像素转弧度坐标
// Returns:   QPointF
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QPoint pt
//************************************
QPointF CS57Transform::pixel2Bl(QPoint pt)
{
	double vpx = 0.0, vpy = 0.0;
	double lon = 0.0, lat = 0.0;
	m_Viewport.convertDp2Vp(pt.x(), pt.y(), &vpx, &vpy);
	m_Proj.getBL(vpy, vpx, &lat, &lon);

	return QPointF(lon, lat);
}

//************************************
// Method:    pixel2Bl
// Brief:	  像素转弧度坐标
// Returns:   QPointF
// Author:    cl
// DateTime:  2022/07/21
// Parameter: int x
// Parameter: int y
//************************************
QPointF CS57Transform::pixel2Bl(int x, int y)
{
	double vpx = 0.0, vpy = 0.0;
	double lon = 0.0, lat = 0.0;
	m_Viewport.convertDp2Vp(x, y, &vpx, &vpy);
	m_Proj.getBL(vpy, vpx, &lat, &lon);

	return QPointF(lon, lat);
}

//************************************
// Method:    pixel2Vp
// Brief:	  像素转投影坐标
// Returns:   QPointF
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QPoint pt
//************************************
QPointF CS57Transform::pixel2Vp(QPoint pt)
{
	double vpx = 0.0, vpy = 0.0;
	m_Viewport.convertDp2Vp(pt.x(), pt.y(), &vpx, &vpy);

	return QPointF(vpx, vpy);
}

//************************************
// Method:    pixel2Vp
// Brief:	  像素转投影坐标
// Returns:   QPointF
// Author:    cl
// DateTime:  2022/07/21
// Parameter: int x
// Parameter: int y
//************************************
QPointF CS57Transform::pixel2Vp(int x, int y)
{
	double vpx = 0.0, vpy = 0.0;
	m_Viewport.convertDp2Vp(x, y, &vpx, &vpy);

	return QPointF(vpx, vpy);
}

//************************************
// Method:    vp2Geo
// Brief:	  投影坐标转地理坐标
// Returns:   QPointF
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QPointF pt
//************************************
QPointF CS57Transform::vp2Geo(QPointF pt)
{
	double vpx = pt.x();
	double vpy = pt.y();
	double lon = 0.0, lat = 0.0;
	m_Proj.getBL(vpy, vpx, &lat, &lon);
	lon *= RO;
	lat *= RO;

	return QPointF(lon, lat);
}

//************************************
// Method:    vp2Geo
// Brief:	  投影坐标转地理坐标
// Returns:   QPointF
// Author:    cl
// DateTime:  2022/07/21
// Parameter: double x
// Parameter: double y
//************************************
QPointF CS57Transform::vp2Geo(double x, double y)
{
	double vpx = x;
	double vpy = y;
	double lon = 0.0, lat = 0.0;
	m_Proj.getBL(vpy, vpx, &lat, &lon);
	lon *= RO;
	lat *= RO;

	return QPointF(lon, lat);
}
//************************************
// Method:    bl2Vp
// Brief:	  弧度坐标转投影坐标
// Returns:   QPointF
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QPointF pt
//************************************
QPointF CS57Transform::bl2Vp(QPointF pt)
{
	double vpx = 0.0, vpy = 0.0;
	m_Proj.getXY(pt.y(), pt.x(), &vpy, &vpx);

	return QPointF(vpx, vpy);
}

//************************************
// Method:    bl2Vp
// Brief:	  弧度坐标转投影坐标
// Returns:   QPointF
// Author:    cl
// DateTime:  2022/07/21
// Parameter: double x
// Parameter: double y
//************************************
QPointF CS57Transform::bl2Vp(double x, double y)
{
	double vpx = 0.0, vpy = 0.0;
	m_Proj.getXY(y, x, &vpy, &vpx);

	return QPointF(vpx, vpy);
}

//************************************
// Method:    vp2Pixel
// Brief:	  投影坐标转像素坐标
// Returns:   QPoint
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QPointF pt
//************************************
QPoint CS57Transform::vp2Pixel(QPointF pt)
{
	int dpx, dpy;
	m_Viewport.convertVp2Dp(pt.x(), pt.y(), &dpx, &dpy);

	return QPoint(dpx, dpy);
}

//************************************
// Method:    vp2Pixel
// Brief:	  投影坐标转像素坐标
// Returns:   QPoint
// Author:    cl
// DateTime:  2022/07/21
// Parameter: double x
// Parameter: double y
//************************************
QPoint CS57Transform::vp2Pixel(double x, double y)
{
	int dpx, dpy;
	m_Viewport.convertVp2Dp(x, y, &dpx, &dpy);

	return QPoint(dpx, dpy);
}
//像素转度分秒(慎用)
//注意：只能用于显示，不可用于计算
QPointF CS57Transform::pixel2DMS(QPoint pt)
{
	double dmsLon = 0.0, dmsLat = 0.0;
	QPointF radPt = pixel2Bl(pt);

	dmsLon = fun_radTodmmss(radPt.x());
	dmsLat = fun_radTodmmss(radPt.y());

	return QPointF(dmsLon, dmsLat);
}
//像素转度分秒(慎用)
//注意：只能用于显示，不可用于计算
QPointF CS57Transform::pixel2DMS(int x, int y)
{
	double dmsLon = 0.0, dmsLat = 0.0;
	QPointF radPt = pixel2Bl(x,y);

	dmsLon = fun_radTodmmss(radPt.x());
	dmsLat = fun_radTodmmss(radPt.y());

	return QPointF(dmsLon, dmsLat);
}

//************************************
// Method:    dms2Geo
// Brief:	  度分秒转地理坐标
// Returns:   QPointF
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QPointF pt
//************************************
QPointF CS57Transform::dms2Geo(QPointF pt)
{
	double lon = 0.0, lat = 0.0;

	double radLon = fun_dmmssTorad(pt.x());
	double radLat = fun_dmmssTorad(pt.y());

	lon = radLon * RO;
	lat = radLat * RO;

	return QPointF(lon, lat);
}

//************************************
// Method:    dms2Geo
// Brief:	  度分秒转地理坐标
// Returns:   QPointF
// Author:    cl
// DateTime:  2022/07/21
// Parameter: double x
// Parameter: double y
//************************************
QPointF CS57Transform::dms2Geo(double x, double y)
{
	double lon = 0.0, lat = 0.0;

	double radLon = fun_dmmssTorad(x);
	double radLat = fun_dmmssTorad(y);

	lon = radLon * RO;
	lat = radLat * RO;

	return QPointF(lon, lat);
}
//************************************
// Method:    dms2Geo
// Brief:	  将拆分的度分秒转换为十进制格式（38°48′24.005″--->38 48 24.005---->???)
// Returns:   void
// Author:    cl
// DateTime:  2021/12/02
// Parameter: int dd	38
// Parameter: int mm	48
// Parameter: float ss	24.005
// Parameter: double & val	输出十进制结果
//************************************
void CS57Transform::dms2Geo(int dd, int mm, float ss, double& val)
{
	val = fabs(dd)+ fabs(mm / 60.) + fabs(ss / 3600.);
}

//************************************
// Method:    geo2DMS
// Brief:	  地理坐标转度分秒
// Returns:   QPointF
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QPointF pt
//************************************
QPointF CS57Transform::geo2DMS(QPointF pt)
{
	double lon = 0.0, lat = 0.0;

	pt.setX(pt.x() / RO);
	pt.setY(pt.y() / RO);

	lon = fun_radTodmmss(pt.x());
	lat = fun_radTodmmss(pt.y());

	return QPointF(lon, lat);
}

//************************************
// Method:    geo2DMS
// Brief:	  地理坐标转度分秒
// Returns:   QPointF
// Author:    cl
// DateTime:  2022/07/21
// Parameter: double x
// Parameter: double y
//************************************
QPointF CS57Transform::geo2DMS(double x, double y)
{
	double lon = 0.0, lat = 0.0;

	x /= RO;
	y /= RO;

	lon = fun_radTodmmss(x);
	lat = fun_radTodmmss(y);

	return QPointF(lon, lat);
}

//************************************
// Method:    geo2Pixel
// Brief:	  地理坐标转像素坐标
// Returns:   QPoint
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QPointF pt
//************************************
QPoint CS57Transform::geo2Pixel(QPointF pt)
{
	double radLon = 0.0, radLat = 0.0;
	radLon = pt.x() / RO;
	radLat = pt.y() / RO;

	QPointF vp = bl2Vp(radLon, radLat);
	QPoint pixel = vp2Pixel(vp);

	return pixel;
}

//************************************
// Method:    geo2Pixel
// Brief:	  地理坐标转像素坐标
// Returns:   QPoint
// Author:    cl
// DateTime:  2022/07/21
// Parameter: double x
// Parameter: double y
//************************************
QPoint CS57Transform::geo2Pixel(double x, double y)
{
	double radLon = 0.0, radLat = 0.0;
	radLon = x / RO;
	radLat = y / RO;

	QPointF vp = bl2Vp(radLon, radLat);
	QPoint pixel = vp2Pixel(vp);

	return pixel;
}

//************************************
// Method:    geo2Vp
// Brief:	  地理坐标转投影坐标
// Returns:   QPointF
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QPointF pt
//************************************
QPointF CS57Transform::geo2Vp(QPointF pt)
{
	double radlon = 0., radlat = 0., vpx = 0., vpy = 0.;
	radlon = pt.x() / RO;
	radlat = pt.y() / RO;
	m_Proj.getXY(radlat, radlon, &vpy, &vpx);

	return QPointF(vpx, vpy);
}

//************************************
// Method:    geo2Vp
// Brief:	  地理坐标转投影坐标
// Returns:   QPointF
// Author:    cl
// DateTime:  2022/07/21
// Parameter: double x
// Parameter: double y
//************************************
QPointF CS57Transform::geo2Vp(double x, double y)
{
	double radlon = 0., radlat = 0., vpx = 0., vpy = 0.;
	radlon = x / RO;
	radlat = y / RO;
	m_Proj.getXY(radlat, radlon, &vpy, &vpx);

	return QPointF(vpx, vpy);
}

//************************************
// Method:    dmsSplit
// Brief:	  度分秒坐标分割
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QPointF pt
// Parameter: QString & strLD
// Parameter: QString & strLM
// Parameter: QString & strLS
// Parameter: QString & strBD
// Parameter: QString & strBM
// Parameter: QString & strBS
//************************************
void CS57Transform::dmsSplit(QPointF pt, QString& strLD, QString& strLM, QString& strLS,QString& strBD, QString& strBM, QString& strBS)
{
	QString strLon = "";
	QString strLat = "";
	//纬度
	double b = pt.y();
	int bd = b;
	int bm = (b - bd) * 100;
	double bs = ((b - (double)bd) * (double)100 - (double)bm) * (double)100;

	//经度
	double l = pt.x();
	int ld = l;
	int lm = (l - ld) * 100;
	double ls = ((l - (double)ld) * (double)100 - (double)lm) * (double)100;

	strBD = QString::number(bd);
	strBM = QString::number(bm);
	strBS = QString::number(bs, 'f', 3);

	if (strBD.contains("-"))
	{
		if (strBM.contains("-"))
			strBM = strBM.replace("-", "");
		if (strBS.contains("-"))
			strBS = strBS.replace("-", "");
	}

	strLD = QString::number(ld);
	strLM = QString::number(lm);
	strLS = QString::number(ls, 'f', 3);
	if (strLD.contains("-"))
	{
		if (strLM.contains("-"))
			strLM = strLM.replace("-", "");
		if (strLS.contains("-"))
			strLS = strLS.replace("-", "");
	}
}

CS57Transform::Garbo::~Garbo()
{
	if (CS57Transform::m_pInstance != nullptr)
		delete m_pInstance;
	m_pInstance = nullptr;
}
