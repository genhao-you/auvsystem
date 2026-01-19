#pragma once

#include "cs57maptool.h"

class CS57MapToolQuery : public CS57MapTool
{
	Q_OBJECT

public:
	CS57MapToolQuery(CS57Control *parent);
	~CS57MapToolQuery();

	void canvasPressEvent(QMouseEvent *event) override;
	void canvasMoveEvent(QMouseEvent *event) override;
	void canvasReleaseEvent(QMouseEvent *event) override;
	void canvasDoubleClickEvent(QMouseEvent *event) override;

signals:
	void updateQueryData(QMultiMap<QString, QVector<QString>>* data);
private:
	bool					m_bMousePress;
	QPoint					m_MouseEndPt;
	QPoint					m_MouseStartPt;
};

