#include "stdafx.h"
#include "cs57feaattrtab.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QMessageBox>
#include <QApplication>

using namespace Doc;
#pragma execution_character_set("utf-8")
CS57FeaAttrTab::CS57FeaAttrTab()
	: m_strAttributesPath(QApplication::applicationDirPath() + "/s57Data/s57attributes.txt")
{
	readAsciiFile();
}

CS57FeaAttrTab::~CS57FeaAttrTab()
{
	qDeleteAll(m_vecAttributeValue);
	m_vecAttributeValue.clear();
}

//************************************
// Method:    getAttrCode
// Brief:	  通过属性缩写获取属性码
// Returns:   unsigned short
// Author:    cl
// DateTime:  2022/07/21
// Parameter: const QString & feaAttrAcronym
//************************************
unsigned short CS57FeaAttrTab::getAttrCode(const QString& feaAttrAcronym) const
{
	for (int i = 0; i < m_vecAttributeValue.size(); i++)
	{
		if (feaAttrAcronym == m_vecAttributeValue[i]->strFeaAttrAcronym)
			return m_vecAttributeValue[i]->nFeaAttrCode;
	}
	return -1;
}
//************************************
// Method:    getAttributeInfo
// Brief:	  通过属性缩写获取属性信息
// Returns:   Doc::S57AttributeValue*
// Author:    cl
// DateTime:  2022/07/21
// Parameter: const QString & feaAttrAcronym
//************************************
S57AttributeValue* CS57FeaAttrTab::getAttributeInfo(const QString& feaAttrAcronym) const
{
	for (int i = 0; i < m_vecAttributeValue.size(); i++)
	{
		if (feaAttrAcronym == m_vecAttributeValue[i]->strFeaAttrAcronym)
			return m_vecAttributeValue[i];
	}
	return nullptr;
}

//************************************
// Method:    getAttributeInfo
// Brief:	  通过属性码获取属性信息
// Returns:   Doc::S57AttributeValue*
// Author:    cl
// DateTime:  2022/07/21
// Parameter: unsigned short attrCode
//************************************
S57AttributeValue* CS57FeaAttrTab::getAttributeInfo(unsigned short attrCode) const
{
	for (int i = 0; i < m_vecAttributeValue.size(); i++)
	{
		if (attrCode == m_vecAttributeValue[i]->nFeaAttrCode)
			return m_vecAttributeValue[i];
	}
	return nullptr;
}

//************************************
// Method:    readAsciiFile
// Brief:	  读取AscII属性文件
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57FeaAttrTab::readAsciiFile()
{
	QFile file(m_strAttributesPath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	QTextStream in(&file);
	QString line = in.readLine();
	if (line.isNull())
	{
		//QMessageBox::information(nullptr, "提示", "s57attributes.txt文件读取失败，即将返回。");
		return;
	}

	QStringList vecValues = line.split('\t');
	if (vecValues.size() == 0 ||
		vecValues.size() != 5)
	{
		//QMessageBox::information(nullptr, "提示", "s57attributes.txt文件读取失败，即将返回。");
		return;
	}

	S57AttributeValue* pAttributeValue = new S57AttributeValue();
	pAttributeValue->nFeaAttrCode = vecValues[0].toUShort();
	pAttributeValue->strFeaAttrAcronym = vecValues[1];
	pAttributeValue->strCName = vecValues[2];
	pAttributeValue->strEName = vecValues[3];
	char cval = vecValues[4][0].toLatin1();
	switch (cval)
	{
	case 'E':
		pAttributeValue->eAttrType = E;
		break;
	case 'L':
		pAttributeValue->eAttrType = L;
		break;
	case 'F':
		pAttributeValue->eAttrType = F;
		break;
	case 'I':
		pAttributeValue->eAttrType = I;
		break;
	case 'A':
		pAttributeValue->eAttrType = A;
		break;
	case 'S':
		pAttributeValue->eAttrType = S;
		break;
	default:
		pAttributeValue->eAttrType = S;
	}
	m_vecAttributeValue.push_back(pAttributeValue);

	while (!line.isNull())
	{
		line = in.readLine();
		if(line.isNull())
			break;

		QStringList vecValues = line.split('\t');
		if (vecValues.size() == 0 ||
			vecValues.size() != 5)
		{
			//QMessageBox::information(nullptr, "提示", "s57attributes.txt文件读取失败，即将返回。");
			return;
		}

		S57AttributeValue* pAttributeValue = new S57AttributeValue();
		pAttributeValue->nFeaAttrCode = vecValues[0].toUShort();
		pAttributeValue->strFeaAttrAcronym = vecValues[1];
		pAttributeValue->strCName = vecValues[2];
		pAttributeValue->strEName = vecValues[3];
		char cval = vecValues[4][0].toLatin1();
		switch (cval)
		{
		case 'E':
			pAttributeValue->eAttrType = E;
			break;
		case 'L':
			pAttributeValue->eAttrType = L;
			break;
		case 'F':
			pAttributeValue->eAttrType = F;
			break;
		case 'I':
			pAttributeValue->eAttrType = I;
			break;
		case 'A':
			pAttributeValue->eAttrType = A;
			break;
		case 'S':
			pAttributeValue->eAttrType = S;
			break;
		default:
			pAttributeValue->eAttrType = S;
		}
		m_vecAttributeValue.push_back(pAttributeValue);
	}
	file.close();
}


