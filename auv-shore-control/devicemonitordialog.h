#pragma once

#include <QDialog>
#include "ui_devicemonitordialog.h"

class DeviceMonitorDialog : public QDialog
{
	Q_OBJECT

public:
	DeviceMonitorDialog(QWidget *parent = nullptr);
	~DeviceMonitorDialog();

private:
	Ui::DeviceMonitorDialogClass ui;
};
