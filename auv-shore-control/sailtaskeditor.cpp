#include "sailtaskeditor.h"
#include<QComboBox>
#include<QDateTime>
#include<QMessageBox>
#include<cmath>
#include <algorithm> 
#pragma execution_character_set("utf-8")

// 常量定义（根据实际需求调整）
const int DEVICE_ID_LENGTH = 6;         // 设备号固定6位
const double MIN_DEPTH = 0.0;           // 最小深度（m）
const double MAX_DEPTH = 500.0;         // 最大深度（m）
const double MIN_HEIGHT = 0.5;          // 最小高度（m）
const double MAX_HEIGHT = 200.0;        // 最大高度（m）
const int MIN_ROTATE_SPEED = 0;         // 最小转速（rpm）
const int MAX_ROTATE_SPEED = 1500;      // 最大转速（rpm）
const double MS_TO_KN_RATIO = 1.94384;  // m/s转节（kn）的系数

SailTaskEditor::SailTaskEditor(QWidget *parent)
	: ITaskEditor(parent)
{
	ui.setupUi(this);
	initConnections(); // 初始化信号槽连接
	initUI();          // 初始化UI默认状态
}	

SailTaskEditor::~SailTaskEditor()
{

}

void SailTaskEditor::on_btn_saveSettings_clicked()
{
    if (!m_currentTask) {
        QMessageBox::warning(this, "提示", "无当前任务可更新");
        return;
    }

    // 同步UI修改到任务对象，并验证有效性
    if (syncTaskProperties()) {
        QMessageBox::information(this, "成功", "任务属性已更新");
        emit taskModified(); // 通知外部任务已修改
    }
}

