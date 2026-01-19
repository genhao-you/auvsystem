#include "datamodule.h"
#include<QJsonDocument>
#include<qDebug>
#include<QMessageBox>
#include<QJsonArray>
DataModule::DataModule(QObject *parent)
	: BaseModule(ModuleType::Data, "Data", parent)
{
    initialize();
}

DataModule::~DataModule()
{

}

bool DataModule::initialize()
{
    subscribeStatus("channel_status",
        [this](const DataMessage& msg) {
            onChannelStatusReceived(msg);
        });
    subscribeEvent("ftp_file_list_result",
        [this](const DataMessage& msg) {
            onFtpFileListReceived(msg);
        });
    subscribeEvent("file_transfer_result",
        [this](const DataMessage& msg) {
            onFtpTransferReceived(msg);
        });
    subscribeEvent("ftp_transfer_progress",
        [this](const DataMessage& msg) {
            onFtpTransferProgressReceived(msg);
        });
	return false;
}

void DataModule::shutdown()
{
}

CheckError DataModule::logIn(const QString& host, const QString& port, const QString& user, const QString& pwd)
{
    // 构建有线网络配置JSON（复用WiredNetworkConfig类）
    int timeout = 86400000; // 超时时间（毫秒）
    bool ftpPassiveMode = true; // FTP被动模式（推荐开启）
    WiredNetworkConfig wiredConfig;
    wiredConfig.m_host = host;
    wiredConfig.m_port = port.toInt();
    wiredConfig.m_timeout = timeout;
    wiredConfig.m_ftpUsername = user;
    wiredConfig.m_ftpPassword = pwd;
    wiredConfig.m_ftpPassiveMode = ftpPassiveMode;
    QJsonObject wiredConfigJson = wiredConfig.toJson(); // 利用已有toJson()方法

    // 构建命令参数（与Radio格式一致）
    QJsonObject cmdParams;
    cmdParams["channel"] = static_cast<int>(CommunicationChannel::WiredNetwork); // 假设枚举有WiredNetwork
    cmdParams["config"] = wiredConfigJson;

    // 发送配置命令到CommunicationModule（与Radio逻辑一致）
    sendCommand(
        "CommunicationModule",          // 目标模块名（与Radio保持一致）
        CommunicationChannel::WiredNetwork, // 有线网络通道
        "set_config",                   // 命令：设置配置（与Radio一致）
        cmdParams);

    return CheckError();
}

CheckError DataModule::logOut()
{
    // 发送停止通信通道命令（触发断开连接）
    QJsonObject cmdParams;
    cmdParams["channel"] = static_cast<int>(CommunicationChannel::WiredNetwork);

    sendCommand(
        "CommunicationModule",
        CommunicationChannel::WiredNetwork,
        "stop",  // 对应之前实现的stopImpl()方法
        cmdParams);

    return CheckError();
}

CheckError DataModule::downloadFile(const QString& localPath, const QString& remotePath)
{
    // 1. 生成全局唯一request_id
    uint64_t requestId = CommandIdGenerator::instance().nextId();
    // 2. 封装元数据（本地路径、远程路径）和JSON数据
    QJsonObject meta;
    meta["localPath"] = localPath;
    meta["remotePath"] = remotePath;
    meta["request_id"] = QString::number(requestId); // 携带request_id
    meta["source_module"] = static_cast<int>(ModuleType::Monitoring); // 携带发送模块标识
   // 直接传递元数据JSON，无需组合JSON内容
    QByteArray metaData = QJsonDocument(meta).toJson(QJsonDocument::Compact);

    //传输数据到通信模块
    // 3. 调用BaseModule的transferData，指定目标和通道
    transferData(
        CommunicationChannel::WiredNetwork, // 通信通道（如WiredNetwork）
        "file_download_transfer",       // 目标接收器标识（唯一）
        metaData                   // 直接传输的二进制数据
    );

    return CheckError();
}

