#include "missionmonitorwidget.h"
#include<QVBoxLayout>
#include<QMessageBox>
#include<QMetaEnum>
#include<QFileDialog>
#include<qDebug>
// 检查指针有效性的宏（增强空指针防护）
#define CHECK_COMPONENT(ptr, msg) \
    if (!(ptr)) { \
        qCritical() << "[MissionMonitorWidget] " << msg; \
        if (this) this->m_statusBar->showMessage(msg, 3000); \
        return; \
    }



MissionMonitorWidget::MissionMonitorWidget(MissionMonitorModule* monitorModule, S57MapModule* mapModule, QWidget* parent)
	: ModuleDialogBase(parent)
	, m_monitorModule(monitorModule)
	, m_pS57MapModule(mapModule)  // 接收外部地图模块，不转移所有权
	, m_mapCanvas(m_pS57MapModule->getMapCanvas())
	// 手动初始化原始指针（头文件未用智能指针，保持原方式）
	, m_infoPanel(new AuvInfoPanel(this))
	, m_auvRenderer(new AuvRenderer(m_mapCanvas, this))
	, m_missionModel(new MissionModel(this))
	, m_missionView(new MissionView(this))
	, m_missionController(new MissionController(this))
	, m_missionTreeWidget(new MissionTreeWidget(this))
	, m_editorContainer(new QStackedWidget(this))
	, m_editorManager(new EditorManager(m_editorContainer, this))
	// 布局指针初始化（避免野指针）
	, m_mainVLayout(new QVBoxLayout(this))
	, m_contentLayout(new QHBoxLayout())
	, m_centerWidget(new QWidget())
	, m_centerLayout(new QHBoxLayout(m_centerWidget))
	// 状态栏控件初始化（确保非空）
	, m_statusBar(new QStatusBar(this))
	, m_auvStatusLabel(new QLabel("AUV状态：加载中...", m_statusBar))
	, m_pGeoLabel(new QLabel(m_statusBar))
	, m_pScaleLabel(new QLabel(m_statusBar))
	, m_dataUpdateTimeLabel(new QLabel("最新更新：--", m_statusBar))
{
	ui.setupUi(this);
	initialize();

	// 注入地图模块依赖（确保在UI初始化后执行）
	if (m_pS57MapModule) {
		m_pS57MapModule->setAuvToolDependencies(m_auvRenderer.data(), m_infoPanel.data());
	}
	// 注册观察者（避免UI未初始化时接收数据）
	if (m_monitorModule) {
		m_monitorModule->addObserver(this);
	}
	initTestData();

}

MissionMonitorWidget::~MissionMonitorWidget()
{
	if (m_missionModel != nullptr)
		delete m_missionModel;
	m_missionModel = nullptr;

	if (m_missionView != nullptr)
		delete m_missionView;
	m_missionView = nullptr;

	if (m_missionController != nullptr)
		delete m_missionController;
	m_missionController = nullptr;

	if (m_missionTreeWidget != nullptr)
		delete m_missionTreeWidget;
	m_missionTreeWidget = nullptr;

	if (m_editorManager != nullptr)
		delete m_editorManager;
	m_editorManager = nullptr;
}

void MissionMonitorWidget::onResultUpdated(const BaseResult * result)
{
	if (!result) return;

	// 只处理使命监控模块的结果
	if (result->sourceType() == ModuleType::Monitoring) {
		handleMissionMonitorResult(static_cast<const MissionMonitorResult*>(result));
	}
}

MissionModel* MissionMonitorWidget::getMissionModel()
{
	return m_missionModel;
}

void MissionMonitorWidget::on_missionOpenAction_triggered()
{
	QString strCellDir = QFileDialog::getExistingDirectory(this, "选择图幅目录", "/");
	if (strCellDir.isEmpty()) {
		QMessageBox::information(this, "提示", "请输入图幅目录");
		return;
	}
	m_pS57MapModule->loadMaps(strCellDir);
}

void MissionMonitorWidget::on_missionPreviewAction_triggered()
{
	if (!m_simTimer) {
		m_simTimer.reset(new QTimer(this));
		connect(m_simTimer.data(), &QTimer::timeout,
			this, &MissionMonitorWidget::simulateAuvMovement);
	}

	if (m_simTimer->isActive()) {
		stopSimulation();
		QMessageBox::information(this, "提示", "模拟已停止");
	}
	else {
		m_simTimer->start(1000);
		QMessageBox::information(this, "提示", "模拟已启动");
	}
}

