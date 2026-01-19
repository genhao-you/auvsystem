#pragma once
#include <QString>
#include <QVariant>

//系统配置文件
class SysIni
{
public:
	SysIni();
	~SysIni();

public:
	void		setValue(const QString& group, const QString& key, const QString& value);
	QVariant	value(const QString& group, const QString& key);

private:
	QString		m_strIniFile;
};
