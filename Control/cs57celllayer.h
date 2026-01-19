#pragma once
#include "control_global.h"
#include "cs57abstractlayer.h"
#include "cs57renderer.h"

using namespace Core;
class CS57Transform;
//新增图幅渲染类层
class CS57CellLayer : public CS57AbstractLayer
{
public:
	CS57CellLayer(const QString& name = "layer_cell");
	~CS57CellLayer();
	struct sGlobalMapParameters
	{
		bool	visible = true;
		int		lineWidth = 1;
		QColor	lineColor = QColor(71, 71, 61);
		QColor	fillColor = QColor(207, 210, 181);
	};
	struct sMcovrParameters
	{
		bool	visible = true;
		int		lineWidth = 1;
		QColor	lineColor = QColor(255, 0, 0);
		QColor	fillColor = QColor(255, 255, 255, 0);
	};
	struct sCellParameters
	{
		QString symbolMode = "";
	};

	struct sCellDataSource
	{
		CS57GlobalMapProvider* pGlobalMapProvider = nullptr;
		CS57McovrProvider* pMcovrProvider = nullptr;
		CS57CellProvider* pCellProvider = nullptr;
	};
	//设置表达库
	void	setPresLib(CS57PresLib* pPresLib);
	//设置符号缩放模式
	void    setSymbolScaleMode(XT::SymbolScaleMode mode);
	//设置渲染类型
	void	setRenderType(XT::RenderType type);
	//设置全球底图参数
	void	setGlobalMapParameter(sGlobalMapParameters para);
	//设置MCOVR参数
	void	setMcovrParameter(sMcovrParameters para);
	//设置图幅参数
	void	setCellParameter(sCellParameters para);
	//设置打印参数
	void	setPrintParameters(XT::sExportParameters* pPara);
	//设置数据源
	void	setDataSource(sCellDataSource ds);
	//设置图廓显隐
	void	setBorderVisible(bool visible);

	//获取全球底图参数
	sGlobalMapParameters	getGlobalMapParameter();
	//获取MCOVR参数
	sMcovrParameters		getMcovrParameter();
	//获取图幅参数
	sCellParameters			getCellParameter();
	//渲染图幅层
	void	render(QPainter* p);
private:
	//渲染全球底图
	void	renderGlobalMap(QPainter* p);
	//渲染Mcovr
	void	renderMcovr(QPainter* p);
	//渲染图幅
	void	renderCell(QPainter* p);
private:
	CS57PresLib*			m_pPresLib;

	sGlobalMapParameters	m_GlobalMapParameters;
	sMcovrParameters		m_McovrParameters;
	sCellParameters			m_CellParameters;

	sCellDataSource			m_CellDataSource;

	CS57GlobalMapRenderer*	m_pGlobalMapRenderer;
	CS57McovrRenderer*		m_pMcovrRenderer;
	CS57CellRenderer*		m_pCellRenderer;
	bool					m_bLayerVisible;
	bool					m_bBorderVisible;
};

