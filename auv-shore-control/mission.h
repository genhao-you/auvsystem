#pragma once

#include <QObject>
#include"plan.h"
#include"missionnameutil.h"
class Mission  
{
public:
    Mission();
    // 添加拷贝构造函数
    Mission(const Mission& other);
    // 添加赋值运算符
    Mission& operator=(const Mission& other);
    ~Mission();

    QString getId()const;
    void setId(QString id);

    QString getName() const;
    void setName(const QString& name);

    const QList<Plan>& getPlans() const;
    QList<Plan>& getPlans();
    void setPlans(QList<Plan>& plans);

    QJsonObject toJson() const;
    static Mission fromJson(const QJsonObject& json);
    static Mission createDefault();

private:
    QString m_sId;
    QString m_sName;
    QList<Plan> m_listPlans;
};
Q_DECLARE_METATYPE(Mission*)
