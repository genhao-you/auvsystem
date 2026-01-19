#include "plan.h"
#include"tasksequenceserializer.h"
#include<QJsonArray>
#include<qDebug>
Plan::Plan()	
{
    generateTasks();
}

Plan::Plan(const QString& waypointId, PlanMode mode)
    :m_sMissionId(waypointId)
    ,m_planMode(mode)
   
{
    m_sPlanId = MissionNamingUtil::generatePlanId(waypointId);
    m_sPlanName = MissionNamingUtil::generatePlanName(waypointId, mode);
    generateTasks();
}
// 添加拷贝构造函数（深拷贝）
Plan::Plan(const Plan& other)
    : m_sPlanId(other.m_sPlanId),
    m_sMissionId(other.m_sMissionId),
    m_sPlanName(other.m_sPlanName),
    m_planMode(other.m_planMode),
    m_dDistance(other.m_dDistance),
    m_dEstimatedTime(other.m_dEstimatedTime),
    m_dMinDepth(other.m_dMinDepth),
    m_dMaxDepth(other.m_dMaxDepth),
    m_iTaskCount(other.m_iTaskCount)
{
    // 深拷贝任务列表
    for (const Task& task : other.m_listTasks) {
        m_listTasks.append(task);
    }
}

// 添加赋值运算符（深拷贝）
Plan& Plan::operator=(const Plan& other) {
    if (this != &other) {
        m_sPlanId = other.m_sPlanId;
        m_sMissionId = other.m_sMissionId;
        m_sPlanName = other.m_sPlanName;
        m_planMode = other.m_planMode;
        m_dDistance = other.m_dDistance;
        m_dEstimatedTime = other.m_dEstimatedTime;
        m_dMinDepth = other.m_dMinDepth;
        m_dMaxDepth = other.m_dMaxDepth;
        m_iTaskCount = other.m_iTaskCount;

        // 清空并深拷贝任务列表
        m_listTasks.clear();
        for (const Task& task : other.m_listTasks) {
            m_listTasks.append(task);
        }
    }
    return *this;
}
Plan::~Plan()
{
}

QString Plan::getId() const
{
    return m_sPlanId;
}

void Plan::setId(QString id)
{
    m_sPlanId = id;
}

QString Plan::getMissionId() const
{
    return m_sMissionId;
}

void Plan::setMissionId(QString id)
{
    m_sMissionId = id;
}

QString Plan::getPlanName()const
{
	return m_sPlanName;
}

void Plan::setPlanName(QString name)
{
	m_sPlanName = name;
}

PlanMode Plan::getPlanMode()const
{
	return m_planMode;
}

/**
 * @brief 
 * @param mode 
*/
void Plan::setPlanMode(PlanMode mode)
{
	m_planMode = mode;
}



const QList<Task>& Plan::getTasks() const
{
    // TODO: 在此处插入 return 语句
    return m_listTasks;
}

QList<Task>& Plan::getTasks()
{
	// TODO: 在此处插入 return 语句
	return m_listTasks;
}

double Plan::getDistance()
{
	return m_dDistance;
}

void Plan::setDistance(double distance)
{
	m_dDistance = distance;
}

double Plan::getEstimatedTime()
{
	return m_dEstimatedTime;
}

void Plan::setEstimatedTime(double time)
{
	m_dEstimatedTime = time;
}

double Plan::getMinDepth()
{
	return m_dMinDepth;
}

void Plan::setMinDepth(double depth)
{
	m_dMinDepth = depth;
}

double Plan::getMaxDepth()
{
	return m_dMaxDepth;
}

void Plan::setMaxDepth(double depth)
{
	m_dMaxDepth = depth;
}

int Plan::getTaskCount()
{
    return m_iTaskCount;
}

void Plan::setTaskCount(int count)
{
    m_iTaskCount = count;
}

