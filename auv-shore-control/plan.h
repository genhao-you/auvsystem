#pragma once

#include <QObject>
#include"task.h"
#include<QList>
#include"missionnameutil.h"

class Plan
{
public:
	Plan();
	Plan(const QString& waypointId, PlanMode mode);
	Plan(const Plan& other); // 声明拷贝构造函数
	~Plan();

	// 声明赋值运算符
	Plan& operator=(const Plan& other);

	QString getId()const;
	void setId(QString id);

	QString getMissionId()const;
	void setMissionId(QString id);

	QString getPlanName() const;
	void setPlanName(QString name);

	PlanMode getPlanMode() const;
	void setPlanMode(PlanMode mode);

	const QList<Task>& getTasks()const;
	QList<Task>& getTasks();
	
	double getDistance();
	void setDistance(double distance);

	double getEstimatedTime();
	void setEstimatedTime(double time);

	double getMinDepth();
	void setMinDepth(double depth);

	double getMaxDepth();
	void setMaxDepth(double depth);

	int getTaskCount();
	void setTaskCount(int count);

	void updateMetrics();
	QJsonObject toJson() const;
	static Plan fromJson(const QJsonObject& json);
	void generateTasks();
private:
	QString m_sPlanId;
	QString m_sMissionId;
	QString m_sPlanName;
	PlanMode m_planMode;
	QList<Task> m_listTasks;
	double m_dDistance;
	double m_dEstimatedTime;
	double m_dMinDepth;
	double m_dMaxDepth;
	int m_iTaskCount;
};
Q_DECLARE_METATYPE(Plan*)