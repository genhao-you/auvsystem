#include "datadownloaddialog.h"
#include<QMessageBox>
#include<QFileInfo>
#include<QLocale>
#include<QTimer>
#include<QStandardPaths>
// 实现RemoteFileInfo结构体的构造函数
RemoteFileInfo::RemoteFileInfo(const QString& infoStr) : size(0) {
    QStringList parts = infoStr.split("|");
    if (parts.size() >= 5) {
        fullPath = parts[0];
        size = parts[1].toLongLong();
        type = parts[2];
        permissions = parts[3];
        timeInfo = parts[4];

        // 从完整路径中提取文件名
        int lastSlash = fullPath.lastIndexOf('/');
        if (lastSlash >= 0) {
            name = fullPath.mid(lastSlash + 1);
        }
        else {
            name = fullPath;
        }
    }
}


DataDownloadDialog::DataDownloadDialog(DataModule* module, QWidget* parent)
    : ModuleDialogBase(parent)
    , m_dataModule(module)
    , m_isConnected(false)
{
    ui.setupUi(this);
    initialize();
}

DataDownloadDialog::~DataDownloadDialog()
{

}

void DataDownloadDialog::onResultUpdated(const BaseResult* result)
{
    if (!result)
        return;
    // 1. 尝试转换为通信通道状态结果
    if (const auto* channelResult = dynamic_cast<const CommunicationChannelStatusResult*>(result)) {
        handleChannelStatusResult(static_cast<const CommunicationChannelStatusResult*>(result));
    }
    // 2. 尝试转换为FTP登录+列表结果
    else if (const auto* ftpResult = dynamic_cast<const FtpLoginListResult*>(result)) {
        handleFtpLoginListResult(ftpResult);
    }
    // 3. 可选：处理FTP传输结果（上传/下载）
    else if (const auto* transferResult = dynamic_cast<const FtpTransferResult*>(result)) {
        handleFtpTransferResult(transferResult);
    }
    else if (const auto* progressResult = dynamic_cast<const FtpProgressResult*>(result)) {
        handleFtpProgressResult(progressResult);
    }
    // 4. 未知结果类型（日志提示）
    else {
        qWarning() << "未知的结果类型：" << typeid(*result).name();
    }

}

void DataDownloadDialog::initialize()
{
    if (!m_dataModule) {
        QMessageBox::critical(this, "错误", "自检模块未初始化");
        return;
    }
    m_dataModule->addObserver(this);
    loadStyleSheet(STYLE_DATADOWNLOAD);
    // 初始化UI和模型
    initUI();
    initModels();
}



void DataDownloadDialog::initUI()
{
    setWindowTitle("数据传输管理");
    ui.progressBar_Transfer->setVisible(false);
    showStatus("状态：未连接");

    // 默认远程路径
    m_currentRemotePath = "/";
    m_currentRemoteDir = "/";
    ui.lineEdit_RemotePath->setText(m_currentRemotePath);
    connect(ui.btn_Connect, &QPushButton::clicked,
        this, &DataDownloadDialog::on_connectBtn_clicked);
    connect(ui.btn_Disconnect, &QPushButton::clicked,
        this, &DataDownloadDialog::on_disconnectBtn_clicked);
    connect(ui.btn_Upload, &QPushButton::clicked,
        this, &DataDownloadDialog::on_uploadBtn_clciked);
    connect(ui.btn_Download, &QPushButton::clicked,
        this, &DataDownloadDialog::on_downloadBtn_clicked);
    connect(ui.btn_DeleteRemote, &QPushButton::clicked,
        this, &DataDownloadDialog::on_deleteRemote_clicked);
    // 连接刷新按钮
    connect(ui.btn_Refresh, &QPushButton::clicked,
        this, &DataDownloadDialog::on_refresh_clciked);
    connect(ui.treeView_Remote, &QTreeView::doubleClicked,
        this, &DataDownloadDialog::on_treeView_Remote_doubleClicked);
}

void DataDownloadDialog::initModels()
{
    // 使用自定义的中文文件系统模型
    ChineseFileSystemModel* localModel = new ChineseFileSystemModel(this);
    // 初始化本地根路径为用户当前目录（或自定义默认路径）
    QString defaultLocalPath = QDir::currentPath();
    localModel->setRootPath(defaultLocalPath);
    localModel->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
    m_localFileModel = localModel;

    // 设置本地文件视图：只显示 defaultLocalPath 下的内容，不随远程操作变化
    ui.treeView_Local->setModel(localModel);
    ui.treeView_Local->setRootIndex(localModel->index(defaultLocalPath));
    setupTreeViewAutoResize(ui.treeView_Local);
    ui.treeView_Local->setHeaderHidden(false);
  

    // 初始化远程文件模型（树形结构）
    m_remoteFileModel = new QStandardItemModel(this);
    m_remoteFileModel->setHorizontalHeaderLabels(QStringList() << "名称" << "类型" << "大小" << "修改时间");

    ui.treeView_Remote->setModel(m_remoteFileModel);
    ui.treeView_Remote->setHeaderHidden(false);
    // 启用树形视图的展开/折叠功能
    ui.treeView_Remote->setSelectionMode(QAbstractItemView::SingleSelection); // 单选模式
    ui.treeView_Remote->setSelectionBehavior(QAbstractItemView::SelectRows); // 整行选中
    ui.treeView_Remote->setItemsExpandable(true);
    ui.treeView_Remote->setExpandsOnDoubleClick(true);
    ui.treeView_Remote->setRootIsDecorated(true); // 显示根节点的展开/折叠图标
    // 设置缩进，让层级更明显
    ui.treeView_Remote->setIndentation(20);
    // 配置远程TreeView的自适应设置
    setupTreeViewAutoResize(ui.treeView_Remote);
}

