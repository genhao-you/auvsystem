#include "thrusterhandler.h"
#include <QIntValidator>
#include "widgetmanager.h"
#include <QMessageBox>
#include <QDebug>
ThrusterHandler::ThrusterHandler(SelfCheckModule* module, QWidget* parent)
    : DeviceHandlerBase(module, parent)
{
    initDefaultDeviceMapping();
    initAllUIControls();
    initializeUI();
}

ThrusterHandler::~ThrusterHandler()
{
    disconnectSignals();
}

void ThrusterHandler::initDefaultDeviceMapping()
{
    m_deviceNumberMap = {
        {DeviceId::TailThruster, "01"},
        {DeviceId::FrontVerticalThruster, "16"},
        {DeviceId::RearVerticalThruster, "17"},
        {DeviceId::FrontSideThruster, "18"},
        {DeviceId::RearSideThruster, "0B"}
    };
}

void ThrusterHandler::ThrusterUIControls::init(QWidget* parent, const QString& prefix)
{
    // 获取按钮控件
    powerOnBtn = WidgetManager::instance().getWidget<QPushButton>(
        QString("btn_%1_PowerOn").arg(prefix), parent);
    powerOffBtn = WidgetManager::instance().getWidget<QPushButton>(
        QString("btn_%1_PowerOff").arg(prefix), parent);
    paramResetBtn = WidgetManager::instance().getWidget<QPushButton>(
        QString("btn_%1_ParamReset").arg(prefix), parent);
    enableWorkBtn = WidgetManager::instance().getWidget<QPushButton>(
        QString("btn_%1_EnableWork").arg(prefix), parent);
    setSpeedBtn = WidgetManager::instance().getWidget<QPushButton>(
        QString("btn_%1_SetTargetSpeed").arg(prefix), parent);
    setDutyBtn = WidgetManager::instance().getWidget<QPushButton>(
        QString("btn_%1_SetTargetDuty").arg(prefix), parent);
    setAccelTimeBtn = WidgetManager::instance().getWidget<QPushButton>(
        QString("btn_%1_SetAccelTime").arg(prefix), parent);
    setRunTimeBtn = WidgetManager::instance().getWidget<QPushButton>(
        QString("btn_%1_SetRunTime").arg(prefix), parent);
    getRunTimeBtn = WidgetManager::instance().getWidget<QPushButton>(
        QString("btn_%1_ReadRunTime").arg(prefix), parent);
    getDutyBtn = WidgetManager::instance().getWidget<QPushButton>(
        QString("btn_%1_ReadTargetDuty").arg(prefix), parent);
    getCurrentBtn = WidgetManager::instance().getWidget<QPushButton>(
        QString("btn_%1_ReadCurrent").arg(prefix), parent);
    getTempBtn = WidgetManager::instance().getWidget<QPushButton>(
        QString("btn_%1_ReadTemp").arg(prefix), parent);
    getSpeedBtn = WidgetManager::instance().getWidget<QPushButton>(
        QString("btn_%1_ReadTargetSpeed").arg(prefix), parent);
    getHallBtn = WidgetManager::instance().getWidget<QPushButton>(
        QString("btn_%1_ReadHall").arg(prefix), parent);
    getVoltageBtn = WidgetManager::instance().getWidget<QPushButton>(
        QString("btn_%1_ReadVoltage").arg(prefix), parent);
    getNoMaintainTimeBtn = WidgetManager::instance().getWidget<QPushButton>(
        QString("btn_%1_ReadNoMaintainTime").arg(prefix), parent);
    getTotalRunTimeBtn = WidgetManager::instance().getWidget<QPushButton>(
        QString("btn_%1_ReadTotalRunTime").arg(prefix), parent);
    setSpeedWithTimeBtn = WidgetManager::instance().getWidget<QPushButton>(
        QString("btn_%1_SetSpeedWithTime").arg(prefix), parent);

    // 获取图标控件
    powerOnIcon = WidgetManager::instance().getWidget<QLabel>(
        QString("lbl_%1_PowerOnIcon").arg(prefix), parent);
    powerOffIcon = WidgetManager::instance().getWidget<QLabel>(
        QString("lbl_%1_PowerOffIcon").arg(prefix), parent);
    paramResetIcon = WidgetManager::instance().getWidget<QLabel>(
        QString("lbl_%1_ParamResetIcon").arg(prefix), parent);
    enableWorkIcon = WidgetManager::instance().getWidget<QLabel>(
        QString("lbl_%1_EnableWorkIcon").arg(prefix), parent);
    setSpeedIcon = WidgetManager::instance().getWidget<QLabel>(
        QString("lbl_%1_SetSpeedIcon").arg(prefix), parent);
    setDutyIcon = WidgetManager::instance().getWidget<QLabel>(
        QString("lbl_%1_SetDutyIcon").arg(prefix), parent);
    setAccelTimeIcon = WidgetManager::instance().getWidget<QLabel>(
        QString("lbl_%1_SetAccelTimeIcon").arg(prefix), parent);
    setRunTimeIcon = WidgetManager::instance().getWidget<QLabel>(
        QString("lbl_%1_SetRunTimeIcon").arg(prefix), parent);
    getRunTimeIcon = WidgetManager::instance().getWidget<QLabel>(
        QString("lbl_%1_ReadRunTimeIcon").arg(prefix), parent);
    getDutyIcon = WidgetManager::instance().getWidget<QLabel>(
        QString("lbl_%1_ReadDutyIcon").arg(prefix), parent);
    getCurrentIcon = WidgetManager::instance().getWidget<QLabel>(
        QString("lbl_%1_ReadCurrentIcon").arg(prefix), parent);
    getTempIcon = WidgetManager::instance().getWidget<QLabel>(
        QString("lbl_%1_ReadTempIcon").arg(prefix), parent);
    getSpeedIcon = WidgetManager::instance().getWidget<QLabel>(
        QString("lbl_%1_ReadSpeedIcon").arg(prefix), parent);
    getHallIcon = WidgetManager::instance().getWidget<QLabel>(
        QString("lbl_%1_ReadHallIcon").arg(prefix), parent);
    getVoltageIcon = WidgetManager::instance().getWidget<QLabel>(
        QString("lbl_%1_ReadVoltageIcon").arg(prefix), parent);
    getNoMaintainTimeIcon = WidgetManager::instance().getWidget<QLabel>(
        QString("lbl_%1_ReadNoMaintainTimeIcon").arg(prefix), parent);
    getTotalRunTimeIcon = WidgetManager::instance().getWidget<QLabel>(
        QString("lbl_%1_ReadTotalRunTimeIcon").arg(prefix), parent);
    setSpeedWithTimeIcon = WidgetManager::instance().getWidget<QLabel>(
        QString("lbl_%1_SetSpeedWithTimeIcon").arg(prefix), parent);

    // 获取反馈文本框
    powerOnFeedback = WidgetManager::instance().getWidget<QLineEdit>(
        QString("le_%1_PowerOn_Feedback").arg(prefix), parent);
    powerOffFeedback = WidgetManager::instance().getWidget<QLineEdit>(
        QString("le_%1_PowerOff_Feedback").arg(prefix), parent);
    paramResetFeedback = WidgetManager::instance().getWidget<QLineEdit>(
        QString("le_%1_ParamReset_Feedback").arg(prefix), parent);
    enableWorkFeedback = WidgetManager::instance().getWidget<QLineEdit>(
        QString("le_%1_EnableWork_Feedback").arg(prefix), parent);
    setAccelTimeFeedback = WidgetManager::instance().getWidget<QLineEdit>(
        QString("le_%1_SetAccelTime_Feedback").arg(prefix), parent);
    setSpeedWithTimeFeedback = WidgetManager::instance().getWidget<QLineEdit>(
        QString("le_%1_SetSpeedWithTime_Feedback").arg(prefix), parent);
    getRunTimeFeedback = WidgetManager::instance().getWidget<QLineEdit>(
        QString("le_%1_ReadRunTime_Feedback").arg(prefix), parent);
    getDutyFeedback = WidgetManager::instance().getWidget<QLineEdit>(
        QString("le_%1_ReadDuty_Feedback").arg(prefix), parent);
    getCurrentFeedback = WidgetManager::instance().getWidget<QLineEdit>(
        QString("le_%1_ReadCurrent_Feedback").arg(prefix), parent);
    getTempFeedback = WidgetManager::instance().getWidget<QLineEdit>(
        QString("le_%1_ReadTemp_Feedback").arg(prefix), parent);
    getSpeedFeedback = WidgetManager::instance().getWidget<QLineEdit>(
        QString("le_%1_ReadSpeed_Feedback").arg(prefix), parent);
    getHallFeedback = WidgetManager::instance().getWidget<QLineEdit>(
        QString("le_%1_ReadHall_Feedback").arg(prefix), parent);
    getVoltageFeedback = WidgetManager::instance().getWidget<QLineEdit>(
        QString("le_%1_ReadVoltage_Feedback").arg(prefix), parent);
    getNoMaintainTimeFeedback = WidgetManager::instance().getWidget<QLineEdit>(
        QString("le_%1_ReadNoMaintainTime_Feedback").arg(prefix), parent);
    getTotalRunTimeFeedback = WidgetManager::instance().getWidget<QLineEdit>(
        QString("le_%1_ReadTotalRunTime_Feedback").arg(prefix), parent);

    // 获取输入文本框
    targetSpeedInput = WidgetManager::instance().getWidget<QLineEdit>(
        QString("le_%1_TargetSpeed_Input").arg(prefix), parent);
    targetDutyInput = WidgetManager::instance().getWidget<QLineEdit>(
        QString("le_%1_TargetDuty_Input").arg(prefix), parent);
    accelTimeInput = WidgetManager::instance().getWidget<QLineEdit>(
        QString("le_%1_AccelTime_Input").arg(prefix), parent);
    setRunTimeInput = WidgetManager::instance().getWidget<QLineEdit>(
        QString("le_%1_SetRunTime_Input").arg(prefix), parent);
    speedInput = WidgetManager::instance().getWidget<QLineEdit>(
        QString("le_%1_Speed").arg(prefix), parent);
    timeInput = WidgetManager::instance().getWidget<QLineEdit>(
        QString("le_%1_Time").arg(prefix), parent);
}

