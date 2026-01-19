#pragma once

#include <QDialog>
#include "ui_missionsettingdialog.h"

class MissionSettingDialog : public QDialog
{
	Q_OBJECT

public:
	MissionSettingDialog(QWidget *parent = nullptr);
	~MissionSettingDialog();

private:
	Ui::MissionSettingDialogClass ui;
};
