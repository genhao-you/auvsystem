#pragma once

#include <QDialog>
#include "ui_controlparamsettingdialog.h"
#include "moduledialogbase.h"
#include "controlparammodule.h"
#include "controlparamresult.h"
class ControlParamSettingDialog : public ModuleDialogBase, IResultObserver
{
	Q_OBJECT

public:
	ControlParamSettingDialog(ControlParamModule * module,QWidget *parent = nullptr);
	~ControlParamSettingDialog() override;
	void onResultUpdated(const BaseResult* result) override;
private:
    // 初始化界面（对齐自检对话框）
    void initialize() override;
    // 处理发送结果
    void handleSendResult(const ControlParamSendResult* result);
    // 处理控制执行结果
    void handleControlResult(const ControlParamResult* result);
    // 界面文本更新
    void updateWidgetText(QWidget* widget, const QString& text, const QColor& color = Qt::black);
    // 构建控制参数结构体（从UI读取）
    ControlParams buildControlParamsFromUI();
private slots:
    // 按钮点击槽函数（补充缺失的槽）
    void on_btn_SendDepthParam_clicked();
    void on_btn_SendHeightParam_clicked();
    void on_btn_SendHeadingParam_clicked();
private:
	Ui::ControlParamSettingDialogClass ui;
	ControlParamModule* m_module = nullptr;
	const QString STYLE_CONTROLDEBUGGING = "./qss/controlparamsettingdlg.qss";
    // UI控件缓存
    QMap<QString, QDoubleSpinBox*> m_depthSpinBoxes;
    QMap<QString, QDoubleSpinBox*> m_heightSpinBoxes;
    QMap<QString, QDoubleSpinBox*> m_headingSpinBoxes;
};
