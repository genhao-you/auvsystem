#pragma once
#include "doc_global.h"
#include "cs52lnst.h"
#include "cs52symb.h"
#include "cs52patt.h"
#include <QMap>

namespace Doc
{
	class DOC_EXPORT CS52SymbolData
	{
	public:
		CS52SymbolData();
		~CS52SymbolData();

		LNST* searchLnst(const QString& lnstName);
		SYMB* searchSymb(const QString& symbName);
		PATT* searchPatt(const QString& pattName);

		void getAllSymbName(QStringList& symbList);
		void getAllPattName(QStringList& pattList);
		void getAllLnstName(QStringList& lnstList);

		int getSymbLibSize() const;
		int getPattLibSize() const;
		int getLnstLibSize() const;

	private:
		void clear();
		bool readAsciiFile(const QString& fileName);
		bool parseVECD(SYMB* symb);
		bool parseVECD(PATT* patt);
		bool parseVECD(LNST* lnst);
		bool parseMap();
	private:
		QString					m_strSymbolDataPath;
		QMap<QString, LNST*>	m_mapLnst;
		QMap<QString, PATT*>	m_mapPatt;
		QMap<QString, SYMB*>	m_mapSymb;
	};
}

