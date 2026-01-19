#include "cs57route.h"
#include "cs57transform.h"
#include "polylinebuffer.h"
#include "dpi.h"
#include <QDebug>
#include <QMessageBox>


#pragma execution_character_set("utf-8")
CS57Route::CS57Route(CS57Control* control,const QString& name)
	: CS57Polyline(control,name)
	, m_pResultGpcPolygon(NULL)
	, m_bEnableBuffer(false)
{}

CS57Route::~CS57Route()
{
	if (m_pResultGpcPolygon != NULL)
		delete m_pResultGpcPolygon;
	m_vecGeoPolyline.clear();
}

//************************************
// Method:    copy
// Brief:	  拷贝航线
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CS57Route * route
//************************************
void CS57Route::copy(CS57Route* route)
{
	m_Pen = route->m_Pen;
	m_dBufferDisM = route->m_dBufferDisM;
	m_nBufferDisPix = route->m_nBufferDisPix;
	m_bEnableBuffer = route->m_bEnableBuffer;

	for (int i = 0; i < route->m_vecGeoPolyline.size(); i++)
	{
		m_vecGeoPolyline.push_back(route->m_vecGeoPolyline[i]);
	}
	for (int i = 0; i < route->m_vecBuffer.size(); i++)
	{
		m_vecBuffer.push_back(route->m_vecBuffer[i]);
	}
	for (int i = 0; i < route->m_vecBufferPix.size(); i++)
	{
		m_vecBufferPix.push_back(route->m_vecBufferPix[i]);
	}
}

//************************************
// Method:    setBufferDis
// Brief:	  设置缓冲距离
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: double dis
//************************************
void CS57Route::setBufferDis(double dis)
{
	m_dBufferDisM = dis;
}
//************************************
// Method:    updateBuffer
// Brief:	  更新缓冲区
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57Route::updateBuffer()
{
	qreal dpi = Dpi::getCurrentDpi();
	qreal scale = CS57Transform::instance()->m_Viewport.getDisplayScale();
	m_nBufferDisPix = m_dBufferDisM / scale * 1000. / 25.4 * dpi;
	makeBuffer();
}

//************************************
// Method:    makeBuffer
// Brief:	  制作缓冲区
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57Route::makeBuffer()
{
	if (m_dBufferDisM == 0 || m_vecPixelPolyline.size() < 2)
	{
		//缓冲距离为0，不缓冲
		return;
	}
	m_vecBufferPix.clear();
	makeGpcPolygon();
	QString strCoords = toString(m_vecPixelPolyline);
	QString strBuffer = PolylineBuffer::GetBufferEdgeCoords(strCoords, m_nBufferDisPix);
	m_vecBufferPix = toPoints(strBuffer);
}

//************************************
// Method:    makeGpcPolygon
// Brief:	  制作Gpc多边形
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57Route::makeGpcPolygon()
{
	for (int i = 0; i < m_vecPixelPolyline.size() - 1; i++)
	{
		CPolyline line;
		QPoint ptS = m_vecPixelPolyline[i];
		QPoint ptE = m_vecPixelPolyline[i+1];
		line.push_back(ptS);
		line.push_back(ptE);
		QString strLine = toString(line);
		QString strBuffer = PolylineBuffer::GetBufferEdgeCoords(strLine, m_nBufferDisPix);
		CPolylineF vecPts;
		vecPts = toPointFs(strBuffer);
		GpcPolygon* gpc = new GpcPolygon();
		gpc->createPolygon(vecPts);// ::createPolygon(vecPts);
		m_vecGpcPolygon.push_back(gpc);
	}
	if (m_vecGpcPolygon.size() == 0)
		return;

	//释放上一次的
	if (m_pResultGpcPolygon != NULL)
	{
		delete m_pResultGpcPolygon;
		m_pResultGpcPolygon = NULL;
	}

	m_pResultGpcPolygon = new GpcPolygon(*m_vecGpcPolygon[0]);
	for (int i = 1; i < m_vecGpcPolygon.size(); i++)
	{
		GpcPolygon * tmpResultGpcPolygon = new GpcPolygon();
		m_pResultGpcPolygon->polygon_clip(GPC_UNION, m_vecGpcPolygon[i], tmpResultGpcPolygon);
		delete m_pResultGpcPolygon;
		m_pResultGpcPolygon = NULL;
		m_pResultGpcPolygon = new GpcPolygon(*tmpResultGpcPolygon);
		delete tmpResultGpcPolygon;
		tmpResultGpcPolygon= NULL;
	}
	for (int i = 0; i < m_vecGpcPolygon.size(); i++)
	{
		delete m_vecGpcPolygon[i];
		m_vecGpcPolygon[i] = NULL;
	}
	m_vecGpcPolygon.clear();
}

