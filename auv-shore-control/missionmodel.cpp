#include "missionmodel.h"
#include <QJsonDocument>
#include<QJsonArray>
#include<QFile>
#include<QRegularExpression>
#include<QQueue>
#include<qDebug>
#include"tasksequenceserializer.h"
// MissionModel implementation
#pragma execution_character_set("utf-8")
MissionModel::MissionModel(QObject* parent) :
    QAbstractItemModel(parent)
    ,m_rootNode(nullptr)
{
    // 直接初始化 missions，不依赖根节点
    m_missions.append(Mission::createDefault());
    // 创建节点结构，但使用局部根节点管理
    Node* tempRoot = new Node{ ROOT, -1, nullptr, {} };
    createMissionNode(m_missions.last(), tempRoot);
    // 将临时根节点的子节点转移出来，避免递归析构问题
    m_rootNode = tempRoot;
    tempRoot = nullptr;
}

MissionModel::~MissionModel()
{
    // 安全释放根节点及其子节点
    if (m_rootNode) {
        // 先清除所有子节点引用
        for (Node* child : m_rootNode->children) {
            child->parent = nullptr; // 切断父子关系
        }

        // 非递归方式释放所有节点
        QQueue<Node*> nodesToDelete;
        nodesToDelete.enqueue(m_rootNode);

        while (!nodesToDelete.isEmpty()) {
            Node* node = nodesToDelete.dequeue();
            // 子节点入队
            for (Node* child : node->children) {
                nodesToDelete.enqueue(child);
            }
            // 清除指针引用
            node->mission = nullptr;
            node->plan = nullptr;
            node->task = nullptr;
            node->children.clear();
            node->parent = nullptr;
            // 删除节点
            delete node;
        }

        m_rootNode = nullptr;
    }

    m_missions.clear();
}

QModelIndex MissionModel::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    Node* parentNode = getNode(parent);
    if (!parentNode || row < 0 || row >= parentNode->children.size())
        return QModelIndex();

    Node* childNode = parentNode->children[row];
    return childNode ? createIndex(row, column, childNode) : QModelIndex();
}

QModelIndex MissionModel::parent(const QModelIndex& child) const {
    if (!child.isValid())
        return QModelIndex();

    Node* childNode = static_cast<Node*>(child.internalPointer());
    Node* parentNode = childNode->parent;

    if (parentNode == m_rootNode)
        return QModelIndex();

    return createIndex(parentNode->row, 0, parentNode);
}

int MissionModel::rowCount(const QModelIndex& parent) const {
    Node* parentNode = getNode(parent);
    return parentNode ? parentNode->children.size() : 0;
}

int MissionModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent)
        return 1;
}

QVariant MissionModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid())
        return QVariant();

    Node* node = static_cast<Node*>(index.internalPointer());

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch (node->type) {
        case MISSION: return node->mission ? node->mission->getName() : "";
        case PLAN: return node->plan ? node->plan->getPlanName() : "";
        case TASK: return node->task ? node->task->getTaskName() : "";
        default: return QVariant();
        }
    case NodeTypeRole: return static_cast<int>(node->type);
   // case MissionRole: return QVariant::fromValue(node->mission);
    case PlanRole: return QVariant::fromValue(node->plan);
    case TaskRole: return QVariant::fromValue(node->task);
    default: return QVariant();
    }
}

bool MissionModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (role != Qt::EditRole || !index.isValid())
        return false;

    Node* node = static_cast<Node*>(index.internalPointer());
    QString newName = value.toString();

    switch (node->type) {
    case MISSION:
        if (node->mission) {
            node->mission->setName(newName);
            emit dataChanged(index, index);
            emit dataModified();
            return true;
        }
        break;
    case PLAN:
        if (node->plan) {
            node->plan->setPlanName(newName);
            emit dataChanged(index, index);
            emit dataModified();
            return true;
        }
        break;
    default:
        break;
    }

    return false;
}

