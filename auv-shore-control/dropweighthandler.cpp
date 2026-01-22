#include "dropweighthandler.h"
#include "widgetmanager.h"
#include <QMessageBox>
#include <QDebug>

DropWeightHandler::DropWeightHandler(SelfCheckModule* module, QWidget* parent)
    : DeviceHandlerBase(module, parent)
{
    initDefaultDeviceMapping();
    initAllUIControls();
    initializeUI();
}

DropWeightHandler::~DropWeightHandler()
{
    disconnectSignals();
}

// 初始化抛载默认编号映射（07）
void DropWeightHandler::initDefaultDeviceMapping()
{
    m_deviceNumberMap = { {DeviceId::DropWeight, "07"} };
}

// 初始化抛载UI控件
void DropWeightHandler::DropWeightUIControls::init(QWidget* parent, const QString& prefix)
{
    // 命令按钮
    powerOnBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_PowerOn").arg(prefix), parent);
    powerOffBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_PowerOff").arg(prefix), parent);
    releaseBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_Release").arg(prefix), parent);
    resetBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_Reset").arg(prefix), parent);
    getFaultBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_StatusCheck").arg(prefix), parent);

    // 状态图标
    powerOnIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_PowerOnIcon").arg(prefix), parent);
    powerOffIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_PowerOffIcon").arg(prefix), parent);
    releaseIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_ReleaseIcon").arg(prefix), parent);
    resetIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_ResetIcon").arg(prefix), parent);
    getFaultIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_StatusCheckIcon").arg(prefix), parent);

    // 反馈文本框
    powerOnFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_PowerOn_Feedback").arg(prefix), parent);
    powerOffFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_PowerOff_Feedback").arg(prefix), parent);
    releaseFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_Release_Feedback").arg(prefix), parent);
    resetFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_Reset_Feedback").arg(prefix), parent);
    getFaultFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_StatusCheck_Feedback").arg(prefix), parent);
}

void DropWeightHandler::initAllUIControls()
{
    for (auto it = m_deviceNumberMap.begin(); it != m_deviceNumberMap.end(); ++it) {
        DropWeightUIControls controls;
        controls.number = it.value();
        controls.init(m_parent, controls.number);
        m_uiControls[it.key()] = controls;
    }
}

// 初始化UI（清空反馈框）
void DropWeightHandler::initializeUI()
{
    for (auto& controls : m_uiControls) {
        if (controls.powerOnFeedback) controls.powerOnFeedback->clear();
        if (controls.powerOffFeedback) controls.powerOffFeedback->clear();
        if (controls.releaseFeedback) controls.releaseFeedback->clear();
        if (controls.resetFeedback) controls.resetFeedback->clear();
        if (controls.getFaultFeedback) controls.getFaultFeedback->clear();
    }
}

// 连接信号（先断后连，避免重复）
void DropWeightHandler::connectSignals()
{
    disconnectSignals();

    for (auto it = m_uiControls.begin(); it != m_uiControls.end(); ++it) {
        DeviceId deviceId = it.key();
        DropWeightUIControls& controls = it.value();

        qDebug() << "[抛载信号连接] DeviceId:" << static_cast<int>(deviceId)
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
        if (controls.releaseBtn) {
            connect(controls.releaseBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onReleaseClicked(deviceId); });
        }
        if (controls.resetBtn) {
            connect(controls.resetBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onResetClicked(deviceId); });
        }
        if (controls.getFaultBtn) {
            connect(controls.getFaultBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetFaultClicked(deviceId); });
        }
    }
}

// 断开所有信号
void DropWeightHandler::disconnectSignals()
{
    for (auto& controls : m_uiControls) {
        if (controls.powerOnBtn) controls.powerOnBtn->disconnect();
        if (controls.powerOffBtn) controls.powerOffBtn->disconnect();
        if (controls.releaseBtn) controls.releaseBtn->disconnect();
        if (controls.resetBtn) controls.resetBtn->disconnect();
        if (controls.getFaultBtn) controls.getFaultBtn->disconnect();
    }
}

// 统一处理抛载命令结果
void DropWeightHandler::handleResult(const SelfCheckResult* result)
{
    if (!result) return;

    DeviceId deviceId = result->deviceId();
    if (!m_uiControls.contains(deviceId)) {
        qWarning() << "不支持的抛载设备:" << static_cast<uint8_t>(deviceId);
        return;
    }

    processDropWeightCommand(deviceId, result->commandCode(), result);
}

// 分发处理不同抛载命令结果
void DropWeightHandler::processDropWeightCommand(DeviceId deviceId, CommandCode cmdCode, const SelfCheckResult* result)
{
    const DropWeightUIControls& controls = m_uiControls[deviceId];
    ResultStatus status = result->status();
    QPixmap icon = getStatusIcon(status);

    switch (cmdCode) {
    case CommandCode::Common_PowerOn:
        setIcon(controls.powerOnIcon, icon);
        updateStatusWidget(controls.powerOnFeedback, status);
        break;
    case CommandCode::Common_PowerOff:
        setIcon(controls.powerOffIcon, icon);
        updateStatusWidget(controls.powerOffFeedback, status);
        break;
    case CommandCode::DropWeight_Release:
        setIcon(controls.releaseIcon, icon);
        updateStatusWidget(controls.releaseFeedback, status);
        break;
    case CommandCode::DropWeight_Reset:
        setIcon(controls.resetIcon, icon);
        updateStatusWidget(controls.resetFeedback, status);
        break;
    case CommandCode::Common_GetFault:
        setIcon(controls.getFaultIcon, icon);
        updateStatusWidget(controls.getFaultFeedback, status);
        break;
    default:
        qDebug() << "未处理的抛载命令:" << static_cast<int>(cmdCode);
        break;
    }
}

// 获取设备编号
QString DropWeightHandler::getDeviceNumber(DeviceId deviceId) const
{
    return m_deviceNumberMap.value(deviceId, "");
}

// 支持的设备
bool DropWeightHandler::supportsDevice(DeviceId deviceId) const
{
    return m_deviceNumberMap.contains(deviceId);
}

// 设置设备映射
void DropWeightHandler::setDeviceMapping(const QMap<DeviceId, QString>& mapping)
{
    m_deviceNumberMap = mapping;
    initAllUIControls();
}

// -------------------- 槽函数实现 --------------------
void DropWeightHandler::onPowerOnClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Common_PowerOn, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("抛载上电命令失败: %1").arg(e.what()));
    }
}

void DropWeightHandler::onPowerOffClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Common_PowerOff, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("抛载下电命令失败: %1").arg(e.what()));
    }
}

void DropWeightHandler::onReleaseClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::DropWeight_Release, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("抛载释放命令失败: %1").arg(e.what()));
    }
}

void DropWeightHandler::onResetClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::DropWeight_Reset, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("抛载复位命令失败: %1").arg(e.what()));
    }
}

void DropWeightHandler::onGetFaultClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Common_GetFault, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("抛载故障查询命令失败: %1").arg(e.what()));
    }
}