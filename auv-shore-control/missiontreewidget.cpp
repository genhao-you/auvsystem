#include "missiontreewidget.h"
#include <QMenu>
#include <QFile>
#include <QJsonArray>
#include <QMessageBox>
#include<QContextMenuEvent>
#include<QJsonDocument>
#include<qDebug>
#include"missionmodel.h"
#include"missionmonitorwidget.h"
#pragma execution_character_set("utf-8")
MissionTreeWidget::MissionTreeWidget(MissionMonitorWidget* widget )
	
{
    m_missionMonitorWidget = widget;
    setColumnCount(1);
    setHeaderLabel("使命列表");

    // 启用编辑后自动保存
    connect(this, &QTreeWidget::itemChanged, this, &MissionTreeWidget::onItemChanged);

    // 初始示例数据
    addMission("新建使命");
    /*this->setStyleSheet("./qss/missiontreewidget.qss");
    qCritical() << "=== 使命树 主题 QSS 加载成功 ===";
    this->style()->unpolish(this);
    this->style()->polish(this);
    this->update();*/
}

MissionTreeWidget::~MissionTreeWidget()
{}

void MissionTreeWidget::addMission( const QString& name)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(this, MISSION_NODE);
    item->setText(0, name);
    item->setIcon(0, QIcon(m_sIconPath + "mission.png"));
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    // 设置节点类型属性，用于QSS选择器
    item->setData(0, Qt::UserRole + 1, "MISSION_NODE");
    addTopLevelItem(item);
    expandItem(item);
    addPlan(item, "新建计划");

}

// 添加计划节点
void MissionTreeWidget::addPlan(QTreeWidgetItem* missionItem, const QString& name) {
    if (!missionItem || missionItem->type() != MISSION_NODE) return;

    QTreeWidgetItem* plan = new QTreeWidgetItem(PLAN_NODE);
    plan->setText(0, name);
    plan->setIcon(0, QIcon(m_sIconPath + "plan.png"));
    plan->setFlags(plan->flags() | Qt::ItemIsEditable);
    // 设置节点类型属性
    plan->setData(0, Qt::UserRole + 1, "PLAN_NODE");
    missionItem->addChild(plan);
    addTask(plan, "新建任务");
}

// 添加任务节点
void MissionTreeWidget::addTask(QTreeWidgetItem* planItem, const QString& name) {
    if (!planItem || planItem->type() != PLAN_NODE) return;

    QTreeWidgetItem* task = new QTreeWidgetItem(TASK_NODE);
    task->setText(0, name);
    task->setIcon(0, QIcon(m_sIconPath + "task.png"));
    task->setFlags(task->flags() | Qt::ItemIsEditable);
    // 设置节点类型属性
    task->setData(0, Qt::UserRole + 1, "TASK_NODE");
    planItem->addChild(task);
}

// 右键菜单事件
void MissionTreeWidget::contextMenuEvent(QContextMenuEvent* event) {
    QTreeWidgetItem* item = itemAt(event->pos());

    if (!item) {
        // 空白处菜单
        QMenu menu;
        menu.addAction("新建使命", this, [this]() { addMission(); });
        menu.exec(event->globalPos());
    }
    else {
        switch (item->type()) {
        case MISSION_NODE:createMissionMenu(item, event->pos()); break;
        case PLAN_NODE: setupPlanMenu(item, event->pos()); break;
        case TASK_NODE: setupTaskMenu(item, event->pos()); break;
        }
    }
}

void MissionTreeWidget::createTaskMenu()
{
    //清空现有动作
    m_taskMenu->clear();
    m_mapTaskActions.clear();
    //定义任务类型和显示名称的映射
    QMap<TaskType, QString>taskTypes = {
        { TaskType::SAIL,taskTypeToString(TaskType::SAIL)},
        { TaskType::SILENT,taskTypeToString(TaskType::SILENT)},
        { TaskType::PATROL_DETECT,taskTypeToString(TaskType::PATROL_DETECT)},
        { TaskType::STRIKE,taskTypeToString(TaskType::STRIKE)},
        { TaskType::SHORE_BASED_STRIKE,taskTypeToString(TaskType::SHORE_BASED_STRIKE)},
        { TaskType::ESCAPE,taskTypeToString(TaskType::ESCAPE)},
        { TaskType::FLOATING_CALIBRATION,taskTypeToString(TaskType::FLOATING_CALIBRATION)},
        { TaskType::FLOATING_COMMUNICATION,taskTypeToString(TaskType::FLOATING_COMMUNICATION)},
        { TaskType::BEACON_COMMUNICATION,taskTypeToString(TaskType::BEACON_COMMUNICATION)},
        { TaskType::RECOVERY,taskTypeToString(TaskType::RECOVERY)}
    };
    //为每种任务创建动作
    for (auto it = taskTypes.constBegin(); it != taskTypes.constEnd(); ++it)
    {
        QAction* action = new QAction(it.value(), this);
        //connect()
    }
}

