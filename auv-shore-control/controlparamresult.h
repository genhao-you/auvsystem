#pragma once
#include <QDateTime>
#include <QString>
#include<QVariant>
#include "controlparamdefs.h"
#include"baseresult.h"
#include"enumconverter.h"
/**
 * @brief 控制参数执行结果类
 * 封装一次控制参数命令（深度/高度/航向）的完整执行结果，对齐 SelfCheckResult 设计
 */
class ControlParamResult : public BaseResult {
public:
    // 默认构造（对齐 SelfCheckResult 默认构造）
    ControlParamResult()
        : BaseResult(ModuleType::ControlParamDebugging, ResultStatus::Pending),  // 模块类型固定为 ControlParam
        m_controlType(ControlType::DepthControl),
        m_status(ResultStatus::Pending),
        m_packetSeq(0),
        m_timestamp(QDateTime::currentDateTime()) {
    }

    // 带参构造（核心：仅控制类型+状态，无命令码/CommandType）
    ControlParamResult(ControlType controlType, ResultStatus status = ResultStatus::Failed)
        : BaseResult(ModuleType::ControlParamDebugging, status),
        m_controlType(controlType),
        m_status(status),
        m_packetSeq(0),
        m_timestamp(QDateTime::currentDateTime()) {
    }

    // ===== Setters（对齐 SelfCheckResult 风格）=====
    void setControlType(ControlType controlType) { m_controlType = controlType; }
    void setStatus(ResultStatus status) { m_status = status; }
    void setPacketSeq(uint8_t seq) { m_packetSeq = seq; }
    void setFeedbackData(const QByteArray& data) { m_feedbackData = data; }
    void setFeedbackDesc(const QString& desc) { m_feedbackDesc = desc; }
    void setTimestamp(const QDateTime& timestamp) { m_timestamp = timestamp; }
    // 存储自定义参数（键值对，复用自检的参数存储逻辑）
    void setParameter(const QString& key, const QVariant& value) {
        m_parameters[key] = value;
    }

    // ===== Getters（对齐 SelfCheckResult 风格）=====
    ControlType controlType() const { return m_controlType; }
    ResultStatus status() const { return m_status; }
    uint8_t packetSeq() const { return m_packetSeq; }
    QByteArray feedbackData() const { return m_feedbackData; }
    QString feedbackDesc() const { return m_feedbackDesc; }
    QDateTime timestamp() const { return m_timestamp; }
    // 获取自定义参数（默认返回空值）
    QVariant getParameter(const QString& key) const {
        return m_parameters.value(key);
    }

    // 获取控制类型名称（枚举转字符串，对齐 deviceName/commandName）
    QString controlTypeName() const {
        return EnumConverter::controlTypeToString(m_controlType);
    }

    // 获取状态名称（复用自检的枚举转换）
    QString statusString() const {
        return EnumConverter::ResultStatusToString(m_status);
    }

    // 重写描述方法（对齐 SelfCheckResult::description）
    QString description() const override {
        return QString("控制参数结果 - 类型:%1,状态:%2,包序号:%3")
            .arg(static_cast<int>(m_controlType))
            .arg(static_cast<int>(m_status))
            .arg(static_cast<int>(m_packetSeq));
    }

private:
    ControlType m_controlType = ControlType::DepthControl;  // 控制类型（深度/高度/航向）
    ResultStatus m_status = ResultStatus::Pending;            // 执行状态（成功/失败/超时）
    uint8_t m_packetSeq = 0;                                  // 包序号（对齐自检的 packetSeq）
    QByteArray m_feedbackData;                                // 原始反馈数据
    QString m_feedbackDesc;                                   // 人类可读的反馈描述
    QDateTime m_timestamp;                                    // 结果时间戳
    QMap<QString, QVariant> m_parameters;                     // 自定义参数存储（复用自检逻辑）
};

/**
 * @brief 控制参数发送结果类
 * 封装控制参数命令的发送结果（仅外部指令，无 CommandType）
 */
class ControlParamSendResult : public BaseResult {
public:
    // 默认构造
    ControlParamSendResult()
        : BaseResult(ModuleType::ControlParamDebugging, ResultStatus::Pending),
        m_controlType(ControlType::DepthControl),
        m_success(false),
        m_packetSeq(0),
        m_requestId(0),
        m_timestamp(QDateTime::currentDateTime()) {
    }

    // 带参构造（核心：控制类型+发送是否成功+包序号）
    ControlParamSendResult(ControlType controlType, bool success, uint8_t packetSeq = 0)
        : BaseResult(ModuleType::ControlParamDebugging, success ? ResultStatus::Success : ResultStatus::Failed),
        m_controlType(controlType),
        m_success(success),
        m_packetSeq(packetSeq),
        m_requestId(0),
        m_timestamp(QDateTime::currentDateTime()) {
    }

    // ===== Setters =====
    void setControlType(ControlType controlType) { m_controlType = controlType; }
    void setSuccess(bool success) {
        m_success = success;
        // 同步更新 BaseResult 的状态
        setStatus(success ? ResultStatus::Success : ResultStatus::Failed);
    }
    void setPacketSeq(uint8_t seq) { m_packetSeq = seq; }
    void setMessage(const QString& msg) { m_message = msg; }
    void setRequestId(uint64_t id) { m_requestId = id; }
    void setFeedbackDesc(const QString& desc) { m_feedbackDesc = desc; }
    void setTimestamp(const QDateTime& timestamp) { m_timestamp = timestamp; }

    // ===== Getters =====
    ControlType controlType() const { return m_controlType; }
    bool isSuccess() const { return m_success; }
    uint8_t packetSeq() const { return m_packetSeq; }
    QString message() const { return m_message; }
    uint64_t requestId() const { return m_requestId; }
    QString feedbackDesc() const { return m_feedbackDesc; }
    QDateTime timestamp() const { return m_timestamp; }

    // 获取控制类型名称
    QString controlTypeName() const {
        return EnumConverter::controlTypeToString(m_controlType);
    }

    // 重写描述方法
    QString description() const override {
        return QString("控制参数发送结果 - 类型:%1,发送%2,包序号:%3,请求ID:%4")
            .arg(static_cast<int>(m_controlType))
            .arg(m_success ? "成功" : "失败")
            .arg(static_cast<int>(m_packetSeq))
            .arg(m_requestId);
    }

private:
    ControlType m_controlType = ControlType::DepthControl;  // 控制类型
    bool m_success = false;                                   // 发送是否成功
    uint8_t m_packetSeq = 0;                                  // 包序号
    QString m_message;                                        // 发送结果描述
    uint64_t m_requestId = 0;                                 // 全局请求ID（对齐自检的 requestId）
    QString m_feedbackDesc;                                   // 扩展反馈描述
    QDateTime m_timestamp;                                    // 发送时间戳
};