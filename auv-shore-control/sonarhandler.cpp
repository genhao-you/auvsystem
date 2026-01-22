#include "sonarhandler.h"
#include "widgetmanager.h"
#include <QMessageBox>
#include <QDebug>

SonarHandler::SonarHandler(SelfCheckModule* module, QWidget* parent)
    : DeviceHandlerBase(module, parent)
{
    initDefaultDeviceMapping();
    initAllUIControls();
    initializeUI();
}

SonarHandler::~SonarHandler()
{
    disconnectSignals();
}

// 初始化声呐默认编号映射（12=侧扫/13=前视/14=下视）
void SonarHandler::initDefaultDeviceMapping()
{
    m_deviceNumberMap = {
        {DeviceId::SideScanSonar, "12"},
        {DeviceId::ForwardSonar, "13"},
        {DeviceId::DownwardSonar, "14"}
    };
}

// 初始化声呐UI控件
void SonarHandler::SonarUIControls::init(QWidget* parent, const QString& prefix)
{
    // 按钮
    powerOnBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_PowerOn").arg(prefix), parent);
    powerOffBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_PowerOff").arg(prefix), parent);
    enableWorkBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_EnableWork").arg(prefix), parent);
    disableWorkBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_DisableWork").arg(prefix), parent);

    // 状态图标（按需扩展）
    powerOnIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_PowerOnIcon").arg(prefix), parent);
    powerOffIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_PowerOffIcon").arg(prefix), parent);
    enableWorkIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_EnableWorkIcon").arg(prefix), parent);
    disableWorkIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_DisableWorkIcon").arg(prefix), parent);

    // 反馈文本框
    powerOnFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_PowerOn_Feedback").arg(prefix), parent);
    powerOffFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_PowerOff_Feedback").arg(prefix), parent);
    enableWorkFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_EnableWork_Feedback").arg(prefix), parent);
    disableWorkFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_DisableWork_Feedback").arg(prefix), parent);
}

void SonarHandler::initAllUIControls()
{
    for (auto it = m_deviceNumberMap.begin(); it != m_deviceNumberMap.end(); ++it) {
        SonarUIControls controls;
        controls.number = it.value();
        controls.init(m_parent, controls.number);
        m_uiControls[it.key()] = controls;
    }
}

// 初始化UI（清空反馈框）
void SonarHandler::initializeUI()
{
    for (auto& controls : m_uiControls) {
        if (controls.powerOnFeedback) controls.powerOnFeedback->clear();
        if (controls.powerOffFeedback) controls.powerOffFeedback->clear();
        if (controls.enableWorkFeedback) controls.enableWorkFeedback->clear();
        if (controls.disableWorkFeedback) controls.disableWorkFeedback->clear();
    }
}

// 连接信号（先断后连，避免重复）
void SonarHandler::connectSignals()
{
    disconnectSignals();

    for (auto it = m_uiControls.begin(); it != m_uiControls.end(); ++it) {
        DeviceId deviceId = it.key();
        SonarUIControls& controls = it.value();

        qDebug() << "[声呐信号连接] DeviceId:" << static_cast<int>(deviceId)
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
        if (controls.enableWorkBtn) {
            connect(controls.enableWorkBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onEnableWorkClicked(deviceId); });
        }
        if (controls.disableWorkBtn) {
            connect(controls.disableWorkBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onDisableWorkClicked(deviceId); });
        }
    }
}

// 断开所有信号
void SonarHandler::disconnectSignals()
{
    for (auto& controls : m_uiControls) {
        if (controls.powerOnBtn) controls.powerOnBtn->disconnect();
        if (controls.powerOffBtn) controls.powerOffBtn->disconnect();
        if (controls.enableWorkBtn) controls.enableWorkBtn->disconnect();
        if (controls.disableWorkBtn) controls.disableWorkBtn->disconnect();
    }
}

// 统一处理声呐命令结果
void SonarHandler::handleResult(const SelfCheckResult* result)
{
    if (!result) return;

    DeviceId deviceId = result->deviceId();
    if (!m_uiControls.contains(deviceId)) {
        qWarning() << "不支持的声呐设备:" << static_cast<uint8_t>(deviceId);
        return;
    }

    processSonarCommand(deviceId, result->commandCode(), result);
}

// 分发处理不同命令结果
void SonarHandler::processSonarCommand(DeviceId deviceId, CommandCode cmdCode, const SelfCheckResult* result)
{
    const SonarUIControls& controls = m_uiControls[deviceId];
    ResultStatus status = result->status();
    QPixmap icon = getStatusIcon(status);
    QString feedbackDesc = result->feedbackDesc();

    switch (cmdCode) {
    case CommandCode::Common_PowerOn:
        setIcon(controls.powerOnIcon, icon);
        updateStatusWidget(controls.powerOnFeedback, status, feedbackDesc);
        break;
    case CommandCode::Common_PowerOff:
        setIcon(controls.powerOffIcon, icon);
        updateStatusWidget(controls.powerOffFeedback, status, feedbackDesc);
        break;
    case CommandCode::Common_Enable:
        setIcon(controls.enableWorkIcon, icon);
        updateStatusWidget(controls.enableWorkFeedback, status, feedbackDesc);
        break;
    case CommandCode::Common_Disable:
        setIcon(controls.disableWorkIcon, icon);
        updateStatusWidget(controls.disableWorkFeedback, status, feedbackDesc);
        break;
    default:
        qDebug() << "未处理的声呐命令:" << static_cast<int>(cmdCode);
        break;
    }
}

// 获取设备编号
QString SonarHandler::getDeviceNumber(DeviceId deviceId) const
{
    return m_deviceNumberMap.value(deviceId, "");
}

// 支持的设备
bool SonarHandler::supportsDevice(DeviceId deviceId) const
{
    return m_deviceNumberMap.contains(deviceId);
}

// 设置设备映射
void SonarHandler::setDeviceMapping(const QMap<DeviceId, QString>& mapping)
{
    m_deviceNumberMap = mapping;
    initAllUIControls();
}

// -------------------- 槽函数实现 --------------------
void SonarHandler::onPowerOnClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Common_PowerOn, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误",
            QString("%1上电命令失败: %2").arg(getDeviceNumber(deviceId)).arg(e.what()));
    }
}

void SonarHandler::onPowerOffClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Common_PowerOff, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误",
            QString("%1下电命令失败: %2").arg(getDeviceNumber(deviceId)).arg(e.what()));
    }
}

void SonarHandler::onEnableWorkClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Common_Enable, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误",
            QString("%1启用命令失败: %2").arg(getDeviceNumber(deviceId)).arg(e.what()));
    }
}

void SonarHandler::onDisableWorkClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Common_Disable, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误",
            QString("%1禁用命令失败: %2").arg(getDeviceNumber(deviceId)).arg(e.what()));
    }
}