#pragma once

#include"itaskeditor.h"
#include "ui_patroldetecttaskeditor.h"
#include<QDateTimeEdit>
#include<QCheckBox>
#include<QComboBox>
#include<QHBoxLayout>
class PatrolDetectTaskEditor : public ITaskEditor
{
	Q_OBJECT

public:
	PatrolDetectTaskEditor(QWidget *parent = nullptr);
	~PatrolDetectTaskEditor()override;
	void setTask(Task* task) override;
	// 从UI收集数据并更新任务对象
	Task* getTask() const override;
	void clear() override;
private:
	void initUI();
	//QWidget * createCheckBox(QCheckBox* checkBox, QWidget* widget);
private:
	QDateTimeEdit* m_timeEditEnd;
	QCheckBox* m_checkBoxIsEnd;
	QComboBox* m_comboBoxEndCondition;//结束条件
	QComboBox* m_comboBoxZUnit;//z向单位
	QComboBox* m_comboBoxSpeedUnit;//速度单位
	QComboBox* m_comboBoxPatrolType;//游曳类型
	QComboBox* m_comboBoxTargetProcess;//游曳类型
	QCheckBox* m_checkBoxPhotoElectricity;//光电检测
	QCheckBox* m_checkBoxElectromagnetism;//电磁检测
	QCheckBox* m_checkBoxGunwale;//船舷
	QCheckBox* m_checkBoxDrag;//拖曳
	Task* m_currentTask = nullptr;
private:
	Ui::PatrolDetectTaskEditorClass ui;
};
