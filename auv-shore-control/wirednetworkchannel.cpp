#include "wirednetworkchannel.h"
#include <QJsonDocument>
#include <QDebug>
#include <QFileInfo>
#include <QUrl>
#include <QDir>
#include <QTextCodec>
// 初始化 curl 全局标记（确保全局仅初始化一次）
bool CurlWorker::s_curlInited = false;

// ---------------------- CurlWorker 核心实现（优化后） ----------------------
CurlWorker::CurlWorker(QObject* parent) : QThread(parent) {
    curlGlobalInit();  // 初始化 curl 全局环境（线程安全）
    m_fileSize = 0;
    m_lastProgress = 0;
}

CurlWorker::~CurlWorker() {
    stopTask();        // 终止当前任务
    wait();            // 等待线程退出，避免资源泄漏
    curlGlobalCleanup();// 清理 curl 全局环境
    if (m_curl) curl_easy_cleanup(m_curl); // 释放 curl 句柄
}

// curl 全局初始化（静态函数，确保线程安全）
void CurlWorker::curlGlobalInit() {
    if (!s_curlInited) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        s_curlInited = true;
        qDebug() << "[curl 7.74.0] 全局环境初始化成功";
    }
}

// curl 全局清理（静态函数）
void CurlWorker::curlGlobalCleanup() {
    if (s_curlInited) {
        curl_global_cleanup();
        s_curlInited = false;
        qDebug() << "[curl 7.74.0] 全局环境清理完成";
    }
}

// 设置任务参数（供外部调用，传递登录/上传信息）
void CurlWorker::setTask(const Task& task) {
    m_task = task;
    m_stop = false; // 重置终止标记
}

// 终止当前任务（线程安全）
void CurlWorker::stopTask() {
    m_stop.store(1); // 原子变量设置为1（终止）
}

bool CurlWorker::curlCheckRemotePathExists(const QString& host, int port, const QString& username, const QString& password, const QString& remoteDir, int timeout)
{
    // 1. 路径编码（处理特殊字符，保留目录分隔符）
    QString encodedRemoteDir = QUrl::toPercentEncoding(remoteDir, "/");
    QString ftpUrl = QString("ftp://%1:%2/%3").arg(host).arg(port).arg(encodedRemoteDir);
    if (!ftpUrl.endsWith("/")) ftpUrl += "/"; // 确保目录路径以/结尾，避免歧义

     // 2. 配置curl命令：发送CWD（切换目录）命令
    int retryCount = 0;
    const int maxRetry = 2;
    CURLcode res = CURLE_FAILED_INIT;

    while (retryCount < maxRetry && res != CURLE_OK) {
        curl_easy_reset(m_curl);
        curl_easy_setopt(m_curl, CURLOPT_URL, ftpUrl.toUtf8().data());
        curl_easy_setopt(m_curl, CURLOPT_USERPWD, QString("%1:%2").arg(username).arg(password).toUtf8().data());
        curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, "CWD"); // 核心：发送切换目录命令
        curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, timeout);
        curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, 10);
        curl_easy_setopt(m_curl, CURLOPT_FTP_USE_EPSV, 0L); // 兼容FileZilla
        curl_easy_setopt(m_curl, CURLOPT_PASSV, 1L); // 关键：强制启用被动模式（必须添加）
        curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L); // 调试日志（可关闭）
        // 在curl配置中添加（与其他选项同级）
        curl_easy_setopt(m_curl, CURLOPT_NOBODY, 1L); // 不读取响应体（仅处理命令本身）
        res = curl_easy_perform(m_curl);
        if (res != CURLE_OK) {
            retryCount++;
            qDebug() << "[curl] CWD 命令失败，重试次数：" << retryCount << "错误：" << curl_easy_strerror(res);
            QThread::msleep(500); // 重试间隔 500ms
        }
    }

    if (res != CURLE_OK) {
        m_lastError = QString("检查目录失败（curl错误）：%1").arg(curl_easy_strerror(res));
        return false;
    }
   
    // 4. 解析FTP服务器响应码
    long responseCode = 0;
    curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &responseCode);
    if (responseCode == 0) {
        qDebug() << "[curl] 响应码解析失败，尝试通过二次验证确认目录存在";
        // 二次验证：发送 PWD 命令（获取当前目录），若成功则认为目录存在
        bool secondCheck = curlVerifyPathByPwd(host, port, username, password, remoteDir, timeout);
        if (secondCheck) {
            qDebug() << "[curl] 二次验证通过，目录存在：" << remoteDir;
            return true;
        }
        else {
            m_lastError = QString("目录存在但响应码解析失败，二次验证也失败：%1").arg(remoteDir);
            return false;
        }
    }
    if (responseCode == 250) {
        qDebug() << "[curl] 远程目录存在：" << remoteDir;
        return true; // 目录存在
    }
    else if (responseCode == 550) {
        m_lastError = QString("远程目录不存在：%1（服务器响应码：%2）").arg(remoteDir).arg(responseCode);
        return false; // 目录不存在
    }
    else {
        m_lastError = QString("检查目录异常（响应码：%1）").arg(responseCode);
        return false; // 其他错误（如权限不足）
    }
}

bool CurlWorker::curlCreateRemoteDir(const QString& host, int port, const QString& username, const QString& password, const QString& remoteDir, int timeout)
{
    // 1. 编码目录路径
    QString encodedRemoteDir = QUrl::toPercentEncoding(remoteDir, "/");
    QString ftpUrl = QString("ftp://%1:%2/%3").arg(host).arg(port).arg(encodedRemoteDir);

    // 2. 配置curl：发送MKD（创建目录）命令，支持多级目录
    curl_easy_reset(m_curl);
    curl_easy_setopt(m_curl, CURLOPT_URL, ftpUrl.toUtf8().data());
    curl_easy_setopt(m_curl, CURLOPT_USERPWD, QString("%1:%2").arg(username).arg(password).toUtf8().data());
    curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, "MKD"); // 创建目录命令
    curl_easy_setopt(m_curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L); // 关键：自动创建多级目录（curl 7.10+支持）
    curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, 10);
    curl_easy_setopt(m_curl, CURLOPT_FTP_USE_EPSV, 0L);
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0L);

    // 3. 执行创建命令
    CURLcode res = curl_easy_perform(m_curl);
    if (res != CURLE_OK) {
        m_lastError = QString("创建目录失败：%1（curl错误：%2）").arg(remoteDir).arg(curl_easy_strerror(res));
        return false;
    }

    // 4. 检查响应码（257表示目录创建成功）
    long responseCode = 0;
    curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &responseCode);
    if (responseCode == 257) {
        qDebug() << "[curl] 远程目录创建成功：" << remoteDir;
        return true;
    }
    else {
        m_lastError = QString("创建目录失败（响应码：%1）").arg(responseCode);
        return false;
    }
}

// 线程执行入口（优化：句柄复用+配置重置）
void CurlWorker::run() {
    bool success = false;
    QString msg;

    // 确保 curl 句柄初始化（复用已有句柄，减少内存开销）
    if (!m_curl) {
        m_curl = curl_easy_init();
        if (!m_curl) {
            emit taskFinished(false, "curl 句柄初始化失败");
            return;
        }
    }

    // 重置 curl 配置（避免上一次任务的选项残留）
    curl_easy_reset(m_curl);

    // 根据任务类型执行操作
    switch (m_task.type) {
    case TaskType::Login:
        success = curlLogin(m_task.host, m_task.port, m_task.username, 
            m_task.password, m_task.timeout);
        msg = success ? QString("登录成功：%1:%2").arg(m_task.host).
            arg(m_task.port)
            : QString("登录失败：%1").arg(m_lastError);
        break;
    case TaskType::Upload:
        success = curlUpload(m_task.host, m_task.port, m_task.username,
            m_task.password,
            m_task.localPath, m_task.remotePath, m_task.timeout);
        msg = success ? QString("上传成功：%1").arg(m_task.remotePath)
            : QString("上传失败：%1").arg(m_lastError);
        break;
    case TaskType::Download:
        success = curlDownload(
            m_task.host, m_task.port, m_task.username, m_task.password,
            m_task.remotePath, m_task.localPath, m_task.timeout
        );
        msg = success ? QString("下载成功：%1→%2").arg(m_task.remotePath).
            arg(m_task.localPath)
            : QString("下载失败：%1").arg(m_lastError);
        break;
    case TaskType::ListFiles:
        success = curlListFiles(m_task.host, m_task.port, m_task.username,
            m_task.password,
            m_task.remotePath, m_task.timeout,m_task.recursiveList);
        msg = success ? QString("获取文件列表成功：%1").arg(m_task.remotePath)
            : QString("获取文件列表失败：%1").arg(m_lastError);
        break;
    case TaskType::Delete:
        success = curlDelete(m_task.host, m_task.port, m_task.username,
            m_task.password, m_task.remotePath, m_task.deleteType, m_task.timeout);
        msg = success ? QString("删除成功：%1").arg(m_task.remotePath)
            : QString("删除失败：%1").arg(m_lastError);
        break;
    case TaskType::Disconnect:
        success = curlDisconnect(m_task.host, m_task.port, m_task.username,
            m_task.password, m_task.timeout);
        msg = success ? "FTP连接已断开" : QString("断开连接失败：%1").
            arg(m_lastError);
        break;
    default:
        msg = "未知任务类型（仅支持登录/上传）";
        break;
    }

    emit taskFinished(success, msg);
}

