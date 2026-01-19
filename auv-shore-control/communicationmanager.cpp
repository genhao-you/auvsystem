#include "communicationmanager.h"
#include"messagedispatcher.h"
#include<QCoreApplication>
//单例初始化
CommunicationManager& CommunicationManager::instance()
{
    static CommunicationManager instance;
    return instance;
}
CommunicationManager::CommunicationManager(QObject* parent) :QObject(parent)
{
    initChannels();
    m_channelConfigured[CommunicationChannel::Radio] = false;
    m_channelConfigured[CommunicationChannel::BDS] = false;
    m_channelConfigured[CommunicationChannel::WaterAcoustic] = false;
    m_channelConfigured[CommunicationChannel::WiredNetwork] = false;
    m_channelConfigured[CommunicationChannel::WirelessNetwork] = false;
    m_timestampProcessor = new TimestampProcessor(this);
    m_dataSelector = new DataSourceSelector(this);
 
    // 设置通道优先级（可根据需要调整）
    m_dataSelector->setChannelPriority(CommunicationChannel::WaterAcoustic, 5);
    m_dataSelector->setChannelPriority(CommunicationChannel::BDS, 4);
    m_dataSelector->setChannelPriority(CommunicationChannel::Radio, 3);
    m_dataSelector->setChannelPriority(CommunicationChannel::WiredNetwork, 2);
    m_dataSelector->setChannelPriority(CommunicationChannel::WirelessNetwork, 1);

    // 连接信号和槽
    for (auto channel : m_channels) {
        connect(channel, &CommunicationChannelBase::dataReceived,
            m_timestampProcessor, &TimestampProcessor::processData);
        connect(channel, &CommunicationChannelBase::statusChanged,
            this, [this, channel](const QString& status)
            {
                emit channelStatusChanged(channel->type(), status);
            });
        connect(channel, &CommunicationChannelBase::errorOccurred,
            this, [this, channel](const QString& error)
            {
                emit channelError(channel->type(), error);
            });
        auto* wiredChannel = qobject_cast<WiredNetworkChannel*>(channel);
        if (wiredChannel)
        {
            // 使用lambda表达式适配参数，添加通道类型参数
            connect(wiredChannel, &WiredNetworkChannel::ftpFileListFetched,
                this, [this](bool success, const QStringList& files, 
                    const QString& msg)
                {
                    // 传递通道类型参数给槽函数
                    emit ftpFileListFetched(CommunicationChannel::WiredNetwork, success,
                        files, msg);
                }, Qt::QueuedConnection);

            connect(wiredChannel, &WiredNetworkChannel::uploadProgressUpdated,
                this, [this](int progress)
                {
                    // 传递通道类型参数给槽函数
                    emit ftpUploadProgress(CommunicationChannel::WiredNetwork, progress);
                }, Qt::QueuedConnection);

            connect(wiredChannel, &WiredNetworkChannel::downloadProgressUpdated,
                this, [this](int progress)
                {
                    // 传递通道类型参数给槽函数
                    emit ftpDownloadProgress(CommunicationChannel::WiredNetwork, progress);
                }, Qt::QueuedConnection);

        }

    }
    //数据处理链路
    connect(m_timestampProcessor, &TimestampProcessor::dataProcessed,
        m_dataSelector, &DataSourceSelector::selectBestData);

    connect(m_dataSelector, &DataSourceSelector::dataSelected,
        this, &CommunicationManager::responseReceived);

        
}

CommunicationManager::~CommunicationManager() {
    stopAllChannels();

    // 清理线程
    for (auto thread : m_threads) {
        thread->quit();
        if (!thread->wait(1000))
        {
            thread->terminate();
        }
        delete thread;
    }
    m_threads.clear();

    //清理通道
    for (auto channel : m_channels)
    {
        delete channel;
    }
    m_channels.clear();
    m_channelMap.clear();

    delete m_timestampProcessor;
    delete m_dataSelector;
}

void CommunicationManager::startAllChannels() {
    QMutexLocker locker(&m_mutex);

    for (auto channel : m_channels)
    {
        if (!channel->isRunning())
        {
            bool success = channel->start();
            if (!success)
            {
                emit channelError(channel->type(), "启动失败");
            }
            else
            {
                emit channelStatusChanged(channel->type(), "运行中");
            }
        }
    }
}

