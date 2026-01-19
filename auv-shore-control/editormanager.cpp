#include "editormanager.h"
#include<QDebug>
EditorManager::EditorManager(QStackedWidget* editorContainer, QObject* parent)
    : QObject(parent), m_editorContainer(editorContainer),
    m_planEditor(nullptr), m_emptyEditor(nullptr)
{
    setupEmptyEditor();
}

EditorManager::~EditorManager()
{
    clearEditors();
    if (m_planEditor != nullptr)
        delete m_planEditor;
    m_planEditor = nullptr;

    if (m_emptyEditor != nullptr)
        delete m_emptyEditor;
    m_emptyEditor = nullptr;
}

void EditorManager::registerEditor(TaskType type, ITaskEditor* editor)
{
   // QMutexLocker locked(&m_mutex);
    if (!editor || m_editorMap.contains(type))
        return;
    //检查编辑器是否已被注册（避免重复注册同一对象）

    for (auto ptr : m_editorOwership)
    {
        if (ptr.data() == editor)
        {
            //qWarning() << "Editor already registered with a different type:" << type;
            return;
        }
    }
    m_editorMap.insert(type, editor);
    m_editorContainer->addWidget(editor);

    // 连接编辑器修改信号
    connect(editor, &ITaskEditor::taskModified, this, &EditorManager::taskModified);
}

void EditorManager::registerPlanEditor(ITaskEditor* editor)
{
    if (!editor || m_planEditor)
        return;

    m_planEditor = editor;
    m_editorContainer->addWidget(editor);
}

void EditorManager::showEditorForTask(Task* task)
{
    if (!task) {
        showEmptyEditor();
        return;
    }

    ITaskEditor* editor = m_editorMap.value(task->getTaskType(), nullptr);
    if (editor) {
        editor->setTask(task);
        m_editorContainer->setCurrentWidget(editor);
    }
    else {
        showEmptyEditor();
    }
}

void EditorManager::showPlanEditor(Plan* plan)
{
    if (!m_planEditor) {
        showEmptyEditor();
        return;
    }

    m_planEditor->setTask(nullptr); // Clear any task data
    // Assuming plan editor has a setPlan() method
    if (auto planEditor = qobject_cast<PlanEditor*>(m_planEditor)) {
        planEditor->setPlan(plan);
        m_editorContainer->setCurrentWidget(m_planEditor);
    }
    else {
        showEmptyEditor();
    }
}

void EditorManager::showEmptyEditor()
{
    if (!m_emptyEditor) {
        m_emptyEditor = new EmptyEditor();
        m_editorContainer->addWidget(m_emptyEditor);
    }
    m_editorContainer->setCurrentWidget(m_emptyEditor);
}

ITaskEditor* EditorManager::editor(TaskType type) const
{
    return m_editorMap.value(type, nullptr);
}

ITaskEditor* EditorManager::planEditor() const
{
    return m_planEditor;
}

void EditorManager::clearEditors()
{
    QMutexLocker locker(&m_mutex);
    //先断开所有链接，防止信号出发
    for (auto editor : m_editorMap)
    {
        disconnect(editor, &QObject::destroyed, this, nullptr);
    }
    qDeleteAll(m_editorMap);
    m_editorMap.clear();
    qDeleteAll(m_editorOwership);
    m_editorOwership.clear();
}

void EditorManager::setupEmptyEditor()
{
    m_emptyEditor = new EmptyEditor();
    m_editorContainer->addWidget(m_emptyEditor);
    m_editorContainer->setCurrentWidget(m_emptyEditor);
}

// 切换编辑器并设置任务
bool EditorManager::switchEditor(TaskType type, Task* task)
{
    if (!m_editorMap.contains(type)) return false;
    m_currentEditor = m_editorMap[type];
    m_editorContainer->setCurrentWidget(m_currentEditor);
    m_currentEditor->setTask(task);
    return true;
}

// 清空当前编辑器
void EditorManager::clearCurrentEditor()
{
    if (m_currentEditor) {
        m_currentEditor->clear();
    }
}

// 获取当前任务
Task* EditorManager::getCurrentTask() const
{
    return m_currentEditor ? m_currentEditor->getTask() : nullptr;
}