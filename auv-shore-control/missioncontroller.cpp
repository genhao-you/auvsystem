#include "missioncontroller.h"
#include"tasksequenceserializer.h"
#include <QFileDialog>
#include <QMessageBox>
#include<qDebug>
#include<QJsonDocument>
#pragma execution_character_set("utf-8")
MissionController::MissionController(QObject* parent) 
    : BaseModule(ModuleType::TaskPlanning, "TaskPlanningModule", parent)
    , m_model(nullptr)
    , m_view(nullptr)
    , m_currentFile("")
    , m_isModified(false)
{
    initialize();
}

bool MissionController::initialize()
{
    if (!BaseModule::initialize()) return false;
    // 订阅任务下发结果事件
    subscribeEvent("taskIssue_result",
        [this](const DataMessage& msg) { onMissionIssueResultReceived(msg); });
    return true;
}

void MissionController::shutdown()
{
}

CheckError MissionController::sendMissionData(const QString& localPath, const QString& remotePath)
{
    // 1. 生成全局唯一request_id
    uint64_t requestId = CommandIdGenerator::instance().nextId();
    // 2. 封装元数据（本地路径、远程路径）和JSON数据
    QJsonObject meta;
    meta["localPath"] = localPath;
    meta["remotePath"] = remotePath;
    meta["request_id"] = QString::number(requestId); // 携带request_id
    meta["source_module"] = static_cast<int>(ModuleType::TaskPlanning); // 携带发送模块标识
   // 直接传递元数据JSON，无需组合JSON内容
    QByteArray metaData = QJsonDocument(meta).toJson(QJsonDocument::Compact);

    //传输数据到通信模块
    // 3. 调用BaseModule的transferData，指定目标和通道
    transferData(
         CommunicationChannel::WiredNetwork, // 通信通道（如WiredNetwork）
        "missionIssue_transfer",       // 目标接收器标识（唯一）
        metaData                   // 直接传输的二进制数据
    );
  
    return CheckError();
}

CheckError MissionController::saveAndSendTasks()
{
    // 生成本地路径
    QString localPath = "./TaskFile/TaskSequence_" +
        QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + ".json";

    // 1. 生成JSON文件（核心：调用现有生成逻辑）
    bool saveOk = saveTasksOnlyJson(localPath);
    if (!saveOk) {
        return CheckError(ErrorCode::FileWriteFailed, "生成JSON文件失败");
    }

    // 2. 读取JSON文件内容
    QByteArray jsonData;
    CheckError readError = readJsonFile(localPath, jsonData);
    if (!readError.isSuccess()) {
        return readError;
    }

    // 3. 生成远程路径（下位机保存路径）
    QString remotePath = "0/" + QFileInfo(localPath).fileName();

    // 4. 打包数据并发送
    return sendMissionData(localPath, remotePath);
}

void MissionController::setModel(MissionModel* model) {
    if (m_model == model)
        return;

    if (m_model) {
        disconnect(m_model, &MissionModel::dataModified,
            this, &MissionController::handleModelModified);
    }

    m_model = model;

    if (m_model) {
        connect(m_model, &MissionModel::dataModified,
            this, &MissionController::handleModelModified);
    }

    if (m_view) {
        m_view->setModel(m_model);
    }
}

void MissionController::setView(MissionView* view) {
    if (m_view == view)
        return;

    if (m_view) {
        disconnect(m_view, &MissionView::addMissionRequested,
            this, &MissionController::addMission);
        disconnect(m_view, &MissionView::addPlanRequested,
            this, &MissionController::addPlan);
        disconnect(m_view, &MissionView::addTaskRequested,
            this, &MissionController::addTask);
        disconnect(m_view, &MissionView::removeNodeRequested,
            this, &MissionController::removeNode);
        disconnect(m_view, &MissionView::selectionChanged,
            this, &MissionController::selectionChanged);
    }

    m_view = view;

    if (m_view) {
        connect(m_view, &MissionView::addMissionRequested,
            this, &MissionController::addMission);
        connect(m_view, &MissionView::addPlanRequested,
            this, &MissionController::addPlan);
        connect(m_view, &MissionView::addTaskRequested,
            this, &MissionController::addTask);
        connect(m_view, &MissionView::removeNodeRequested,
            this, &MissionController::removeNode);
        connect(m_view, &MissionView::selectionChanged,
            this, &MissionController::selectionChanged);

        if (m_model) {
            m_view->setModel(m_model);
        }
    }
}

