#pragma once

#include "control_global.h"
#include <QGraphicsItem>

class CS57Control;
class CONTROL_EXPORT CS57CanvasItem : public QGraphicsItem
{
protected:
	CS57CanvasItem(CS57Control *control = nullptr);
	~CS57CanvasItem() override;

	virtual void paint(QPainter *painter) = 0;
	void paint(QPainter *painter,const QStyleOptionGraphicsItem *option,QWidget *widget = nullptr) override;
public:
	//设置Item矩形
	void setRect(const QRectF rect);
	virtual void updatePosition();
	QRectF boundingRect() const override;
private:
	CS57Control* m_pControl;
	QRectF m_Rect;
};

class CONTROL_EXPORT CS57CanvasMap : public CS57CanvasItem
{
public:
	CS57CanvasMap(CS57Control *control = nullptr);
	~CS57CanvasMap();

public:
	//设置画布内容
	void setContent(QImage* img,QRectF rect);
protected:
	//绘制
	void paint(QPainter *painter) override;

private:
	QImage* m_pImage;
};
