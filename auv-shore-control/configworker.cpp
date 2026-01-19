#include "configworker.h"
ConfigWorker::ConfigWorker(CommunicationChannel channel, 
	CommunicationManager* commManager, QObject* parent)
	:QObject(parent),
	m_channel(channel),
	m_commManager(commManager)
{

}
ConfigWorker::ConfigWorker(CommunicationChannel channel, const ChannelConfigPtr& config,
	CommunicationManager* commManager, QObject *parent)
	: QObject(parent),
	m_channel(channel),
	m_config(config),
	m_commManager(commManager)
{}
ConfigWorker::ConfigWorker(CommunicationChannel channel, QByteArray frame, 
	CommunicationManager* commManager, BDSDataType bdsDataType, QObject* parent )
	:QObject(parent),
	m_channel(channel),
	m_frame(frame),
	m_commManager(commManager),
	m_bdsDataType(bdsDataType) // 初始化类型
{

}
ConfigWorker::ConfigWorker(CommunicationChannel channel, const QString& localPath, 
	const QString& remotePath, ModuleType sourceModule, uint64_t requestId, CommunicationManager* commManager, QObject* parent)
	:QObject(parent),
	m_channel(channel),
	m_localPath(localPath),
	m_remotePath(remotePath),
	m_sourceModule(sourceModule),
	m_requestId(requestId),
	m_commManager(commManager)
{

}
ConfigWorker::~ConfigWorker()
{

}
void ConfigWorker::setDeleteType(FtpDeleteResult::DeleteType type)
{
	 m_deleteType = type; 
}
void ConfigWorker::doConfig()
{
	auto* channelInst = m_commManager->getChannelInstance(m_channel);
	if (!channelInst)
	{
		emit configFinished(false, m_channel);
		return;
	}

	// 1. 断开上一次未释放的连接（避免残留）
	if (m_currentConn)
	{
		QObject::disconnect(m_currentConn);
	}


	// 2. 连接信号并保存连接对象
	m_currentConn = QObject::connect(
		channelInst, &CommunicationChannelBase::setConfigFinished,
		this,
		[=](bool success)
		{
			emit configFinished(success, m_channel);
			QObject::disconnect(m_currentConn); // 任务完成，断开当前连接
			m_currentConn = QMetaObject::Connection(); // 重置
		},
		Qt::QueuedConnection
			);

	// 3. 检查配置触发是否成功
	bool triggerSuccess = m_commManager->setChannelConfig(m_channel, m_config);
	if (!triggerSuccess)
	{
		QObject::disconnect(m_currentConn); // 触发失败时及时断开连接，避免内存泄漏
		m_currentConn = QMetaObject::Connection();
		emit configFinished(false, m_channel);
	}
}
void ConfigWorker::doSendData()
{
	auto* channelInst = m_commManager->getChannelInstance(m_channel);
	if (!channelInst)
	{
		emit dataSendFinished(false, m_channel);
		return;
	}

	// 1. 断开上一次未释放的连接（避免残留）
	if (m_currentConn)
	{
		QObject::disconnect(m_currentConn);
	}

	//监听通道的发送结果信号
	m_currentConn = QObject::connect(
		channelInst, &CommunicationChannelBase::sendDataFinished,
		this,
		[=](bool success)
		{
			emit dataSendFinished(success, m_channel);
			QObject::disconnect(m_currentConn); // 任务完成，断开当前连接
			m_currentConn = QMetaObject::Connection(); // 重置
		},
		Qt::QueuedConnection
			);

	// BDS通道：触发带类型的发送信号
	if (m_channel == CommunicationChannel::BDS) {
		BDSChannel* bdsChannel = dynamic_cast<BDSChannel*>(channelInst);
		if (bdsChannel) {
			bdsChannel->setCurrentSendType(m_bdsDataType);
			// 触发基类原始信号（与RadioChannel完全一致）
			emit channelInst->sendDataRequested(m_frame);
		}
		else {
			// 兼容：转换失败则用原有AT指令逻辑
			emit channelInst->sendDataRequested(m_frame);
		}
	}
	else {
		// 非BDS通道：原有逻辑
		bool triggerSuccess = m_commManager->sendData(m_channel, m_frame);
		if (!triggerSuccess) {
			QObject::disconnect(m_currentConn);
			m_currentConn = QMetaObject::Connection();
			emit dataSendFinished(false, m_channel);
		}
	}
	
}

