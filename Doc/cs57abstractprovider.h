#pragma once
#include "doc_global.h"

class CSpatialPolygon;
namespace Doc
{
	class DOC_EXPORT CS57AbstractProvider
	{
	public:
		CS57AbstractProvider();
		~CS57AbstractProvider();

		virtual void selectChart(CSpatialPolygon* pPolygon, double scale) = 0;
	};
}

