#include "tifmaptool.h"


TifMapTool::TifMapTool(ElevationCanvas* parent)
	: QObject(parent)
	,m_canvas(parent)
{

}

TifMapTool::~TifMapTool()
{

}

void TifMapTool::mousePressEvent(QMouseEvent * event)
{
	Q_UNUSED(event);
}

void TifMapTool::mouseMoveEvent(QMouseEvent* event)
{
	Q_UNUSED(event);
}

void TifMapTool::mouseReleaseEvent(QMouseEvent* event)
{
	Q_UNUSED(event);
}

void TifMapTool::wheelEvent(QWheelEvent* event)
{
	event->ignore();
}

void TifMapTool::setCursor(QCursor cursor)
{
	m_canvas->setCursor(cursor);
}
