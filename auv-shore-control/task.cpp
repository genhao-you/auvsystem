#include "task.h"
#include<qDebug>
#include<QDateTime>
#include<QJsonArray>
Task::Task(const QString& planId, TaskType type, NavigationMode navMode):
      m_sPlanId(planId)
    , m_type(type)
    , m_sDeviceId("021001") // 默认设备号
    , m_sTaskNum(QDateTime::currentDateTime().toString("yyMMdd01")) // 默认任务序号（yyMMdd01）
    , m_navMode(navMode)
    , m_iRotateSpeed(0)
    , m_iSpeed(0)
    , m_iTimeOutPeriod(0)
    , m_iDangerDepth(0)
    , m_iDangerHeight(0)
    , m_iObstacleDistance(0)
    , m_iDangerVoltage(0)
    , m_fDangerCurrent(0)
    , m_bNeedFloat(false)
{
    m_sId = MissionNamingUtil::generateTaskId(planId);
    // 生成任务名称（航行任务拼接模式，如"航行任务-定深航行"）
    if (type == TaskType::SAIL) {
        m_sName = QString("%1-%2").arg(taskTypeToString(type)).arg(navigationModeToString(navMode));
    }
    else {
        m_sName = MissionNamingUtil::generateTaskName(planId, type);
    }

    // 起点默认值（示例：经纬度、深度）
    m_startPoint["longitude"] = 120.00000;
    m_startPoint["latitude"] = 30.00000;
    m_startPoint["depth"] = 10.0; // 默认深度10米

    // 终点默认值
    m_endPoint["longitude"] = 120.00100;
    m_endPoint["latitude"] = 30.00100;
    m_endPoint["depth"] = 10.0;
}
// 添加拷贝构造函数（深拷贝）
Task::Task(const Task& other)
    : m_sPlanId(other.m_sPlanId)
    , m_sId(other.m_sId)
    , m_sName(other.m_sName)
    , m_type(other.m_type)
    , m_params(other.m_params) // QMap会自动深拷贝
    , m_sDeviceId(other.m_sDeviceId)
    , m_sTaskNum(other.m_sTaskNum)
    , m_startPoint(other.m_startPoint)
    , m_endPoint(other.m_endPoint)
    , m_iRotateSpeed(other.m_iRotateSpeed)
    , m_iSpeed(other.m_iSpeed)
    , m_iTimeOutPeriod(other.m_iTimeOutPeriod)
    , m_iDangerDepth(other.m_iDangerDepth)
    , m_iDangerHeight(other.m_iDangerHeight)
    , m_iObstacleDistance(other.m_iObstacleDistance)
    , m_iDangerVoltage(other.m_iDangerVoltage)
    , m_fDangerCurrent(other.m_fDangerCurrent)
    , m_bNeedFloat(other.m_bNeedFloat)
    , m_navMode(other.m_navMode)
{
    // QMap的拷贝是深拷贝，不需要额外处理
}

// 添加赋值运算符（深拷贝）
Task& Task::operator=(const Task& other) {
    if (this != &other) {
        m_sPlanId = other.m_sPlanId;
        m_sId = other.m_sId;
        m_sName = other.m_sName;
        m_type = other.m_type;
        m_params = other.m_params; // QMap自动深拷贝
        m_sDeviceId = other.m_sDeviceId;
        m_sTaskNum = other.m_sTaskNum;
        m_startPoint = other.m_startPoint;
        m_endPoint = other.m_endPoint;
        m_iRotateSpeed = other.m_iRotateSpeed;
        m_iSpeed = other.m_iSpeed;
        m_iTimeOutPeriod = other.m_iTimeOutPeriod;
        m_iDangerDepth = other.m_iDangerDepth;
        m_iDangerHeight = other.m_iDangerHeight;
        m_iObstacleDistance = other.m_iObstacleDistance;
        m_iDangerVoltage = other.m_iDangerVoltage;
        m_fDangerCurrent = other.m_fDangerCurrent;
        m_bNeedFloat = other.m_bNeedFloat;
        m_navMode = other.m_navMode;
    }
    return *this;
}

Task::~Task()
{}

void Task::setTaskAdjust(TaskAdjustOp op)
{
    m_taskAdjust = op;
}

Task::TaskAdjustOp Task::getTaskAdjust() const
{
    return m_taskAdjust;
}

QString Task::getId() const
{
    return m_sId;
}

void Task::setId(QString id)
{
    m_sId = id;
}

QString Task::getPlanId() const
{
    return m_sPlanId;
}

void Task::setPlanId(QString id)
{
    m_sPlanId = id;
}
TaskType Task::getTaskType()const
{
    return m_type;
}

