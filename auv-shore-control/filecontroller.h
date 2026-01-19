#pragma once

#include <QObject>
#include"documentmodel.h"
class QMainWindow;
class QFileDialog;
class FileController  : public QObject
{
	Q_OBJECT

public:
	explicit FileController(QMainWindow* mainwindow,DocumentModel * documentModel, QObject *parent = nullptr);
	~FileController();

public slots:
	void newFile();
	void openFile();
	void saveFile();
	void saveFileAs();
	void closeFile();
	bool maybeSave();
private:
	QMainWindow* m_mainWindow;
	DocumentModel* m_documentModel;
	QFileDialog* m_fileDialog;


};