void ThrusterHandler::initAllUIControls()
{
    for (auto it = m_deviceNumberMap.begin(); it != m_deviceNumberMap.end(); ++it) {
        ThrusterUIControls controls;
        controls.number = it.value();
        controls.init(m_parent, controls.number);
        m_uiControls[it.key()] = controls;
    }
}

void ThrusterHandler::initializeUI()
{
    // 设置输入验证器
    for (auto& controls : m_uiControls) {
        if (controls.targetSpeedInput) {
            controls.targetSpeedInput->setValidator(new QIntValidator(-450, 450, controls.targetSpeedInput));
        }
        if (controls.targetDutyInput) {
            controls.targetDutyInput->setValidator(new QIntValidator(0, 2000, controls.targetDutyInput));
        }
        if (controls.accelTimeInput) {
            controls.accelTimeInput->setValidator(new QIntValidator(0, 65535, controls.accelTimeInput));
        }
        if (controls.setRunTimeInput) {
            controls.setRunTimeInput->setValidator(new QIntValidator(0, 65535, controls.setRunTimeInput));
        }
        if (controls.speedInput) {
            controls.speedInput->setValidator(new QIntValidator(-450, 450, controls.speedInput));
        }
        if (controls.timeInput) {
            controls.timeInput->setValidator(new QIntValidator(0, 255, controls.timeInput));
        }

        // 清空反馈文本框
        if (controls.powerOnFeedback) controls.powerOnFeedback->clear();
        if (controls.powerOffFeedback) controls.powerOffFeedback->clear();
        if (controls.paramResetFeedback) controls.paramResetFeedback->clear();
        if (controls.enableWorkFeedback) controls.enableWorkFeedback->clear();
        if (controls.setAccelTimeFeedback) controls.setAccelTimeFeedback->clear();
        if (controls.setSpeedWithTimeFeedback) controls.setSpeedWithTimeFeedback->clear();
        if (controls.getRunTimeFeedback) controls.getRunTimeFeedback->clear();
        if (controls.getDutyFeedback) controls.getDutyFeedback->clear();
        if (controls.getCurrentFeedback) controls.getCurrentFeedback->clear();
        if (controls.getTempFeedback) controls.getTempFeedback->clear();
        if (controls.getSpeedFeedback) controls.getSpeedFeedback->clear();
        if (controls.getHallFeedback) controls.getHallFeedback->clear();
        if (controls.getVoltageFeedback) controls.getVoltageFeedback->clear();
        if (controls.getNoMaintainTimeFeedback) controls.getNoMaintainTimeFeedback->clear();
        if (controls.getTotalRunTimeFeedback) controls.getTotalRunTimeFeedback->clear();
    }
}

