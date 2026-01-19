#include "cs57customlayer.h"


CS57CustomLayer::CS57CustomLayer(const QString& name)
	: CS57AbstractLayer(name)
	, m_pAuxiliaryRenderer(new CS57AuxiliaryRenderer())
{}

CS57CustomLayer::~CS57CustomLayer()
{
	if (m_pAuxiliaryRenderer != nullptr)
		delete m_pAuxiliaryRenderer;
	m_pAuxiliaryRenderer = nullptr;
}

//************************************
// Method:    renderAuxiliary
// Brief:	  渲染辅助线
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QPainter * p
//************************************
void CS57CustomLayer::renderAuxiliary(QPainter* p)
{
	m_pAuxiliaryRenderer->setEnabled(m_AuxiliaryParameters.enabled);
	m_pAuxiliaryRenderer->setCenterPt(m_AuxiliaryParameters.centerPt);
	m_pAuxiliaryRenderer->doRender(p);
}

//************************************
// Method:    setAuxiliaryParameter
// Brief:	  设置辅助线参数
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: sAuxiliaryParameters para
//************************************
void CS57CustomLayer::setAuxiliaryParameter(sAuxiliaryParameters para)
{
	m_AuxiliaryParameters = para;
}

//************************************
// Method:    render
// Brief:	  渲染
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QPainter * p
//************************************
void CS57CustomLayer::render(QPainter* p)
{
	renderAuxiliary(p);
}

//************************************
// Method:    getAuxiliaryParameter
// Brief:	  获取辅助线参数
// Returns:   CS57CustomLayer::sAuxiliaryParameters
// Author:    cl
// DateTime:  2022/07/21
//************************************
CS57CustomLayer::sAuxiliaryParameters CS57CustomLayer::getAuxiliaryParameter()
{
	return m_AuxiliaryParameters;
}
