#pragma once
#include "control_global.h"
#include "cs57abstractlayer.h"
#include "cs57renderer.h"

using namespace Core;
class CS57Transform;
//–¬‘ˆÕº∑˘‰÷»æ¿‡≤„
class CS57McovrRegionLayer : public CS57AbstractLayer
{
public:
	CS57McovrRegionLayer(const QString& name = "layer_mcovrregion");
	~CS57McovrRegionLayer();
	struct sSubRegion
	{
		bool	visible = true;
		int		lineWidth = 1;
		QColor	lineColor;
		QColor	fillColor;
	};
	struct sMcovrRegionParameters
	{
		sSubRegion mcovrRegionI = { true,1,QColor(71, 71, 61) ,QColor(255, 0, 0, 100) };
		sSubRegion mcovrRegionII = { true,1,QColor(71, 71, 61) ,QColor(255, 255, 0, 100) };
		sSubRegion mcovrRegionIII = { true,1,QColor(71, 71, 61) ,QColor(0, 255, 0, 100) };
		sSubRegion mcovrRegionIV = { true,1,QColor(71, 71, 61) , QColor(0, 255, 255, 100) };
		sSubRegion mcovrRegionV = { true,1,QColor(71, 71, 61) ,QColor(0, 0, 255, 100) };
	};

	struct sMcoverRegionDataSource
	{
		CS57McovrRegionProvider* pMcovrRegionProvider = nullptr;
	};

	void	setMcovrRegionParameter(sMcovrRegionParameters para);
	void	setDataSource(sMcoverRegionDataSource ds);

	sMcovrRegionParameters	getMcovrRegionParameter();

	void	render(QPainter* p);
private:
	CS57Transform*			m_pTransform;

	sMcovrRegionParameters	m_McovrRegionParameters;
	sMcoverRegionDataSource	m_McovrRegionDataSource;
	CS57McovrRegionRenderer*m_pMcovrRegionRenderer;
	bool					m_bLayerVisible;
};

