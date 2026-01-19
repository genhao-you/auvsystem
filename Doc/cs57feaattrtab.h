#pragma once
#include "doc_global.h"
#include <QString>
#include <QVector>

namespace Doc
{
	enum CS57AttrType
	{
		E = 1,	// enumerated 预期输入是从预定义属性值列表中选择的数字。 必须选择一个值。 此类型的缩写是“ E”。
		L,		// list 预期的输入是一个从一个预定义属性值列表中选择的一个或多个数字的列表。 如果使用多个值，则通常必须用逗号分隔，但在特殊情况下，可以使用斜杠（“ /”）。 此类型的缩写是“ L”。
		F,		// float 预期的输入是具有定义范围，分辨率，单位和格式的浮点数值。 此类型的缩写是“ F”。
		I,		// integer 预期的输入是具有定义范围，单位和格式的整数值。 此类型的缩写是“I”。
		A,		// coded string 预期的输入是预定义格式的ASCII字符字符串。 信息是根据已定义的编码系统进行编码的，例如：国籍将由ISO 3166“国家名称表示代码”指定的两个字符字段进行编码。 加拿大=>“ CA”（请参阅S-57附录A附录A）。 此类型的缩写是“ A”。
		S		// free text 预期的输入是自由格式的字母数字字符串。 它可以是指向文本或图形文件的文件名。 此类型的缩写是“ S”。
	};
	struct S57AttributeValue
	{
		unsigned short nFeaAttrCode;
		QString strFeaAttrAcronym;
		QString strCName;
		QString strEName;
		CS57AttrType eAttrType;
	};

	class DOC_EXPORT CS57FeaAttrTab
	{
	public:
		CS57FeaAttrTab();
		~CS57FeaAttrTab();

	public:
		//通过物标属性缩写获取物标属性编码
		//成功返回物标属性编码，失败返回-1
		unsigned short getAttrCode(const QString& feaAttrAcronym) const;

		//通过物标属性缩写获取物标属性结构体指针
		//成功返回有效指针，失败返回nullptr
		S57AttributeValue* getAttributeInfo(const QString& feaAttrAcronym) const;

		//通过物标属性编码获取物标属性结构体指针
		//成功返回有效指针，失败返回nullptr
		S57AttributeValue* getAttributeInfo(unsigned short attrCode) const;
	private:
		void readAsciiFile();
	private:
		QString m_strAttributesPath;
		QVector<S57AttributeValue*> m_vecAttributeValue;
	};
}
