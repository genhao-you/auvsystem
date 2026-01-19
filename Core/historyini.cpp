#include "historyini.h"
#include "xt_define.h"
#include <QSettings>
#include <QDir>
#include <QApplication>

HistoryIni::HistoryIni()
	: m_strHistoryIni(QApplication::applicationDirPath() + "/s57Config/history.ini")
{
	for (int i = 0; i < HistoryMaxNum; i++)
	{
		QString strDir = QString("HistoryDir/Dir_%1").arg(i, 2, 10, QLatin1Char('0'));
		m_lstDirs << strDir;
	}
}

HistoryIni::~HistoryIni()
{}

//************************************
// Method:    getDirs
// Brief:	  获取配置中所有存在目录
// Returns:   QT_NAMESPACE::QStringList
// Author:    cl
// DateTime:  2021/09/03
//************************************
QStringList HistoryIni::getDirs()
{
	QStringList lstDirs;
	QSettings setting(m_strHistoryIni, QSettings::IniFormat);
	for (int i = 0; i < HistoryMaxNum; i++)
	{
		QString dirPath = setting.value(m_lstDirs[i]).toString();
		QDir dir(dirPath);
		bool exist = dir.exists();
		if (dirPath != "" && exist)
		{
			lstDirs << dirPath;
		}
		else if (dirPath != "" && !exist)
		{
			deleteDir(dirPath);
		}
	}

	return lstDirs;
}

//************************************
// Method:    addDir
// Brief:	  添加目录到配置文件
// Returns:   void
// Author:    cl
// DateTime:  2021/09/03
// Parameter: const QString & dir
//************************************
void HistoryIni::addDir(const QString& dir)
{
	QString strDir = dir;
	//判断是否已经存在，存在返回，不存在添加
	bool hasExists = false;
	QSettings setting(m_strHistoryIni, QSettings::IniFormat);
	for (int i = 0; i < HistoryMaxNum; i++)
	{
		QString value = setting.value(m_lstDirs[i]).toString();
		if(QString::compare(value, strDir, Qt::CaseInsensitive) == 0)
		{
			hasExists = true;
			return;
		}
	}
	if (!hasExists)
	{
		for (int i = 0; i < HistoryMaxNum; i++)
		{
			QString value = setting.value(m_lstDirs[i]).toString();
			if (value.isEmpty())
			{
				setting.setValue(m_lstDirs[i], strDir);
				break;
			}
		}
	}
}

//************************************
// Method:    deleteDir
// Brief:	  删除单个目录
// Returns:   void
// Author:    cl
// DateTime:  2021/09/03
// Parameter: const QString & dir
//************************************
void HistoryIni::deleteDir(const QString& dir)
{
	QSettings setting(m_strHistoryIni, QSettings::IniFormat);
	for (int i = 0; i < HistoryMaxNum; i++)
	{
		QString value = setting.value(m_lstDirs[i]).toString();
		if (value == dir)
		{
			setting.setValue(m_lstDirs[i], "");
			break;
		}
	}
}

//************************************
// Method:    clearAll
// Brief:	  清理全部
// Returns:   void
// Author:    cl
// DateTime:  2021/09/03
//************************************
void HistoryIni::clearAll()
{
	QSettings setting(m_strHistoryIni, QSettings::IniFormat);
	for (int i = 0; i < HistoryMaxNum; i++)
	{
		setting.setValue(m_lstDirs[i], "");
	}
}
