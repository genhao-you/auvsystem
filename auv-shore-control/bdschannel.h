#pragma once

#include <QObject>
#include<QMutex>
#include<QTimer>
#include"communicationchannelbase.h"
#include"BDSconfig.h"
// 短报文结果类型（用于DataMessage中标记结果）
enum class BDSShortMsgResult {
    Success,   // 数据一致
    Failed,    // 数据不一致
    Timeout    // 超时未回复
};
// 短报文数据类型标记（区分AT指令和短报文）
enum class BDSDataType {
    ATCommand,    // AT指令（如查询信号、卡号）
    ShortMessage  // 短报文数据（需校验一致性）
};



class BDSChannel : public CommunicationChannelBase
{
	Q_OBJECT

public:
	explicit BDSChannel(QObject* parent = nullptr);
	~BDSChannel()override;
    void setCurrentSendType(BDSDataType type) {
        QMutexLocker locker(&m_typeMutex);
        m_currentSendType = type;
    }
    bool setConfig(const ChannelConfigPtr& config) override;
    ChannelConfigPtr config() const override;

    bool start() override;
    void stop() override;
    bool isRunning() const override;
    QString lastError()const override;
    QString statusInfo() const override;
    // 发送短报文（对外接口，标记为短报文类型）
    bool sendShortMessage(const QByteArray& data);
signals:
    void sendDataRequestedWithType(const QByteArray& data, BDSDataType type);
private slots:
    void onDataReady();
    void onError(QSerialPort::SerialPortError error);
    void sendDataImpl(const QByteArray& data);
    //带类型标记的发送实现（内部区分AT/短报文）
    void sendDataImplWithType(const QByteArray& data, BDSDataType type);
    void setConfigImpl(const ChannelConfigPtr& config);
    void startImpl();
    void stopImpl();
    // 短报文回复超时处理
    void onShortMessageTimeout();
    void checkPortAvailability(); // 串口可用性检测槽函数
private:
    // 解析AT反馈（无帧，直接封装为DataMessage）
    void parseATResponse();
    QJsonObject parseGNRMC(const QString& gnrmcStr);
    QStringList splitBufferByLine();
    // 提取短报文回复数据（从接收缓冲区中过滤纯短报文内容）
    QByteArray extractShortMessageData(const QByteArray& rawData);
    // 清理短报文发送缓存
    void clearShortMessageCache();
    DataMessage createShortMessageDataMsg(BDSShortMsgResult result, 
        const QByteArray& sentData, const QByteArray& recvData);
private:
    QMutex m_typeMutex;
    BDSDataType m_currentSendType = BDSDataType::ATCommand;
    QSharedPointer<BDSConfig> m_config;
    QSerialPort* m_serialPort;
    QString m_lastError;
    QByteArray m_recvBuffer;//接收缓冲区

      //短报文校验相关成员
    QMutex m_shortMsgMutex;       // 短报文缓存线程锁
    QByteArray m_sentShortMsg;    // 已发送的短报文缓存
    QTimer* m_shortMsgTimer;      // 短报文回复超时定时器
    const int SHORT_MSG_TIMEOUT = 30000; // 短报文超时时间（30秒）

    bool m_isParsingWhitelist = false; // 是否处于白名单解析状态
    QStringList m_whitelistCardCache; // 缓存已收集的白名单卡号
    QMutex m_whitelistMutex; // 白名单缓存互斥锁
     // 白名单指令缓存：记录最近发送的白名单操作类型（Add/Del/Get）
    CommandCode m_lastWhitelistCmd = CommandCode::Invalid;
    // 缓存互斥锁（保证多线程安全，与原有锁分开）
    QMutex m_whitelistCmdMutex;
    QTimer* m_portCheckTimer; // 串口硬件可用性检测定时器

};