void ThrusterHandler::connectSignals()
{
    disconnectSignals();
    // 为每个推进器连接信号
    for (auto it = m_uiControls.begin(); it != m_uiControls.end(); ++it) {
        DeviceId deviceId = it.key();
        ThrusterUIControls& controls = it.value();
        // 🔴 调试日志：打印当前连接的设备
        qDebug() << "[连接信号] DeviceId:" << static_cast<int>(deviceId)
            << " 前缀:" << controls.number
            << " PowerOnBtn地址:" << controls.powerOnBtn;
        // 连接每个按钮
        if (controls.powerOnBtn) {
            connect(controls.powerOnBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onPowerOnClicked(deviceId); });
        }
        if (controls.powerOffBtn) {
            connect(controls.powerOffBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onPowerOffClicked(deviceId); });
        }
        if (controls.paramResetBtn) {
            connect(controls.paramResetBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onParamResetClicked(deviceId); });
        }
        if (controls.enableWorkBtn) {
            connect(controls.enableWorkBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onEnableWorkClicked(deviceId); });
        }
        if (controls.setSpeedBtn) {
            connect(controls.setSpeedBtn, &QPushButton::clicked,
                this, [this, deviceId, &controls]() { onSetSpeedClicked(deviceId, controls); });
        }
        if (controls.setDutyBtn) {
            connect(controls.setDutyBtn, &QPushButton::clicked,
                this, [this, deviceId, &controls]() { onSetDutyClicked(deviceId, controls); });
        }
        if (controls.setAccelTimeBtn) {
            connect(controls.setAccelTimeBtn, &QPushButton::clicked,
                this, [this, deviceId, &controls]() { onSetAccelTimeClicked(deviceId, controls); });
        }
        if (controls.setRunTimeBtn) {
            connect(controls.setRunTimeBtn, &QPushButton::clicked,
                this, [this, deviceId, &controls]() { onSetRunTimeClicked(deviceId, controls); });
        }
        if (controls.getRunTimeBtn) {
            connect(controls.getRunTimeBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetRunTimeClicked(deviceId); });
        }
        if (controls.getDutyBtn) {
            connect(controls.getDutyBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetDutyClicked(deviceId); });
        }
        if (controls.getCurrentBtn) {
            connect(controls.getCurrentBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetCurrentClicked(deviceId); });
        }
        if (controls.getTempBtn) {
            connect(controls.getTempBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetTempClicked(deviceId); });
        }
        if (controls.getSpeedBtn) {
            connect(controls.getSpeedBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetSpeedClicked(deviceId); });
        }
        if (controls.getHallBtn) {
            connect(controls.getHallBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetHallClicked(deviceId); });
        }
        if (controls.getVoltageBtn) {
            connect(controls.getVoltageBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetVoltageClicked(deviceId); });
        }
        if (controls.getNoMaintainTimeBtn) {
            connect(controls.getNoMaintainTimeBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetNoMaintainTimeClicked(deviceId); });
        }
        if (controls.getTotalRunTimeBtn) {
            connect(controls.getTotalRunTimeBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetTotalRunTimeClicked(deviceId); });
        }
        if (controls.setSpeedWithTimeBtn) {
            connect(controls.setSpeedWithTimeBtn, &QPushButton::clicked,
                this, [this, deviceId, &controls]() { onSetSpeedWithTimeClicked(deviceId, controls); });
        }
    }
}

