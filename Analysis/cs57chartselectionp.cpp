#include "cs57chartselectionp.h"

using namespace Analysis;
CS57ChartSelectionP::CS57ChartSelectionP()
	: CS57ChartSelection()
{}

CS57ChartSelectionP::~CS57ChartSelectionP()
{}

//************************************
// Method:    startSelect
// Brief:	  ¿ªÊ¼³éÑ¡
// Returns:   void
// Author:    cl
// DateTime:  2022/07/22
// Parameter: QPointF ptf
//************************************
void CS57ChartSelectionP::startSelect(QPointF ptf)
{
	m_vecSelectedMcovrs.clear();
	QMap<CS57CellMessage*, CSpatialPolygon*> mapSpatialPolygon = m_pDoc->getMcovrProviderPtr()->getMcovrPolygons();
	if (mapSpatialPolygon.size() == 0)
		return;

	QMap<CS57CellMessage*, CSpatialPolygon*>::iterator itr = mapSpatialPolygon.begin();
	for (; itr != mapSpatialPolygon.end(); itr++)
	{
		CSpatialPolygon* spatialPolygon = itr.value();
		if (spatialPolygon->ptInPolygon(ptf))
		{
			m_vecSelectedMcovrs.push_back(itr.key());
		}
	}
}
