#include "depthmeterhandler.h"
#include "widgetmanager.h"
#include <QMessageBox>
#include <QDebug>

DepthMeterHandler::DepthMeterHandler(SelfCheckModule* module, QWidget* parent)
    : DeviceHandlerBase(module, parent)
{
    initDefaultDeviceMapping();
    initAllUIControls();
    initializeUI();
}

DepthMeterHandler::~DepthMeterHandler()
{
    disconnectSignals();
}

// 初始化深度计默认编号映射（0A）
void DepthMeterHandler::initDefaultDeviceMapping()
{
    m_deviceNumberMap = { {DeviceId::DepthMeter, "0A"} };
}

// 初始化深度计UI控件
void DepthMeterHandler::DepthMeterUIControls::init(QWidget* parent, const QString& prefix)
{
    // 按钮
    powerOnBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_PowerOn").arg(prefix), parent);
    powerOffBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_PowerOff").arg(prefix), parent);
    calibrateBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_SetReference").arg(prefix), parent);
    getDepthBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_ReadDepth").arg(prefix), parent);

    // 图标
    powerOnIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_PowerOnIcon").arg(prefix), parent);
    powerOffIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_PowerOffIcon").arg(prefix), parent);
    calibrateIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_ReferenceIcon").arg(prefix), parent);
    getDepthIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_ReadDepthIcon").arg(prefix), parent);

    // 反馈框
    powerOnFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_PowerOn_Feedback").arg(prefix), parent);
    powerOffFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_PowerOff_Feedback").arg(prefix), parent);
    calibrateFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_Reference_Feedback").arg(prefix), parent);
    getDepthFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadDepth_Feedback").arg(prefix), parent);
}

void DepthMeterHandler::initAllUIControls()
{
    for (auto it = m_deviceNumberMap.begin(); it != m_deviceNumberMap.end(); ++it) {
        DepthMeterUIControls controls;
        controls.number = it.value();
        controls.init(m_parent, controls.number);
        m_uiControls[it.key()] = controls;
    }
}

// 初始化UI（清空反馈框）
void DepthMeterHandler::initializeUI()
{
    for (auto& controls : m_uiControls) {
        if (controls.powerOnFeedback) controls.powerOnFeedback->clear();
        if (controls.powerOffFeedback) controls.powerOffFeedback->clear();
        if (controls.calibrateFeedback) controls.calibrateFeedback->clear();
        if (controls.getDepthFeedback) controls.getDepthFeedback->clear();
    }
}

// 连接信号（先断后连，避免重复）
void DepthMeterHandler::connectSignals()
{
    disconnectSignals();

    for (auto it = m_uiControls.begin(); it != m_uiControls.end(); ++it) {
        DeviceId deviceId = it.key();
        DepthMeterUIControls& controls = it.value();

        qDebug() << "[深度计信号连接] DeviceId:" << static_cast<int>(deviceId)
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
        if (controls.calibrateBtn) {
            connect(controls.calibrateBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onCalibrateClicked(deviceId); });
        }
        if (controls.getDepthBtn) {
            connect(controls.getDepthBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetDepthClicked(deviceId); });
        }
    }
}

// 断开所有信号
void DepthMeterHandler::disconnectSignals()
{
    for (auto& controls : m_uiControls) {
        if (controls.powerOnBtn) controls.powerOnBtn->disconnect();
        if (controls.powerOffBtn) controls.powerOffBtn->disconnect();
        if (controls.calibrateBtn) controls.calibrateBtn->disconnect();
        if (controls.getDepthBtn) controls.getDepthBtn->disconnect();
    }
}

// 统一处理深度计命令结果
void DepthMeterHandler::handleResult(const SelfCheckResult* result)
{
    if (!result) return;

    DeviceId deviceId = result->deviceId();
    if (!m_uiControls.contains(deviceId)) {
        qWarning() << "不支持的深度计设备:" << static_cast<uint8_t>(deviceId);
        return;
    }

    processDepthMeterCommand(deviceId, result->commandCode(), result);
}

// 分发处理不同命令结果
void DepthMeterHandler::processDepthMeterCommand(DeviceId deviceId, CommandCode cmdCode, const SelfCheckResult* result)
{
    const DepthMeterUIControls& controls = m_uiControls[deviceId];
    ResultStatus status = result->status();
    QPixmap icon = getStatusIcon(status);
    QString feedbackDesc = result->feedbackDesc();

    switch (cmdCode) {
    case CommandCode::Common_PowerOn:{
        setIcon(controls.powerOnIcon, icon);
        updateStatusWidget(controls.powerOnFeedback, status, feedbackDesc);
        break;
    }
    case CommandCode::Common_PowerOff:{
        setIcon(controls.powerOffIcon, icon);
        updateStatusWidget(controls.powerOffFeedback, status, feedbackDesc);
        break;
    }
    case CommandCode::Depthgauge_Calibration:{
        setIcon(controls.calibrateIcon, icon);
        updateStatusWidget(controls.calibrateFeedback, status, feedbackDesc);
        break;
    }
    case CommandCode::Depthgauge_GetDepth:{
        setIcon(controls.getDepthIcon, icon);
        double depth = result->getParameter("depth").toDouble();
        updateStatusWidget(controls.getDepthFeedback, status, QString("%1 米").arg(depth, 0, 'f', 4));
        break;
    }
    default:{
        qDebug() << "未处理的深度计命令:" << static_cast<int>(cmdCode);
        break;
    }
    }
}

// 获取设备编号
QString DepthMeterHandler::getDeviceNumber(DeviceId deviceId) const
{
    return m_deviceNumberMap.value(deviceId, "");
}

// 支持的设备
bool DepthMeterHandler::supportsDevice(DeviceId deviceId) const
{
    return m_deviceNumberMap.contains(deviceId);
}

// 设置设备映射
void DepthMeterHandler::setDeviceMapping(const QMap<DeviceId, QString>& mapping)
{
    m_deviceNumberMap = mapping;
    initAllUIControls();
}

// -------------------- 槽函数实现 --------------------
void DepthMeterHandler::onPowerOnClicked(DeviceId deviceId)
{
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Common_PowerOn, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("深度计上电命令失败: %1").arg(e.what()));
    }
}

void DepthMeterHandler::onPowerOffClicked(DeviceId deviceId)
{
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Common_PowerOff, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("深度计下电命令失败: %1").arg(e.what()));
    }
}

void DepthMeterHandler::onCalibrateClicked(DeviceId deviceId)
{
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Depthgauge_Calibration, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("深度计校准命令失败: %1").arg(e.what()));
    }
}

void DepthMeterHandler::onGetDepthClicked(DeviceId deviceId)
{
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Depthgauge_GetDepth, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("获取深度命令失败: %1").arg(e.what()));
    }
}
