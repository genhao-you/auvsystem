#include "inshandler.h"

#include "widgetmanager.h"
#include <QMessageBox>
#include <QDebug>

INSHandler::INSHandler(SelfCheckModule* module, QWidget* parent)
    : DeviceHandlerBase(module, parent)
{
    initDefaultDeviceMapping();
    initAllUIControls();
    initializeUI();
}

INSHandler::~INSHandler()
{
    disconnectSignals();
}

// 初始化INS默认编号映射（0D）
void INSHandler::initDefaultDeviceMapping()
{
    m_deviceNumberMap = { {DeviceId::INS, "0D"} };
}

// 初始化INS UI控件
void INSHandler::INSUIControls::init(QWidget* parent, const QString& prefix)
{
    // 按钮
    powerOnBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_PowerOn").arg(prefix), parent);
    powerOffBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_PowerOff").arg(prefix), parent);
    setGnssBindBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_SetGnssBind").arg(prefix), parent);
    calibrationWithDVLBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_SetCalibWithDvl").arg(prefix), parent);
    combinedWithDVLBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_SetCombinedWithDvl").arg(prefix), parent);
    combinedAutoBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_SetCombinedAuto").arg(prefix), parent);
    getUtcDateBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_GetUtcDate").arg(prefix), parent);
    getUtcTimeBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_GetUtcTime").arg(prefix), parent);
    getAttitudeBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_GetAttitude").arg(prefix), parent);
    getBodySpeedBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_GetBodySpeed").arg(prefix), parent);
    getGeoSpeedBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_GetGeoSpeed").arg(prefix), parent);
    getPositionBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_GetPosition").arg(prefix), parent);
    getGyroscopeSpeedBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_GetGyroSpeed").arg(prefix), parent);
    getAccelerationBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_GetAccel").arg(prefix), parent);
    getStatusBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_GetStatus").arg(prefix), parent);

    // 参数输入框（GNSS绑定）
    setGnssBindLonEdit = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_SetGnssBindLon").arg(prefix), parent);
    setGnssBindLatEdit = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_SetGnssBindLat").arg(prefix), parent);

    // 状态图标
    powerOnIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_PowerOnIcon").arg(prefix), parent);
    powerOffIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_PowerOffIcon").arg(prefix), parent);
    setGnssBindIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_SetGnssBindIcon").arg(prefix), parent);
    calibrationWithDVLIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_CalibWithDvlIcon").arg(prefix), parent);
    combinedWithDVLIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_CombinedWithDvlIcon").arg(prefix), parent);
    combinedAutoIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_CombinedAutoIcon").arg(prefix), parent);
    getUtcDateIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_GetUtcDateIcon").arg(prefix), parent);
    getUtcTimeIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_GetUtcTimeIcon").arg(prefix), parent);
    getAttitudeIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_GetAttitudeIcon").arg(prefix), parent);
    getBodySpeedIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_GetBodySpeedIcon").arg(prefix), parent);
    getGeoSpeedIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_GetGeoSpeedIcon").arg(prefix), parent);
    getPositionIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_GetPositionIcon").arg(prefix), parent);
    getGyroscopeSpeedIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_GetGyroIcon").arg(prefix), parent);
    getAccelerationIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_GetAccelIcon").arg(prefix), parent);
    getStatusIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_GetStatusIcon").arg(prefix), parent);

    // 反馈文本框
    powerOnFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_PowerOn_Feedback").arg(prefix), parent);
    powerOffFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_PowerOff_Feedback").arg(prefix), parent);
    setGnssBindFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_SetGnssBind_Feedback").arg(prefix), parent);
    calibrationWithDVLFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_CalibWithDvl_Feedback").arg(prefix), parent);
    combinedWithDVLFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_CombinedWithDvl_Feedback").arg(prefix), parent);
    combinedAutoFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_CombinedAuto_Feedback").arg(prefix), parent);
    getUtcDateFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadUtcDate_Feedback").arg(prefix), parent);
    getUtcTimeFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadUtcTime_Feedback").arg(prefix), parent);
    getYawFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadYaw_Feedback").arg(prefix), parent);
    getPitchFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadPitch_Feedback").arg(prefix), parent);
    getRollFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadRoll_Feedback").arg(prefix), parent);
    getBodyXFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadBodyX_Feedback").arg(prefix), parent);
    getBodyYFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadBodyY_Feedback").arg(prefix), parent);
    getBodyZFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadBodyZ_Feedback").arg(prefix), parent);
    getGeoXFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadGeoX_Feedback").arg(prefix), parent);
    getGeoYFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadGeoY_Feedback").arg(prefix), parent);
    getGeoZFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadGeoZ_Feedback").arg(prefix), parent);
    getLatitudeFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadLatitude_Feedback").arg(prefix), parent);
    getLongitudeFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadLongitude_Feedback").arg(prefix), parent);
    getGyroYawFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadGyroYaw_Feedback").arg(prefix), parent);
    getGyroPitchFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadGyroPitch_Feedback").arg(prefix), parent);
    getGyroRollFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadGyroRoll_Feedback").arg(prefix), parent);
    getAccelXFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadAccelX_Feedback").arg(prefix), parent);
    getAccelYFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadAccelY_Feedback").arg(prefix), parent);
    getAccelZFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadAccelZ_Feedback").arg(prefix), parent);
    getWorkStatusFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReadWorkStatus_Feedback").arg(prefix), parent);
    getCombinationStateFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_CombinationState_Feedback").arg(prefix), parent);
    getSelfCheckFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_SelfCheck_Feedback").arg(prefix), parent);
}