void DataDownloadDialog::handleFtpProgressResult(const FtpProgressResult* result)
{
    if (!result) return;

    // 提取核心数据（和FtpLoginListResult的处理逻辑一致）
    int progress = result->progress();
    FtpProgressResult::ProgressType type = result->type();
    QString opStr = (type == FtpProgressResult::ProgressType::Upload) ? "上传" : "下载";

    // 进度条初始化（确保和UI状态一致）
    if (!ui.progressBar_Transfer->isVisible()) {
        ui.progressBar_Transfer->setVisible(true);
        ui.progressBar_Transfer->setRange(0, 100);
        ui.progressBar_Transfer->setValue(0);
    }

    // 更新进度条（限制0-100，避免非法值）
    progress = qBound(0, progress, 100);
    ui.progressBar_Transfer->setValue(progress);

    // 显示状态文本（复用showStatus，和文件列表的状态显示逻辑一致）
    showStatus(QString("FTP%1进度：%2%").arg(opStr).arg(progress));

}

// 格式化文件大小显示
QString DataDownloadDialog::formatFileSize(qlonglong size)
{
    if (size < 0) return "-";

    QStringList units = { "B", "KB", "MB", "GB", "TB" };
    int unitIndex = 0;
    double fileSize = size;

    while (fileSize >= 1024 && unitIndex < units.size() - 1) {
        fileSize /= 1024;
        unitIndex++;
    }

    return QString("%1 %2").arg(fileSize, 0, 'f', 2).arg(units[unitIndex]);
}

void DataDownloadDialog::updateButtonStates()
{
    // 1. 基础状态：是否已连接
    bool isConnected = m_isConnected;

    // 2. 本地选中状态
    bool hasLocalSelection = !m_currentLocalPath.isEmpty() && QFileInfo(m_currentLocalPath).exists();
    bool isLocalFile = hasLocalSelection && QFileInfo(m_currentLocalPath).isFile(); // 本地选中的是文件
    bool isLocalDir = hasLocalSelection && QFileInfo(m_currentLocalPath).isDir();   // 本地选中的是目录

    // 3. 远程选中状态
    bool hasRemoteSelection = !m_currentRemotePath.isEmpty() && m_currentRemotePath != "/";
    bool isRemoteDir = hasRemoteSelection && isRemoteItemDirectory(); // 远程选中的是目录
    bool isRemoteFile = hasRemoteSelection && !isRemoteItemDirectory(); // 远程选中的是文件

    // 4. 按钮状态控制（核心修复）
    // 断开连接：仅连接时启用
    ui.btn_Disconnect->setEnabled(isConnected);
    // 连接：仅未连接时启用
    ui.btn_Connect->setEnabled(!isConnected);
    // 刷新：仅连接时启用
    ui.btn_Refresh->setEnabled(isConnected);

    // ========== 修复2：上传按钮启用条件 ==========
    // 上传：已连接 + 本地选中文件 + 远程选中目录（或根目录）
    bool canUpload = isConnected && isLocalFile && (isRemoteDir || m_currentRemotePath == "/");
    ui.btn_Upload->setEnabled(canUpload);
    qDebug() << "[更新按钮状态] 上传按钮是否可用：" << canUpload
        << "，已连接：" << isConnected
        << "，本地选中文件：" << isLocalFile
        << "，远程选中目录/根目录：" << (isRemoteDir || m_currentRemotePath == "/");

    // ========== 下载按钮启用条件 ==========
    // 下载：已连接 + 远程选中文件
    ui.btn_Download->setEnabled(isConnected && isRemoteFile);

    // ==========删除按钮启用条件 ==========
    // 删除：已连接 + 远程选中项（文件/目录，非根目录）
    ui.btn_DeleteRemote->setEnabled(isConnected && hasRemoteSelection);

    // 路径输入框状态
    ui.lineEdit_RemotePath->setReadOnly(!isConnected);
    // TreeView交互状态
    ui.treeView_Remote->setEnabled(isConnected);
}

