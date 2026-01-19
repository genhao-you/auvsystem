#include "cmapcanvas.h"

CMapCanvas::CMapCanvas(QWidget *parent)
	: CS57Control(parent)
{
	connect(this, &CS57Control::updateLoadCellProgress, this, &CMapCanvas::updateLoadCellProgress);
}

CMapCanvas::~CMapCanvas()
{}