// curl 登录实现（优化：NOOP 轻量验证+TCP保活+精准错误处理）
bool CurlWorker::curlLogin(const QString& host, int port, const QString& username, const QString& password, int timeout) {
    // 构建基础 FTP URL（根目录即可，NOOP 无需具体路径）
    QString ftpUrl = QString("ftp://%1:%2/").arg(host).arg(port);

    // curl 核心配置（适配 curl 7.74.0，禁用冗余操作）
    curl_easy_setopt(m_curl, CURLOPT_URL, ftpUrl.toUtf8().data());
    curl_easy_setopt(m_curl, CURLOPT_USERPWD, QString("%1:%2").arg(username).arg(password).toUtf8().data());
    curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, "NOOP");  // 核心：NOOP 轻量验证（无数据连接）
    curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, timeout);       // 整体超时（秒）
    curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, 10);     // 连接超时（秒）
    curl_easy_setopt(m_curl, CURLOPT_FTP_USE_EPSV, 0L);       // 禁用 EPSV，兼容 FileZilla 0.9.60
    curl_easy_setopt(m_curl, CURLOPT_NOBODY, 1L);             // 不读取响应体（NOOP 仅返回状态码）
    curl_easy_setopt(m_curl, CURLOPT_FTP_FILEMETHOD, CURLFTPMETHOD_NOCWD);  // 不自动切换目录
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0L);     // 禁用 SSL 验证（非 FTPS 场景）
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L);            // 调试日志（测试通过后可设为 0L 关闭）
    curl_easy_setopt(m_curl, CURLOPT_DEBUGFUNCTION, nullptr); // 禁用自定义调试回调（用默认输出）

    // TCP 保活配置（避免空闲超时断开）
    curl_easy_setopt(m_curl, CURLOPT_TCP_KEEPALIVE, 1L);    // 启用 TCP 保活
    curl_easy_setopt(m_curl, CURLOPT_TCP_KEEPIDLE, 60L);    // 空闲 60 秒后发送保活包
    curl_easy_setopt(m_curl, CURLOPT_TCP_KEEPINTVL, 30L);   // 每 30 秒发送一次保活包

    // 执行 curl 操作（curl 7.74.0 对 NOOP 响应解析更稳定）
    CURLcode res = curl_easy_perform(m_curl);
    if (res != CURLE_OK) {
        m_lastError = QString("curl 错误（码：%1）：%2")
            .arg(res).arg(curl_easy_strerror(res));
        qDebug() << "[curl 7.74.0] 登录错误：" << m_lastError;
        return false;
    }

    // 验证 NOOP 响应码（必须为 200 OK，curl 7.74.0 支持精准获取）
    long responseCode = 0;
    curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &responseCode);
    if (responseCode != 200 && responseCode != 257)
    {
        m_lastError = QString("服务器响应异常：NOOP 命令返回 %1（预期 200）").arg(responseCode);
        qDebug() << "[curl 7.74.0] 登录响应码异常：" << responseCode;
        return false;
    }

    qDebug() << "[curl 7.74.0] FTP 登录成功：" << host << ":" << port << "（用户：" << username << "）";
    return true;
}

// curl 上传数据读取回调（读取本地文件内容）
static size_t curlReadCallback(void* ptr, size_t size, size_t nmemb, void* userdata) {
    QFile* file = static_cast<QFile*>(userdata);
    if (!file->isOpen() || file->atEnd()) return 0;
    // 读取数据（size*nmemb 为单次最大读取字节数）
    return file->read(static_cast<char*>(ptr), size * nmemb);
}

// curl 上传进度回调（优化：支持终止上传+精准进度计算）
static int curlProgressCallback(void* userdata, curl_off_t dltotal, 
    curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) 
{
    CurlWorker* worker = static_cast<CurlWorker*>(userdata);
    // 线程安全检查终止标记（原子变量）
    if (worker->m_stop.load() == 1) return 1; // 返回1终止curl操作

    // 边界值校验
    if (ultotal == 0 && dltotal == 0) return 0;

    if (worker->getTaskType() == CurlWorker::TaskType::Upload) {
        // 使用文件实际大小而非ultotal（解决ultotal=0的问题）
        curl_off_t actualTotal = worker->m_fileSize;
        curl_off_t total = (actualTotal > 0) ? actualTotal : ultotal;

        if (total <= 0 || ulnow < 0) return 0;
        if (ulnow > total) ulnow = total;

        // 计算进度（仅定义一次progress变量）
        double progressDouble = (static_cast<double>(ulnow) / static_cast<double>(total)) * 100.0;
        progressDouble = qBound(0.0, progressDouble, 100.0);
        int currentProgress = static_cast<int>(progressDouble);

        // 频率限制：仅进度变化≥1%时才发射信号（避免循环触发）
        if (currentProgress != worker->m_lastProgress) {
            worker->m_lastProgress = currentProgress;
            emit worker->uploadProgress(currentProgress);

            // 调试日志（简化，避免刷屏）
            qDebug() << QString("[上传进度] %1/%2 KB (%3%)")
                .arg(static_cast<qulonglong>(ulnow) / 1024)
                .arg(static_cast<qulonglong>(total) / 1024)
                .arg(currentProgress);
        }
    }
    else if (worker->getTaskType() == CurlWorker::TaskType::Download) {
        if (dltotal <= 0 || dlnow < 0) return 0;

        double progressDouble = (static_cast<double>(dlnow) / static_cast<double>(dltotal)) * 100.0;
        progressDouble = qBound(0.0, progressDouble, 100.0);
        int currentProgress = static_cast<int>(progressDouble);

        // 频率限制：仅进度变化时发射信号
        if (currentProgress != worker->m_lastProgress) {
            worker->m_lastProgress = currentProgress;
            emit worker->downloadProgress(currentProgress);

            qDebug() << QString("[下载进度] %1/%2 KB (%3%)")
                .arg(static_cast<qulonglong>(dlnow) / 1024)
                .arg(static_cast<qulonglong>(dltotal) / 1024)
                .arg(currentProgress);
        }

        // 关键：进度100%时主动提示curl收尾（非终止，仅日志）
        if (currentProgress == 100) {
            qDebug() << "[下载完成] 进度100%，等待服务器响应...";
        }
    }
    return 0; // 返回0继续curl操作
}

size_t curlWriteCallback(void* ptr, size_t size, size_t nmemb, void* userdata)
{
    QFile* file = static_cast<QFile*>(userdata);
    if (!file->isOpen()) return 0;
    // 写入数据到本地文件（size*nmemb 为单次接收字节数）
    return file->write(static_cast<const char*>(ptr), size * nmemb);
}

size_t curlListCallback(void* ptr, size_t size, size_t nmemb, void* userdata)
{
    static thread_local QString s_unfinishedLine; // 线程安全的未完成行缓存
    QStringList* fileInfoList = static_cast<QStringList*>(userdata);
    size_t total = size * nmemb;
    if (total == 0) return 0;

    QByteArray byteData(static_cast<char*>(ptr), total);
    QString data = s_unfinishedLine + QString(byteData);
    s_unfinishedLine.clear();

    int lastNewLine = data.lastIndexOf('\n');
    if (lastNewLine == -1) {
        s_unfinishedLine = data;
        return total;
    }

    QString completeData = data.left(lastNewLine);
    s_unfinishedLine = data.mid(lastNewLine + 1);

    QStringList lines = completeData.split("\n", Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        QString trimmedLine = line.trimmed();
        if (trimmedLine.isEmpty()) continue;

        QStringList parts = trimmedLine.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
        if (parts.size() < 8) { // 完整LIST至少8个字段（权限、链接数、所有者、组、大小、月、日、时间/年、名称）
            qDebug() << "[列表解析] 非标准LIST行，跳过：" << trimmedLine;
            continue; // 跳过无效行，避免解析错乱
        }

        // ========== 核心优化：优先通过权限字段判断目录 ==========
        QString permissions = parts[0].trimmed();
        QString type = "file";
        if (permissions.startsWith('d')) { // 权限以d开头 → 目录
            type = "dir";
        }
        else if (permissions.startsWith('l')) { // 软链接
            type = "link";
        }

        // 提取有效字段（完整LIST格式：权限 链接数 所有者 组 大小 月 日 时间 名称）
        QString sizeStr = parts[4].trimmed();
        QString timeInfo = QString("%1 %2 %3").arg(parts[5]).arg(parts[6]).arg(parts[7]);
        // 名称可能包含空格（如"岸基截图"），拼接剩余所有字段
        QString name = parts.mid(8).join(" ").trimmed();

        // 移除目录名称末尾的/（如果有）
        if (type == "dir" && name.endsWith("/")) {
            name = name.left(name.length() - 1);
        }

        // 拼接标准格式：名称|大小|类型|权限|时间
        QString fileInfo = QString("%1|%2|%3|%4|%5")
            .arg(name)
            .arg(sizeStr)
            .arg(type)
            .arg(permissions)
            .arg(timeInfo);
        fileInfoList->append(fileInfo);

        qDebug() << "[列表解析] 成功解析：" << name << "→ 类型：" << type << "，大小：" << sizeStr;
    }

    return total;
}

