#include "cs57polyline.h"
#include "cs57transform.h"


CS57Polyline::CS57Polyline(CS57Control* control,const QString& name)
	: CS57Geometry(control,name)
	, m_pTransform(CS57Transform::instance())
{
	QColor color(Qt::blue);
	color.setAlpha(100);
	setPenWidth(2);
	setColor(color);
	setLineStyle(Qt::SolidLine);
}

CS57Polyline::~CS57Polyline()
{
	m_vecGeoPolyline.clear();
	m_vecPixelPolyline.clear();
}

//************************************
// Method:    setColor
// Brief:	  设置线眼色
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: const QColor & color
//************************************
void CS57Polyline::setColor(const QColor &color)
{
	m_Pen.setColor(color);
}

//************************************
// Method:    setPenWidth
// Brief:	  设置画笔宽度
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: int w
//************************************
void CS57Polyline::setPenWidth(int w)
{
	m_Pen.setWidth(w);
}

//************************************
// Method:    setLineStyle
// Brief:	  设置线样式
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: Qt::PenStyle penStyle
//************************************
void CS57Polyline::setLineStyle(Qt::PenStyle penStyle)
{
	m_Pen.setStyle(penStyle);
}

//************************************
// Method:    addPoint
// Brief:	  添加坐标点
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QPointF pt
//************************************
void CS57Polyline::addPoint(QPointF pt)
{
	m_vecGeoPolyline.push_back(pt);
}

//************************************
// Method:    addPoint
// Brief:	  添加坐标点
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: double lon
// Parameter: double lat
//************************************
void CS57Polyline::addPoint(double lon, double lat)
{
	QPointF ptf(lon, lat);
	m_vecGeoPolyline.push_back(ptf);
}

//************************************
// Method:    modityPt
// Brief:	  修改点
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: int idx
// Parameter: QPointF pt
//************************************
void CS57Polyline::modityPt(int idx, QPointF pt)
{
	for (int i = 0; i < m_vecGeoPolyline.size(); i++)
	{
		if (i == idx)
		{
			m_vecGeoPolyline[i] = pt;
		}
	}
}

//************************************
// Method:    removePt
// Brief:	  移除点
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: int idx
//************************************
void CS57Polyline::removePt(int idx)
{
	int n = 0;
	CPolylineF::iterator it;
	for (it = m_vecGeoPolyline.begin(); it != m_vecGeoPolyline.end(); it++)
	{
		if (idx == n)
		{
			it = m_vecGeoPolyline.erase(it);
			if (it == m_vecGeoPolyline.end())
				break;
			break;
		}
		n++;
	}
}
//************************************
// Method:    clearPt
// Brief:	  清理点
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57Polyline::clearPt()
{
	m_vecGeoPolyline.clear();
}


//************************************
// Method:    calcBoundingBox
// Brief:	  计算线包围框
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57Polyline::calcBoundingBox()
{
	if (m_vecGeoPolyline.size() < 2)
		return;

	m_vecPixelPolyline.clear();
	for (int i = 0; i < m_vecGeoPolyline.size(); i++)
	{
		QPoint pt = m_pTransform->geo2Pixel(m_vecGeoPolyline[i]);
		m_vecPixelPolyline.push_back(pt);
	}

	int xMin = std::numeric_limits<int>::max();
	int yMin = std::numeric_limits<int>::max();
	int xMax = -std::numeric_limits<int>::max();
	int yMax = -std::numeric_limits<int>::max();
	for (int i = 0; i < m_vecPixelPolyline.size(); i++)
	{
		int y = m_vecPixelPolyline[i].y();
		int x = m_vecPixelPolyline[i].x();
		xMin = std::min(xMin, x);
		yMin = std::min(yMin, y);
		xMax = std::max(xMax, x);
		yMax = std::max(yMax, y);
	}
	int x = xMin;
	int y = yMin;
	int w = xMax - xMin;
	int h = yMax - yMin;

	setRect(QRect(x, y, w, h));
}

//************************************
// Method:    paint
// Brief:	  绘制
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QPainter * p
//************************************
void CS57Polyline::paint(QPainter *p)
{
	if (m_vecPixelPolyline.size() < 2)
		return;

	p->setPen(m_Pen);
	p->drawPolyline(m_vecPixelPolyline);
}

QString CS57Polyline::toString(CPolyline& vecPts)
{
	QString strCoodinate = "";
	if (vecPts.size() < 2)
		return strCoodinate;

	for (int i = 0; i < vecPts.size(); i++)
	{
		if(i != vecPts.size() - 1)
			strCoodinate.append(QString::number(vecPts[i].x()) + "," + QString::number(vecPts[i].y()) + ";");
		else
			strCoodinate.append(QString::number(vecPts[i].x()) + "," + QString::number(vecPts[i].y()));
	}

	return strCoodinate;
}

//************************************
// Method:    addPts
// Brief:	  添加线
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CPolylineF pts
//************************************
void CS57Polyline::addPts(CPolylineF pts)
{
	m_vecGeoPolyline.clear();
	for (int i = 0; i < pts.size(); i++)
	{
		m_vecGeoPolyline.push_back(pts[i]);
	}
}

