#pragma once

#include"itaskeditor.h"
#include "ui_shorebasedstriketaskeditor.h"

class ShoreBasedStrikeTaskEditor : public ITaskEditor
{
	Q_OBJECT

public:
	ShoreBasedStrikeTaskEditor(QWidget *parent = nullptr);
	~ShoreBasedStrikeTaskEditor();
	void setTask(Task* task) override;
	// 从UI收集数据并更新任务对象
	Task* getTask() const override;
	void clear() override;
private:
	Ui::ShoreBasedStrikeTaskEditorClass ui;
	Task* m_currentTask = nullptr;
};
