#include "stdafx.h"
#include "cs57cellbase.h"

using namespace Doc;
CS57CellBase::CS57CellBase()
	: m_pRecDsgi(nullptr)
	, m_pRecDsgr(nullptr)
{}

CS57CellBase::~CS57CellBase()
{}

//************************************
// Method:    getCellName
// Brief:	  获取图幅名称
// Returns:   QString
// Author:    cl
// DateTime:  2022/07/21
//************************************
QString CS57CellBase::getCellName() const
{

	return m_pRecDsgi->m_Dsid.dsnm;
}

//************************************
// Method:    getCellEdtn
// Brief:	  获取图幅Edtn
// Returns:   QString
// Author:    cl
// DateTime:  2022/07/21
//************************************
QString CS57CellBase::getCellEdtn() const
{
	return m_pRecDsgi->m_Dsid.edtn;
}

//************************************
// Method:    getCellUpdn
// Brief:	  获取图幅Updn
// Returns:   QString
// Author:    cl
// DateTime:  2022/07/21
//************************************
QString CS57CellBase::getCellUpdn() const
{
	return m_pRecDsgi->m_Dsid.updn;
}

//************************************
// Method:    getCellCscl
// Brief:	  获取图幅Cscl
// Returns:   double
// Author:    cl
// DateTime:  2022/07/21
//************************************
double CS57CellBase::getCellCscl() const
{
	return m_pRecDsgr->m_Dspm.cscl;
}

//************************************
// Method:    getCellComf
// Brief:	  获取图幅Comf
// Returns:   double
// Author:    cl
// DateTime:  2022/07/21
//************************************
double CS57CellBase::getCellComf() const
{
	return m_pRecDsgr->m_Dspm.comf;
}
