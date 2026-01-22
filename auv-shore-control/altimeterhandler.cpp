#include "altimeterhandler.h"
#include "widgetmanager.h"
#include <QMessageBox>
#include <QDebug>

AltimeterHandler::AltimeterHandler(SelfCheckModule* module, QWidget* parent)
    : DeviceHandlerBase(module, parent)
{
    initDefaultDeviceMapping();
    initAllUIControls();
    initializeUI();
}

AltimeterHandler::~AltimeterHandler()
{
    disconnectSignals();
}

// 初始化高度计默认编号映射（0B）
void AltimeterHandler::initDefaultDeviceMapping()
{
    m_deviceNumberMap = { {DeviceId::Altimeter, "0B"} };
}

// 初始化高度计UI控件
void AltimeterHandler::AltimeterUIControls::init(QWidget* parent, const QString& prefix)
{
    // 按钮
    powerOnBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_PowerOn").arg(prefix), parent);
    powerOffBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_PowerOff").arg(prefix), parent);
    getHeightBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_ReadHeight").arg(prefix), parent);

    // 图标
    powerOnIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_PowerOnIcon").arg(prefix), parent);
    powerOffIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_PowerOffIcon").arg(prefix), parent);
    getHeightIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_ReadHeightIcon").arg(prefix), parent);

    // 反馈框
    powerOnFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_PowerOn_Feedback").arg(prefix), parent);
    powerOffFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_PowerOff_Feedback").arg(prefix), parent);
    getHeightFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadHeight_Feedback").arg(prefix), parent);
}

void AltimeterHandler::initAllUIControls()
{
    for (auto it = m_deviceNumberMap.begin(); it != m_deviceNumberMap.end(); ++it) {
        AltimeterUIControls controls;
        controls.number = it.value();
        controls.init(m_parent, controls.number);
        m_uiControls[it.key()] = controls;
    }
}

// 初始化UI
void AltimeterHandler::initializeUI()
{
    for (auto& controls : m_uiControls) {
        if (controls.powerOnFeedback) controls.powerOnFeedback->clear();
        if (controls.powerOffFeedback) controls.powerOffFeedback->clear();
        if (controls.getHeightFeedback) controls.getHeightFeedback->clear();
    }
}

// 连接信号
void AltimeterHandler::connectSignals()
{
    disconnectSignals();

    for (auto it = m_uiControls.begin(); it != m_uiControls.end(); ++it) {
        DeviceId deviceId = it.key();
        AltimeterUIControls& controls = it.value();

        qDebug() << "[高度计信号连接] DeviceId:" << static_cast<int>(deviceId)
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
        if (controls.getHeightBtn) {
            connect(controls.getHeightBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetHeightClicked(deviceId); });
        }
    }
}

// 断开信号
void AltimeterHandler::disconnectSignals()
{
    for (auto& controls : m_uiControls) {
        if (controls.powerOnBtn) controls.powerOnBtn->disconnect();
        if (controls.powerOffBtn) controls.powerOffBtn->disconnect();
        if (controls.getHeightBtn) controls.getHeightBtn->disconnect();
    }
}

// 统一处理高度计命令结果
void AltimeterHandler::handleResult(const SelfCheckResult* result)
{
    if (!result) return;

    DeviceId deviceId = result->deviceId();
    if (!m_uiControls.contains(deviceId)) {
        qWarning() << "不支持的高度计设备:" << static_cast<uint8_t>(deviceId);
        return;
    }

    processAltimeterCommand(deviceId, result->commandCode(), result);
}

// 分发处理命令结果
void AltimeterHandler::processAltimeterCommand(DeviceId deviceId, CommandCode cmdCode, const SelfCheckResult* result)
{
    const AltimeterUIControls& controls = m_uiControls[deviceId];
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
    case CommandCode::Altimeter_GetHeight:{
        setIcon(controls.getHeightIcon, icon);
        double height = result->getParameter("height").toDouble();
        updateStatusWidget(controls.getHeightFeedback, status, QString("%1 米").arg(height, 0, 'f', 4));
        break;
    }
    default:{
        qDebug() << "未处理的高度计命令:" << static_cast<int>(cmdCode);
        break;
    }
    }
}

// 获取设备编号
QString AltimeterHandler::getDeviceNumber(DeviceId deviceId) const
{
    return m_deviceNumberMap.value(deviceId, "");
}

// 支持的设备
bool AltimeterHandler::supportsDevice(DeviceId deviceId) const
{
    return m_deviceNumberMap.contains(deviceId);
}

// 设置设备映射
void AltimeterHandler::setDeviceMapping(const QMap<DeviceId, QString>& mapping)
{
    m_deviceNumberMap = mapping;
    initAllUIControls();
}

// -------------------- 槽函数实现 --------------------
void AltimeterHandler::onPowerOnClicked(DeviceId deviceId)
{
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Common_PowerOn, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("高度计上电命令失败: %1").arg(e.what()));
    }
}

void AltimeterHandler::onPowerOffClicked(DeviceId deviceId)
{
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Common_PowerOff, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("高度计下电命令失败: %1").arg(e.what()));
    }
}

void AltimeterHandler::onGetHeightClicked(DeviceId deviceId)
{
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Altimeter_GetHeight, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("获取高度命令失败: %1").arg(e.what()));
    }
}