MissionModel* MissionController::getModel()
{
    return m_model;
}

void MissionController::newProject() {
    if (!m_model)
        return;

    m_model->setMissions({ Mission::createDefault() });
    m_currentFile.clear();
    m_isModified = false;
    emit modificationChanged(false);
    emit currentFileChanged(m_currentFile);
    updateWindowTitle();
}

void MissionController::loadProject(const QString& filePath) {
    if (!m_model || filePath.isEmpty())
        return;

    m_model->loadFromJson(filePath);
    m_currentFile = filePath;
    m_isModified = false;
    emit modificationChanged(false);
    emit currentFileChanged(m_currentFile);
    emit projectLoaded();
    updateWindowTitle();
}

void MissionController::saveProject(const QString& filePath) {
    if (!m_model)
        return;

    QString savePath = filePath.isEmpty() ? m_currentFile : filePath;
    if (savePath.isEmpty()) {
        saveProjectAs();
        return;
    }
    // 保存前先验证所有任务合法性（可选，提前暴露错误）
    bool allValid = true;
    QString errorMsg;
    for (const Mission& mission : m_model->missions()) {
        for (const Plan& plan : mission.getPlans()) {
            for (const Task& task : plan.getTasks()) {
                if (!TaskSequenceSerializer::validateTaskField(&task, errorMsg)) {
                    allValid = false;
                    qWarning() << "任务验证失败：" << errorMsg;
                }
            }
        }
    }
    if (!allValid) {
        QMessageBox::warning(nullptr, "保存失败", "部分任务字段不合法，请检查后重试");
        return;
    }
    m_model->saveToJson(savePath);
    m_currentFile = savePath;
    m_isModified = false;
    emit modificationChanged(false);
    emit currentFileChanged(m_currentFile);
    emit projectSaved();
    updateWindowTitle();
}

void MissionController::saveProjectAs() {
    if (!m_model)
        return;

    QString savePath = QFileDialog::getSaveFileName(nullptr,
        tr("保存项目"), "", tr("任务规划文件 (*.mtp)"));
    if (savePath.isEmpty())
        return;

    saveProject(savePath);
}


QByteArray MissionController::generateTasksOnlyJson() const {
    if (!m_model) {
        qWarning() << "模型为空，无法生成任务JSON";
        return "";
    }

    // 1. 按原有顺序遍历：Mission → Plan → Task（与完整架构遍历顺序一致）
    QList<Task> allTasks;
    const QList<Mission>& missions = m_model->missions(); // 先取所有使命

    // 遍历每个使命（顺序不变）
    for (const Mission& mission : missions) {
        const QList<Plan>& plans = mission.getPlans(); // 再取使命下的所有计划

        // 遍历每个计划（顺序不变）
        for (const Plan& plan : plans) {
            const QList<Task>& tasks = plan.getTasks(); // 最后取计划下的所有任务

            // 收集任务（保持在计划中的原有顺序）
            allTasks.append(tasks);
        }
    }

    if (allTasks.isEmpty()) {
        qWarning() << "没有可提取的任务";
        return "";
    }

    // 2. 生成仅包含任务的"mission"数组（不含任何使命/计划信息）
    QJsonArray missionArray;
    for (const Task& task : allTasks) {
        missionArray.append(task.toJson()); // 仅用Task自身的JSON（不含所属计划/使命信息）
    }

    // 3. 构建顶层JSON（仅"mission"键）
    QJsonObject root;
    root["mission"] = missionArray;

    return orderedRootJsonSerialize(root, true);
}