void INSHandler::initAllUIControls()
{
    for (auto it = m_deviceNumberMap.begin(); it != m_deviceNumberMap.end(); ++it) {
        INSUIControls controls;
        controls.number = it.value();
        controls.init(m_parent, controls.number);
        m_uiControls[it.key()] = controls;
    }
}

// 初始化UI（清空反馈框）
void INSHandler::initializeUI()
{
    for (auto& controls : m_uiControls) {
        // 清空所有反馈框
        if (controls.powerOnFeedback) controls.powerOnFeedback->clear();
        if (controls.powerOffFeedback) controls.powerOffFeedback->clear();
        if (controls.setGnssBindFeedback) controls.setGnssBindFeedback->clear();
        if (controls.calibrationWithDVLFeedback) controls.calibrationWithDVLFeedback->clear();
        if (controls.combinedWithDVLFeedback) controls.combinedWithDVLFeedback->clear();
        if (controls.combinedAutoFeedback) controls.combinedAutoFeedback->clear();
        if (controls.getUtcDateFeedback) controls.getUtcDateFeedback->clear();
        if (controls.getUtcTimeFeedback) controls.getUtcTimeFeedback->clear();
        if (controls.getYawFeedback) controls.getYawFeedback->clear();
        if (controls.getPitchFeedback) controls.getPitchFeedback->clear();
        if (controls.getRollFeedback) controls.getRollFeedback->clear();
        if (controls.getBodyXFeedback) controls.getBodyXFeedback->clear();
        if (controls.getBodyYFeedback) controls.getBodyYFeedback->clear();
        if (controls.getBodyZFeedback) controls.getBodyZFeedback->clear();
        if (controls.getGeoXFeedback) controls.getGeoXFeedback->clear();
        if (controls.getGeoYFeedback) controls.getGeoYFeedback->clear();
        if (controls.getGeoZFeedback) controls.getGeoZFeedback->clear();
        if (controls.getLatitudeFeedback) controls.getLatitudeFeedback->clear();
        if (controls.getLongitudeFeedback) controls.getLongitudeFeedback->clear();
        if (controls.getGyroYawFeedback) controls.getGyroYawFeedback->clear();
        if (controls.getGyroPitchFeedback) controls.getGyroPitchFeedback->clear();
        if (controls.getGyroRollFeedback) controls.getGyroRollFeedback->clear();
        if (controls.getAccelXFeedback) controls.getAccelXFeedback->clear();
        if (controls.getAccelYFeedback) controls.getAccelYFeedback->clear();
        if (controls.getAccelZFeedback) controls.getAccelZFeedback->clear();
        if (controls.getWorkStatusFeedback) controls.getWorkStatusFeedback->clear();
        if (controls.getCombinationStateFeedback) controls.getCombinationStateFeedback->clear();
        if (controls.getSelfCheckFeedback) controls.getSelfCheckFeedback->clear();
    }
}