void ThrusterHandler::disconnectSignals()
{
    // 断开所有按钮的信号
    for (auto& controls : m_uiControls) {
        if (controls.powerOnBtn) controls.powerOnBtn->disconnect();
        if (controls.powerOffBtn) controls.powerOffBtn->disconnect();
        if (controls.paramResetBtn) controls.paramResetBtn->disconnect();
        if (controls.enableWorkBtn) controls.enableWorkBtn->disconnect();
        if (controls.setSpeedBtn) controls.setSpeedBtn->disconnect();
        if (controls.setDutyBtn) controls.setDutyBtn->disconnect();
        if (controls.setAccelTimeBtn) controls.setAccelTimeBtn->disconnect();
        if (controls.setRunTimeBtn) controls.setRunTimeBtn->disconnect();
        if (controls.getRunTimeBtn) controls.getRunTimeBtn->disconnect();
        if (controls.getDutyBtn) controls.getDutyBtn->disconnect();
        if (controls.getCurrentBtn) controls.getCurrentBtn->disconnect();
        if (controls.getTempBtn) controls.getTempBtn->disconnect();
        if (controls.getSpeedBtn) controls.getSpeedBtn->disconnect();
        if (controls.getHallBtn) controls.getHallBtn->disconnect();
        if (controls.getVoltageBtn) controls.getVoltageBtn->disconnect();
        if (controls.getNoMaintainTimeBtn) controls.getNoMaintainTimeBtn->disconnect();
        if (controls.getTotalRunTimeBtn) controls.getTotalRunTimeBtn->disconnect();
        if (controls.setSpeedWithTimeBtn) controls.setSpeedWithTimeBtn->disconnect();
    }
}