void Task::setTaskType(TaskType type)
{
    m_type = type;
    // 航行任务自动初始化模式名称，非航行任务清空模式关联
    if (type == TaskType::SAIL) {
        m_sName = QString("%1-%2").arg(taskTypeToString(type)).arg(navigationModeToString(m_navMode));
    }
    else {
        m_sName = MissionNamingUtil::generateTaskName(m_sPlanId, type);
    }
}

QMap<QString, QVariant> Task::getParams()
{
    return m_params;
}

void Task::setParam(const QString& key, const QVariant& value)
{
    m_params[key] = value;
}

QVariant Task::getParam(const QString& key)
{
    return m_params.value(key);
}


QString Task::getTaskName()const
{
    return m_sName;
}

void Task::setTaskName(QString name)
{
    m_sName = name;
}

NavigationMode Task::getNavigationMode() const
{ 
    return m_navMode;
}

void Task::setNavigationMode(NavigationMode mode)
{
    // 仅航行任务允许设置航行模式
    if (m_type != TaskType::SAIL) {
        qWarning() << "非航行任务无法设置航行模式：" << taskTypeToString(m_type);
        return;
    }
    m_navMode = mode;
    // 更新任务名称（拼接模式）
    m_sName = QString("%1-%2").arg(taskTypeToString(m_type)).arg(navigationModeToString(mode));
    // 初始化对应模式的默认参数
    if (mode == NavigationMode::DepthKeeping) {
        if (!m_startPoint.contains("depth")) m_startPoint["depth"] = 10.0;
    }
    else {
        if (!m_startPoint.contains("height")) m_startPoint["height"] = 5.0;
    }
}

// ---------------------- 任务管理策略表格字段实现 ----------------------
// 设备号（6字节校验）
QString Task::getDeviceId() const
{ 
    return m_sDeviceId; 
}

void Task::setDeviceId(const QString& deviceId) {
    if (deviceId.length() == 6 && deviceId.contains(QRegExp("^[A-Za-z0-9]{6}$"))) {
        m_sDeviceId = deviceId;
    }
    else {
        qWarning() << "设备号格式错误（6字节数字/字母）：" << deviceId;
    }
}

// 任务序号（8位数字校验）
QString Task::getTaskNum() const 
{ 
    return m_sTaskNum;
}

void Task::setTaskNum(const QString& taskNum)
{
    if (taskNum.length() == 8 && taskNum.contains(QRegExp("^\\d{8}$"))) {
        m_sTaskNum = taskNum;
    }
    else {
        qWarning() << "任务序号格式错误（8位数字）：" << taskNum;
    }
}

// 起点（经纬度5位小数，深度/高度贴合模式）
QVariantMap Task::getStartPoint() const 
{
    return m_startPoint;
}

void Task::setStartPoint(const QVariantMap& startPoint)
{
    bool valid = true;
    double lon = startPoint["longitude"].toDouble(&valid);
    double lat = startPoint["latitude"].toDouble(&valid);
    if (!valid)
    {
        qWarning() << "起点经纬度格式错误";
        return;
    }

    // 经纬度保留5位小数
    lon = round(lon * 100000) / 100000;
    lat = round(lat * 100000) / 100000;

    m_startPoint["longitude"] = lon;
    m_startPoint["latitude"] = lat;

    // 深度/高度校验（贴合当前航行模式）
    if (m_type == TaskType::SAIL)
    {
        if (m_navMode == NavigationMode::DepthKeeping)
        {
            double depth = startPoint["depth"].toDouble(&valid);
            if (valid) m_startPoint["depth"] = qBound(0.0, depth, 500.0); // 定深0~500米
        }
        else 
        {
            double height = startPoint["height"].toDouble(&valid);
            if (valid) m_startPoint["height"] = qBound(0.5, height, 200.0); // 定高0.5~200米
        }
    }
}

// 终点（同起点逻辑）
QVariantMap Task::getEndPoint() const
{ 
    return m_endPoint; 
}

void Task::setEndPoint(const QVariantMap& endPoint)
{
    bool valid = true;
    double lon = endPoint["longitude"].toDouble(&valid);
    double lat = endPoint["latitude"].toDouble(&valid);
    if (!valid) {
        qWarning() << "终点经纬度格式错误";
        return;
    }

    lon = round(lon * 100000) / 100000;
    lat = round(lat * 100000) / 100000;

    m_endPoint["longitude"] = lon;
    m_endPoint["latitude"] = lat;

    if (m_type == TaskType::SAIL)
    {
        if (m_navMode == NavigationMode::DepthKeeping)
        {
            double depth = endPoint["depth"].toDouble(&valid);
            if (valid) m_endPoint["depth"] = qBound(0.0, depth, 500.0);
        }
        else 
        {
            double height = endPoint["height"].toDouble(&valid);
            if (valid) m_endPoint["height"] = qBound(0.5, height, 200.0);
        }
    }
}

