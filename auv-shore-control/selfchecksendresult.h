#pragma once
#include <QDateTime>
#include <QString>
#include <QVariant>
#include "selfcheckenum.h"
#include "baseresult.h"

/**
 * @brief 自检命令发送结果类
 * 专门封装命令发送阶段的结果信息
 */
class SelfCheckSendResult : public BaseResult {
public:
    SelfCheckSendResult() = default;

    SelfCheckSendResult(DeviceId deviceId, CommandCode commandCode, bool success, uint8_t packetSeq = 0)
        : BaseResult(ModuleType::SelfCheck, success ? ResultStatus::Success : ResultStatus::Failed),
        m_deviceId(deviceId),
        m_commandCode(commandCode),
        m_success(success),
        m_packetSeq(packetSeq) {
        m_timestamp = QDateTime::currentDateTime();
    }

    // Setters
    void setDeviceId(DeviceId deviceId) { m_deviceId = deviceId; }
    void setCommandCode(CommandCode commandCode) { m_commandCode = commandCode; }
    void setSuccess(bool success) {
        m_success = success;
        setStatus(success ? ResultStatus::Success : ResultStatus::Failed);
    }
    void setMessage(const QString& message) { m_message = message; }
    void setPacketSeq(uint8_t seq) { m_packetSeq = seq; }
    void setRequestId(uint64_t id) { m_requestId = id; }
    void setFeedbackDesc(const QString& desc) { m_feedbackDesc = desc; }
    // Getters
    DeviceId deviceId() const { return m_deviceId; }
    CommandCode commandCode() const { return m_commandCode; }
    bool isSuccess() const { return m_success; }
    QString message() const { return m_message; }
    uint8_t packetSeq() const { return m_packetSeq; }
    uint64_t requestId() const { return m_requestId; }
    QString feedbackDesc() const { return m_feedbackDesc; }
    // 获取器件名称
    QString deviceName() const {
        return EnumConverter::deviceIdToString(m_deviceId);
    }

    // 获取命令名称
    QString commandName() const {
        return EnumConverter::commandCodeToString(m_deviceId, m_commandCode);
    }

    QString description() const override {
        return QString("发送结果 - 器件:%1,命令:%2,包序号:%3,状态:%4")
            .arg(deviceName())
            .arg(commandName())
            .arg(static_cast<int>(m_packetSeq))
            .arg(m_success ? "成功" : "失败");
    }

private:
    DeviceId m_deviceId = DeviceId::Unknown;      // 器件ID
    CommandCode m_commandCode = static_cast<CommandCode>(0);  // 命令编码
    bool m_success = false;                       // 发送是否成功
    QString m_message;                            // 发送结果描述信息
    uint8_t m_packetSeq = 0;                      // 包序号
    uint64_t m_requestId = 0;                     // 请求ID
    QDateTime m_timestamp;                        // 发送时间戳
    QString m_feedbackDesc;                       // 反馈描述（人类可读）
};