// 连接信号（先断后连，避免重复）
void INSHandler::connectSignals()
{
    disconnectSignals();

    for (auto it = m_uiControls.begin(); it != m_uiControls.end(); ++it) {
        DeviceId deviceId = it.key();
        INSUIControls& controls = it.value();

        qDebug() << "[INS信号连接] DeviceId:" << static_cast<int>(deviceId)
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
        if (controls.setGnssBindBtn) {
            connect(controls.setGnssBindBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onSetGnssBindClicked(deviceId); });
        }
        if (controls.calibrationWithDVLBtn) {
            connect(controls.calibrationWithDVLBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onCalibrationWithDVLClicked(deviceId); });
        }
        if (controls.combinedWithDVLBtn) {
            connect(controls.combinedWithDVLBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onCombinedWithDVLClicked(deviceId); });
        }
        if (controls.combinedAutoBtn) {
            connect(controls.combinedAutoBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onCombinedAutoClicked(deviceId); });
        }
        if (controls.getUtcDateBtn) {
            connect(controls.getUtcDateBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetUtcDateClicked(deviceId); });
        }
        if (controls.getUtcTimeBtn) {
            connect(controls.getUtcTimeBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetUtcTimeClicked(deviceId); });
        }
        if (controls.getAttitudeBtn) {
            connect(controls.getAttitudeBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetAttitudeClicked(deviceId); });
        }
        if (controls.getBodySpeedBtn) {
            connect(controls.getBodySpeedBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetBodySpeedClicked(deviceId); });
        }
        if (controls.getGeoSpeedBtn) {
            connect(controls.getGeoSpeedBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetGeoSpeedClicked(deviceId); });
        }
        if (controls.getPositionBtn) {
            connect(controls.getPositionBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetPositionClicked(deviceId); });
        }
        if (controls.getGyroscopeSpeedBtn) {
            connect(controls.getGyroscopeSpeedBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetGyroscopeSpeedClicked(deviceId); });
        }
        if (controls.getAccelerationBtn) {
            connect(controls.getAccelerationBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetAccelerationClicked(deviceId); });
        }
        if (controls.getStatusBtn) {
            connect(controls.getStatusBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetStatusClicked(deviceId); });
        }
    }
}

// 断开所有信号
void INSHandler::disconnectSignals()
{
    for (auto& controls : m_uiControls) {
        if (controls.powerOnBtn) controls.powerOnBtn->disconnect();
        if (controls.powerOffBtn) controls.powerOffBtn->disconnect();
        if (controls.setGnssBindBtn) controls.setGnssBindBtn->disconnect();
        if (controls.calibrationWithDVLBtn) controls.calibrationWithDVLBtn->disconnect();
        if (controls.combinedWithDVLBtn) controls.combinedWithDVLBtn->disconnect();
        if (controls.combinedAutoBtn) controls.combinedAutoBtn->disconnect();
        if (controls.getUtcDateBtn) controls.getUtcDateBtn->disconnect();
        if (controls.getUtcTimeBtn) controls.getUtcTimeBtn->disconnect();
        if (controls.getAttitudeBtn) controls.getAttitudeBtn->disconnect();
        if (controls.getBodySpeedBtn) controls.getBodySpeedBtn->disconnect();
        if (controls.getGeoSpeedBtn) controls.getGeoSpeedBtn->disconnect();
        if (controls.getPositionBtn) controls.getPositionBtn->disconnect();
        if (controls.getGyroscopeSpeedBtn) controls.getGyroscopeSpeedBtn->disconnect();
        if (controls.getAccelerationBtn) controls.getAccelerationBtn->disconnect();
        if (controls.getStatusBtn) controls.getStatusBtn->disconnect();
    }
}

// 统一处理INS命令结果
void INSHandler::handleResult(const SelfCheckResult* result)
{
    if (!result) return;

    DeviceId deviceId = result->deviceId();
    if (!m_uiControls.contains(deviceId)) {
        qWarning() << "不支持的INS设备:" << static_cast<uint8_t>(deviceId);
        return;
    }

    processINSCommand(deviceId, result->commandCode(), result);
}

