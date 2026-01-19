#pragma once

#include <QObject>
#include<QMap>
#include<QJsonObject>
#include<QVariant>
#include<mutex>
#include"missionenum.h"
#include"missionnameutil.h"
class Task
{
public:
   
	
	Task(const QString& planId,TaskType type ,NavigationMode navMode = NavigationMode::DepthKeeping);
	Task(const Task& other); // 声明拷贝构造函数
	~Task(); 

	// 声明赋值运算符
	Task& operator=(const Task& other);
    // 任务调整操作枚举（与协议对应）
    enum TaskAdjustOp {
        Add = 0x01,    // 增添任务
        Replace = 0x02,// 替换任务
        Delete = 0x03  // 删除任务
    };

    // 设置/获取任务调整操作
    void setTaskAdjust(TaskAdjustOp op);
    TaskAdjustOp getTaskAdjust() const;

	QString getId()const;
	void setId(QString id);

	QString getPlanId()const;
	void setPlanId(QString id);

	 TaskType getTaskType()const ;
	void setTaskType(TaskType type);

	QMap<QString, QVariant> getParams();
	void setParam(const QString& key, const QVariant& value);
	QVariant getParam(const QString& key);

	

	QString getTaskName()const;
	void setTaskName(QString name);

    // ---------------------- 原有字段（保留并完善）----------------------
   // 设备号（6字节string，前3字节型号+后3字节序号）
    QString getDeviceId() const;
    void setDeviceId(const QString& deviceId); // 校验：必须6字节

    // 任务序号（8字节string，年月日序数，如"25091101"）
    QString getTaskNum() const;
    void setTaskNum(const QString& taskNum); // 校验：必须8字节

    // 航行模式（1=定深航行，2=定高航行）
    NavigationMode getNavigationMode() const;
    void setNavigationMode(NavigationMode mode);

    // 起点（经/纬/深，double，保留小数点后5位）
    QVariantMap getStartPoint() const; // key: "longitude"/"latitude"/"depth"
    void setStartPoint(const QVariantMap& startPoint); // 精度校验

    // 终点（经/纬/深，double，保留小数点后5位）
    QVariantMap getEndPoint() const;
    void setEndPoint(const QVariantMap& endPoint); // 精度校验

    // 转速（int16，0~1500rpm，覆盖表格“最高1500rpm”约束）
    int16_t getRotateSpeed() const;
    void setRotateSpeed(int16_t rotateSpeed);

    // 速度（int16，实际值×10取整，0~3276.7kn）
    int16_t getSpeed() const;
    void setSpeed(double speedKn); // 入参为实际速度（kn），内部×10取整

    // 超时时间（int32，单位秒，0~24855天）
    int32_t getTimeOutPeriod() const;
    void setTimeoutPeriod(int32_t timeOut);

    // 危险深度（int16，0~32767米）
    int16_t getDangerDepth() const;
    void setDangerDepth(int16_t dangerDepth);

    // 是否上浮（bool，1=上浮，0=不上浮）
    bool getNeedFloat() const;
    void setNeedFloat(bool needFloat);

    // ---------------------- 字段（表格补充）----------------------
    // 危险高度（int16，0~32767米，定高航行时有效）
    int16_t getDangerHeight() const;
    void setDangerHeight(int16_t dangerHeight);

    // 障碍物距离（int16，0~32767米，传感器探测0.5~450米）
    int16_t getObstacleDistance() const;
    void setObstacleDistance(int16_t obstacleDistance);

    // 危险电压（int16，0~32767V）
    int16_t getDangerVoltage() const;
    void setDangerVoltage(int16_t dangerVoltage);

    // 危险电流（int16，实际值×1000取整，0~65.535A）
    float getDangerCurrent() const;
    void setDangerCurrent(float currentA); 

    QJsonObject toJson() const;
    static Task fromJson(const QJsonObject& json);
private:
	QString m_sPlanId;
	QString m_sId;
	QString m_sName;
	TaskType m_type;
	QMap<QString, QVariant> m_params;

	// 协议必填字段
	QString m_sDeviceId;       // 设备号（6字节string，如"021001"）
    TaskAdjustOp m_taskAdjust; // 任务调整操作（默认0x00，无效值）
	QString m_sTaskNum;        // 任务序号（8字节string，如"25091101"）
	QVariantMap m_startPoint;  // 起点（longitude:double, latitude:double, depth:double）
	QVariantMap m_endPoint;    // 终点（同起点）
	int16_t m_iRotateSpeed;    // 转速（0~32767 rpm）
	int16_t m_iSpeed;          // 速度（0~3276.7 kn，精度0.1）
	int32_t m_iTimeOutPeriod;  // 超时时间（秒，0~24855天）
    int16_t m_iDangerDepth;      // 危险深度
    int16_t m_iDangerHeight;     // 危险高度
    int16_t m_iObstacleDistance; // 障碍物距离
    int16_t m_iDangerVoltage;    // 危险电压
    float m_fDangerCurrent;    // 危险电流
    bool m_bNeedFloat;           // 是否上浮
    NavigationMode m_navMode;
};
Q_DECLARE_METATYPE(Task*)