void DataDownloadDialog::getRemoteFileList(const QString& remotePath)
{
    static QTime lastRequestTime = QTime::currentTime();
    int elapsed = lastRequestTime.elapsed();
    // 1. 规范化要刷新的目标目录（核心：不再默认用根目录）
    QString targetDir = normalizeRemotePath(remotePath);

    // 关键：增加路径有效性校验，避免空路径/无效路径请求
    if (targetDir.isEmpty() || targetDir == "/../") {
        qWarning() << "[getRemoteFileList] 无效路径，跳过请求：" << remotePath;
        showStatus("获取文件列表失败：无效路径", true);
        ui.btn_Refresh->setEnabled(true);
        return;
    }

    // 500ms内防止重复请求（仅当目标目录相同才拦截）
    if (!m_isConnected || (elapsed < 500 && targetDir == m_currentRemoteDir)) {
        return;
    }

    lastRequestTime = QTime::currentTime();

    // 核心修复：更新当前远程目录为要刷新的目标目录（而非根目录）
    m_currentRemoteDir = targetDir;

    showStatus(QString("正在获取远程文件列表：%1").arg(targetDir));
    ui.btn_Refresh->setEnabled(false);

    // 清空当前列表并显示加载状态
    m_remoteFileModel->clear();
    m_remoteFileModel->setHorizontalHeaderLabels(QStringList() << "名称" << "类型" << "大小" << "修改时间");

    QList<QStandardItem*> loadingItems;
    loadingItems.append(new QStandardItem("加载中..."));
    loadingItems.append(new QStandardItem(""));
    loadingItems.append(new QStandardItem(""));
    loadingItems.append(new QStandardItem(""));
    foreach(QStandardItem * item, loadingItems) {
        item->setEditable(false);
    }
    m_remoteFileModel->appendRow(loadingItems);

    // 发送请求（添加失败回调处理）
    if (m_dataModule) {
        qDebug() << "[getRemoteFileList] 发送请求：刷新目录=" << targetDir;
        m_dataModule->getRemoteFileList(targetDir);
    }
    else {
        qWarning() << "[getRemoteFileList] 数据模块未初始化";
        showStatus("获取文件列表失败：数据模块异常", true);
        ui.btn_Refresh->setEnabled(true);
        // 恢复缓存的列表
        if (!m_cachedRemoteFiles.isEmpty()) {
            buildRemoteFileTree(m_cachedRemoteFiles);
        }
    }
}
// 构建完整的远程文件树形结构
void DataDownloadDialog::buildRemoteFileTree(const QStringList& files)
{
    m_cachedRemoteFiles = files; // 每次成功构建都更新缓存
    // 清空现有模型
    m_remoteFileModel->clear();
    m_remoteFileModel->setHorizontalHeaderLabels(QStringList() << "名称" << "类型" << "大小" << "修改时间");
    qDebug() << "[buildRemoteFileTree] 开始处理，files总数：" << files.size();



    // 如果不是根目录，添加上级目录（核心修复：使用m_currentRemoteDir计算父目录）
    if (m_currentRemoteDir != "/") {
        QString parentPath = getParentDirectory(m_currentRemoteDir);
        QList<QStandardItem*> parentItems;
        QStandardItem* parentItem = new QStandardItem("..");
        parentItem->setEditable(false);
        // 存储上级目录路径（用于返回上级）
        parentItem->setData(parentPath, Qt::UserRole + 3);
        parentItem->setData(true, Qt::UserRole + 2); // 标记为目录

        parentItems.append(parentItem);
        parentItems.append(new QStandardItem("目录"));
        parentItems.append(new QStandardItem("-"));
        parentItems.append(new QStandardItem("-"));

        foreach(QStandardItem * item, parentItems) {
            item->setEditable(false);
        }
        m_remoteFileModel->appendRow(parentItems);
    }

    // 创建根节点（不可见）
    QStandardItem* invisibleRoot = m_remoteFileModel->invisibleRootItem();

    // 用于临时存储目录节点（key：完整目录路径，value：对应的QStandardItem）
    QMap<QString, QStandardItem*> dirNodeMap;

    // 第一步：先处理所有目录，创建目录节点（确保目录在文件之前显示）
    foreach(const QString & fileStr, files) {
        QStringList parts = fileStr.split("|");
        qDebug() << "[buildRemoteFileTree] 遍历元素：" << fileStr
            << "，拆分后parts：" << parts
            << "，type字段：" << (parts.size() >= 3 ? parts[2].trimmed() : "空");

        if (parts.size() < 5) continue;

        QString itemPath = parts[0].trimmed();
        QString type = parts[2].trimmed();
        QString permissions = parts[3].trimmed();

        // 修正类型：权限字段为d则是目录
        if (permissions.startsWith('d') && type != "dir") {
            type = "dir";
            qDebug() << "[buildRemoteFileTree] 修正类型：" << itemPath << "→ dir（权限字段为目录）";
        }

        if (type != "dir") continue;
        qDebug() << "[buildRemoteFileTree] 处理目录：" << itemPath;

        // 计算目录的绝对路径（核心修复：基于当前目录拼接）
        QString dirAbsolutePath = normalizeRemotePath(m_currentRemoteDir + "/" + itemPath);
        // 提取相对于当前目录的名称（仅用于显示）
        QString dirName = itemPath;
        if (dirAbsolutePath.startsWith(m_currentRemoteDir + "/")) {
            dirName = dirAbsolutePath.mid(m_currentRemoteDir.length() + 1);
        }

        // 跳过当前目录本身
        if (dirAbsolutePath == m_currentRemoteDir) continue;

        // 创建目录节点
        QList<QStandardItem*> dirItems;
        QStandardItem* dirItem = new QStandardItem(dirName);
        dirItem->setEditable(false);
        dirItem->setData(dirAbsolutePath, Qt::UserRole + 3); // 存储完整目录路径
        dirItem->setData(true, Qt::UserRole + 2); // 标记为目录

        dirItems.append(dirItem);
        dirItems.append(new QStandardItem("目录"));
        dirItems.append(new QStandardItem("-"));
        dirItems.append(new QStandardItem(parts[4].trimmed()));

        foreach(QStandardItem * item, dirItems) {
            item->setEditable(false);
        }

        invisibleRoot->appendRow(dirItems);
        dirNodeMap[dirAbsolutePath] = dirItem;
    }

    // 第二步：处理所有文件，添加到当前目录根节点（核心修复：文件仅作为当前目录直接子项）
    foreach(const QString & fileStr, files) {
        QStringList parts = fileStr.split("|");
        if (parts.size() < 5) continue;

        QString itemPath = parts[0].trimmed();
        qlonglong size = parts[1].toLongLong();
        QString type = parts[2].trimmed();
        QString permissions = parts[3].trimmed();
        QString timeInfo = parts[4].trimmed();

        // 修正类型：权限字段非d则是文件
        if (!permissions.startsWith('d') && type != "file") {
            type = "file";
            qDebug() << "[buildRemoteFileTree] 修正类型：" << itemPath << "→ file（权限字段为文件）";
        }

        if (type != "file") continue;

        // 核心修复：计算文件的绝对路径（仅拼接当前目录+文件名）
        QString fileAbsolutePath = normalizeRemotePath(m_currentRemoteDir + "/" + itemPath);
        QString fileName = itemPath;
        // 提取纯文件名（去掉路径前缀）
        int lastSlash = itemPath.lastIndexOf('/');
        if (lastSlash >= 0) {
            fileName = itemPath.mid(lastSlash + 1);
        }

        qDebug() << "[buildRemoteFileTree] 处理文件：" << fileAbsolutePath;

        // 检查文件是否已存在（避免重复）
        bool exists = false;
        for (int i = 0; i < invisibleRoot->rowCount(); ++i) {
            QStandardItem* child = invisibleRoot->child(i, 0);
            if (child && child->text() == fileName && !child->data(Qt::UserRole + 2).toBool()) {
                exists = true;
                break;
            }
        }
        if (exists) continue;

        // 创建文件节点（仅添加到当前目录根节点，不添加到任何子目录）
        QList<QStandardItem*> fileItems;
        QStandardItem* fileItem = new QStandardItem(fileName);
        fileItem->setEditable(false);
        fileItem->setData(fileAbsolutePath, Qt::UserRole + 3); // 存储绝对路径
        fileItem->setData(false, Qt::UserRole + 2); // 标记为文件

        fileItems.append(fileItem);
        fileItems.append(new QStandardItem("文件"));
        fileItems.append(new QStandardItem(formatFileSize(size)));
        fileItems.append(new QStandardItem(timeInfo));

        foreach(QStandardItem * item, fileItems) {
            item->setEditable(false);
        }

        invisibleRoot->appendRow(fileItems);
    }

    // 展开当前目录的直接子节点（方便用户操作）
    ui.treeView_Remote->expandToDepth(0);
    ui.lineEdit_RemotePath->setText(m_currentRemoteDir);
}
// 在树形模型中查找或创建父节点
QStandardItem* DataDownloadDialog::findOrCreateParentNode(const QString& parentPath, QStandardItem* rootNode)
{
    // 根节点
    if (parentPath.isEmpty() || parentPath == "/" || parentPath == ".") {
        return rootNode;
    }

    // 规范化路径：处理相对路径和特殊情况
    QString path = parentPath;
    if (path.startsWith("./")) path = path.mid(2);
    if (!path.startsWith("/")) path = "/" + path;
    if (path.endsWith("/")) path = path.left(path.length() - 1);

    // 处理空路径
    if (path.isEmpty()) return rootNode;

    // 递归查找或创建父节点路径
    QString currentPath = "";
    QStringList pathParts = path.split("/", Qt::SkipEmptyParts);
    QStandardItem* currentNode = rootNode;

    foreach(const QString & part, pathParts) {
        if (part.isEmpty() || part == ".") continue;

        currentPath += "/" + part;

        // 在当前节点的子节点中查找
        bool found = false;
        for (int i = 0; i < currentNode->rowCount(); ++i) {
            QStandardItem* child = currentNode->child(i, 0);
            if (child && child->data(Qt::UserRole + 3).toString() == currentPath) {
                currentNode = child;
                found = true;
                break;
            }
        }

        // 如果没找到，创建新节点
        if (!found) {
            QList<QStandardItem*> newDirItems;
            QStandardItem* dirItem = new QStandardItem(part);
            dirItem->setEditable(false);
            dirItem->setData(currentPath, Qt::UserRole + 3); // 存储完整路径
            dirItem->setData(true, Qt::UserRole + 2); // 标记为目录

            newDirItems.append(dirItem);
            newDirItems.append(new QStandardItem("目录"));
            newDirItems.append(new QStandardItem("-"));
            newDirItems.append(new QStandardItem(""));

            foreach(QStandardItem * item, newDirItems) {
                item->setEditable(false);
            }

            currentNode->appendRow(newDirItems);
            currentNode = dirItem;
        }
    }

    return currentNode;
}

