#pragma once
#include "xt_global.h"
#include "analysis_global.h"
#include "cs57featurequery.h"
#include "cs57projection.h"
#include "cs57viewport.h"

namespace Analysis
{
	class CS57ChartSelection;
	class CS57ChartSelectionP;
	class CS57ChartSelectionL;
	class CS57InSetSelection;
	class CS57SummarySelection;
	class CS57InDetailSelection;
	class CS57InRegionSelection;
	class CS57ChartSelectionA;
	class CS57Analysis
	{
	public:
		~CS57Analysis();

		static CS57Analysis* instance()
		{
			if (m_pInstance == nullptr)
				m_pInstance = new CS57Analysis();

			return m_pInstance;
		}

	public:
		void setDoc(CS57DocManager* doc);
		void setPresLib(CS57PresLib* prelib);
		CS57FeatureQuery* getFeatureQueryPtr();
		CS57ChartSelectionP* getChartSelectionPPtr();
		CS57ChartSelectionL* getChartSelectionLPtr();
		CS57InDetailSelection* getInDetailSelectionPtr();
		CS57InSetSelection* getInSetSelectionPtr();
		CS57InRegionSelection* getInRegionSelectionPtr();
		CS57SummarySelection* getSummarySelectionPtr();
		CS57ChartSelectionA* getChartSelectionAPtr();

	private:
		//不让外部通过new方式创建
		CS57Analysis();
	private:
		static CS57Analysis*		m_pInstance;

		CS57FeatureQuery*			m_pFeatureQuery;
		CS57ChartSelectionP*		m_pChartSelectionP;
		CS57ChartSelectionL*		m_pChartSelectionL;
		CS57InSetSelection*			m_pInSetSelection;
		CS57SummarySelection*		m_pSummarySelection;
		CS57InRegionSelection*		m_pInRegionSelection;
		CS57InDetailSelection*		m_pInDetailSelection;
		CS57ChartSelectionA*		m_pChartSelectionA;
	};
}
