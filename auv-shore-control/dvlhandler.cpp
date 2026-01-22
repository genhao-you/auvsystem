#include "dvlhandler.h"
#include "widgetmanager.h"
#include <QMessageBox>
#include <QDebug>

DvlHandler::DvlHandler(SelfCheckModule* module, QWidget* parent)
    : DeviceHandlerBase(module, parent)
{
    initDefaultDeviceMapping();
    initAllUIControls();
    initializeUI();
}

DvlHandler::~DvlHandler()
{
    disconnectSignals();
}

// 初始化DVL默认编号映射（0E）
void DvlHandler::initDefaultDeviceMapping()
{
    m_deviceNumberMap = { {DeviceId::DVL, "0E"} };
}

// 初始化DVL UI控件
void DvlHandler::DvlUIControls::init(QWidget* parent, const QString& prefix)
{
    // 按钮
    powerOnBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_PowerOn").arg(prefix), parent);
    powerOffBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_PowerOff").arg(prefix), parent);
    getBottomSpeedBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_ReadBottomSpeed").arg(prefix), parent);
    getWaterSpeedBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_ReadWaterSpeed").arg(prefix), parent);
    getStatusBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_ReadStatus").arg(prefix), parent);

    // 状态图标
    powerOnIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_PowerOnIcon").arg(prefix), parent);
    powerOffIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_PowerOffIcon").arg(prefix), parent);
    getBottomSpeedIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_ReadBottomSpeedIcon").arg(prefix), parent);
    getWaterSpeedIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_ReadWaterSpeedIcon").arg(prefix), parent);
    getStatusIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_ReadStatusIcon").arg(prefix), parent);

    // 反馈文本框
    powerOnFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_PowerOn_Feedback").arg(prefix), parent);
    powerOffFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_PowerOff_Feedback").arg(prefix), parent);
    // 底速三轴
    getBottomXFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadBottomX_Feedback").arg(prefix), parent);
    getBottomYFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadBottomY_Feedback").arg(prefix), parent);
    getBottomZFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadBottomZ_Feedback").arg(prefix), parent);
    // 水速三轴
    getWaterXFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadWaterX_Feedback").arg(prefix), parent);
    getWaterYFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadWaterY_Feedback").arg(prefix), parent);
    getWaterZFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadWaterZ_Feedback").arg(prefix), parent);
    // 状态反馈
    selfCheckCardFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_SelfCheckCard_Feedback").arg(prefix), parent);
    outputStateFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_OutputState_Feedback").arg(prefix), parent);
}

void DvlHandler::initAllUIControls()
{
    for (auto it = m_deviceNumberMap.begin(); it != m_deviceNumberMap.end(); ++it) {
        DvlUIControls controls;
        controls.number = it.value();
        controls.init(m_parent, controls.number);
        m_uiControls[it.key()] = controls;
    }
}

// 初始化UI（清空反馈框）
void DvlHandler::initializeUI()
{
    for (auto& controls : m_uiControls) {
        if (controls.powerOnFeedback) controls.powerOnFeedback->clear();
        if (controls.powerOffFeedback) controls.powerOffFeedback->clear();
        // 底速三轴
        if (controls.getBottomXFeedback) controls.getBottomXFeedback->clear();
        if (controls.getBottomYFeedback) controls.getBottomYFeedback->clear();
        if (controls.getBottomZFeedback) controls.getBottomZFeedback->clear();
        // 水速三轴
        if (controls.getWaterXFeedback) controls.getWaterXFeedback->clear();
        if (controls.getWaterYFeedback) controls.getWaterYFeedback->clear();
        if (controls.getWaterZFeedback) controls.getWaterZFeedback->clear();
        // 状态反馈
        if (controls.selfCheckCardFeedback) controls.selfCheckCardFeedback->clear();
        if (controls.outputStateFeedback) controls.outputStateFeedback->clear();
    }
}

// 连接信号（先断后连，避免重复）
void DvlHandler::connectSignals()
{
    disconnectSignals();

    for (auto it = m_uiControls.begin(); it != m_uiControls.end(); ++it) {
        DeviceId deviceId = it.key();
        DvlUIControls& controls = it.value();

        qDebug() << "[DVL信号连接] DeviceId:" << static_cast<int>(deviceId)
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
        if (controls.getBottomSpeedBtn) {
            connect(controls.getBottomSpeedBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetBottomSpeedClicked(deviceId); });
        }
        if (controls.getWaterSpeedBtn) {
            connect(controls.getWaterSpeedBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetWaterSpeedClicked(deviceId); });
        }
        if (controls.getStatusBtn) {
            connect(controls.getStatusBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetStatusClicked(deviceId); });
        }
    }
}

