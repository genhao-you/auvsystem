#include "missionnameutil.h"
#pragma execution_character_set("utf-8")
// 初始化静态成员
MissionNamingUtil::CounterMap MissionNamingUtil::s_missionNameCounter;
MissionNamingUtil::CounterMap MissionNamingUtil::s_planCounter;
MissionNamingUtil::CounterMap MissionNamingUtil::s_planNameCounter;
MissionNamingUtil::CounterMap MissionNamingUtil::s_taskCounter;
MissionNamingUtil::CounterMap MissionNamingUtil::s_taskNameCounter;
std::unique_ptr<MissionNamingUtil::CleanupHelper> MissionNamingUtil::cleanupHelper;
// 实现析构助手
MissionNamingUtil::CleanupHelper::~CleanupHelper()
{
   // MissionNamingUtil::cleanupCounters(MissionNamingUtil::missionNameCounter());
    //枷锁保证线程安全
    std::lock_guard<std::mutex> lock(MissionNamingUtil::mapMutex());
    s_missionNameCounter.clear();
    s_planCounter.clear();
    s_planNameCounter.clear();
    s_taskCounter.clear();
    s_taskNameCounter.clear();
    
}

std::mutex& MissionNamingUtil::mapMutex() {
    static std::mutex mutex;
    return mutex;
}

QString MissionNamingUtil::generateMissionId() {
    return generateUuid();
}

QString MissionNamingUtil::generateMissionName() {
    std::lock_guard<std::mutex> lock(mapMutex());
    // 确保清理助手在首次使用时初始化，晚于计数器映射
    if (!cleanupHelper) {
        cleanupHelper = std::make_unique<CleanupHelper>();
    }
    int num = incrementCounter(s_missionNameCounter, "mission");
    return QString("使命%1").arg(num);
}

QString MissionNamingUtil::generatePlanId(const QString& missionId) {
    std::lock_guard<std::mutex> lock(mapMutex());
    int num = incrementCounter(s_planCounter, missionId);
    return QString("%1_plan%2").arg(missionId).arg(num);
}

QString MissionNamingUtil::generatePlanName(const QString& missionId, PlanMode mode) {
    std::lock_guard<std::mutex> lock(mapMutex());
    QString key = QString("%1_%2").arg(missionId).arg(static_cast<int>(mode));
    int num = incrementCounter(s_planNameCounter, key);
    return QString("%1计划%2").arg(getModeName(mode)).arg(num);
}

QString MissionNamingUtil::generateTaskId(const QString& planId) {
    std::lock_guard<std::mutex> lock(mapMutex());
    int num = incrementCounter(s_taskCounter, planId);
    return QString("%1_task%2").arg(planId).arg(num);
}

QString MissionNamingUtil::generateTaskName(const QString& planId, TaskType taskType) {
    std::lock_guard<std::mutex> lock(mapMutex());
    QString key = QString("%1_%2").arg(planId).arg(static_cast<int>(taskType));
    int num = incrementCounter(s_taskNameCounter, key);
    return QString("%1%2").arg(getTaskTypeName(taskType)).arg(num);
}

void MissionNamingUtil::missionRemoved(const QString& missionId) {
    std::lock_guard<std::mutex> lock(mapMutex());

    // 移除与该使命相关的所有计划计数器
    s_planCounter.remove(missionId);

    // 移除该使命下的所有计划名称计数器
    removeCountersByPrefix(s_planNameCounter, missionId);

    // 重置使命名称计数器（可选）
    resetCounter(s_missionNameCounter, "mission");
}

void MissionNamingUtil::planRemoved(const QString& missionId, const QString& planId, PlanMode mode) {
    std::lock_guard<std::mutex> lock(mapMutex());

    // 移除该计划下的所有任务计数器
    s_taskCounter.remove(planId);

    // 移除该计划下的所有任务名称计数器
    removeCountersByPrefix(s_taskNameCounter, planId);

    // 重置该模式下的计划名称计数器
    QString key = QString("%1_%2").arg(missionId).arg(static_cast<int>(mode));
    resetCounter(s_planNameCounter, key);
}

void MissionNamingUtil::taskRemoved(const QString& planId, TaskType taskType) {
    std::lock_guard<std::mutex> lock(mapMutex());
    QString key = QString("%1_%2").arg(planId).arg(static_cast<int>(taskType));
    resetCounter(s_taskNameCounter, key);
}

QString MissionNamingUtil::getModeName(PlanMode mode) {
    
    return planModeToString(mode);
}

QString MissionNamingUtil::getTaskTypeName(TaskType taskType) {
    return taskTypeToString(taskType);
}

QString MissionNamingUtil::generateUuid() {
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

int MissionNamingUtil::incrementCounter(CounterMap& counterMap, const QString& key) {
    // 确保计数器存在
    if (!counterMap.contains(key)) {
        counterMap[key] = std::make_shared<AtomicCounter>(0);
    }

    // 原子递增并返回值
    return ++(*counterMap[key]);
}

void MissionNamingUtil::resetCounter(CounterMap& counterMap, const QString& key) {
    if (counterMap.contains(key)) {
        counterMap[key]->store(0);
    }
}

void MissionNamingUtil::removeCountersByPrefix(CounterMap& counterMap, const QString& prefix) {
    auto it = counterMap.begin();
    while (it != counterMap.end()) {
        if (it.key().startsWith(prefix)) {
            it = counterMap.erase(it);
        }
        else {
            ++it;
        }
    }
}

void MissionNamingUtil::cleanupCounters(QMap<QString, AtomicCounter*>& counterMap) {
    for (auto* counter : counterMap) {
        delete counter;
    }
    counterMap.clear();
}

