#include "buoyancyhandler.h"
#include "widgetmanager.h"
#include <QMessageBox>
#include <QDebug>

BuoyancyHandler::BuoyancyHandler(SelfCheckModule* module, QWidget* parent)
    : DeviceHandlerBase(module, parent)
{
    initDefaultDeviceMapping();
    initAllUIControls();
    initializeUI();
}

BuoyancyHandler::~BuoyancyHandler()
{
    disconnectSignals();
}

// 初始化浮力调节默认编号映射（06）
void BuoyancyHandler::initDefaultDeviceMapping()
{
    m_deviceNumberMap = { {DeviceId::BuoyancyAdjust, "06"} };
}

// 初始化浮力调节UI控件
void BuoyancyHandler::BuoyancyUIControls::init(QWidget* parent, const QString& prefix)
{
    // 按钮
    powerOnBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_PowerOn").arg(prefix), parent);
    powerOffBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_PowerOff").arg(prefix), parent);
    getBuoyancyValueBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_ReadBuoyancyValue").arg(prefix), parent);
    setBuoyancyValueBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_SetBuoyancyValue").arg(prefix), parent);
    stopBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_Stop").arg(prefix), parent);

    // 参数输入框
    setBuoyancyValueInput = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_SetBuoyancyValue_input").arg(prefix), parent);

    // 状态图标（按需扩展，若有对应UI控件）
    powerOnIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_PowerOnIcon").arg(prefix), parent);
    powerOffIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_PowerOffIcon").arg(prefix), parent);
    getBuoyancyValueIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_ReadBuoyancyValueIcon").arg(prefix), parent);
    setBuoyancyValueIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_SetBuoyancyValueIcon").arg(prefix), parent);
    stopIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_StopIcon").arg(prefix), parent);

    // 反馈文本框
    powerOnFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_PowerOn_Feedback").arg(prefix), parent);
    powerOffFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_PowerOff_Feedback").arg(prefix), parent);
    getBuoyancyValueFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadBuoyancyValue_Feedback").arg(prefix), parent);
    setBuoyancyValueFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_SetBuoyancyValue_Feedback").arg(prefix), parent);
    stopFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_Stop_Feedback").arg(prefix), parent);
}

void BuoyancyHandler::initAllUIControls()
{
    for (auto it = m_deviceNumberMap.begin(); it != m_deviceNumberMap.end(); ++it) {
        BuoyancyUIControls controls;
        controls.number = it.value();
        controls.init(m_parent, controls.number);
        m_uiControls[it.key()] = controls;
    }
}

// 初始化UI（清空反馈框）
void BuoyancyHandler::initializeUI()
{
    for (auto& controls : m_uiControls) {
        if (controls.powerOnFeedback) controls.powerOnFeedback->clear();
        if (controls.powerOffFeedback) controls.powerOffFeedback->clear();
        if (controls.getBuoyancyValueFeedback) controls.getBuoyancyValueFeedback->clear();
        if (controls.setBuoyancyValueFeedback) controls.setBuoyancyValueFeedback->clear();
        if (controls.stopFeedback) controls.stopFeedback->clear();
    }
}

// 连接信号（先断后连，避免重复）
void BuoyancyHandler::connectSignals()
{
    disconnectSignals();

    for (auto it = m_uiControls.begin(); it != m_uiControls.end(); ++it) {
        DeviceId deviceId = it.key();
        BuoyancyUIControls& controls = it.value();

        qDebug() << "[浮力调节信号连接] DeviceId:" << static_cast<int>(deviceId)
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
        if (controls.getBuoyancyValueBtn) {
            connect(controls.getBuoyancyValueBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetBuoyancyValueClicked(deviceId); });
        }
        if (controls.setBuoyancyValueBtn) {
            connect(controls.setBuoyancyValueBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onSetBuoyancyValueClicked(deviceId); });
        }
        if (controls.stopBtn) {
            connect(controls.stopBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onStopClicked(deviceId); });
        }
    }
}

// 断开所有信号
void BuoyancyHandler::disconnectSignals()
{
    for (auto& controls : m_uiControls) {
        if (controls.powerOnBtn) controls.powerOnBtn->disconnect();
        if (controls.powerOffBtn) controls.powerOffBtn->disconnect();
        if (controls.getBuoyancyValueBtn) controls.getBuoyancyValueBtn->disconnect();
        if (controls.setBuoyancyValueBtn) controls.setBuoyancyValueBtn->disconnect();
        if (controls.stopBtn) controls.stopBtn->disconnect();
    }
}

