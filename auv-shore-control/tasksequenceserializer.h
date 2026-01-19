#pragma once
#include <QJsonObject>
#include <QJsonArray>
#include <QList>
#include <QString>
#include "task.h" // 包含Task类定义
class TaskSequenceSerializer 
{
public:
    // 将使命树任务列表序列化为JSON数组
    static bool serializeMissionTree(const QList<const Task*>& missionTasks, QJsonArray& taskArray);

    // 将JSON数组保存为文件
    static bool saveJsonToFile(const QJsonArray& taskArray, const QString& filePath);

    // 字段校验（确保符合策略约束）
    static bool validateTaskField(const Task* task, QString& errorMsg);
private:
    // 单个任务转换为JSON对象（映射策略字段）
    static QJsonObject taskToJson(const Task* task);

  
};
