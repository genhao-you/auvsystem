#pragma once
#include"datafilter.h"
#include <QObject>
#include<QMutex>
#include"datamessage.h"
#include<QList>
#include<QQueue>
#include<memory>
// 时间戳处理器
class TimestampProcessor  : public QObject
{
	Q_OBJECT

public:
    explicit TimestampProcessor(QObject* parent = nullptr);

    // 设置时间窗口（毫秒）
    void setTimeWindow(int windowMs);

    // 添加数据过滤器
    void addFilter(std::shared_ptr<DataFilter> filter);

    // 处理接收到的数据，添加时间戳并排序
    void processData(const DataMessage& message);

signals:
    void dataProcessed(const DataMessage& message);

private:
    QMutex m_mutex;
    QQueue<DataMessage> m_dataQueue;
    int m_timeWindow = 30000; // 默认30秒时间窗口
    QList<std::shared_ptr<DataFilter>> m_filters;
};
