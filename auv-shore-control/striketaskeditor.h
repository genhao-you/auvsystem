#pragma once

#include"itaskeditor.h"
#include "ui_striketaskeditor.h"

class StrikeTaskEditor : public ITaskEditor
{
	Q_OBJECT

public:
	StrikeTaskEditor(QWidget *parent = nullptr);
	~StrikeTaskEditor();
	void setTask(Task* task) override;
	// 从UI收集数据并更新任务对象
	Task* getTask() const override;
	void clear() override;
private:
	Ui::StrikeTaskEditorClass ui;
	Task* m_currentTask = nullptr;
};