bool MissionController::saveTasksOnlyJson(const QString& filePath) const {
    if (filePath.isEmpty()) return false;

    QByteArray jsonData = generateTasksOnlyJson();
    if (jsonData.isEmpty()) return false;

    QFileInfo fileInfo(filePath);
    QDir dir(fileInfo.path());
    if (!dir.exists() && !dir.mkpath(".")) return false;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;

    qint64 written = file.write(jsonData);
    file.close();
    return written == jsonData.size();
}
QString MissionController::currentFile() const {
    return m_currentFile;
}

bool MissionController::isModified() const {
    return m_isModified;
}

void MissionController::addMission() {
    if (!m_model || !m_view)
        return;

    QModelIndex newIndex = m_model->addMission();
    m_view->setCurrentIndex(newIndex);
    m_view->edit(newIndex);
}

void MissionController::addPlan(const QModelIndex& parent, PlanMode mode) {
    if (!m_model || !m_view || !parent.isValid())
        return;

    QModelIndex newIndex = m_model->addPlan(parent,mode);
    if (newIndex.isValid()) {
        m_view->setCurrentIndex(newIndex);
        m_view->edit(newIndex);
    }
}

void MissionController::addTask(const QModelIndex& parent, TaskType type) {
    if (!m_model || !m_view || !parent.isValid())
        return;

    QModelIndex newIndex = m_model->addTask(parent, type);
    if (newIndex.isValid()) {
        m_view->setCurrentIndex(newIndex);
    }

}

void MissionController::removeNode(const QModelIndex& index) {
    if (m_model && index.isValid())
    {
        QString name = m_model->data(index, Qt::DisplayRole).toString();
        QString message = tr("确定要删除%1吗").arg(name);
        QMessageBox::StandardButton reply = QMessageBox::question(
            m_view,
            tr("确认删除"),
            message, 
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::NoButton);
        if (reply == QMessageBox::Yes)
        {
              m_model->removeNode(index);
        }
      
    }
}

void MissionController::selectionChanged(const QModelIndex& current) {
    Q_UNUSED(current)
     // 可以在这里处理选择变化逻辑
    updateWindowTitle();
    if (!m_model || !current.isValid()) {
        // 非有效节点：清空编辑器
        emit clearEditor();
        return;
    }

    // 判断节点类型
    int nodeType = m_model->data(current, MissionModel::NodeTypeRole).toInt();
    if (nodeType != MissionModel::TASK) {
        emit clearEditor();
        return;
    }

    // 获取任务并切换对应编辑器
    Task* task = m_model->taskAt(current);
    if (task) {
        emit switchEditor(task->getTaskType(), task);
    }
}

void MissionController::onTaskModified()
{
    emit dataModified(); // 通知模型数据变更
}

void MissionController::onMissionIssueResultReceived(const DataMessage& data)
{

}

void MissionController::handleModelModified() {
    if (!m_isModified) {
        m_isModified = true;
        emit modificationChanged(true);
        updateWindowTitle();
    }
}

void MissionController::updateWindowTitle() {
    QString title = "任务规划系统";

    if (!m_currentFile.isEmpty()) {
        title += " - [" + QFileInfo(m_currentFile).fileName() + "]";
    }
    else {
        title += " - [未命名]";
    }

    if (m_isModified) {
        title += "*";
    }

    emit windowTitleChanged(title);
}



CheckError MissionController::readJsonFile(const QString& path, QByteArray& outData)
{
    if (!QFile::exists(path)) {
        return CheckError(ErrorCode::FileNotExists, "文件不存在: " + path);
    }
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return CheckError(ErrorCode::FileOpenFailed, "无法打开文件: " + path);
    }
    outData = file.readAll();
    file.close();
    if (outData.isEmpty()) {
        return CheckError(ErrorCode::FileEmpty, "文件内容为空: " + path);
    }
    return CheckError();
}
QByteArray MissionController::orderedRootJsonSerialize(const QJsonObject& rootObj, bool indented)

