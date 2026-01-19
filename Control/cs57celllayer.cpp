#include "cs57celllayer.h"
#include "cs57transform.h"


CS57CellLayer::CS57CellLayer(const QString& name)
	: CS57AbstractLayer(name)
	, m_pGlobalMapRenderer(new CS57GlobalMapRenderer())
	, m_pMcovrRenderer(new CS57McovrRenderer())
	, m_pCellRenderer(new CS57CellRenderer())
	, m_bLayerVisible(true)
{}

CS57CellLayer::~CS57CellLayer()
{
	if (m_pGlobalMapRenderer != nullptr)
		delete m_pGlobalMapRenderer;
	m_pGlobalMapRenderer = nullptr;

	if (m_pMcovrRenderer != nullptr)
		delete m_pMcovrRenderer;
	m_pMcovrRenderer = nullptr;

	if (m_pCellRenderer != nullptr)
		delete m_pCellRenderer;
	m_pCellRenderer = nullptr;
}

//************************************
// Method:    renderGlobalMap
// Brief:	  渲染全球底图
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QPainter * p
//************************************
void CS57CellLayer::renderGlobalMap(QPainter* p)
{
	m_pGlobalMapRenderer->setVisible(m_GlobalMapParameters.visible);
	m_pGlobalMapRenderer->setLineWidth(m_GlobalMapParameters.lineWidth);
	m_pGlobalMapRenderer->setLineColor(m_GlobalMapParameters.lineColor);
	m_pGlobalMapRenderer->setFillColor(m_GlobalMapParameters.fillColor);
	m_pGlobalMapRenderer->setDataSource(m_CellDataSource.pGlobalMapProvider);
	m_pGlobalMapRenderer->doRender(p);
}

//************************************
// Method:    renderMcovr
// Brief:	  渲染MCOVER
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QPainter * p
//************************************
void CS57CellLayer::renderMcovr(QPainter* p)
{
	m_pMcovrRenderer->setVisible(m_McovrParameters.visible);//暂时没加
	m_pMcovrRenderer->setLineWidth(m_McovrParameters.lineWidth);
	m_pMcovrRenderer->setLineColor(m_McovrParameters.lineColor);
	m_pMcovrRenderer->setFillColor(m_McovrParameters.fillColor);
	m_pMcovrRenderer->setDataSource(m_CellDataSource.pMcovrProvider);
	m_pMcovrRenderer->doRender(p);
}

//************************************
// Method:    renderCell
// Brief:	  渲染图层
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QPainter * p
//************************************
void CS57CellLayer::renderCell(QPainter* p)
{
	m_pCellRenderer->setBorderVisible(m_bBorderVisible);
	m_pCellRenderer->setSymbolMode(m_CellParameters.symbolMode);
	m_pCellRenderer->setDataSource(m_CellDataSource.pCellProvider);
	m_pCellRenderer->doRender(p);
}

//************************************
// Method:    setPresLib
// Brief:	  设置表达库
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CS57PresLib * pPresLib
//************************************
void CS57CellLayer::setPresLib(CS57PresLib* pPresLib)
{
	m_pGlobalMapRenderer->setPresLib(pPresLib);
	m_pMcovrRenderer->setPresLib(pPresLib);
	m_pCellRenderer->setPresLib(pPresLib);
}

//************************************
// Method:    setSymbolScaleMode
// Brief:	  设置符号缩放模式
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: XT::SymbolScaleMode mode
//************************************
void CS57CellLayer::setSymbolScaleMode(XT::SymbolScaleMode mode)
{
	m_pCellRenderer->setSymbolScaleMode(mode);
}

//************************************
// Method:    render
// Brief:	  渲染
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QPainter * p
//************************************
void CS57CellLayer::render(QPainter* p)
{
	renderGlobalMap(p);
	if (m_bLayerVisible)
	{
		renderMcovr(p);
		renderCell(p);
	}
}

//************************************
// Method:    setCellParameter
// Brief:	  设置图幅参数
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: sCellParameters para
//************************************
void CS57CellLayer::setCellParameter(sCellParameters para)
{
	m_CellParameters = para;
}

//************************************
// Method:    setPrintParameters
// Brief:	  设置打印参数
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: XT::sExportParameters * pPara
//************************************
void CS57CellLayer::setPrintParameters(XT::sExportParameters* pPara)
{
	m_pCellRenderer->setPrintParameters(pPara);
}

//************************************
// Method:    setDataSource
// Brief:	  设置数据源
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: sCellDataSource ds
//************************************
void CS57CellLayer::setDataSource(sCellDataSource ds)
{
	m_CellDataSource = ds;
}

//************************************
// Method:    setGlobalMapParameter
// Brief:	  设置全球底图参数
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: sGlobalMapParameters para
//************************************
void CS57CellLayer::setGlobalMapParameter(sGlobalMapParameters para)
{
	m_GlobalMapParameters = para;
}

//************************************
// Method:    setMcovrParameter
// Brief:	  设置Mcover参数
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: sMcovrParameters para
//************************************
void CS57CellLayer::setMcovrParameter(sMcovrParameters para)
{
	m_McovrParameters = para;
}

//************************************
// Method:    getGlobalMapParameter
// Brief:	  获取全球底图参数
// Returns:   CS57CellLayer::sGlobalMapParameters
// Author:    cl
// DateTime:  2022/07/21
//************************************
CS57CellLayer::sGlobalMapParameters CS57CellLayer::getGlobalMapParameter()
{
	return m_GlobalMapParameters;
}

//************************************
// Method:    getMcovrParameter
// Brief:	  获取Mcovr参数
// Returns:   CS57CellLayer::sMcovrParameters
// Author:    cl
// DateTime:  2022/07/21
//************************************
CS57CellLayer::sMcovrParameters CS57CellLayer::getMcovrParameter()
{
	return m_McovrParameters;
}

//************************************
// Method:    getCellParameter
// Brief:	  获取图幅参数
// Returns:   CS57CellLayer::sCellParameters
// Author:    cl
// DateTime:  2022/07/21
//************************************
CS57CellLayer::sCellParameters CS57CellLayer::getCellParameter()
{
	return m_CellParameters;
}

//************************************
// Method:    setBorderVisible
// Brief:	  设置图廓显隐
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: bool visible
//************************************
void CS57CellLayer::setBorderVisible(bool visible)
{
	m_bBorderVisible = visible;
}

//************************************
// Method:    setRenderType
// Brief:	  设置渲染类型
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: XT::RenderType type
//************************************
void CS57CellLayer::setRenderType(XT::RenderType type)
{
	m_pCellRenderer->setRenderType(type);
}


