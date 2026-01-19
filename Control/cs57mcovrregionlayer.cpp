#include "cs57mcovrregionlayer.h"
#include "cs57transform.h"


CS57McovrRegionLayer::CS57McovrRegionLayer(const QString& name)
	: CS57AbstractLayer(name)
	, m_pTransform(CS57Transform::instance())
	, m_pMcovrRegionRenderer(new CS57McovrRegionRenderer())
	, m_bLayerVisible(true)
{}

CS57McovrRegionLayer::~CS57McovrRegionLayer()
{
	if (m_pMcovrRegionRenderer != nullptr)
		delete m_pMcovrRegionRenderer;
	m_pMcovrRegionRenderer = nullptr;
}

//************************************
// Method:    render
// Brief:	  渲染
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QPainter * p
//************************************
void CS57McovrRegionLayer::render(QPainter* p)
{
	if (m_bLayerVisible)
	{
		m_pMcovrRegionRenderer->setLineWidth(CS57McovrRegionRenderer::Region_I, m_McovrRegionParameters.mcovrRegionI.lineWidth);
		m_pMcovrRegionRenderer->setLineColor(CS57McovrRegionRenderer::Region_I, m_McovrRegionParameters.mcovrRegionI.lineColor);
		m_pMcovrRegionRenderer->setFillColor(CS57McovrRegionRenderer::Region_I, m_McovrRegionParameters.mcovrRegionI.fillColor);

		m_pMcovrRegionRenderer->setLineWidth(CS57McovrRegionRenderer::Region_II, m_McovrRegionParameters.mcovrRegionII.lineWidth);
		m_pMcovrRegionRenderer->setLineColor(CS57McovrRegionRenderer::Region_II, m_McovrRegionParameters.mcovrRegionII.lineColor);
		m_pMcovrRegionRenderer->setFillColor(CS57McovrRegionRenderer::Region_II, m_McovrRegionParameters.mcovrRegionII.fillColor);

		m_pMcovrRegionRenderer->setLineWidth(CS57McovrRegionRenderer::Region_III, m_McovrRegionParameters.mcovrRegionIII.lineWidth);
		m_pMcovrRegionRenderer->setLineColor(CS57McovrRegionRenderer::Region_III, m_McovrRegionParameters.mcovrRegionIII.lineColor);
		m_pMcovrRegionRenderer->setFillColor(CS57McovrRegionRenderer::Region_III, m_McovrRegionParameters.mcovrRegionIII.fillColor);

		m_pMcovrRegionRenderer->setLineWidth(CS57McovrRegionRenderer::Region_IV, m_McovrRegionParameters.mcovrRegionIV.lineWidth);
		m_pMcovrRegionRenderer->setLineColor(CS57McovrRegionRenderer::Region_IV, m_McovrRegionParameters.mcovrRegionIV.lineColor);
		m_pMcovrRegionRenderer->setFillColor(CS57McovrRegionRenderer::Region_IV, m_McovrRegionParameters.mcovrRegionIV.fillColor);

		m_pMcovrRegionRenderer->setLineWidth(CS57McovrRegionRenderer::Region_V, m_McovrRegionParameters.mcovrRegionV.lineWidth);
		m_pMcovrRegionRenderer->setLineColor(CS57McovrRegionRenderer::Region_V, m_McovrRegionParameters.mcovrRegionV.lineColor);
		m_pMcovrRegionRenderer->setFillColor(CS57McovrRegionRenderer::Region_V, m_McovrRegionParameters.mcovrRegionV.fillColor);

		m_pMcovrRegionRenderer->setDataSource(m_McovrRegionDataSource.pMcovrRegionProvider);
		m_pMcovrRegionRenderer->doRender(p);
	}
}

//************************************
// Method:    setMcovrRegionParameter
// Brief:	  设置MCOVR分区参数
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: sMcovrRegionParameters para
//************************************
void CS57McovrRegionLayer::setMcovrRegionParameter(sMcovrRegionParameters para)
{
	m_McovrRegionParameters = para;
}

//************************************
// Method:    setDataSource
// Brief:	  设置数据源
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: sMcoverRegionDataSource ds
//************************************
void CS57McovrRegionLayer::setDataSource(sMcoverRegionDataSource ds)
{
	m_McovrRegionDataSource = ds;
}


//************************************
// Method:    getMcovrRegionParameter
// Brief:	  获取MCOVR分区参数
// Returns:   CS57McovrRegionLayer::sMcovrRegionParameters
// Author:    cl
// DateTime:  2022/07/21
//************************************
CS57McovrRegionLayer::sMcovrRegionParameters CS57McovrRegionLayer::getMcovrRegionParameter()
{
	return m_McovrRegionParameters;
}