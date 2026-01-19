#include "timestampprocessor.h"

TimestampProcessor::TimestampProcessor(QObject* parent) : QObject(parent) {}

void TimestampProcessor::setTimeWindow(int windowMs) {
    m_timeWindow = windowMs;
}

void TimestampProcessor::addFilter(std::shared_ptr<DataFilter> filter) {
    m_filters.append(filter);
}

void TimestampProcessor::processData(const DataMessage& message) {
    QMutexLocker locker(&m_mutex);

    // 应用过滤器
    for (const auto& filter : m_filters) {
        if (!filter->filter(message)) {
            // 数据被过滤
            return;
        }
    }

    // 将数据添加到队列
    m_dataQueue.enqueue(message);

    // 按时间戳排序（最新的在前）
    std::sort(m_dataQueue.begin(), m_dataQueue.end(),
        [](const DataMessage& a, const DataMessage& b) {
            return a.timestamp() > b.timestamp();
        });

    // 移除超时的数据
    QDateTime cutoffTime = QDateTime::currentDateTime().addMSecs(-m_timeWindow);
    while (!m_dataQueue.isEmpty() && m_dataQueue.last().timestamp() < cutoffTime) {
        m_dataQueue.dequeue();
    }

    emit dataProcessed(message);
}