void MissionMonitorWidget::on_missionIssueAction_triggered()
{
	QString localPath = "./TaskFile/mission_" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + ".json";
	CheckError success = m_missionController->saveAndSendTasks();
	if (success.isSuccess()) {
		qInfo() << "仅任务JSON已保存到：" << localPath;
		action_implement->setEnabled(true);
	}
}

void MissionMonitorWidget::on_missionImplementAction_triggered()
{
	QString localPath = "./CameraFiles/20251107095916_00007.avi";
	QString remotePath = "/mnt/mySSD/Camera/video/20251107095916_00007.avi";
	CheckError success = m_monitorModule->downloadFile(localPath, remotePath);
	if (success.isSuccess()) {
		qInfo() << "下载文件已发送：" << localPath;
	}
}

void MissionMonitorWidget::on_missionStopAction_triggered()
{
}

void MissionMonitorWidget::on_missionTreeAction_triggered()
{

}

void MissionMonitorWidget::on_taskPropertiesAction_triggered()
{

}

void MissionMonitorWidget::on_taskEditorSelection_changed(const QModelIndex& current)
{
	if (!m_editorManager) {
		//LOG_ERROR("EditorManager is not initialized");
		return;
	}

	if (!current.isValid()) {
		m_editorManager->showEmptyEditor();
		return;
	}

	const int nodeType = m_missionController->getModel()->data(current, MissionModel::NodeTypeRole).toInt();
	switch (nodeType) {
	case MissionModel::MISSION:
		m_editorManager->showEmptyEditor();
		break;
	case MissionModel::PLAN:
		m_editorManager->showPlanEditor(m_missionController->getModel()->planAt(current));
		break;
	case MissionModel::TASK:
		m_editorManager->showEditorForTask(m_missionController->getModel()->taskAt(current));
		break;
	default:
		m_editorManager->showEmptyEditor();
		break;
	}
}

void MissionMonitorWidget::on_missionDockAction_triggered()
{
	bool visible = action_toggleMissionDock->isChecked();
	m_leftMissionDock->setVisible(visible);
	m_statusBar->showMessage(visible ? "任务面板已显示" : "任务面板已隐藏", 2000);
}

void MissionMonitorWidget::on_editorAction_triggered()
{
	bool visible = action_toggleEditorDock->isChecked();
	m_rightEditorDock->setVisible(visible);
	m_statusBar->showMessage(visible ? "编辑面板已显示" : "编辑面板已隐藏", 2000);
}

void MissionMonitorWidget::initialize()
{
	initUI();
}


// 分发监控结果到渲染器
void MissionMonitorWidget::handleMissionMonitorResult(const MissionMonitorResult* result)
{
	if (!result || !m_auvRenderer) return;
	MissionTrajectory mission;
	// 获取AUV ID（转为QString，适配渲染器接口）
	const QString auvId = result->auvId();

	// 根据结果类型，调用对应渲染接口
	switch (result->resultType()) {
	case MissionMonitorResultType::RealTimePosition:
		// 实时定位：更新AUV位置
		m_auvRenderer->updateAuv(auvId, result->realTimeData());
		break;
	case MissionMonitorResultType::HistoricalTrail:
		// 历史轨迹：添加轨迹点
		for (const auto& point : result->historicalTrail()) {
			ActualTrajectoryPoint trajPoint;  // 假设该结构体可从GeoCoord构造
			trajPoint.coord = point;
			trajPoint.timestamp = QDateTime::currentDateTime();  // 可从结果补充时间
			m_auvRenderer->addActualTrajectoryPoint(auvId, trajPoint);
		}
		break;
	case MissionMonitorResultType::PlannedTrail:
		// 计划轨迹：设置使命航点
		  // 假设该结构体可从Waypoint列表构造
		mission.waypoints = result->plannedWaypoins();
		mission.isActive = true;
		m_auvRenderer->setAuvMissionTrajectory(auvId, mission);
		break;
	case MissionMonitorResultType::DetectedTarget:
		// 环境目标：添加/更新探测目标
		for (const auto& target : result->environmentTargets()) {
			m_auvRenderer->addTarget(target);
		}
		break;
	default:
		break;
	}

	// 触发渲染刷新（确保界面及时更新）
	renderCanvas();
}