void DataDownloadDialog::populateRemoteFileList(const QStringList& files)
{
    m_remoteFileModel->clear();
    m_remoteFileModel->setHorizontalHeaderLabels(QStringList() << "名称" << "类型" << "大小" << "修改时间");

    // 添加上级目录
    if (m_currentRemotePath != "/") {
        QList<QStandardItem*> items;
        items.append(new QStandardItem(".."));
        items.append(new QStandardItem("目录"));
        items.append(new QStandardItem("-"));
        items.append(new QStandardItem("-"));
        m_remoteFileModel->appendRow(items);
    }

    // 添加文件和文件夹
    for (const QString& fileInfo : files) {
        QList<QStandardItem*> items;

        // 解析文件信息：格式为 "名称|大小|类型|权限|时间"
        QStringList parts = fileInfo.split("|");
        if (parts.size() < 3) {
            // 兼容旧格式（只有文件名）
            items.append(new QStandardItem(fileInfo));
            if (fileInfo.endsWith("/") || !fileInfo.contains(".")) {
                items.append(new QStandardItem("目录"));
                items.append(new QStandardItem("-"));
            }
            else {
                items.append(new QStandardItem("文件"));
                items.append(new QStandardItem("未知"));
            }
            items.append(new QStandardItem("-"));
        }
        else {
            // 解析新格式的详细信息
            QString name = parts[0];
            qlonglong size = parts[1].toLongLong();
            QString type = parts[2] == "dir" ? "目录" : "文件";
            QString timeInfo = parts.size() >= 5 ? parts[4] : "-";

            items.append(new QStandardItem(name));
            items.append(new QStandardItem(type));

            // 格式化文件大小
            if (type == "目录") {
                items.append(new QStandardItem("-"));
            }
            else {
                items.append(new QStandardItem(formatFileSize(size)));
            }

            items.append(new QStandardItem(timeInfo));

            // 设置数据角色，存储原始信息
            items[0]->setData(fileInfo, Qt::UserRole);
            items[0]->setData(name, Qt::UserRole + 1); // 存储纯文件名
            items[0]->setData(type == "目录", Qt::UserRole + 2); // 是否是目录
        }

        // 设置项为可选但不可编辑
        foreach(QStandardItem * item, items) {
            item->setEditable(false);
        }

        m_remoteFileModel->appendRow(items);
    }

    // 自动调整列宽
    for (int i = 0; i < m_remoteFileModel->columnCount() - 1; ++i) {
        ui.treeView_Remote->header()->setSectionResizeMode(m_remoteFileModel->columnCount() - 1, QHeaderView::Stretch);
    }
}

void DataDownloadDialog::showStatus(const QString& text, bool isError)
{
    ui.label_Status->setText(text);

    if (isError) {
        // 失败状态：红色（#e74c3c）
        ui.label_Status->setStyleSheet("color: #e74c3c;");
    }
    else {
        // 成功/普通状态：绿色（#27ae60，标准成功绿）
        ui.label_Status->setStyleSheet("color: #27ae60;");
    }
}

void DataDownloadDialog::handleChannelStatusResult(const CommunicationChannelStatusResult* result)
{
    CommunicationChannel channel = result->channel();
    CommunicationChannelStatus status = result->channelStatus();
    QString error = result->errorMsg();
    QString tooltip;
    switch (status)
    {
    case CommunicationChannelStatus::Stopped:
        m_isConnected = false;  // 重要：更新连接状态标志
        showStatus("状态：连接断开", true);
        updateButtonStates();   // 更新按钮状态
        clearRemoteFileList();  // 清空远程文件列表
        break;
    case CommunicationChannelStatus::Running:
        m_isConnected = true;
        showStatus("已连接到服务器");
        updateButtonStates();
        break;
    case CommunicationChannelStatus::Error:
        m_isConnected = false;
        showStatus(QString("连接错误：%1").arg(error), true);
        updateButtonStates();
        clearRemoteFileList();
        break;
    case CommunicationChannelStatus::Initializing:
        showStatus("正在连接中...");
        break;
    default:
        break;
    }
}