Qt::ItemFlags MissionModel::flags(const QModelIndex& index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;

    Node* node = static_cast<Node*>(index.internalPointer());
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);

    if (node->type == MISSION || node->type == PLAN) {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

QModelIndex MissionModel::addMission() {
    beginInsertRows(QModelIndex(), m_missions.size(), m_missions.size());

    Mission newMission;
    m_missions.append(newMission);
    Node* missionNode = createMissionNode(m_missions.last(), m_rootNode);

    endInsertRows();
    emit missionStructureChanged();
    emit dataModified();
    return createIndex(missionNode->row, 0, missionNode);
}

QModelIndex MissionModel::addPlan(const QModelIndex& parentMission, PlanMode mode) {
    if (!parentMission.isValid())
        return QModelIndex();

    Node* missionNode = getNode(parentMission);
    if (!missionNode || missionNode->type != MISSION || !missionNode->mission)
        return QModelIndex();

    Mission* mission = missionNode->mission;
    beginInsertRows(parentMission, mission->getPlans().size(), mission->getPlans().size());

    Plan newPlan(mission->getId(),mode);
    mission->getPlans().append(newPlan);
    Node* planNode = createPlanNode(mission->getPlans().last(), missionNode);

    endInsertRows();
    emit missionStructureChanged();
    emit dataModified();
    return createIndex(planNode->row, 0, planNode);
}

QModelIndex MissionModel::addTask(const QModelIndex& parentPlan, TaskType type) {
    if (!parentPlan.isValid())
        return QModelIndex();

    Node* planNode = getNode(parentPlan);
    if (!planNode || planNode->type != PLAN || !planNode->plan)
        return QModelIndex();

    Plan* plan = planNode->plan;
    beginInsertRows(parentPlan, plan->getTasks().size(), plan->getTasks().size());

    Task newTask(plan->getId(),type);
    plan->getTasks().append(newTask);
    Node* taskNode = createTaskNode(plan->getTasks().last(), planNode);

    endInsertRows();
    emit missionStructureChanged();
    emit dataModified();
    return createIndex(taskNode->row, 0, taskNode);
}

bool MissionModel::removeNode(const QModelIndex& index) {
    if (!index.isValid() || !m_rootNode)
        return false;

    Node* node = getNode(index);
    if (!node || !node->parent || node == m_rootNode) // 禁止删除根节点
        return false;

    // 保存删除前的关键信息
    QString waypointId, planId, taskId;
    PlanMode planMode = PlanMode::TEST_DEBUG;
    TaskType taskType = TaskType::SAIL;

    // 提取信息时增加空指针检查
    switch (node->type)
    {
    case MISSION:
        if (node->mission) waypointId = node->mission->getId();
        break;
    case PLAN:
        if (node->plan) {
            planId = node->plan->getId();
            waypointId = node->plan->getMissionId();
            planMode = node->plan->getPlanMode();
        }
        break;
    case TASK:
        if (node->task) {
            taskType = node->task->getTaskType();
            planId = node->task->getPlanId();
        }
        break;
    default:
        break;
    }

    beginRemoveRows(index.parent(), index.row(), index.row());

    Node* parentNode = node->parent;
    // 从父节点列表中移除
    parentNode->children.removeAt(index.row());

    // 更新行号时增加边界检查
    for (int i = index.row(); i < parentNode->children.size(); ++i) {
        if (parentNode->children[i]) {
            parentNode->children[i]->row = i;
        }
    }

    // 根据节点类型从容器中移除
    switch (node->type) {
    case MISSION:
        for (int i = 0; i < m_missions.size(); ++i) {
            if (&m_missions[i] == node->mission) {
                m_missions.removeAt(i);
                break;
            }
        }
        break;
    case PLAN:
        if (node->plan && parentNode->mission) {
            auto& plans = parentNode->mission->getPlans();
            for (int i = 0; i < plans.size(); ++i) {
                if (&plans[i] == node->plan) {
                    plans.removeAt(i);
                    break;
                }
            }
        }
        break;
    case TASK:
        if (node->task && parentNode->plan) {
            auto& tasks = parentNode->plan->getTasks();
            for (int i = 0; i < tasks.size(); ++i) {
                if (&tasks[i] == node->task) {
                    tasks.removeAt(i);
                    break;
                }
            }
        }
        break;
    default:
        break;
    }

    // 清除指针引用
    node->mission = nullptr;
    node->plan = nullptr;
    node->task = nullptr;
    node->parent = nullptr;
    delete node; // 最后删除节点

    endRemoveRows();

    // 调用清理函数前检查ID有效性
    switch (node->type)
    {
    case MISSION:
        if (!waypointId.isEmpty())
            MissionNamingUtil::missionRemoved(waypointId);
        break;
    case PLAN:
        if (!planId.isEmpty() && !waypointId.isEmpty())
            MissionNamingUtil::planRemoved(waypointId, planId, planMode);
        break;
    case TASK:
        if (!planId.isEmpty())
            MissionNamingUtil::taskRemoved(planId, taskType);
        break;
    }

    emit missionStructureChanged();
    emit dataModified();
    return true;
}


Mission* MissionModel::missionAt(const QModelIndex& index) const {
    if (!index.isValid())
        return nullptr;

    Node* node = getNode(index);
    if (!node || node->type != MISSION)
        return nullptr;
    return node->mission;
}

Plan* MissionModel::planAt(const QModelIndex& index) const {
    if (!index.isValid())
        return nullptr;

    Node* node = getNode(index);
    if (!node || node->type != PLAN)
        return nullptr;
    return node->plan;
}

Task* MissionModel::taskAt(const QModelIndex& index) const {
    if (!index.isValid())
        return nullptr;

    Node* node = getNode(index);
    if (!node || node->type != TASK)
        return nullptr;
    return node->task;
}

void MissionModel::setMissions(const QList<Mission>& missions) {
    beginResetModel();

    // 先清理现有节点
    if (m_rootNode) {
        qDeleteAll(m_rootNode->children);
        m_rootNode->children.clear();
        delete m_rootNode;
        m_rootNode = nullptr;
    }

    // 创建新的根节点和结构
    m_rootNode = new Node{ ROOT, -1, nullptr, {} };
    m_missions = missions;

    for (Mission& mission : m_missions) {
        createMissionNode(mission, m_rootNode);
    }

    endResetModel();
    emit missionStructureChanged();
    emit dataModified();
}

void MissionModel::saveToJson(const QString& filePath) {

    QJsonArray missionArray;
    // 遍历所有使命，序列化每个使命（包含计划和任务）
    for (const Mission& mission : m_missions) {
        missionArray.append(mission.toJson());
    }

    // 调用TaskSequenceSerializer的工具方法保存JSON到文件
    if (!TaskSequenceSerializer::saveJsonToFile(missionArray, filePath)) {
        qWarning() << "任务树保存到文件失败：" << filePath;
    }
    else {
        qInfo() << "任务树已成功保存到：" << filePath;
    }
}

void MissionModel::loadFromJson(const QString& filePath) {
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开文件进行读取:" << filePath << file.errorString();
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isArray()) {
        qWarning() << "无效的JSON格式:" << filePath;
        return;
    }

    QList<Mission> missions;
    QJsonArray missionsArray = doc.array();
    for (const QJsonValue& value : missionsArray) {
        if (value.isObject()) {
            missions.append(Mission::fromJson(value.toObject()));
        }
    }

    setMissions(missions);
}