// curl 上传实现（优化：路径编码+文件预校验+大文件支持+保活）
bool CurlWorker::curlUpload(const QString& host, int port, const QString& username, const QString& password,
    const QString& localPath, const QString& remotePath, int timeout)
{
    m_fileSize = 0;
    // 1. 增强文件预校验（避免无效上传）
    QFileInfo fileInfo(localPath);
    if (!fileInfo.exists()) {
        m_lastError = QString("文件不存在：%1").arg(localPath);
        return false;
    }
    if (!fileInfo.isFile() || !fileInfo.isReadable()) {
        m_lastError = QString("文件不可读或不是常规文件：%1").arg(localPath);
        return false;
    }

    // 保存实际文件大小
    m_fileSize = static_cast<curl_off_t>(fileInfo.size());
    qDebug() << "[curl] 实际文件大小：" << m_fileSize << "字节";

    // 1. 提取远程路径中的目录部分（如remotePath为a/b/c.json，则目录为a/b）
    QFileInfo remoteFileInfo(remotePath);
    QString remoteDir = remoteFileInfo.path(); // 获取文件所在目录

    // 2. 检查远程目录是否存在
    bool dirExists = curlCheckRemotePathExists(host, port, username, password, remoteDir, timeout);
    if (!dirExists) {
        // 3. 目录不存在，尝试创建
        bool dirCreated = curlCreateRemoteDir(host, port, username, password, remoteDir, timeout);
        if (!dirCreated) 
        {
            m_lastError = QString("上传失败：远程目录不存在且无法创建（%1）").arg(m_lastError);
            return false;
        }
    }

    // 2. 打开本地文件（只读模式）
    QFile file(localPath);
    if (!file.open(QIODevice::ReadOnly)) {
        m_lastError = QString("文件打开失败：%1（错误：%2）").arg(localPath).arg(file.errorString());
        return false;
    }

    // 3. 路径编码（处理中文、空格、括号等特殊字符）
    QString encodedRemotePath = QUrl::toPercentEncoding(remotePath, "/"); // 保留 "/" 不编码
    QString ftpUrl = QString("ftp://%1:%2/%3").arg(host).arg(port).arg(encodedRemotePath);
    if (!ftpUrl.startsWith("ftp://")) ftpUrl.prepend("ftp://");

    // 4. curl 上传核心配置（适配 curl 7.74.0）
    curl_easy_setopt(m_curl, CURLOPT_URL, ftpUrl.toUtf8().data());
    curl_easy_setopt(m_curl, CURLOPT_USERPWD, QString("%1:%2").arg(username).arg(password).toUtf8().data());
    curl_easy_setopt(m_curl, CURLOPT_UPLOAD, 1L);               // 启用上传模式
    curl_easy_setopt(m_curl, CURLOPT_READFUNCTION, curlReadCallback);  // 数据读取回调
    curl_easy_setopt(m_curl, CURLOPT_READDATA, &file);          // 传递文件句柄
    curl_easy_setopt(m_curl, CURLOPT_INFILESIZE_LARGE, m_fileSize); // 大文件支持（curl 7.74.0 兼容）
    curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, timeout);         // 整体超时（秒）
    curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, 10);       // 连接超时（秒）
    curl_easy_setopt(m_curl, CURLOPT_FTP_USE_EPSV, 0L);          // 禁用 EPSV，兼容 FileZilla
    //curl_easy_setopt(m_curl, CURLOPT_PASSV, 1L);                // 明确启用被动模式（避免歧义）
    curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, 0L);           // 启用进度回调
    curl_easy_setopt(m_curl, CURLOPT_PROGRESSFUNCTION, curlProgressCallback); // 进度回调
    curl_easy_setopt(m_curl, CURLOPT_PROGRESSDATA, this);       // 传递线程指针（用于终止判断）
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0L);       // 禁用 SSL 验证（非 FTPS 场景）
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L);              // 调试日志（可按需关闭）
    curl_easy_setopt(m_curl, CURLOPT_UPLOAD_BUFFERSIZE, 8192L); // 减小上传缓冲区，提高进度更新频率
    curl_easy_setopt(m_curl, CURLOPT_FTP_CREATE_MISSING_DIRS, CURLFTP_CREATE_DIR_RETRY); // 自动创建目录
    // 5. 上传保活（大文件上传避免服务器空闲超时）
    curl_easy_setopt(m_curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(m_curl, CURLOPT_TCP_KEEPIDLE, 60L);
    curl_easy_setopt(m_curl, CURLOPT_TCP_KEEPINTVL, 30L);

    // 6. 执行上传并处理结果
    CURLcode res = curl_easy_perform(m_curl);
    file.close(); // 无论成功与否，先关闭文件

    // 处理主动终止场景
    if (m_stop) {
        m_lastError = "上传被用户终止";
        qDebug() << "[curl 7.74.0] 上传终止：" << localPath << "→" << remotePath;
        return false;
    }

    // 7. 精准错误定位（结合 curl 错误码和服务器响应码）
    if (res != CURLE_OK) {
        long responseCode = 0;
        curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &responseCode);
        m_lastError = QString("curl 上传错误（码：%1）：%2 | 服务器响应码：%3")
            .arg(res).arg(curl_easy_strerror(res)).arg(responseCode);
        qDebug() << "[curl 7.74.0] 上传错误详情：" << m_lastError;
        return false;
    }

    // 8. 验证服务器响应码（FTP 226=上传成功，200=部分服务器兼容响应）
    long responseCode = 0;
    curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &responseCode);
    if (responseCode != 226 && responseCode != 200) {
        m_lastError = QString("服务器上传失败：响应码 %1（预期 226/200）").arg(responseCode);
        qDebug() << "[curl 7.74.0] 上传响应码异常：" << responseCode;
        return false;
    }

    // 上传成功日志
    qDebug() << "[curl 7.74.0] 上传成功：" << localPath << "→" << remotePath
        << "（大小：" << fileInfo.size() << "字节）";
    return true;
}

bool CurlWorker::curlDownload(const QString& host, int port, const QString& username, const QString& password, const QString& remotePath, const QString& localSavePath, int timeout)
{
    // 1. 基础校验（保留）
    if (!m_curl) {
        m_curl = curl_easy_init();
        if (!m_curl) {
            m_lastError = "curl句柄初始化失败";
            qDebug() << "[下载错误]" << m_lastError;
            return false;
        }
    }

    QFile file(localSavePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        m_lastError = QString("本地文件打开失败：%1（错误：%2）").arg(localSavePath).arg(file.errorString());
        qDebug() << "[下载错误]" << m_lastError;
        return false;
    }

    // ========== 核心修改：适配相对路径，跳过显式CWD ==========
    // 拆分路径并转为相对路径（去掉开头的/）
    QString actualRemotePath = remotePath;
    if (actualRemotePath.startsWith("/")) {
        actualRemotePath = actualRemotePath.mid(1);
    }
    // 拼接URL：用相对路径，无开头/
    QString ftpUrl = QString("ftp://%1:%2/%3").arg(host).arg(port).arg(actualRemotePath);
    qDebug() << "[适配服务器路径] 最终FTP URL：" << ftpUrl;

    // 2. 重置curl，清空旧配置
    curl_easy_reset(m_curl);

    // 3. 核心配置：模拟网口助手的关键参数（去掉显式CWD/TYPE，让curl自动处理）
    curl_easy_setopt(m_curl, CURLOPT_URL, ftpUrl.toUtf8().data());
    curl_easy_setopt(m_curl, CURLOPT_USERPWD, QString("%1:%2").arg(username).arg(password).toUtf8().data());
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &file);

    // ========== 网口助手的核心适配参数 ==========
    curl_easy_setopt(m_curl, CURLOPT_PASSV, 1L); // 用PASV模式（和网口助手一致）
    curl_easy_setopt(m_curl, CURLOPT_FTP_SKIP_PASV_IP, 1L); // 替换0.0.0.0为服务器IP
    curl_easy_setopt(m_curl, CURLOPT_FTP_USE_EPSV, 0L);
    curl_easy_setopt(m_curl, CURLOPT_TRANSFERTEXT, 0L); // 二进制传输（TYPE I）
    curl_easy_setopt(m_curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 0L); // 不自动建目录

    // 宽松超时：对齐网口助手
    curl_easy_setopt(m_curl, CURLOPT_TIMEOUT_MS, 60000L);
    curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT_MS, 15000L);
    curl_easy_setopt(m_curl, CURLOPT_LOW_SPEED_LIMIT, 0L); // 关闭低速限制
    curl_easy_setopt(m_curl, CURLOPT_LOW_SPEED_TIME, 0L);

    // 调试日志：保留
    curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L);

    // 4. 执行下载（无需先CWD，curl会自动处理路径）
    qDebug() << "[下载开始] 适配服务器相对路径，执行下载：" << ftpUrl;
    CURLcode res = curl_easy_perform(m_curl);

    // 5. 后续处理（保留）
    file.flush();
    file.close();

    qDebug() << "[下载结束] curl返回码：" << res << "（" << curl_easy_strerror(res) << "）";

    if (res != CURLE_OK) {
        long respCode = 0;
        curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &respCode);
        m_lastError = QString("下载失败：%1（响应码：%2）").arg(curl_easy_strerror(res)).arg(respCode);
        qDebug() << "[下载错误]" << m_lastError;
        return false;
    }

    // 验证226响应码
    long respCode = 0;
    curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &respCode);
    if (respCode == 226) {
        qDebug() << "[下载成功] 收到226响应码，文件大小：" << QFileInfo(localSavePath).size() << "字节";
        return true;
    }
    else {
        m_lastError = QString("未收到226响应码，响应码：%1").arg(respCode);
        qDebug() << "[下载错误]" << m_lastError;
        return false;
    }
}

