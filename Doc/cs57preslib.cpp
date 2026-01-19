#include "stdafx.h"
#include "cs57preslib.h"
#include "cs52luttable.h"
#include "ccdcluttable.h"

using namespace Doc;
CS57PresLib::CS57PresLib()
	: m_pS57AttrExpectedValue(new CS57AttrExpectedValue())
	, m_pS57FeaAttrTab(new CS57FeaAttrTab())
	, m_pS57FeaObjTab(new CS57FeaObjTab())
	, m_pS52ColorTable(new CS52ColorTable())
	, m_pS52LutTable(new CS52LutTable())
	, m_pCDCLutTable(new CCDCLutTable())
	, m_pS52SymbData(new CS52SymbolData())
{}

CS57PresLib::~CS57PresLib()
{
	if (m_pS57AttrExpectedValue != nullptr)
		delete m_pS57AttrExpectedValue;
	m_pS57AttrExpectedValue = nullptr;

	if (m_pS57FeaAttrTab != nullptr)
		delete m_pS57FeaAttrTab;
	m_pS57FeaAttrTab = nullptr;

	if (m_pS57FeaObjTab != nullptr)
		delete m_pS57FeaObjTab;
	m_pS57FeaObjTab = nullptr;
	
	if (m_pS52ColorTable != nullptr)
		delete m_pS52ColorTable;
	m_pS52ColorTable = nullptr;

	if (m_pS52LutTable != nullptr)
		delete m_pS52LutTable;
	m_pS52LutTable = nullptr;

	if (m_pCDCLutTable != nullptr)
		delete m_pCDCLutTable;
	m_pCDCLutTable = nullptr;

	if (m_pS52SymbData != nullptr)
		delete m_pS52SymbData;
	m_pS52SymbData = nullptr;
}

SYMB* CS57PresLib::getSymb(const QString& symbName) const
{
	return m_pS52SymbData->searchSymb(symbName);
}

PATT* CS57PresLib::getPatt(const QString& pattName) const
{
	return m_pS52SymbData->searchPatt(pattName);
}

LNST* CS57PresLib::getLnst(const QString& lnstName) const
{
	return m_pS52SymbData->searchLnst(lnstName);
}

void CS57PresLib::getAllLnstName(QStringList& lstLnst)
{
	m_pS52SymbData->getAllLnstName(lstLnst);
}

void CS57PresLib::getAllSymbName(QStringList& lstSymb)
{
	m_pS52SymbData->getAllSymbName(lstSymb);
}

void CS57PresLib::getAllPattName(QStringList& lstPatt)
{
	m_pS52SymbData->getAllPattName(lstPatt);
}

//************************************
// Method:    getS52Color
// Brief:	  获取S52色表颜色
// Returns:   Doc::S52Color*
// Author:    cl
// DateTime:  2022/07/22
// Parameter: const QString & colorName
//************************************
S52Color* CS57PresLib::getS52Color(const QString& colorName) const
{
	return m_pS52ColorTable->serachColor(colorName);
}

//************************************
// Method:    getS52Color
// Brief:	  获取S52色表颜色
// Returns:   void
// Author:    cl
// DateTime:  2022/07/22
// Parameter: const QString & colorName
// Parameter: unsigned short & r
// Parameter: unsigned short & g
// Parameter: unsigned short & b
//************************************
void CS57PresLib::getS52Color(const QString& colorName, unsigned short& r, unsigned short& g, unsigned short& b) const
{
	m_pS52ColorTable->serachColor(colorName, r, g, b);
}

QVector<LUPT*> CS57PresLib::getLuptLst(int prim) const
{
	return m_pS52LutTable->getLuptLst(prim);
}

LUPT* CS57PresLib::serachLupt(const QString& objlName, const QStringList& lstAttrCombin, eLookupTabType tabType) const
{
	switch (tabType)
	{
	case PLAIN_BOUNDARIES:
	case SYMBOLIZED_BOUNDARIES:
	case SIMPLIFIED:
	case PAPER_CHART:
	case LINES:
		return m_pS52LutTable->serachLupt(objlName, lstAttrCombin, tabType);
		break;
	case CDC_P:
	case CDC_L:
	case CDC_A:
		return m_pCDCLutTable->serachLupt(objlName, lstAttrCombin, tabType);
		break;
	default:
		break;
	}
	return nullptr;
}

