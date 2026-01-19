#pragma once

#include <QDialog>
#include "ui_unitssettingdialog.h"

class UnitsSettingDialog : public QDialog
{
	Q_OBJECT

public:
	UnitsSettingDialog(QWidget *parent = nullptr);
	~UnitsSettingDialog();

private:
	Ui::UnitsSettingDialogClass ui;
};
