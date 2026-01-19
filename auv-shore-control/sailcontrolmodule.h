#pragma once
#include <QObject>
#include <QTimer>
#include <QHash>
#include <QMutex>
#include "basemodule.h"
#include "sailcontroldefs.h"
#include "observer.h"
#include "frameglobal.h"

class SailControlModule  : public BaseModule
{
    Q_OBJECT

public:
    explicit SailControlModule(QObject* parent = nullptr);
    ~SailControlModule() override;

    /**
     * @brief 初始化模块
     */
    bool initialize() override;

    /**
     * @brief 清理模块资源
     */
    void shutdown() override;

    /**
     * @brief 发送航行控制命令
     * @param cmdCode 命令码（开始/停止）
     * @param channel 通信信道（默认串口）
     * @return 操作结果
     */
    CheckError sendSailCommand(SailCommandCode cmdCode,
        CommunicationChannel channel = CommunicationChannel::Radio);

    /**
     * @brief 获取当前航行状态
     */
    bool isSailing() const { return m_isSailing; }

signals:
    //void sailStatusChanged(bool isSailing);
    //void commandResult(SailCommandCode cmdCode, bool success, const QString& message);

private:
    // 命令状态结构体
    struct CommandState {
        SailCommandCode cmdCode;        // 命令码
        QTimer* timer;                  // 超时定时器
        bool isProcessed;               // 是否已处理
        uint8_t packetSeq;              // 包序号
        QDateTime sendTime;             // 发送时间
        QAtomicInt refCount;            // 引用计数
    };

private slots:
    /**
     * @brief 处理航行控制结果
     */
    void onSailResultReceived(const DataMessage& data);

    /**
     * @brief 处理发送结果
     */
    void onSendResultReceived(const DataMessage& data);

    /**
     * @brief 处理命令超时
     */
    void onCommandTimeout(SailCommandCode cmdCode, uint8_t packetSeq);

    /**
     * @brief 定期清理残留命令状态
     */
    void cleanupStaleStates();

private:
    /**
     * @brief 生成下一个包序号
     */
    uint8_t getNextPacketSeq();

    /**
     * @brief 获取动态超时时间
     */
    int getDynamicTimeoutMs(SailCommandCode cmdCode);

    /**
     * @brief 解析航行控制结果
     */
    void parseSailResult(const QJsonObject& resultJson);

    /**
     * @brief 通用状态清理函数
     */
    void cleanupCommandState(uint8_t packetSeq);

    /**
     * @brief 更新航行状态
     */
    void updateSailStatus(bool isSailing);

private:
    QHash<uint8_t, QPointer<QTimer>> m_cmdTimerMap;     // 定时器映射
    QHash<uint8_t, CommandState> m_commandStates;       // 命令状态
    QMutex m_mutex;                                     // 线程安全锁
    QAtomicInteger<uint8_t> m_nextPacketSeq = 1;        // 包序号原子变量
    QTimer* m_cleanupTimer{ nullptr };                  // 清理定时器
    bool m_isSailing = false;                           // 当前航行状态
};