void MissionMonitorWidget::initUI()
{

	// 1. 先初始化基础控件（避免后续布局引用空控件）
	initBaseWidgets();

	// 2. 初始化业务模块（Dock内容依赖这些模块，必须先执行）
	setupCoreModules();
	setupTaskEditors();

	// 3. 构建主布局框架（从外到内：主布局→内容区→中间容器）
	initMainLayout();

	// 4. 初始化工具栏、Dock、状态栏（加入已构建的布局）
	initToolBar();
	initDockWidgets();
	initStatusBar();

	// 5. 最后绑定信号槽（确保所有控件已初始化）
	bindAllSignals();

	loadStyleSheet(STYLE_MONITORWIDGET);
}

void MissionMonitorWidget::initBaseWidgets()
{
	m_editorContainer->setObjectName("editorContainer");
	m_statusBar->setObjectName("statusBar");
	m_auvStatusLabel->setObjectName("auvStatusLabel");
	m_pGeoLabel->setObjectName("pGeoLabel");
	// 信息面板：固定宽度+默认隐藏
	if (m_infoPanel) {
		m_infoPanel->setFixedWidth(450);
		m_infoPanel->setVisible(false);
	}
	// 模拟角度初始化
	m_simulationAngle = 0.0;

	// 状态栏标签基础样式（避免显示异常）
	if (m_pGeoLabel) {
		m_pGeoLabel->setAlignment(Qt::AlignHCenter);
		m_pGeoLabel->setFixedSize(QSize(250, 20));
	}
	if (m_pScaleLabel) {
		m_pScaleLabel->setAlignment(Qt::AlignHCenter);
		m_pScaleLabel->setFixedSize(QSize(250, 20));
	}
}

void MissionMonitorWidget::initMainLayout()
{
	// 主布局：最外层，控制整体结构（工具栏→内容区→状态栏）
	CHECK_COMPONENT(m_mainVLayout, "主布局为空，无法构建UI");
	m_mainVLayout->setContentsMargins(0, 0, 0, 0);
	m_mainVLayout->setSpacing(0);
	this->setLayout(m_mainVLayout); // 强制绑定当前Widget的布局

	// 中间容器布局：承载地图+信息面板（确保地图占主要空间）
	CHECK_COMPONENT(m_centerLayout, "中间容器布局为空");
	m_centerLayout->setContentsMargins(0, 0, 0, 0);
	m_centerLayout->setSpacing(0);
	if (m_mapCanvas) {
		m_centerLayout->addWidget(m_mapCanvas, 1); // 权重1：占满剩余空间
	}
	if (m_infoPanel) {
		m_centerLayout->addWidget(m_infoPanel.data(), 0); // 权重0：固定宽度
	}

	// 内容区布局：承载左侧Dock+中间容器+右侧Dock（先加入中间容器）
	CHECK_COMPONENT(m_contentLayout, "内容区布局为空");
	m_contentLayout->setContentsMargins(0, 0, 0, 0);
	m_contentLayout->setSpacing(0);
	if (m_centerWidget) {
		m_contentLayout->addWidget(m_centerWidget, 1); // 中间容器占主要空间
	}
}

void MissionMonitorWidget::setupCoreModules()
{
	// 初始化核心业务模块
	m_missionController->setModel(m_missionModel);
	m_missionController->setView(m_missionView);
	m_missionTreeWidget->setObjectName("missionTree");
	m_missionView->setObjectName("missionView");
	// 初始化任务树

	m_missionTreeWidget->setFixedWidth(280);


	// 使命视图连接
	/*connect(m_missionView, &MissionView::selectionChanged,
		this, &MissionMonitorWidget::onTaskEditorSelectionChanged,
		Qt::UniqueConnection);*/
}

void MissionMonitorWidget::setupTaskEditors()
{
	// 清理旧编辑器（避免重复创建）
	qDeleteAll(m_taskEditors.values());
	m_taskEditors.clear();

	auto addEditor = [this](TaskType type, ITaskEditor* editor) {
		if (!editor) {
			// 修复TaskType输出问题（转为枚举字符串）
			qCritical() << "[MissionMonitorWidget] 任务编辑器创建失败，类型："
				<< (static_cast<int>(type));
			return;
		}
		editor->setParent(m_editorContainer);
		m_editorContainer->addWidget(editor);
		// 用QSharedPointer管理编辑器生命周期（避免内存泄漏）
		m_taskEditors[type] = editor;
		if (m_editorManager) {
			m_editorManager->registerEditor(type, editor);
		}
	};

	// 创建所有任务编辑器（确保加入StackedWidget）
	addEditor(TaskType::SAIL, new SailTaskEditor(this));
	addEditor(TaskType::SILENT, new SilentTaskEditor(this));
	addEditor(TaskType::PATROL_DETECT, new PatrolDetectTaskEditor(this));
	addEditor(TaskType::STRIKE, new StrikeTaskEditor(this));
	addEditor(TaskType::SHORE_BASED_STRIKE, new ShoreBasedStrikeTaskEditor(this));
	addEditor(TaskType::ESCAPE, new EscapeTaskEditor(this));
	addEditor(TaskType::FLOATING_CALIBRATION, new FloatingCalibrationTaskEditor(this));
	addEditor(TaskType::FLOATING_COMMUNICATION, new FloatingCommunicationTaskEdiotr(this));
	addEditor(TaskType::BEACON_COMMUNICATION, new BeaconCommunciationTaskEdiotor(this));
	addEditor(TaskType::RECOVERY, new RecoveryTaskEditor(this));

	// 注册计划编辑器（确保右侧Dock非空）
	if (m_editorManager) {
		m_editorManager->registerPlanEditor(new PlanEditor());
	}
}

