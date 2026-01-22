#include "rudderhandler.h"
#include <QIntValidator>
#include <QDoubleValidator>
#include "widgetmanager.h"
#include <QMessageBox>
#include <QDebug>

RudderHandler::RudderHandler(SelfCheckModule* module, QWidget* parent)
    : DeviceHandlerBase(module, parent)
{
    initDefaultDeviceMapping();  // 初始化4个舵机的默认编号
    initAllUIControls();         // 初始化所有舵机的UI控件
    initializeUI();              // 设置输入验证器、清空反馈框
}

RudderHandler::~RudderHandler()
{
    disconnectSignals();  // 析构时断开所有信号
}

// 初始化4个舵机的默认编号映射（可根据实际UI命名调整）
void RudderHandler::initDefaultDeviceMapping()
{
    m_deviceNumberMap = {
        {DeviceId::Rudder1, "02"},
        {DeviceId::Rudder2, "03"},
        {DeviceId::Rudder3, "04"},
        {DeviceId::Rudder4, "05"},
        {DeviceId::AntennaFold, "08"}
    };
}

// 初始化单个舵机的所有UI控件（按前缀匹配，和推进器逻辑一致）
void RudderHandler::RudderUIControls::init(QWidget* parent, const QString& prefix)
{
    // 1. 获取按钮控件
    powerOnBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_PowerOn").arg(prefix), parent);
    powerOffBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_PowerOff").arg(prefix), parent);
    setAngleBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_SetAngle").arg(prefix), parent);
    getRunTimeBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_ReadRunTime").arg(prefix), parent);
    getVoltageBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_ReadVoltage").arg(prefix), parent);
    getAngleBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_ReadAngle").arg(prefix), parent);
    getCurrentBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_ReadCurrent").arg(prefix), parent);
    getTempBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_ReadTemp").arg(prefix), parent);
    setZeroBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_SetZero").arg(prefix), parent);
    getFaultBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_ReadFault").arg(prefix), parent);

    // 2. 获取状态图标控件
    powerOnIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_PowerOnIcon").arg(prefix), parent);
    powerOffIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_PowerOffIcon").arg(prefix), parent);
    setAngleIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_SetAngleIcon").arg(prefix), parent);
    getRunTimeIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_ReadRunTimeIcon").arg(prefix), parent);
    getVoltageIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_ReadVoltageIcon").arg(prefix), parent);
    getAngleIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_ReadAngleIcon").arg(prefix), parent);
    getCurrentIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_ReadCurrentIcon").arg(prefix), parent);
    getTempIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_ReadTempIcon").arg(prefix), parent);
    getFaultIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_GetFaultIcon").arg(prefix), parent);

    // 3. 获取反馈文本框
    powerOnFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_PowerOn_Feedback").arg(prefix), parent);
    powerOffFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_PowerOff_Feedback").arg(prefix), parent);
    getRunTimeFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadRunTime_Feedback").arg(prefix), parent);
    getVoltageFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadVoltage_Feedback").arg(prefix), parent);
    getAngleFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadAngle_Feedback").arg(prefix), parent);
    getCurrentFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadCurrent_Feedback").arg(prefix), parent);
    getTempFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadTemp_Feedback").arg(prefix), parent);
    getFaultFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_GetFault_Feedback").arg(prefix), parent);

    // 4. 获取输入文本框
    targetAngleInput = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_TargetAngle_Input").arg(prefix), parent);
}

// 初始化所有舵机的UI控件
void RudderHandler::initAllUIControls()
{
    for (auto it = m_deviceNumberMap.begin(); it != m_deviceNumberMap.end(); ++it) {
        RudderUIControls controls;
        controls.number = it.value();
        controls.init(m_parent, controls.number);
        m_uiControls[it.key()] = controls;
    }
}

// 初始化UI（设置输入验证器、清空反馈框）
void RudderHandler::initializeUI()
{
    for (auto& controls : m_uiControls) {
        // 设置角度输入验证器（支持正负小数，范围-90~90）
        if (controls.targetAngleInput) {
            controls.targetAngleInput->setValidator(new QDoubleValidator(-90.0, 90.0, 1, controls.targetAngleInput));
        }

        // 清空所有反馈文本框
        if (controls.powerOnFeedback) controls.powerOnFeedback->clear();
        if (controls.powerOffFeedback) controls.powerOffFeedback->clear();
        if (controls.getRunTimeFeedback) controls.getRunTimeFeedback->clear();
        if (controls.getVoltageFeedback) controls.getVoltageFeedback->clear();
        if (controls.getAngleFeedback) controls.getAngleFeedback->clear();
        if (controls.getCurrentFeedback) controls.getCurrentFeedback->clear();
        if (controls.getTempFeedback) controls.getTempFeedback->clear();
        if (controls.getFaultFeedback) controls.getFaultFeedback->clear();
    }
}

