#include "stdafx.h"
#include "sysIni.h"
#include <QSettings>

SysIni::SysIni()
	: m_strIniFile("s57Config\\sys.ini")
{}

SysIni::~SysIni()
{}

void SysIni::setValue(const QString& group, const QString& key, const QString& value)
{
	QSettings setting(m_strIniFile, QSettings::IniFormat);

	setting.beginGroup(group);
	setting.setValue(key, value);
	setting.endGroup();
}

QVariant SysIni::value(const QString& group, const QString& key)
{
	QSettings setting(m_strIniFile, QSettings::IniFormat);
	return setting.value(group + "/" + key);
}