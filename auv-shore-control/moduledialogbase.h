#pragma once

#include <QDialog>
#include"module_style_helper.h"
class ModuleDialogBase  : public QDialog
{
	
public:
	explicit ModuleDialogBase(QWidget *parent = nullptr);
	virtual ~ModuleDialogBase() = default;

	//模块对话框初始化
	virtual void initialize() = 0;
	
	// 实现样式加载接口（调用通用逻辑，目标窗口是当前对话框）
	void loadStyleSheet(const QString& fileName){
		ModuleStyleHelper::loadStyleSheet(fileName, this);
	}
};
