#pragma once

#include <QWidget>
#include "ui_missionmonitorwidget.h"
#include"observer.h"
#include"MapModule/s57mapmodule.h"
#include"missionmonitormodule.h"
#include"auvinfopanel.h"
#include "missiontreewidget.h"
#include"missioncontroller.h"
#include"editormanager.h"
#include "sailtaskeditor.h"
#include "silenttaskeditor.h"
#include "patroldetecttaskeditor.h"
#include "shorebasedstriketaskeditor.h"
#include "striketaskeditor.h"
#include "escapetaskeditor.h"
#include "beaconcommunciationtaskediotor.h"
#include "floatingcalibrationtaskeditor.h"
#include "floatingcommunicationtaskediotr.h"
#include "recoverytaskeditor.h"
#include"moduledialogbase.h"
#include <QScopedPointer>  // 用于智能指针管理
#include<QStatusBar>
#include <QToolBar>  // ：菜单栏头文件
#include <QAction>   // ：动作头文件
#include<QDockWidget>
#include<QTimer>
class MissionMonitorWidget : public ModuleDialogBase,IResultObserver
{
	Q_OBJECT

public:
	
	MissionMonitorWidget(MissionMonitorModule *monitorModule, S57MapModule* mapModule, QWidget* parent = nullptr);
	~MissionMonitorWidget() override;

	//实现观察者接口
	void onResultUpdated(const BaseResult* result) override;

	MissionModel* getMissionModel();
public slots:
	void closeAuvInfoPanel();
	// ：工具动作对应的槽函数
	void on_missionOpenAction_triggered();
	void on_missionPreviewAction_triggered();        // 使命预览
	void on_missionIssueAction_triggered();          // 使命下发
	void on_missionImplementAction_triggered();      // 使命执行
	void on_missionStopAction_triggered();           // 使命停止
	void on_missionTreeAction_triggered();           //使命树显示
	void on_taskPropertiesAction_triggered();        //任务属性
	void on_taskEditorSelection_changed(const QModelIndex& current);
	// DockWidget显隐控制槽函数
	void on_missionDockAction_triggered();            // 切换左侧任务Dock显隐
	void on_editorAction_triggered();             // 切换右侧编辑器Dock显隐
private:
	void initialize()override;
	
	void handleMissionMonitorResult(const  MissionMonitorResult* result);

	// 初始化UI与依赖对象
	void initUI();
	void initBaseWidgets();                // 初始化基础控件
	void initMainLayout();                 // 初始化主布局
	void initStatusBar();                  // 初始化状态栏
	void initToolBar();                    // 初始化工具栏
	void initDockWidgets();                // 初始化DockWidget

	// 业务模块初始化
	void setupCoreModules();    // 初始化核心业务模块
	void setupTaskEditors();    // 设置任务编辑器

	// 数据与渲染
	void initTestData();
	void renderCanvas();
	void updateAuvStatusInStatusBar();     // 更新AUV状态（在线/离线、数量）
	void updateDataTimeInStatusBar();      // 更新最新数据时间戳
	 // 工具与动作
	void createToolActions();              // 创建工具动作
	void addToolSeparators();              // 添加工具分隔符
	void bindAllSignals();            // 绑定动作与槽函数
	void createDockActions();              // 创建控制Dock显隐的动作

	// 模拟相关
	void simulateAuvMovement();
	void stopSimulation();      // 停止模拟定时器
private:
	Ui::MissionMonitorWidgetClass ui;

	// 外部模块引用（不负责销毁）
	MissionMonitorModule* m_monitorModule = nullptr;  
	S57MapModule* m_pS57MapModule = nullptr;          
	CMapCanvas* m_mapCanvas = nullptr;                

	// 智能指针管理的成员变量
	QScopedPointer<AuvInfoPanel> m_infoPanel;         // 信息面板
	QScopedPointer<AuvRenderer> m_auvRenderer;        // AUV渲染器

	// 状态栏组件
	QStatusBar* m_statusBar = nullptr;          // 状态栏主体
	QLabel* m_auvStatusLabel = nullptr;         // 显示AUV状态（如“在线：1/1”）
	QLabel* m_pGeoLabel = nullptr;
	QLabel* m_pScaleLabel = nullptr;        // 显示地图缩放（如“1:5000”）
	QLabel* m_dataUpdateTimeLabel = nullptr;    // 显示最新数据时间（如“更新：2024-10-01 12:00:00”）
	QString m_lastUpdateTime;                   // 缓存最新数据时间戳

	 // 工具栏组件
	QToolBar* m_toolBar = nullptr;         // 工具栏主体
	QAction* m_toolSep1 = nullptr;  // 使命工具与地图工具的分隔符
	QAction* m_toolSep2 = nullptr;  // 地图工具与查询工具的分隔符
	// 工具动作
	QAction* action_missionOpen = nullptr;
	QAction* action_missionPreview = nullptr;  // 使命预览
	QAction* action_issue = nullptr;           // 使命下发
	QAction* action_implement = nullptr;       // 使命执行
	QAction* action_stop = nullptr;            // 使命停止
	QAction* action_zoomIn = nullptr;          // 地图放大
	QAction* action_zoomOut = nullptr;         // 地图缩小
	QAction* action_translate = nullptr;       // 地图平移
	QAction* action_auvQuery = nullptr;        // AUV查询

	// 使命相关
	MissionTreeWidget* m_missionTreeWidget = nullptr;
	MissionController* m_missionController = nullptr;
	MissionView* m_missionView = nullptr;
	MissionModel* m_missionModel = nullptr;

	// 编辑器相关
	QMap<TaskType, QWidget*> m_taskEditors;
	QWidget* m_currentEditor = nullptr;
	QStackedWidget* m_editorContainer = nullptr;// 编辑器容器
	EditorManager* m_editorManager = nullptr;

	// DockWidget
	QDockWidget* m_leftMissionDock = nullptr;  // 左侧任务Dock
	QDockWidget* m_rightEditorDock = nullptr;  // 右侧编辑器Dock
	QAction* action_toggleMissionDock = nullptr; // 控制左侧Dock显隐的动作
	QAction* action_toggleEditorDock = nullptr;  // 控制右侧Dock显隐的动作
	QAction* m_dockSep = nullptr;              // 工具栏中Dock控制动作的分隔符

	// 布局管理
	QVBoxLayout* m_mainVLayout;        // 主垂直布局
	QHBoxLayout* m_contentLayout;      // 内容区水平布局
	QWidget* m_centerWidget;           // 中间地图容器
	QHBoxLayout* m_centerLayout;       // 中间地图布局

	  // 模拟相关
	QScopedPointer<QTimer> m_simTimer;
	double m_simulationAngle = 0.0;  // 使用成员变量替代静态变量

	const QString STYLE_MONITORWIDGET = "./qss/missionmonitorwidget.qss";
};