// 连接所有舵机的按钮信号
void RudderHandler::connectSignals()
{
    disconnectSignals();

    for (auto it = m_uiControls.begin(); it != m_uiControls.end(); ++it) {
        DeviceId deviceId = it.key();
        RudderUIControls& controls = it.value();

        // 调试日志
        qDebug() << "[舵机信号连接] DeviceId:" << static_cast<int>(deviceId)
            << " 前缀:" << controls.number
            << " PowerOnBtn地址:" << controls.powerOnBtn;

        // 连接按钮信号
        if (controls.powerOnBtn) {
            connect(controls.powerOnBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onPowerOnClicked(deviceId); });
        }
        if (controls.powerOffBtn) {
            connect(controls.powerOffBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onPowerOffClicked(deviceId); });
        }
        if (controls.setAngleBtn) {
            connect(controls.setAngleBtn, &QPushButton::clicked,
                this, [this, deviceId, &controls]() { onSetAngleClicked(deviceId, controls); });
        }
        if (controls.getRunTimeBtn) {
            connect(controls.getRunTimeBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetRunTimeClicked(deviceId); });
        }
        if (controls.getVoltageBtn) {
            connect(controls.getVoltageBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetVoltageClicked(deviceId); });
        }
        if (controls.getAngleBtn) {
            connect(controls.getAngleBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetAngleClicked(deviceId); });
        }
        if (controls.getCurrentBtn) {
            connect(controls.getCurrentBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetCurrentClicked(deviceId); });
        }
        if (controls.getTempBtn) {
            connect(controls.getTempBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetTempClicked(deviceId); });
        }
        if (controls.setZeroBtn) {
            connect(controls.setZeroBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onSetZeroClicked(deviceId); });
        }
        if (controls.getFaultBtn) {
            connect(controls.getFaultBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetFaultClicked(deviceId); });
        }
    }
}

// 断开所有信号
void RudderHandler::disconnectSignals()
{
    for (auto& controls : m_uiControls) {
        if (controls.powerOnBtn) controls.powerOnBtn->disconnect();
        if (controls.powerOffBtn) controls.powerOffBtn->disconnect();
        if (controls.setAngleBtn) controls.setAngleBtn->disconnect();
        if (controls.getRunTimeBtn) controls.getRunTimeBtn->disconnect();
        if (controls.getVoltageBtn) controls.getVoltageBtn->disconnect();
        if (controls.getAngleBtn) controls.getAngleBtn->disconnect();
        if (controls.getCurrentBtn) controls.getCurrentBtn->disconnect();
        if (controls.getTempBtn) controls.getTempBtn->disconnect();
        if (controls.setZeroBtn) controls.setZeroBtn->disconnect();
        if (controls.getFaultBtn) controls.getFaultBtn->disconnect();
    }
}

// 处理舵机命令返回结果（核心统一处理逻辑）
void RudderHandler::handleResult(const SelfCheckResult* result)
{
    if (!result) return;

    DeviceId deviceId = result->deviceId();
    if (!m_uiControls.contains(deviceId)) {
        qWarning() << "不支持的舵机设备:" << EnumConverter::deviceIdToString(deviceId);
        return;
    }

    processRudderCommand(deviceId, result->commandCode(), result);
}

// 分发处理不同舵机命令的结果
void RudderHandler::processRudderCommand(DeviceId deviceId, CommandCode cmdCode, const SelfCheckResult* result)
{
    const RudderUIControls& controls = m_uiControls[deviceId];
    ResultStatus status = result->status();
    QPixmap icon = getStatusIcon(status);
    QString feedbackDesc = result->feedbackDesc();

    switch (cmdCode) {
    case CommandCode::Common_PowerOn:
        handlePowerCommand(controls, cmdCode, result);
        break;
    case CommandCode::Common_PowerOff:
        handlePowerCommand(controls, cmdCode, result);
        break;
    case CommandCode::Rudder_SetAngle:
        handleSetAngleCommand(controls, result);
        break;
    case CommandCode::Rudder_GetRunTime:
        handleGetRunTimeCommand(controls, result);
        break;
    case CommandCode::Rudder_GetVoltage:
        handleGetVoltageCommand(controls, result);
        break;
    case CommandCode::Rudder_GetAngle:
        handleGetAngleCommand(controls, result);
        break;
    case CommandCode::Rudder_GetCurrent:
        handleGetCurrentCommand(controls, result);
        break;
    case CommandCode::Rudder_GetTemp:
        handleGetTempCommand(controls, result);
        break;
    case CommandCode::Common_GetFault:
        handleGetFaultCommand(controls, result);
        break;
    default:
        qDebug() << "未处理的舵机命令:" << static_cast<int>(cmdCode);
        break;
    }
}