void DataDownloadDialog::handleFtpLoginListResult(const FtpLoginListResult* result)
{
    if (!result) return;

    // 登录成功处理（仅登录场景）
    if (result->loginSuccess())
    {
        m_isConnected = true;
        showStatus("FTP登录成功");
        updateButtonStates();

        if (!result->fileList().isEmpty())
        {
            buildRemoteFileTree(result->fileList());
            showStatus("已连接并加载文件列表");
        }
        else
        {
            getRemoteFileList("/");
        }
    }
    // 文件列表成功处理（仅列表请求场景）
    else if (result->listSuccess())
    {
        // 列表请求成功，必须有文件数据才构建（避免空列表）
        if (!result->fileList().isEmpty()) {
            buildRemoteFileTree(result->fileList());
            showStatus(QString("成功获取文件列表，共%1个项目").arg(result->fileList().size()));
        }
        else {
            // 服务器返回空列表（正常情况，如空目录）
            buildRemoteFileTree(result->fileList()); // 空列表也构建（显示上级目录）
            showStatus("当前目录无文件");
        }
        ui.btn_Refresh->setEnabled(true);
    }
    // 登录/列表失败处理
    else
    {
        m_isConnected = false;
        showStatus(QString("操作失败：%1").arg(result->errorMsg()), true);
        updateButtonStates();
       
    }
}

void DataDownloadDialog::handleFtpTransferResult(const FtpTransferResult* result)
{
    if (!result) return;
    ResultStatus status = result->status();
    FtpTransferResult::TransferType type = result->transferType();

    // 存储操作的目标目录（用于刷新）
    QString operateDir = "";
    if (type == FtpTransferResult::TransferType::Upload) {
        operateDir = m_lastUploadRemoteDir.isEmpty() ? m_currentRemoteDir : m_lastUploadRemoteDir;
    }
    else {
        // 下载的目标目录：本地目录，不影响远程
        operateDir = m_currentRemoteDir;
    }

    QString fileName; // 存储提取后的纯文件名
    if (type == FtpTransferResult::TransferType::Upload) {
        // 上传：从本地路径提取文件名
        fileName = QFileInfo(m_currentLocalPath).fileName();
    }
    else {
        // 下载：从远程路径提取文件名
        fileName = QFileInfo(m_currentRemotePath).fileName();
    }
    // 空值保护：若文件名为空，显示默认占位符
    if (fileName.isEmpty()) {
        fileName = "未知文件";
    }
    // =====================================================

    switch (status)
    {
    case ResultStatus::Success:
        if (type == FtpTransferResult::TransferType::Upload) {
            // 只刷新上传的目标目录
            refreshContent(RefreshType::RemoteOnly, operateDir);
            showStatus(QString("文件 %1 上传成功").arg(fileName));
        }
        else {
            // 下载成功：只传一个参数（依赖默认值）
            refreshContent(RefreshType::LocalOnly);
            showStatus(QString("文件 %1 下载成功").arg(fileName));
        }
        // 隐藏进度条
        ui.progressBar_Transfer->setVisible(false);
        break;
    case ResultStatus::Failed:
        showStatus(QString("文件 %1 传输失败：%2").arg(fileName).arg(result->errorMsg()), true);
        ui.progressBar_Transfer->setVisible(false);
        break;
    case ResultStatus::Pending:
        showStatus(type == FtpTransferResult::TransferType::Upload
            ? QString("正在上传文件 %1...").arg(fileName)
            : QString("正在下载文件 %1...").arg(fileName));
        break;
    case ResultStatus::Timeout:
        showStatus(type == FtpTransferResult::TransferType::Upload
            ? QString("文件 %1 上传超时").arg(fileName)
            : QString("文件 %1 下载超时").arg(fileName), true);
        ui.progressBar_Transfer->setVisible(false);
        break;
    case ResultStatus::Unknown:
        showStatus(QString("文件 %1 传输状态未知").arg(fileName), true);
        ui.progressBar_Transfer->setVisible(false);
        break;
    default:
        showStatus(QString("文件 %1 传输状态未知").arg(fileName), true);
        ui.progressBar_Transfer->setVisible(false);
        break;
    }
}
void DataDownloadDialog::refreshContent(RefreshType type, const QString& targetRemoteDir)
{
    // 1. 刷新远程：优先使用传入的目标目录，无则用当前目录
    QString remoteDirToRefresh = targetRemoteDir.isEmpty() ? m_currentRemoteDir : targetRemoteDir;
    if (type == RefreshType::RemoteOnly || type == RefreshType::Both) {
        qDebug() << "[refreshContent] 刷新远程目录：" << remoteDirToRefresh;
        if (m_isConnected && !remoteDirToRefresh.isEmpty()) {
            getRemoteFileList(remoteDirToRefresh);
        }
    }

    // 2. 刷新本地：逻辑不变
    if (type == RefreshType::LocalOnly || type == RefreshType::Both) {
        qDebug() << "[refreshContent] 刷新本地目录，根路径：" << m_localFileModel->rootPath();
        if (m_localFileModel) {
            QString originalRootPath = m_localFileModel->rootPath();
            m_localFileModel->setRootPath("");
            m_localFileModel->setRootPath(originalRootPath);
            ui.treeView_Local->setRootIndex(m_localFileModel->index(originalRootPath));
            ui.treeView_Local->viewport()->update();
        }
    }

}
QString DataDownloadDialog::normalizeRemotePath(const QString& path)
{
    if (path.isEmpty()) return "/";

    QString normalized = path;
    // 替换多个斜杠为单个斜杠
    normalized.replace(QRegularExpression("//+"), "/");
    // 移除末尾斜杠（根目录除外）
    if (normalized != "/" && normalized.endsWith("/")) {
        normalized.chop(1);
    }
    // 确保路径以斜杠开头
    if (!normalized.startsWith("/")) {
        normalized = "/" + normalized;
    }
    // 处理./和../（核心修复：健壮的相对路径处理）
    normalized.replace(QRegularExpression("/\\./"), "/");
    while (normalized.contains("/../") && normalized != "/../") {
        int pos = normalized.indexOf("/../");
        if (pos == 0) {
            normalized = normalized.replace("/../", "/");
        }
        else {
            int prevSlash = normalized.lastIndexOf('/', pos - 1);
            if (prevSlash >= 0) {
                normalized = normalized.remove(prevSlash, pos - prevSlash + 3);
            }
            else {
                normalized = normalized.remove(0, pos + 3);
            }
        }
    }
    // 最终兜底：确保根目录正确
    if (normalized.isEmpty()) {
        normalized = "/";
    }
    return normalized;

}
bool DataDownloadDialog::isRemoteItemDirectory()
{
    // 获取选中行的索引（无论点击哪一列，都取第一列的Item）
    QModelIndex currentIndex = ui.treeView_Remote->currentIndex();
    if (!currentIndex.isValid()) {
        return false;
    }
    // 切换到第一列的索引（UserRole数据仅存在于第一列）
    QModelIndex firstColIndex = currentIndex.siblingAtColumn(0);
    QStandardItem* item = m_remoteFileModel->itemFromIndex(firstColIndex);

    if (!item) {
        return false;
    }
    // 返回是否为目录的标记
    return item->data(Qt::UserRole + 2).toBool();
}
QString DataDownloadDialog::getParentDirectory(const QString& path)
{
    if (path.isEmpty() || path == "/") {
        return "/";
    }
    QString cleanPath = normalizeRemotePath(path);

    // 1. 处理文件路径：直接返回文件所在目录（不再提取父目录）
    if (!isPathDirectory(cleanPath)) {
        int lastSlash = cleanPath.lastIndexOf('/');
        if (lastSlash <= 0) {
            return "/";
        }
        return cleanPath.left(lastSlash); // 返回文件所在目录（如 /icon/INS_DATA.log → /icon）
    }

    // 2. 处理目录路径：返回该目录的父目录（如 /icon → /）
    int lastSlash = cleanPath.lastIndexOf('/');
    if (lastSlash <= 0) {
        return "/";
    }
    return cleanPath.left(lastSlash);
}
bool DataDownloadDialog::isPathDirectory(const QString& path)
{
     // 简单规则：目录路径以/结尾（根目录除外），或路径中无文件名后缀
    if (path == "/") return true;
    if (path.endsWith("/")) return true;
    // 有后缀（如.png/.docx）则是文件
    int dotIndex = path.lastIndexOf('.');
    int slashIndex = path.lastIndexOf('/');
    return dotIndex < slashIndex; // 后缀在最后一个/之前，说明是目录
}
void DataDownloadDialog::on_connectBtn_clicked()
{
    // 获取连接参数
    QString host = ui.le_host->text().trimmed();
    QString port = ui.le_port->text().trimmed();
    QString user = ui.lineEdit_User->text().trimmed();
    QString pwd = ui.lineEdit_Pwd->text();
    m_dataModule->logIn(host, port, user, pwd);
}