// 断开所有信号
void DvlHandler::disconnectSignals()
{
    for (auto& controls : m_uiControls) {
        if (controls.powerOnBtn) controls.powerOnBtn->disconnect();
        if (controls.powerOffBtn) controls.powerOffBtn->disconnect();
        if (controls.getBottomSpeedBtn) controls.getBottomSpeedBtn->disconnect();
        if (controls.getWaterSpeedBtn) controls.getWaterSpeedBtn->disconnect();
        if (controls.getStatusBtn) controls.getStatusBtn->disconnect();
    }
}

// 统一处理DVL命令结果
void DvlHandler::handleResult(const SelfCheckResult* result)
{
    if (!result) return;

    DeviceId deviceId = result->deviceId();
    if (!m_uiControls.contains(deviceId)) {
        qWarning() << "不支持的DVL设备:" << static_cast<uint8_t>(deviceId);
        return;
    }

    processDvlCommand(deviceId, result->commandCode(), result);
}

// 分发处理不同DVL命令结果
void DvlHandler::processDvlCommand(DeviceId deviceId, CommandCode cmdCode, const SelfCheckResult* result)
{
    const DvlUIControls& controls = m_uiControls[deviceId];
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
    case CommandCode::DVL_GetBottomSpeed:{
        setIcon(controls.getBottomSpeedIcon, icon);
        // 读取解析后的三轴底速参数（带默认值）
        double bottomX = result->getParameter("dvl_bottom_x").toDouble();
        double bottomY = result->getParameter("dvl_bottom_y").toDouble();
        double bottomZ = result->getParameter("dvl_bottom_z").toDouble();
        // 更新三轴速度UI（保留2位小数）
        updateStatusWidget(controls.getBottomXFeedback, status, QString("%1").arg(bottomX, 0, 'f', 2));
        updateStatusWidget(controls.getBottomYFeedback, status, QString("%1").arg(bottomY, 0, 'f', 2));
        updateStatusWidget(controls.getBottomZFeedback, status, QString("%1").arg(bottomZ, 0, 'f', 2));
        break;
    }
    case CommandCode::DVL_GetWaterSpeed:{
        setIcon(controls.getWaterSpeedIcon, icon);
        // 读取解析后的三轴水速参数（带默认值）
        double waterX = result->getParameter("dvl_water_x").toDouble();
        double waterY = result->getParameter("dvl_water_y").toDouble();
        double waterZ = result->getParameter("dvl_water_z").toDouble();
        // 更新三轴速度UI（保留2位小数）
        updateStatusWidget(controls.getWaterXFeedback, status, QString("%1").arg(waterX, 0, 'f', 2));
        updateStatusWidget(controls.getWaterYFeedback, status, QString("%1").arg(waterY, 0, 'f', 2));
        updateStatusWidget(controls.getWaterZFeedback, status, QString("%1").arg(waterZ, 0, 'f', 2));
        break;
    }
    case CommandCode::DVL_GetStatus:{
        setIcon(controls.getStatusIcon, icon);
        // 读取解析后的状态参数（无参数时赋默认值）
        QString selfCheckCardState = result->getParameter("dvl_selfcheck_card_state").toString();
        QString outputState = result->getParameter("dvl_output_state").toString();
        // 更新自检+存储卡状态文本框
        updateStatusWidget(controls.selfCheckCardFeedback, status, selfCheckCardState);
        // 更新输出状态文本框
        updateStatusWidget(controls.outputStateFeedback, status, outputState);
        break;
    }
    default:
        qDebug() << "未处理的DVL命令:" << static_cast<int>(cmdCode);
        break;
    }
}

// 获取设备编号
QString DvlHandler::getDeviceNumber(DeviceId deviceId) const
{
    return m_deviceNumberMap.value(deviceId, "");
}

// 支持的设备
bool DvlHandler::supportsDevice(DeviceId deviceId) const
{
    return m_deviceNumberMap.contains(deviceId);
}

// 设置设备映射
void DvlHandler::setDeviceMapping(const QMap<DeviceId, QString>& mapping)
{
    m_deviceNumberMap = mapping;
    initAllUIControls();
}

// -------------------- 槽函数实现 --------------------
void DvlHandler::onPowerOnClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Common_PowerOn, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("DVL上电命令失败: %1").arg(e.what()));
    }
}

void DvlHandler::onPowerOffClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Common_PowerOff, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("DVL下电命令失败: %1").arg(e.what()));
    }
}

void DvlHandler::onGetBottomSpeedClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::DVL_GetBottomSpeed, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("读取DVL底速命令失败: %1").arg(e.what()));
    }
}

void DvlHandler::onGetWaterSpeedClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::DVL_GetWaterSpeed, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("读取DVL水速命令失败: %1").arg(e.what()));
    }
}

void DvlHandler::onGetStatusClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::DVL_GetStatus, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("读取DVL状态命令失败: %1").arg(e.what()));
    }
}
