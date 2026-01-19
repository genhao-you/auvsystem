#include "dataquerydialog.h"

DataQueryDialog::DataQueryDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	connect(ui.tabWidget_dataList, &QTabWidget::currentChanged, this, &DataQueryDialog::on_dataQueryTabWidget_currentChanged);
}

DataQueryDialog::~DataQueryDialog()
{}

void DataQueryDialog::on_dataQueryTabWidget_currentChanged(int index)
{
	if (index == 0)
	{
		ui.stackedWidget_dataQuery->setCurrentIndex(0);
	}
	else
	{
		ui.stackedWidget_dataQuery->setCurrentIndex(1);
	}
}