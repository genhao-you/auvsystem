#include "cs57insetselection.h"

using namespace Analysis;
CS57InSetSelection::CS57InSetSelection()
	: CS57ChartSelectionL()
{
}

CS57InSetSelection::~CS57InSetSelection()
{
}
//************************************
// Method:    selectInSet
// Brief:	  套系抽选
// Returns:   QString
// Author:    cl
// DateTime:  2022/07/22
// Parameter: QVector<QVector<QPointF> * > * vecPtfs
//************************************
QString CS57InSetSelection::selectInSet(QVector<QVector<QPointF>*>* vecPtfs)
{
	QString strFlag = "";
	if (isIntersectRegionI(vecPtfs))
		strFlag += "A5";

	if (isIntersectRegionII(vecPtfs))
		strFlag += "B4";

	if (isIntersectRegionIII(vecPtfs))
		strFlag += "C3";

	if (isIntersectRegionIV(vecPtfs))
		strFlag += "D2";

	if (isIntersectRegionV(vecPtfs))
		strFlag += "E1";

	if (strFlag.length() == 2)
		return strFlag;

	return "";
}

//************************************
// Method:    selectInSet
// Brief:	  套系抽选
// Returns:   QString
// Author:    cl
// DateTime:  2022/07/22
// Parameter: CPolylineF * vecPtfs
//************************************
QString CS57InSetSelection::selectInSet(CPolylineF* vecPtfs)
{
	QString strFlag = "";
	if (isIntersectRegionI(vecPtfs))
		strFlag += "A5";

	if (isIntersectRegionII(vecPtfs))
		strFlag += "B4";

	if (isIntersectRegionIII(vecPtfs))
		strFlag += "C3";

	if (isIntersectRegionIV(vecPtfs))
		strFlag += "D2";

	if (isIntersectRegionV(vecPtfs))
		strFlag += "E1";

	if (strFlag.length() == 2)
		return strFlag;

	return "";
}
bool CS57InSetSelection::isIntersectRegionI(QVector<QVector<QPointF>*>* vecPtfs)
{
	CS57McovrRegionProvider* provider = m_pDoc->getMcovrRegionProviderPtr();
	if (provider->m_pResultSpatialPolygonI == nullptr)
		return false;

	//double minX = provider->m_pResultSpatialPolygonI->minX();
	//double minY = provider->m_pResultSpatialPolygonI->minY();
	//double maxX = provider->m_pResultSpatialPolygonI->maxX();
	//double maxY = provider->m_pResultSpatialPolygonI->maxY();
	//QString strValue = QString("%1,%2,%3,%4").arg(QString::number(minX)).arg(QString::number(minY)).arg(QString::number(maxX)).arg(QString::number(maxY));

	if (provider->m_pResultSpatialPolygonI->isIntersectPolyline(vecPtfs))
		return true;

	return false;
}
bool CS57InSetSelection::isIntersectRegionII(QVector<QVector<QPointF>*>* vecPtfs)
{
	CS57McovrRegionProvider* provider = m_pDoc->getMcovrRegionProviderPtr();
	if (provider->m_pResultSpatialPolygonII == nullptr)
		return false;

	//double minX = provider->m_pResultSpatialPolygonII->minX();
	//double minY = provider->m_pResultSpatialPolygonII->minY();
	//double maxX = provider->m_pResultSpatialPolygonII->maxX();
	//double maxY = provider->m_pResultSpatialPolygonII->maxY();
	//QString strValue = QString("%1,%2,%3,%4").arg(QString::number(minX)).arg(QString::number(minY)).arg(QString::number(maxX)).arg(QString::number(maxY));

	if (provider->m_pResultSpatialPolygonII->isIntersectPolyline(vecPtfs))
		return true;

	return false;
}
bool CS57InSetSelection::isIntersectRegionIII(QVector<QVector<QPointF>*>* vecPtfs)
{
	CS57McovrRegionProvider* provider = m_pDoc->getMcovrRegionProviderPtr();
	if (provider->m_pResultSpatialPolygonIII == nullptr)
		return false;

	//double minX = provider->m_pResultSpatialPolygonIII->minX();
	//double minY = provider->m_pResultSpatialPolygonIII->minY();
	//double maxX = provider->m_pResultSpatialPolygonIII->maxX();
	//double maxY = provider->m_pResultSpatialPolygonIII->maxY();
	//QString strValue = QString("%1,%2,%3,%4").arg(QString::number(minX)).arg(QString::number(minY)).arg(QString::number(maxX)).arg(QString::number(maxY));

	if (provider->m_pResultSpatialPolygonIII->isIntersectPolyline(vecPtfs))
		return true;

	return false;
}
bool CS57InSetSelection::isIntersectRegionIV(QVector<QVector<QPointF>*>* vecPtfs)
{
	CS57McovrRegionProvider* provider = m_pDoc->getMcovrRegionProviderPtr();
	if (provider->m_pResultSpatialPolygonIV == nullptr)
		return false;

	//double minX = provider->m_pResultSpatialPolygonIV->minX();
	//double minY = provider->m_pResultSpatialPolygonIV->minY();
	//double maxX = provider->m_pResultSpatialPolygonIV->maxX();
	//double maxY = provider->m_pResultSpatialPolygonIV->maxY();
	//QString strValue = QString("%1,%2,%3,%4").arg(QString::number(minX)).arg(QString::number(minY)).arg(QString::number(maxX)).arg(QString::number(maxY));

	if (provider->m_pResultSpatialPolygonIV->isIntersectPolyline(vecPtfs))
		return true;

	return false;
}
bool CS57InSetSelection::isIntersectRegionV(QVector<QVector<QPointF>*>* vecPtfs)
{
	CS57McovrRegionProvider* provider = m_pDoc->getMcovrRegionProviderPtr();
	if (provider->m_pResultSpatialPolygonV == nullptr)
		return false;

	//double minX = provider->m_pResultSpatialPolygonV->minX();
	//double minY = provider->m_pResultSpatialPolygonV->minY();
	//double maxX = provider->m_pResultSpatialPolygonV->maxX();
	//double maxY = provider->m_pResultSpatialPolygonV->maxY();
	//QString strValue = QString("%1,%2,%3,%4").arg(QString::number(minX)).arg(QString::number(minY)).arg(QString::number(maxX)).arg(QString::number(maxY));
	//QMessageBox::information(nullptr, "提示", strValue);

	if (provider->m_pResultSpatialPolygonV->isIntersectPolyline(vecPtfs))
		return true;

	return false;
}
bool CS57InSetSelection::isIntersectRegionI(CPolylineF* vecPtfs)
{
	CS57McovrRegionProvider* provider = m_pDoc->getMcovrRegionProviderPtr();
	if (provider->m_pResultSpatialPolygonI == nullptr)
		return false;

	//double minX = provider->m_pResultSpatialPolygonI->minX();
	//double minY = provider->m_pResultSpatialPolygonI->minY();
	//double maxX = provider->m_pResultSpatialPolygonI->maxX();
	//double maxY = provider->m_pResultSpatialPolygonI->maxY();
	//QString strValue = QString("%1,%2,%3,%4").arg(QString::number(minX)).arg(QString::number(minY)).arg(QString::number(maxX)).arg(QString::number(maxY));

	if (provider->m_pResultSpatialPolygonI->isIntersectPolyline(vecPtfs))
		return true;

	return false;
}
bool CS57InSetSelection::isIntersectRegionII(CPolylineF* vecPtfs)
{
	CS57McovrRegionProvider* provider = m_pDoc->getMcovrRegionProviderPtr();
	if (provider->m_pResultSpatialPolygonII == nullptr)
		return false;

	//double minX = provider->m_pResultSpatialPolygonII->minX();
	//double minY = provider->m_pResultSpatialPolygonII->minY();
	//double maxX = provider->m_pResultSpatialPolygonII->maxX();
	//double maxY = provider->m_pResultSpatialPolygonII->maxY();
	//QString strValue = QString("%1,%2,%3,%4").arg(QString::number(minX)).arg(QString::number(minY)).arg(QString::number(maxX)).arg(QString::number(maxY));

	if (provider->m_pResultSpatialPolygonII->isIntersectPolyline(vecPtfs))
		return true;

	return false;
}
bool CS57InSetSelection::isIntersectRegionIII(CPolylineF* vecPtfs)
{
	CS57McovrRegionProvider* provider = m_pDoc->getMcovrRegionProviderPtr();
	if (provider->m_pResultSpatialPolygonIII == nullptr)
		return false;

	//double minX = provider->m_pResultSpatialPolygonIII->minX();
	//double minY = provider->m_pResultSpatialPolygonIII->minY();
	//double maxX = provider->m_pResultSpatialPolygonIII->maxX();
	//double maxY = provider->m_pResultSpatialPolygonIII->maxY();
	//QString strValue = QString("%1,%2,%3,%4").arg(QString::number(minX)).arg(QString::number(minY)).arg(QString::number(maxX)).arg(QString::number(maxY));

	if (provider->m_pResultSpatialPolygonIII->isIntersectPolyline(vecPtfs))
		return true;

	return false;
}
bool CS57InSetSelection::isIntersectRegionIV(CPolylineF* vecPtfs)
{
	CS57McovrRegionProvider* provider = m_pDoc->getMcovrRegionProviderPtr();
	if (provider->m_pResultSpatialPolygonIV == nullptr)
		return false;

	//double minX = provider->m_pResultSpatialPolygonIV->minX();
	//double minY = provider->m_pResultSpatialPolygonIV->minY();
	//double maxX = provider->m_pResultSpatialPolygonIV->maxX();
	//double maxY = provider->m_pResultSpatialPolygonIV->maxY();
	//QString strValue = QString("%1,%2,%3,%4").arg(QString::number(minX)).arg(QString::number(minY)).arg(QString::number(maxX)).arg(QString::number(maxY));

	if (provider->m_pResultSpatialPolygonIV->isIntersectPolyline(vecPtfs))
		return true;

	return false;
}
bool CS57InSetSelection::isIntersectRegionV(CPolylineF* vecPtfs)
{
	CS57McovrRegionProvider* provider = m_pDoc->getMcovrRegionProviderPtr();
	if (provider->m_pResultSpatialPolygonV == nullptr)
		return false;

	//double minX = provider->m_pResultSpatialPolygonV->minX();
	//double minY = provider->m_pResultSpatialPolygonV->minY();
	//double maxX = provider->m_pResultSpatialPolygonV->maxX();
	//double maxY = provider->m_pResultSpatialPolygonV->maxY();
	//QString strValue = QString("%1,%2,%3,%4").arg(QString::number(minX)).arg(QString::number(minY)).arg(QString::number(maxX)).arg(QString::number(maxY));
	//QMessageBox::information(nullptr, "提示", strValue);

	if (provider->m_pResultSpatialPolygonV->isIntersectPolyline(vecPtfs))
		return true;

	return false;
}