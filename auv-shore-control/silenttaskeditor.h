#pragma once

#include"itaskeditor.h"
#include "ui_silenttaskeditor.h"
#include<QDateTimeEdit>
#include<QCheckBox>
#include<QComboBox>
class SilentTaskEditor : public ITaskEditor
{
	Q_OBJECT

public:
	SilentTaskEditor(QWidget *parent = nullptr);
	~SilentTaskEditor()override;
	void setTask(Task* task) override;
	// 从UI收集数据并更新任务对象
	Task* getTask() const override;
	void clear() override;
private:
	void initUI();
private:
	QDateTimeEdit* m_timeEditEnd;
	QCheckBox* m_checkBoxIsEnd;
	QComboBox* m_comboBoxEndCondition;//结束条件
	QComboBox* m_comboBoxZUnit;//z向单位
private:
	Ui::SilentTaskEditorClass ui;
	Task* m_currentTask = nullptr;
};