void ConfigWorker::doSendTasksFile()
{
	// 1. 安全获取通道实例（避免重复转换）
	auto* wiredChannel = m_commManager->getTypedChannelInstance<WiredNetworkChannel>(m_channel);
	if (!wiredChannel) {
		emit TaskFileSendFinished(false, m_channel, m_requestId);
		return;
	}
	// 2. 检查通道线程状态
	if (!wiredChannel->thread() || wiredChannel->thread()->isFinished()) {
		qWarning() << "通道线程已退出，无法发送文件";
		emit TaskFileSendFinished(false, m_channel, m_requestId);
		return;
	}
	// 1. 断开上一次未释放的连接（避免残留）
	if (m_currentConn)
	{
		QObject::disconnect(m_currentConn);
	}
	// 2. 打包参数（路径、模块标识、请求ID）
	QVariantMap params;
	params["localPath"] = m_localPath;
	params["remotePath"] = m_remotePath;
	params["sourceModule"] = static_cast<int>(m_sourceModule);
	params["requestId"] = static_cast<qulonglong>(m_requestId);

	// 3. 连接通道发送结果信号
	m_currentConn = QObject::connect(
		wiredChannel, &WiredNetworkChannel::ftpFileSent,
		this,
		[this](bool success)
		{
			emit TaskFileSendFinished(success, m_channel, m_requestId);
			QObject::disconnect(m_currentConn); // 任务完成，断开当前连接
			m_currentConn = QMetaObject::Connection(); // 重置
		},
		Qt::QueuedConnection
			);

	// 4. 触发通道发送（通过扩展接口传递参数）
	bool triggerSuccess = m_commManager->sendFtpFile(m_channel, params);
	if (!triggerSuccess)
	{
		QObject::disconnect(m_currentConn);  // 修正：使用 QObject::disconnect
		m_currentConn = QMetaObject::Connection();
		emit TaskFileSendFinished(false, m_channel, m_requestId);
	}
}

void ConfigWorker::doUploadFile()
{
	// 1. 安全获取通道实例（避免重复转换）
	auto* wiredChannel = m_commManager->getTypedChannelInstance<WiredNetworkChannel>(m_channel);
	if (!wiredChannel)
	{
		emit FileUploadFinished(false, m_channel, m_requestId);
		return;
	}
	// 2. 检查通道线程状态
	if (!wiredChannel->thread() || wiredChannel->thread()->isFinished()) {
		qWarning() << "通道线程已退出，无法发送文件";
		emit FileUploadFinished(false, m_channel, m_requestId);
		return;
	}
	// 1. 断开上一次未释放的连接（避免残留）
	if (m_currentConn)
	{
		QObject::disconnect(m_currentConn);
	}
	// 2. 打包参数（路径、模块标识、请求ID）
	QVariantMap params;
	params["localPath"] = m_localPath;
	params["remotePath"] = m_remotePath;
	params["sourceModule"] = static_cast<int>(m_sourceModule);
	params["requestId"] = static_cast<qulonglong>(m_requestId);

	// 3. 连接通道发送结果信号
	m_currentConn = QObject::connect(
		wiredChannel, &WiredNetworkChannel::ftpFileSent,
		this,
		[this](bool success)
		{
			emit FileUploadFinished(success, m_channel, m_requestId);
			QObject::disconnect(m_currentConn); // 任务完成，断开当前连接
			m_currentConn = QMetaObject::Connection(); // 重置
		},
		Qt::QueuedConnection
			);

	// 4. 触发通道发送（通过扩展接口传递参数）
	bool triggerSuccess = m_commManager->sendFtpFile(m_channel, params);
	if (!triggerSuccess)
	{
		QObject::disconnect(m_currentConn);  // 修正：使用 QObject::disconnect
		m_currentConn = QMetaObject::Connection();
		emit FileUploadFinished(false, m_channel, m_requestId);
	}
}

void ConfigWorker::doDownloadFile()
{
	// 1. 获取通道实例（与上传逻辑一致，确保通道存在）
	auto* wiredChannel = qobject_cast<WiredNetworkChannel*>(m_commManager->getChannelInstance(m_channel));
	if (!wiredChannel) {
		emit FileDownloadFinished(false, m_channel, m_requestId);
		return;
	}

	// 2. 断开上一次未释放的连接（避免信号残留，与其他方法保持一致）
	if (m_currentConn) {
		QObject::disconnect(m_currentConn);
	}

	// 3. 打包下载参数（远程路径、本地保存路径、来源模块、请求ID）
	//    - m_remotePath：下位机的远程文件路径（待下载的文件）
	//    - m_localPath：本地保存路径（下载后存放的位置）
	QVariantMap params;
	params["remotePath"] = m_remotePath;       // 远程文件路径（下位机）
	params["localSavePath"] = m_localPath;     // 本地保存路径（上位机）
	params["sourceModule"] = static_cast<int>(m_sourceModule); // 来源模块（如任务规划）
	params["requestId"] = static_cast<qulonglong>(m_requestId); // 下载请求ID

	// 4. 连接通道的下载完成信号（复用通道的结果信号，或使用专门的下载完成信号）
	//    注意：需与通道中下载完成后发射的信号对应（此处假设通道用 sendDataWithParamsFinished）
	m_currentConn = QObject::connect(
		wiredChannel, &WiredNetworkChannel::ftpFileDownloaded,
		this,
		[=](bool success) {
			// 转发下载结果，携带请求ID
			emit FileDownloadFinished(success, m_channel, m_requestId);
			// 断开当前连接，避免内存泄漏
			QObject::disconnect(m_currentConn);
			m_currentConn = QMetaObject::Connection();
		},
		Qt::QueuedConnection // 跨线程安全连接
			);

	// 5. 触发通道执行下载（通过 CommunicationManager 调用通道的下载接口）
	bool triggerSuccess = m_commManager->downloadFtpFile(m_channel, params);
	if (!triggerSuccess) {
		// 触发失败：及时断开连接，发射失败信号
		QObject::disconnect(m_currentConn);
		m_currentConn = QMetaObject::Connection();
		emit FileDownloadFinished(false, m_channel, m_requestId);
	}
}


