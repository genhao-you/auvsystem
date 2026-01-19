#include "documentmodel.h"
#include<QFile>
#include<QTextStream>
#include<QFileInfo>
#pragma execution_character_set("utf-8")
DocumentModel::DocumentModel(QObject *parent)
	: QObject(parent), m_bmodified(false)
{}

DocumentModel::~DocumentModel()
{}

QString DocumentModel::filePath() const
{
	return m_sfilePath;
}

QString DocumentModel::content() const
{
	return m_scontent;
}

bool DocumentModel::isModified() const
{
	return m_bmodified;
}
bool DocumentModel::load(const QString& filePath)
{
	//检查文件是否存在
	if (!QFile::exists(filePath))
	{
		emit errorOccurred(tr("文件不存在：%1").arg(filePath));
		return false;
	}
	//检查文件权限
	QFileInfo fileInfo(filePath);
	if (fileInfo.isReadable())
	{
		emit errorOccurred(tr("没有读写权限：%1").arg(filePath));
		return false;
	}
	//打开文件
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly))
	{
		emit errorOccurred(tr("无法打开文件：%1").arg(file.errorString()));
		return false;
	}
	QTextStream in(&file);
	m_scontent = in.readAll();
	file.close();

	m_sfilePath = filePath;
	m_bmodified = false;

	emit filePathChanged(m_sfilePath);
	emit contentChanged(m_scontent);
	emit modifiedChanged(m_bmodified);
	return true;
}
bool DocumentModel::save()
{	
	if (m_sfilePath.isEmpty())
	{
		return false;
	}
	return saveAs(m_sfilePath);
}

bool DocumentModel::saveAs(const QString& filePath)
{
	QFile file(filePath);
	if (!file.open(QIODevice::WriteOnly))
	{
		emit errorOccurred(tr("无法保存文件：%1").arg(file.errorString()));
		return false;
	}
	QTextStream out(&file);
	out << m_scontent;
	file.close();
	m_sfilePath = filePath;
	m_bmodified = false;

	emit filePathChanged(m_sfilePath);
	emit modifiedChanged(m_bmodified);
	return true;
}

void DocumentModel::setContent(const QString& content)
{
	if (m_scontent != content)
	{
		m_scontent = content;
		m_bmodified = true;
		emit contentChanged(m_scontent);
		emit modifiedChanged(m_bmodified);
	}
}

void DocumentModel::setFilePath(const QString& filePath)
{
	m_sfilePath = filePath;
}

void DocumentModel::clear()
{
	m_sfilePath = "";
	m_scontent = "";
	m_bmodified = false;
	emit filePathChanged(m_sfilePath);
	emit contentChanged(m_scontent);
	emit modifiedChanged(m_bmodified);
}
