#pragma once

#include"itaskeditor.h"
#include "ui_sailtaskeditor.h"
#include"task.h"
#include "module_style_helper.h" 
class SailTaskEditor : public ITaskEditor
{
	Q_OBJECT

public:
	explicit SailTaskEditor(QWidget *parent = nullptr);
	~SailTaskEditor()override;
	// 从任务对象加载数据到UI
	void setTask(Task* task) override;
	// 从UI收集数据并更新任务对象
	Task* getTask() const override;
	// 清空UI内容
	void clear() override;
signals:
	// 任务内容修改时发射
	void taskModified();
private slots:
	// UI控件修改触发数据同步
	void onUIChanged();
	//确认按钮事件（触发任务属性更新）
	void on_btn_saveSettings_clicked();
private:
	// 初始化UI默认状态
	void initUI();
	// 连接UI信号与槽
	void initConnections();
	// 根据航行模式更新深度/高度标签
	void updateDepthHeightLabel(NavigationMode mode);
	// 校验UI输入合法性
	bool validateInput() const;

	//单独的属性同步方法
	bool syncTaskProperties();
	// 批量阻塞/恢复UI控件的信号
	void blockUISignals(bool block);
private:
	Ui::SailTaskEditorClass ui;
	Task* m_currentTask = nullptr;
	const QString STYLE_SAILTASKEDITOR = "./qss/sailtaskeditor.qss";
	// 下拉框索引与任务调整操作的映射
	QMap<int, Task::TaskAdjustOp> m_adjustMap;
	NavigationMode m_currentMode = NavigationMode::DepthKeeping; // 当前航行模式
};
