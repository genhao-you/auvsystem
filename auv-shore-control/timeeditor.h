#pragma once

#include <QWidget>
#include "ui_timeeditor.h"

class TimeEditor : public QWidget
{
	Q_OBJECT

public:
	TimeEditor(QWidget *parent = nullptr);
	~TimeEditor();

private:
	Ui::TimeEditorClass ui;
};
