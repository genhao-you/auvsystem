#include "cs57highlightlayer.h"


CS57HighlightLayer::CS57HighlightLayer(const QString& name)
	: CS57AbstractLayer(name)
	, m_pHighlightRenderer(new CS57FeatureHighlight())
{}

CS57HighlightLayer::~CS57HighlightLayer()
{
	if (m_pHighlightRenderer != nullptr)
		delete m_pHighlightRenderer;
	m_pHighlightRenderer = nullptr;
}

//************************************
// Method:    renderHighlight
// Brief:	  渲染高亮部分
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QPainter * p
//************************************
void CS57HighlightLayer::renderHighlight(QPainter* p)
{
	m_pHighlightRenderer->setFillColor(m_HighlightParameters.fillColor);
	m_pHighlightRenderer->setLineColor(m_HighlightParameters.lineColor);
	m_pHighlightRenderer->setLineWidth(m_HighlightParameters.lineWidth);
	m_pHighlightRenderer->setOperateMode(m_HighlightParameters.operateMode);
	m_pHighlightRenderer->setQueryMode(m_HighlightParameters.queryMode);
	m_pHighlightRenderer->setDataSource(m_HighlightDataSource.pHighlightFeature);
	m_pHighlightRenderer->doRender(p);
}

//************************************
// Method:    render
// Brief:	  渲染
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QPainter * p
//************************************
void CS57HighlightLayer::render(QPainter* p)
{
	renderHighlight(p);
}

//************************************
// Method:    setHighlightParameter
// Brief:	  设置高亮参数
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: sHighlightParameters para
//************************************
void CS57HighlightLayer::setHighlightParameter(sHighlightParameters para)
{
	m_HighlightParameters = para;
}

//************************************
// Method:    setDataSource
// Brief:	  设置数据源
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: sHighlightDataSource ds
//************************************
void CS57HighlightLayer::setDataSource(sHighlightDataSource ds)
{
	m_HighlightDataSource = ds;
}
