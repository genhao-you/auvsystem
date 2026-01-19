#pragma once

#include <QDialog>
#include "ui_sailrestrictiondialog.h"

class SailRestrictionDialog : public QDialog
{
	Q_OBJECT

public:
	SailRestrictionDialog(QWidget *parent = nullptr);
	~SailRestrictionDialog();

private:
	Ui::SailRestrictionDialogClass ui;
};
