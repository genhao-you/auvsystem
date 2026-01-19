#pragma once

#include"itaskeditor.h"
#include "ui_floatingcommunicationtaskediotr.h"

class FloatingCommunicationTaskEdiotr : public ITaskEditor
{
	Q_OBJECT

public:
	FloatingCommunicationTaskEdiotr(QWidget *parent = nullptr);
	~FloatingCommunicationTaskEdiotr();
	void setTask(Task* task) override;
	void clear() override;
	// 从UI收集数据并更新任务对象
	Task* getTask() const override;
private:
	Ui::FloatingCommunicationTaskEdiotrClass ui;
	Task* m_currentTask = nullptr;
};
