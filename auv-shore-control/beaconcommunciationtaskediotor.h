#pragma once

#include "ui_beaconcommunciationtaskediotor.h"
#include"task.h"
#include"plan.h"
class BeaconCommunciationTaskEdiotor : public ITaskEditor
{
	Q_OBJECT

public:
	BeaconCommunciationTaskEdiotor(QWidget *parent = nullptr);
	~BeaconCommunciationTaskEdiotor();
	void setTask(Task* task) override;
	// 从UI收集数据并更新任务对象
	Task* getTask() const override;
	void clear() override;
private:
	void connectSignals();
	void setupUI();
	void updateFields();
private:
	Task* m_currentTask;
	Ui::BeaconCommunciationTaskEdiotorClass ui;
};
