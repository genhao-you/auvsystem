#include "auvshorecontrol.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>
#include <cmath>
#include <qDebug>

// 日志宏定义，便于调试和问题定位
#define LOG_DEBUG(msg) qDebug() << QString("[%1] DEBUG: %2").arg(QDateTime::currentDateTime().toString("HH:mm:ss")).arg(msg)
#define LOG_ERROR(msg) qCritical() << QString("[%1] ERROR: %2").arg(QDateTime::currentDateTime().toString("HH:mm:ss")).arg(msg)

AuvShoreControl::AuvShoreControl(QWidget* parent)
    : QMainWindow(parent)
    ,m_s57MapModule(new S57MapModule(this))
    ,m_communicationModule(&CommunicationModule::instance())
    ,m_wdtMissionMonitor(new MissionMonitorWidget(new MissionMonitorModule(this),m_s57MapModule))
    ,m_dlgControlParamSetting(new ControlParamSettingDialog(new ControlParamModule))
    ,m_sailControlModule(new SailControlModule(this))
   
    

{
    ui.setupUi(this);
    initialization();
    LOG_DEBUG("AuvShoreControl initialized successfully");
}

AuvShoreControl::~AuvShoreControl()
{
    
    

    if (m_dlgSelfCheck != nullptr)
        delete m_dlgSelfCheck;
    m_dlgSelfCheck = nullptr;

    if (m_dlgCommunicationsSetting != nullptr)
        delete m_dlgCommunicationsSetting;
    m_dlgCommunicationsSetting = nullptr;

    if (m_dlgDeviceMonitor != nullptr)
        delete m_dlgDeviceMonitor;
    m_dlgDeviceMonitor = nullptr;

    if (m_dlgTaskMonitor != nullptr)
        delete m_dlgTaskMonitor;
    m_dlgTaskMonitor = nullptr;

    if (m_dlgDataQuery != nullptr)
        delete m_dlgDataQuery;
    m_dlgDataQuery = nullptr;

    if (m_dlgDataDownload != nullptr)
        delete m_dlgDataDownload;
    m_dlgDataDownload = nullptr;

    if (m_dlgSailRestriction != nullptr)
        delete m_dlgSailRestriction;
    m_dlgSailRestriction = nullptr;

    


    if (m_s57MapModule != nullptr)
        delete m_s57MapModule;
    m_s57MapModule = nullptr;

    if (m_wdtMissionMonitor != nullptr)
        delete m_wdtMissionMonitor;
    m_wdtMissionMonitor = nullptr;
    // 强制垃圾回收
    qApp->processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}


void AuvShoreControl::initialization()
{
    // 按顺序初始化，确保依赖正确
   
    setupUIComponents();
    setupConnections();
   
}



void AuvShoreControl::setupUIComponents()
{
    // 设置中心部件
    setMouseTracking(true);
    // 初始化定时器，设置间隔为1000毫秒（1秒）
    timeTimer = new QTimer(this);
    timeTimer->setInterval(1000);  // 1秒触发一次

    // 连接定时器的timeout信号到更新时间的槽函数
    connect(timeTimer, &QTimer::timeout, this, &AuvShoreControl::updateCurrentTime);

    // 启动定时器（程序运行时立即开始更新时间）
    timeTimer->start();

    // 初始化时先更新一次时间（避免首次显示空白）
    updateCurrentTime();
    // 加载QSS样式
    ModuleStyleHelper::loadStyleSheet(STYLE_MAINWINDOW,this);
    QIcon windowIcon("./icon/mainwindow/auv_displaycontrol.png");
    setWindowIcon(windowIcon);
}

void AuvShoreControl::setupConnections()
{
    // 使用Qt::UniqueConnection防止重复连接
    connect(ui.btnEnableConfig, &QPushButton::clicked,
        this, &AuvShoreControl::on_selfCheckDlgAction_triggered,
        Qt::UniqueConnection);

    connect(ui.btnCommunicationSettinng, &QPushButton::clicked,
        this, &AuvShoreControl::on_communicationSettingDlgAction_triggered,
        Qt::UniqueConnection);

    connect(ui.action_deviceStatus, &QAction::triggered,
        this, &AuvShoreControl::on_deviceStatusAction_triggered,
        Qt::UniqueConnection);

    connect(ui.action_taskMonitor, &QAction::triggered,
        this, &AuvShoreControl::on_taskMonitorAction_triggered,
        Qt::UniqueConnection);

    connect(ui.btnControlParamDebug, &QPushButton::clicked,
        this, &AuvShoreControl::on_controlDebugAction_triggered,
        Qt::UniqueConnection);

    connect(ui.btnDataDownload, &QPushButton::clicked,
        this, &AuvShoreControl::on_dataDownloadBtn_triggered,
        Qt::UniqueConnection);

    connect(ui.btnStartSail, &QPushButton::clicked,
        this, &AuvShoreControl::on_startSailBtn_clicked,
        Qt::UniqueConnection);

    connect(ui.action_dataSelect, &QAction::triggered,
        this, &AuvShoreControl::on_dataSelectAction_triggered,
        Qt::UniqueConnection);

    connect(ui.action_sailRestriction, &QAction::triggered,
        this, &AuvShoreControl::on_sailRestrictionAction_triggered,
        Qt::UniqueConnection);

    // 文件菜单连接
    connect(ui.action_missionMonitor, &QAction::triggered,
        this, &AuvShoreControl::on_missionMonitorAction_triggered,
        Qt::UniqueConnection);
    connect(ui.btnMissionReset, &QPushButton::clicked,
        this, &AuvShoreControl::on_missionMonitorAction_triggered,
        Qt::UniqueConnection);

    connect(ui.action_openMission, &QAction::triggered,
        this, &AuvShoreControl::on_openMissionAction_triggered,
        Qt::UniqueConnection);

    
}



