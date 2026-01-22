#include "batteryhandler.h"

#include "widgetmanager.h"
#include <QMessageBox>
#include <QDebug>

BatteryHandler::BatteryHandler(SelfCheckModule* module, QWidget* parent)
    : DeviceHandlerBase(module, parent)
{
    initDefaultDeviceMapping();
    initAllUIControls();
    initializeUI();
}

BatteryHandler::~BatteryHandler()
{
    disconnectSignals();
}

// 初始化电池默认编号映射（10=PowerBattery1, 1A=PowerBattery2, 11=MeterBattery）
void BatteryHandler::initDefaultDeviceMapping()
{
    m_deviceNumberMap = {
        {DeviceId::PowerBattery1, "10"},
        {DeviceId::PowerBattery2, "1A"},
        {DeviceId::MeterBattery, "11"}
    };
}

// 初始化电池UI控件
void BatteryHandler::BatteryUIControls::init(QWidget* parent, const QString& prefix)
{
    // 命令按钮
    getSocBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_ReadSOC").arg(prefix), parent);
    getTotalVoltageBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_ReadVoltage").arg(prefix), parent);
    getCurrentBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_ReadCurrent").arg(prefix), parent);
    getAvgTempBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_ReadAvgTemp").arg(prefix), parent);
    hvOnBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_HVOn").arg(prefix), parent);
    hvOffBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_HVOff").arg(prefix), parent);
    lockBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_Lock").arg(prefix), parent);
    unlockBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_Unlock").arg(prefix), parent);

    // 状态图标
    getSocIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_ReadSOCIcon").arg(prefix), parent);
    getTotalVoltageIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_ReadVoltageIcon").arg(prefix), parent);
    getCurrentIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_ReadCurrentIcon").arg(prefix), parent);
    getAvgTempIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_ReadAvgTempIcon").arg(prefix), parent);
    hvOnIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_HVOnIcon").arg(prefix), parent);
    hvOffIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_HVOffIcon").arg(prefix), parent);
    lockIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_LockIcon").arg(prefix), parent);
    unlockIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_UnlockIcon").arg(prefix), parent);

    // 反馈文本框
    getSocFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadSOC_Feedback").arg(prefix), parent);
    getTotalVoltageFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadVoltage_Feedback").arg(prefix), parent);
    getCurrentFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadCurrent_Feedback").arg(prefix), parent);
    getAvgTempFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadAvgTemp_Feedback").arg(prefix), parent);
    hvOnFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_HVOn_Feedback").arg(prefix), parent);
    hvOffFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_HVOff_Feedback").arg(prefix), parent);
    lockFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_Lock_Feedback").arg(prefix), parent);
    unlockFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_Unlock_Feedback").arg(prefix), parent);
}

void BatteryHandler::initAllUIControls()
{
    for (auto it = m_deviceNumberMap.begin(); it != m_deviceNumberMap.end(); ++it) {
        BatteryUIControls controls;
        controls.number = it.value();
        controls.init(m_parent, controls.number);
        m_uiControls[it.key()] = controls;
    }
}

// 初始化UI（清空反馈框）
void BatteryHandler::initializeUI()
{
    for (auto& controls : m_uiControls) {
        if (controls.getSocFeedback) controls.getSocFeedback->clear();
        if (controls.getTotalVoltageFeedback) controls.getTotalVoltageFeedback->clear();
        if (controls.getCurrentFeedback) controls.getCurrentFeedback->clear();
        if (controls.getAvgTempFeedback) controls.getAvgTempFeedback->clear();
        if (controls.hvOnFeedback) controls.hvOnFeedback->clear();
        if (controls.hvOffFeedback) controls.hvOffFeedback->clear();
        if (controls.lockFeedback) controls.lockFeedback->clear();
        if (controls.unlockFeedback) controls.unlockFeedback->clear();
    }
}

