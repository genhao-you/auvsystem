#pragma once
#include "cs57projection.h"
#include "cs57viewport.h"
#include <QPoint>

//坐标转换类
class CS57Transform
{
public:
	static CS57Transform* instance()
	{
		if (m_pInstance == nullptr)
			m_pInstance = new CS57Transform();
		return m_pInstance;
	}
	~CS57Transform();

	//像素转经纬度
	QPointF pixel2Geo(QPoint pt);
	QPointF pixel2Geo(int x, int y);

	//像素转弧度
	QPointF pixel2Bl(QPoint pt);
	QPointF pixel2Bl(int x, int y);

	//像素转投影
	QPointF pixel2Vp(QPoint pt);
	QPointF pixel2Vp(int x, int y);

	//投影转经纬度
	QPointF vp2Geo(QPointF pt);
	QPointF vp2Geo(double x, double y);

	//弧度转投影
	QPointF bl2Vp(QPointF pt);
	QPointF bl2Vp(double x, double y);

	//投影转像素
	QPoint  vp2Pixel(QPointF pt);
	QPoint  vp2Pixel(double x, double y);

	//像素转度分秒(慎用)
	//注意：只能用于显示，不可用于计算
	QPointF pixel2DMS(QPoint pt);
	QPointF pixel2DMS(int x, int y);

	//度分秒转经纬度
	QPointF dms2Geo(QPointF pt);
	QPointF dms2Geo(double x, double y);

	//经纬度转度分秒(慎用)
	//注意：只能用于显示，不可用于计算
	QPointF geo2DMS(QPointF pt);
	QPointF geo2DMS(double x, double y);

	//经纬度转像素
	QPoint geo2Pixel(QPointF pt);
	QPoint geo2Pixel(double x, double y);

	//经纬度到投影
	QPointF geo2Vp(QPointF pt);
	QPointF geo2Vp(double x, double y);

	//************************************
	// Method:    dmsSplit
	// Brief:	  小数类型度分秒拆分成单个整数度、整数分、浮点型秒（例如：38°48′24.005″---> 38.4824005(此处即为该类型)--->38 48 24.005）
	// Returns:   void
	// Author:    cl
	// DateTime:  2021/12/02
	// Parameter: QPointF pt （38°48′24.005″---> 38.4824005(此处即为该类型分、秒不能超过60)）
	// Parameter: QString & strLD	
	// Parameter: QString & strLM
	// Parameter: QString & strLS
	// Parameter: QString & strBD	输出38
	// Parameter: QString & strBM	输出48
	// Parameter: QString & strBS	输出24.005
	//************************************
	void dmsSplit(QPointF pt, QString& strLD, QString& strLM, QString& strLS,QString& strBD,QString& strBM,QString& strBS);

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
	void dms2Geo(int dd, int mm, float ss, double& val);

	CS57Projection	m_Proj;
	CS57Viewport	m_Viewport;
private:
	CS57Transform();

	class Garbo
	{
	public:
		~Garbo();
	};

private:
	static CS57Transform* m_pInstance;
};

