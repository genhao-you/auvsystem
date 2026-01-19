#pragma once
#include <QVariantMap>
#include <QStringList>
#include <QObject>
class FtpSupportInterface 
{
public:
    // 纯虚函数：FTP 特有操作
    virtual bool sendFtpFile(const QVariantMap& params) = 0;
    virtual bool downloadFtpFile(const QVariantMap& params) = 0;
    virtual bool listFtpFiles(const QVariantMap& params) = 0;
    virtual bool deleteFtpItem(const QVariantMap& params) = 0;

    // 纯虚信号
    virtual void ftpFileSent(bool success) = 0;
    virtual void ftpFileDownloaded(bool success) = 0;
    virtual void ftpFileListFetched(bool success, const QStringList& files, const QString& msg) = 0;
    virtual void ftpFileDeleted(bool success) = 0; // 删除完成信号
protected:
    virtual ~FtpSupportInterface() = default; // 保护析构，避免直接删除接口指针
};
// 声明接口，让 Qt 元对象系统识别
Q_DECLARE_INTERFACE(FtpSupportInterface, "com.example.FtpSupportInterface")