// 初始化UI默认状态
void SailTaskEditor::initUI()
{
	// 设置窗口标题
	setWindowTitle("航行任务编辑器");
	// 设备号默认值
	ui.le_deviceID->setText("021001");
	// 任务序号默认当前日期时间（格式：yyMMdd）
	ui.dte_taskNum->setDateTime(QDateTime::currentDateTime());
	// 任务类型默认定深航行
	ui.cb_taskType->setCurrentIndex(0);
    // 下拉框索引 -> 任务调整操作
    m_adjustMap[0] = Task::Add;       // "增加" -> 0x01
    m_adjustMap[1] = Task::Delete;    // "删除" -> 0x03
    m_adjustMap[2] = Task::Replace;   // "修改" -> 0x02
	// 初始化标签文本
	updateDepthHeightLabel(NavigationMode::DepthKeeping);
    // 1. 设备号（6位数字，正则验证）
    QRegExp deviceIdReg("^\\d{6}$"); // 6位数字
    QRegExpValidator* deviceIdValidator = new QRegExpValidator(deviceIdReg, this);
    ui.le_deviceID->setValidator(deviceIdValidator);
    ui.le_deviceID->setPlaceholderText("6位数字");

    // 2. 经度（-180.0 ~ 180.0，保留6位小数）
    // 起点经度
    QDoubleValidator* startLonValidator = new QDoubleValidator(-180.0, 180.0, 6, this);
    startLonValidator->setNotation(QDoubleValidator::StandardNotation); // 禁用科学计数法
    ui.le_startLon->setValidator(startLonValidator);
    ui.le_startLon->setPlaceholderText("范围：-180.0 ~ 180.0（保留6位小数）");
    // 终点经度（同起点）
    QDoubleValidator* endLonValidator = new QDoubleValidator(-180.0, 180.0, 6, this);
    endLonValidator->setNotation(QDoubleValidator::StandardNotation);
    ui.le_endLon->setValidator(endLonValidator);
    ui.le_endLon->setPlaceholderText("范围：-180.0 ~ 180.0（保留6位小数）");

    // 3. 纬度（-90.0 ~ 90.0，保留6位小数）
    // 起点纬度
    QDoubleValidator* startLatValidator = new QDoubleValidator(-90.0, 90.0, 6, this);
    startLatValidator->setNotation(QDoubleValidator::StandardNotation);
    ui.le_startLat->setValidator(startLatValidator);
    ui.le_startLat->setPlaceholderText("范围：-90.0 ~ 90.0（保留6位小数）");
    // 终点纬度（同起点）
    QDoubleValidator* endLatValidator = new QDoubleValidator(-90.0, 90.0, 6, this);
    endLatValidator->setNotation(QDoubleValidator::StandardNotation);
    ui.le_endLat->setValidator(endLatValidator);
    ui.le_endLat->setPlaceholderText("范围：-90.0 ~ 90.0（保留6位小数）");

    // 4. 转速（0 ~ 1500 rpm，整数）
    QIntValidator* rotateSpeedValidator = new QIntValidator(0, 1500, this);
    ui.le_rotateSpeed->setValidator(rotateSpeedValidator);
    ui.le_rotateSpeed->setPlaceholderText("范围：0 ~ 1500（整数）");

    // 5. 速度（0 ~ 10 m/s，保留2位小数）
    QDoubleValidator* speedValidator = new QDoubleValidator(0.0, 10.0, 2, this);
    speedValidator->setNotation(QDoubleValidator::StandardNotation);
    ui.le_Speed->setValidator(speedValidator);
    ui.le_Speed->setPlaceholderText("范围：0.0 ~ 10.0（保留2位小数）");

    // 6. 超时时间（0 ~ 3600 秒，整数）
    QIntValidator* timeoutValidator = new QIntValidator(0, 3600, this);
    ui.le_timeOutPeriod->setValidator(timeoutValidator);
    ui.le_timeOutPeriod->setPlaceholderText("范围：0 ~ 3600（秒，整数）");

    // 7. 安全参数（非负数值）
    // 危险深度（0 ~ 600 m，保留1位小数）
    QDoubleValidator* dangerDepthValidator = new QDoubleValidator(0.0, 600.0, 1, this);
    dangerDepthValidator->setNotation(QDoubleValidator::StandardNotation);
    ui.le_dangerDepth->setValidator(dangerDepthValidator);
    // 危险高度（0 ~ 300 m，保留1位小数）
    QDoubleValidator* dangerHeightValidator = new QDoubleValidator(0.0, 300.0, 1, this);
    dangerHeightValidator->setNotation(QDoubleValidator::StandardNotation);
    ui.le_dangerHeight->setValidator(dangerHeightValidator);
    // 障碍物距离（0 ~ 100 m，保留1位小数）
    QDoubleValidator* obstacleValidator = new QDoubleValidator(0.0, 100.0, 1, this);
    obstacleValidator->setNotation(QDoubleValidator::StandardNotation);
    ui.le_obstacleDistance->setValidator(obstacleValidator);
    // 危险电压（0 ~ 30 V，保留1位小数）
    QDoubleValidator* voltageValidator = new QDoubleValidator(0.0, 30.0, 1, this);
    voltageValidator->setNotation(QDoubleValidator::StandardNotation);
    ui.le_dangerVoltage->setValidator(voltageValidator);
    // 危险电流（0 ~ 5 A，保留3位小数）
    QDoubleValidator* currentValidator = new QDoubleValidator(0.0, 5.0, 3, this);
    currentValidator->setNotation(QDoubleValidator::StandardNotation);
    ui.le_dangerCurrent->setValidator(currentValidator);
    ModuleStyleHelper::loadStyleSheet(STYLE_SAILTASKEDITOR,this);
}

