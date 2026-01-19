#pragma once
#include "dataresult.h"
#include "communicationchannel.h"
#include <QString>

/**
 * @brief FTP文件传输结果类
 * 封装文件上传/下载操作结果
 */
class FtpTransferResult : public DataResult {
public:
    // 传输类型枚举
    enum class TransferType {
        Upload,     // 上传
        Download    // 下载
    };

    // 构造函数
    FtpTransferResult(CommunicationChannel channel,
        TransferType transferType,
        bool success,
        const QString& errorMsg = "")
        : DataResult(ModuleType::Data, success ? ResultStatus::Success : ResultStatus::Failed),
        m_channel(channel),
        m_transferType(transferType),
        m_errorMsg(errorMsg) {
        m_timestamp = QDateTime::currentDateTime();
    }

    // Setter方法
    void setSuccess(bool success) {
        setStatus(success ? ResultStatus::Success : ResultStatus::Failed);
    }
    void setErrorMsg(const QString& msg) { m_errorMsg = msg; }

    // Getter方法
    CommunicationChannel channel() const { return m_channel; }
    TransferType transferType() const { return m_transferType; }
    QString errorMsg() const { return m_errorMsg; }

    // 重写描述方法
    QString description() const override {
        QString typeStr = (m_transferType == TransferType::Upload) ? "上传" : "下载";
        QString baseDesc = QString("FTP%1结果 - 通道:%1, 本地:%2, 远程:%3")
            .arg(typeStr)
            .arg(static_cast<int>(m_channel));

        if (status() == ResultStatus::Success) {
            baseDesc += "，状态:成功";
        }
        else {
            baseDesc += QString("，状态:失败，错误:%1").arg(m_errorMsg);
        }

        return baseDesc;
    }

private:
    CommunicationChannel m_channel;      // 所属通信通道
    TransferType m_transferType;         // 传输类型（上传/下载）
    QString m_errorMsg;                  // 错误信息
};