void DataDownloadDialog::setupTreeViewAutoResize(QTreeView* treeView)
{
    // 设置列宽自适应策略
    QHeaderView* header = treeView->header();

    // 最后一列拉伸以填充剩余空间
    header->setStretchLastSection(true);

    // 启用交互式调整大小
    header->setSectionsMovable(true);
    header->setSectionsClickable(true);

    // 设置默认列宽策略
    header->setSectionResizeMode(QHeaderView::Interactive);
    // 最后一列设置为拉伸模式
    header->setSectionResizeMode(header->count() - 1, QHeaderView::Stretch);

    // 设置行高自适应
    treeView->setUniformRowHeights(false); // 允许不同行有不同高度
    treeView->setIconSize(QSize(16, 16));  // 设置图标大小

    // 设置滚动模式为即时滚动
    treeView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    treeView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    // 启用鼠标跟踪，以便更好地处理悬停效果
    treeView->setMouseTracking(true);
}

void DataDownloadDialog::on_disconnectBtn_clicked()
{
    if (m_isConnected) {
        showStatus("正在断开连接...");
        m_dataModule->logOut();
    }
    else {
        showStatus("当前未连接", true);
    }
}

void DataDownloadDialog::clearRemoteFileList()
{
    // 清空远程文件列表
    m_remoteFileModel->clear();
    m_remoteFileModel->setHorizontalHeaderLabels(QStringList() << "名称" << "类型" << "大小" << "修改时间");

    // 重置远程路径和目录
    m_currentRemotePath = "/";
    m_currentRemoteDir = "/"; // 核心修复：重置当前目录
    ui.lineEdit_RemotePath->setText(m_currentRemotePath);

    // 隐藏进度条（如果显示）
    ui.progressBar_Transfer->setVisible(false);
}

void DataDownloadDialog::parsePath(const QString& fullPath, QString& parentPath, QString& fileName)
{
    if (fullPath == "/" || fullPath.isEmpty()) {
        parentPath = "";
        fileName = "/";
        return;
    }

    QString path = fullPath;
    if (path.startsWith("/")) {
        path = path.mid(1);
    }
    if (path.endsWith("/")) {
        path = path.left(path.length() - 1);
    }

    int lastSlash = path.lastIndexOf('/');
    if (lastSlash >= 0) {
        parentPath = "/" + path.left(lastSlash);
        fileName = path.mid(lastSlash + 1);
    }
    else {
        parentPath = "/";
        fileName = path;
    }

    // 确保父路径格式正确
    if (parentPath == "//") parentPath = "/";
}

