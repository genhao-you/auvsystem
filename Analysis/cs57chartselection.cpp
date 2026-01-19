#include "cs57chartselection.h"
#include "cs57transform.h"

using namespace Analysis;
CS57ChartSelection::CS57ChartSelection()
	: m_pTransform(CS57Transform::instance())
	, m_pDoc(nullptr)
{}

CS57ChartSelection::~CS57ChartSelection()
{}

void CS57ChartSelection::setDoc(CS57DocManager* doc)
{
	m_pDoc = doc;
}

//************************************
// Method:    sortResults
// Brief:	  整理结果并返回
// Returns:   QVector<Analysis::CS57ChartSelection::CS57ChartSelection::sCellRecord>
// Author:    cl
// DateTime:  2021/08/28
//************************************
QVector<CS57ChartSelection::sCellRecord> CS57ChartSelection::sortResults()
{
	QVector<sCellRecord> vecResults;
	for (int i = 0; i < m_vecSelectedMcovrs.size(); i++)
	{
		QString strCellName = m_vecSelectedMcovrs[i]->getCellName();
		double dCscl = m_vecSelectedMcovrs[i]->getCellCscl();

		sCellRecord record;
		record.strCellNum = strCellName.replace(".000","");
		record.strCellName = "";
		record.nCscl = dCscl;
		record.strDate = "";

		vecResults.push_back(record);
	}
	return vecResults;
}