void ThrusterHandler::handleResult(const SelfCheckResult* result)
{
    if (!result) return;

    DeviceId deviceId = result->deviceId();
    if (!m_uiControls.contains(deviceId)) {
        qWarning() << "不支持的推进器设备:" << EnumConverter::deviceIdToString(deviceId);
        return;
    }

    processThrusterCommand(deviceId, result->commandCode(), result);
}

void ThrusterHandler::processThrusterCommand(DeviceId deviceId, CommandCode cmdCode,
    const SelfCheckResult* result)
{
    const ThrusterUIControls& controls = m_uiControls[deviceId];
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

    case CommandCode::Thruster_ParamReset:
        setIcon(controls.paramResetIcon, icon);
        updateStatusWidget(controls.paramResetFeedback, status, feedbackDesc);
        break;

    case CommandCode::Common_Enable:
        setIcon(controls.enableWorkIcon, icon);
        updateStatusWidget(controls.enableWorkFeedback, status, feedbackDesc);
        break;

    case CommandCode::Thruster_SetSpeed:
        setIcon(controls.setSpeedIcon, icon);
        break;

    case CommandCode::Thruster_SetDuty:
        setIcon(controls.setDutyIcon, icon);
        break;

    case CommandCode::Thruster_SetAccelTime:
        setIcon(controls.setAccelTimeIcon, icon);
        updateStatusWidget(controls.setAccelTimeFeedback, status, feedbackDesc);
        break;

    case CommandCode::Thruster_SetRunTime:
        setIcon(controls.setRunTimeIcon, icon);
        break;

    case CommandCode::Thruster_GetSpeed:
        handleSpeedCommand(controls, result);
        break;

    case CommandCode::Thruster_GetDuty:
        handleDutyCommand(controls, result);
        break;

    case CommandCode::Thruster_GetCurrent:
        handleCurrentCommand(controls, result);
        break;

    case CommandCode::Thruster_GetTemperature:
        handleTemperatureCommand(controls, result);
        break;

    case CommandCode::Thruster_GetBusVoltage:
        handleVoltageCommand(controls, result);
        break;

    case CommandCode::Thruster_GetRunTime:
        handleRunTimeCommand(controls, result);
        break;

    case CommandCode::Thruster_GetNoMaintainTime:
        handleNoMaintainTimeCommand(controls, result);
        break;

    case CommandCode::Thruster_GetTotalRunTime:
        handleTotalRunTimeCommand(controls, result);
        break;

    case CommandCode::Thruster_GetHall:
        handleHallCommand(controls, result);
        break;

    case CommandCode::Thruster_SetSpeedWithTime:
        handleSetSpeedWithTimeCommand(controls, result);
        break;

    default:
        qDebug() << "未处理的推进器命令:" << static_cast<int>(cmdCode);
        break;
    }
}

void ThrusterHandler::handlePowerCommand(const ThrusterUIControls& controls, CommandCode cmdCode,
    const SelfCheckResult* result)
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

void ThrusterHandler::handleSpeedCommand(const ThrusterUIControls& controls, const SelfCheckResult* result)
{
    setIcon(controls.getSpeedIcon, getStatusIcon(result->status()));
    uint16_t speed = result->getParameter("speed").toUInt();
    updateStatusWidget(controls.getSpeedFeedback, result->status(),
        QString("%1 RPM").arg(speed));
}

void ThrusterHandler::handleDutyCommand(const ThrusterUIControls& controls, const SelfCheckResult* result)
{
    setIcon(controls.getDutyIcon, getStatusIcon(result->status()));
    uint16_t duty = result->getParameter("duty").toUInt();
    updateStatusWidget(controls.getDutyFeedback, result->status(),
        QString("%1 (1000=停止)").arg(duty));
}