// 连接UI信号与槽函数
void SailTaskEditor::initConnections()
{
    // 基础信息修改
    connect(ui.le_deviceID, &QLineEdit::textChanged, this, &SailTaskEditor::onUIChanged);
    connect(ui.cb_taskCrud,
        static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this,
        &SailTaskEditor::onUIChanged);
    connect(ui.dte_taskNum, &QDateTimeEdit::dateTimeChanged, this, &SailTaskEditor::onUIChanged);
    connect(ui.cb_taskType,
        static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this,
        [this](int index) {
            m_currentMode = index == 0 ? NavigationMode::DepthKeeping : NavigationMode::HeightKeeping;
            updateDepthHeightLabel(m_currentMode);
            onUIChanged();
        });

    // 起点参数修改
    connect(ui.le_startLon, &QLineEdit::textChanged, this, &SailTaskEditor::onUIChanged);
    connect(ui.le_startLat, &QLineEdit::textChanged, this, &SailTaskEditor::onUIChanged);
    connect(ui.le_startdepth, &QLineEdit::textChanged, this, &SailTaskEditor::onUIChanged);

    // 终点参数修改
    connect(ui.le_endLon, &QLineEdit::textChanged, this, &SailTaskEditor::onUIChanged);
    connect(ui.le_endLat, &QLineEdit::textChanged, this, &SailTaskEditor::onUIChanged);
    connect(ui.le_enddepth, &QLineEdit::textChanged, this, &SailTaskEditor::onUIChanged);

    // 运动参数修改
    connect(ui.le_rotateSpeed, &QLineEdit::textChanged, this, &SailTaskEditor::onUIChanged);
    connect(ui.le_Speed, &QLineEdit::textChanged, this, &SailTaskEditor::onUIChanged);
    connect(ui.le_timeOutPeriod, &QLineEdit::textChanged, this, &SailTaskEditor::onUIChanged);

    // 安全参数修改
    connect(ui.le_dangerDepth, &QLineEdit::textChanged, this, &SailTaskEditor::onUIChanged);
    connect(ui.le_dangerHeight, &QLineEdit::textChanged, this, &SailTaskEditor::onUIChanged);
    connect(ui.le_obstacleDistance, &QLineEdit::textChanged, this, &SailTaskEditor::onUIChanged);
    connect(ui.le_dangerVoltage, &QLineEdit::textChanged, this, &SailTaskEditor::onUIChanged);
    connect(ui.le_dangerCurrent, &QLineEdit::textChanged, this, &SailTaskEditor::onUIChanged);

    // 上浮设置修改
    connect(ui.cb_isFloat, &QCheckBox::stateChanged, this, &SailTaskEditor::onUIChanged);

    //确认属性同步
    //connect(ui.btn_saveSettings, &QPushButton::clicked, this, &SailTaskEditor::on_btn_saveSettings_clicked);
}

// 更新深度/高度标签文本
void SailTaskEditor::updateDepthHeightLabel(NavigationMode mode)
{
    const QString labelText = (mode == NavigationMode::DepthKeeping) ? "深度" : "高度";
    ui.lbl_startDepth->setText(labelText);
    ui.lbl_endDepth->setText(labelText);
}

bool SailTaskEditor::validateInput() const
{
    // 设备号校验（6位）
    if (ui.le_deviceID->text().trimmed().length() != DEVICE_ID_LENGTH) {
        QMessageBox::warning(const_cast<SailTaskEditor*>(this), "输入错误",
            QString("设备号必须为%1位").arg(DEVICE_ID_LENGTH));
        return false;
    }

    // 经纬度校验（非空且数值有效）
    bool lonValid, latValid;
    ui.le_startLon->text().toDouble(&lonValid);
    ui.le_startLat->text().toDouble(&latValid);
    if (!lonValid || !latValid) {
        QMessageBox::warning(const_cast<SailTaskEditor*>(this), "输入错误", "起点经纬度格式无效");
        return false;
    }

    // 速度校验（非负）
    if (ui.le_Speed->text().toDouble() < 0) {
        QMessageBox::warning(const_cast<SailTaskEditor*>(this), "输入错误", "速度不能为负数");
        return false;
    }
}

