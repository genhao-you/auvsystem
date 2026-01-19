#include "cs57rubberband.h"
#include "crectangle.h"
#include <QPainter>


CS57RubberBand::CS57RubberBand(CS57Control* control, XT::GeometryType geometryType)
	: QObject(nullptr)
	, CS57CanvasItem(control)
	, m_GeometryType(geometryType)
{
	QColor color(Qt::blue);
	color.setAlpha(63);
	setColor(color);
	setWidth(3);
	setLineStyle(Qt::SolidLine);
	setBrushStyle(Qt::SolidPattern);
	setSecondaryStrokeColor(QColor());
}

CS57RubberBand::~CS57RubberBand()
{}

//************************************
// Method:    setColor
// Brief:	  设置橡皮条颜色
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: const QColor & color
//************************************
void CS57RubberBand::setColor(const QColor &color)
{
	setStrokeColor(color);
	setFillColor(color);
}

//************************************
// Method:    setFillColor
// Brief:	  设置橡皮条填充颜色
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: const QColor & color
//************************************
void CS57RubberBand::setFillColor(const QColor &color)
{
	if (m_Brush.color() == color)
		return;

	m_Brush.setColor(color);
}
void CS57RubberBand::setToCanvasRectangle(QRect rect)
{
	//确定新的矩形
	CRectangle<int> rectangle(rect.left(), rect.top(), rect.right(), rect.bottom());
	int x = rectangle.xMinimum();
	int y = rectangle.yMinimum();
	int w = rectangle.xMaximum() - rectangle.xMinimum();
	int h = rectangle.yMaximum() - rectangle.yMinimum();

	m_Rect = QRect(x,y,w,h);
	setRect(m_Rect);
}
void CS57RubberBand::setStrokeColor(const QColor &color)
{
	m_Pen.setColor(color);
}

void CS57RubberBand::setSecondaryStrokeColor(const QColor &color)
{
	m_SecondaryPen.setColor(color);
}

void CS57RubberBand::setWidth(int width)
{
	m_Pen.setWidth(width);
}
void CS57RubberBand::setLineStyle(Qt::PenStyle penStyle)
{
	m_Pen.setStyle(penStyle);
}

void CS57RubberBand::setBrushStyle(Qt::BrushStyle brushStyle)
{
	m_Brush.setStyle(brushStyle);
}
void CS57RubberBand::paint(QPainter *p)
{

	switch (m_GeometryType)
	{
	case XT::PointGeometry:
		break;
	case XT::LineGeometry:
	{
		if (m_vecPoint.size() == 0)
			return;

		p->setPen(m_Pen);

		p->drawPolyline(m_vecPoint);
		break;
	}
	case XT::PolygonGeometry:
	{
		int iterations = m_SecondaryPen.color().isValid() ? 2 : 1;
		for (int i = 0; i < iterations; ++i)
		{
			if (i == 0 && iterations > 1)
			{
				m_SecondaryPen.setWidth(m_Pen.width() + 2);
				p->setBrush(Qt::NoBrush);
				p->setPen(m_SecondaryPen);
			}
			else
			{
				p->setBrush(m_Brush);
				p->setPen(m_Pen);
			}
		}
		p->drawPolygon(m_Rect);
		break;
	}
	case XT::UnknownGeometry:
		break;
	case XT::NullGeometry:
		break;
	default:
		break;
	}
}

void CS57RubberBand::addPoint(QPoint pt)
{
	m_vecPoint.push_back(pt);
}

void CS57RubberBand::movePoint(QPoint pt)
{
	m_vecPoint.last() = pt;
}

void CS57RubberBand::calcBoundingBox()
{
	int xMin = std::numeric_limits<int>::max();
	int yMin = std::numeric_limits<int>::max();
	int xMax = -std::numeric_limits<int>::max();
	int yMax = -std::numeric_limits<int>::max();
	for (int i = 0; i < m_vecPoint.size(); i++)
	{
		int y = m_vecPoint[i].y();
		int x = m_vecPoint[i].x();
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