#pragma once
#include <QString>
#include <QStringList>

class HistoryIni
{
public:
	HistoryIni();
	~HistoryIni();

	QStringList getDirs();
	void addDir(const QString& dir);
	void deleteDir(const QString& dir);
	void clearAll();

private:
	QString m_strHistoryIni;
	QStringList m_lstDirs;
};