// 处理电源相关命令结果
void RudderHandler::handlePowerCommand(const RudderUIControls& controls, CommandCode cmdCode, const SelfCheckResult* result)
{
    ResultStatus status = result->status();
    QPixmap icon = getStatusIcon(status);
    QString feedbackDesc = result->feedbackDesc();

    if (cmdCode == CommandCode::Common_PowerOn) {
        setIcon(controls.powerOnIcon, icon);
        updateStatusWidget(controls.powerOnFeedback, status, feedbackDesc);
    }
    else if (cmdCode == CommandCode::Common_PowerOff) {
        setIcon(controls.powerOffIcon, icon);
        updateStatusWidget(controls.powerOffFeedback, status, feedbackDesc);
    }
}

// 处理设置角度命令结果
void RudderHandler::handleSetAngleCommand(const RudderUIControls& controls, const SelfCheckResult* result)
{
    setIcon(controls.setAngleIcon, getStatusIcon(result->status()));
}

// 处理获取运行时间命令结果
void RudderHandler::handleGetRunTimeCommand(const RudderUIControls& controls, const SelfCheckResult* result)
{
    setIcon(controls.getRunTimeIcon, getStatusIcon(result->status()));
    qlonglong runtimeVal = result->getParameter("runtime").toLongLong();
    updateStatusWidget(controls.getRunTimeFeedback, result->status(),
        QString("%1 秒").arg(static_cast<uint32_t>(runtimeVal)));
}

// 处理获取电压命令结果
void RudderHandler::handleGetVoltageCommand(const RudderUIControls& controls, const SelfCheckResult* result)
{
    setIcon(controls.getVoltageIcon, getStatusIcon(result->status()));
    double voltage = result->getParameter("voltage").toDouble();
    updateStatusWidget(controls.getVoltageFeedback, result->status(),
        QString("%1 V").arg(voltage, 0, 'f', 1));
}

// 处理获取角度命令结果
void RudderHandler::handleGetAngleCommand(const RudderUIControls& controls, const SelfCheckResult* result)
{
    setIcon(controls.getAngleIcon, getStatusIcon(result->status()));
    double angle = result->getParameter("angle").toDouble();
    updateStatusWidget(controls.getAngleFeedback, result->status(),
        QString("%1 °").arg(angle, 0, 'f', 1));
}

// 处理获取电流命令结果
void RudderHandler::handleGetCurrentCommand(const RudderUIControls& controls, const SelfCheckResult* result)
{
    setIcon(controls.getCurrentIcon, getStatusIcon(result->status()));
    qlonglong currRaw = result->getParameter("current").toLongLong();
    double current = static_cast<uint16_t>(currRaw) / 100.0;
    updateStatusWidget(controls.getCurrentFeedback, result->status(),
        QString("%1 A").arg(current, 0, 'f', 2));
}

// 处理获取温度命令结果
void RudderHandler::handleGetTempCommand(const RudderUIControls& controls, const SelfCheckResult* result)
{
    setIcon(controls.getTempIcon, getStatusIcon(result->status()));
    qlonglong tempRaw = result->getParameter("temperature").toLongLong();
    double temperature = static_cast<uint16_t>(tempRaw) / 10.0;
    updateStatusWidget(controls.getTempFeedback, result->status(),
        QString("%1 ℃").arg(temperature, 0, 'f', 1));
}

// 处理获取故障命令结果
void RudderHandler::handleGetFaultCommand(const RudderUIControls& controls, const SelfCheckResult* result)
{
    setIcon(controls.getFaultIcon, getStatusIcon(result->status()));
    QString faultCode = result->getParameter("fault").toString();
    QString faultDesc = faultCode.isEmpty() ? "无故障" : QString("故障码：%1").arg(faultCode);
    updateStatusWidget(controls.getFaultFeedback, result->status(), faultDesc);
}