bool AuvShoreControl::checkModuleValidity() const
{
    // 检查核心模块是否有效
    if (!m_s57MapModule) {
        QMessageBox::critical(const_cast<AuvShoreControl*>(this), "错误", "地图模块未初始化");
        return false;
    }
    return true;
}

void AuvShoreControl::on_openMissionAction_triggered()
{
    if (!checkModuleValidity()) return;

    QString strCellDir = QFileDialog::getExistingDirectory(this, "选择图幅目录", "/");
    if (strCellDir.isEmpty()) {
        QMessageBox::information(this, "提示", "请输入图幅目录");
        return;
    }

    LOG_DEBUG(QString("加载地图目录: %1").arg(strCellDir));
    m_s57MapModule->loadMaps(strCellDir);
}

void AuvShoreControl::on_selfCheckDlgAction_triggered()
{
    // 延迟初始化：第一次使用时才创建
    if (!m_dlgSelfCheck) {
        m_dlgSelfCheck = new SelfCheckMainDialog_533(new SelfCheckModule(this), this);
    }
    m_dlgSelfCheck->show();
}

void AuvShoreControl::on_communicationSettingDlgAction_triggered()
{
    if (!m_dlgCommunicationsSetting) {
        m_dlgCommunicationsSetting = new CommunicationsSettingDialog(m_communicationModule,this);
    }
    m_dlgCommunicationsSetting->show();
}

void AuvShoreControl::on_deviceStatusAction_triggered()
{
    if (!m_dlgDeviceMonitor) {
        m_dlgDeviceMonitor = new DeviceMonitorDialog(this);
    }
    m_dlgDeviceMonitor->show();
}

void AuvShoreControl::on_taskMonitorAction_triggered()
{
    if (!m_dlgTaskMonitor) {
        m_dlgTaskMonitor = new TaskMonitorDialog(this);
    }
    m_dlgTaskMonitor->show();
}

void AuvShoreControl::on_controlDebugAction_triggered()
{
    if (!m_dlgControlParamSetting) {
        m_dlgControlParamSetting = new ControlParamSettingDialog(new ControlParamModule,this);
    }
    m_dlgControlParamSetting->show();
}




void AuvShoreControl::on_dataSelectAction_triggered()
{
    if (!m_dlgDataQuery) {
        m_dlgDataQuery = new DataQueryDialog(this);
    }
    m_dlgDataQuery->show();
}

void AuvShoreControl::on_dataDownloadBtn_triggered()
{
    if (!m_dlgDataDownload) {
        m_dlgDataDownload = new DataDownloadDialog(new DataModule(this),this);
    }
    m_dlgDataDownload->show();
}

void AuvShoreControl::on_sailRestrictionAction_triggered()
{
    if (!m_dlgSailRestriction) {
        m_dlgSailRestriction = new SailRestrictionDialog(this);
    }
    m_dlgSailRestriction->show();
}

void AuvShoreControl::on_startSailBtn_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认", "确认开始航行？",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // 发送开始航行命令（使用串口通道）
        m_sailControlModule->sendSailCommand(SailCommandCode::Sail_Start,
            CommunicationChannel::Radio);
    }
}




void AuvShoreControl::on_missionMonitorAction_triggered()
{
    m_wdtMissionMonitor->show();
    m_s57MapModule->doRender();
}

void AuvShoreControl::updateCurrentTime()
{
    // 获取当前系统时间，并格式化为 "年-月-日 时:分:秒" 格式
    QDateTime currentTime = QDateTime::currentDateTime();
    QString timeStr = currentTime.toString("yyyy-MM-dd HH:mm:ss");

    // 将格式化后的时间字符串设置到 lblCurrentTime 标签
    ui.lblCurrentTime->setText(timeStr);
}
