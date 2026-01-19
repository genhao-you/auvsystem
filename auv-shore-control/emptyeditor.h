#pragma once

#include"itaskeditor.h"

class EmptyEditor  : public ITaskEditor
{
	Q_OBJECT

public:
	EmptyEditor(QWidget  *parent =nullptr);
	~EmptyEditor();
	void setTask(Task* task) override {}
	// 从UI收集数据并更新任务对象
	Task* getTask() const override { return m_currentTask; }
	 void setPlan(Plan* plan) {} // Optional for plan editors
	 void clear() {}
private:
	Task* m_currentTask;
};
