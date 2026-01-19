#pragma once
#include <QObject>
#include "basemodule.h"
#include "controlparamdefs.h"
#include "observer.h"
#include "frameglobal.h"
#include "commandparameter.h"
class ControlParamModule : public BaseModule
{
	Q_OBJECT

        // 命令状态结构体（移除cmdCode字段）
        struct CommandState
    {
        ControlType controlType;       // 仅保留控制类型（深度/高度/航向）
        QTimer* timer;
        bool isProcessed;
        uint8_t packetSeq;
        QDateTime sendTime;            // 命令发送时间（用于超时清理）
    };
public:
    explicit ControlParamModule(QObject* parent = nullptr);
    ~ControlParamModule() override;

    /**
    * @brief 初始化模块（对齐SelfCheckModule::initialize）
    */
    bool initialize() override;

    /**
     * @brief 清理模块资源（对齐SelfCheckModule::shutdown）
     */
    void shutdown() override;

    /**
      * @brief 发送控制参数命令（仅外部指令，无CommandType参数）
      * @param controlType 控制类型（深度/高度/航向）
      * @param param 控制参数结构体
      * @param phase 使用阶段（默认任务指令）
      * @param channel 通信信道（默认无线电）
      * @return 操作结果
      */
    CheckError sendControlParamCommand(ControlType controlType,
        const std::shared_ptr<ControlParamParameter>& param,
        WorkPhase phase = WorkPhase::TaskCommand,
        CommunicationChannel channel = CommunicationChannel::Radio);

    /**
     * @brief 获取全局唯一的packetSeq
     */
    uint8_t getUniquePacketSeq();

private slots:
    /**
     * @brief 处理控制参数结果
     */
    void onControlResultReceived(const DataMessage& data);

    /**
     * @brief 处理发送结果
     */
    void onSendResultReceived(const DataMessage& data);

    /**
     * @brief 处理命令超时（移除cmdCode参数）
     */
    void onCommandTimeout(ControlType controlType, uint8_t packetSeq);

    /**
     * @brief 定期清理残留命令状态（对齐SelfCheckModule::cleanupStaleStates）
     */
    void cleanupStaleStates();

private:
    /**
     * @brief 生成下一个包序号
     */
    uint8_t getNextPacketSeq();

    /**
     * @brief 获取动态超时时间（仅按控制类型判断）
     */
    int getDynamicTimeoutMs(ControlType controlType);

    /**
     * @brief 解析控制参数结果（移除cmdCode相关）
     */
    void parseControlParamResult(const QJsonObject& resultJson);

    /**
     * @brief 通用状态清理函数（对齐SelfCheckModule::cleanupCommandState）
     */
    void cleanupCommandState(uint8_t packetSeq);


private:
    QHash<uint8_t, QPointer<QTimer>> m_cmdTimerMap; // 按packetSeq映射定时器（复用）
    QHash<uint8_t, CommandState> m_commandStates;   // 外部指令状态（按packetSeq）
    QHash<QString, CommandState> m_internalCmdStates; // 内部指令状态（按key）
    QList<IResultObserver*> m_observers;            // 结果观察者列表（复用）
    QMutex m_mutex;                                 // 线程安全锁（复用）
    QAtomicInteger<uint8_t> m_nextPacketSeq = 1;    // 包序号原子变量（复用）
    QTimer* m_cleanupTimer{ nullptr };              // 定期清理定时器（复用）

};
