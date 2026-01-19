#pragma once
#include "doc_global.h"
#include "xt_global.h"
#include <QString>
#include <QMap>
namespace Doc
{
	struct S52Color
	{
		QString			colorName;
		unsigned short	r = 0;
		unsigned short	g = 0;
		unsigned short	b = 0;
	};
	class DOC_EXPORT CS52ColorTable
	{
	public:
		CS52ColorTable();
		~CS52ColorTable();
	public:
		S52Color*		serachColor(QString colorName, XT::S52ColorMode colorMode);
		S52Color*		serachColor(QString colorName);
		void			serachColor(QString colorName, unsigned short& r, unsigned short& g, unsigned short& b);
		void			setColorMode(XT::S52ColorMode colorMode);

	private:
		bool			readAsciiFile(const QString& fileName);
		void			clear();
	private:
		QString						m_strColorTabPath;
		XT::S52ColorMode			m_eColorMode;
		QMap<QString, S52Color*>	m_mapDay;
		QMap<QString, S52Color*>	m_mapDwb;
		QMap<QString, S52Color*>	m_mapDbb;
		QMap<QString, S52Color*>	m_mapDusk;
		QMap<QString, S52Color*>	m_mapNight;
	};
}

