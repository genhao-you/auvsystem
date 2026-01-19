#include "stdafx.h"
#include "cs57canvas.h"
#include "cs57control.h"
#include <QPainter>


CS57CanvasItem::CS57CanvasItem(CS57Control *control)
	: m_pControl(control)
{
	m_pControl->scene()->addItem(this);
}

CS57CanvasItem::~CS57CanvasItem()
{
	update();
}

void CS57CanvasItem::setRect(const QRectF rect)
{
	prepareGeometryChange();
	m_Rect = rect;
	update();
}

QRectF CS57CanvasItem::boundingRect() const
{
	return m_Rect;
}

void CS57CanvasItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	paint(painter);
}

void CS57CanvasItem::updatePosition()
{
	setRect(m_Rect);
}

CS57CanvasMap::CS57CanvasMap(CS57Control *control)
	: CS57CanvasItem(control)
{}

CS57CanvasMap::~CS57CanvasMap()
{}

//************************************
// Method:    setContent
// Brief:	  设置画布内容
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QImage * img
// Parameter: QRectF rect
//************************************
void CS57CanvasMap::setContent(QImage* img, QRectF rect)
{
	m_pImage = img;
	setRect(rect);
}
//************************************
// Method:    paint
// Brief:	  绘制画布
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QPainter * painter
//************************************
void CS57CanvasMap::paint(QPainter *painter)
{
	painter->drawImage(0, 0, *m_pImage);
}