bool CurlWorker::curlListFiles(const QString& host, int port, const QString& username,
    const QString& password, const QString& remotePath, int timeout, bool recursive )
{
    
   
    if (!s_curlInited) {
        curlGlobalInit(); // 确保全局初始化完成
    }

    if (!m_curl) {
        m_curl = curl_easy_init();
        if (!m_curl) {
            m_lastError = "curl 句柄初始化失败（列表请求）";
            qCritical() << "[列表请求] 句柄为空，终止请求";
            return false;
        }
    }

    QStringList fileInfoList;
    fileInfoList.clear();
    QString actualRemotePath = remotePath;
    if (actualRemotePath == "/" || actualRemotePath.isEmpty()) {
        actualRemotePath = "";
    }

    QThread::msleep(500);
    qDebug() << "[列表请求] 等待服务器同步，延迟500ms后请求";
    QMutexLocker locker(&m_curlMutex);

    if (recursive) {
        if (!curlListFilesRecursive(host, port, username, password, actualRemotePath, timeout, fileInfoList)) {
            return false;
        }
    }
    else {
        QString ftpUrl = QString("ftp://%1:%2").arg(host).arg(port);
        if (!actualRemotePath.isEmpty()) {
            QString encodedRemotePath = QUrl::toPercentEncoding(actualRemotePath);
            if (!ftpUrl.endsWith("/") && !encodedRemotePath.startsWith("/")) {
                ftpUrl += "/";
            }
            ftpUrl += encodedRemotePath;
        }
        if (!ftpUrl.endsWith("/") && !actualRemotePath.isEmpty()) {
            ftpUrl += "/";
        }
        qDebug() << "[列表请求] 修正后URL：" << ftpUrl;

        curl_easy_reset(m_curl);
        curl_easy_setopt(m_curl, CURLOPT_URL, ftpUrl.toUtf8().data());
        curl_easy_setopt(m_curl, CURLOPT_USERPWD, QString("%1:%2").arg(username).arg(password).toUtf8().data());
        curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, curlListCallback);
        curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &fileInfoList);

        // 超时/终止配置（保留，补充句柄非空检查）
        curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, timeout);
        curl_easy_setopt(m_curl, CURLOPT_FTP_RESPONSE_TIMEOUT, 15L);
        curl_easy_setopt(m_curl, CURLOPT_LOW_SPEED_LIMIT, 1L);
        curl_easy_setopt(m_curl, CURLOPT_LOW_SPEED_TIME, 10L);
        curl_easy_setopt(m_curl, CURLOPT_PROGRESSFUNCTION, [](void* userdata, curl_off_t, curl_off_t, curl_off_t, curl_off_t) -> int {
            CurlWorker* worker = static_cast<CurlWorker*>(userdata);
            if (worker->m_stop.load() == 1) {
                return 1;
            }
            return 0;
            });
        curl_easy_setopt(m_curl, CURLOPT_PROGRESSDATA, this);
        curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, 0L);

        // 原有配置
        curl_easy_setopt(m_curl, CURLOPT_FTP_USE_EPSV, 0L);
        curl_easy_setopt(m_curl, CURLOPT_PASSV, 1L);
        curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, 10);
        curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0L);

        // ========== 核心修复4：执行前再次校验句柄 ==========
        if (!m_curl) {
            m_lastError = "curl 句柄为空（执行列表请求前）";
            qCritical() << "[列表请求] 句柄为空，终止 perform";
            return false;
        }

        CURLcode res = curl_easy_perform(m_curl);

        if (m_stop.load() == 1) {
            m_lastError = "列表请求被终止";
            qDebug() << "[列表请求] 被主动终止：" << ftpUrl;
            return false;
        }

        if (res == CURLE_FTP_PASV_ERROR) {
            qDebug() << "[curl] PASV模式失败，降级为主动模式（PORT）";
            curl_easy_reset(m_curl);
            curl_easy_setopt(m_curl, CURLOPT_URL, ftpUrl.toUtf8().data());
            curl_easy_setopt(m_curl, CURLOPT_USERPWD, QString("%1:%2").arg(username).arg(password).toUtf8().data());
            curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, curlListCallback);
            curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &fileInfoList);
            // 同样注释掉极简LIST选项
            // curl_easy_setopt(m_curl, CURLOPT_DIRLISTONLY, 1L);
            // curl_easy_setopt(m_curl, CURLOPT_FTPLISTONLY, 1L);
            curl_easy_setopt(m_curl, CURLOPT_PASSV, 0L);
            curl_easy_setopt(m_curl, CURLOPT_FTP_USE_EPSV, 0L);
            curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, timeout);
            curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, 10);

            // ========== 核心修复5：降级模式下再次校验句柄 ==========
            if (!m_curl) {
                m_lastError = "curl 句柄为空（降级主动模式）";
                qCritical() << "[列表请求] 句柄为空，终止降级模式";
                return false;
            }

            res = curl_easy_perform(m_curl);
        }

        if (res != CURLE_OK) {
            m_lastError = QString("获取文件列表失败：%1").arg(curl_easy_strerror(res));
            qDebug() << "[FTP] 列表获取错误：" << m_lastError << "URL：" << ftpUrl;
            return false;
        }

        qDebug() << "[列表请求] 回调捕获文件数：" << fileInfoList.size() << "列表内容：" << fileInfoList;
    }

    emit fileListReceived(fileInfoList);
    return true;
}

bool CurlWorker::curlListFilesRecursive(const QString& host, int port, 
const QString& username, const QString& password, const QString& remotePath,
int timeout, QStringList& resultList, const QString& parentPath, const QString& userRootPath)
{
   
    QMutexLocker locker(&m_curlMutex);

    if (!m_curl) {
        m_curl = curl_easy_init();
        if (!m_curl) {
            m_lastError = "curl 句柄初始化失败（递归列表）";
            qCritical() << "[递归列表] 句柄为空，终止递归";
            return false;
        }
    }

    // 使用相对路径
    QString currentRemotePath = remotePath;
    QString ftpUrl = QString("ftp://%1:%2").arg(host).arg(port);

    if (!currentRemotePath.isEmpty()) {
        QString encodedRemotePath = QUrl::toPercentEncoding(currentRemotePath, "/");
        ftpUrl += "/" + encodedRemotePath;
    }
    if (!ftpUrl.endsWith("/")) ftpUrl += "/";

    QStringList currentLevelList;

    // 获取当前目录的一级列表
    curl_easy_reset(m_curl);
    curl_easy_setopt(m_curl, CURLOPT_URL, ftpUrl.toUtf8().data());
    curl_easy_setopt(m_curl, CURLOPT_USERPWD, QString("%1:%2").arg(username).arg(password).toUtf8().data());
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, curlListCallback);
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &currentLevelList);
    curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, 10);
    curl_easy_setopt(m_curl, CURLOPT_FTP_USE_EPSV, 0L);
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(m_curl, CURLOPT_FTP_FILEMETHOD, CURLFTPMETHOD_NOCWD); // 不切换目录
    curl_easy_setopt(m_curl, CURLOPT_PASSV, 1L); // 被动模式
    curl_easy_setopt(m_curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 0L); // 禁用自动创建目录

    // ========== 执行前校验句柄 ==========
    if (!m_curl) {
        m_lastError = "curl 句柄为空（递归执行前）";
        qCritical() << "[递归列表] 句柄为空，终止 perform";
        return false;
    }

    CURLcode res = curl_easy_perform(m_curl);
    bool isPasvError = (res == CURLE_FTP_PASV_ERROR) || (res == 131);
    QString errDesc = curl_easy_strerror(res);
    if (isPasvError || errDesc.contains("PASV", Qt::CaseInsensitive)) {
        qDebug() << "[curl] 递归列表PASV失败（错误码：" << res << "），降级为主动模式：" << ftpUrl;
        curl_easy_reset(m_curl);
        curl_easy_setopt(m_curl, CURLOPT_URL, ftpUrl.toUtf8().data());
        curl_easy_setopt(m_curl, CURLOPT_USERPWD, QString("%1:%2").arg(username).arg(password).toUtf8().data());
        curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, curlListCallback);
        curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &currentLevelList);
        curl_easy_setopt(m_curl, CURLOPT_PASSV, 0L); // 主动模式
        curl_easy_setopt(m_curl, CURLOPT_FTP_USE_EPSV, 0L);

        // ========== 核心修复：降级模式下再次校验句柄 ==========
        if (!m_curl) {
            m_lastError = "curl 句柄为空（递归降级模式）";
            qCritical() << "[递归列表] 句柄为空，终止降级模式";
            return false;
        }

        res = curl_easy_perform(m_curl);
    }
    if (res != CURLE_OK) {
        m_lastError = QString("获取目录列表失败：%1").arg(curl_easy_strerror(res));
        return false;
    }

    // 处理当前层级的条目
    for (const QString& entry : currentLevelList) {
        // 解析条目信息
        QStringList parts = entry.split("|");
        if (parts.size() < 3) continue;

        QString name = parts[0];
        QString type = parts[2];

        // 跳过上级目录和当前目录
        if (name == "." || name == "..") continue;

        QString fullPath = parentPath.isEmpty() ? name : parentPath + "/" + name;

        // 添加到结果列表（标记完整路径）
        QString modifiedEntry = entry;
        modifiedEntry.replace(name, fullPath);
        resultList.append(modifiedEntry);

        // 如果是目录，递归处理
        if (type == "dir") {
            QString newRemotePath = currentRemotePath.isEmpty() ? name : currentRemotePath + "/" + name;
            QString newParentPath = fullPath;
            bool recursiveOk = curlListFilesRecursive(host, port, username, password, newRemotePath, timeout, resultList, newParentPath);
            if (!recursiveOk) {
                return false;
            }
        }
    }

    return true;
}

bool CurlWorker::curlDisconnect(const QString& host, int port, const QString& username, const QString& password, int timeout)
{
    QString ftpUrl = QString("ftp://%1:%2/").arg(host).arg(port);

    // 发送QUIT命令优雅断开
    curl_easy_reset(m_curl);
    curl_easy_setopt(m_curl, CURLOPT_URL, ftpUrl.toUtf8().data());
    curl_easy_setopt(m_curl, CURLOPT_USERPWD, QString("%1:%2").arg(username).arg(password).toUtf8().data());
    curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, "QUIT"); // FTP断开命令
    curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, 5);
    curl_easy_setopt(m_curl, CURLOPT_FTP_USE_EPSV, 0L);
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0L);

    CURLcode res = curl_easy_perform(m_curl);
    if (res != CURLE_OK) {
        m_lastError = QString("断开连接失败：%1").arg(curl_easy_strerror(res));
        return false;
    }

    qDebug() << "[curl] FTP连接已断开";
    return true;
}

