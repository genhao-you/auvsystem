#pragma once
#include "doc_global.h"
#include <QString>
#include <QVector>

namespace Doc 
{
	struct S57ExpectedValue
	{
		unsigned int nCombineAttrCodeValue;		// CombineAttrCodeValue = FeaAttrCode * 100 + ExpectedValue 
		unsigned short nFeaAttrCode;
		unsigned short nExpectedValue;
		QString strCName;
		QString strEName;
	};
	class DOC_EXPORT CS57AttrExpectedValue
	{
	public:
		CS57AttrExpectedValue();
		~CS57AttrExpectedValue();

		//通过物标属性码和期望值获取期望值结构体指针
		//成功返回有效指针，失败返回nullptr
		S57ExpectedValue* getS57AttrExpValueInfo(unsigned short& code, unsigned short value) const;

		//通过物标属性码和期望值返回物标中英文名称
		//成功返回true，失败返回false
		bool getAttrExpInfo(unsigned short attrCode, unsigned short expValue, QString& cName, QString& eName)const;

	private:
		//通过组合码获取期望值结构体指针
		//成功返回有效指针，失败返回nullptr
		S57ExpectedValue* getAttrExp(unsigned int& mCombin) const;

	private:
		//读配置文件
		void readAsciiFile();

	private:
		QString m_strExpectedPath;
		QVector<S57ExpectedValue*> m_vecExpectedValue;
	};

}