void MissionMonitorWidget::initTestData()
{
	AuvRealTimeData auv;
	auv.auvId = "AUV-001";
	//QPointF point =  m_s57MapModule->getMapCanvas()->getViewportCenterPt();  { x = 108.87541665000001, y = 18.383194399847842 }   108°52′31″，18°22′54″
	auv.geoPos = { 108.8754,18.38319 };
	auv.heading = 0;
	auv.speed = 1.2;
	auv.isConnected = true;
	m_auvRenderer->addAuv(auv);
	////添加轨迹
	for (int i = 0; i < 50; ++i)
	{
		double angle = 2 * M_PI * i / 50;
		GeoCoord trajCoord(108.8754 + 0.01 * cos(angle), 18.38319 + 0.01 * sin(angle));
		m_auvRenderer->addActualTrajectoryPoint("AUV-001", ActualTrajectoryPoint(trajCoord));
	}
	////添加目标
	DetectedTarget target1;
	target1.targetId = "T001";
	target1.coord = { 108.876,18.384 };
	target1.type = "礁石";
	target1.confidence = 0.95;
	m_auvRenderer->addTarget(target1);

	 //添加增强版使命轨迹
	MissionTrajectory mission;
	mission.missionId = "MISSION-001";
	mission.isActive = true;

	//起点航点
	MissionWaypoint wp1("WPT-001", GeoCoord(108.880, 18.3841));
	wp1.type = WaypointType::Start;
	wp1.stayTime = 0.0;
	wp1.radius = 0.0;
	wp1.taskDesc = "航行任务";
	wp1.isSelected = true; //高亮起点
	mission.waypoints.append(wp1);

	//拐点航点
	MissionWaypoint wp2("WPT-002", GeoCoord(108.881, 18.3842));
	wp2.type = WaypointType::Turning;
	wp2.stayTime = 15.0;
	wp2.radius = 10.0;
	wp2.taskDesc = "静默任务";
	wp2.isSelected = true; //高亮
	mission.waypoints.append(wp2);

	//任务点航点
	MissionWaypoint wp3("WPT-003", GeoCoord(108.882, 18.3843));
	wp3.type = WaypointType::Mission;
	wp3.stayTime = 60.0;
	wp3.radius = 20.0;
	wp3.taskDesc = "探测任务";
	wp3.isSelected = true; //高亮
	mission.waypoints.append(wp3);

	//终点航点
	MissionWaypoint wp4("WPT-004", GeoCoord(108.883, 18.3844));
	wp4.type = WaypointType::End;
	wp4.stayTime = 60.0;
	wp4.radius = 20.0;
	wp4.taskDesc = "打击任务";
	wp4.isSelected = true; //高亮
	mission.waypoints.append(wp4);
	m_auvRenderer->setAuvMissionTrajectory("AUV-001", mission);
}

void MissionMonitorWidget::renderCanvas()
{
	if (!m_mapCanvas || !m_auvRenderer) return;

	CS57CanvasImage* cs57Canvas = m_mapCanvas->getCanvasImage();
	if (!cs57Canvas)
	{
		qDebug() << "[AuvRenderControl]无法获取CS57画布，AUV绘制失败";
		return;
	}

	m_auvRenderer->paintAuvOnCs57Canvas(cs57Canvas);
	m_mapCanvas->updateCanvas();
}

