#include "stdafx.h"
#include "cs57feaobjtab.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QApplication>

using namespace Doc;
#pragma execution_character_set("utf-8")
CS57FeaObjTab::CS57FeaObjTab()
	: m_strObjectPath(QApplication::applicationDirPath() + "/s57Data/s57object.txt")
{
	readAsciiFile();
}

CS57FeaObjTab::~CS57FeaObjTab()
{
	qDeleteAll(m_vecObject);
	m_vecObject.clear();
}

//************************************
// Method:    readAsciiFile
// Brief:	  读取AscII属性文件
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57FeaObjTab::readAsciiFile()
{
	QFile file(m_strObjectPath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	QTextStream in(&file);
	QString line = in.readLine();
	if (line.isNull())
	{
		//QMessageBox::information(nullptr, "提示", "s57object.txt文件读取失败，即将返回。");
		return;
	}

	QStringList vecValues = line.split('\t');
	if (vecValues.size() == 0 ||
		vecValues.size() != 4)
	{
		//QMessageBox::information(nullptr, "提示", "s57object.txt文件读取失败，即将返回。");
		return;
	}

	S57Object* pObject = new S57Object();
	pObject->strFeaObjectAcronym = vecValues[0];
	pObject->nFeaObjectCode = vecValues[1].toUShort();
	pObject->strFeaObjectMean = vecValues[2];
	pObject->strFeaObjectCname = vecValues[3];
	m_vecObject.push_back(pObject);

	while (!line.isNull())
	{
		line = in.readLine();
		if (line.isNull())
			break;

		QStringList vecValues = line.split('\t');
		if (vecValues.size() == 0 ||
			vecValues.size() != 4)
		{
			//QMessageBox::information(nullptr, "提示", "s57attributes.txt文件读取失败，即将返回。");
			return;
		}

		S57Object* pObject = new S57Object();
		pObject->strFeaObjectAcronym = vecValues[0];
		pObject->nFeaObjectCode = vecValues[1].toUShort();
		pObject->strFeaObjectMean = vecValues[2];
		pObject->strFeaObjectCname = vecValues[3];
		m_vecObject.push_back(pObject);
	}
	file.close();
}

//************************************
// Method:    getFeatureInfo
// Brief:	  通过物标缩写获取物标信息
// Returns:   Doc::S57Object*
// Author:    cl
// DateTime:  2022/07/21
// Parameter: const QString & featureAcronym
//************************************
S57Object* CS57FeaObjTab::getFeatureInfo(const QString& featureAcronym) const
{
	for (int i = 0; i < m_vecObject.size(); i++)
	{
		if (featureAcronym == m_vecObject[i]->strFeaObjectAcronym)
			return m_vecObject[i];
	}
	return nullptr;
}

//************************************
// Method:    getFeatureInfo
// Brief:	  通过物标码获取物标信息
// Returns:   Doc::S57Object*
// Author:    cl
// DateTime:  2022/07/21
// Parameter: unsigned short featureCode
//************************************
S57Object* CS57FeaObjTab::getFeatureInfo(unsigned short featureCode) const
{
	for (int i = 0; i < m_vecObject.size(); i++)
	{
		if (featureCode == m_vecObject[i]->nFeaObjectCode)
			return m_vecObject[i];
	}
	return nullptr;
}

//************************************
// Method:    getFeatureCode
// Brief:	  通过物标缩写获取物标码
// Returns:   unsigned short
// Author:    cl
// DateTime:  2022/07/21
// Parameter: const QString & featureAcronym
//************************************
unsigned short CS57FeaObjTab::getFeatureCode(const QString& featureAcronym) const
{
	for (int i = 0; i < m_vecObject.size(); i++)
	{
		if (featureAcronym == m_vecObject[i]->strFeaObjectAcronym)
			return m_vecObject[i]->nFeaObjectCode;
	}
	return -1;
}
