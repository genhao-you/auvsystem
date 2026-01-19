#pragma once

#include "cs57chartselectionl.h"

namespace Analysis
{
	class CS57InSetSelection : public CS57ChartSelectionL
	{
	public:
		CS57InSetSelection();
		~CS57InSetSelection();

		//套系抽选
		QString selectInSet(QVector<QVector<QPointF>*>* vecPtfs);
		QString selectInSet(CPolylineF* vecPtfs);

	private:
		//是否与区域I相交
		bool isIntersectRegionI(QVector<QVector<QPointF>*>* vecPtfs);
		//是否与区域II相交
		bool isIntersectRegionII(QVector<QVector<QPointF>*>* vecPtfs);
		//是否与区域III相交
		bool isIntersectRegionIII(QVector<QVector<QPointF>*>* vecPtfs);
		//是否与区域IV相交
		bool isIntersectRegionIV(QVector<QVector<QPointF>*>* vecPtfs);
		//是否与区域V相交
		bool isIntersectRegionV(QVector<QVector<QPointF>*>* vecPtfs);

		bool isIntersectRegionI(CPolylineF* vecPtfs);
		bool isIntersectRegionII(CPolylineF* vecPtfs);
		bool isIntersectRegionIII(CPolylineF* vecPtfs);
		bool isIntersectRegionIV(CPolylineF* vecPtfs);
		bool isIntersectRegionV(CPolylineF* vecPtfs);
	};
}
