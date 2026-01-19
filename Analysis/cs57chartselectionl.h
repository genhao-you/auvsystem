#pragma once
#include "analysis_global.h"
#include "cs57chartselection.h"

namespace Analysis
{
	class CS57ChartSelectionL : public CS57ChartSelection
	{
	public:
		CS57ChartSelectionL();
		~CS57ChartSelectionL();

		//其他抽选
		void startSelect(QVector<QVector<QPointF>*>* vecPtfs);//航线带
		void startSelect(CPolylineF* vecPtfs);//航线
	};
}