bool CurlWorker::curlUploadFolder(const QString& host, int port, const QString& username,
    const QString& password, const QString& localFolderPath, const QString& remoteRootPath, int timeout)
{
    // 1. 校验本地文件夹
    QFileInfo folderInfo(localFolderPath);
    if (!folderInfo.exists() || !folderInfo.isDir()) {
        m_lastError = QString("本地文件夹不存在或不是目录：%1").arg(localFolderPath);
        return false;
    }

    // 2. 递归获取文件夹内所有文件的相对路径
    QStringList relativeFilePaths;
    curlGetAllFilesInFolder(localFolderPath, relativeFilePaths);
    if (relativeFilePaths.isEmpty()) {
        m_lastError = QString("本地文件夹为空：%1").arg(localFolderPath);
        return false;
    }

    // 3. 计算总文件大小（用于聚合进度）
    qint64 totalSize = 0;
    for (const QString& relativePath : relativeFilePaths) {
        QString localFilePath = QDir(localFolderPath).filePath(relativePath);
        totalSize += QFileInfo(localFilePath).size();
    }
    m_fileSize = static_cast<curl_off_t>(totalSize); // 用于进度计算
    qDebug() << "[curl] 文件夹上传：总文件数=" << relativeFilePaths.size() << "，总大小=" << totalSize << "字节";

    // 4. 遍历所有文件，逐文件上传
    qint64 uploadedSize = 0;
    for (int i = 0; i < relativeFilePaths.size(); ++i) {
        const QString& relativePath = relativeFilePaths[i];
        QString localFilePath = QDir(localFolderPath).filePath(relativePath);
        QString remoteFilePath = QDir(remoteRootPath).filePath(relativePath);

        // 4.1 提取远程文件对应的目录，确保目录存在
        QFileInfo remoteFileInfo(remoteFilePath);
        QString remoteDir = remoteFileInfo.path();
        bool dirExists = curlCheckRemotePathExists(host, port, username, password, remoteDir, timeout);
        if (!dirExists) {
            if (!curlCreateRemoteDir(host, port, username, password, remoteDir, timeout)) {
                m_lastError = QString("创建远程目录失败：%1（文件：%2）").arg(remoteDir).arg(relativePath);
                return false;
            }
        }

        // 4.2 上传单个文件（复用现有 curlUpload 方法）
        bool fileUploadSuccess = curlUpload(host, port, username, password, localFilePath, remoteFilePath, timeout);
        if (!fileUploadSuccess) {
            m_lastError = QString("文件上传失败：%1 → %2（原因：%3）").arg(localFilePath).arg(remoteFilePath).arg(m_lastError);
            return false;
        }

        // 4.3 更新已上传大小，发送聚合进度
        uploadedSize += QFileInfo(localFilePath).size();
        int overallProgress = static_cast<int>((static_cast<double>(uploadedSize) / totalSize) * 100.0);
        emit uploadProgress(overallProgress);
        qDebug() << "[curl] 文件夹上传进度：" << overallProgress << "%（已上传" << i + 1 << "/" << relativeFilePaths.size() << "个文件）";
    }

    qDebug() << "[curl] 文件夹上传成功：" << localFolderPath << "→" << remoteRootPath;
    return true;
}

bool CurlWorker::curlDelete(const QString& host, int port, const QString& username, const QString& password, const QString& remotePath, FtpDeleteResult::DeleteType deleteType, int timeout)
{
    QMutexLocker locker(&m_curlMutex);

    if (!m_curl) {
        m_curl = curl_easy_init();
        if (!m_curl) {
            m_lastError = "curl句柄初始化失败";
            qDebug() << "[curl] 删除失败：" << m_lastError;
            return false;
        }
    }

    // 1. 基础校验
    if (host.isEmpty() || port <= 0 || username.isEmpty() || password.isEmpty() || remotePath.isEmpty()) {
        m_lastError = "删除参数无效（空值/非法端口）";
        qDebug() << "[curl] 删除失败：" << m_lastError;
        return false;
    }

    // 2. 使用绝对路径（确保以/开头）
    QString absolutePath = remotePath.trimmed();
    if (!absolutePath.startsWith("/")) {
        absolutePath = "/" + absolutePath;
    }

    qDebug() << "[删除] 绝对路径：" << absolutePath;

    // 3. 构建FTP URL（只包含主机和端口）
    QString ftpUrl = QString("ftp://%1:%2/").arg(host).arg(port);

    // 4. 重置curl配置
    curl_easy_reset(m_curl);

    // 5. 使用QUOTE命令发送绝对路径的DELE命令
    struct curl_slist* cmdList = nullptr;
    QString deleCmd = QString("DELE %1").arg(absolutePath);
    cmdList = curl_slist_append(cmdList, deleCmd.toUtf8().data());

    curl_easy_setopt(m_curl, CURLOPT_URL, ftpUrl.toUtf8().data());
    curl_easy_setopt(m_curl, CURLOPT_USERPWD, QString("%1:%2").arg(username).arg(password).toUtf8().data());
    curl_easy_setopt(m_curl, CURLOPT_QUOTE, cmdList);
    curl_easy_setopt(m_curl, CURLOPT_NOBODY, 1L); // 不传输数据体
    curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, (long)timeout);
    curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(m_curl, CURLOPT_FTP_USE_EPSV, 0L);
    curl_easy_setopt(m_curl, CURLOPT_PASSV, 1L);
    curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 0L); // 关闭调试日志（可选）

    // 6. 执行删除命令
    CURLcode res = CURLE_OK; // 明确初始化变量
    long respCode = 0;

    res = curl_easy_perform(m_curl);
    curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &respCode);

    qDebug() << "[curl] 删除结果：curl=" << res << "(" << curl_easy_strerror(res)
        << ")，响应码=" << respCode;

    // 7. 清理命令列表
    if (cmdList) {
        curl_slist_free_all(cmdList);
    }

    // 8. 判断结果
    if (res == CURLE_OK && (respCode == 250 || respCode == 226)) {
        qDebug() << "[curl] 删除成功：" << absolutePath;
        return true;
    }
    else {
        m_lastError = QString("删除失败：curl错误%1，响应码%2")
            .arg(curl_easy_strerror(res)).arg(respCode);
        qDebug() << "[curl] 删除失败：" << m_lastError;
        return false;
    }
}

QString CurlWorker::getFtpUserRootPath(const QString& host, int port, const QString& username, const QString& password, int timeout)
{
    QString userRootPath;
    char* pwdResponse = nullptr;

    // 连接到FTP服务器并发送PWD命令
    QString ftpUrl = QString("ftp://%1:%2/").arg(host).arg(port);

    curl_easy_reset(m_curl);
    curl_easy_setopt(m_curl, CURLOPT_URL, ftpUrl.toUtf8().data());
    curl_easy_setopt(m_curl, CURLOPT_USERPWD, QString("%1:%2").arg(username).arg(password).toUtf8().data());
    curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, "PWD");
    curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, 10);
    curl_easy_setopt(m_curl, CURLOPT_FTP_USE_EPSV, 0L);
    curl_easy_setopt(m_curl, CURLOPT_PASSV, 1L); // ：被动模式
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L); // 开启调试

   // 获取PWD响应
    CURLcode res = curl_easy_perform(m_curl);
    if (res == CURLE_OK) {
        // 解析PWD响应获取用户根目录
        res = curl_easy_getinfo(m_curl, CURLINFO_FTP_ENTRY_PATH, &pwdResponse);
        if (res == CURLE_OK && pwdResponse) {
            userRootPath = QString(pwdResponse);
            qDebug() << "[FTP] 用户根目录：" << userRootPath;
        }
        else {
            // 备选方案：解析响应体获取PWD结果
            long responseCode = 0;
            curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &responseCode);
            if (responseCode == 257) {
                // 257响应格式：257 "/home/user" is current directory
                QString responseBody;
                // 读取响应体（需要回调函数）
                curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, [](void* ptr, size_t size, size_t nmemb, void* userdata) -> size_t {
                    QString* body = static_cast<QString*>(userdata);
                    *body += QString::fromUtf8(static_cast<char*>(ptr), size * nmemb);
                    return size * nmemb;
                    });
                curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &responseBody);
                curl_easy_perform(m_curl);
                // 提取路径
                QRegExp rx(R"delimiter("([^"]+)")delimiter");
                    if (rx.indexIn(responseBody) != -1) {
                        userRootPath = rx.cap(1);
                        qDebug() << "[FTP] 从响应体解析用户根目录：" << userRootPath;
                    }
            }
        }
    }
    else {
        qDebug() << "[FTP] 获取用户根目录失败：" << curl_easy_strerror(res);
    }
    return userRootPath;
}

curl_off_t CurlWorker::getFtpFileSize(CURL* curl, const QString& ftpUrl, const QString& userPwd)
{
    curl_easy_reset(curl);
    curl_easy_setopt(curl, CURLOPT_URL, ftpUrl.toUtf8().data());
    curl_easy_setopt(curl, CURLOPT_USERPWD, userPwd.toUtf8().data());
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L); // 只获取文件头，不下载内容
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 10000L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) return -1;

    curl_off_t fileSize = 0;
    curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &fileSize);
    return fileSize;
}

