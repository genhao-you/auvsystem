#pragma once

#include <QWidget>
#include "ui_missionview.h"
#include <QTreeView>
#include <QAction>
#include <QMenu>
#include "missionmodel.h"
class MissionView : public QWidget
{
	Q_OBJECT

public:
    explicit MissionView(QWidget* parent = nullptr);
    ~MissionView();
    void setModel(MissionModel* model);
    MissionModel* model() const;

    QModelIndex currentIndex() const;
    void setCurrentIndex(const QModelIndex& index);
    void edit(const QModelIndex& index);

signals:
    void addMissionRequested();
    void addPlanRequested(const QModelIndex& parent,PlanMode mode);
    void addTaskRequested(const QModelIndex& parent, TaskType type);
    void removeNodeRequested(const QModelIndex& index);
    void selectionChanged(const QModelIndex& current);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
private slots:
    void on_selection_changed(const QModelIndex& current);
private:
    void setupUi();
    void createActions();
    void createTaskMenu();
    void createPlanMenu();
    void createMissionMenu();
    void createEmptyAreaMenu();
private:
    QTreeView* m_treeView;
    MissionModel* m_model;
   
   
   
   
    QAction* m_setModeAction;

    //菜单
    QMenu* m_taskMenu;
    QMap<TaskType, QAction*> m_taskActions;
    QMenu* m_planMenu; 
    QMap<PlanMode, QAction*> m_planActions;
    QMenu* m_missionMenu;
    QMenu* m_emptyAreaMenu;

    //公共动作
    QAction* m_newAction;
    QAction* m_removeNodeAction;

    //使命菜单动作
    QAction* m_addPlanAction;

    //计划菜单动作
    QAction* m_addTaskAction;
    QMenu* m_specificTaskMenu;
    QMenu* m_planModeMenu;

    //空白区域菜单操作
    QAction* m_addMissionAction;
private:
	Ui::MissionViewClass ui;
};
