#pragma once

#include"itaskeditor.h"
#include "ui_escapetaskeditor.h"

class EscapeTaskEditor : public ITaskEditor
{
	Q_OBJECT

public:
	EscapeTaskEditor(QWidget *parent = nullptr);
	~EscapeTaskEditor();
	void setTask(Task* task) override;
	// 从UI收集数据并更新任务对象
	Task* getTask() const override;
	void clear() override;
private:
	QComboBox* m_comboBoxSpeedUnit;//速度单位
	Task* m_currentTask = nullptr;
private:
	Ui::EscapeTaskEditorClass ui;
};
