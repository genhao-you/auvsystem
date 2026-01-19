#pragma once
#include "communicationchannelbase.h"  // 基础通道抽象类（策略未提及，按架构依赖保留）
#include "WiredNetworkConfig.h"
#include"curl.h"
#include"ftpsupportinterface.h"
#include"ftpdeleteresult.h"
#include <QFile>                  // 本地 JSON 文件操作
#include <QSharedPointer>              // 智能指针（管理配置生命周期）
#include<QTimer>
#include<QThread>
#include<QMutex>

// 手动定义 curl 缺失的宏（curl 7.74.0 标准数值）
#ifndef CURLOPT_PASSV
#define CURLOPT_PASSV static_cast<CURLoption>(10000L) // 被动模式开关（CURLoption类型）
#endif

#ifndef CURLOPT_FTP_FILETYPE
#define CURLOPT_FTP_FILETYPE 10009L // FTP传输类型配置项
#endif

#ifndef CURLFTP_FILETYPE_ASCII
#define CURLFTP_FILETYPE_ASCII 0L   // ASCII传输类型
#endif

#ifndef CURLE_FTP_PASV_ERROR
#define CURLE_FTP_PASV_ERROR 131
#endif

#ifndef CURLOPT_FTP_DATA_TIMEOUT
#define CURLOPT_FTP_DATA_TIMEOUT 91 // 该值是curl官方定义的宏值
#endif
// FTP操作类型（贴合策略“任务序列传输方向”：上传=上位机→下位机，下载=下位机→上位机）
enum class FtpOpType {
    Upload = 0x01,  // 上传任务序列JSON（策略“任务制定”后传输到下位机）
    Download = 0x02, // 下载任务序列JSON（策略“关键数据回传”前从下位机获取）
};

// Curl 工作线程（异步执行 FTP 操作，避免阻塞主线程）
class CurlWorker : public QThread {
    Q_OBJECT
public:
    enum class TaskType { Login, Upload, UploadFolder, Download,ListFiles,
        Delete ,Disconnect
    };
    struct Task {
        TaskType type;
        QString host;
        int port;
        QString username;
        QString password;
        QString localPath;   // 单文件上传：本地文件路径；文件夹上传：本地文件夹路径
        QString remotePath;  // 单文件上传：远程文件路径；文件夹上传：远程根目录路径
        int timeout = 30;    // 超时时间（秒）
        bool recursiveList = false;
        FtpDeleteResult::DeleteType deleteType = FtpDeleteResult::DeleteType::File; // 新增：删除类型（文件/目录）
    };

public:
    explicit CurlWorker(QObject* parent = nullptr);
    ~CurlWorker() override;

    void setTask(const Task& task);  // 设置任务（登录/上传）
    void stopTask();                 // 停止当前任务
    TaskType getTaskType() const {
        return m_task.type;  // 通过公有函数返回私有成员的值
    }
    // 检查远程目录是否存在（返回true表示存在）
    bool curlCheckRemotePathExists(const QString& host, int port, const QString& username, 
        const QString& password,const QString& remoteDir, int timeout);
    bool curlCreateRemoteDir(const QString& host, int port, const QString& username, const QString& password,
        const QString& remoteDir, int timeout);
    bool curlVerifyPathByPwd(const QString& host, int port, const QString& username,
        const QString& password, const QString& remoteDir, int timeout);
    bool verifyFileExists(const QString& host, int port, const QString& username,
        const QString& password, const QString& remotePath, int timeout);
    // 辅助函数：递归遍历本地文件夹，获取所有文件的相对路径
    QStringList curlGetAllFilesInFolder(const QString& folderPath, QStringList& relativePaths);
     // 声明回调函数为友元（允许访问私有成员）
    friend int curlProgressCallback(void* userdata, curl_off_t dltotal, curl_off_t dlnow,
        curl_off_t ultotal, curl_off_t ulnow);
signals:
    void taskFinished(bool success, const QString& msg);  // 任务结果
    void uploadProgress(int progress);                    // 上传进度
    void downloadProgress(int progress);                  // 下载进度信号（区分上传）
    void fileListReceived(const QStringList& files);      // 远程文件列表获取完成
protected:
    void run() override;  // 线程执行入口

private:
    // curl 登录实现（用 PWD 命令验证登录）
    bool curlLogin(const QString& host, int port, const QString& username, 
        const QString& password, int timeout);
    // curl 上传实现
    bool curlUpload(const QString& host, int port, const QString& username,
        const QString& password,
        const QString& localPath, const QString& remotePath, int timeout);
    // curl 下载实现
    bool curlDownload(const QString& host, int port, const QString& username,
        const QString& password,
        const QString& remotePath, const QString& localSavePath, int timeout);
    // 获取远程文件列表
    bool curlListFiles(const QString& host, int port, const QString& username,
        const QString& password,const QString& remotePath, int timeout, bool recursive = false);
    bool curlListFilesRecursive(const QString& host, int port, const QString& username,
        const QString& password, const QString& remotePath, int timeout,
        QStringList& resultList, const QString& parentPath = "", const QString& userRootPath = "");
    bool curlDisconnect(const QString& host, int port, const QString& username,
        const QString& password, int timeout);
    bool curlUploadFolder(const QString& host, int port, const QString& username, 
        const QString& password,const QString& localFolderPath, 
        const QString& remoteRootPath, int timeout);
    bool curlDelete(const QString& host, int port, const QString& username,
        const QString& password, const QString& remotePath,
        FtpDeleteResult::DeleteType deleteType, int timeout);
    QString getFtpUserRootPath(const QString& host, int port, const QString& username,
        const QString& password, int timeout);
   
