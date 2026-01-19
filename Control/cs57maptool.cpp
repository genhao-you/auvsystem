#include "cs57maptool.h"


CS57MapTool::CS57MapTool(CS57Control *parent)
	: QObject(parent)
	, m_pS57Control(parent)
	, m_Cursor(Qt::CrossCursor)
{}

CS57MapTool::~CS57MapTool()
{}

//************************************
// Method:    canvasMoveEvent
// Brief:	  画布鼠标移动事件
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QMouseEvent * e
//************************************
void CS57MapTool::canvasMoveEvent(QMouseEvent *e)
{
	Q_UNUSED(e);
}

//************************************
// Method:    canvasDoubleClickEvent
// Brief:	  画布鼠标双击事件
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QMouseEvent * e
//************************************
void CS57MapTool::canvasDoubleClickEvent(QMouseEvent *e)
{
	Q_UNUSED(e);
}

//************************************
// Method:    canvasPressEvent
// Brief:	  画布鼠标按压事件
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QMouseEvent * e
//************************************
void CS57MapTool::canvasPressEvent(QMouseEvent *e)
{
	Q_UNUSED(e);
}

//************************************
// Method:    canvasReleaseEvent
// Brief:	  画布鼠标释放事件
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QMouseEvent * e
//************************************
void CS57MapTool::canvasReleaseEvent(QMouseEvent *e)
{
	Q_UNUSED(e);
}

//************************************
// Method:    wheelEvent
// Brief:	  鼠标滚轮事件
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QWheelEvent * e
//************************************
void CS57MapTool::wheelEvent(QWheelEvent *e)
{
	e->ignore();
}

//************************************
// Method:    setCursor
// Brief:	  设置鼠标指针样式
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QCursor cursor
//************************************
void CS57MapTool::setCursor(QCursor cursor)
{
	m_pS57Control->setCursor(cursor);
}
