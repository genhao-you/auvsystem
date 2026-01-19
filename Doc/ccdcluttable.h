#pragma once
#include <QString>
#include "cs52luttable.h"
namespace Doc
{
	class CCDCLutTable
	{
	public:
		CCDCLutTable();
		~CCDCLutTable();
		enum SymbolType
		{
			CDC_P,
			CDC_L,
			CDC_A
		};

	public:
		LUPT* serachLupt(const QString& objlName, const QStringList& lstAttrCombin, eLookupTabType tabType);

	private:
		void readLut(QVector<LUPT*>& vecLut, const QString& strLutPath,SymbolType type);
		void parseInst(S52InstField& instField, QString instString);
		LUPT* serachLupt(const QString& objlName, const QStringList& lstAttrCombin, QVector<LUPT*>& vLupt);
	private:
		QString m_strLutPointPath;
		QString m_strLutLinePath;
		QString m_strLutAreaPath;

		QVector<LUPT*> m_vecCDCPointLut;
		QVector<LUPT*> m_vecCDCLineLut;
		QVector<LUPT*> m_vecCDCAreaLut;
	};

}
