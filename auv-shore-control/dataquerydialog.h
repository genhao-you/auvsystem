#pragma once

#include <QDialog>
#include "ui_dataquerydialog.h"

class DataQueryDialog : public QDialog
{
	Q_OBJECT

public:
	DataQueryDialog(QWidget *parent = nullptr);
	~DataQueryDialog();
private slots:
	void on_dataQueryTabWidget_currentChanged(int index);
private:
	Ui::DataQueryDialogClass ui;
};
