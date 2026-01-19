#pragma once
#include "dataresult.h"
#include "communicationchannel.h"

class FtpDeleteResult : public DataResult {
public:
    enum class DeleteType { File = 0, Directory = 1 };

    FtpDeleteResult(CommunicationChannel channel,
        DeleteType type,
        ResultStatus status,
        uint64_t requestId = 0,
        const QString& remotePath = "",
        const QString& errorMsg = "")
        : DataResult(ModuleType::Data, status),
        m_channel(channel),
        m_deleteType(type),
        m_requestId(requestId),
        m_remotePath(remotePath),
        m_errorMsg(errorMsg) {
        m_timestamp = QDateTime::currentDateTime();
    }

    // Getter
    CommunicationChannel channel() const { return m_channel; }
    DeleteType deleteType() const { return m_deleteType; }
    uint64_t requestId() const { return m_requestId; }
    QString remotePath() const { return m_remotePath; }
    QString errorMsg() const { return m_errorMsg; }

    QString description() const override {
        QString typeStr = (m_deleteType == DeleteType::File) ? "文件" : "目录";
        QString statusStr = (status() == ResultStatus::Success) ? "成功" : "失败";
        return QString("FTP删除%1[请求%2]：%3（%4）")
            .arg(typeStr).arg(m_requestId)
            .arg(m_remotePath).arg(statusStr) +
            (m_errorMsg.isEmpty() ? "" : QString("，错误：%1").arg(m_errorMsg));
    }

private:
    CommunicationChannel m_channel = CommunicationChannel::Unknown;
    DeleteType m_deleteType = DeleteType::File;
    uint64_t m_requestId = 0;
    QString m_remotePath;
    QString m_errorMsg;
};