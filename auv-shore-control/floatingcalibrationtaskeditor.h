#pragma once

#include"itaskeditor.h"
#include "ui_floatingcalibrationtaskeditor.h"

class FloatingCalibrationTaskEditor : public ITaskEditor
{
	Q_OBJECT

public:
	FloatingCalibrationTaskEditor(QWidget *parent = nullptr);
	~FloatingCalibrationTaskEditor();
	void setTask(Task* task) override;
	// 从UI收集数据并更新任务对象
	Task* getTask() const override;
	void clear() override;
private:
	Ui::FloatingCalibrationTaskEditorClass ui;
	Task* m_currentTask = nullptr;
};
