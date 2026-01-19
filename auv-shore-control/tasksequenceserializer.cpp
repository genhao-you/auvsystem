#include "tasksequenceserializer.h"
#include <QFile>
#include <QJsonDocument>
#include <QDebug>
#include <cmath>

// 使命树任务列表序列化
bool TaskSequenceSerializer::serializeMissionTree(const QList<const Task*>& missionTasks, QJsonArray& taskArray) {
    taskArray = QJsonArray(); // 清空数组
    for (const Task* task : missionTasks) {
        // 校验任务字段合法性
        QString errorMsg;
        if (!validateTaskField(task, errorMsg)) {
            qWarning() << "任务校验失败：" << errorMsg << "（任务序号：" << task->getTaskNum() << "）";
            return false;
        }
        // 转换为JSON对象并添加到数组
        taskArray.append(taskToJson(task));
    }
    return true;
}

// 保存JSON文件
bool TaskSequenceSerializer::saveJsonToFile(const QJsonArray& taskArray, const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "无法打开文件写入：" << filePath << "，错误：" << file.errorString();
        return false;
    }
    // 生成带缩进的JSON文档（便于调试）
    QJsonDocument doc(taskArray);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

// 单个任务转换为JSON对象（严格映射策略字段）
QJsonObject TaskSequenceSerializer::taskToJson(const Task* task) {
    QJsonObject taskJson;

    // 1. 设备号（6字节string）
    taskJson["deviceId"] = task->getDeviceId();

    // 2. 任务序号（8字节string，年月日序数）
    taskJson["taskNum"] = task->getTaskNum();

    // 3. 任务类型（int8：1=定深航行，2=定高航行）
    // 映射Task类中的NavigationMode到策略的任务类型
    if (task->getNavigationMode() == NavigationMode::DepthKeeping) {
        taskJson["taskType"] = 1; // 定深航行
    }
    else {
        taskJson["taskType"] = 2; // 定高航行
    }

    // 4. 起点（经/纬/深/高，double，保留5位小数）
    QJsonObject startPoint;
    const QVariantMap& start = task->getStartPoint();
    startPoint["longitude"] = QString::number(start["longitude"].toDouble(), 'f', 5); // 经度（5位小数）
    startPoint["latitude"] = QString::number(start["latitude"].toDouble(), 'f', 5);   // 纬度（5位小数）
    // 定深/定高区分字段
    if (task->getNavigationMode() == NavigationMode::DepthKeeping) {
        startPoint["depth"] = QString::number(start["depth"].toDouble(), 'f', 2); // 深度（精度0.05米）
    }
    else {
        startPoint["height"] = QString::number(start["height"].toDouble(), 'f', 2); // 高度（精度0.01米）
    }
    taskJson["startPoint"] = startPoint;

    // 5. 终点（同起点格式）
    QJsonObject endPoint;
    const QVariantMap& end = task->getEndPoint();
    endPoint["longitude"] = QString::number(end["longitude"].toDouble(), 'f', 5);
    endPoint["latitude"] = QString::number(end["latitude"].toDouble(), 'f', 5);
    if (task->getNavigationMode() == NavigationMode::DepthKeeping) {
        endPoint["depth"] = QString::number(end["depth"].toDouble(), 'f', 2);
    }
    else {
        endPoint["height"] = QString::number(end["height"].toDouble(), 'f', 2);
    }
    taskJson["endPoint"] = endPoint;

    // 6. 转速/速度（int16，0~1500rpm）
    taskJson["rotateSpeed"] = task->getRotateSpeed();

    // 7. 速度（int16，实际值×10取整）
    taskJson["speed"] = task->getSpeed();

    // 8. 超时时间（int32，单位秒）
    taskJson["timeOutPeriod"] = task->getTimeOutPeriod();

    // 9. 危险深度（int16）
    taskJson["dangerDepth"] = task->getDangerDepth();

    // 10. 危险高度（int16）
    taskJson["dangerHeight"] = task->getDangerHeight();

    // 11. 障碍物距离（int16）
    taskJson["obstacleDistance"] = task->getObstacleDistance();

    // 12. 危险电压（int16）
    taskJson["dangerVoltage"] = task->getDangerVoltage();

    // 13. 危险电流（int16，实际值×1000取整）
    taskJson["dangerCurrent"] = task->getDangerCurrent();

    // 14. 是否上浮（bool，1=上浮，0=不上浮）
    taskJson["needFloat"] = task->getNeedFloat() ? 1 : 0; // 转换为int便于下位机解析

    return taskJson;
}

// 字段校验（确保符合任务管理策略约束）
bool TaskSequenceSerializer::validateTaskField(const Task* task, QString& errorMsg) {
    // 1. 设备号校验（6字节）
    if (task->getDeviceId().length() != 6) {
        errorMsg = "设备号必须为6字节（当前：" + QString::number(task->getDeviceId().length()) + "）";
        return false;
    }

    // 2. 任务序号校验（8字节）
    if (task->getTaskNum().length() != 8) {
        errorMsg = "任务序号必须为8字节（当前：" + QString::number(task->getTaskNum().length()) + "）";
        return false;
    }

    // 3. 任务类型校验（仅定深/定高）
    NavigationMode navMode = task->getNavigationMode();
    if (navMode != NavigationMode::DepthKeeping && navMode != NavigationMode::HeightKeeping) {
        errorMsg = "任务类型必须为定深（1）或定高（2）";
        return false;
    }

    // 4. 起点经纬度精度校验（小数点后5位）
    const QVariantMap& start = task->getStartPoint();
    double lon = start["longitude"].toDouble();
    double lat = start["latitude"].toDouble();
    if (std::fmod(lon * 100000, 1) != 0 || std::fmod(lat * 100000, 1) != 0) {
        errorMsg = "起点经纬度必须保留小数点后5位";
        return false;
    }

    // 5. 深度/高度范围校验
    if (navMode == NavigationMode::DepthKeeping) {
        double depth = start["depth"].toDouble();
        if (depth < 0 || depth > 500) {
            errorMsg = "定深航行深度必须在0~500米范围内";
            return false;
        }
    }
    else {
        double height = start["height"].toDouble();
        if (height < 0.5 || height > 200) {
            errorMsg = "定高航行高度必须在0.5~200米范围内";
            return false;
        }
    }

    // 6. 转速范围校验（0~1500rpm）
    if (task->getRotateSpeed() < 0 || task->getRotateSpeed() > 1500) {
        errorMsg = "转速必须在0~1500rpm范围内";
        return false;
    }

    // 7. 危险电流范围校验（0~65535，对应0~65.535A）
    if (task->getDangerCurrent() < 0 || task->getDangerCurrent() > 65535) {
        errorMsg = "危险电流必须在0~65535范围内（对应0~65.535A）";
        return false;
    }

    // 其他字段校验（略，可根据需要补充）
    return true;
}