void CommunicationManager::stopAllChannels() {
    QMutexLocker locker(&m_mutex);
    for (auto channel : m_channels) {
        if (channel->isRunning()) {
            channel->stop();
            emit channelStatusChanged(channel->type(), "已停止");
        }
    }
}

bool CommunicationManager::startChannel(CommunicationChannel channel)
{
   
    CommunicationChannelBase* channelInst = getChannelInstance(channel);
    if (!channelInst)
    {
        qWarning() << "启动通道失败：通道不存在(" << static_cast<int>(channel) << ")";
        return false;
    }
    QMutexLocker locker(&m_mutex);
    //关键检查：通道必须配置才能启动
    if (!m_channelConfigured[channel])
    {
        qWarning() << "通道未配置，无法启动：" << static_cast<int> (channel);
        emit channelError(channel, "启动失败：通道未配置");
        return false;
    }
    if (channelInst->isRunning())
    {
        qInfo() << "通道已在运行(" << static_cast<int>(channel) << ")";
        return true;
    }

    //发射信号触发启动(由通道的startImpl处理)
    emit channelInst->startRequested();
    //监听启动结果
    QMetaObject::Connection conn;
    conn = connect(channelInst, &CommunicationChannelBase::startFinished, this, [=](bool success)
        {
            if (success)
            {
                // 关键：仅启动成功且状态变化时发射
                emit channelStatusChanged(channel, "运行中");
            }
            else
            {
                emit channelError(channel, "启动失败");
            }
            disconnect(conn);
        }, Qt::QueuedConnection
    );
    // 返回true仅表示“成功触发启动流程”，不代表启动成功
    return true;
}

bool CommunicationManager::stopChannel(CommunicationChannel channel)
{
    CommunicationChannelBase* channelInst = getChannelInstance(channel);
    if (!channelInst)
    {
        qWarning() << "停止通道失败：通道不存在(" << static_cast<int>(channel) << ")";
        return false;
    }
    QMutexLocker locker(&m_mutex);
    if (!channelInst->isRunning())
    {
        qInfo() << "通道已停止(" << static_cast<int>(channel) << ")";
        return true;
    }

    //调用通道的stop方法
    channelInst->stop();
    emit channelStatusChanged(channel, "已停止");
}

bool CommunicationManager::isChannelRunning(CommunicationChannel channel) const
{
    CommunicationChannelBase* channelInst = getChannelInstance(channel);
    if (!channelInst)
    {
        return false;
    }
    QMutexLocker locker(&m_mutex);
    return channelInst->isRunning();
}

bool CommunicationManager::sendData(CommunicationChannel channel, const QByteArray& data) {
    QMutexLocker locker(&m_mutex);
    if (!isChannelValid(channel))
    {
        emit channelError(channel, "通道无效（未初始化）");
        return false;
    }
    auto* channelInst = m_channelMap[channel];
    if (!channelInst) {
        emit channelError(channel, "通道实例不存在");
        return false;
    }
    if (!channelInst->isRunning()) {
        emit channelError(channel, "通道未运行，无法发送数据");
        return false;
    }
   
    emit channelInst->sendDataRequested(data);
    return true;//仅表示触发成功，实际结果通过sendDataFinished获取


}


QString CommunicationManager::channelStatus(CommunicationChannel channel) const {
    QMutexLocker locker(&m_mutex);

    if (isChannelValid(channel))
    {
        return m_channelMap[channel]->statusInfo();
    }

    return "未知通道";
}

bool CommunicationManager::setChannelConfig(CommunicationChannel channel, const ChannelConfigPtr& config) {
    QMutexLocker locker(&m_mutex);

    if (!isChannelValid(channel)&&!config) 
    {
        qWarning() << "无效通道或空配置：" << static_cast<int>(channel);
        return false;
      
    }
    CommunicationChannelBase* channelInst = m_channelMap[channel];
    //发射信号触发配置更新(由通道的setconfigImpl处理)
    emit channelInst->setConfig(config);
    //监听配置结果
    QMetaObject::Connection conn;
    conn = connect(channelInst, &CommunicationChannelBase::setConfigFinished, this, [=](bool success)
        {
            m_channelConfigured[channel] = success;
            disconnect(conn);//单次触发后断开连接
        }, Qt::QueuedConnection);
    return true;


}