// 从UI同步属性到Task对象（核心方法）
bool SailTaskEditor::syncTaskProperties()
{
    // 1. 基础属性：设备号（6位校验）
    QString deviceId = ui.le_deviceID->text().trimmed();
    if (deviceId.length() != 6) {
        QMessageBox::warning(this, "输入错误", "设备号必须为6位");
        return false;
    }
    m_currentTask->setDeviceId(deviceId);

    // 2. 任务调整类型（增删改）
    int adjustIdx = ui.cb_taskCrud->currentIndex();
    m_currentTask->setTaskAdjust(m_adjustMap[adjustIdx]); // 基于之前的映射表

    // 3. 任务序号（日期时间转换）
    QString taskNum = ui.dte_taskNum->dateTime().toString("yyMMdd01");
    m_currentTask->setTaskNum(taskNum);

    // 4. 航行模式（定深/定高）
    NavigationMode navMode = ui.cb_taskType->currentIndex() == 0
        ? NavigationMode::DepthKeeping
        : NavigationMode::HeightKeeping;
    m_currentTask->setNavigationMode(navMode);

    // 5. 起点属性（经纬度+深度/高度）
    bool lonOk, latOk, depthOk;
    double startLon = ui.le_startLon->text().toDouble(&lonOk);
    double startLat = ui.le_startLat->text().toDouble(&latOk);
    double startDepth = ui.le_startdepth->text().toDouble(&depthOk);
    if (!lonOk || !latOk || !depthOk) {
        QMessageBox::warning(this, "输入错误", "起点参数格式无效（需为数字）");
        return false;
    }
    // 经纬度范围校验（示例：经度-180~180，纬度-90~90）
    if (startLon < -180 || startLon > 180 || startLat < -90 || startLat > 90) {
        QMessageBox::warning(this, "输入错误", "经纬度超出有效范围");
        return false;
    }
    QVariantMap startPoint;
    startPoint["longitude"] = startLon;
    startPoint["latitude"] = startLat;
    startPoint[navMode == NavigationMode::DepthKeeping ? "depth" : "height"] = startDepth;
    m_currentTask->setStartPoint(startPoint);

    // 6. 终点属性（同起点逻辑）
    double endLon = ui.le_endLon->text().toDouble(&lonOk);
    double endLat = ui.le_endLat->text().toDouble(&latOk);
    double endDepth = ui.le_enddepth->text().toDouble(&depthOk);
    if (!lonOk || !latOk || !depthOk) {
        QMessageBox::warning(this, "输入错误", "终点参数格式无效");
        return false;
    }
    QVariantMap endPoint;
    endPoint["longitude"] = endLon;
    endPoint["latitude"] = endLat;
    endPoint[navMode == NavigationMode::DepthKeeping ? "depth" : "height"] = endDepth;
    m_currentTask->setEndPoint(endPoint);

    // 7. 运动参数（转速、速度、超时时间）
    int rotateSpeed = ui.le_rotateSpeed->text().toInt();
    if (rotateSpeed < 0 || rotateSpeed > 1500) { // 假设转速范围0~1500rpm
        QMessageBox::warning(this, "输入错误", "转速超出有效范围（0~1500rpm）");
        return false;
    }
    m_currentTask->setRotateSpeed(rotateSpeed);

    double speed = ui.le_Speed->text().toDouble();
    if (speed < 0) {
        QMessageBox::warning(this, "输入错误", "速度不能为负数");
        return false;
    }
    m_currentTask->setSpeed(speed);

    int timeout = ui.le_timeOutPeriod->text().toInt();
    if (timeout < 0) {
        QMessageBox::warning(this, "输入错误", "超时时间不能为负数");
        return false;
    }
    m_currentTask->setTimeoutPeriod(timeout);

    // 8. 安全参数（危险深度、高度等）
    m_currentTask->setDangerDepth(ui.le_dangerDepth->text().toInt());
    m_currentTask->setDangerHeight(ui.le_dangerHeight->text().toInt());
    m_currentTask->setObstacleDistance(ui.le_obstacleDistance->text().toInt());
    m_currentTask->setDangerVoltage(ui.le_dangerVoltage->text().toInt());
    m_currentTask->setDangerCurrent(ui.le_dangerCurrent->text().toFloat());

    // 9. 上浮设置
    m_currentTask->setNeedFloat(ui.cb_isFloat->isChecked());

    // 所有属性验证通过，同步完成
    return true;
}

void SailTaskEditor::blockUISignals(bool block)
{
    // 基础信息控件
    ui.le_deviceID->blockSignals(block);
    ui.cb_taskCrud->blockSignals(block);
    ui.dte_taskNum->blockSignals(block);
    ui.cb_taskType->blockSignals(block);

    // 起点参数控件
    ui.le_startLon->blockSignals(block);
    ui.le_startLat->blockSignals(block);
    ui.le_startdepth->blockSignals(block);

    // 终点参数控件
    ui.le_endLon->blockSignals(block);
    ui.le_endLat->blockSignals(block);
    ui.le_enddepth->blockSignals(block);

    // 运动参数控件
    ui.le_rotateSpeed->blockSignals(block);
    ui.le_Speed->blockSignals(block);
    ui.le_timeOutPeriod->blockSignals(block);

    // 安全参数控件
    ui.le_dangerDepth->blockSignals(block);
    ui.le_dangerHeight->blockSignals(block);
    ui.le_obstacleDistance->blockSignals(block);
    ui.le_dangerVoltage->blockSignals(block);
    ui.le_dangerCurrent->blockSignals(block);

    // 上浮设置控件
    ui.cb_isFloat->blockSignals(block);
}