{
    const QString indentUnit = indented ? "    " : "";
    int indentLevel = 0;
    QString jsonStr = "{\n";
    indentLevel++;

    // 顶层仅"mission"字段，直接处理
    if (rootObj.contains("mission")) {
        jsonStr += indentUnit.repeated(indentLevel) + "\"mission\": [";
        QJsonArray missionArr = rootObj.value("mission").toArray();

        // 遍历mission数组中的每个Task对象
        for (int i = 0; i < missionArr.size(); ++i) {
            QJsonObject taskObj = missionArr.at(i).toObject();
            // 序列化单个Task（使用上面的有序序列化函数）
            QByteArray taskJson = orderedTaskJsonSerialize(taskObj, indented);
            QString taskJsonStr = QString::fromUtf8(taskJson);

            // 调整Task的缩进（适配数组格式）
            if (indented) {
                taskJsonStr = taskJsonStr.replace("\n", "\n" + indentUnit.repeated(indentLevel + 1));
                jsonStr += "\n" + indentUnit.repeated(indentLevel + 1) + taskJsonStr;
            }
            else {
                jsonStr += taskJsonStr;
            }

            // 最后一个Task不加逗号
            if (i != missionArr.size() - 1) {
                jsonStr += ",";
            }
        }

        jsonStr += "\n" + indentUnit.repeated(indentLevel) + "]";
    }

    indentLevel--;
    jsonStr += "\n" + indentUnit.repeated(indentLevel) + "}";

    return jsonStr.toUtf8();
}

 QByteArray MissionController::orderedTaskJsonSerialize(const QJsonObject& jsonObj, bool indented)
{
    // 预设Task字段的有序列表（与Task::toJson()中的字段顺序完全一致）
    QStringList taskOrderedKeys = {
        "device_id", "task_id", "task_type", "start_pos", "end_pos",
        "rotor_speed", "timeout", "max_depth", "min_height", "obstacle_dist",
        "min_voltage", "max_current", "is_floating"
    };

    const QString indentUnit = indented ? "    " : ""; // 缩进单位：4个空格
    int indentLevel = 0;
    QString jsonStr = "{\n";
    indentLevel++; // 内部字段缩进+1

    int validFieldCount = 0; // 统计有效字段数（避免最后一个字段多余逗号）
    int totalValidCount = 0;

    // 先统计有效字段数
    for (const QString& key : taskOrderedKeys) {
        if (jsonObj.contains(key)) {
            totalValidCount++;
        }
    }

    // 按有序键列表拼接字段
    for (const QString& key : taskOrderedKeys) {
        if (!jsonObj.contains(key)) {
            continue;
        }

        validFieldCount++;
        // 拼接缩进
        jsonStr += indentUnit.repeated(indentLevel);
        // 拼接键名
        jsonStr += "\"" + key + "\": ";

        // 拼接字段值（适配你的字段类型：字符串、数字、数组、布尔值）
        QJsonValue value = jsonObj.value(key);
        if (value.isString()) {
            jsonStr += "\"" + value.toString() + "\"";
        }
        else if (value.isDouble()) {
            // 区分整数和小数，保持格式整洁
            double doubleVal = value.toDouble();
            if (doubleVal == static_cast<qint64>(doubleVal)) {
                jsonStr += QString::number(static_cast<qint64>(doubleVal));
            }
            else {
                jsonStr += QString::number(doubleVal);
            }
        }
        else if (value.isArray()) {
            QJsonArray arr = value.toArray();
            jsonStr += "[";
            // 拼接数组元素
            for (int i = 0; i < arr.size(); ++i) {
                QJsonValue arrVal = arr.at(i);
                if (arrVal.isDouble()) {
                    jsonStr += QString::number(arrVal.toDouble());
                }
                if (i != arr.size() - 1) {
                    jsonStr += ", ";
                }
            }
            jsonStr += "]";
        }
        else if (value.isBool()) {
            jsonStr += value.toBool() ? "true" : "false";
        }

        // 最后一个有效字段不加逗号
        if (validFieldCount < totalValidCount) {
            jsonStr += ",";
        }
        jsonStr += "\n";
    }

    indentLevel--; // 顶层缩进-1
    jsonStr += indentUnit.repeated(indentLevel) + "}";

    return jsonStr.toUtf8();
}



