#pragma once
#include "dataresult.h"
#include "communicationchannel.h"
#include <QStringList>

/**
 * @brief FTP登录+文件列表整合结果类
 * 封装FTP登录结果和文件列表获取结果
 */
class FtpLoginListResult : public DataResult {
public:
    // 完整构造函数
    FtpLoginListResult(CommunicationChannel channel,
        bool loginSuccess,
        bool listSuccess,
        const QStringList& fileList = {},
        const QString& errorMsg = "")
        : DataResult(ModuleType::Data,
            (loginSuccess&& listSuccess) ? ResultStatus::Success : ResultStatus::Failed),
        m_channel(channel),
        m_loginSuccess(loginSuccess),
        m_listSuccess(listSuccess),
        m_fileList(fileList),
        m_errorMsg(errorMsg) {
        m_timestamp = QDateTime::currentDateTime();
    }

    // Setter方法
    void setLoginSuccess(bool success) { m_loginSuccess = success; updateStatus(); }
    void setListSuccess(bool success) { m_listSuccess = success; updateStatus(); }
    void setFileList(const QStringList& files) { m_fileList = files; }
    void setErrorMsg(const QString& msg) { m_errorMsg = msg; }

    // Getter方法
    CommunicationChannel channel() const { return m_channel; }
    bool loginSuccess() const { return m_loginSuccess; }
    bool listSuccess() const { return m_listSuccess; }
    QStringList fileList() const { return m_fileList; }
    QString errorMsg() const { return m_errorMsg; }

    // 重写描述方法
    QString description() const override {
        QString baseDesc = QString("FTP操作结果 - 通道:%1, 登录:%2, 列表:%3")
            .arg(static_cast<int>(m_channel))
            .arg(m_loginSuccess ? "成功" : "失败")
            .arg(m_listSuccess ? "成功" : "失败");

        if (!m_errorMsg.isEmpty()) {
            baseDesc += QString("，错误:%1").arg(m_errorMsg);
        }
        else if (m_listSuccess) {
            baseDesc += QString("，文件数:%1").arg(m_fileList.size());
        }

        return baseDesc;
    }

private:
    // 更新结果状态
    void updateStatus() {
        if (m_loginSuccess && m_listSuccess) {
            setStatus(ResultStatus::Success);
        }
        else if (!m_loginSuccess) {
            setStatus(ResultStatus::Failed);
        }
        else {
            setStatus(ResultStatus::Failed);
        }
    }

    CommunicationChannel m_channel;      // 所属通信通道
    bool m_loginSuccess = false;         // 登录是否成功
    bool m_listSuccess = false;          // 文件列表获取是否成功
    QStringList m_fileList;              // 获取到的文件列表
    QString m_errorMsg;                  // 错误信息
};