void SailTaskEditor::setTask(Task * task)
{
    if (!task || task->getTaskType() != TaskType::SAIL) {
        clear();
        return;
    }
    // 临时阻塞所有相关控件的信号，避免设置值时触发onUIChanged
    blockUISignals(true);

    m_currentTask = task;

    // 1. 基础信息
    ui.le_deviceID->setText(task->getDeviceId());
    // 加载任务调整操作到下拉框
    Task::TaskAdjustOp op = task->getTaskAdjust();
    // 根据操作值反查下拉框索引
    for (auto it = m_adjustMap.begin(); it != m_adjustMap.end(); ++it) {
        if (it.value() == op) {
            ui.cb_taskCrud->setCurrentIndex(it.key());
            break;
        }
    }
    // 任务序号（字符串转日期时间）
    const QString taskNum = task->getTaskNum();
    if (taskNum.length() == 12) { // 假设格式：yyMMddHHmmss
        QDateTime dt = QDateTime::fromString(taskNum, "yyMMddHHmmss");
        if (dt.isValid()) ui.dte_taskNum->setDateTime(dt);
    }

    // 2. 航行模式与任务类型
    m_currentMode = (task->getNavigationMode() == NavigationMode::DepthKeeping)
        ? NavigationMode::DepthKeeping
        : NavigationMode::HeightKeeping;
    ui.cb_taskType->setCurrentIndex(m_currentMode == NavigationMode::DepthKeeping ? 0 : 1);
    updateDepthHeightLabel(m_currentMode);

    // 3. 起点参数
    const auto& startPoint = task->getStartPoint();
    ui.le_startLon->setText(QString::number(startPoint["longitude"].toDouble(), 'f', 5));
    ui.le_startLat->setText(QString::number(startPoint["latitude"].toDouble(), 'f', 5));
    ui.le_startdepth->setText(QString::number(
        m_currentMode == NavigationMode::DepthKeeping
        ? startPoint["depth"].toDouble()
        : startPoint["height"].toDouble(), 'f', 2
    ));

    // 4. 终点参数
    const auto& endPoint = task->getEndPoint();
    ui.le_endLon->setText(QString::number(endPoint["longitude"].toDouble(), 'f', 5));
    ui.le_endLat->setText(QString::number(endPoint["latitude"].toDouble(), 'f', 5));
    ui.le_enddepth->setText(QString::number(
        m_currentMode == NavigationMode::DepthKeeping
        ? endPoint["depth"].toDouble()
        : endPoint["height"].toDouble(), 'f', 2
    ));

    // 5. 运动参数
    ui.le_rotateSpeed->setText(QString::number(task->getRotateSpeed()));
    // 任务中存储的是kn，UI显示m/s（转换：1kn ≈ 0.514444m/s）
    const double speedMs = task->getSpeed() * 0.514444;
    ui.le_Speed->setText(QString::number(speedMs, 'f', 2));
    ui.le_timeOutPeriod->setText(QString::number(task->getTimeOutPeriod()));

    // 6. 安全参数
    ui.le_dangerDepth->setText(QString::number(task->getDangerDepth()));
    ui.le_dangerHeight->setText(QString::number(task->getDangerHeight()));
    ui.le_obstacleDistance->setText(QString::number(task->getObstacleDistance()));
    ui.le_dangerVoltage->setText(QString::number(task->getDangerVoltage()));
    ui.le_dangerCurrent->setText(QString::number(task->getDangerCurrent() , 'f', 3));

    // 7. 上浮设置
    ui.cb_isFloat->setChecked(task->getNeedFloat());
    // 恢复控件信号发射
    blockUISignals(false);
}