void ThrusterHandler::handleCurrentCommand(const ThrusterUIControls& controls, const SelfCheckResult* result)
{
    setIcon(controls.getCurrentIcon, getStatusIcon(result->status()));
    double current = result->getParameter("current").toDouble();
    updateStatusWidget(controls.getCurrentFeedback, result->status(),
        QString("%1 A").arg(current, 0, 'f', 2));
}

void ThrusterHandler::handleTemperatureCommand(const ThrusterUIControls& controls, const SelfCheckResult* result)
{
    setIcon(controls.getTempIcon, getStatusIcon(result->status()));
    uint16_t temp = result->getParameter("temperature").toUInt();
    updateStatusWidget(controls.getTempFeedback, result->status(),
        QString("%1 ℃").arg(temp / 10.0, 0, 'f', 1));
}

void ThrusterHandler::handleVoltageCommand(const ThrusterUIControls& controls, const SelfCheckResult* result)
{
    setIcon(controls.getVoltageIcon, getStatusIcon(result->status()));
    double volt = result->getParameter("volt").toDouble();
    updateStatusWidget(controls.getVoltageFeedback, result->status(),
        QString("%1 V").arg(volt, 0, 'f', 1));
}

void ThrusterHandler::handleRunTimeCommand(const ThrusterUIControls& controls, const SelfCheckResult* result)
{
    setIcon(controls.getRunTimeIcon, getStatusIcon(result->status()));
    uint16_t runtime = result->getParameter("runtime").toUInt();
    updateStatusWidget(controls.getRunTimeFeedback, result->status(),
        QString("%1 秒").arg(runtime));
}

void ThrusterHandler::handleHallCommand(const ThrusterUIControls& controls, const SelfCheckResult* result)
{
    setIcon(controls.getHallIcon, getStatusIcon(result->status()));
    uint8_t hall = result->getParameter("hall").toUInt();
    QString hallDesc = (hall == 0 || hall == 7) ?
        QString("%1 (故障)").arg(hall) : QString("%1 (正常)").arg(hall);
    updateStatusWidget(controls.getHallFeedback, result->status(), hallDesc);
}

void ThrusterHandler::handleNoMaintainTimeCommand(const ThrusterUIControls& controls, const SelfCheckResult* result)
{
    setIcon(controls.getNoMaintainTimeIcon, getStatusIcon(result->status()));
    uint16_t hours = result->getParameter("noMaintainTime").toUInt();
    updateStatusWidget(controls.getNoMaintainTimeFeedback, result->status(),
        QString("%1 小时").arg(hours));
}

void ThrusterHandler::handleTotalRunTimeCommand(const ThrusterUIControls& controls, const SelfCheckResult* result)
{
    setIcon(controls.getTotalRunTimeIcon, getStatusIcon(result->status()));
    uint32_t totalSec = result->getParameter("totalRunTime").toUInt();
    updateStatusWidget(controls.getTotalRunTimeFeedback, result->status(),
        QString("%1 秒").arg(totalSec));
}

void ThrusterHandler::handleSetSpeedWithTimeCommand(const ThrusterUIControls& controls, const SelfCheckResult* result)
{
    setIcon(controls.setSpeedWithTimeIcon, getStatusIcon(result->status()));
    updateStatusWidget(controls.setSpeedWithTimeFeedback, result->status(),
        result->feedbackDesc());
}

std::shared_ptr<SpeedParameter> ThrusterHandler::createSpeedParameter(const QString& inputText,
    const QString& paramName) const
{
    if (inputText.isEmpty()) {
        throw std::runtime_error(QString("%1不能为空").arg(paramName).toStdString());
    }

    bool ok = false;
    int16_t speed = inputText.toInt(&ok);
    if (!ok) {
        throw std::runtime_error(QString("%1必须为整数").arg(paramName).toStdString());
    }

    if (speed < -450 || speed > 450) {
        throw std::runtime_error(QString("%1超出范围(-450~450)").arg(paramName).toStdString());
    }

    return std::make_shared<SpeedParameter>(speed);
}

