#pragma once
#include "dataresult.h"
#include "communicationchannel.h"
#include <QDateTime>

/**
 * @brief FTP文件传输进度类
 * 封装文件上传/下载进度结果
 */
class FtpProgressResult : public DataResult { 
public:
    // 进度类型（上传/下载）- 内部枚举，保持代码内聚性
    enum class ProgressType {
        Upload,   // 上传
        Download  // 下载
    };

    FtpProgressResult(CommunicationChannel channel,
        ProgressType type,
        int progress = 0)  // 进度默认值0，提升易用性
        : DataResult(ModuleType::Data,  
            ResultStatus::Pending),  // 进度状态固定为Pending
        m_channel(channel),
        m_type(type),
        m_progress(progress) {
        m_timestamp = QDateTime::currentDateTime();
    }

    // Setter方法
    void setChannel(CommunicationChannel channel) { m_channel = channel; }
    void setProgressType(ProgressType type) { m_type = type; }
    void setProgress(int progress) {
        m_progress = progress;
        // 进度值限制在0-100，避免非法值
        m_progress = qBound(0, m_progress, 100);
        updateStatus(); 
    }

    // Getter方法
    CommunicationChannel channel() const { return m_channel; }
    ProgressType type() const { return m_type; }
    int progress() const { return m_progress; }

    QString description() const override {
        QString typeStr = (m_type == ProgressType::Upload) ? "上传" : "下载";
        QString baseDesc = QString("FTP%1进度结果 - 通道:%2, 进度:%3%")
            .arg(typeStr)
            .arg(static_cast<int>(m_channel))
            .arg(m_progress);

        return baseDesc;
    }

private:
 
    void updateStatus() {
        // 进度状态始终为Pending（进度完成由TransferResult标识，此处仅更新进度值）
        setStatus(ResultStatus::Pending);
    }

    // 成员变量
    CommunicationChannel m_channel = CommunicationChannel::Unknown;  // 初始化默认值，避免野值
    ProgressType m_type = ProgressType::Upload;                      // 默认上传类型
    int m_progress = 0;                                              // 进度默认0
};
