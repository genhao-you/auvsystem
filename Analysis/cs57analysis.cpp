#include "stdafx.h"
#include "cs57analysis.h"
#include "cs57chartselection.h"
#include "cs57chartselectionp.h"
#include "cs57chartselectionl.h"
#include "cs57insetselection.h"
#include "cs57summaryselection.h"
#include "cs57inregionselection.h"
#include "cs57indetailselection.h"
#include "cs57chartselectiona.h"


using namespace Analysis;
CS57Analysis* CS57Analysis::m_pInstance = nullptr;
CS57Analysis::CS57Analysis()
	: m_pFeatureQuery(new CS57FeatureQuery())
	, m_pChartSelectionP(new CS57ChartSelectionP())
	, m_pChartSelectionL(new CS57ChartSelectionL())
	, m_pInSetSelection(new CS57InSetSelection())
	, m_pInDetailSelection(new CS57InDetailSelection())
	, m_pSummarySelection(new CS57SummarySelection())
	, m_pInRegionSelection(new CS57InRegionSelection())
	, m_pChartSelectionA(new CS57ChartSelectionA())
{
	m_pInstance = this;
}

CS57Analysis::~CS57Analysis()
{
	if (m_pFeatureQuery != nullptr)
		delete m_pFeatureQuery;
	m_pFeatureQuery = nullptr;

	if (m_pChartSelectionP != nullptr)
		delete m_pChartSelectionP;
	m_pChartSelectionP = nullptr;

	if (m_pChartSelectionL != nullptr)
		delete m_pChartSelectionL;
	m_pChartSelectionL = nullptr;

	if (m_pInSetSelection != nullptr)
		delete m_pInSetSelection;
	m_pInSetSelection = nullptr;

	if (m_pSummarySelection != nullptr)
		delete m_pSummarySelection;
	m_pSummarySelection = nullptr;

	if (m_pInRegionSelection != nullptr)
		delete m_pInRegionSelection;
	m_pInRegionSelection = nullptr;

	if (m_pInDetailSelection != nullptr)
		delete m_pInDetailSelection;
	m_pInDetailSelection = nullptr;

	if (m_pChartSelectionA != nullptr)
		delete m_pChartSelectionA;
	m_pChartSelectionA = nullptr;
}

void CS57Analysis::setDoc(CS57DocManager* doc)
{
	m_pFeatureQuery->setDoc(doc);
	m_pChartSelectionP->setDoc(doc);
	m_pChartSelectionL->setDoc(doc);
	m_pInSetSelection->setDoc(doc);
	m_pSummarySelection->setDoc(doc);
	m_pInRegionSelection->setDoc(doc);
	m_pInDetailSelection->setDoc(doc);
	m_pChartSelectionA->setDoc(doc);
}
void CS57Analysis::setPresLib(CS57PresLib* prelib)
{
	m_pFeatureQuery->setPresLib(prelib);
}
CS57FeatureQuery* CS57Analysis::getFeatureQueryPtr()
{
	return m_pFeatureQuery;
}

CS57ChartSelectionP* CS57Analysis::getChartSelectionPPtr()
{
	return m_pChartSelectionP;
}

CS57ChartSelectionL* CS57Analysis::getChartSelectionLPtr()
{
	return m_pChartSelectionL;
}

CS57ChartSelectionA* CS57Analysis::getChartSelectionAPtr()
{
	return m_pChartSelectionA;
}

CS57InSetSelection* CS57Analysis::getInSetSelectionPtr()
{
	return m_pInSetSelection;
}

CS57SummarySelection* CS57Analysis::getSummarySelectionPtr()
{
	return m_pSummarySelection;
}

CS57InRegionSelection* CS57Analysis::getInRegionSelectionPtr()
{
	return m_pInRegionSelection;
}

CS57InDetailSelection* CS57Analysis::getInDetailSelectionPtr()
{
	return m_pInDetailSelection;
}