// 统一处理浮力调节命令结果
void BuoyancyHandler::handleResult(const SelfCheckResult* result)
{
    if (!result) return;

    DeviceId deviceId = result->deviceId();
    if (!m_uiControls.contains(deviceId)) {
        qWarning() << "不支持的浮力调节设备:" << static_cast<uint8_t>(deviceId);
        return;
    }

    processBuoyancyCommand(deviceId, result->commandCode(), result);
}

// 分发处理不同命令结果
void BuoyancyHandler::processBuoyancyCommand(DeviceId deviceId, CommandCode cmdCode, const SelfCheckResult* result)
{
    const BuoyancyUIControls& controls = m_uiControls[deviceId];
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
    case CommandCode::Buoyancy_ReadValue:{
        setIcon(controls.getBuoyancyValueIcon, icon);
        // 解析浮力值（反向转换：uint16_t -> double / 100）
        uint16_t rawValue = result->getParameter("buoyancy_value").toUInt();
        double realValue = static_cast<double>(rawValue) / 100.0;
        updateStatusWidget(controls.getBuoyancyValueFeedback, status,
            QString("%1").arg(realValue, 0, 'f', 2));
        break;
    }
    case CommandCode::Buoyancy_SetValue:{
        setIcon(controls.setBuoyancyValueIcon, icon);
        updateStatusWidget(controls.setBuoyancyValueFeedback, status, feedbackDesc);
        break;
    }
    case CommandCode::Buoyancy_Stop:{
        setIcon(controls.stopIcon, icon);
        updateStatusWidget(controls.stopFeedback, status, feedbackDesc);
        break;
    }
    default:
        qDebug() << "未处理的浮力调节命令:" << static_cast<int>(cmdCode);
        break;
    }
}

// 获取设备编号
QString BuoyancyHandler::getDeviceNumber(DeviceId deviceId) const
{
    return m_deviceNumberMap.value(deviceId, "");
}

// 支持的设备
bool BuoyancyHandler::supportsDevice(DeviceId deviceId) const
{
    return m_deviceNumberMap.contains(deviceId);
}

// 设置设备映射
void BuoyancyHandler::setDeviceMapping(const QMap<DeviceId, QString>& mapping)
{
    m_deviceNumberMap = mapping;
    initAllUIControls();
}

// -------------------- 槽函数实现 --------------------
void BuoyancyHandler::onPowerOnClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Common_PowerOn, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("浮力调节上电命令失败: %1").arg(e.what()));
    }
}

void BuoyancyHandler::onPowerOffClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Common_PowerOff, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("浮力调节下电命令失败: %1").arg(e.what()));
    }
}

void BuoyancyHandler::onGetBuoyancyValueClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Buoyancy_ReadValue, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("读取浮力值命令失败: %1").arg(e.what()));
    }
}

void BuoyancyHandler::onSetBuoyancyValueClicked(DeviceId deviceId)
{
    if (!m_module) return;

    const BuoyancyUIControls& controls = m_uiControls[deviceId];
    if (!controls.setBuoyancyValueInput) {
        QMessageBox::critical(m_parent, "错误", "浮力值输入框未找到");
        return;
    }

    // 1. 获取用户输入的浮力值文本并去空
    QString volumeText = controls.setBuoyancyValueInput->text().trimmed();
    if (volumeText.isEmpty()) {
        QMessageBox::warning(m_parent, "输入错误", "请输入浮力值");
        return;
    }

    // 2. 文本转double（附带有效性校验）
    bool convertOk = false;
    double volumeD = volumeText.toDouble(&convertOk);
    if (!convertOk) {
        QMessageBox::warning(m_parent, "输入错误", "请输入有效的数字格式浮力值");
        return;
    }

    // 3. 核心校验：浮力值范围（0 ~ 10.06）
    if (volumeD < 0 || volumeD > MAX_BUOYANCY_VALUE) {
        QString warningMsg = QString("浮力值输入无效！\n请输入0到%1之间的数值").arg(MAX_BUOYANCY_VALUE);
        QMessageBox::warning(m_parent, "输入错误", warningMsg);
        return;
    }

    // 4. 合法值：转换为uint16_t类型（放大100倍）
    uint16_t volume = static_cast<uint16_t>(volumeD * 100);
    try {
        auto params = std::make_shared<BuoyancyVolumeParameter>(volume);
        m_module->sendCheckCommand(deviceId, CommandCode::Buoyancy_SetValue, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("设置浮力值命令失败: %1").arg(e.what()));
    }
}

void BuoyancyHandler::onStopClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Buoyancy_Stop, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("浮力调节停止命令失败: %1").arg(e.what()));
    }
}