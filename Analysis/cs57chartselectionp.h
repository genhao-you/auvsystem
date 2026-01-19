#pragma once

#include "cs57chartselection.h"
namespace Analysis
{
	class CS57ChartSelectionP : public CS57ChartSelection
	{
	public:
		CS57ChartSelectionP();
		~CS57ChartSelectionP();

		//¿ªÊ¼³éÑ¡
		void startSelect(QPointF ptf);
	};
}