bool CurlWorker::curlVerifyPathByPwd(const QString& host, int port, const QString& username, const QString& password, const QString& remoteDir, int timeout)
{
    // 构建完整的FTP URL（包含要验证的目录）
    QString encodedRemoteDir = QUrl::toPercentEncoding(remoteDir, "/");
    QString ftpUrl = QString("ftp://%1:%2/%3").arg(host).arg(port).arg(encodedRemoteDir);
    if (!ftpUrl.endsWith("/")) ftpUrl += "/";

    curl_easy_reset(m_curl);

    // 先尝试CWD到目标目录，再执行PWD验证
    curl_easy_setopt(m_curl, CURLOPT_URL, ftpUrl.toUtf8().data());
    curl_easy_setopt(m_curl, CURLOPT_USERPWD, QString("%1:%2").arg(username).arg(password).toUtf8().data());
    curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, "CWD"); // 先切换到目标目录
    curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, 5);
    curl_easy_setopt(m_curl, CURLOPT_FTP_USE_EPSV, 0L);
    curl_easy_setopt(m_curl, CURLOPT_PASSV, 1L);
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(m_curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L); // 开启调试日志，查看详细交互

    // 执行CWD命令
    CURLcode res = curl_easy_perform(m_curl);
    if (res != CURLE_OK) {
        qDebug() << "[curl] 二次验证CWD命令失败：" << curl_easy_strerror(res);

        // 如果CWD失败，尝试直接PWD根目录验证连接
        ftpUrl = QString("ftp://%1:%2/").arg(host).arg(port);
        curl_easy_setopt(m_curl, CURLOPT_URL, ftpUrl.toUtf8().data());
        curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, "PWD");
        res = curl_easy_perform(m_curl);

        if (res != CURLE_OK) {
            qDebug() << "[curl] 二次验证PWD命令失败：" << curl_easy_strerror(res);
            return false;
        }
    }

    // 获取响应码
    long pwdResponseCode = 0;
    curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &pwdResponseCode);

    // 250=CWD成功，257=PWD成功，都表示目录/连接有效
    bool success = (pwdResponseCode == 250 || pwdResponseCode == 257 || pwdResponseCode == 0);

    if (success && pwdResponseCode == 0) {
        qDebug() << "[curl] 二次验证：响应码为0但命令执行成功，判定目录存在";
    }

    return success;
}

bool CurlWorker::verifyFileExists(const QString& host, int port, const QString& username, const QString& password, const QString& remotePath, int timeout)
{
    // 构建LIST命令来检查文件是否存在
    QString ftpUrl = QString("ftp://%1:%2/").arg(host).arg(port);
    QString parentDir = QFileInfo(remotePath).path();
    QString fileName = QFileInfo(remotePath).fileName();

    if (parentDir == ".") {
        parentDir = "";
    }

    // 首先切换到父目录
    struct curl_slist* cmdList = nullptr;
    if (!parentDir.isEmpty()) {
        QString cwdCmd = QString("CWD %1").arg(parentDir);
        cmdList = curl_slist_append(cmdList, cwdCmd.toUtf8().data());
    }

    // 然后列出文件
    cmdList = curl_slist_append(cmdList, "LIST");

    curl_easy_reset(m_curl);
    curl_easy_setopt(m_curl, CURLOPT_URL, ftpUrl.toUtf8().data());
    curl_easy_setopt(m_curl, CURLOPT_USERPWD, QString("%1:%2").arg(username).arg(password).toUtf8().data());
    curl_easy_setopt(m_curl, CURLOPT_QUOTE, cmdList);
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, [](void* ptr, size_t size, size_t nmemb, void* userdata) -> size_t {
        QByteArray* buffer = static_cast<QByteArray*>(userdata);
        buffer->append(static_cast<char*>(ptr), size * nmemb);
        return size * nmemb;
        });

    QByteArray listOutput;
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &listOutput);
    curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, (long)timeout);
    curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(m_curl, CURLOPT_FTP_USE_EPSV, 0L);
    curl_easy_setopt(m_curl, CURLOPT_PASSV, 1L);

    CURLcode res = curl_easy_perform(m_curl);

    if (cmdList) {
        curl_slist_free_all(cmdList);
    }

    if (res != CURLE_OK) {
        return false;
    }

    // 检查输出中是否包含目标文件名
    QString listStr = QString::fromUtf8(listOutput);
    QStringList lines = listStr.split("\n", Qt::SkipEmptyParts);

    for (const QString& line : lines) {
        if (line.contains(fileName, Qt::CaseInsensitive)) {
            return true; // 文件存在
        }
    }

    return false; // 文件不存在
}

QStringList CurlWorker::curlGetAllFilesInFolder(const QString& folderPath, QStringList& relativePaths)
{
    QDir dir(folderPath);
    if (!dir.exists()) return relativePaths;

    // 遍历当前目录下的所有文件和子目录
    QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden);
    for (const QFileInfo& fileInfo : fileInfoList) {
        if (fileInfo.isDir()) {
            // 递归处理子目录
            curlGetAllFilesInFolder(fileInfo.filePath(), relativePaths);
        }
        else if (fileInfo.isFile()) {
            // 计算文件相对于根文件夹的相对路径
            QString relativePath = dir.relativeFilePath(fileInfo.filePath());
            relativePaths.append(relativePath);
        }
    }
    return relativePaths;
}

// ---------------------- WiredNetworkChannel 实现（保持原有业务逻辑，兼容优化后 curl） ----------------------
WiredNetworkChannel::WiredNetworkChannel(QObject* parent)
    : CommunicationChannelBase(CommunicationChannel::WiredNetwork, parent)
    , m_currentOpType(FtpOpType::Upload)
    , m_transferTimer(new QTimer(this))
{
    // 初始化 curl 工作线程（异步执行 FTP 操作，避免阻塞主线程）
    m_curlWorker = new CurlWorker(this);
    connect(m_curlWorker, &CurlWorker::taskFinished, this, &WiredNetworkChannel::onCurlTaskFinished);
    connect(m_curlWorker, &CurlWorker::uploadProgress, this, &WiredNetworkChannel::onCurlUploadProgress);
    connect(m_curlWorker, &CurlWorker::downloadProgress, this, &WiredNetworkChannel::onCurlDownloadProgress);
    connect(m_curlWorker, &CurlWorker::fileListReceived, this, [this](const QStringList& files) {
        m_fetchedFileList = files;

        });

    // 原有跨线程信号连接（保持线程安全）
    connect(this, &CommunicationChannelBase::setConfigRequested, this, &WiredNetworkChannel::setConfigImpl, Qt::QueuedConnection);
    connect(this, &CommunicationChannelBase::startRequested, this, &WiredNetworkChannel::startImpl, Qt::QueuedConnection);
    connect(this, &CommunicationChannelBase::stopRequested, this, &WiredNetworkChannel::stopImpl, Qt::QueuedConnection);

    // 连接跨线程触发信号到实现槽函数
    connect(this, &WiredNetworkChannel::sendFileRequested, this, &WiredNetworkChannel::sendFileImpl, Qt::QueuedConnection);
    connect(this, &WiredNetworkChannel::downloadFileRequested, this, &WiredNetworkChannel::downloadImpl, Qt::QueuedConnection);
    connect(this, &WiredNetworkChannel::listFilesRequested, this, &WiredNetworkChannel::listFilesImpl, Qt::QueuedConnection);
    connect(this, &WiredNetworkChannel::deleteFileRequested,this, &WiredNetworkChannel::deleteFileImpl, Qt::QueuedConnection);
    // 配置上传超时定时器（单次触发）
    m_transferTimer->setSingleShot(true);
    connect(m_transferTimer, &QTimer::timeout, this, &WiredNetworkChannel::onTransferTimeout);
}

WiredNetworkChannel::~WiredNetworkChannel() {
    stop();               // 停止通道
    releaseCurlWorker();  // 释放 curl 工作线程
    delete m_transferTimer; // 释放超时定时器
}

// 设置配置（保持原有逻辑）
bool WiredNetworkChannel::setConfig(const ChannelConfigPtr& config) {
    emit setConfigRequested(config);
    return true;
}

// 获取当前配置（保持原有逻辑）
ChannelConfigPtr WiredNetworkChannel::config() const {
    return m_ftpConfig;
}

// 启动通道（保持原有逻辑）
bool WiredNetworkChannel::start() {
    if (isRunning()) {
        emit startFinished(true);
        return true;
    }
    emit startRequested();
    return true;
}

// 停止通道（保持原有逻辑）
void WiredNetworkChannel::stop() {
    if (!isRunning() && !m_isUploading) {
        emit stopFinished();
        return;
    }
    emit stopRequested();
}

// 检查通道运行状态（保持原有逻辑）
bool WiredNetworkChannel::isRunning() const {
    return m_running && m_ftpConfig && m_isReady && !m_isUploading;
}

// 获取最后错误信息（保持原有逻辑）
QString WiredNetworkChannel::lastError() const {
    return m_lastError;
}

// 获取通道状态描述（保持原有逻辑）
QString WiredNetworkChannel::statusInfo() const {
    if (!m_running) return "FTP通道（curl）：已停止";
    if (!m_ftpConfig) return "FTP通道（curl）：配置未设置";
    if (m_isUploading) return QString("FTP通道（curl）：正在上传 %1").arg(m_currentRemotePath);
    if (m_isDownloading) return QString("FTP通道（curl）：正在下载 %1→%2").arg(m_currentRemotePath).arg(m_currentLocalSavePath);
    if (!m_isReady) return QString("FTP通道（curl）：未登录（服务器：%1:%2）").arg(m_ftpConfig->m_host).arg(m_ftpConfig->m_port);
    return QString("FTP通道（curl）：已登录就绪（服务器：%1:%2）").arg(m_ftpConfig->m_host).arg(m_ftpConfig->m_port);
}

bool WiredNetworkChannel::sendFtpFile(const QVariantMap& params)
{
    if (!m_running || !m_isReady) {
        m_lastError = "FTP通道未就绪，无法发送文件";
        return false;
    }
    emit sendFileRequested(params); // 转发到通道线程处理
    return true;
}

bool WiredNetworkChannel::downloadFtpFile(const QVariantMap& params)
{
    if (!m_running || !m_isReady) {
        m_lastError = "FTP通道未就绪，无法下载文件";
        return false;
    }
    emit downloadFileRequested(params); // 转发到通道线程处理
    return true;
}

bool WiredNetworkChannel::listFtpFiles(const QVariantMap& params)
{
    if (!m_running || !m_isReady) {
        m_lastError = "FTP通道未就绪，无法获取文件列表";
        return false;
    }
    emit listFilesRequested(params); // 转发到通道线程处理
    return true;
}

bool WiredNetworkChannel::deleteFtpItem(const QVariantMap& params)
{
    if (!m_running || !m_isReady) {
        m_lastError = "FTP通道未就绪，无法删除文件/目录";
        return false;
    }
    emit deleteFileRequested(params); // 转发到通道线程处理
    return true;
}

