#include "mission.h"
#include<QJsonArray>

Mission::Mission() {
    //m_listPlans.append(Plan());
    m_sId = MissionNamingUtil::generateMissionId();
    m_sName = MissionNamingUtil::generateMissionName();

}

Mission::Mission(const Mission& other)
    : m_sId(other.m_sId), m_sName(other.m_sName)
{
    // 显式深拷贝每个Plan对象，避免依赖Plan的默认拷贝
    for (const Plan& plan : other.m_listPlans) {
        m_listPlans.append(Plan(plan)); // 强制调用Plan的拷贝构造
    }
}

Mission& Mission::operator=(const Mission& other)
{
    if (this != &other) {
        m_sId = other.m_sId;
        m_sName = other.m_sName;

        m_listPlans.clear(); // 先清空现有内容
        // 显式复制每个Plan
        for (const Plan& plan : other.m_listPlans) {
            m_listPlans.append(Plan(plan));
        }
    }
    return *this;
}
Mission::~Mission()
{
    // 清理资源，如果有动态分配的成员需要在这里释放
    m_listPlans.clear(); // 显式清空列表
}

QString Mission::getName() const
{
	return m_sName;
}

void Mission::setName(const QString& name)
{
	m_sName = name;
}

const QList<Plan>& Mission::getPlans() const
{
	// TODO: 在此处插入 return 语句
	return m_listPlans;
}

QList<Plan>& Mission::getPlans()
{
    // TODO: 在此处插入 return 语句
    return m_listPlans;
}

void Mission::setPlans(QList<Plan>& plans)
{
	m_listPlans = plans;
}




QString Mission::getId() const
{
    return m_sId;
}

void Mission::setId(QString id)
{
    m_sId = id;
}

QJsonObject Mission::toJson() const {
    QJsonObject obj;
    obj["name"] = m_sName;

    QJsonArray plans;
    for (const Plan& plan : m_listPlans) {
        plans.append(plan.toJson());
    }
    obj["plans"] = plans;

    return obj;
}

Mission Mission::fromJson(const QJsonObject& json) {
    Mission mission;
    mission.getPlans().clear();
    QJsonArray plans = json["plans"].toArray();
    for (const QJsonValue& planValue : plans) {
        mission.getPlans().append(Plan::fromJson(planValue.toObject()));
    }

    return mission;
}

Mission Mission::createDefault() {
    Mission mission;
    // 确保默认任务不包含可能导致问题的Plan
    mission.getPlans().clear();
    return mission;
}
