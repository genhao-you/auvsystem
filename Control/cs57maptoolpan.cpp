#include "cs57maptoolpan.h"

CS57MapToolPan::CS57MapToolPan(CS57Control *parent)
	: CS57MapTool(parent)
	, m_bMousePress(false)
{}

CS57MapToolPan::~CS57MapToolPan()
{}

//************************************
// Method:    canvasPressEvent
// Brief:	  画布按压事件
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QMouseEvent * event
//************************************
void CS57MapToolPan::canvasPressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_bMousePress = true;
		m_MouseStartPt = event->pos();
		setCursor(Qt::ClosedHandCursor);
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
void CS57MapToolPan::canvasMoveEvent(QMouseEvent *event)
{
	if (m_bMousePress)
	{
		QPoint pt(0, 0);
		m_MouseEndPt = event->pos();
		pt += m_MouseEndPt - m_MouseStartPt;

		QSize lastSize = m_pS57Control->viewport()->size();
		m_pS57Control->setSceneRect(-pt.x(), -pt.y(), lastSize.width(), lastSize.height());
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
void CS57MapToolPan::canvasReleaseEvent(QMouseEvent *event)
{
	if (m_bMousePress)
	{
		setCursor(Qt::OpenHandCursor);

		m_bMousePress = false;
		m_MouseEndPt = event->pos();
		int offsetX = m_MouseEndPt.x() - m_MouseStartPt.x();
		int offsetY = m_MouseEndPt.y() - m_MouseStartPt.y();
		if (offsetX == 0 && offsetY == 0)
			return;

		m_pS57Control->destoryImage();
		m_pS57Control->offsetViewport(offsetX, offsetY);

		QSize lastSize = m_pS57Control->viewport()->size();
		m_pS57Control->updateViewport(QRectF(0, 0, lastSize.width(), lastSize.height()));
		m_pS57Control->setSceneRect(0, 0, lastSize.width(), lastSize.height());

		m_pS57Control->createCanvas();
		m_pS57Control->renderCanvas();
	}
}

//************************************
// Method:    canvasDoubleClickEvent
// Brief:	  画布双击事件
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QMouseEvent * event
//************************************
void CS57MapToolPan::canvasDoubleClickEvent(QMouseEvent *event)
{
	Q_UNUSED(event);
}