// 应用配置（保持原有逻辑）
void WiredNetworkChannel::setConfigImpl(const ChannelConfigPtr& config) {
    auto ftpConfig = qSharedPointerDynamicCast<WiredNetworkConfig>(config);
    if (!ftpConfig) {
        m_lastError = "配置无效：非WiredNetworkConfig类型";
        emit errorOccurred(m_lastError);
        emit setConfigFinished(false);
        return;
    }

    if (ftpConfig->m_host.isEmpty() || ftpConfig->m_port <= 0 || ftpConfig->m_port > 65535) {
        m_lastError = "FTP配置无效：主机或端口错误";
        emit errorOccurred(m_lastError);
        emit setConfigFinished(false);
        return;
    }

    m_ftpConfig = ftpConfig;
    m_isReady = false;  // 配置更新后需重新登录
    emit setConfigFinished(true);
    qDebug() << QString("FTP配置更新成功：%1:%2").arg(m_ftpConfig->m_host).arg(m_ftpConfig->m_port);
  
}

// 启动通道实现（保持原有逻辑，提交登录任务）
void WiredNetworkChannel::startImpl() {
    // 启动前强制重置异常状态
    m_isReady = false;
    m_running = false;

    if (!m_ftpConfig) {
        m_lastError = "FTP启动失败：配置未设置";
        emit errorOccurred(m_lastError);
        emit startFinished(false);
        return;
    }

    m_running = true;
    qDebug() << QString("初始化：FTP通道启动-正在登录服务器 %1:%2...").arg(m_ftpConfig->m_host).arg(m_ftpConfig->m_port);

    CurlWorker::Task loginTask;
    loginTask.type = CurlWorker::TaskType::Login;
    loginTask.host = m_ftpConfig->m_host;
    loginTask.port = m_ftpConfig->m_port;
    loginTask.username = m_ftpConfig->m_ftpUsername;
    loginTask.password = m_ftpConfig->m_ftpPassword;
    loginTask.timeout = 30;

    m_curlWorker->setTask(loginTask);
    m_curlWorker->start();
}

// 停止通道实现（保持原有逻辑）
void WiredNetworkChannel::stopImpl() {
    // 1. 立即终止curl任务（线程安全）
    if (m_curlWorker && m_curlWorker->isRunning()) {
        m_curlWorker->stopTask();
        m_curlWorker->wait(3000); // 最多等待3秒，避免无限阻塞
        if (m_curlWorker->isRunning()) {
            m_curlWorker->terminate(); // 强制终止（兜底）
            m_curlWorker->wait(1000);
        }
    }

    // 2. 强制重置所有状态（关键：解决再次连接无反应）
    m_running = false;
    m_isReady = false;
    m_isUploading = false;
    m_isDownloading = false;
    m_currentRemotePath.clear();
    m_currentLocalSavePath.clear();
    m_lastError.clear();
    m_lastUploadProgress = 0;

    // 3. 提交断开连接任务（仅当curl句柄有效时）
    if (m_ftpConfig && m_curlWorker) {
        CurlWorker::Task disconnectTask;
        disconnectTask.type = CurlWorker::TaskType::Disconnect;
        disconnectTask.host = m_ftpConfig->m_host;
        disconnectTask.port = m_ftpConfig->m_port;
        disconnectTask.username = m_ftpConfig->m_ftpUsername;
        disconnectTask.password = m_ftpConfig->m_ftpPassword;
        disconnectTask.timeout = 5;

        m_curlWorker->setTask(disconnectTask);
        m_curlWorker->start();
        m_curlWorker->wait(2000); // 等待断开任务完成
    }

    emit statusChanged("FTP通道已停止");
    emit stopFinished();
}

// 带参数发送数据（上传文件，保持原有业务逻辑）
void WiredNetworkChannel::sendFileImpl(const QVariantMap& params) {
    if (!m_running || !m_isReady || m_isUploading) {
        m_lastError = "FTP传输失败：通道未就绪或正在上传";
        emit errorOccurred(m_lastError);
        emit ftpFileSent(false);
        return;
    }

    // 提取参数（本地路径、远程路径、来源模块）
    QString localPath = params["localPath"].toString();
    QString remotePath = params["remotePath"].toString();
    ModuleType sourceModule = static_cast<ModuleType>(params["sourceModule"].toInt());

    // 仅处理任务规划模块的上传请求
    FtpOpType opType = FtpOpType::Upload;
    //if (sourceModule != ModuleType::TaskPlanning || !parseTaskPlanningData(localPath, remotePath, opType)) {
    //    emit ftpFileSent(false);
    //    return;
    //}

    // 提交上传任务给 curl 线程
    CurlWorker::Task uploadTask;
    uploadTask.type = CurlWorker::TaskType::Upload;
    uploadTask.host = m_ftpConfig->m_host;
    uploadTask.port = m_ftpConfig->m_port;
    uploadTask.username = m_ftpConfig->m_ftpUsername;
    uploadTask.password = m_ftpConfig->m_ftpPassword;
    uploadTask.localPath = localPath;
    uploadTask.remotePath = remotePath;
    uploadTask.timeout = m_ftpConfig->m_timeout / 1000;  // 转换为秒（curl 超时单位为秒）

    // 更新状态
    m_isUploading = true;
    m_currentRemotePath = remotePath;
    qDebug() << QString("FTP上传开始：%1 → %2").arg(localPath).arg(remotePath);
  

    // 启动上传线程和超时定时器
    m_curlWorker->setTask(uploadTask);
    m_curlWorker->start();
    m_transferTimer->start(m_ftpConfig->m_timeout);
}

void WiredNetworkChannel::downloadImpl(const QVariantMap& params)
{
    if (!m_running || !m_isReady || m_isUploading || m_isDownloading) {
        m_lastError = "FTP下载失败：通道未就绪或正在执行其他操作";
        emit errorOccurred(m_lastError);
        emit ftpFileDownloaded(false); // 复用完成信号（或 downloadFinished）
        return;
    }

    // 提取下载参数（远程文件路径、本地保存路径、来源模块）
    QString remotePath = params["remotePath"].toString();
    QString localSavePath = params["localSavePath"].toString();
    ModuleType sourceModule = static_cast<ModuleType>(params["sourceModule"].toInt());

    // ========== ：调试日志 ==========
    qDebug() << "[下载参数] 远程路径：" << remotePath;
    qDebug() << "[下载参数] 本地路径：" << localSavePath;
    qDebug() << "[下载参数] 来源模块：" << static_cast<int>(sourceModule);

    // 路径校验（远程路径非空、本地保存路径非空）
    if (remotePath.isEmpty() || localSavePath.isEmpty()) {
        m_lastError = "下载参数错误：远程路径或本地保存路径为空";
        emit errorOccurred(m_lastError);
        emit ftpFileDownloaded(false);
        return;
    }

    // 仅允许任务规划模块触发下载（按需调整）
    if (sourceModule != ModuleType::Monitoring) {
        m_lastError = "下载权限拒绝：仅任务规划模块可触发下载";
        emit errorOccurred(m_lastError);
        emit ftpFileDownloaded(false);
        return;
    }

    // 提交下载任务给 CurlWorker
    CurlWorker::Task downloadTask;
    downloadTask.type = CurlWorker::TaskType::Download;
    downloadTask.host = m_ftpConfig->m_host;
    downloadTask.port = m_ftpConfig->m_port;
    downloadTask.username = m_ftpConfig->m_ftpUsername;
    downloadTask.password = m_ftpConfig->m_ftpPassword;
    downloadTask.remotePath = remotePath; // 远程文件路径
    downloadTask.localPath = localSavePath; // 本地保存路径
    downloadTask.timeout = m_ftpConfig->m_timeout / 1000; // 转换为秒

    // 更新下载状态
    m_isDownloading = true;
    m_currentRemotePath = remotePath;
    m_currentLocalSavePath = localSavePath;
    qDebug() << QString("FTP下载开始：%1 → %2").arg(remotePath).arg(localSavePath);

    // 启动下载线程和超时定时器
    m_curlWorker->setTask(downloadTask);
    m_curlWorker->start();
    m_transferTimer->start(m_ftpConfig->m_timeout);
}

void WiredNetworkChannel::listFilesImpl(const QVariantMap& params)
{
    m_fetchedFileList.clear();
    qDebug() << "[列表刷新] 清空旧缓存，准备请求新列表";

    QString remotePath = params["remotePath"].toString();
    bool recursive = params.value("recursive", false).toBool(); // 递归参数
    ModuleType sourceModule = static_cast<ModuleType>(params["sourceModule"].toInt());

    if (remotePath.isEmpty() || sourceModule == ModuleType::Unknown) {
        m_lastError = "获取文件列表失败：参数无效";
        emit ftpFileListFetched(false, {}, m_lastError);
        return;
    }
    if (m_curlWorker->isRunning()) {
        m_curlWorker->stopTask();
        m_curlWorker->wait(1000); // 等待旧任务终止
    }
    // 提交任务给 CurlWorker
    CurlWorker::Task listTask;
    listTask.type = CurlWorker::TaskType::ListFiles;
    listTask.host = m_ftpConfig->m_host;
    listTask.port = m_ftpConfig->m_port;
    listTask.username = m_ftpConfig->m_ftpUsername;
    listTask.password = m_ftpConfig->m_ftpPassword;
    listTask.remotePath = remotePath;
    listTask.timeout = m_ftpConfig->m_timeout / 1000;
    listTask.recursiveList = recursive;
    // 存储递归标志（可以扩展Task结构体添加recursive字段）
   
    m_currentRemotePath = remotePath;
    m_curlWorker->setTask(listTask);
    m_curlWorker->start();
    m_transferTimer->start(m_ftpConfig->m_timeout);
}