CPolylineF CS57Route::toPointFs(QString coords)
{
	CPolylineF vecPtFs;
	QStringList coordinates = coords.split(";");
	for (int i = 0; i < coordinates.size(); i++)
	{
		QStringList values = coordinates[i].split(",");
		if (values.size() == 2)
		{
			double x = values[0].toDouble();
			double y = values[1].toDouble();
			QPointF ptf = CS57Transform::instance()->pixel2Geo((int)x, (int)y);
			vecPtFs.push_back(ptf);
		}
	}

	return vecPtFs;
}
CPolyline CS57Route::toPoints(QString coords)
{
	CPolyline vecPts;
	QStringList coordinates = coords.split(";");
	for (int i = 0; i < coordinates.size(); i++)
	{
		QStringList values = coordinates[i].split(",");
		if (values.size() == 2)
		{
			QPoint pt;
			double x = values[0].toDouble();
			double y = values[1].toDouble();
			pt.setX((int)x);
			pt.setY((int)y);
			vecPts.push_back(pt);
		}
	}

	return vecPts;
}


//************************************
// Method:    calcBoundingBox
// Brief:	  计算航线包围框
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57Route::calcBoundingBox()
{
	if (m_vecGeoPolyline.size() < 2)
		return;

	CS57Polyline::calcBoundingBox();

	if (m_bEnableBuffer)
	{
		updateBuffer();
		int xMin = (std::numeric_limits<int>::max)();
		int yMin = (std::numeric_limits<int>::max)();
		int xMax = -(std::numeric_limits<int>::max)();
		int yMax = -(std::numeric_limits<int>::max)();
		for (int i = 0; i < m_vecBufferPix.size(); i++)
		{
			int y = m_vecBufferPix[i].y();
			int x = m_vecBufferPix[i].x();
			xMin = (std::min)(xMin, x);
			yMin = (std::min)(yMin, y);
			xMax = (std::max)(xMax, x);
			yMax = (std::max)(yMax, y);
		}
		int x = xMin;
		int y = yMin;
		int w = xMax - xMin;
		int h = yMax - yMin;

		setRect(QRect(x, y, w, h));
	}
}

//************************************
// Method:    paint
// Brief:	  绘制
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QPainter * p
//************************************
void CS57Route::paint(QPainter *p)
{
	if (m_vecPixelPolyline.size() < 2)
		return;

	CS57Polyline::paint(p);

	if (m_bEnableBuffer)
	{
		QPen pen(QColor(170, 5, 234, 255), 1);
		p->setPen(pen);
		renderBufferPolygon(p, m_pResultGpcPolygon);
	}
}
//************************************
// Method:    renderBufferPolygon
// Brief:	  渲染缓冲区多边形
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QPainter * painter
// Parameter: GpcPolygon * polygon
//************************************
void CS57Route::renderBufferPolygon(QPainter* painter, GpcPolygon* polygon)
{
	QPainterPath path;
	int num_contours = polygon->m_gpcPolygon.num_contours;
	int *hole = polygon->m_gpcPolygon.hole;
	for (int i = 0; i < num_contours; i++)
	{
		QPolygon pol;
		int num_vertices = polygon->m_gpcPolygon.contour[i].num_vertices;
		for (int j = 0; j < num_vertices; j++)
		{
			QPoint pt;
			pt = CS57Transform::instance()->geo2Pixel(polygon->m_gpcPolygon.contour[i].vertex[j].x,
				polygon->m_gpcPolygon.contour[i].vertex[j].y);
			//QMessageBox::information(nullptr, "", QString::number(pt.x()) + "," + QString::number(pt.y()));
			pol.append(pt);
		}
		path.addPolygon(pol);
		painter->drawPolygon(pol);
	}
	painter->fillPath(path, QColor(255, 0, 0, 50));
}

//************************************
// Method:    buffer2Polygon
// Brief:	  缓冲区转多边形
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QVector<QVector<QPointF> * > * vecPtfs
//************************************
void CS57Route::buffer2Polygon(QVector<QVector<QPointF>*>* vecPtfs)
{
	if (m_pResultGpcPolygon == nullptr)
		return;

	m_pResultGpcPolygon->toPolygonF(vecPtfs);
}

//************************************
// Method:    setBufferEnabled
// Brief:	  设置缓冲区可用性
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: bool enable
//************************************
void CS57Route::setBufferEnabled(bool enable)
{
	m_bEnableBuffer = enable;
}

//************************************
// Method:    ptCount
// Brief:	  航线点总数
// Returns:   int
// Author:    cl
// DateTime:  2022/07/21
//************************************
int CS57Route::ptCount() const
{
	return m_vecGeoPolyline.size();
}
