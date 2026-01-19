#pragma once

#include <QObject>
#include"tifmaptool.h"
class TifMapToolPan  : public TifMapTool
{
	Q_OBJECT

public:
	TifMapToolPan(ElevationCanvas* parent);
	~TifMapToolPan();
	 void mousePressEvent(QMouseEvent* event) override;
	 void mouseMoveEvent(QMouseEvent* event)override;
	 void mouseReleaseEvent(QMouseEvent* event)override;

private:
	bool m_isPanning;
	QPoint m_startPos;
};
