#pragma once

#include "cs57control.h"

class CMapCanvas : public CS57Control
{
	Q_OBJECT

public:
	CMapCanvas(QWidget *parent = nullptr);
	~CMapCanvas();
	
signals:
	void updateLoadCellProgress(int cur, int total);

};