bool CommunicationManager::sendFtpFile(CommunicationChannel channel, const QVariantMap& params)
{
    auto* wiredChannel = getValidFtpChannel(channel);
    if (!wiredChannel) return false;
    return wiredChannel->sendFtpFile(params);
}

bool CommunicationManager::downloadFtpFile(CommunicationChannel channel, const QVariantMap& params)
{
    auto* wiredChannel = getValidFtpChannel(channel);
    if (!wiredChannel) return false;

    return wiredChannel->downloadFtpFile(params);
}

bool CommunicationManager::listFtpFiles(CommunicationChannel channel, const QVariantMap& params)
{
    auto* wiredChannel = getValidFtpChannel(channel);
    if (!wiredChannel) return false;

    return wiredChannel->listFtpFiles(params);
    return false;
}

bool CommunicationManager::deleteFtpItem(CommunicationChannel channel, const QVariantMap& params)
{
    auto* wiredChannel = getValidFtpChannel(channel);
    if (!wiredChannel) return false;

    return wiredChannel->deleteFtpItem(params);
}


void CommunicationManager::initChannels() {
    // 创建所有通信通道
    WiredNetworkChannel* wiredChannel = new WiredNetworkChannel();
    WirelessNetworkChannel* wirelessChannel = new WirelessNetworkChannel();
    RadioChannel* radioChannel = new RadioChannel();
    BDSChannel* bdsChannel = new BDSChannel();
    WaterAcousticChannel* acousticChannel = new WaterAcousticChannel();

    // 添加到通道列表
    m_channels << wiredChannel << wirelessChannel << radioChannel << bdsChannel << acousticChannel;

    // 填充通道映射
    m_channelMap[CommunicationChannel::WiredNetwork] = wiredChannel;
    m_channelMap[CommunicationChannel::WirelessNetwork] = wirelessChannel;
    m_channelMap[CommunicationChannel::Radio] = radioChannel;
    m_channelMap[CommunicationChannel::BDS] = bdsChannel;
    m_channelMap[CommunicationChannel::WaterAcoustic] = acousticChannel;

    // 为每个通道创建线程
    for (auto channel : m_channels) {
        QThread* thread = new QThread(this);
        channel->moveToThread(thread);
        m_threads << thread;
        thread->start();
        // 强制移到主线程
        acousticChannel->moveToThread(QCoreApplication::instance()->thread());
        qInfo() << "[CommManager] 水声通道已移至主线程，线程ID：" << (quintptr)acousticChannel->thread();
    }
}

bool CommunicationManager::isChannelValid(CommunicationChannel channel) const
{
    return m_channelMap.contains(channel) && m_channelMap[channel] != nullptr;
}

WiredNetworkChannel* CommunicationManager::getValidFtpChannel(CommunicationChannel channel)
{
    CommunicationChannelBase* baseChannel = getChannelInstance(channel);
    if (!baseChannel) {
        emit channelError(channel, "通道不存在");
        return nullptr;
    }

    auto* wiredChannel = qobject_cast<WiredNetworkChannel*>(baseChannel);
    if (!wiredChannel) {
        emit channelError(channel, "通道不支持FTP功能");
        return nullptr;
    }

    {
        QMutexLocker locker(&m_mutex);
        if (!wiredChannel->isRunning()) {
            emit channelError(channel, "通道未运行");
            return nullptr;
        }
    }

    return wiredChannel;
}

CommunicationChannelBase* CommunicationManager::getChannelInstance(CommunicationChannel channel) const
{
    QMutexLocker locker(&m_mutex);
    // 双重检查：确保通道存在且线程有效
    if (m_channelMap.contains(channel)) {
        CommunicationChannelBase* channelInst = m_channelMap[channel];
        if (channelInst && channelInst->thread() && !channelInst->thread()->isFinished()) {
            return channelInst;
        }
        qWarning() << "通道实例无效或线程已退出:" << static_cast<int>(channel);
    }

    return nullptr;
}
