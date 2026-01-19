#include "cs57layertask.h"

CS57LayerTask::CS57LayerTask()
{}

CS57LayerTask::~CS57LayerTask()
{}

//************************************
// Method:    addLayer
// Brief:	  添加层
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CS57AbstractLayer * layer
//************************************
void CS57LayerTask::addLayer(CS57AbstractLayer* layer)
{
	if (m_pvecTaskLayers.size() != 0)
	{
		QString layerName = layer->getLayerName();
		bool in = false;
		for (int i = 0; i < m_pvecTaskLayers.size(); i++)
		{
			QString strLayerName = m_pvecTaskLayers[i]->getLayerName();
			if (layerName == strLayerName)
			{
				//m_pvecTaskLayers[i] = layer;//存在直接替换
				in = true;
				break;
			}
		}
		if (!in)
			m_pvecTaskLayers.push_back(layer);//不存在直接添加
	}
	else
	{
		m_pvecTaskLayers.push_back(layer);//不存在直接添加
	}
}

//************************************
// Method:    removeLayer
// Brief:	  移除层
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CS57AbstractLayer * layer
//************************************
void CS57LayerTask::removeLayer(CS57AbstractLayer* layer)
{
	removeLayerFromName(layer->getLayerName());
}

//************************************
// Method:    removeLayerFromName
// Brief:	  通过层名称移除层
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: const QString & name
//************************************
void CS57LayerTask::removeLayerFromName(const QString& name)
{
	if (m_pvecTaskLayers.size() == 0)
		return;

	//默认必备层
	if (name == "layer_cell" || name == "layer_custom" ||
		name == "layer_highlight" || name == "layer_mcovrregion")
		return;

	QVector<CS57AbstractLayer*>::iterator iter;
	for (iter = m_pvecTaskLayers.begin(); iter != m_pvecTaskLayers.end();)
	{
		QString strLayerName = (*iter)->getLayerName();
		if (strLayerName == name)
		{
			delete *iter;
			*iter = nullptr;

			iter = m_pvecTaskLayers.erase(iter);

			if (iter == m_pvecTaskLayers.end())
				break;
		}
		iter++;
	}
}

//************************************
// Method:    getLayer
// Brief:	  通过层名称获取层
// Returns:   CS57AbstractLayer*
// Author:    cl
// DateTime:  2022/07/21
// Parameter: const QString & name
//************************************
CS57AbstractLayer* CS57LayerTask::getLayer(const QString& name)
{
	if (m_pvecTaskLayers.size() == 0)
		return nullptr;

	//默认必备层
	if (name == "layer_cell" || name == "layer_custom" ||
		name == "layer_highlight" || name == "layer_mcovrregion")
		return nullptr;

	for (int i = 0; i < m_pvecTaskLayers.size(); i++)
	{
		QString strLayerName = m_pvecTaskLayers[i]->getLayerName();
		if (strLayerName == name)
		{
			return m_pvecTaskLayers[i];
		}
	}
	return  nullptr;
}

//************************************
// Method:    render
// Brief:	  渲染层任务
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QPainter * p
//************************************
void CS57LayerTask::render(QPainter* p)
{
	for (int i = 0; i < m_pvecTaskLayers.size(); i++)
	{
		if(m_pvecTaskLayers[i]->getLayerVisible())
			m_pvecTaskLayers[i]->render(p);
	}
}