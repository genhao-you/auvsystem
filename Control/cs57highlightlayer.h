#pragma once
#include "control_global.h"
#include "cs57abstractlayer.h"
#include "cs57highlight.h"
#include <QPainter>

using namespace Core;
class CS57HighlightLayer : public CS57AbstractLayer
{
public:
	CS57HighlightLayer(const QString& name = "layer_highlight");
	~CS57HighlightLayer();

	struct sHighlightParameters
	{
		int		lineWidth = 2;
		QColor	lineColor = QColor(255, 0, 0);
		QColor	fillColor = QColor(255, 0, 0, 100);
		XT::OperateMode operateMode;
		XT::QueryMode	queryMode;
	};
	struct sHighlightDataSource
	{
		CS57RecFeature* pHighlightFeature = nullptr;
	};
	void	setHighlightParameter(sHighlightParameters para);
	void	setDataSource(sHighlightDataSource ds);

	void	render(QPainter* p);
private:
	void	renderHighlight(QPainter* p);

private:
	sHighlightParameters	m_HighlightParameters;
	sHighlightDataSource	m_HighlightDataSource;
	CS57FeatureHighlight*	m_pHighlightRenderer;
};