std::shared_ptr<ThrusterDutyParameter> ThrusterHandler::createDutyParameter(const QString& inputText,
    const QString& paramName) const
{
    if (inputText.isEmpty()) {
        throw std::runtime_error(QString("%1不能为空").arg(paramName).toStdString());
    }

    bool ok = false;
    uint16_t duty = inputText.toUInt(&ok);
    if (!ok) {
        throw std::runtime_error(QString("%1必须为非负整数").arg(paramName).toStdString());
    }

    if (duty > 2000) {
        throw std::runtime_error(QString("%1超出范围(0~2000)").arg(paramName).toStdString());
    }

    return std::make_shared<ThrusterDutyParameter>(duty);
}

std::shared_ptr<ThrusterRunTimeParameter> ThrusterHandler::createRunTimeParameter(const QString& inputText,
    const QString& paramName) const
{
    if (inputText.isEmpty()) {
        throw std::runtime_error(QString("%1不能为空").arg(paramName).toStdString());
    }

    bool ok = false;
    uint16_t runTime = inputText.toUInt(&ok);
    if (!ok) {
        throw std::runtime_error(QString("%1必须为非负整数").arg(paramName).toStdString());
    }

    return std::make_shared<ThrusterRunTimeParameter>(runTime);
}

std::shared_ptr<AccelTimeParameter> ThrusterHandler::createAccelTimeParameter(const QString& inputText,
    const QString& paramName) const
{
    if (inputText.isEmpty()) {
        throw std::runtime_error(QString("%1不能为空").arg(paramName).toStdString());
    }

    bool ok = false;
    uint16_t accelTime = inputText.toUInt(&ok);
    if (!ok) {
        throw std::runtime_error(QString("%1必须为非负整数").arg(paramName).toStdString());
    }

    return std::make_shared<AccelTimeParameter>(accelTime);
}

std::shared_ptr<ThrusterSpeedAndRunTimeParameter> ThrusterHandler::createSpeedAndTimeParameter(
    const QString& speedInput, const QString& timeInput, const QString& paramName) const
{
    if (speedInput.isEmpty()) {
        throw std::runtime_error(QString("%1转速不能为空").arg(paramName).toStdString());
    }

    if (timeInput.isEmpty()) {
        throw std::runtime_error(QString("%1时间不能为空").arg(paramName).toStdString());
    }

    bool ok = false;
    int16_t speed = speedInput.toInt(&ok);
    if (!ok || speed < -450 || speed > 450) {
        throw std::runtime_error(QString("%1转速超出范围(-450~450)").arg(paramName).toStdString());
    }

    ok = false;
    uint8_t runTime = timeInput.toUInt(&ok);
    if (!ok || runTime > 255) {
        throw std::runtime_error(QString("%1时间超出范围(0~255)").arg(paramName).toStdString());
    }

    return std::make_shared<ThrusterSpeedAndRunTimeParameter>(speed, runTime);
}

bool ThrusterHandler::validateInput(const QString& inputText, const QString& paramName,
    int minVal, int maxVal, QString& errorMsg) const
{
    if (inputText.isEmpty()) {
        errorMsg = QString("%1不能为空").arg(paramName);
        return false;
    }

    bool ok = false;
    int value = inputText.toInt(&ok);
    if (!ok) {
        errorMsg = QString("%1必须为整数").arg(paramName);
        return false;
    }

    if (value < minVal || value > maxVal) {
        errorMsg = QString("%1超出范围(%2~%3)").arg(paramName).arg(minVal).arg(maxVal);
        return false;
    }

    return true;
}

QString ThrusterHandler::getDeviceNumber(DeviceId deviceId) const
{
    return m_deviceNumberMap.value(deviceId, "");
}

bool ThrusterHandler::supportsDevice(DeviceId deviceId) const
{
    return m_deviceNumberMap.contains(deviceId);
}

void ThrusterHandler::setDeviceMapping(const QMap<DeviceId, QString>& mapping)
{
    m_deviceNumberMap = mapping;
    // 重新初始化UI控件
    initAllUIControls();
}

void ThrusterHandler::addDevice(DeviceId deviceId, const QString& number)
{
    m_deviceNumberMap[deviceId] = number;
    ThrusterUIControls controls;
    controls.number = number;
    controls.init(m_parent, number);
    m_uiControls[deviceId] = controls;
}

void ThrusterHandler::onPowerOnClicked(DeviceId deviceId)
{
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Common_PowerOn, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("发送命令失败: %1").arg(e.what()));
    }
}

