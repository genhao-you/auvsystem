#include "camerahandler.h"
#include "widgetmanager.h"
#include <QMessageBox>
#include <QDebug>

CameraHandler::CameraHandler(SelfCheckModule* module, QWidget* parent)
    : DeviceHandlerBase(module, parent)
{
    initDefaultDeviceMapping();
    initAllUIControls();
    initializeUI();
}

CameraHandler::~CameraHandler()
{
    disconnectSignals();
}

// 初始化摄像头默认编号映射（0F）
void CameraHandler::initDefaultDeviceMapping()
{
    m_deviceNumberMap = { {DeviceId::CameraLight, "0F"} };
}

// 初始化摄像头UI控件
void CameraHandler::CameraUIControls::init(QWidget* parent, const QString& prefix)
{
    // 按钮
    powerOnBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_PowerOn").arg(prefix), parent);
    powerOffBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_PowerOff").arg(prefix), parent);
    startRecordBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_StartRecord").arg(prefix), parent);
    stopRecordBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_StopRecord").arg(prefix), parent);
    takePhotoBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_TakePhoto").arg(prefix), parent);

    // 图标
    powerOnIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_PowerOnIcon").arg(prefix), parent);
    powerOffIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_PowerOffIcon").arg(prefix), parent);
    startRecordIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_StartRecordIcon").arg(prefix), parent);
    stopRecordIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_StopRecordIcon").arg(prefix), parent);
    takePhotoIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_TakePhotoIcon").arg(prefix), parent);

    // 反馈框
    powerOnFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_PowerOn_Feedback").arg(prefix), parent);
    powerOffFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_PowerOff_Feedback").arg(prefix), parent);
    startRecordFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_StartRecord_Feedback").arg(prefix), parent);
    stopRecordFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_StopRecord_Feedback").arg(prefix), parent);
    takePhotoFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_Photo_Feedback").arg(prefix), parent);
}

void CameraHandler::initAllUIControls()
{
    for (auto it = m_deviceNumberMap.begin(); it != m_deviceNumberMap.end(); ++it) {
        CameraUIControls controls;
        controls.number = it.value();
        controls.init(m_parent, controls.number);
        m_uiControls[it.key()] = controls;
    }
}

// 初始化UI（清空反馈框）
void CameraHandler::initializeUI()
{
    for (auto& controls : m_uiControls) {
        if (controls.powerOnFeedback) controls.powerOnFeedback->clear();
        if (controls.powerOffFeedback) controls.powerOffFeedback->clear();
        if (controls.startRecordFeedback) controls.startRecordFeedback->clear();
        if (controls.stopRecordFeedback) controls.stopRecordFeedback->clear();
        if (controls.takePhotoFeedback) controls.takePhotoFeedback->clear();
    }
}

// 连接信号（先断后连，避免重复）
void CameraHandler::connectSignals()
{
    disconnectSignals();

    for (auto it = m_uiControls.begin(); it != m_uiControls.end(); ++it) {
        DeviceId deviceId = it.key();
        CameraUIControls& controls = it.value();

        qDebug() << "[摄像头信号连接] DeviceId:" << static_cast<int>(deviceId)
            << " 前缀:" << controls.number;

        // 连接按钮信号
        if (controls.powerOnBtn) {
            connect(controls.powerOnBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onPowerOnClicked(deviceId); });
        }
        if (controls.powerOffBtn) {
            connect(controls.powerOffBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onPowerOffClicked(deviceId); });
        }
        if (controls.startRecordBtn) {
            connect(controls.startRecordBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onStartRecordClicked(deviceId); });
        }
        if (controls.stopRecordBtn) {
            connect(controls.stopRecordBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onStopRecordClicked(deviceId); });
        }
        if (controls.takePhotoBtn) {
            connect(controls.takePhotoBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onTakePhotoClicked(deviceId); });
        }
    }
}

