#pragma once
#include <QSerialPort>
#include <QDateTime>
#include<QMutex>
#include<QTimer>
#include "communicationchannelbase.h"
#include "wateracousticconfig.h"
#include"selfcheckenum.h"
// 水声数据校验结果
enum class WaterAcousticResult {
    Success,   // 数据一致
    Failed,    // 数据不一致
    Pending,   //等待中
    Timeout    // 超时未回复
};
class WaterAcousticChannel : public CommunicationChannelBase
{
	Q_OBJECT
public:
    explicit WaterAcousticChannel(QObject* parent = nullptr);
    ~WaterAcousticChannel() override;

    // 重写基类纯虚函数
    bool setConfig(const ChannelConfigPtr& config) override;
    ChannelConfigPtr config() const override;
    bool start() override;
    void stop() override;
    bool isRunning() const override;
    QString lastError() const override;
    QString statusInfo() const override;

    // 辅助函数：指令类型转字符串
    static QString waUplinkCmdToString(WaterAcousticUplinkCmd cmd);
private slots:
    // 串口信号槽
    void onDataReady();
    void onError(QSerialPort::SerialPortError error);

    // 业务逻辑槽（对应基类信号）
    void sendDataImpl(const QByteArray& data);
    void setConfigImpl(const ChannelConfigPtr& config);
    void startImpl();
    void stopImpl();
    // 水声校验超时处理
    void onWaDataTimeout();
    void onRecvNotifyTimeout();
    void checkPortAvailability(); // 串口可用性检测槽函数
private:
    void parseFrames(); // 解析水声帧
    WaterAcousticUplinkCmd parseWaterAcousticUplinkCmd(const QByteArray& content);
    void clearWaDataCache(); // 清理发送数据缓存
       // 数据校验结果消息（含发送/接收数据）
    DataMessage createWaDataMsg(WaterAcousticResult result, WaterAcousticUplinkCmd cmd,
        const QByteArray& sentData, const QByteArray& recvData);

    // 状态通知消息（含状态描述，恢复该函数）
    DataMessage createWaStatusMsg(const QString& statusDesc, WaterAcousticResult result,
        WaterAcousticUplinkCmd cmd);
private:
    struct SentFrameField {
        uint8_t packetSeq = 0;            // 包序号（默认值）
        uint8_t channel = 0;              // 通信信道（默认值）
        uint16_t senderDevice = 0;        // 发送设备（默认值）
        uint16_t receiverDevice = 0;      // 接收设备（默认值）
        uint8_t phase = 0;                // 使用阶段（默认值）
        QByteArray dataBody;              // 业务数据体
        QByteArray fullWaterFrame;        // 完整发送的水声帧
        // 清空函数（替代列表初始化）
        void clear() {
            packetSeq = 0;
            channel = 0;
            senderDevice = 0;
            receiverDevice = 0;
            phase = 0;
            dataBody.clear();
            fullWaterFrame.clear();
        }
    };
    WaterAcousticConfigPtr m_config;       // 水声配置
    QSerialPort* m_serialPort = nullptr;             // 串口对象
    QString m_lastError;                   // 最后错误信息
    QByteArray m_recvBuffer;               // 接收缓冲区
    QDateTime m_lastSendTime;              // 上次发送时间（控制3s间隔）

    QMutex m_waMutex{ QMutex::Recursive }; // 显式初始化为递归锁

    SentFrameField m_sentFrameField;                // 缓存发送的水声核心数据
    QTimer* m_waTimer;                     // 超时定时器
    const int WA_TIMEOUT_MS = 30000;       // 水声超时时间（30秒，可配置）

    bool m_hasRecvNotify = false;               // 标记是否收到“新数据通知”指令
    QTimer* m_recvNotifyTimer = nullptr;         // “新数据通知”后的超时定时器
    const int RECV_NOTIFY_TIMEOUT_MS = 10000;    // 新数据校验超时时间（10秒）
    QTimer* m_portCheckTimer; // 串口硬件可用性检测定时器
};
