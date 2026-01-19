#pragma once

#include"itaskeditor.h"
#include "ui_recoverytaskeditor.h"

class RecoveryTaskEditor : public ITaskEditor
{
	Q_OBJECT

public:
	RecoveryTaskEditor(QWidget *parent = nullptr);
	~RecoveryTaskEditor();
	void setTask(Task* task) override;
	// 从UI收集数据并更新任务对象
	Task* getTask() const override;
	void clear() override;
private:
	Ui::RecoveryTaskEditorClass ui;
	Task* m_currentTask = nullptr;
};
