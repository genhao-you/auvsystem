#pragma once

#include "cs57chartselection.h"

namespace Analysis
{
	class CS57ChartSelectionA : public CS57ChartSelection
	{
	public:
		CS57ChartSelectionA();
		~CS57ChartSelectionA();

		//¿ªÊ¼³éÑ¡
		void startSelect( CPolylineF vecPts);
	};
}
