#pragma once
#pragma once
#include <QString>
#include "moduleenum.h"
#include "communicationchannel.h"
#include"baseresult.h"

/**
 * @brief 通信通道状态结果类
 * 仅封装通道状态监控数据，不包含其他业务模块信息
 */
class CommunicationChannelStatusResult : public BaseResult {
public:
    // 核心构造函数
    CommunicationChannelStatusResult(CommunicationChannel channel,
        CommunicationChannelStatus channelStatus,
        const QString& errorMsg = "")
        : BaseResult(ModuleType::Communication,
            channelStatus == CommunicationChannelStatus::Running ? ResultStatus::Success
            : ResultStatus::Failed),
        m_channel(channel),
        m_channelStatus(channelStatus),
        m_errorMsg(errorMsg) {
        m_timestamp = QDateTime::currentDateTime();
    }

    // Setter
    void setChannelStatus(CommunicationChannelStatus status) {
        m_channelStatus = status;
        // 同步 BaseResult 的状态
        m_status = (status == CommunicationChannelStatus::Running) ? ResultStatus::Success : ResultStatus::Failed;
    }
    void setErrorMsg(const QString& msg) { m_errorMsg = msg; }

    // Getter
    CommunicationChannel channel() const { return m_channel; }
    CommunicationChannelStatus channelStatus() const { return m_channelStatus; }
    QString errorMsg() const { return m_errorMsg; }

    // 重写描述方法
    QString description() const override {
        QString baseDesc = QString("通信通道状态 - 通道:%1, 状态:%2")
            .arg(static_cast<int>(m_channel))
            .arg(static_cast<int>(m_channelStatus));
        return m_errorMsg.isEmpty() ? baseDesc : baseDesc + QString("，错误:%1").arg(m_errorMsg);
    }

private:
    CommunicationChannel m_channel;                // 通道类型
    CommunicationChannelStatus m_channelStatus;    // 通道具体状态
    QString m_errorMsg;                            // 异常时的错误信息
};