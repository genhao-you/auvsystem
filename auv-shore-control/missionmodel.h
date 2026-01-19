#pragma once

#include <QAbstractItemModel>
#include"mission.h"
class MissionModel  : public QAbstractItemModel
{
	Q_OBJECT

public:
    explicit MissionModel(QObject* parent = nullptr);
	~MissionModel();
public:
    enum NodeType { ROOT, MISSION, PLAN, TASK };
    enum CustomRoles {
        NodeTypeRole = Qt::UserRole + 1,
        NodePointerRole,
        MissionRole,
        PlanRole,
        TaskRole
    };
    // QAbstractItemModel interface
    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // Custom methods
    QModelIndex addMission();
    QModelIndex addPlan(const QModelIndex& parentMission, PlanMode mode);
    QModelIndex addTask(const QModelIndex& parentPlan, TaskType type = TaskType::SAIL);
    bool removeNode(const QModelIndex& index);

    Mission* missionAt(const QModelIndex& index) const;
    Plan* planAt(const QModelIndex& index) const;
    Task* taskAt(const QModelIndex& index) const;

    const QList<Mission>& missions() const { return m_missions; }
    void setMissions(const QList<Mission>& missions);

    void saveToJson(const QString& filePath);
    void loadFromJson(const QString& filePath);

signals:
    void missionStructureChanged();
    void dataModified();

private:
    struct Node {
        NodeType type;
        int row;
        Node* parent;
        QList<Node*> children;

        Mission* mission = nullptr;
        Plan* plan = nullptr;
        Task* task = nullptr;

        ~Node() {
            qDeleteAll(children);
            children.clear();
        }
    };

    Node* m_rootNode = nullptr;
    QList<Mission> m_missions;

    Node* getNode(const QModelIndex& index) const;
    Node* createMissionNode(Mission& mission, Node* parent);
    Node* createPlanNode(Plan& plan, Node* parent);
    Node* createTaskNode(Task& task, Node* parent);
    void removeNode(Node* node);

};
