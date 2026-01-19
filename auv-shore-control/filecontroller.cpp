#include "filecontroller.h"
#include<QMainWindow>
#include<QFileDialog>
#include<QFile>
#include<QTextDocument>
#include<QMessageBox>
#include<QApplication>
#pragma execution_character_set("utf-8")
FileController::FileController(QMainWindow* mainwindow, DocumentModel* documentModel, QObject *parent)
	: QObject(parent),m_mainWindow(mainwindow),m_documentModel(documentModel),m_fileDialog(nullptr)
{}

FileController::~FileController()
{}
void  FileController::newFile()
{
	if (maybeSave())
	{
		//清空文档内容和路径
		m_documentModel->setFilePath("");
		m_documentModel->setContent("");
		m_mainWindow->setWindowTitle("未命名文档 - 应用程序");
	}
}
void FileController::openFile()
{

	if (!m_fileDialog)
	{
		m_fileDialog = new QFileDialog(m_mainWindow);
		m_fileDialog->setFileMode(QFileDialog::ExistingFile);
		m_fileDialog->setNameFilter("文本文件（*.txt);;所有文件(*)");
	}
	if (m_fileDialog->exec())
	{
		QString filePath = m_fileDialog->selectedFiles().first();
		if (!filePath.isEmpty())
		{
			if (maybeSave())
			{
				if (m_documentModel->load(filePath))
				{
					m_mainWindow->setWindowTitle(QString("%1 - 应用程序").arg(filePath));
				}
				else
				{
					QMessageBox::critical(m_mainWindow, "错误", "文件打开失败");
				}
			}
		}
	}
}

void FileController::saveFile()
{
	if (m_documentModel->filePath().isEmpty())
	{
		saveFileAs();
	}
	else
	{
		if (m_documentModel->save())
		{
			
		}
		else
		{
			QMessageBox::critical(m_mainWindow, "错误", "文件保存失败");
		}
	}
}

void FileController::saveFileAs()
{
	if (!m_fileDialog)
	{
		m_fileDialog = new QFileDialog(m_mainWindow);
		m_fileDialog->setFileMode(QFileDialog::AnyFile);
		m_fileDialog->setAcceptMode(QFileDialog::AcceptSave);
		m_fileDialog->setNameFilter("文本文件（*.txt);;所有文件(*)");

	}
	if (m_fileDialog->exec())
	{
		QString filePath = m_fileDialog->selectedFiles().first();
		if (!filePath.isEmpty())
		{
			if (m_documentModel->saveAs(filePath))
			{
				m_mainWindow->setWindowTitle(QString("%1 - 应用程序").arg(filePath));
			}
			else
			{
				QMessageBox::critical(m_mainWindow, "错误", "文件保存失败");
			}
		}
	}
}

void FileController::closeFile()
{
	if (maybeSave())
	{
		m_documentModel->setFilePath("");
		m_documentModel->setContent("");
		m_mainWindow->setWindowTitle("未命名文档 - 应用程序");
	}
}

bool FileController::maybeSave()
{
	if (!m_documentModel->isModified())
		return true;

	const QMessageBox::StandardButton ret = QMessageBox::warning(m_mainWindow, QApplication::applicationName(), "文档已修改，是否保存？", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
	switch (ret)
	{
	case QMessageBox::Save:
		return m_documentModel->save();
	case QMessageBox::Cancel:
		return false;
	default:
		return true;
	}
}