MissionModel::Node* MissionModel::getNode(const QModelIndex& index) const {
    if (!index.isValid()) {
        // 如果根节点不存在，创建临时空节点作为根
        static Node* dummyRoot = nullptr;
        if (!dummyRoot && !m_rootNode) {
            dummyRoot = new Node{ ROOT, -1, nullptr, {} };
        }
        return m_rootNode ? m_rootNode : dummyRoot;
    }
    return static_cast<Node*>(index.internalPointer());
}

MissionModel::Node* MissionModel::createMissionNode(Mission& mission, Node* parent) {
    Node* node = new Node{ MISSION, parent->children.size(), parent, {} };
    node->mission = &mission;
    parent->children.append(node);

    for (Plan& plan : mission.getPlans()) {
        createPlanNode(plan, node);
    }

    return node;
}

MissionModel::Node* MissionModel::createPlanNode(Plan& plan, Node* parent) {
    Node* node = new Node{ PLAN, parent->children.size(), parent, {} };
    node->plan = &plan;
    parent->children.append(node);

    for (Task& task : plan.getTasks()) {
        createTaskNode(task, node);
    }

    return node;
}

MissionModel::Node* MissionModel::createTaskNode(Task& task, Node* parent) {
    Node* node = new Node{ TASK, parent->children.size(), parent, {} };
    node->task = &task;
    parent->children.append(node);
    return node;
}

void MissionModel::removeNode(Node* node)
{

}