void ThrusterHandler::onPowerOffClicked(DeviceId deviceId)
{
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Common_PowerOff, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("发送命令失败: %1").arg(e.what()));
    }
}

void ThrusterHandler::onParamResetClicked(DeviceId deviceId)
{
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Thruster_ParamReset, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("发送命令失败: %1").arg(e.what()));
    }
}

void ThrusterHandler::onEnableWorkClicked(DeviceId deviceId)
{
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Common_Enable, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("发送命令失败: %1").arg(e.what()));
    }
}

void ThrusterHandler::onSetSpeedClicked(DeviceId deviceId, const ThrusterUIControls& controls)
{
    try {
        auto params = createSpeedParameter(controls.targetSpeedInput ?
            controls.targetSpeedInput->text() : "", "目标转速");
        m_module->sendCheckCommand(deviceId, CommandCode::Thruster_SetSpeed, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("参数错误: %1").arg(e.what()));
    }
}

void ThrusterHandler::onSetDutyClicked(DeviceId deviceId, const ThrusterUIControls& controls)
{
    try {
        auto params = createDutyParameter(controls.targetDutyInput ?
            controls.targetDutyInput->text() : "", "目标占空比");
        m_module->sendCheckCommand(deviceId, CommandCode::Thruster_SetDuty, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("参数错误: %1").arg(e.what()));
    }
}

void ThrusterHandler::onSetAccelTimeClicked(DeviceId deviceId, const ThrusterUIControls& controls)
{
    try {
        auto params = createAccelTimeParameter(controls.accelTimeInput ?
            controls.accelTimeInput->text() : "", "加速时间");
        m_module->sendCheckCommand(deviceId, CommandCode::Thruster_SetAccelTime, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("参数错误: %1").arg(e.what()));
    }
}

void ThrusterHandler::onSetRunTimeClicked(DeviceId deviceId, const ThrusterUIControls& controls)
{
    try {
        auto params = createRunTimeParameter(controls.setRunTimeInput ?
            controls.setRunTimeInput->text() : "", "运行时间");
        m_module->sendCheckCommand(deviceId, CommandCode::Thruster_SetRunTime, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("参数错误: %1").arg(e.what()));
    }
}

void ThrusterHandler::onGetRunTimeClicked(DeviceId deviceId)
{
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Thruster_GetRunTime, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("发送命令失败: %1").arg(e.what()));
    }
}

void ThrusterHandler::onGetDutyClicked(DeviceId deviceId)
{
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Thruster_GetDuty, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("发送命令失败: %1").arg(e.what()));
    }
}

void ThrusterHandler::onGetCurrentClicked(DeviceId deviceId)
{
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Thruster_GetCurrent, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("发送命令失败: %1").arg(e.what()));
    }
}

void ThrusterHandler::onGetTempClicked(DeviceId deviceId)
{
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Thruster_GetTemperature, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("发送命令失败: %1").arg(e.what()));
    }
}

void ThrusterHandler::onGetSpeedClicked(DeviceId deviceId)
{
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Thruster_GetSpeed, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("发送命令失败: %1").arg(e.what()));
    }
}

void ThrusterHandler::onGetHallClicked(DeviceId deviceId)
{
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Thruster_GetHall, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("发送命令失败: %1").arg(e.what()));
    }
}

void ThrusterHandler::onGetVoltageClicked(DeviceId deviceId)
{
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Thruster_GetBusVoltage, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("发送命令失败: %1").arg(e.what()));
    }
}

void ThrusterHandler::onGetNoMaintainTimeClicked(DeviceId deviceId)
{
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Thruster_GetNoMaintainTime, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("发送命令失败: %1").arg(e.what()));
    }
}

void ThrusterHandler::onGetTotalRunTimeClicked(DeviceId deviceId)
{
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Thruster_GetTotalRunTime, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("发送命令失败: %1").arg(e.what()));
    }
}

void ThrusterHandler::onSetSpeedWithTimeClicked(DeviceId deviceId, const ThrusterUIControls& controls)
{
    try {
        QString speedText = controls.speedInput ? controls.speedInput->text() : "";
        QString timeText = controls.timeInput ? controls.timeInput->text() : "";
        auto params = createSpeedAndTimeParameter(speedText, timeText, "转速和时间");
        m_module->sendCheckCommand(deviceId, CommandCode::Thruster_SetSpeedWithTime, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("参数错误: %1").arg(e.what()));
    }
}