// 分发处理不同命令结果
void INSHandler::processINSCommand(DeviceId deviceId, CommandCode cmdCode, const SelfCheckResult* result)
{
    const INSUIControls& controls = m_uiControls[deviceId];
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
    case CommandCode::INS_SetGnssBind:
        setIcon(controls.setGnssBindIcon, icon);
        updateStatusWidget(controls.setGnssBindFeedback, status, feedbackDesc);
        break;
    case CommandCode::INS_CalibrationWithDVL:
        setIcon(controls.calibrationWithDVLIcon, icon);
        updateStatusWidget(controls.calibrationWithDVLFeedback, status, feedbackDesc);
        break;
    case CommandCode::INS_CombinedWithDVL:
        setIcon(controls.combinedWithDVLIcon, icon);
        updateStatusWidget(controls.combinedWithDVLFeedback, status, feedbackDesc);
        break;
    case CommandCode::INS_CombinedAuto:
        setIcon(controls.combinedAutoIcon, icon);
        updateStatusWidget(controls.combinedAutoFeedback, status, feedbackDesc);
        break;
    case CommandCode::INS_GetUtcDate:
        setIcon(controls.getUtcDateIcon, icon);
        updateStatusWidget(controls.getUtcDateFeedback, status, result->getParameter("date").toString());
        break;
    case CommandCode::INS_GetUtcTime:
        setIcon(controls.getUtcTimeIcon, icon);
        updateStatusWidget(controls.getUtcTimeFeedback, status, result->getParameter("time").toString());
        break;
    case CommandCode::INS_GetAttitude: {
        setIcon(controls.getAttitudeIcon, icon);
        double yaw = result->getParameter("yaw").toDouble();
        double pitch = result->getParameter("pitch").toDouble();
        double roll = result->getParameter("roll").toDouble();
        updateStatusWidget(controls.getYawFeedback, status, QString("%1°").arg(yaw, 0, 'f', 2));
        updateStatusWidget(controls.getPitchFeedback, status, QString("%1°").arg(pitch, 0, 'f', 2));
        updateStatusWidget(controls.getRollFeedback, status, QString("%1°").arg(roll, 0, 'f', 2));
        break;
    }
    case CommandCode::INS_GetBodySpeed: {
        setIcon(controls.getBodySpeedIcon, icon);
        double vx = result->getParameter("vx").toDouble();
        double vy = result->getParameter("vy").toDouble();
        double vz = result->getParameter("vz").toDouble();
        updateStatusWidget(controls.getBodyXFeedback, status, QString("%1m/s").arg(vx, 0, 'f', 2));
        updateStatusWidget(controls.getBodyYFeedback, status, QString("%1m/s").arg(vy, 0, 'f', 2));
        updateStatusWidget(controls.getBodyZFeedback, status, QString("%1m/s").arg(vz, 0, 'f', 2));
        break;
    }
    case CommandCode::INS_GetGeoSpeed: {
        setIcon(controls.getGeoSpeedIcon, icon);
        double ex = result->getParameter("ex").toDouble();
        double ey = result->getParameter("ey").toDouble();
        double ez = result->getParameter("ez").toDouble();
        updateStatusWidget(controls.getGeoXFeedback, status, QString("%1m/s").arg(ex, 0, 'f', 2));
        updateStatusWidget(controls.getGeoYFeedback, status, QString("%1m/s").arg(ey, 0, 'f', 2));
        updateStatusWidget(controls.getGeoZFeedback, status, QString("%1m/s").arg(ez, 0, 'f', 2));
        break;
    }
    case CommandCode::INS_GetPosition: {
        setIcon(controls.getPositionIcon, icon);
        QString lat = result->getParameter("lat").toString() + "°";
        QString lon = result->getParameter("lon").toString() + "°";
        updateStatusWidget(controls.getLatitudeFeedback, status, lat);
        updateStatusWidget(controls.getLongitudeFeedback, status, lon);
        break;
    }
    case CommandCode::INS_GetGyroscopeSpeed: {
        setIcon(controls.getGyroscopeSpeedIcon, icon);
        double yawGyro = result->getParameter("yaw_gyro").toDouble();
        double pitchGyro = result->getParameter("pitch_gyro").toDouble();
        double rollGyro = result->getParameter("roll_gyro").toDouble();
        updateStatusWidget(controls.getGyroYawFeedback, status, QString("%1°/s").arg(yawGyro, 0, 'f', 2));
        updateStatusWidget(controls.getGyroPitchFeedback, status, QString("%1°/s").arg(pitchGyro, 0, 'f', 4));
        updateStatusWidget(controls.getGyroRollFeedback, status, QString("%1°/s").arg(rollGyro, 0, 'f', 2));
        break;
    }
    case CommandCode::INS_GetAcceleration: {
        setIcon(controls.getAccelerationIcon, icon);
        double xAccel = result->getParameter("x_accel").toDouble();
        double yAccel = result->getParameter("y_accel").toDouble();
        double zAccel = result->getParameter("z_accel").toDouble();
        updateStatusWidget(controls.getAccelXFeedback, status, QString("%1m/s²").arg(xAccel, 0, 'f', 2));
        updateStatusWidget(controls.getAccelYFeedback, status, QString("%1m/s²").arg(yAccel, 0, 'f', 2));
        updateStatusWidget(controls.getAccelZFeedback, status, QString("%1m/s²").arg(zAccel, 0, 'f', 2));
        break;
    }
    case CommandCode::INS_GetStatus: {
        setIcon(controls.getStatusIcon, icon);
        QString workState = result->getParameter("ins_work_state").toString();
        QString comboState = result->getParameter("ins_combination_state").toString();
        QString checkState = result->getParameter("ins_selfcheck_state").toString();
        updateStatusWidget(controls.getWorkStatusFeedback, status, workState);
        updateStatusWidget(controls.getCombinationStateFeedback, status, comboState);
        updateStatusWidget(controls.getSelfCheckFeedback, status, checkState);
        break;
    }
    default:
        qDebug() << "未处理的INS命令:" << static_cast<int>(cmdCode);
        break;
    }
}