    curl_off_t getFtpFileSize(CURL* curl, const QString& ftpUrl,
        const QString& userPwd);
    // curl 全局初始化/清理
    static void curlGlobalInit();
    static void curlGlobalCleanup();

private:
    Task m_task;
    CURL* m_curl = nullptr;
    QAtomicInt m_stop;
    static bool s_curlInited;  // 标记 curl 全局是否初始化
    QString m_lastError;  // 声明错误信息成员变量
    int m_lastProgress = 0; // 记录上次进度值
    curl_off_t m_fileSize = 0; // 存储实际文件大小
    QRecursiveMutex  m_curlMutex; // 保护 curl 句柄的线程安全锁
};

//下载文件写入回调（curl 回调函数，需在类外声明）
static size_t curlWriteCallback(void* ptr, size_t size, size_t nmemb, void* userdata);
// 文件列表回调（类外声明）
static size_t curlListCallback(void* ptr, size_t size, size_t nmemb, void* userdata);

class WiredNetworkChannel : public CommunicationChannelBase, public FtpSupportInterface
{
	Q_OBJECT
    // 声明实现接口（Qt 元对象系统需要）
    Q_INTERFACES(FtpSupportInterface)
public:
    explicit WiredNetworkChannel(QObject* parent = nullptr);
    ~WiredNetworkChannel() override;

    // ---------------------- 重写基础通道接口（贴合策略传输需求）----------------------
    // 设置配置（加载FTP服务器信息，支撑策略中JSON传输）
    bool setConfig(const ChannelConfigPtr& config) override;
    // 获取当前配置（用于查看FTP连接信息，适配策略调试需求）
    ChannelConfigPtr config() const override;
    // 启动通道（建立FTP连接，准备策略中的JSON传输）
    bool start() override;
    // 停止通道（断开FTP连接，释放资源）
    void stop() override;
    // 检查通道运行状态（判断是否可传输策略中的任务序列）
    bool isRunning() const override;
    // 获取最后错误信息（定位策略传输失败原因）
    QString lastError() const override;
    // 获取通道状态描述（展示FTP连接/登录状态，适配策略监控需求）
    QString statusInfo() const override;
    // 实现 FTP 特性接口
    bool sendFtpFile(const QVariantMap& params) override;
    bool downloadFtpFile(const QVariantMap& params) override;
    bool listFtpFiles(const QVariantMap& params) override;
    bool deleteFtpItem(const QVariantMap& params) override; 
    void setInUse(bool inUse) { m_inUse = inUse; }
    bool isInUse() const { return m_inUse; }
signals:
    // 实现 FTP 接口信号
    void ftpFileSent(bool success) override;
    void ftpFileDownloaded(bool success) override;
    void ftpFileListFetched(bool success, const QStringList& files, const QString& msg) override;
    void ftpFileDeleted(bool success) override; 

    void sendFileRequested(const QVariantMap& params);
    void downloadFileRequested(const QVariantMap& params);
    void listFilesRequested(const QVariantMap& params);
    void deleteFileRequested(const QVariantMap& params); 
    // 上传进度（0-100）
    void uploadProgressUpdated(int progress);
    // 下载进度（0-100）
    void downloadProgressUpdated(int progress);
private slots:

    void onTransferTimeout();
   
    // ---------------------- 跨线程槽函数（保持原逻辑，确保线程安全） ----------------------

    void setConfigImpl(const ChannelConfigPtr& config);
    void startImpl();
    void stopImpl();
    void sendFileImpl(const QVariantMap& params);
    void downloadImpl(const QVariantMap& params); // ：下载实现槽函数
    void listFilesImpl(const QVariantMap& params);
    bool uploadFtpFolderImpl(const QVariantMap& params);
    void deleteFileImpl(const QVariantMap& params); // 删除实现槽函数
    // curl 线程结果回调
    void onCurlTaskFinished(bool success, const QString& msg);
    void onCurlUploadProgress(int progress);
    void onCurlDownloadProgress(int progress); // ：接收Curl下载进度
private:

    // ：任务规划模块路径解析
    bool parseTaskPlanningData(const QString& localPath, const QString& remotePath, FtpOpType& opType);
    // ：释放登录请求资源
    void releaseCurlWorker();

private:
    // ---------------------- 成员变量（替换 QFtp 相关为 Qt 5 网络组件） ----------------------
    QSharedPointer<WiredNetworkConfig> m_ftpConfig; // FTP 配置（同之前）
    QFile* m_localFile;                             // 本地 JSON 文件句柄（同之前）
    QString m_lastError;                            // 最后错误信息（同之前）
    bool m_isReady;                                 // “登录成功且就绪”
    FtpOpType m_currentOpType;                      // 当前 FTP 操作类型（同之前）
    QString m_currentRemotePath;                    // 当前远程路径（用于状态描述）
    QTimer* m_transferTimer;
    CurlWorker* m_curlWorker = nullptr;  // curl 异步工作线程
    bool m_isUploading = false;          // 是否正在上传
    int m_lastUploadProgress = -1; // 上次发射的进度（初始值-1）
    bool m_isDownloading = false; // 下载状态标志（避免并发操作）
    QString m_currentLocalSavePath; // 当前下载的本地保存路径
    QStringList m_fetchedFileList; // ：缓存文件列表
    bool m_inUse = false;
    FtpDeleteResult::DeleteType m_currentDeleteType; // 当前删除类型
};