void ConfigWorker::doListFiles()
{
	auto* wiredChannel = qobject_cast<WiredNetworkChannel*>(m_commManager->getChannelInstance(m_channel));
	if (!wiredChannel) {
		emit fileListFetched(false, {}, m_channel, m_requestId);
		return;
	}

	if (m_currentConn) {
		QObject::disconnect(m_currentConn);
	}


	QVariantMap params;
	params["remotePath"] = m_remotePath;
	params["sourceModule"] = static_cast<int>(m_sourceModule);
	params["requestId"] = static_cast<qulonglong>(m_requestId);

	//主动从界面获取刷新列表的连接
	m_currentConn = connect(wiredChannel, &WiredNetworkChannel::ftpFileListFetched,
		this, [=](bool success, const QStringList& files, const QString& msg) {
			emit fileListFetched(success, files, m_channel, m_requestId);
			disconnect(m_currentConn);
			m_currentConn = QMetaObject::Connection();
		}, Qt::QueuedConnection);

	bool triggerSuccess = m_commManager->listFtpFiles(m_channel,params);
	if (!triggerSuccess) {
		QObject::disconnect(m_currentConn);
		m_currentConn = QMetaObject::Connection();
		emit fileListFetched(false, {}, m_channel, m_requestId);
	}
}

void ConfigWorker::doStop()
{
	auto* channelInst = m_commManager->getChannelInstance(m_channel);
	if (!channelInst)
	{
		emit stopFinished(false, m_channel);
		return;
	}

	// 1. 断开上一次未释放的连接（避免残留）
	if (m_currentConn)
	{
		QObject::disconnect(m_currentConn);
	}

	// 2. 连接停止完成信号
	m_currentConn = QObject::connect(
		channelInst, &CommunicationChannelBase::stopFinished,
		this,
		[=]()
		{
			// 验证停止结果（通过isRunning判断）
			bool stopSuccess = !channelInst->isRunning();
			emit stopFinished(stopSuccess, m_channel);

			// 断开连接，避免内存泄漏
			QObject::disconnect(m_currentConn);
			m_currentConn = QMetaObject::Connection();
		},
		Qt::QueuedConnection
			);

	// 3. 触发停止操作
	bool triggerSuccess = m_commManager->stopChannel(m_channel);
	if (!triggerSuccess)
	{
		QObject::disconnect(m_currentConn);
		m_currentConn = QMetaObject::Connection();
		emit stopFinished(false, m_channel);
	}
}

void ConfigWorker::doDeleteFile()
{
	// 1. 获取通道实例（与doDownloadFile完全一致）
	auto* wiredChannel = qobject_cast<WiredNetworkChannel*>(m_commManager->getChannelInstance(m_channel));
	if (!wiredChannel) {
		emit FileDeleteFinished(false, m_channel, m_requestId);
		return;
	}

	// 2. 断开残留连接（与现有逻辑一致）
	if (m_currentConn) {
		QObject::disconnect(m_currentConn);
	}

	// 3. 打包删除参数（对齐下载的params）
	QVariantMap params;
	params["remotePath"] = m_remotePath;                  // 删除路径
	params["deleteType"] = static_cast<int>(m_deleteType);// 删除类型（文件/目录）
	params["sourceModule"] = static_cast<int>(m_sourceModule);
	params["requestId"] = static_cast<qulonglong>(m_requestId);

	// 4. 连接删除完成信号（对齐下载的ftpFileDownloaded）
	m_currentConn = QObject::connect(
		wiredChannel, &WiredNetworkChannel::ftpFileDeleted,
		this,
		[=](bool success) {
			emit FileDeleteFinished(success, m_channel, m_requestId);
			QObject::disconnect(m_currentConn);
			m_currentConn = QMetaObject::Connection();
		},
		Qt::QueuedConnection
			);

	// 5. 触发删除（对齐下载的downloadFtpFile）
	bool triggerSuccess = m_commManager->deleteFtpItem(m_channel, params);
	if (!triggerSuccess) {
		QObject::disconnect(m_currentConn);
		m_currentConn = QMetaObject::Connection();
		emit FileDeleteFinished(false, m_channel, m_requestId);
	}
}
