#pragma once

#include "cs57maptool.h"

class CS57MapToolPan : public CS57MapTool
{
	Q_OBJECT

public:
	CS57MapToolPan(CS57Control *parent);
	~CS57MapToolPan();

	void canvasPressEvent(QMouseEvent *event) override;
	void canvasMoveEvent(QMouseEvent *event) override;
	void canvasReleaseEvent(QMouseEvent *event) override;
	void canvasDoubleClickEvent(QMouseEvent *event) override;

private:
	bool					m_bMousePress;
	QPoint					m_MouseEndPt;
	QPoint					m_MouseStartPt;
};

