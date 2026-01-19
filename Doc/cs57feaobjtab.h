#pragma once
#include "doc_global.h"
#include <QString>
#include <QVector>

namespace Doc
{
	struct S57Object
	{
		unsigned short nFeaObjectCode;	// 物标代码
		QString strFeaObjectAcronym;	// 物标缩写代码
		QString strFeaObjectMean;		// 物标含义
		QString strFeaObjectCname;		// 物标中文名		
	};
	//要素物标表类
	class CS57FeaObjTab
	{
	public:
		CS57FeaObjTab();
		~CS57FeaObjTab();

	public:
		//如果找到返回结构体指针，否知返回nullptr
		S57Object* getFeatureInfo(const QString& featureAcronym) const;

		//如果找到返回结构体指针，否知返回nullptr
		S57Object* getFeatureInfo(unsigned short featureCode) const;

		//通过物标缩写返回物标码
		//如果找到返回物标码，否知返回-1
		unsigned short getFeatureCode(const QString& featureAcronym) const;
	private:
		void readAsciiFile();
	private:
		QString m_strObjectPath;
		QVector<S57Object*> m_vecObject;
	};
}