void DataDownloadDialog::on_uploadBtn_clciked()
{
    if (!m_isConnected) {
        showStatus("未连接到服务器", true);
        QMessageBox::warning(this, "上传失败", "请先连接到FTP服务器！");
        return;
    }

    if (m_currentLocalPath.isEmpty()) {
        showStatus("请选择要上传的文件", true);
        QMessageBox::warning(this, "上传失败", "未选中有效的本地文件！");
        return;
    }

    QFileInfo fileInfo(m_currentLocalPath);
    if (!fileInfo.exists()) {
        showStatus("所选文件不存在", true);
        QMessageBox::warning(this, "上传失败", "选中的本地文件不存在！");
        return;
    }

    if (!fileInfo.isFile()) {
        showStatus("只能上传文件，不能上传目录", true);
        QMessageBox::warning(this, "上传失败", "请选择文件而非目录进行上传！");
        return;
    }

    // 获取本地文件名
    QString fileName = fileInfo.fileName();
    // 远程目标路径：选中的远程目录 + 文件名
    QString remoteTargetPath = normalizeRemotePath(m_currentRemotePath + "/" + fileName);
    // 核心：提取上传的目标目录并记录
    m_lastUploadRemoteDir = getParentDirectory(remoteTargetPath);

    // 日志：验证上传路径
    qDebug() << "[上传文件] 本地路径：" << m_currentLocalPath
        << "，远程目标路径：" << remoteTargetPath
        << "，选中的远程目录：" << m_currentRemotePath
        << "，上传目标目录：" << m_lastUploadRemoteDir; // 新增日志

    // 显示进度条
    ui.progressBar_Transfer->setVisible(true);
    ui.progressBar_Transfer->setValue(0);
    ui.progressBar_Transfer->setRange(0, 100); // 设置确定范围
    showStatus(QString("正在上传文件：%1").arg(fileName));

    // 调用本模块的DataModule上传文件
    m_dataModule->uploadFile(m_currentLocalPath, remoteTargetPath);
}

void DataDownloadDialog::on_downloadBtn_clicked()
{
    // 1. 检查连接状态
    if (!m_isConnected) {
        showStatus("未连接到服务器，无法下载", true);
        QMessageBox::warning(this, "下载失败", "请先连接到FTP服务器！");
        return;
    }

    // ========== 修复4：精准校验远程路径 ==========
    // 2. 检查远程选中项（必须是文件，且路径有效）
    if (m_currentRemotePath.isEmpty() || m_currentRemotePath == "/") {
        showStatus("请选择要下载的远程文件", true);
        QMessageBox::warning(this, "下载失败", "未选中有效的远程文件！\n原因：远程路径为空或为根目录");
        qCritical() << "[下载失败] 远程路径为空或为根目录：" << m_currentRemotePath;
        return;
    }
    // 验证远程选中项是否为文件（禁止下载目录）
    if (isRemoteItemDirectory()) {
        showStatus("仅支持下载文件，无法下载目录", true);
        QMessageBox::warning(this, "下载失败", "请选择文件而非目录进行下载！\n当前选中：" + m_currentRemotePath);
        qCritical() << "[下载失败] 选中的是目录，不是文件：" << m_currentRemotePath;
        return;
    }

    // 3. 处理本地保存路径（保留之前的本地路径修复逻辑）
    QString localSavePath = m_currentLocalPath;
    // 3.1 若本地路径为空，兜底为桌面
    if (localSavePath.isEmpty()) {
        localSavePath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        m_currentLocalPath = localSavePath; // 更新全局变量
        ui.lineEdit_LocalPath->setText(localSavePath);
    }

    // 3.2 提取远程文件名（用于拼接本地路径）
    QString remoteFileName = QFileInfo(m_currentRemotePath).fileName();
    if (remoteFileName.isEmpty()) {
        showStatus("无法解析远程文件名", true);
        QMessageBox::warning(this, "下载失败", "远程文件路径解析失败！\n远程路径：" + m_currentRemotePath);
        qCritical() << "[下载失败] 无法解析远程文件名，路径：" << m_currentRemotePath;
        return;
    }

    // 3.3 确保本地保存路径是文件路径（而非目录）
    QFileInfo localFileInfo(localSavePath);
    if (localFileInfo.isDir()) {
        // 若本地选中的是目录，拼接文件名
        localSavePath = QDir(localSavePath).filePath(remoteFileName);
    }
    else if (localFileInfo.isFile()) {
        // 若本地选中的是文件，询问是否覆盖
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "文件已存在",
            QString("本地路径 %1 已存在同名文件，是否覆盖？").arg(localSavePath),
            QMessageBox::Yes | QMessageBox::No
        );
        if (reply != QMessageBox::Yes) {
            showStatus("用户取消下载", false);
            return;
        }
    }

    // 3.4 最终路径校验
    QDir localDir = QFileInfo(localSavePath).dir();
    if (!localDir.exists()) {
        // 若目录不存在，创建目录
        if (!localDir.mkpath(".")) {
            showStatus("无法创建本地保存目录", true);
            QMessageBox::critical(this, "下载失败", QString("无法创建目录：%1").arg(localDir.path()));
            return;
        }
    }

    // 4. 日志打印（方便调试）
    qDebug() << "[下载文件] 远程路径：" << m_currentRemotePath
        << "，本地保存路径：" << localSavePath
        << "，远程文件名：" << remoteFileName;

    // 5. 初始化进度条
    ui.progressBar_Transfer->setVisible(true);
    ui.progressBar_Transfer->setValue(0);
    ui.progressBar_Transfer->setRange(0, 100);
    showStatus(QString("正在下载文件：%1").arg(remoteFileName));

    // 6. 调用下载接口（确保传递的远程路径有效）
    m_dataModule->downloadFile(localSavePath, m_currentRemotePath);
}

void DataDownloadDialog::on_deleteRemote_clicked()
{
    if (!m_isConnected) {
        showStatus("未连接到服务器，无法删除", true);
        return;
    }

    if (m_currentRemotePath.isEmpty() || m_currentRemotePath == "/") {
        showStatus("禁止删除根目录", true);
        return;
    }

    // 1. 判断删除类型（文件/目录）
    bool isDir = isRemoteItemDirectory();
    FtpDeleteResult::DeleteType delType = isDir ? FtpDeleteResult::DeleteType::Directory : FtpDeleteResult::DeleteType::File;
    QString typeStr = isDir ? "目录" : "文件";

    // 2. 二次确认
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认删除",
        QString("确定删除远程%1：%2？").arg(typeStr).arg(m_currentRemotePath),
        QMessageBox::Yes | QMessageBox::No
    );
    if (reply != QMessageBox::Yes) return;

    // 3. 记录删除项所在的目录（用于后续刷新）
    QString deleteTargetDir = getParentDirectory(m_currentRemotePath);
    qDebug() << "[删除操作] 待删除项路径：" << m_currentRemotePath << "，所在目录：" << deleteTargetDir;

    // 4. 显示删除状态
    showStatus(QString("正在删除%1：%2").arg(typeStr).arg(m_currentRemotePath));

    // 5. 调用删除接口
    m_dataModule->deleteRemoteItem(m_currentRemotePath, delType);

    // 6. 核心修复：删除后只刷新删除项所在的目录（而非根目录）
    QTimer::singleShot(500, this, [this, deleteTargetDir]() {
        getRemoteFileList(deleteTargetDir);
        // 刷新后更新输入框为当前操作的目录
        ui.lineEdit_RemotePath->setText(deleteTargetDir);
        m_currentRemotePath = deleteTargetDir;
        });
}