void Plan::generateTasks() {
    //m_listTask.clear();
    //统计每种任务类型的数量，用于生成编号
    switch (m_planMode) {
    case PlanMode::TEST_DEBUG:
        m_listTasks.append(Task(m_sPlanId, TaskType::SAIL));
       /* m_listTasks.append(Task(m_sPlanId, TaskType::SAIL));
        m_listTasks.append(Task(m_sPlanId, TaskType::RECOVERY));*/
        break;
    /*case PlanMode::SURFACE_RECON:
        m_listTasks.append(Task(m_sPlanId, TaskType::SAIL));
        m_listTasks.append(Task(m_sPlanId, TaskType::PATROL_DETECT));
        m_listTasks.append(Task(m_sPlanId, TaskType::SAIL));
        m_listTasks.append(Task(m_sPlanId, TaskType::RECOVERY));
        break;
    case PlanMode::UNDERWATER_RECON:
        m_listTasks.append(Task(m_sPlanId, TaskType::SAIL));
        m_listTasks.append(Task(m_sPlanId, TaskType::PATROL_DETECT));
        m_listTasks.append(Task(m_sPlanId, TaskType::SAIL));
        m_listTasks.append(Task(m_sPlanId, TaskType::RECOVERY));
        break;
    case PlanMode::SURFACE_STRIKE:
        m_listTasks.append(Task(m_sPlanId, TaskType::SAIL));
        m_listTasks.append(Task(m_sPlanId, TaskType::PATROL_DETECT));
        m_listTasks.append(Task(m_sPlanId, TaskType::STRIKE));
        m_listTasks.append(Task(m_sPlanId, TaskType::ESCAPE));
        m_listTasks.append(Task(m_sPlanId, TaskType::SILENT));
        m_listTasks.append(Task(m_sPlanId, TaskType::SAIL));
        m_listTasks.append(Task(m_sPlanId, TaskType::RECOVERY));
        break;
    case PlanMode::UNDERWATER_STRIKE:
        m_listTasks.append(Task(m_sPlanId, TaskType::SAIL));
        m_listTasks.append(Task(m_sPlanId, TaskType::PATROL_DETECT));
        m_listTasks.append(Task(m_sPlanId, TaskType::STRIKE));
        m_listTasks.append(Task(m_sPlanId, TaskType::ESCAPE));
        m_listTasks.append(Task(m_sPlanId, TaskType::SILENT));
        m_listTasks.append(Task(m_sPlanId, TaskType::SAIL));
        m_listTasks.append(Task(m_sPlanId, TaskType::RECOVERY));
        break;
    case PlanMode::LAND_STRIKE:
        m_listTasks.append(Task(m_sPlanId, TaskType::SAIL));
        m_listTasks.append(Task(m_sPlanId, TaskType::SHORE_BASED_STRIKE));
        m_listTasks.append(Task(m_sPlanId, TaskType::ESCAPE));
        m_listTasks.append(Task(m_sPlanId, TaskType::SAIL));
        m_listTasks.append(Task(m_sPlanId, TaskType::RECOVERY));
        break;*/
    default:
        break;
    }
}

void Plan::updateMetrics() {
    m_dDistance = 0;
    m_dEstimatedTime = 0;

    for ( Task& task : m_listTasks) {
        if (task.getTaskType() == TaskType::SAIL) {
            m_dDistance += task.getParam("distance").toDouble();
            double speed = task.getParam("speed").toDouble();
            if (speed > 0) {
                m_dEstimatedTime += task.getParam("distance").toDouble() / speed;
            }
        }
    }
}

//QJsonObject Plan::toJson() const {
//    QJsonObject obj;
//    obj["name"] = m_sName;
//    obj["mode"] = static_cast<int>(m_planMode);
//    obj["distance"] = m_dDistance;
//    obj["estimatedTime"] = m_dEstimatedTime;
//    obj["minDepth"] = m_dMinDepth;
//    obj["maxDepth"] = m_dMaxDepth;
//
//    QJsonArray tasks;
//    for (const Task& task : m_listTask) {
//        tasks.append(task.toJson());
//    }
//    obj["tasks"] = tasks;
//
//    return obj;
//}
QJsonObject Plan::toJson() const
{
    QJsonObject obj;
    obj["planId"] = m_sPlanId;       // 计划ID
    obj["planName"] = m_sPlanName;   // 计划名称
    obj["planMode"] = static_cast<int>(m_planMode);  // 计划模式

    // 序列化当前计划包含的所有任务（核心）
    QJsonArray taskArray;
    // 转换任务列表为指针列表（若原列表为非指针，需取地址）
    QList<const Task*> taskPtrs;
    for (const Task& task : m_listTasks) {
        taskPtrs.append(&task);
    }
    // 调用TaskSequenceSerializer序列化任务列表
    if (!TaskSequenceSerializer::serializeMissionTree(taskPtrs, taskArray)) {
        qWarning() << "计划" << m_sPlanId << "的任务序列化失败";
    }
    obj["tasks"] = taskArray;  // 任务列表JSON数组

    return obj;
}
Plan Plan::fromJson(const QJsonObject& json) {
    Plan plan;
    plan.setPlanName(json["name"].toString());
    plan.setPlanMode(static_cast<PlanMode>(json["mode"].toInt()));
    plan.getTasks().clear();

    QJsonArray tasks = json["tasks"].toArray();
    for (const QJsonValue& taskValue : tasks) {
        plan.getTasks().append(Task::fromJson(taskValue.toObject()));
    }

    return plan;
}