bool WiredNetworkChannel::uploadFtpFolderImpl(const QVariantMap& params)
{
    if (!m_running || !m_isReady || m_isUploading) {
        m_lastError = "FTP通道未就绪或正在上传";
        emit errorOccurred(m_lastError);
        return false;
    }

    // 提取参数：本地文件夹路径、远程根目录路径、来源模块
    QString localFolderPath = params["localFolderPath"].toString();
    QString remoteRootPath = params["remoteRootPath"].toString();
    ModuleType sourceModule = static_cast<ModuleType>(params["sourceModule"].toInt());

    // 参数校验
    if (localFolderPath.isEmpty() || remoteRootPath.isEmpty()) {
        m_lastError = "文件夹上传参数错误：本地路径或远程路径为空";
        emit errorOccurred(m_lastError);
        return false;
    }

    // 提交文件夹上传任务给 CurlWorker
    CurlWorker::Task uploadFolderTask;
    uploadFolderTask.type = CurlWorker::TaskType::UploadFolder;
    uploadFolderTask.host = m_ftpConfig->m_host;
    uploadFolderTask.port = m_ftpConfig->m_port;
    uploadFolderTask.username = m_ftpConfig->m_ftpUsername;
    uploadFolderTask.password = m_ftpConfig->m_ftpPassword;
    uploadFolderTask.localPath = localFolderPath;
    uploadFolderTask.remotePath = remoteRootPath;
    uploadFolderTask.timeout = m_ftpConfig->m_timeout / 1000;

    // 更新状态
    m_isUploading = true;
    m_currentRemotePath = remoteRootPath;
    qDebug() << QString("FTP文件夹上传开始：%1 → %2").arg(localFolderPath).arg(remoteRootPath);

    // 启动任务和超时定时器
    m_curlWorker->setTask(uploadFolderTask);
    m_curlWorker->start();
    m_transferTimer->start(m_ftpConfig->m_timeout);

    return true;
}

void WiredNetworkChannel::deleteFileImpl(const QVariantMap& params)
{
    if (!m_running || !m_isReady) {
        m_lastError = "FTP删除失败：通道未就绪";
        emit ftpFileDeleted(false);
        return;
    }

    // 提取删除参数
    QString remotePath = params["remotePath"].toString();
    FtpDeleteResult::DeleteType deleteType = static_cast<FtpDeleteResult::DeleteType>(params["deleteType"].toInt());
    ModuleType sourceModule = static_cast<ModuleType>(params["sourceModule"].toInt());

    // 参数校验
    if (remotePath.isEmpty()) {
        m_lastError = "删除参数错误：远程路径为空";
        emit errorOccurred(m_lastError);
        emit ftpFileDeleted(false);
        return;
    }

    // 权限控制（仅允许指定模块删除）
    if (sourceModule != ModuleType::Data && sourceModule != ModuleType::TaskPlanning) {
        m_lastError = "删除权限拒绝：仅数据/任务规划模块可触发删除";
        emit errorOccurred(m_lastError);
        emit ftpFileDeleted(false);
        return;
    }

    // 提交删除任务给CurlWorker
    CurlWorker::Task deleteTask;
    deleteTask.type = CurlWorker::TaskType::Delete;
    deleteTask.host = m_ftpConfig->m_host;
    deleteTask.port = m_ftpConfig->m_port;
    deleteTask.username = m_ftpConfig->m_ftpUsername;
    deleteTask.password = m_ftpConfig->m_ftpPassword;
    deleteTask.remotePath = remotePath;
    deleteTask.deleteType = deleteType; // 设置删除类型
    deleteTask.timeout = m_ftpConfig->m_timeout / 1000;

    // 更新状态
    m_currentRemotePath = remotePath;
    m_currentDeleteType = deleteType;
    qDebug() << QString("FTP删除开始：%1（类型：%2）").arg(remotePath)
        .arg(deleteType == FtpDeleteResult::DeleteType::File ? "文件" : "目录");

    // 启动删除线程和超时定时器
    m_curlWorker->setTask(deleteTask);
    m_curlWorker->start();
    m_transferTimer->start(m_ftpConfig->m_timeout);
}

// curl 任务完成回调（处理登录/上传结果）
void WiredNetworkChannel::onCurlTaskFinished(bool success, const QString& msg)
{
    m_transferTimer->stop();
    switch (m_curlWorker->getTaskType()) {
    case CurlWorker::TaskType::Login:
        m_isReady = success;
        m_lastError = success ? "" : msg;
        emit statusChanged(msg);
        emit startFinished(success);
         // ========== 登录成功后自动获取根目录文件列表 ==========
        if (success) {
            QVariantMap params;
            params["remotePath"] = "/"; // 默认获取根目录
            params["sourceModule"] = static_cast<int>(ModuleType::Data); // 数据模块请求
            params["recursive"] = false; // 设置为true获取完整结构
            listFtpFiles(params); // 自动触发文件列表获取
        }
        break;
    case CurlWorker::TaskType::Upload:
        m_isUploading = false;
        m_lastError = success ? "" : msg;
        emit ftpFileSent(success);
       
        break;
    case CurlWorker::TaskType::Download:
        m_isDownloading = false;
        m_lastError = success ? "" : msg;
        emit ftpFileDownloaded(success);
        m_currentLocalSavePath.clear();
        break;
    case CurlWorker::TaskType::ListFiles:
        qDebug() << "发射ftpFileListFetched信号，success=" << success
            << "文件数量=" << m_fetchedFileList.size()
            << "消息=" << msg;
        qDebug() << "[WiredNetworkChannel] Emitting thread:" << QThread::currentThreadId();
        // 确保在正确的线程发射信号
        if (thread() != QThread::currentThread()) {
            // 如果不在通道线程，使用invokeMethod确保线程安全
            QMetaObject::invokeMethod(this, [this, success, msg]() {
                emit ftpFileListFetched(success, m_fetchedFileList, msg);
                }, Qt::QueuedConnection);
        }
        else {
            // 直接发射信号
            emit ftpFileListFetched(success, m_fetchedFileList, msg);
        }
        m_currentRemotePath.clear();
        break;
    case CurlWorker::TaskType::UploadFolder:
        m_isUploading = false;
        m_lastError = success ? "" : msg;
        emit ftpFileSent(success);
        break;
    case CurlWorker::TaskType::Delete:
        m_lastError = success ? "" : msg;
        emit ftpFileDeleted(success);
        // 删除成功后刷新对应目录列表
        if (success && !m_currentRemotePath.isEmpty()) {
            // 延迟刷新列表，避免句柄冲突
            QTimer::singleShot(100, this, [this]() {
                QFileInfo remoteFileInfo(m_currentRemotePath);
                QString remoteDir = remoteFileInfo.path();
                QVariantMap listParams;
                listParams["remotePath"] = remoteDir;
                listParams["sourceModule"] = static_cast<int>(ModuleType::Monitoring);
                listParams["recursive"] = true;
                listFtpFiles(listParams); // 刷新列表
                });
        }
        break;
    }

}

// 上传进度回调（转发进度信号）
void WiredNetworkChannel::onCurlUploadProgress(int progress)
{
    // 确保进度值有效
    progress = qBound(0, progress, 100);
    // 仅当进度变化≥1% 时发射信号（避免频繁刷新）
    if (progress != m_lastUploadProgress)
    {
        qDebug() << QString("FTP上传进度：%1%").arg(progress);
        m_lastUploadProgress = progress; // 更新上次进度
           // ========== 发射上传进度信号 ==========
        emit uploadProgressUpdated(progress);
    }
}

void WiredNetworkChannel::onCurlDownloadProgress(int progress)
{
    progress = qBound(0, progress, 100);
    qDebug() << QString("FTP下载进度：%1%").arg(progress);
    // ========== 发射下载进度信号 ==========
    emit downloadProgressUpdated(progress);
    
}

// 上传超时处理（终止上传任务）
void WiredNetworkChannel::onTransferTimeout()
{
    if (m_isUploading && m_curlWorker->isRunning())
    {
        m_curlWorker->stopTask();
        m_isUploading = false;
        m_lastError = QString("FTP上传超时（%1毫秒）").arg(m_ftpConfig->m_timeout);
        emit errorOccurred(m_lastError);
        emit ftpFileSent(false);
    }
    else if (m_isDownloading && m_curlWorker->isRunning())
    {
        // 下载超时处理
        m_curlWorker->stopTask();
        m_isDownloading = false;
        m_lastError = QString("FTP下载超时（%1毫秒）").arg(m_ftpConfig->m_timeout);
        emit errorOccurred(m_lastError);
        emit ftpFileDownloaded(false);
    }
    else if (m_curlWorker->getTaskType() == CurlWorker::TaskType::ListFiles) {
        m_curlWorker->stopTask();
        m_lastError = "获取文件列表超时";
        emit errorOccurred(m_lastError);
        emit ftpFileListFetched(false, {}, m_lastError);
    }
    else if (m_curlWorker->getTaskType() == CurlWorker::TaskType::Delete) {
        m_curlWorker->stopTask();
        m_lastError = "删除操作超时";
        emit errorOccurred(m_lastError);
        emit ftpFileDeleted(false);
    }
}

// 任务规划模块路径解析（保持原有逻辑）
bool WiredNetworkChannel::parseTaskPlanningData(const QString& localPath, const QString& remotePath, FtpOpType& opType) {
    opType = FtpOpType::Upload; // 固定为上传
    if (localPath.isEmpty() || remotePath.isEmpty() || !localPath.endsWith(".json", Qt::CaseInsensitive)) {
        m_lastError = "任务规划模块解析失败：路径为空或非JSON文件";
        emit errorOccurred(m_lastError);
        return false;
    }
    return true;
}

// 释放 curl 工作线程（避免内存泄漏）
void WiredNetworkChannel::releaseCurlWorker() {
    if (m_curlWorker) {
        m_curlWorker->stopTask();
        m_curlWorker->wait();
        delete m_curlWorker;
        m_curlWorker = nullptr;
    }
}