// 获取设备编号
QString INSHandler::getDeviceNumber(DeviceId deviceId) const
{
    return m_deviceNumberMap.value(deviceId, "");
}

// 支持的设备
bool INSHandler::supportsDevice(DeviceId deviceId) const
{
    return m_deviceNumberMap.contains(deviceId);
}

// 设置设备映射
void INSHandler::setDeviceMapping(const QMap<DeviceId, QString>& mapping)
{
    m_deviceNumberMap = mapping;
    initAllUIControls();
}

// -------------------- 槽函数实现 --------------------
void INSHandler::onPowerOnClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Common_PowerOn, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("INS上电命令失败: %1").arg(e.what()));
    }
}

void INSHandler::onPowerOffClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::Common_PowerOff, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("INS下电命令失败: %1").arg(e.what()));
    }
}

void INSHandler::onSetGnssBindClicked(DeviceId deviceId)
{
    if (!m_module) return;

    const INSUIControls& controls = m_uiControls[deviceId];
    if (!controls.setGnssBindLonEdit || !controls.setGnssBindLatEdit) {
        QMessageBox::critical(m_parent, "错误", "GNSS绑定参数输入框未找到");
        return;
    }

    // 解析经纬度参数
    double lon = controls.setGnssBindLonEdit->text().toDouble();
    double lat = controls.setGnssBindLatEdit->text().toDouble();

    // 手动创建参数（兼容原有逻辑）
    CommandParameter* rawParam = nullptr;
    try {
        rawParam = new InsGnssParam(lat, lon);
        std::shared_ptr<CommandParameter> params(rawParam);

        m_module->sendCheckCommand(deviceId, CommandCode::INS_SetGnssBind, params);
        QMessageBox::information(m_parent, "成功", "发送成功");
    }
    catch (...) {
        delete rawParam; // 异常时释放原始指针
        QMessageBox::critical(m_parent, "错误", "参数错误");
    }
}

void INSHandler::onCalibrationWithDVLClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::INS_CalibrationWithDVL, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("INS DVL校准命令失败: %1").arg(e.what()));
    }
}

void INSHandler::onCombinedWithDVLClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::INS_CombinedWithDVL, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("INS DVL组合命令失败: %1").arg(e.what()));
    }
}

void INSHandler::onCombinedAutoClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::INS_CombinedAuto, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("INS自动组合命令失败: %1").arg(e.what()));
    }
}

void INSHandler::onGetUtcDateClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::INS_GetUtcDate, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("获取INS UTC日期命令失败: %1").arg(e.what()));
    }
}

void INSHandler::onGetUtcTimeClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::INS_GetUtcTime, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("获取INS UTC时间命令失败: %1").arg(e.what()));
    }
}

void INSHandler::onGetAttitudeClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::INS_GetAttitude, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("获取INS姿态命令失败: %1").arg(e.what()));
    }
}

void INSHandler::onGetBodySpeedClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::INS_GetBodySpeed, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("获取INS载体速度命令失败: %1").arg(e.what()));
    }
}

void INSHandler::onGetGeoSpeedClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::INS_GetGeoSpeed, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("获取INS地理速度命令失败: %1").arg(e.what()));
    }
}

void INSHandler::onGetPositionClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::INS_GetPosition, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("获取INS位置命令失败: %1").arg(e.what()));
    }
}

void INSHandler::onGetGyroscopeSpeedClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::INS_GetGyroscopeSpeed, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("获取INS陀螺仪角速度命令失败: %1").arg(e.what()));
    }
}

void INSHandler::onGetAccelerationClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::INS_GetAcceleration, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("获取INS加速度命令失败: %1").arg(e.what()));
    }
}

void INSHandler::onGetStatusClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId, CommandCode::INS_GetStatus, params);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("获取INS状态命令失败: %1").arg(e.what()));
    }
}
