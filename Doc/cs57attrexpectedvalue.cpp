#include "stdafx.h"
#include "cs57attrexpectedvalue.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QApplication>

using namespace Doc;
#pragma execution_character_set("utf-8")
CS57AttrExpectedValue::CS57AttrExpectedValue()
	: m_strExpectedPath(QApplication::applicationDirPath() + "/s57Data/s57expected.txt")
{
	readAsciiFile();
}

CS57AttrExpectedValue::~CS57AttrExpectedValue()
{
	qDeleteAll(m_vecExpectedValue);
	m_vecExpectedValue.clear();
}

//************************************
// Method:    readAsciiFile
// Brief:	  读取AscII属性文件
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57AttrExpectedValue::readAsciiFile()
{
	QFile file(m_strExpectedPath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	QTextStream in(&file);
	QString line = in.readLine();
	if (line.isNull())
	{
		//QMessageBox::information(nullptr, "提示", "s57expected.txt文件读取失败，即将返回。");
		return;
	}

	QStringList vecValues = line.split('\t');
	if (vecValues.size() == 0 ||
		vecValues.size() != 5)
	{
		//QMessageBox::information(nullptr, "提示", "s57expected.txt文件读取失败，即将返回。");
		return;
	}

	S57ExpectedValue* pAttributeValue = new S57ExpectedValue();
	pAttributeValue->nCombineAttrCodeValue = vecValues[0].toLong();
	pAttributeValue->nFeaAttrCode = vecValues[1].toUShort();
	pAttributeValue->nExpectedValue = vecValues[2].toUShort();
	pAttributeValue->strCName = vecValues[3];
	pAttributeValue->strEName = vecValues[4];
	m_vecExpectedValue.push_back(pAttributeValue);

	while (!line.isNull())
	{
		line = in.readLine();
		if (line.isNull())
			break;

		QStringList vecValues = line.split('\t');
		if (vecValues.size() == 0 ||
			vecValues.size() != 5)
		{
			//QMessageBox::information(nullptr, "提示", "s57expected.txt文件读取失败，即将返回。");
			return;
		}

		S57ExpectedValue* pAttributeValue = new S57ExpectedValue();
		pAttributeValue->nCombineAttrCodeValue = vecValues[0].toLong();
		pAttributeValue->nFeaAttrCode = vecValues[1].toUShort();
		pAttributeValue->nExpectedValue = vecValues[2].toUShort();
		pAttributeValue->strCName = vecValues[3];
		pAttributeValue->strEName = vecValues[4];
		m_vecExpectedValue.push_back(pAttributeValue);
	}
	file.close();
}

//************************************
// Method:    getS57AttrExpValueInfo
// Brief:	  通过要素属性码和期望值获取期望值结构体指针
// Returns:   Doc::S57ExpectedValue*
// Author:    cl
// DateTime:  2022/07/21
// Parameter: unsigned short & code
// Parameter: unsigned short value
//************************************
S57ExpectedValue* CS57AttrExpectedValue::getS57AttrExpValueInfo(unsigned short& code, unsigned short value) const
{
	for (int i = 0; i < m_vecExpectedValue.size(); i++)
	{
		if (code == m_vecExpectedValue[i]->nFeaAttrCode && 
			value == m_vecExpectedValue[i]->nExpectedValue)
			return m_vecExpectedValue[i];
	}
	return nullptr;
}

bool CS57AttrExpectedValue::getAttrExpInfo(unsigned short attrCode, unsigned short expValue, QString& cName, QString& eName) const
{
	unsigned int nCombin;
	nCombin = attrCode * 1000 + expValue;
	S57ExpectedValue * expInfo = getAttrExp(nCombin);
	if (expInfo != nullptr)
	{
		cName = expInfo->strCName;
		eName = expInfo->strEName;

		return true;
	}
	return false;
}

S57ExpectedValue* CS57AttrExpectedValue::getAttrExp(unsigned int& mCombin) const
{
	int startNum = 0;
	int endNum = m_vecExpectedValue.size();
	int mid = 0;
	while (startNum <= endNum)
	{
		mid = (startNum + endNum) / 2;
		if (mCombin < m_vecExpectedValue[mid]->nCombineAttrCodeValue)
			endNum = mid - 1;
		else if (mCombin == m_vecExpectedValue[mid]->nCombineAttrCodeValue)
			return m_vecExpectedValue[mid];
		else
			startNum = mid + 1;
	}
	return nullptr;
}
