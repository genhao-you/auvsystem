#pragma once
#include "doc_global.h"
#include <QVector>
#include <QStringList>
#include "cs57preslib.h"

namespace Doc
{
	class DOC_EXPORT CS52LutTable
	{
	public:
		CS52LutTable();
		~CS52LutTable();

		LUPT*			serachLupt(const QString& objlName, const QStringList& lstAttrCombin, eLookupTabType tabType);
		QVector<LUPT*>	getLuptLst(int prim) const;
		bool			parseCS(S52InstField& instField, std::string ruleString);
	private:
		bool			readAsciiFile(const QString& fileName);
		void			clear();
		void			clear(QVector<LUPT*>& vLupt);
		LUPT*			serachLupt(const QString& objlName, const QStringList& lstAttrCombin, QVector<LUPT*>& vLupt);

	private:
		QString			m_strLutTabPath;
	private:
		QVector<LUPT*>	m_vecLutPlainBoundaries;
		QVector<LUPT*>	m_vecLutSimplifiedBoundaries;
		QVector<LUPT*>	m_vecLutSimplified;
		QVector<LUPT*>	m_vecLutPaperChart;
		QVector<LUPT*>	m_vecLutLines;
	};
}

