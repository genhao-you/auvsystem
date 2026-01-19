#pragma once

#include <QObject>
#include <QMap>
#include <QList>
#include <memory>
#include <QAtomicInteger>
#include "basemodule.h"
#include "selfcheckdevice.h"
#include "selfcheckresult.h"
#include "selfchecksendresult.h"
#include "observer.h"
#include "frameglobal.h"
#include "internalprotocoladapter.h"
#include "BeidouATAdapter .h"
/**
 * @brief 自检模块核心类
 *
 * 负责管理所有自检器件、发送自检命令、处理自检结果
 * 实现了观察者模式，允许外部对象订阅自检结果
 */
class SelfCheckModule : public BaseModule {
    Q_OBJECT
        struct CommandState
    {
    
        DeviceId deviceId;
        CommandCode cmdCode;
        QTimer* timer;
        bool isProcessed;
        uint8_t packetSeq; 
        QDateTime sendTime;     // 命令发送时间（用于超时清理）
        CommandType cmdType;   //指令类型记录
    };
public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit SelfCheckModule(QObject* parent = nullptr);

    /**
     * @brief 初始化模块
     * @return 初始化是否成功
     */
    bool initialize() override;

    /**
     * @brief 清理模块资源
     */
    void shutdown() override;

    /**
     * @brief 获取所有器件
     * @return 器件列表
     */
    const QList<SelfCheckDevice>& getAllDevices() const { return m_devices; }

    /**
     * @brief 根据ID获取器件
     * @param deviceId 器件ID
     * @return 器件指针（如果存在）
     */
    const SelfCheckDevice* getDevice(DeviceId deviceId) const;

    /**
     * @brief 发送自检命令
     * @param deviceId 器件ID
     * @param cmdCode 命令编码
     * @param param 命令参数
     * @return 操作结果
     */
    CheckError sendCheckCommand(DeviceId deviceId,
        CommandCode cmdCode, 
        const std::shared_ptr<CommandParameter>& param,
        WorkPhase parse = WorkPhase::DeviceTest,
        CommunicationChannel channel = CommunicationChannel::Radio,
        CommandType cmdType = CommandType::External);

    //封装北斗通信测试指令所需参数
    std::shared_ptr<InternalCommandParam>createBeidouSNDBParam(uint8_t packetSeq,
        DeviceId deviceId, CommandCode cmdCode,
        const std::shared_ptr<CommandParameter>& extParam);
    // ：获取全局唯一的packetSeq（供外部提前获取，用于帧构建）
    uint8_t getUniquePacketSeq();
private slots:
    /**
     * @brief 处理接收到的消息
     * @param data 消息数据
     */
    void onCheckResultReceived(const DataMessage& data);

    /*
    * 
    */
    void  onSendResultReceived(const DataMessage& data);
    /**
     * @brief 处理命令超时
     * @param packetSeq 超时的包序号
     */
    void onCommandTimeout(DeviceId deviceId,CommandCode cmdCode, uint8_t packetSeq,CommandType type);

    //定期清理残留命令状态
    void cleanupStaleStates();


private:
    /**
     * @brief 初始化器件列表
     */
    void initializeDevices();

    /**
     * @brief 处理自检结果
     * @param frame 解析后的帧
     */
   // void processCheckResult(const Frame& frame);

    void parseSelfcheckResult(const QJsonObject& resultJson);

    void parseBDSAtResponse(const QJsonObject& bdsJson);

    void parseBDSShortMessageResponse(const QJsonObject& bdsJson);

    void parseWaterAcousticResponse(const QJsonObject& waJson);
    /**
     * @brief 生成下一个包序号
     * @return 包序号（1-256循环）
     */
    uint8_t getNextPacketSeq();

    int getDynamicTimeoutMs(DeviceId deviceId, CommandCode cmdCode);

    // 创建舵设备的辅助函数
    void createRudderDevice(DeviceId id, const QString& name);
    
    // 通用状态清理函数
    void cleanupCommandState(uint8_t packetSeq);
   

    bool parseGNRMC(const QString& gnrmcStr,
        double& latitudeDegree,  // 十进制纬度（引用传出）
        double& longitudeDegree, // 十进制经度（引用传出）
        bool& isValidFix,        // 是否有效定位（引用传出）
        QString& fixStatusDesc);
    QStringList splitBufferByLine(QByteArray recevBuffer);
    // 辅助函数：生成内部指令的状态键
    QString generateInternalCmdKey(DeviceId deviceId, CommandCode cmdCode);
private:
    QList<SelfCheckDevice> m_devices;  // 所有自检器件

    QHash<uint8_t, QPointer<QTimer>> m_cmdTimerMap; // 按packetSeq映射定时器
    QHash<uint8_t, CommandState> m_commandStates;
    // ：按「设备ID+命令码+指令类型」存储（内部指令，无packetSeq场景）
    QHash<QString, CommandState> m_internalCmdStates;

    QList<IResultObserver*> m_observers;  // 结果观察者列表
    QMutex m_mutex;
    QAtomicInteger<uint8_t> m_nextPacketSeq = 1; // 原子变量，避免多线程竞争
    QTimer* m_cleanupTimer{ nullptr };                     // ：定期清理定时器
};
