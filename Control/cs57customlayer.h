#pragma once
#include "control_global.h"
#include "cs57abstractlayer.h"
#include "cs57auxiliaryrenderer.h"
#include <QPoint>

using namespace Core;
class CS57CustomLayer : public CS57AbstractLayer
{
public:
	CS57CustomLayer(const QString& name = "layer_custom");
	~CS57CustomLayer();

	struct sAuxiliaryParameters
	{
		bool	enabled = false;
		QPoint	centerPt = QPoint(0, 0);
	};

	//设置辅助线参数
	void	setAuxiliaryParameter(sAuxiliaryParameters para);
	//获取辅助线参数
	sAuxiliaryParameters getAuxiliaryParameter();
	//渲染辅助线
	void	render(QPainter* p);

private:
	//渲染辅助线
	void	renderAuxiliary(QPainter* p);

private:
	sAuxiliaryParameters	m_AuxiliaryParameters;
	CS57AuxiliaryRenderer*	m_pAuxiliaryRenderer;
};