// 转速（0~1500rpm）
int16_t Task::getRotateSpeed() const
{ 
    return m_iRotateSpeed;
}

void Task::setRotateSpeed(int16_t rotateSpeed)
{
    m_iRotateSpeed = qBound<int16_t>(0, rotateSpeed, 1500);
}

// 速度（实际值×10取整）
int16_t Task::getSpeed() const
{ 
    return m_iSpeed;
}

void Task::setSpeed(double speedKn)
{
    int16_t storedValue = static_cast<int16_t>(floor(speedKn * 10));
    m_iSpeed = qBound<int16_t>(0, storedValue, 32767);
}

// 超时时间（≥0秒）
int32_t Task::getTimeOutPeriod() const
{ 
    return m_iTimeOutPeriod;
}

void Task::setTimeoutPeriod(int32_t timeOut)
{
    m_iTimeOutPeriod = qMax<int32_t>(0, timeOut);
}

// 危险深度
int16_t Task::getDangerDepth() const 
{ 
    return m_iDangerDepth;
}

void Task::setDangerDepth(int16_t dangerDepth)
{
    m_iDangerDepth = qBound<int16_t>(0, dangerDepth, 32767);
}

// 危险高度
int16_t Task::getDangerHeight() const
{ 
    return m_iDangerHeight;
}

void Task::setDangerHeight(int16_t dangerHeight)
{
    m_iDangerHeight = qBound<int16_t>(0, dangerHeight, 32767);
}

// 障碍物距离
int16_t Task::getObstacleDistance() const
{ 
    return m_iObstacleDistance;
}

void Task::setObstacleDistance(int16_t obstacleDistance)
{
    m_iObstacleDistance = qBound<int16_t>(0, obstacleDistance, 32767);
}

// 危险电压
int16_t Task::getDangerVoltage() const
{ 
    return m_iDangerVoltage;
}

void Task::setDangerVoltage(int16_t dangerVoltage)
{
    m_iDangerVoltage = qBound<int16_t>(0, dangerVoltage, 32767);
}

// 危险电流（实际值×1000取整）
float Task::getDangerCurrent() const
{ 
    return m_fDangerCurrent;
}

void Task::setDangerCurrent(float currentA)
{
    m_fDangerCurrent = currentA;
}

// 是否上浮
bool Task::getNeedFloat() const
{ 
    return m_bNeedFloat;
}

void Task::setNeedFloat(bool needFloat)
{ 
    m_bNeedFloat = needFloat;
}

// ---------------------- JSON序列化（含航行模式联动）----------------------
QJsonObject Task::toJson() const
{
    // 1. 定义有序字段列表（严格按目标JSON顺序排列，核心：保留插入顺序）
    QList<QPair<QString, QJsonValue>> orderedFields;

    // 基础字段（表格必填）—— 按目标顺序添加
    orderedFields.append(qMakePair(QString("device_id"), QJsonValue(m_sDeviceId)));
    orderedFields.append(qMakePair(QString("task_id"), QJsonValue(m_sTaskNum)));
    orderedFields.append(qMakePair(QString("task_type"), QJsonValue(static_cast<int>(m_type))));

    // 2. 起点坐标（数组：[经度, 纬度, 深度/高度]，保留5位小数）—— 原有逻辑完全复用
    QJsonArray startPosArray;
    double startLon = m_startPoint["longitude"].toDouble();
    double startLat = m_startPoint["latitude"].toDouble();
    startPosArray.append(QString::number(startLon, 'f', 5).toDouble()); // 经度
    startPosArray.append(QString::number(startLat, 'f', 5).toDouble()); // 纬度
    // 深度/高度（根据航行模式取对应值，保留2位小数）
    if (m_type == TaskType::SAIL) {
        if (m_navMode == NavigationMode::DepthKeeping) {
            double depth = m_startPoint["depth"].toDouble();
            startPosArray.append(QString::number(depth, 'f', 2).toDouble());
        }
        else {
            double height = m_startPoint["height"].toDouble();
            startPosArray.append(QString::number(height, 'f', 2).toDouble());
        }
    }
    // 将起点坐标添加到有序字段列表
    orderedFields.append(qMakePair(QString("start_pos"), QJsonValue(startPosArray)));

    // 3. 终点坐标（同起点逻辑）—— 原有逻辑完全复用
    QJsonArray endPosArray;
    double endLon = m_endPoint["longitude"].toDouble();
    double endLat = m_endPoint["latitude"].toDouble();
    endPosArray.append(QString::number(endLon, 'f', 5).toDouble());
    endPosArray.append(QString::number(endLat, 'f', 5).toDouble());
    if (m_type == TaskType::SAIL) {
        if (m_navMode == NavigationMode::DepthKeeping) {
            double depth = m_endPoint["depth"].toDouble();
            endPosArray.append(QString::number(depth, 'f', 2).toDouble());
        }
        else {
            double height = m_endPoint["height"].toDouble();
            endPosArray.append(QString::number(height, 'f', 2).toDouble());
        }
    }
    // 将终点坐标添加到有序字段列表
    orderedFields.append(qMakePair(QString("end_pos"), QJsonValue(endPosArray)));

    // 4. 运动参数（键名匹配）—— 原有逻辑完全复用
    orderedFields.append(qMakePair(QString("rotor_speed"), QJsonValue(static_cast<int>(m_iRotateSpeed))));
    orderedFields.append(qMakePair(QString("timeout"), QJsonValue(static_cast<int>(m_iTimeOutPeriod))));

    // 5. 安全参数（键名匹配，单位转换）—— 原有逻辑完全复用
    orderedFields.append(qMakePair(QString("max_depth"), QJsonValue(static_cast<int>(m_iDangerDepth))));
    orderedFields.append(qMakePair(QString("min_height"), QJsonValue(static_cast<int>(m_iDangerHeight))));
    orderedFields.append(qMakePair(QString("obstacle_dist"), QJsonValue(static_cast<int>(m_iObstacleDistance))));
    orderedFields.append(qMakePair(QString("min_voltage"), QJsonValue(static_cast<int>(m_iDangerVoltage))));
    // 电流：
    double maxCurrent = QString::number(m_fDangerCurrent , 'f', 3).toDouble();
    orderedFields.append(qMakePair(QString("max_current"), QJsonValue(maxCurrent)));

    // 6. 上浮设置（键名匹配）—— 原有逻辑完全复用
    orderedFields.append(qMakePair(QString("is_floating"), QJsonValue(m_bNeedFloat)));

    // 2. 遍历有序字段列表，构建QJsonObject（插入顺序与列表一致）
    QJsonObject json;
    for (const QPair<QString, QJsonValue>& field : orderedFields) {
        json.insert(field.first, field.second);
    }

    return json;
}