// 创建角度参数（封装验证逻辑）
std::shared_ptr<RudderAngleParameter> RudderHandler::createAngleParameter(const QString& inputText, const QString& paramName) const
{
    QString errorMsg;
    double angleDegree = 0.0;

    if (!validateAngleInput(inputText, errorMsg, angleDegree)) {
        throw std::runtime_error(errorMsg.toStdString());
    }

    // 转换为十分度（int16_t）
    int16_t angleTenthDegree = static_cast<int16_t>(angleDegree);
    return std::make_shared<RudderAngleParameter>(angleTenthDegree);
}

// 验证角度输入
bool RudderHandler::validateAngleInput(const QString& inputText, QString& errorMsg, double& angleDegree) const
{
    if (inputText.isEmpty()) {
        errorMsg = "目标角度不能为空";
        return false;
    }

    bool ok = false;
    angleDegree = inputText.toDouble(&ok);
    if (!ok) {
        errorMsg = "角度必须为数字（支持正负小数）";
        return false;
    }

    if (angleDegree < -90.0 || angleDegree > 90.0) {
        errorMsg = "角度超出范围（-90~90度）";
        return false;
    }

    return true;
}

// 获取舵机编号
QString RudderHandler::getDeviceNumber(DeviceId deviceId) const
{
    return m_deviceNumberMap.value(deviceId, "");
}

// 判断是否支持该舵机设备
bool RudderHandler::supportsDevice(DeviceId deviceId) const
{
    return m_deviceNumberMap.contains(deviceId);
}

// 设置舵机编号映射
void RudderHandler::setDeviceMapping(const QMap<DeviceId, QString>& mapping)
{
    m_deviceNumberMap = mapping;
    initAllUIControls();
}

// 添加单个舵机设备
void RudderHandler::addDevice(DeviceId deviceId, const QString& number)
{
    m_deviceNumberMap[deviceId] = number;
    RudderUIControls controls;
    controls.number = number;
    controls.init(m_parent, number);
    m_uiControls[deviceId] = controls;
}

// -------------------- 槽函数实现 --------------------
void RudderHandler::onPowerOnClicked(DeviceId deviceId)
{
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Common_PowerOn, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("发送上电命令失败: %1").arg(e.what()));
    }
}

void RudderHandler::onPowerOffClicked(DeviceId deviceId)
{
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Common_PowerOff, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("发送下电命令失败: %1").arg(e.what()));
    }
}

void RudderHandler::onSetAngleClicked(DeviceId deviceId, const RudderUIControls& controls)
{
    try {
        // 获取输入并创建角度参数
        auto params = createAngleParameter(controls.targetAngleInput ? controls.targetAngleInput->text().trimmed() : "",
            "目标角度");
        // 发送设置角度命令
        m_module->sendCheckCommand(deviceId, CommandCode::Rudder_SetAngle, params);

        // 提示信息
        double angle = controls.targetAngleInput->text().toDouble();
        QMessageBox::information(m_parent, "提示", QString("已发送舵角设置命令：%1度").arg(angle, 0, 'f', 1));
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "参数错误", QString("%1").arg(e.what()));
    }
}

void RudderHandler::onGetRunTimeClicked(DeviceId deviceId)
{
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Rudder_GetRunTime, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("发送获取运行时间命令失败: %1").arg(e.what()));
    }
}

void RudderHandler::onGetVoltageClicked(DeviceId deviceId)
{
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Rudder_GetVoltage, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("发送获取电压命令失败: %1").arg(e.what()));
    }
}

void RudderHandler::onGetAngleClicked(DeviceId deviceId)
{
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Rudder_GetAngle, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("发送获取角度命令失败: %1").arg(e.what()));
    }
}

void RudderHandler::onGetCurrentClicked(DeviceId deviceId)
{
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Rudder_GetCurrent, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("发送获取电流命令失败: %1").arg(e.what()));
    }
}

void RudderHandler::onGetTempClicked(DeviceId deviceId)
{
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Rudder_GetTemp, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("发送获取温度命令失败: %1").arg(e.what()));
    }
}

void RudderHandler::onSetZeroClicked(DeviceId deviceId)
{
    // 暂留扩展：零位设置逻辑
    Q_UNUSED(deviceId);
    QMessageBox::information(m_parent, "提示", "零位设置功能待实现");
}

void RudderHandler::onGetFaultClicked(DeviceId deviceId)
{
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Common_GetFault, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("发送获取故障命令失败: %1").arg(e.what()));
    }
}