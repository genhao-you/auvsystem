#pragma once

#include <QDialog>
#include "ui_selfcheckmaindialog.h"
#include"devicepage.h"
#include"moduledialogbase.h"
#include"selfcheckmodule.h"
#include"baseresult.h"
#include "devicehandlerbase.h"
class SelfCheckMainDialog_533 : public ModuleDialogBase,IResultObserver
{
	Q_OBJECT

public:
	SelfCheckMainDialog_533(SelfCheckModule* module, QWidget *parent = nullptr);
	~SelfCheckMainDialog_533() override;
	void onResultUpdated(const BaseResult* result) override;
private:
	void initialize()override;
	void initDeviceHandlers();
	void initTreeWidget();
	void initPageMapping();
	void initCommandRegistry();

	// 工具方法
	void switchToPage(DevicePage page);


	void setTreeWidgetItemData(QTreeWidget* treeWidget);
	void handleSendResult(const SelfCheckSendResult* result);
	void handleSelfCheckResult(const SelfCheckResult* result);
private slots:
	void on_missionTreeWidget_currentItemChanged(QTreeWidgetItem* currentItem, QTreeWidgetItem* previous);
	
private:
	Ui::SelfCheckMainDialogClass ui;
	SelfCheckModule* m_module;

	// 设备处理器映射
	QMap<DeviceId, QSharedPointer<DeviceHandlerBase>> m_deviceHandlers;

	// 页面映射
	QMap<DevicePage, int> m_pageIndexMap;

	// 当前页面
	DevicePage m_currentDevicePage = DevicePage::CORRESPOND;

	// 样式和图标
	const QString STYLE_SELFCHECK = "./qss/selfcheckdlg.qss";
};