// JSON反序列化（解析航行模式联动）
Task Task::fromJson(const QJsonObject& json)
{
    // 解析任务类型和航行模式
    TaskType taskType = static_cast<TaskType>(json["taskType"].toInt());
    NavigationMode navMode = NavigationMode::DepthKeeping; // 默认定深
    if (taskType == TaskType::SAIL && json.contains("navigationMode")) {
        navMode = static_cast<NavigationMode>(json["navigationMode"].toInt());
    }

    // 构造任务对象
    Task task(json["planId"].toString(), taskType, navMode);
    task.setDeviceId(json["deviceId"].toString());
    task.setTaskNum(json["taskNum"].toString());

    // 解析起点
    QVariantMap startPoint;
    QJsonObject startJson = json["startPoint"].toObject();
    startPoint["longitude"] = startJson["longitude"].toDouble();
    startPoint["latitude"] = startJson["latitude"].toDouble();
    if (taskType == TaskType::SAIL) {
        if (navMode == NavigationMode::DepthKeeping) {
            startPoint["depth"] = startJson["depth"].toDouble();
        }
        else {
            startPoint["height"] = startJson["height"].toDouble();
        }
    }
    task.setStartPoint(startPoint);

    // 解析终点
    QVariantMap endPoint;
    QJsonObject endJson = json["endPoint"].toObject();
    endPoint["longitude"] = endJson["longitude"].toDouble();
    endPoint["latitude"] = endJson["latitude"].toDouble();
    if (taskType == TaskType::SAIL) {
        if (navMode == NavigationMode::DepthKeeping) {
            endPoint["depth"] = endJson["depth"].toDouble();
        }
        else {
            endPoint["height"] = endJson["height"].toDouble();
        }
    }
    task.setEndPoint(endPoint);

    // 解析运动参数
    task.setRotateSpeed(static_cast<int16_t>(json["rotateSpeed"].toInt()));
    task.setSpeed(json["speed"].toInt() / 10.0); // 还原实际速度（kn）
    task.setTimeoutPeriod(static_cast<int32_t>(json["timeOutPeriod"].toInt()));

    // 解析安全参数
    task.setDangerDepth(static_cast<int16_t>(json["dangerDepth"].toInt()));
    task.setDangerHeight(static_cast<int16_t>(json["dangerHeight"].toInt()));
    task.setObstacleDistance(static_cast<int16_t>(json["obstacleDistance"].toInt()));
    task.setDangerVoltage(static_cast<int16_t>(json["dangerVoltage"].toInt()));
    task.setDangerCurrent(json["dangerCurrent"].toInt() / 1000.0); // 还原实际电流（A）

    // 解析其他参数
    task.setNeedFloat(json["needFloat"].toBool());

    return task;
}