CheckError DataModule::uploadFile(const QString& localPath, const QString& remotePath)
{
    // 1. 生成全局唯一request_id
    uint64_t requestId = CommandIdGenerator::instance().nextId();
    // 2. 封装元数据（本地路径、远程路径）和JSON数据
    QJsonObject meta;
    meta["localPath"] = localPath;
    meta["remotePath"] = remotePath;
    meta["request_id"] = QString::number(requestId); // 携带request_id
    meta["source_module"] = static_cast<int>(ModuleType::Monitoring); // 携带发送模块标识
   // 直接传递元数据JSON，无需组合JSON内容
    QByteArray metaData = QJsonDocument(meta).toJson(QJsonDocument::Compact);

    //传输数据到通信模块
    // 3. 调用BaseModule的transferData，指定目标和通道
    transferData(
        CommunicationChannel::WiredNetwork, // 通信通道（如WiredNetwork）
        "file_upload_transfer",       // 目标接收器标识（唯一）
        metaData                   // 直接传输的二进制数据
    );

    return CheckError();
}

CheckError DataModule::getRemoteFileList( const QString& remotePath)
{
    // 1. 生成全局唯一request_id
    uint64_t requestId = CommandIdGenerator::instance().nextId();
    // 2. 封装元数据（本地路径、远程路径）和JSON数据
    QJsonObject meta;
    meta["remotePath"] = remotePath;
    meta["request_id"] = QString::number(requestId); // 携带request_id
    meta["source_module"] = static_cast<int>(ModuleType::Data); // 携带发送模块标识
   // 直接传递元数据JSON，无需组合JSON内容
    QByteArray metaData = QJsonDocument(meta).toJson(QJsonDocument::Compact);

    //传输数据到通信模块
    // 3. 调用BaseModule的transferData，指定目标和通道
    transferData(
        CommunicationChannel::WiredNetwork, // 通信通道（如WiredNetwork）
        "filelist_get_transfer",       // 目标接收器标识（唯一）
        metaData                   // 直接传输的二进制数据
    );

    return CheckError();
}

CheckError DataModule::deleteRemoteItem(const QString& remotePath, FtpDeleteResult::DeleteType delType)
{
    // 1. 生成全局唯一request_id（与下载完全一致）
    uint64_t requestId = CommandIdGenerator::instance().nextId();

    // 2. 封装元数据（对齐下载的meta结构）
    QJsonObject meta;
    meta["remotePath"] = remotePath;                  // 删除目标路径
    meta["deleteType"] = static_cast<int>(delType);   // 删除类型（文件/目录）
    meta["request_id"] = QString::number(requestId);  // 请求ID
    meta["source_module"] = static_cast<int>(ModuleType::Data); // 来源模块

    // 3. 转换为二进制数据（与下载一致）
    QByteArray metaData = QJsonDocument(meta).toJson(QJsonDocument::Compact);

    // 4. 传输到通信模块（对齐下载的transferData，仅修改接收器标识）
    transferData(
        CommunicationChannel::WiredNetwork,  // 通信通道（与下载一致）
        "file_delete_transfer",              // 删除专属接收器标识
        metaData                             // 元数据
    );

    return CheckError();
}

void DataModule::onChannelStatusReceived(const DataMessage& msg)
{
    QJsonObject outerJson;
    QJsonDocument outerDoc = QJsonDocument::fromJson(msg.data());
    if (outerDoc.isNull()) {
        qWarning() << "JSON解析失败：" << msg.data();
        return ;
    }
    outerJson = outerDoc.object();

    if (!outerJson.contains("data") || !outerJson["data"].isObject()) {
        qWarning() << "状态信息缺少data字段";
        return;
    }
    QJsonObject data = outerJson["data"].toObject();
    CommunicationChannel channel = static_cast<CommunicationChannel>(data["channel"].toInt());
    QString statusDesc = data["status"].toString();

    CommunicationChannelStatus newStatus = CommunicationChannelStatus::Stopped;
    if (statusDesc == "运行中") {
        newStatus = CommunicationChannelStatus::Running;
    }
    else if (statusDesc == "初始化中") {
        newStatus = CommunicationChannelStatus::Initializing;
    }
  
    CommunicationChannelStatusResult result(channel, newStatus);
    notifyObservers(&result);
    qInfo() << result.description();
}