//************************************
// Method:    setColorMode
// Brief:	  设置颜色模式
// Returns:   void
// Author:    cl
// DateTime:  2022/07/22
// Parameter: XT::S52ColorMode mode
//************************************
void CS57PresLib::setColorMode(XT::S52ColorMode mode)
{
	m_pS52ColorTable->setColorMode(mode);
}

//************************************
// Method:    parseCS
// Brief:	  解析CS指令
// Returns:   bool
// Author:    cl
// DateTime:  2022/07/22
// Parameter: S52InstField & instField
// Parameter: std::string ruleString
//************************************
bool CS57PresLib::parseCS(S52InstField& instField, std::string ruleString)
{
	return m_pS52LutTable->parseCS(instField, ruleString);
}

//************************************
// Method:    getFeatureInfo
// Brief:	  通过物标缩写获取物标信息
// Returns:   Doc::S57Object*
// Author:    cl
// DateTime:  2022/07/22
// Parameter: const QString & featureAcronym
//************************************
S57Object* CS57PresLib::getFeatureInfo(const QString& featureAcronym) const
{
	return m_pS57FeaObjTab->getFeatureInfo(featureAcronym);
}

//************************************
// Method:    getFeatureInfo
// Brief:	  通过物标码获取物标信息
// Returns:   Doc::S57Object*
// Author:    cl
// DateTime:  2022/07/22
// Parameter: unsigned short featureCode
//************************************
S57Object* CS57PresLib::getFeatureInfo(unsigned short featureCode) const
{
	return m_pS57FeaObjTab->getFeatureInfo(featureCode);
}

//************************************
// Method:    getFeatureCode
// Brief:	  通过物标缩写获取物标码
// Returns:   unsigned short
// Author:    cl
// DateTime:  2022/07/22
// Parameter: const QString & featureAcronym
//************************************
unsigned short CS57PresLib::getFeatureCode(const QString& featureAcronym) const
{
	return m_pS57FeaObjTab->getFeatureCode(featureAcronym);
}

S57ExpectedValue* CS57PresLib::getS57AttrExpValueInfo(unsigned short& code, unsigned short value) const
{
	return m_pS57AttrExpectedValue->getS57AttrExpValueInfo(code, value);
}

bool CS57PresLib::getAttrExpInfo(unsigned short attrCode, unsigned short expValue, QString& cName, QString& eName) const
{
	return m_pS57AttrExpectedValue->getAttrExpInfo(attrCode, expValue, cName, eName);
}

//************************************
// Method:    getAttrCode
// Brief:	  通过物标属性缩写获取物标码
// Returns:   unsigned short
// Author:    cl
// DateTime:  2022/07/22
// Parameter: const QString & feaAttrAcronym
//************************************
unsigned short CS57PresLib::getAttrCode(const QString& feaAttrAcronym) const
{
	return m_pS57FeaAttrTab->getAttrCode(feaAttrAcronym);
}

//************************************
// Method:    getAttributeInfo
// Brief:	  通过获取物标属性缩写获取属性信息
// Returns:   Doc::S57AttributeValue*
// Author:    cl
// DateTime:  2022/07/22
// Parameter: const QString & feaAttrAcronym
//************************************
S57AttributeValue* CS57PresLib::getAttributeInfo(const QString& feaAttrAcronym) const
{
	return m_pS57FeaAttrTab->getAttributeInfo(feaAttrAcronym);
}

//************************************
// Method:    getAttributeInfo
// Brief:	  通过属性码获取属性信息
// Returns:   Doc::S57AttributeValue*
// Author:    cl
// DateTime:  2022/07/22
// Parameter: unsigned short attrCode
//************************************
S57AttributeValue* CS57PresLib::getAttributeInfo(unsigned short attrCode) const
{
	return m_pS57FeaAttrTab->getAttributeInfo(attrCode);
}




