#pragma once

#include"itaskeditor.h"
#include "ui_planeditor.h"

class PlanEditor : public ITaskEditor
{
	Q_OBJECT

public:
	explicit PlanEditor(QWidget* parent = nullptr);
	~PlanEditor();
	void setTask(Task* task) override { Q_UNUSED(task); };
	// 从UI收集数据并更新任务对象
	Task* getTask() const override;
	void setPlan(Plan* plan);
	void clear() override;
signals:
	void planDataChanged(Plan* plan);//
private:
	void connectSignals();
	void updateFields();
private:
	Plan* m_currentPlan;
	Task* m_currentTask = nullptr;
	Ui::PlanEditorClass ui;
};
