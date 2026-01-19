#pragma once
#include "cs57abstractlayer.h"

class CONTROL_EXPORT CS57LayerTask
{
public:
	CS57LayerTask();
	~CS57LayerTask();

	//添加层
	void addLayer(CS57AbstractLayer* layer);
	//移除层
	void removeLayer(CS57AbstractLayer* layer);
	//通过层名称移除层
	void removeLayerFromName(const QString& name);
	//通过层名称获取层
	CS57AbstractLayer* getLayer(const QString& name);

	//渲染所有层到画布
	void render(QPainter* p);

private:
	QVector<CS57AbstractLayer*> m_pvecTaskLayers;
};
