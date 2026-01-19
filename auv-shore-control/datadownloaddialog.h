#pragma once

#include <QDialog>
#include "ui_datadownloaddialog.h"
#include"observer.h"
#include"datamodule.h"
#include"moduledialogbase.h"
#include<QFileSystemModel>
#include<QStandardItemModel>
#include<QStyledItemDelegate>
#include<QTableWidgetItem>

// 用于存储文件信息的结构体
struct RemoteFileInfo {
	QString name;
	QString fullPath;
	qlonglong size;
	QString type; // "file" 或 "dir"
	QString permissions;
	QString timeInfo;

	RemoteFileInfo() : size(0) {}
	RemoteFileInfo(const QString& infoStr);
};
// 1. 添加枚举区分刷新类型
enum class RefreshType {
	RemoteOnly,    // 只刷新远程目录
	LocalOnly,     // 只刷新本地目录
	Both           // 都刷新
};
class DataDownloadDialog : public ModuleDialogBase,  IResultObserver
{
	Q_OBJECT

public:
	DataDownloadDialog(DataModule *module,QWidget *parent = nullptr);
	~DataDownloadDialog();

	//实现观察者接口
	void onResultUpdated(const BaseResult* result) override;
	
private:
	void initialize()override;
	
	void initUI();

	// 初始化模型
	void initModels();

	// 更新按钮状态
	void updateButtonStates();
	// 获取远程文件列表
	void getRemoteFileList(const QString& remotePath = "/");

	// 构建完整的远程文件树形结构
	void buildRemoteFileTree(const QStringList& files);

	// 在树形模型中查找或创建父节点
	QStandardItem* findOrCreateParentNode(const QString& parentPath, QStandardItem* rootNode);

	// 填充远程文件列表
	void populateRemoteFileList(const QStringList& files);

	// 显示状态信息
	void showStatus(const QString& text, bool isError = false);

	void handleChannelStatusResult(const CommunicationChannelStatusResult* result);

	//处理FTPList结果
	void handleFtpLoginListResult(const FtpLoginListResult* result);

	//处理FTP下载结果
	void handleFtpTransferResult(const FtpTransferResult* result);

	//处理文件传输进度
	void handleFtpProgressResult(const FtpProgressResult* result);

	QString formatFileSize(qlonglong size);

	void setupTreeViewAutoResize(QTreeView* treeView);

	void clearRemoteFileList();

	// 解析路径，获取父路径和文件名
	void parsePath(const QString& fullPath, QString& parentPath, QString& fileName);
	void refreshContent(RefreshType type, const QString& targetRemoteDir = "");
	// 添加路径规范化辅助函数（关键！处理斜杠、空路径等问题）
	QString normalizeRemotePath(const QString& path);

	// 获取远程选中项的真实状态（是否为目录）
	bool isRemoteItemDirectory();

	QString getParentDirectory(const QString& path);
	bool isPathDirectory(const QString& path);
private slots:
	// 服务器连接相关
	void on_connectBtn_clicked();
	void on_disconnectBtn_clicked();

	// 文件操作相关
	void on_uploadBtn_clciked();
	void on_downloadBtn_clicked();
	void on_deleteRemote_clicked();
	void on_refresh_clciked();
	void on_treeView_Local_clicked(const QModelIndex& index);
	void on_treeView_Remote_clicked(const QModelIndex& index);
	void on_treeView_Remote_doubleClicked(const QModelIndex& index);
	// 远程文件列表获取结果
	void onRemoteFileListFetched(bool success, const QStringList& files, const QString& msg);
	// 传输进度更新
	void onTransferProgress(int progress);
private:
	Ui::DataDownloadDialogClass ui;
	// 本地文件模型
	QFileSystemModel* m_localFileModel;

	// 远程文件模型
	QStandardItemModel* m_remoteFileModel;

	// 当前选中的路径
	QString m_currentLocalPath;
	QString m_currentRemotePath;
	QString m_currentRemoteDir; // 单独存储当前远程目录（区别于选中的文件路径）
	// 数据模块实例
	DataModule* m_dataModule;

	// 连接状态
	bool m_isConnected;
	const QString STYLE_DATADOWNLOAD = "./qss/datadownloaddlg.qss";

	// 存储所有远程文件信息
	QList<RemoteFileInfo> m_remoteFileInfos;
	QStringList m_cachedRemoteFiles; // 缓存上次成功加载的远程文件列表
	QString m_lastUploadRemoteDir; // 新增：记录最后一次上传的目标目录
};
class ChineseFileSystemModel : public QFileSystemModel
{
	Q_OBJECT
public:
	explicit ChineseFileSystemModel(QObject* parent = nullptr)
		: QFileSystemModel(parent) {}

	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override
	{
		if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
			switch (section) {
			case 0: return QString("名称");
			case 1: return QString("大小");
			case 2: return QString("类型");
			case 3: return QString("修改时间");
			default: return QFileSystemModel::headerData(section, orientation, role);
			}
		}
		return QFileSystemModel::headerData(section, orientation, role);
	}
};
