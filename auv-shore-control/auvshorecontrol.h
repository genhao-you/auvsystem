#pragma once

#include <QtWidgets/QMainWindow>
#include <QScopedPointer>
#include <QMap>
#include <QTimer>
#include "ui_auvshorecontrol.h"
#include "selfcheckmaindialog.h"
#include "communicationssettingdialog.h"
#include "devicemonitordialog.h"
#include "taskmonitordialog.h"
#include "dataquerydialog.h"
#include "datadownloaddialog.h"
#include "sailrestrictiondialog.h"
#include "editormanager.h"
#include "communicationmodule.h"
#include "sailcontrolmodule.h"
#include "missionmonitorwidget.h"
#include "MapModule/s57mapmodule.h"
#include "auvinfopanel.h"
#include "module_style_helper.h"
#include "controlparamsettingdialog.h"
class AuvShoreControl : public QMainWindow
{
    Q_OBJECT

public:
    AuvShoreControl(QWidget* parent = nullptr);
    ~AuvShoreControl() override;  // 明确标记为override
private:
    // 主初始化函数
    void initialization();
    // 拆分的初始化函数，职责单一
    void setupUIComponents();   // 初始化UI组件
    void setupConnections();    // 建立信号槽连接
   

    // 辅助函数
    bool checkModuleValidity() const;  // 检查核心模块有效性
  

private slots:
    // 各种动作的槽函数
    void on_openMissionAction_triggered();
    void on_selfCheckDlgAction_triggered();
    void on_communicationSettingDlgAction_triggered();
    void on_deviceStatusAction_triggered();
    void on_taskMonitorAction_triggered();
    void on_controlDebugAction_triggered();

    void on_dataSelectAction_triggered();
    void on_dataDownloadBtn_triggered();
    void on_sailRestrictionAction_triggered();
   
    void on_startSailBtn_clicked();

    //使命监控
    void on_missionMonitorAction_triggered();

    // 声明更新时间的槽函数
    void updateCurrentTime();

private:
    // 核心模块
    S57MapModule* m_s57MapModule = nullptr;
    SelfCheckMainDialog_533* m_dlgSelfCheck = nullptr;
    CommunicationsSettingDialog* m_dlgCommunicationsSetting = nullptr;
    DeviceMonitorDialog* m_dlgDeviceMonitor = nullptr;
    TaskMonitorDialog* m_dlgTaskMonitor = nullptr;
    DataDownloadDialog* m_dlgDataDownload = nullptr;
    DataQueryDialog* m_dlgDataQuery = nullptr;
    SailRestrictionDialog* m_dlgSailRestriction = nullptr;
    MissionMonitorWidget* m_wdtMissionMonitor = nullptr;
    ControlParamSettingDialog* m_dlgControlParamSetting = nullptr;
    // 通信模块（单例，不需要智能指针管理）
    CommunicationModule* m_communicationModule = nullptr;
    QTimer* timeTimer;  // 定时器对象（用于每秒触发时间更新）
    const QString STYLE_MAINWINDOW = "./qss/mainwindow.qss";
    SailControlModule* m_sailControlModule;  // 新增航行控制模块
private:
    Ui::auvshorecontrolClass ui;
};