// 断开所有信号
void CameraHandler::disconnectSignals()
{
    for (auto& controls : m_uiControls) {
        if (controls.powerOnBtn) controls.powerOnBtn->disconnect();
        if (controls.powerOffBtn) controls.powerOffBtn->disconnect();
        if (controls.startRecordBtn) controls.startRecordBtn->disconnect();
        if (controls.stopRecordBtn) controls.stopRecordBtn->disconnect();
        if (controls.takePhotoBtn) controls.takePhotoBtn->disconnect();
    }
}

// 统一处理摄像头命令结果
void CameraHandler::handleResult(const SelfCheckResult* result)
{
    if (!result) return;

    DeviceId deviceId = result->deviceId();
    if (!m_uiControls.contains(deviceId)) {
        qWarning() << "不支持的摄像头设备:" << static_cast<uint8_t>(deviceId);
        return;
    }

    processCameraCommand(deviceId, result->commandCode(), result);
}

// 分发处理不同命令结果
void CameraHandler::processCameraCommand(DeviceId deviceId, CommandCode cmdCode, const SelfCheckResult* result)
{
    const CameraUIControls& controls = m_uiControls[deviceId];
    ResultStatus status = result->status();
    QPixmap icon = getStatusIcon(status);
    QString feedbackDesc = result->feedbackDesc();

    switch (cmdCode) {
    case CommandCode::Common_PowerOn:{
    
        setIcon(controls.powerOnIcon, icon);
        updateStatusWidget(controls.powerOnFeedback, status, feedbackDesc);
        break;
    }
       
    case CommandCode::Common_PowerOff: {
        setIcon(controls.powerOffIcon, icon);
        updateStatusWidget(controls.powerOffFeedback, status, feedbackDesc);
        break;
    }
    case CommandCode::CameraLight_StartVideoSave: {
        setIcon(controls.startRecordIcon, icon);
        updateStatusWidget(controls.startRecordFeedback, status, feedbackDesc);
        break;
    }
    case CommandCode::CameraLight_StopVideoSave: {
        setIcon(controls.stopRecordIcon, icon);
        updateStatusWidget(controls.stopRecordFeedback, status, feedbackDesc);
        break; 
    }
    case CommandCode::CameraLight_TakePhoto: {
        setIcon(controls.takePhotoIcon, icon);
        QString photoPath = result->getParameter("path").toString();
        updateStatusWidget(controls.takePhotoFeedback, status, feedbackDesc);
        break; 
    }
    default:
        qDebug() << "未处理的摄像头命令:" << static_cast<int>(cmdCode);
        break;
    }
}

// 获取设备编号
QString CameraHandler::getDeviceNumber(DeviceId deviceId) const
{
    return m_deviceNumberMap.value(deviceId, "");
}

// 支持的设备
bool CameraHandler::supportsDevice(DeviceId deviceId) const
{
    return m_deviceNumberMap.contains(deviceId);
}

// 设置设备映射
void CameraHandler::setDeviceMapping(const QMap<DeviceId, QString>& mapping)
{
    m_deviceNumberMap = mapping;
    initAllUIControls();
}

// 获取下一个媒体序列（复用原有逻辑）
int CameraHandler::getNextCameraMediaSeq()
{
    return ++m_mediaSeq; // 简单自增，可根据实际需求调整
}

// -------------------- 槽函数实现 --------------------
void CameraHandler::onPowerOnClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Common_PowerOn, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("摄像头上电命令失败: %1").arg(e.what()));
    }
}

void CameraHandler::onPowerOffClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Common_PowerOff, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("摄像头下电命令失败: %1").arg(e.what()));
    }
}

void CameraHandler::onStartRecordClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<CameraMediaParam>(getNextCameraMediaSeq());
        m_module->sendCheckCommand(deviceId, CommandCode::CameraLight_StartVideoSave, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("摄像头开始录像命令失败: %1").arg(e.what()));
    }
}

void CameraHandler::onStopRecordClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::CameraLight_StopVideoSave, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("摄像头停止录像命令失败: %1").arg(e.what()));
    }
}

void CameraHandler::onTakePhotoClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<CameraMediaParam>(getNextCameraMediaSeq());
        m_module->sendCheckCommand(deviceId, CommandCode::CameraLight_TakePhoto, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("摄像头拍照命令失败: %1").arg(e.what()));
    }
}