void DataModule::onFtpTransferReceived(const DataMessage& msg)
{
    QJsonObject outerJson;
    QJsonDocument outerDoc = QJsonDocument::fromJson(msg.data());
    if (outerDoc.isNull()) {
        qWarning() << "FTP文件列表数据JSON解析失败：" << msg.data();
        return;
    }
    outerJson = outerDoc.object();

    // 2. 提取内层data字段
    if (!outerJson.contains("data") || !outerJson["data"].isObject()) {
        qWarning() << "FTP文件列表数据缺少data字段";
        return;
    }
    QJsonObject data = outerJson["data"].toObject();

    // 2. 提取内层data字段（注意：根据CommunicationModule的publish逻辑，这里直接解析外层即可）
    bool success = data["success"].toBool();
    QString message = data["message"].toString();
    QString operationType = data["operation_type"].toString();
    CommunicationChannel channel = static_cast<CommunicationChannel>(data["channel"].toInt(-1));

    // 3. 确定传输类型
    FtpTransferResult::TransferType transferType =
        (operationType == "upload") ? FtpTransferResult::TransferType::Upload :
        FtpTransferResult::TransferType::Download;

    // 4. 创建FtpTransferResult对象（参数匹配构造函数）
    FtpTransferResult result(
        channel,                      // 通信通道
        transferType,                 // 传输类型
        success,                      // 是否成功
        message                       // 错误信息（成功时为空）
    );

    // 5. 通知观察者（DataDownloadDialog）
    notifyObservers(&result);

    // 6. 日志输出
    qInfo() << result.description();
   
}

void DataModule::onFtpFileListReceived(const DataMessage& msg)
{
    // 1. 解析外层JSON数据
    QJsonObject outerJson;
    QJsonDocument outerDoc = QJsonDocument::fromJson(msg.data());
    if (outerDoc.isNull()) {
        qWarning() << "FTP文件列表数据JSON解析失败：" << msg.data();
        return;
    }
    outerJson = outerDoc.object();

    // 2. 提取内层data字段
    if (!outerJson.contains("data") || !outerJson["data"].isObject()) {
        qWarning() << "FTP文件列表数据缺少data字段";
        return;
    }
    QJsonObject data = outerJson["data"].toObject();

    // 3. 解析核心字段
    CommunicationChannel channel = static_cast<CommunicationChannel>(data["channel"].toInt());
    bool listSuccess = data["success"].toBool();
    QStringList fileList;
    if (data.contains("fileList") && data["fileList"].isArray()) {
        QJsonArray fileArray = data["fileList"].toArray();
        // 遍历数组并转换为QStringList
        for (const QJsonValue& value : fileArray) {
            if (value.isString()) {
                fileList.append(value.toString());
            }
        }
    }
    QString errorMsg = data["message"].toString();

    // 4. 登录状态：文件列表操作基于登录成功，所以loginSuccess为true
    bool loginSuccess = true;

    // 5. 创建结果对象
    FtpLoginListResult result(
        channel,
        loginSuccess,
        listSuccess,
        fileList,
        errorMsg
    );

    // 6. 通知观察者（DataDownloadDialog）
    notifyObservers(&result);

    // 7. 日志输出
    qInfo() << result.description();
}

void DataModule::onFtpTransferProgressReceived(const DataMessage& msg)
{
    // 步骤1：解析外层JSON（和文件列表解析逻辑完全一致）
    QJsonObject outerJson;
    QJsonDocument outerDoc = QJsonDocument::fromJson(msg.data());
    if (outerDoc.isNull()) {
        qWarning() << "FTP进度数据JSON解析失败：" << msg.data();
        return;
    }
    outerJson = outerDoc.object();

    // 步骤2：提取内层data字段（和文件列表解析逻辑完全一致）
    if (!outerJson.contains("data") || !outerJson["data"].isObject()) {
        qWarning() << "FTP进度数据缺少data字段";
        return;
    }
    QJsonObject data = outerJson["data"].toObject();

    // 步骤3：解析核心字段
    CommunicationChannel channel = static_cast<CommunicationChannel>(data["channel"].toInt());
    int progress = data["progress"].toInt(0); // 进度值，默认0
    QString opType = data["operationType"].toString(); // uploadProgress/downloadProgress

    FtpProgressResult::ProgressType progressType = FtpProgressResult::ProgressType::Upload;
    if (opType == "downloadProgress") {
        progressType = FtpProgressResult::ProgressType::Download;
    }
    // 步骤4：创建进度结果对象
    FtpProgressResult result(channel, progressType, progress);

    // 步骤5：通知观察者
    notifyObservers(&result);

    // 步骤6：日志输出
    qInfo() << result.description();
}