// 连接信号（先断后连，避免重复）
void BatteryHandler::connectSignals()
{
    disconnectSignals();

    for (auto it = m_uiControls.begin(); it != m_uiControls.end(); ++it) {
        DeviceId deviceId = it.key();
        BatteryUIControls& controls = it.value();

        qDebug() << "[电池信号连接] DeviceId:" << static_cast<int>(deviceId)
            << " 前缀:" << controls.number;

        // 连接按钮信号
        if (controls.getSocBtn) {
            connect(controls.getSocBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetSocClicked(deviceId); });
        }
        if (controls.getTotalVoltageBtn) {
            connect(controls.getTotalVoltageBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetTotalVoltageClicked(deviceId); });
        }
        if (controls.getCurrentBtn) {
            connect(controls.getCurrentBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetCurrentClicked(deviceId); });
        }
        if (controls.getAvgTempBtn) {
            connect(controls.getAvgTempBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetAvgTempClicked(deviceId); });
        }
        if (controls.hvOnBtn) {
            connect(controls.hvOnBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onHVOnClicked(deviceId); });
        }
        if (controls.hvOffBtn) {
            connect(controls.hvOffBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onHVOffClicked(deviceId); });
        }
        if (controls.lockBtn) {
            connect(controls.lockBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onLockClicked(deviceId); });
        }
        if (controls.unlockBtn) {
            connect(controls.unlockBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onUnlockClicked(deviceId); });
        }
    }
}

// 断开所有信号
void BatteryHandler::disconnectSignals()
{
    for (auto& controls : m_uiControls) {
        if (controls.getSocBtn) controls.getSocBtn->disconnect();
        if (controls.getTotalVoltageBtn) controls.getTotalVoltageBtn->disconnect();
        if (controls.getCurrentBtn) controls.getCurrentBtn->disconnect();
        if (controls.getAvgTempBtn) controls.getAvgTempBtn->disconnect();
        if (controls.hvOnBtn) controls.hvOnBtn->disconnect();
        if (controls.hvOffBtn) controls.hvOffBtn->disconnect();
        if (controls.lockBtn) controls.lockBtn->disconnect();
        if (controls.unlockBtn) controls.unlockBtn->disconnect();
    }
}

// 统一处理电池命令结果
void BatteryHandler::handleResult(const SelfCheckResult* result)
{
    if (!result) return;

    DeviceId deviceId = result->deviceId();
    if (!m_uiControls.contains(deviceId)) {
        qWarning() << "不支持的电池设备:" << static_cast<uint8_t>(deviceId);
        return;
    }

    processBatteryCommand(deviceId, result->commandCode(), result);
}

