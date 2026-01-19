#include "cs57chartselectionl.h"
#include <QMessageBox>

using namespace Analysis;
#pragma execution_character_set("utf-8")
CS57ChartSelectionL::CS57ChartSelectionL()
	: CS57ChartSelection()
{}

CS57ChartSelectionL::~CS57ChartSelectionL()
{}

//************************************
// Method:    startSelect
// Brief:	  开始抽选
// Returns:   void
// Author:    cl
// DateTime:  2022/07/22
// Parameter: QVector<QVector<QPointF> * > * vecPtfs
//************************************
void CS57ChartSelectionL::startSelect(QVector<QVector<QPointF>*>* vecPtfs)
{
	if (vecPtfs->size() == 0)
		return;

	m_vecSelectedMcovrs.clear();
	QMap<CS57CellMessage*, CSpatialPolygon*> mapSpatialPolygon = m_pDoc->getMcovrProviderPtr()->getMcovrPolygons();
	if (mapSpatialPolygon.size() == 0)
		return;

	QMap<CS57CellMessage*, CSpatialPolygon*>::iterator itr = mapSpatialPolygon.begin();
	for (; itr != mapSpatialPolygon.end(); itr++)
	{
		CSpatialPolygon* spatialPolygon = itr.value();
		if (spatialPolygon->isIntersectPolyline(vecPtfs))
		{
			m_vecSelectedMcovrs.push_back(itr.key());
		}
	}
}

//************************************
// Method:    startSelect
// Brief:	  开始抽选
// Returns:   void
// Author:    cl
// DateTime:  2022/07/22
// Parameter: CPolylineF * vecPtfs
//************************************
void Analysis::CS57ChartSelectionL::startSelect(CPolylineF* vecPtfs)
{
	if (vecPtfs->size() == 0)
		return;

	m_vecSelectedMcovrs.clear();
	QMap<CS57CellMessage*, CSpatialPolygon*> mapSpatialPolygon = m_pDoc->getMcovrProviderPtr()->getMcovrPolygons();
	if (mapSpatialPolygon.size() == 0)
		return;

	QMap<CS57CellMessage*, CSpatialPolygon*>::iterator itr = mapSpatialPolygon.begin();
	for (; itr != mapSpatialPolygon.end(); itr++)
	{
		CSpatialPolygon* spatialPolygon = itr.value();
		if (spatialPolygon->isIntersectPolyline(vecPtfs))
		{
			m_vecSelectedMcovrs.push_back(itr.key());
		}
	}
}