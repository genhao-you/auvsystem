#pragma once

#include <QObject>
#include <QStackedWidget>
#include <QMap>
#include "missionmodel.h"
#include "itaskeditor.h"
#include"planeditor.h"
#include"emptyeditor.h"
#include<QMutex>
#include<QPointer>
class EditorManager  : public QObject
{
	Q_OBJECT

public:
 explicit EditorManager(QStackedWidget* editorContainer, QObject* parent = nullptr);
	~EditorManager();
   
    void registerEditor(TaskType type, ITaskEditor* editor);
    void registerPlanEditor(ITaskEditor* editor);

    void showEditorForTask(Task* task);
    void showPlanEditor(Plan* plan);
    void showEmptyEditor();

    ITaskEditor* editor(TaskType type) const;
    ITaskEditor* planEditor() const;

    // 根据任务类型切换编辑器并设置任务数据
    bool switchEditor(TaskType type, Task* task);
    // 清空当前编辑器
    void clearCurrentEditor();
    // 获取当前编辑器修改后的任务
    Task* getCurrentTask() const;

signals:
    void taskModified();
private:
    void clearEditors();
    void setupEmptyEditor();
private:
    QStackedWidget* m_editorContainer;
    QMap<TaskType, ITaskEditor*> m_editorMap; // 任务类型-编辑器映射
    ITaskEditor* m_planEditor;
    ITaskEditor* m_emptyEditor;
    ITaskEditor* m_currentEditor = nullptr; // 当前激活的编辑器
    QMutex m_mutex;
    QList<QPointer<ITaskEditor>>m_editorOwership;//弱引用列表，用于跟踪编辑生命周期
    
};
