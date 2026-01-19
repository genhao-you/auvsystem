#pragma once
#include <QString>
#include <QMap>
#include <QUuid>
#include <atomic>
#include <mutex>
#include"missionenum.h"
class MissionNamingUtil
{
public:
    // 禁用构造/拷贝
    MissionNamingUtil() = delete;
    MissionNamingUtil(const MissionNamingUtil&) = delete;

    // ID生成
    static QString generateMissionId();
    static QString generateMissionName();
    static QString generatePlanId(const QString& missionId);  // 修正参数名waypointId->missionId
    static QString generatePlanName(const QString& missionId, PlanMode mode);  // 修正参数名
    static QString generateTaskId(const QString& planId);
    static QString generateTaskName(const QString& planId, TaskType taskType);

    // 删除后的计数器重置
    static void missionRemoved(const QString& missionId);
    static void planRemoved(const QString& missionId, const QString& planId, PlanMode mode);
    static void taskRemoved(const QString& planId, TaskType taskType);

    // 获取类型名称
    static QString getModeName(PlanMode mode);
    static QString getTaskTypeName(TaskType taskType);


private:
    // 原子计数器类型
    using AtomicCounter = std::atomic<int>;
    using CounterMap = QMap<QString, std::shared_ptr<AtomicCounter>>;
    //
    // 线程安全的计数器操作
    static int incrementCounter(CounterMap& counterMap, const QString& key);
    static void resetCounter(CounterMap& counterMap, const QString& key);
    static void removeCountersByPrefix(CounterMap& counterMap, const QString& prefix);

    // 计数器映射
    
    static CounterMap s_missionNameCounter;
    static CounterMap s_planCounter;
    static CounterMap s_planNameCounter;
    static CounterMap s_taskCounter;
    static CounterMap s_taskNameCounter;
    
    // 保护计数器映射的互斥锁
    static std::mutex& mapMutex();

    // 生成UUID
    static QString generateUuid();

    // 清理资源的静态方法（由析构助手调用）
    static void cleanupCounters(QMap<QString, AtomicCounter*>& counterMap);

    // 析构助手类（确保程序退出时清理资源）
    class CleanupHelper {
    public:
        ~CleanupHelper();
    };
    // 声明为指针，控制初始化顺序
    static std::unique_ptr<CleanupHelper> cleanupHelper;
};
