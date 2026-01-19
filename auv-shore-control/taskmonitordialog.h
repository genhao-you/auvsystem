#pragma once

#include <QDialog>
#include "ui_taskmonitordialog.h"

class TaskMonitorDialog : public QDialog
{
	Q_OBJECT

public:
	TaskMonitorDialog(QWidget *parent = nullptr);
	~TaskMonitorDialog();

private:
	Ui::TaskMonitorDialogClass ui;
};
