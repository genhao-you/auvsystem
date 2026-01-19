#pragma once

#include <QTreeWidget>
#include <QJsonObject>
#include"task.h"
class MissionMonitorWidget;
class MissionTreeWidget  : public QTreeWidget
{
	Q_OBJECT

public:
	MissionTreeWidget(MissionMonitorWidget* widget);
	~MissionTreeWidget();
    enum NodeType { MISSION_NODE = 1001,PLAN_NODE, TASK_NODE };
    Q_ENUM(NodeType)

      // 数据操作
    void addMission( const QString& name = "新建使命");
    void addPlan(QTreeWidgetItem* item, const QString& name = "新建计划");
    void addTask(QTreeWidgetItem* planItem, const QString& name = "新建任务");
    bool saveToJson(const QString& path);
    bool loadFromJson(const QString& path);
    void updateTreeContent();
protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    void createTaskMenu();
private slots:
    void onItemChanged(QTreeWidgetItem* item, int column);

private:
    // 菜单动作
    void createMissionMenu(QTreeWidgetItem* item, const QPoint& pos);
    void setupPlanMenu(QTreeWidgetItem* item, const QPoint& pos);
    void setupTaskMenu(QTreeWidgetItem* item, const QPoint& pos);

    // 数据序列化
    QJsonObject itemToJson(QTreeWidgetItem* item);
    QTreeWidgetItem* jsonToItem(const QJsonObject& obj);
private:
    QString m_sIconPath = ":/icons/missionlist/";
    QMenu* m_taskMenu;//新建任务主菜单
    QMap<TaskType, QAction*> m_mapTaskActions;//任务类型对应的动作
    MissionMonitorWidget* m_missionMonitorWidget;
};