// 分发处理不同电池命令结果
void BatteryHandler::processBatteryCommand(DeviceId deviceId, CommandCode cmdCode, const SelfCheckResult* result)
{
    const BatteryUIControls& controls = m_uiControls[deviceId];
    ResultStatus status = result->status();
    QPixmap icon = getStatusIcon(status);
    QString feedbackDesc = result->feedbackDesc();

    switch (cmdCode) {
    case CommandCode::Battery_GetSoc:
    {   
        setIcon(controls.getSocIcon, icon);
        if (deviceId == DeviceId::MeterBattery) {
            // 仪表电池：原始值是uint16，比例尺0.1
            uint16_t socRaw = result->getParameter("soc").toUInt();
            double soc = socRaw * 0.1;
            updateStatusWidget(controls.getSocFeedback, status, QString("%1%").arg(soc, 0, 'f', 1));
        }
        else {
            // 动力电池：原始值是uint8，比例尺1
            uint8_t soc = result->getParameter("soc").toUInt();
            updateStatusWidget(controls.getSocFeedback, status, QString("%1%").arg(soc));
        }
        break;
    }
    case CommandCode::Battery_GetTotalVoltage:
    {  
        setIcon(controls.getTotalVoltageIcon, icon);
        uint16_t voltRaw = result->getParameter("volt").toUInt();
        double volt = voltRaw * 0.1; // 协议比例尺0.1
        updateStatusWidget(controls.getTotalVoltageFeedback, status, QString("%1V").arg(volt, 0, 'f', 1));
        break;
    }
    case CommandCode::Battery_GetCurrent:
    { 
        setIcon(controls.getCurrentIcon, icon);
        int16_t currRaw = result->getParameter("current").toInt();
        double curr = currRaw * 0.1; // 协议比例尺0.1
        updateStatusWidget(controls.getCurrentFeedback, status, QString("%1A").arg(curr, 0, 'f', 1));
        break;
    }
    case CommandCode::Battery_GetAvgTemp:
    {   
        setIcon(controls.getAvgTempIcon, icon);
        int16_t tempRaw = result->getParameter("avgTemp").toInt();
        updateStatusWidget(controls.getAvgTempFeedback, status, QString("%1℃").arg(tempRaw));
        break;
    }
    case CommandCode::Battery_HVOn:
    { 
        setIcon(controls.hvOnIcon, icon);
        updateStatusWidget(controls.hvOnFeedback, status, feedbackDesc);
        break;
    }
    case CommandCode::Battery_HVOff:
    {   
        setIcon(controls.hvOffIcon, icon);
        updateStatusWidget(controls.hvOffFeedback, status, feedbackDesc);
        break;
    }
    case CommandCode::Battery_Lock:
    {  
        setIcon(controls.lockIcon, icon);
        updateStatusWidget(controls.lockFeedback, status, feedbackDesc);
        break;
    }
    case CommandCode::Battery_Unlock:
    { 
        setIcon(controls.unlockIcon, icon);
        updateStatusWidget(controls.unlockFeedback, status, feedbackDesc);
        break;
    }
    default:
        qDebug() << "未处理的电池命令:" << static_cast<int>(cmdCode);
        break;
    }
}

// 获取设备编号
QString BatteryHandler::getDeviceNumber(DeviceId deviceId) const
{
    return m_deviceNumberMap.value(deviceId, "");
}

// 支持的设备
bool BatteryHandler::supportsDevice(DeviceId deviceId) const
{
    return m_deviceNumberMap.contains(deviceId);
}

// 设置设备映射
void BatteryHandler::setDeviceMapping(const QMap<DeviceId, QString>& mapping)
{
    m_deviceNumberMap = mapping;
    initAllUIControls();
}

// -------------------- 槽函数实现 --------------------
void BatteryHandler::onGetSocClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Battery_GetSoc, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误",
            QString("%1读取SOC命令失败: %2").arg(getDeviceNumber(deviceId)).arg(e.what()));
    }
}

void BatteryHandler::onGetTotalVoltageClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Battery_GetTotalVoltage, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误",
            QString("%1读取总电压命令失败: %2").arg(getDeviceNumber(deviceId)).arg(e.what()));
    }
}

void BatteryHandler::onGetCurrentClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Battery_GetCurrent, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误",
            QString("%1读取电流命令失败: %2").arg(getDeviceNumber(deviceId)).arg(e.what()));
    }
}

void BatteryHandler::onGetAvgTempClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Battery_GetAvgTemp, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误",
            QString("%1读取平均温度命令失败: %2").arg(getDeviceNumber(deviceId)).arg(e.what()));
    }
}

void BatteryHandler::onHVOnClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Battery_HVOn, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误",
            QString("%1高压上电命令失败: %2").arg(getDeviceNumber(deviceId)).arg(e.what()));
    }
}

void BatteryHandler::onHVOffClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Battery_HVOff, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误",
            QString("%1高压下电命令失败: %2").arg(getDeviceNumber(deviceId)).arg(e.what()));
    }
}

void BatteryHandler::onLockClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Battery_Lock, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误",
            QString("%1锁定命令失败: %2").arg(getDeviceNumber(deviceId)).arg(e.what()));
    }
}

void BatteryHandler::onUnlockClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Battery_Unlock, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误",
            QString("%1解锁命令失败: %2").arg(getDeviceNumber(deviceId)).arg(e.what()));
    }
}