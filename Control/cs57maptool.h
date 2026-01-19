#pragma once

#include <QObject>
#include <QMouseEvent>
#include <QCursor>
#include "cs57control.h"

class CS57MapTool : public QObject
{
	Q_OBJECT

public:
	CS57MapTool(CS57Control *parent);
	~CS57MapTool();

	virtual void canvasMoveEvent(QMouseEvent *e);

	virtual void canvasDoubleClickEvent(QMouseEvent *e);

	virtual void canvasPressEvent(QMouseEvent *e);

	virtual void canvasReleaseEvent(QMouseEvent *e);

	virtual void wheelEvent(QWheelEvent *e);

	void setCursor(QCursor cursor);

	CS57Control* m_pS57Control;
	QCursor		 m_Cursor;
};
