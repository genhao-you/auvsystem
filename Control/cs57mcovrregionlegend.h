#pragma once

#include <QDialog>
#include "ui_cs57mcovrregionlegend.h"

class CS57McovrRegionLegend : public QDialog
{
	Q_OBJECT

public:
	CS57McovrRegionLegend(QWidget *parent = Q_NULLPTR);
	~CS57McovrRegionLegend();

	int getWidth(){return _size.width();}
	int getHight(){return _size.height();}

protected:
	void paintEvent(QPaintEvent *event);
private:
	Ui::CS57McovrRegionLegend ui;
	QSize _size;
};
