#pragma once

#include "cs57maptool.h"
#include "cs57rubberband.h"

class CS57Route;
class CS57MapToolSelect : public CS57MapTool
{
	Q_OBJECT

public:
	CS57MapToolSelect(CS57Control *parent);
	~CS57MapToolSelect();

	void canvasPressEvent(QMouseEvent *event) override;
	void canvasMoveEvent(QMouseEvent *event) override;
	void canvasReleaseEvent(QMouseEvent *event) override;
	void canvasDoubleClickEvent(QMouseEvent *event) override;

signals:
	void updateStartPos(XT::SelectionMode mode, QPointF sPos);
	void updateEndPos(XT::SelectionMode mode, QPointF ePos);
	void normalTool();

private:
	CS57Control*			m_pS57Control;
	CS57Route*				m_pRoute;
	QPoint					m_MouseStartPt;
	QPoint					m_MouseEndPt;
	QRect					m_Rect;
	bool					m_bDragging;
	CS57RubberBand*			m_pRubberBand;
	CS57Transform*			m_pTransform;
};

