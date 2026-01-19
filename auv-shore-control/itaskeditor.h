#pragma once

#include <QWidget>
#include<QCheckBox>
#include<QComboBox>
#include<QHBoxLayout>
#include"task.h"
#include"plan.h"
class ITaskEditor  : public QWidget
{
	Q_OBJECT
public:
	explicit ITaskEditor(QWidget* parent = nullptr) : QWidget(parent) {}
	// 设置任务数据到编辑器UI
	virtual void setTask(Task* task) = 0;
	// 从编辑器UI获取修改后的任务数据
	virtual Task* getTask() const = 0;
	virtual void setPlan(Plan* plan) { Q_UNUSED(plan); } // Optional for plan editors
	virtual void clear() = 0;
	virtual QWidget* createCheckBox(QCheckBox* checkBox, QWidget* widget)
	{
		QWidget* cbWidget = new QWidget(widget);
		QHBoxLayout* hLayout = new QHBoxLayout(this);
		hLayout->addWidget(checkBox);
		hLayout->setMargin(0);
		hLayout->setAlignment(checkBox, Qt::AlignCenter);//居中
		hLayout->setMargin(0);
		cbWidget->setLayout(hLayout);
		return cbWidget;
	};
signals:
	void dataChanged();
	// 编辑器内容修改时发射
	void taskModified();
};