void MissionMonitorWidget::initStatusBar()
{
	CHECK_COMPONENT(m_statusBar, "状态栏创建失败");
	
	m_statusBar->setMinimumHeight(40); // 或更高的数值如50
	// 添加标签到状态栏（顺序：AUV状态→经纬度→缩放→时间）
	if (m_auvStatusLabel) {
		m_auvStatusLabel->setMinimumWidth(150);
		m_auvStatusLabel->setMinimumHeight(30); // 增大标签高度
		m_auvStatusLabel->setAlignment(Qt::AlignCenter); // 垂直+水平居中
		m_statusBar->addWidget(m_auvStatusLabel);
	}
	if (m_pGeoLabel) {
		m_pGeoLabel->setMinimumHeight(30); // 关键：增大经纬度标签高度
		m_pGeoLabel->setAlignment(Qt::AlignCenter); // 垂直居中
		m_pGeoLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred); // 允许垂直扩展
		m_statusBar->addWidget(m_pGeoLabel, 1);   // 权重1：自适应
	}
	if (m_pScaleLabel) {
		// 初始化缩放文本（避免空值）
		if (m_mapCanvas) {
			int initScale = m_mapCanvas->startDisplayScale();
			m_pScaleLabel->setText(QString("1:%1").arg(initScale));
		}
		m_pScaleLabel->setMinimumHeight(30); // 关键：增大比例尺标签高度
		m_pScaleLabel->setAlignment(Qt::AlignCenter); // 垂直居中
		m_pScaleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
		m_statusBar->addWidget(m_pScaleLabel, 1);
	}
	if (m_dataUpdateTimeLabel) {
		m_dataUpdateTimeLabel->setMinimumWidth(180);
		m_dataUpdateTimeLabel->setMinimumHeight(30); // 增大时间标签高度
		m_dataUpdateTimeLabel->setAlignment(Qt::AlignCenter);
		m_statusBar->addPermanentWidget(m_dataUpdateTimeLabel); // 固定在右侧
	}


	// 关键：将状态栏加入主布局底部
	CHECK_COMPONENT(m_mainVLayout, "主布局为空，无法添加状态栏");
	m_mainVLayout->addWidget(m_statusBar);

	// 更新状态栏数据（避免初始空值）
	updateAuvStatusInStatusBar();
	updateDataTimeInStatusBar();
}

void MissionMonitorWidget::updateAuvStatusInStatusBar()
{

}


void MissionMonitorWidget::updateDataTimeInStatusBar()
{
}

