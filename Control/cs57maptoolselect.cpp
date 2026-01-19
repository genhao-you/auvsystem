#include "cs57maptoolselect.h"
#include "cs57route.h"
#include "cs57routemanager.h"
#include <QDebug>


CS57MapToolSelect::CS57MapToolSelect(CS57Control *parent)
	: CS57MapTool(parent)
	, m_pRoute(nullptr)
	, m_pS57Control(parent)
	, m_pRubberBand(nullptr)
	, m_bDragging(false)
	, m_pTransform(CS57Transform::instance())
{}

CS57MapToolSelect::~CS57MapToolSelect()
{
	if (m_pRubberBand)
		delete m_pRubberBand;
	m_pRubberBand = nullptr;
}

//************************************
// Method:    canvasPressEvent
// Brief:	  画布鼠标按压事件
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QMouseEvent * event
//************************************
void CS57MapToolSelect::canvasPressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_Rect.setRect(0, 0, 0, 0);

		//获取鼠标坐标，并返回度分秒值
		QPointF sPos = CS57Transform::instance()->pixel2DMS(event->pos());

		switch (m_pS57Control->getSelectionMode())
		{
		case XT::P_SELECT:
			emit updateStartPos(XT::P_SELECT, sPos);
			break;
		case XT::INDETAIL_SELECT:
		case XT::SUMMARY_SELECT:
		case XT::INREGION_SELECT:
		case XT::INSET_SELECT:
		{
			if (!m_pRoute)
			{
				m_pRoute = new CS57Route(m_pS57Control,"line_select");
			}

			if (m_pRubberBand)
			{
				delete m_pRubberBand;
				m_pRubberBand = nullptr;
			}
			m_pRubberBand = new CS57RubberBand(m_pS57Control, XT::LineGeometry);
			m_pRubberBand->addPoint(event->pos());
			m_pRubberBand->addPoint(event->pos());
			m_pRoute->addPoint(m_pTransform->pixel2Geo(event->pos()));
			m_pRoute->calcBoundingBox();
			m_pRoute->show();

			emit updateStartPos(m_pS57Control->getSelectionMode(), sPos);
			break;
		}
		}
	}
	else if (event->button() == Qt::RightButton)
	{
		if (m_pRubberBand)
		{
			delete m_pRubberBand;
			m_pRubberBand = nullptr;
		}
		if (m_pRoute)
		{
			m_pS57Control->routeManager()->addRoute(CS57RouteManager::Temp,m_pRoute);
			m_pRoute = nullptr;
		}
		emit normalTool();
	}
}

//************************************
// Method:    canvasMoveEvent
// Brief:	  画布鼠标移动事件
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QMouseEvent * event
//************************************
void CS57MapToolSelect::canvasMoveEvent(QMouseEvent *event)
{
	switch (m_pS57Control->getSelectionMode())
	{
	case XT::P_SELECT:
		break;
	case XT::INDETAIL_SELECT:
	case XT::SUMMARY_SELECT:
	case XT::INREGION_SELECT:
	case XT::INSET_SELECT:
	{
		if (m_pRubberBand)
		{
			m_pRubberBand->movePoint(event->pos());
			if (m_pRubberBand)
			{
				m_pRubberBand->calcBoundingBox();
				m_pRubberBand->show();
			}
		}
		break;
	}
	case XT::A_SELECT:
	{
		if (!(event->buttons() & Qt::LeftButton))
			return;

		if (!m_bDragging)
		{
			m_bDragging = true;
			if (m_pRubberBand)
			{
				delete m_pRubberBand;
				m_pRubberBand = nullptr;
			}

			m_pRubberBand = new CS57RubberBand(m_pS57Control, XT::PolygonGeometry);
			QColor color(Qt::blue);
			color.setAlpha(63);
			m_pRubberBand->setColor(color);
			m_Rect.setTopLeft(event->pos());

			QPointF sPos = CS57Transform::instance()->pixel2DMS(event->pos());

			emit updateStartPos(XT::A_SELECT, sPos);
		}
		m_Rect.setBottomRight(event->pos());

		QPointF ePos = CS57Transform::instance()->pixel2DMS(event->pos());
		emit updateEndPos(XT::A_SELECT, ePos);

		if (m_pRubberBand)
		{
			m_pRubberBand->setToCanvasRectangle(m_Rect);
			//qDebug() << "Rect:" << m_Rect.x() << "," << m_Rect.y() << "," << m_Rect.width() << "," << m_Rect.height() << endl;
			m_pRubberBand->show();
		}
		break;
	}
	}
}

//************************************
// Method:    canvasReleaseEvent
// Brief:	  画布鼠标释放事件
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QMouseEvent * event
//************************************
void CS57MapToolSelect::canvasReleaseEvent(QMouseEvent *event)
{
	switch (m_pS57Control->getSelectionMode())
	{
	case XT::P_SELECT:
		break;
	case XT::INDETAIL_SELECT:
	case XT::SUMMARY_SELECT:
	case XT::INREGION_SELECT:
	case XT::INSET_SELECT:
		break;
	case XT::A_SELECT:
	{
		if (event->button() != Qt::LeftButton)
			return;

		m_bDragging = false;
		if (m_pRubberBand)
		{
			delete m_pRubberBand;
			m_pRubberBand = nullptr;
		}
		break;
	}
	}
}

//************************************
// Method:    canvasDoubleClickEvent
// Brief:	  画布鼠标双击事件
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QMouseEvent * event
//************************************
void CS57MapToolSelect::canvasDoubleClickEvent(QMouseEvent *event)
{
	Q_UNUSED(event);
}
