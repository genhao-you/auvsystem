#pragma once
#include <QDateTime>
#include <QString>
#include<QVariant>
#include "selfcheckenum.h"
#include"baseresult.h"
#include"enumconverter.h"
/**
 * @brief 自检结果类
 *
 * 封装一次自检的完整结果信息
 */
class SelfCheckResult :public BaseResult{
public:
    SelfCheckResult()
        : BaseResult(ModuleType::SelfCheck, ResultStatus::Pending), // 父类无默认构造，必须显式传参
        m_deviceId(DeviceId::Unknown),
        m_commandCode(static_cast<CommandCode>(0)),
        m_status(ResultStatus::Pending),
        m_commandType(CommandType::External), // 给默认值
        m_timestamp(QDateTime::currentDateTime()) {
    }
    SelfCheckResult(DeviceId deviceId, CommandCode commandCode, 
        ResultStatus status =  ResultStatus::Failed,CommandType type= CommandType::External)
        : BaseResult(ModuleType::SelfCheck,status), m_deviceId(deviceId),
        m_commandCode(commandCode), m_status(status) ,m_commandType(type){
        m_timestamp = QDateTime::currentDateTime();
    }

    // Setters
    void setDeviceId(DeviceId deviceId) { m_deviceId = deviceId; }
    void setCommandCode(CommandCode commandCode) { m_commandCode = commandCode; }
    void setStatus(ResultStatus status) { m_status = status; }
    void setFeedbackData(const QByteArray& data) { m_feedbackData = data; }
    void setFeedbackDesc(const QString& desc) { m_feedbackDesc = desc; }
    void setTimestamp(const QDateTime& timestamp) { m_timestamp = timestamp; }
    // 存储具体参数（键值对形式，支持多种类型）
    void setParameter(const QString& key, const QVariant& value) {
        m_parameters[key] = value;
    }
    void setCommandType(CommandType type) { m_commandType = type; }

    // Getters
    DeviceId deviceId() const { return m_deviceId; }
    CommandCode commandCode() const { return m_commandCode; }
    ResultStatus status() const { return m_status; }
    QByteArray feedbackData() const { return m_feedbackData; }
    QString feedbackDesc() const { return m_feedbackDesc; }
    QDateTime timestamp() const { return m_timestamp; }
    CommandType commandType()const { return m_commandType; }
    // 获取器件名称（通过枚举转换）
    QString deviceName() const {
        return EnumConverter::deviceIdToString(m_deviceId);
    }

    // 获取命令名称（通过枚举转换）
    QString commandName() const {
        return EnumConverter::commandCodeToString(m_deviceId,m_commandCode);
    }

    // 获取状态名称（通过枚举转换）
    QString statusString() const {
        return EnumConverter::ResultStatusToString(m_status);
    }

    QString description()const override {
        return QString("自检结果 - 器件:%1,命令:%2,状态:%3")
            .arg(static_cast<int>(m_deviceId))
            .arg(static_cast<int>(m_commandCode))
            .arg(static_cast<int>(m_status));
    }
    // 获取具体参数（默认返回空值）
    QVariant getParameter(const QString& key) const {
        return m_parameters.value(key);
    }
private:
    DeviceId m_deviceId = DeviceId::Unknown;      // 器件ID
    CommandCode m_commandCode = static_cast<CommandCode>(0);  // 命令编码
    ResultStatus m_status = ResultStatus::Pending;  // 自检状态
    QByteArray m_feedbackData;                    // 原始反馈数据
    QString m_feedbackDesc;                       // 反馈描述（人类可读）
    QDateTime m_timestamp;                        // 自检时间戳
    QMap<QString, QVariant> m_parameters; // ：存储具体参数
    CommandType m_commandType;
};