void MissionMonitorWidget::initToolBar()
{
	// 创建工具栏（指定父对象，避免内存泄漏）
	m_toolBar = new QToolBar("主工具栏", this);
	m_toolBar->setObjectName("mainToolBar");
	CHECK_COMPONENT(m_toolBar, "工具栏创建失败");

	// 工具栏样式（确保显示正常，不浮动）
	m_toolBar->setStyleSheet(R"(
        QToolBar {
            background-color: #f0f0f0;
            border-bottom: 1px solid #dddddd;
            padding: 2px;
            spacing: 6px;
        }
        QToolButton {
            padding: 4px 8px;
            font-size: 12px;
            border-radius: 3px;
        }
        QToolButton:hover { background-color: #e0e0e0; }
        QToolButton:pressed { background-color: #d0d0d0; }
        QToolButton:checked { background-color: #4a90e2; color: white; }
    )");
	m_toolBar->setFloatable(false);
	m_toolBar->setMovable(false);

	// 先创建动作和分隔符（避免空动作加入）
	createToolActions();
	createDockActions();
	addToolSeparators();

	// 按逻辑顺序添加动作（使命→地图→查询→Dock控制）
	if (action_missionOpen)m_toolBar->addAction(action_missionOpen);
	if (action_missionPreview) m_toolBar->addAction(action_missionPreview);
	if (action_issue) m_toolBar->addAction(action_issue);
	if (action_implement) m_toolBar->addAction(action_implement);
	if (action_stop) m_toolBar->addAction(action_stop);
	if (m_toolSep1) m_toolBar->addAction(m_toolSep1);

	if (action_zoomIn) m_toolBar->addAction(action_zoomIn);
	if (action_zoomOut) m_toolBar->addAction(action_zoomOut);
	if (action_translate) m_toolBar->addAction(action_translate);
	if (m_toolSep2) m_toolBar->addAction(m_toolSep2);

	if (action_auvQuery) m_toolBar->addAction(action_auvQuery);
	if (m_dockSep) m_toolBar->addAction(m_dockSep);
	if (action_toggleMissionDock) m_toolBar->addAction(action_toggleMissionDock);
	if (action_toggleEditorDock) m_toolBar->addAction(action_toggleEditorDock);

	// 加入伸缩项（工具靠左，占满宽度）
	m_toolBar->addWidget(new QWidget(this));

	// 关键：将工具栏加入主布局顶部（第0个位置，确保在最上方）
	CHECK_COMPONENT(m_mainVLayout, "主布局为空，无法添加工具栏");
	m_mainVLayout->insertWidget(0, m_toolBar);
}

void MissionMonitorWidget::createToolActions()
{
	// -------------------------- 1. 使命菜单动作 --------------------------
	//使命打开：打开使命文件，包括任务序列，海图限制等
	action_missionOpen = new QAction("打开使命", this);
	action_missionOpen->setShortcut(QKeySequence("Ctrl+K"));
	action_missionOpen->setToolTip("打开使命序列文件（包括航行限制）");
	action_missionOpen->setIcon(QIcon("./icon/menubar/dakai.png"));
	
	// 使命预览：快捷键 Ctrl+P，提示“预览已规划的使命轨迹”
	action_missionPreview = new QAction("使命预览", this);
	action_missionPreview->setShortcut(QKeySequence("Ctrl+P"));
	action_missionPreview->setToolTip("预览已规划的使命轨迹（高亮显示）");
	// 可选：设置图标（需项目有资源文件）
	action_missionPreview->setIcon(QIcon("./icon/plantoolbar/missionPreview.png"));

	// 使命下发：快捷键 Ctrl+I，提示“将使命下发到AUV”
	action_issue = new QAction("使命下发", this);
	action_issue->setShortcut(QKeySequence("Ctrl+I"));
	action_issue->setToolTip("将已规划的使命下发到指定AUV");
	action_issue->setIcon(QIcon("./icon/plantoolbar/issue.png"));

	// 使命执行：快捷键 Ctrl+R，提示“启动AUV使命执行”
	action_implement = new QAction("使命执行", this);
	action_implement->setShortcut(QKeySequence("Ctrl+R"));
	action_implement->setToolTip("启动AUV的使命执行流程");
	action_implement->setIcon(QIcon("./icon/plantoolbar/implement.png"));
	// 默认禁用：需先下发使命才能执行
	//action_implement->setEnabled(false);

	// 使命停止：快捷键 Ctrl+S，提示“停止当前使命执行”
	action_stop = new QAction("使命停止", this);
	action_stop->setShortcut(QKeySequence("Ctrl+S"));
	action_stop->setToolTip("停止AUV当前正在执行的使命");
	action_stop->setIcon(QIcon("./icon/plantoolbar/stop.png"));
	// 默认禁用：无执行中的使命时不可用
	action_stop->setEnabled(false);

	// -------------------------- 2. 地图菜单动作 --------------------------
	// 地图放大：快捷键 Ctrl++，提示“放大地图视图”
	action_zoomIn = new QAction("地图放大", this);
	action_zoomIn->setShortcut(QKeySequence("Ctrl++"));
	action_zoomIn->setToolTip("放大地图视图（每次放大20%）");
	action_zoomIn->setIcon(QIcon("./icon/mapeditbar/zoomIn.png"));

	// 地图缩小：快捷键 Ctrl+-，提示“缩小地图视图”
	action_zoomOut = new QAction("地图缩小", this);
	action_zoomOut->setShortcut(QKeySequence("Ctrl+-"));
	action_zoomOut->setToolTip("缩小地图视图（每次缩小20%）");
	action_zoomOut->setIcon(QIcon("./icon/mapeditbar/zoomOut.png"));

	// 地图平移：快捷键 Ctrl+T，提示“切换地图平移模式”
	action_translate = new QAction("地图平移", this);
	action_translate->setShortcut(QKeySequence("Ctrl+T"));
	action_translate->setToolTip("切换地图平移模式（点击后拖动地图）");
	action_translate->setIcon(QIcon("./icon/mapeditbar/translate.png"));
	action_translate->setCheckable(true);  // 可勾选：标记当前是否为平移模式

	// -------------------------- 3. 查询菜单动作 --------------------------
	// AUV查询：快捷键 Ctrl+Q，提示“查询AUV详细状态”
	action_auvQuery = new QAction("AUV查询", this);
	action_auvQuery->setShortcut(QKeySequence("Ctrl+Q"));
	action_auvQuery->setToolTip("查询所有AUV的实时状态、位置、任务信息");
	action_auvQuery->setIcon(QIcon("./icon/mapeditbar/auv_query.png"));
}

void MissionMonitorWidget::addToolSeparators()
{
	m_toolSep1 = new QAction(this);
	m_toolSep1->setSeparator(true);  // 使命组与地图组的分隔符

	m_toolSep2 = new QAction(this);
	m_toolSep2->setSeparator(true);  // 地图组与查询组的分隔符
}

void MissionMonitorWidget::bindAllSignals()
{
	// 1. 使命动作信号
	if (action_missionOpen)
	{
		connect(action_missionOpen, &QAction::triggered, this, &MissionMonitorWidget::on_missionOpenAction_triggered);
	}

	if (action_missionPreview) {
		connect(action_missionPreview, &QAction::triggered,
			this, &MissionMonitorWidget::on_missionPreviewAction_triggered);
	}
	if (action_issue) {
		connect(action_issue, &QAction::triggered,
			this, &MissionMonitorWidget::on_missionIssueAction_triggered);
	}
	if (action_implement) {
		connect(action_implement, &QAction::triggered,
			this, &MissionMonitorWidget::on_missionImplementAction_triggered);
	}
	if (action_stop) {
		connect(action_stop, &QAction::triggered,
			this, &MissionMonitorWidget::on_missionStopAction_triggered);
	}

	// 2. 地图动作信号（核心修复：地图缩小绑定到正确的zoomOut函数）
	if (action_zoomIn && m_pS57MapModule) {
		connect(action_zoomIn, &QAction::triggered,
			m_pS57MapModule, &S57MapModule::zoomIn, Qt::UniqueConnection);
	}
	if (action_zoomOut && m_pS57MapModule) {
		// 原错误：绑定到setAuvQueryTool，修正为zoomOut
		connect(action_zoomOut, &QAction::triggered,
			m_pS57MapModule, &S57MapModule::zoomOut, Qt::UniqueConnection);
	}
	if (action_translate && m_pS57MapModule) {
		connect(action_translate, &QAction::triggered,
			m_pS57MapModule, &S57MapModule::setPanTool, Qt::UniqueConnection);
	}

	// 3. 查询动作信号
	if (action_auvQuery && m_pS57MapModule) {
		connect(action_auvQuery, &QAction::triggered,
			m_pS57MapModule, &S57MapModule::setAuvQueryTool, Qt::UniqueConnection);
	}

	// 4. Dock显隐信号（确保动作与Dock状态同步）
	if (action_toggleMissionDock && m_leftMissionDock) {
		connect(action_toggleMissionDock, &QAction::triggered,
			this, &MissionMonitorWidget::on_missionDockAction_triggered);
		connect(m_leftMissionDock, &QDockWidget::visibilityChanged,
			this, [this](bool visible) {
				if (action_toggleMissionDock) action_toggleMissionDock->setChecked(visible);
			});
	}
	if (action_toggleEditorDock && m_rightEditorDock) {
		connect(action_toggleEditorDock, &QAction::triggered,
			this, &MissionMonitorWidget::on_editorAction_triggered);
		connect(m_rightEditorDock, &QDockWidget::visibilityChanged,
			this, [this](bool visible) {
				if (action_toggleEditorDock) action_toggleEditorDock->setChecked(visible);
			});
	}

	// 5. 地图和信息面板信号
	if (m_mapCanvas) {
		connect(m_mapCanvas, &CMapCanvas::canvasRefreshed,
			this, &MissionMonitorWidget::renderCanvas);
	}
	if (m_infoPanel) {
		connect(m_infoPanel.data(), &AuvInfoPanel::closeClicked,
			this, &MissionMonitorWidget::closeAuvInfoPanel);
	}

	// 6. 地图模块数据更新信号
	if (m_pS57MapModule && m_pGeoLabel) {
		connect(m_pS57MapModule, &S57MapModule::updateGeo,
			this, [this](const QString& text) {
				m_pGeoLabel->setText(text);
			}, Qt::UniqueConnection);
	}
	if (m_pS57MapModule && m_pScaleLabel) {
		connect(m_pS57MapModule, &S57MapModule::updateScale,
			this, [this](int scale) {
				m_pScaleLabel->setText(QString("1:%1").arg(scale));
			}, Qt::UniqueConnection);
	}

	// 7. 使命视图选择信号（确保编辑器切换正常）
	// 连接Controller信号
	connect(m_missionController, &MissionController::switchEditor, this, [this](TaskType type, Task* task) {
		m_editorManager->switchEditor(type, task);
		});
	connect(m_missionController, &MissionController::clearEditor, this, [this]() {
		m_editorManager->clearCurrentEditor();
		});
	connect(m_editorManager, &EditorManager::taskModified, m_missionController, &MissionController::onTaskModified);

}

void MissionMonitorWidget::initDockWidgets()
{
	// 左侧任务Dock（内容为m_missionView，必须先初始化业务模块）
	CHECK_COMPONENT(m_missionView, "任务视图为空，无法创建左侧Dock");
	m_leftMissionDock = new QDockWidget("任务视图", this);
	m_leftMissionDock->setObjectName("leftMissionDock");
	m_leftMissionDock->setWidget(m_missionView);
	m_leftMissionDock->setMinimumWidth(400);
	m_leftMissionDock->setMaximumWidth(600);
	m_leftMissionDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
	m_leftMissionDock->setAllowedAreas(Qt::LeftDockWidgetArea);

	// 右侧编辑器Dock（内容为m_editorContainer，确保StackedWidget有内容）
	CHECK_COMPONENT(m_editorContainer, "编辑器容器为空，无法创建右侧Dock");
	m_rightEditorDock = new QDockWidget("编辑器", this);
	m_rightEditorDock->setObjectName("rightEditorDock");
	m_rightEditorDock->setWidget(m_editorContainer);
	m_rightEditorDock->setMinimumWidth(500);
	m_rightEditorDock->setMaximumWidth(600);
	m_rightEditorDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
	m_rightEditorDock->setAllowedAreas(Qt::RightDockWidgetArea);

	// 关键：将Dock加入内容区布局（左侧在前，右侧在后）
	CHECK_COMPONENT(m_contentLayout, "内容区布局为空，无法添加Dock");
	if (m_leftMissionDock) {
		m_contentLayout->insertWidget(0, m_leftMissionDock); // 左侧Dock：索引0
	}
	if (m_rightEditorDock) {
		m_contentLayout->addWidget(m_rightEditorDock);       // 右侧Dock：索引2
	}

	// 关键：将内容区布局加入主布局（在工具栏下方）
	CHECK_COMPONENT(m_mainVLayout, "主布局为空，无法添加内容区");
	m_mainVLayout->addLayout(m_contentLayout, 1); // 权重1：占满剩余空间

	
}

void MissionMonitorWidget::createDockActions()
{
	// 左侧任务Dock显隐控制
	action_toggleMissionDock = new QAction("任务面板", this);
	action_toggleMissionDock->setShortcut(QKeySequence("Ctrl+D"));
	action_toggleMissionDock->setToolTip("显示/隐藏任务面板 (Ctrl+D)");
	action_toggleMissionDock->setCheckable(true);
	action_toggleMissionDock->setChecked(true);
	action_toggleMissionDock->setIcon(QIcon("./icon/monitor/taskpanel.png"));

	// 右侧编辑器Dock显隐控制
	action_toggleEditorDock = new QAction("编辑面板", this);
	action_toggleEditorDock->setShortcut(QKeySequence("Ctrl+E"));
	action_toggleEditorDock->setToolTip("显示/隐藏编辑面板 (Ctrl+E)");
	action_toggleEditorDock->setCheckable(true);
	action_toggleEditorDock->setChecked(true);
	action_toggleEditorDock->setIcon(QIcon("./icon/monitor/editpanel.png"));

	// Dock控制动作的分隔符
	m_dockSep = new QAction(this);
	m_dockSep->setSeparator(true);
}


void MissionMonitorWidget::simulateAuvMovement()
{
	// 使用成员变量替代静态变量，避免线程安全问题
	// 使用成员变量替代静态变量，避免线程安全问题
	m_simulationAngle += 0.05; // 减小角度增量，使运动更平滑

	AuvRealTimeData auv;
	auv.auvId = "AUV-001";
	auv.geoPos = { 108.87541665 + 0.01 * cos(m_simulationAngle),
				  18.3831943 + 0.01 * sin(m_simulationAngle) };
	auv.heading = fmod(m_simulationAngle * 180 / M_PI + 90, 360);
	auv.speed = 1.0 + 0.5 * sin(m_simulationAngle);
	auv.isConnected = true;

	// 更新AUV显示和轨迹（根据实际项目补充）
	m_auvRenderer->updateAuv("AUV-001", auv);
	m_auvRenderer->addActualTrajectoryPoint("AUV-001", ActualTrajectoryPoint(auv.geoPos));
}

void MissionMonitorWidget::stopSimulation()
{
	if (m_simTimer && m_simTimer->isActive()) {
		m_simTimer->stop();
	}
}

void MissionMonitorWidget::closeAuvInfoPanel()
{
	m_infoPanel->setVisible(false);
}