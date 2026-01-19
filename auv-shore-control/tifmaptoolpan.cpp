#include "tifmaptoolpan.h"

TifMapToolPan::TifMapToolPan(ElevationCanvas* parent)
	:TifMapTool(parent)
{

}

TifMapToolPan::~TifMapToolPan()
{}

void TifMapToolPan::mousePressEvent(QMouseEvent * event)
{
    if (event->button() == Qt::LeftButton) {
        m_isPanning = true;
        m_startPos = event->pos();
        m_canvas->setCursor(Qt::ClosedHandCursor);
    }
}

void TifMapToolPan::mouseMoveEvent(QMouseEvent* event)
{
    if (m_isPanning) {
        // 计算平移距离
        int dx = event->x() - m_startPos.x();
        int dy = event->y() - m_startPos.y();

        // 平移视图
        m_canvas->translate(dx, dy);

        m_startPos = event->pos();
    }
}

void TifMapToolPan::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_isPanning = false;
        m_canvas->setCursor(Qt::OpenHandCursor);
    }
}
