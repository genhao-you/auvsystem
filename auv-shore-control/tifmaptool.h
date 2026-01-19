#pragma once

#include <QObject>
#include <QMouseEvent>
#include"elevationcanvas.h"
class TifMapTool  : public QObject
{
    Q_OBJECT
public:
    TifMapTool(ElevationCanvas* canvas);
    virtual ~TifMapTool();

    // 事件处理函数
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void wheelEvent(QWheelEvent* event);
    void setCursor(QCursor cursor);

    ElevationCanvas* m_canvas;
    QCursor		 m_Cursor;
};