Task* SailTaskEditor::getTask() const
{
    if (!m_currentTask || !validateInput()) return nullptr;

    // 1. 基础信息
    m_currentTask->setDeviceId(ui.le_deviceID->text().trimmed());
    // 更新任务调整操作（根据下拉框索引映射）
    int currentIdx = ui.cb_taskCrud->currentIndex();
    if (m_adjustMap.contains(currentIdx)) {
        m_currentTask->setTaskAdjust(m_adjustMap[currentIdx]);
    }
    // 任务序号（日期时间转字符串：yyMMddHHmmss）
    const QString taskNum = ui.dte_taskNum->dateTime().toString("yyMMddHHmmss");
    m_currentTask->setTaskNum(taskNum);

    // 2. 航行模式
    m_currentTask->setNavigationMode(
        m_currentMode == NavigationMode::DepthKeeping ? NavigationMode::DepthKeeping : NavigationMode::HeightKeeping
    );

    // 3. 起点参数（经纬度保留5位小数）
    QVariantMap startPoint;
    startPoint["longitude"] = std::round(ui.le_startLon->text().toDouble() * 100000) / 100000;
    startPoint["latitude"] = std::round(ui.le_startLat->text().toDouble() * 100000) / 100000;
    const double startDepthHeight = ui.le_startdepth->text().toDouble();
    if (m_currentMode == NavigationMode::DepthKeeping) {
        startPoint["depth"] = std::clamp(startDepthHeight, MIN_DEPTH, MAX_DEPTH);
    }
    else {
        startPoint["height"] = std::clamp(startDepthHeight, MIN_HEIGHT, MAX_HEIGHT);
    }
    m_currentTask->setStartPoint(startPoint);

    // 4. 终点参数
    QVariantMap endPoint;
    endPoint["longitude"] = std::round(ui.le_endLon->text().toDouble() * 100000) / 100000;
    endPoint["latitude"] = std::round(ui.le_endLat->text().toDouble() * 100000) / 100000;
    const double endDepthHeight = ui.le_enddepth->text().toDouble();
    if (m_currentMode == NavigationMode::DepthKeeping) {
        endPoint["depth"] = std::clamp(endDepthHeight, MIN_DEPTH, MAX_DEPTH);
    }
    else {
        endPoint["height"] = std::clamp(endDepthHeight, MIN_HEIGHT, MAX_HEIGHT);
    }
    m_currentTask->setEndPoint(endPoint);

    // 5. 运动参数（UI显示m/s转任务存储的kn）
    m_currentTask->setRotateSpeed(
        std::clamp(ui.le_rotateSpeed->text().toInt(), MIN_ROTATE_SPEED, MAX_ROTATE_SPEED)
    );
    const double speedKn = ui.le_Speed->text().toDouble() / 0.514444; // m/s转kn
    m_currentTask->setSpeed(std::round(speedKn * 10) / 10); // 保留1位小数
    m_currentTask->setTimeoutPeriod(std::max(0, ui.le_timeOutPeriod->text().toInt()));

    // 6. 安全参数
    m_currentTask->setDangerDepth(ui.le_dangerDepth->text().toDouble());
    m_currentTask->setDangerHeight(ui.le_dangerHeight->text().toDouble());
    m_currentTask->setObstacleDistance(ui.le_obstacleDistance->text().toDouble());
    m_currentTask->setDangerVoltage(ui.le_dangerVoltage->text().toDouble());
    m_currentTask->setDangerCurrent(
        std::round(ui.le_dangerCurrent->text().toDouble()) 
    );

    // 7. 上浮设置
    m_currentTask->setNeedFloat(ui.cb_isFloat->isChecked());

    return m_currentTask;
}

void SailTaskEditor::clear()
{
    m_currentTask = nullptr;
    ui.le_deviceID->clear();
    ui.cb_taskCrud->setCurrentIndex(0);
    ui.dte_taskNum->setDateTime(QDateTime::currentDateTime());
    ui.cb_taskType->setCurrentIndex(0);
    m_currentMode = NavigationMode::DepthKeeping;
    updateDepthHeightLabel(m_currentMode);

    ui.le_startLon->clear();
    ui.le_startLat->clear();
    ui.le_startdepth->clear();
    ui.le_endLon->clear();
    ui.le_endLat->clear();
    ui.le_enddepth->clear();

    ui.le_rotateSpeed->clear();
    ui.le_Speed->clear();
    ui.le_timeOutPeriod->clear();

    ui.le_dangerDepth->clear();
    ui.le_dangerHeight->clear();
    ui.le_obstacleDistance->clear();
    ui.le_dangerVoltage->clear();
    ui.le_dangerCurrent->clear();

    ui.cb_isFloat->setChecked(false);
}

// UI修改时更新任务并发射信号
void SailTaskEditor::onUIChanged()
{
    if (m_currentTask) {
        getTask(); // 同步UI数据到任务对象
        emit taskModified();
    }
}