void MissionTreeWidget::createMissionMenu(QTreeWidgetItem* item, const QPoint& pos)
{
    QMenu menu;

    // 新建使命动作
    QAction* newMissionAction = new QAction(QIcon(":/icons/add.png"), "新建使命", this);
    newMissionAction->setProperty("menuAction", "newMission");
    connect(newMissionAction, &QAction::triggered, [this, item]() {
        addMission("");
        });
    menu.addAction(newMissionAction);

    // 保存使命动作
    QAction* saveMissionAction = new QAction(QIcon(":/icons/save.png"), "保存使命", this);
    saveMissionAction->setProperty("menuAction", "saveMission");
    connect(saveMissionAction, &QAction::triggered, [item]() {
        QMessageBox::information(nullptr, "保存", "使命已保存: " + item->text(0));
        });
    menu.addAction(saveMissionAction);
    menu.addSeparator();

    // 删除使命动作
    QAction* delAction = new QAction(QIcon(":/icons/delete.png"), "删除使命", this);
    delAction->setProperty("menuAction", "deleteMission");
    connect(delAction, &QAction::triggered, [this, item]() {
        if (QMessageBox::question(nullptr, "确认", "删除使命将同时删除所有子任务，确定继续吗？")
            == QMessageBox::Yes) {
            delete takeTopLevelItem(indexOfTopLevelItem(item));
        }
        });
    menu.addAction(delAction);

    menu.exec(viewport()->mapToGlobal(pos));
}

// 计划节点菜单
void MissionTreeWidget::setupPlanMenu(QTreeWidgetItem* item, const QPoint& pos) {
    QMenu menu;

    menu.addAction(QIcon(":/icons/add.png"), "新建计划", [this, item]() {
        addPlan(item);
        });

    menu.addAction(QIcon(":/icons/save.png"), "保存计划", [item]() {
        QMessageBox::information(nullptr, "保存", "计划已保存: " + item->text(0));
        });

    menu.addSeparator();

    QAction* delAction = menu.addAction(QIcon(":/icons/delete.png"), "删除计划");
    connect(delAction, &QAction::triggered, [this, item]() {
        if (QMessageBox::question(nullptr, "确认", "删除计划将同时删除所有子任务，确定继续吗？")
            == QMessageBox::Yes) {
            delete takeTopLevelItem(indexOfTopLevelItem(item));
        }
        });

    menu.exec(viewport()->mapToGlobal(pos));
}

// 任务节点菜单
void MissionTreeWidget::setupTaskMenu(QTreeWidgetItem* item, const QPoint& pos) {
    QMenu menu;

    menu.addAction(QIcon(":/icons/add.png"), "新建任务", [this, item]() {
        addTask(item);
        });
    menu.addAction(QIcon(":/icons/save.png"), "保存任务", [item]() {
        QMessageBox::information(nullptr, "保存", "任务已保存: " + item->text(0));
        });

    QAction* delAction = menu.addAction(QIcon(":/icons/delete.png"), "删除任务");
    connect(delAction, &QAction::triggered, [this, item]() {
        if (item->parent()) {
            delete item->parent()->takeChild(item->parent()->indexOfChild(item));
        }
        });

    menu.exec(viewport()->mapToGlobal(pos));
}

// JSON序列化
bool MissionTreeWidget::saveToJson(const QString& path)
{
  /*  QJsonArray plans;
    for (int i = 0; i < topLevelItemCount(); ++i) {
        plans.append(itemToJson(topLevelItem(i)));
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) return false;

    file.write(QJsonDocument(plans).toJson());
    return true;*/
    MissionModel* model = m_missionMonitorWidget->getMissionModel();  // 需根据实际架构实现获取逻辑
    if (!model) {
        qWarning() << "未找到任务模型，保存失败";
        return false;
    }
    model->saveToJson(path);  // 委托模型处理保存
    return true;
}

QJsonObject MissionTreeWidget::itemToJson(QTreeWidgetItem* item) {
    QJsonObject obj;
    obj["type"] = item->type();
    obj["name"] = item->text(0);

    if (item->childCount() > 0) {
        QJsonArray tasks;
        for (int i = 0; i < item->childCount(); ++i) {
            tasks.append(itemToJson(item->child(i)));
        }
        obj["tasks"] = tasks;
    }

    return obj;
}

// JSON反序列化
bool MissionTreeWidget::loadFromJson(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return false;

    clear();
    QJsonArray plans = QJsonDocument::fromJson(file.readAll()).array();

    for (const QJsonValue& plan : plans) {
        QTreeWidgetItem* planItem = jsonToItem(plan.toObject());
        addTopLevelItem(planItem);
        expandItem(planItem);
    }

    return true;
}

void MissionTreeWidget::updateTreeContent()
{
    updateGeometry();
    if (parentWidget())
    {
        parentWidget()->adjustSize();
    }
}

QTreeWidgetItem* MissionTreeWidget::jsonToItem(const QJsonObject& obj) {
    QTreeWidgetItem* item = new QTreeWidgetItem(obj["type"].toInt());
    item->setText(0, obj["name"].toString());

    if (obj.contains("tasks")) {
        for (const QJsonValue& task : obj["tasks"].toArray()) {
            item->addChild(jsonToItem(task.toObject()));
        }
    }

    return item;
}

// 自动保存回调
void MissionTreeWidget::onItemChanged(QTreeWidgetItem* item, int) {
   
    // 实际项目中此处应触发保存逻辑
}