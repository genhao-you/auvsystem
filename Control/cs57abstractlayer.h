#pragma once
#include "control_global.h"
#include <QPainter>
#include <QString>
class CS57AbstractLayer : public QObject
{
	Q_OBJECT
public:
	CS57AbstractLayer();
	CS57AbstractLayer(const QString& name);
	virtual ~CS57AbstractLayer() = 0;
	//设置层名称
	void setLayerName(const QString& name);
	//获取层名称
	QString getLayerName() const;

	//设置层显隐
	void setLayerVisible(bool visible);
	//获取层显隐
	bool getLayerVisible() const;

	virtual void render(QPainter* p) = 0;

private:
	QString _layerName;
	bool	_layerVisible;
};
