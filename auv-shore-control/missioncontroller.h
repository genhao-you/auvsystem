#pragma once

#include <QObject>
#include<QMessageBox>
#include "missionmodel.h"
#include"missionview.h"
#include"basemodule.h"
#include "observer.h"
#include "frameglobal.h"
class MissionController : public BaseModule
{
	Q_OBJECT

public:
    explicit MissionController(QObject* parent = nullptr);
    /**
     * @brief 初始化模块
     * @return 初始化是否成功
     */
    bool initialize() override;

    /**
     * @brief 清理模块资源
     */
    void shutdown() override;


    // 下发使命树（对外接口）
    CheckError sendMissionData(const QString& localPath, const QString& remotePath);

    // 生成任务JSON并自动下发
    CheckError saveAndSendTasks();
    void setModel(MissionModel* model);
    void setView(MissionView* view);
    MissionModel* getModel();
    void newProject();
    void loadProject(const QString& filePath);
    void saveProject(const QString& filePath);
    void saveProjectAs();
    // 生成仅包含任务的JSON（按原有顺序遍历，不含使命和计划信息）
    QByteArray generateTasksOnlyJson() const;
    // 保存仅任务JSON到文件
    bool saveTasksOnlyJson(const QString& filePath) const;
    QString currentFile() const;
    bool isModified() const;
private:
    void updateWindowTitle();
    // 按协议打包数据（关键：与下位机约定的格式）
    QByteArray packProtocolData(const QString& remotePath, const QByteArray& jsonData);
    // 读取JSON文件内容
    CheckError readJsonFile(const QString& path, QByteArray& outData);

    static QByteArray orderedRootJsonSerialize(const QJsonObject& rootObj, bool indented);
    static QByteArray orderedTaskJsonSerialize(const QJsonObject& jsonObj, bool indented);
 signals:
    void projectLoaded();
    void projectSaved();
    void modificationChanged(bool modified);
    void currentFileChanged(const QString& filePath);
    void windowTitleChanged(const QString& title);
    // 切换编辑器信号
    void switchEditor(TaskType type, Task* task);
    // 清空编辑器信号
    void clearEditor();
    void dataModified();
public slots:
    void addMission();
    void addPlan(const QModelIndex& parent, PlanMode mode);
    void addTask(const QModelIndex& parent, TaskType type);
    void removeNode(const QModelIndex& index);
    void selectionChanged(const QModelIndex& current);
    // 处理编辑器任务修改
    void onTaskModified();
private slots:
    void handleModelModified();
    /**
     * @brief 处理接收到的消息
     * @param data 消息数据
     */
    void onMissionIssueResultReceived(const DataMessage& data);
private:
    MissionModel* m_model;
    MissionView* m_view;
    QString m_currentFile;
    bool m_isModified;
    QString m_tempJsonPath = "./temp_task_seq.json"; // 临时JSON文件路径
   

};