void DataDownloadDialog::on_refresh_clciked()
{
    getRemoteFileList(m_currentRemoteDir);
}

void DataDownloadDialog::on_treeView_Local_clicked(const QModelIndex& index)
{
    if (!index.isValid()) return;

    // 只获取本地选中的路径，不修改本地模型的根路径（根路径是默认目录，用户可通过导航栏修改）
    m_currentLocalPath = m_localFileModel->filePath(index);
    ui.lineEdit_LocalPath->setText(m_currentLocalPath);

    // 添加文件类型判断，更新按钮状态更准确
    QFileInfo fileInfo(m_currentLocalPath);
    bool isFile = fileInfo.isFile();
    bool isDir = fileInfo.isDir();

    // 更新上传按钮状态：只能上传文件
    ui.btn_Upload->setEnabled(m_isConnected && isFile);
    updateButtonStates();
}

void DataDownloadDialog::on_treeView_Remote_clicked(const QModelIndex& index)
{
    if (!index.isValid()) {
        qWarning() << "[Remote Click] 无效的索引，清空远程路径";
        m_currentRemotePath = "";
        ui.lineEdit_RemotePath->setText(""); // 空索引时清空输入框
        updateButtonStates();
        return;
    }

    // 获取选中项的完整路径和类型（切换到第一列）
    QModelIndex firstColIndex = index.siblingAtColumn(0);
    QStandardItem* item = m_remoteFileModel->itemFromIndex(firstColIndex);
    if (!item) {
        qWarning() << "[Remote Click] 无法获取选中项的Item，清空远程路径";
        m_currentRemotePath = "";
        ui.lineEdit_RemotePath->setText(""); // 空Item时清空输入框
        updateButtonStates();
        return;
    }

    // 校验数据是否存在
    QVariant pathVariant = item->data(Qt::UserRole + 3);
    if (!pathVariant.isValid() || pathVariant.toString().isEmpty()) {
        qWarning() << "[Remote Click] Item的路径数据为空，文本=" << item->text();
        // 兜底：基于当前目录+文本拼接路径
        QString fallbackPath = normalizeRemotePath(m_currentRemoteDir + "/" + item->text());
        m_currentRemotePath = fallbackPath;
        qWarning() << "[Remote Click] 使用兜底路径：" << m_currentRemotePath;
    }
    else {
        // 正常获取路径并规范化
        QString fullPath = pathVariant.toString().trimmed();
        // 如果路径不是绝对路径，拼接当前目录
        if (!fullPath.startsWith("/")) {
            fullPath = normalizeRemotePath(m_currentRemoteDir + "/" + fullPath);
        }
        // 二次规范化，确保路径格式正确
        m_currentRemotePath = normalizeRemotePath(fullPath);
    }

    bool isDirectory = item->data(Qt::UserRole + 2).toBool();
    QString itemText = item->text();

    // 日志：验证最终路径是否正确
    qDebug() << "[Remote Click] 选中项：" << itemText
        << "，当前目录：" << m_currentRemoteDir
        << "，最终绝对路径：" << m_currentRemotePath
        << "，是否目录：" << isDirectory;

    // ========== 修复1：输入框显示选中项的完整路径 ==========
    ui.lineEdit_RemotePath->setText(m_currentRemotePath);

    updateButtonStates(); // 核心：选中后立即更新按钮状态
}
void DataDownloadDialog::on_treeView_Remote_doubleClicked(const QModelIndex& index)
{
    // 切换到第一列获取正确的Item
    QModelIndex firstColIndex = index.siblingAtColumn(0);
    QStandardItem* item = m_remoteFileModel->itemFromIndex(firstColIndex);
    if (!item) return;

    bool isDirectory = item->data(Qt::UserRole + 2).toBool();
    QString fullPath = item->data(Qt::UserRole + 3).toString();
    QString itemText = item->text();

    // 核心：仅目录支持双击跳转（文件双击无响应）
    if (!isDirectory) {
        qDebug() << "[Remote DoubleClick] 选中的是文件，不跳转：" << itemText;
        return;
    }

    // 日志：双击目录跳转
    qDebug() << "[Remote DoubleClick] 跳转目录：" << itemText << "，路径：" << fullPath;

    // 处理上级目录".."：双击返回上级
    if (itemText == "..") {
        QString parentPath = normalizeRemotePath(fullPath);
        getRemoteFileList(parentPath); // 触发上级目录列表刷新
        // 跳转后更新输入框为新的当前目录
        m_currentRemotePath = parentPath;
        ui.lineEdit_RemotePath->setText(parentPath);
        return;
    }

    // 处理普通目录：双击进入子目录
    QString normalizedPath = normalizeRemotePath(fullPath);
    getRemoteFileList(normalizedPath); // 触发子目录列表刷新
    // 跳转后更新输入框为新的当前目录
    m_currentRemotePath = normalizedPath;
    ui.lineEdit_RemotePath->setText(normalizedPath);
}
void DataDownloadDialog::onRemoteFileListFetched(bool success, const QStringList& files, const QString& msg)
{
    if (success) {
        buildRemoteFileTree(files);
        showStatus(QString("远程文件列表加载完成，共 %1 个项目").arg(files.size()));
    }
    else {
        showStatus(QString("获取远程文件列表失败：%1").arg(msg), true);
    }

    updateButtonStates();
}

void DataDownloadDialog::onTransferProgress(int progress)
{
    ui.progressBar_Transfer->setValue(progress);
}