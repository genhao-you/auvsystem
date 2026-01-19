#include "cs57chartselectiona.h"

using namespace Analysis;
CS57ChartSelectionA::CS57ChartSelectionA()
	: CS57ChartSelection()
{}

CS57ChartSelectionA::~CS57ChartSelectionA()
{}

//************************************
// Method:    startSelect
// Brief:	  开始抽选
// Returns:   void
// Author:    cl
// DateTime:  2022/07/22
// Parameter: CPolylineF vecPts
//************************************
void CS57ChartSelectionA::startSelect(CPolylineF vecPts)
{
	if (vecPts.size() == 0)
		return;

	m_vecSelectedMcovrs.clear();
	QMap<CS57CellMessage*, CSpatialPolygon*> mapSpatialPolygon = m_pDoc->getMcovrProviderPtr()->getMcovrPolygons();
	if (mapSpatialPolygon.size() == 0)
		return;

	QMap<CS57CellMessage*, CSpatialPolygon*>::iterator itr = mapSpatialPolygon.begin();
	for (; itr != mapSpatialPolygon.end(); itr++)
	{
		CSpatialPolygon* spatialPolygon = itr.value();

		//如果首尾不同，说明没有闭合，此处闭合一下
		if (vecPts[0] != vecPts[vecPts.size() - 1])
			vecPts.push_back(vecPts[0]);

		if (spatialPolygon->isIntersectPolygon(&vecPts))
		{
			m_vecSelectedMcovrs.push_back(itr.key());
		}
	}
}
