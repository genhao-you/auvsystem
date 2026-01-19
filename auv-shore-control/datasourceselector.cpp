#include "datasourceselector.h"
#include<qDebug>
DataSourceSelector::DataSourceSelector(QObject* parent) : QObject(parent) {}

void DataSourceSelector::setChannelPriority(CommunicationChannel channel, int priority) {
    QMutexLocker locker(&m_mutex);
    m_channelPriorities[channel] = priority;
}

void DataSourceSelector::addConflictResolver(CommunicationChannel channelA, CommunicationChannel channelB,
    std::function<bool(const DataMessage&, const DataMessage&)> resolver) {
    QMutexLocker locker(&m_mutex);
    auto key = std::make_pair(
        std::min(channelA, channelB),
        std::max(channelA, channelB)
    );
    m_m_conflictResolvers[key] = resolver;
}

void DataSourceSelector::registerBDSInnerOuterKeep(MessageType targetMsgType, CommunicationChannel targetChannel)
{
    QMutexLocker locker(&m_mutex);
    if (targetChannel != CommunicationChannel::BDS) {
        qWarning() << "[DataSourceSelector] 仅支持BDS通道";
        return;
    }
    m_bdsInnerOuterKeepConfig[static_cast<int>(targetMsgType)] = targetChannel;
}

void DataSourceSelector::selectBestData(const DataMessage& message) {
    QMutexLocker locker(&m_mutex);
    ////1.添加新消息到队列
    //m_recentMessages.enqueue(message);
    //if (m_recentMessages.size() > MAX_QUEUE_SIZE)
    //{
    //    m_recentMessages.dequeue();
    //}
    //// 2. 按“通道+消息类型+指令类型”分组（替代messageId，无需额外结构体）
    //QList<DataMessage> sameGroupMessages;
    //for (const auto& msg : m_recentMessages)
    //{
    //    // 分组条件：通道相同、消息类型相同、指令类型相同
    //    if (msg.channel() == message.channel() &&
    //        msg.type() == message.type() &&
    //        msg.commandType() == message.commandType())
    //    {
    //        sameGroupMessages.append(msg);
    //    }
    //}

    //// 3. 判断是否为北斗需拆分指令（按MessageType，无需messageId）
    //bool needBDSInnerOuterSplit = false;
    //CommunicationChannel targetChannel = CommunicationChannel::BDS;
    //int msgType = static_cast<int>(message.type());
    //if (m_bdsInnerOuterKeepConfig.count(msgType) > 0 && message.channel() == targetChannel)
    //{
    //    needBDSInnerOuterSplit = true;
    //}

    //// ===================== 北斗拆分逻辑（无messageId，用data()判断空） =====================
    //if (needBDSInnerOuterSplit)
    //{
    //    QMap<CommandType, DataMessage> latestMsgByCmdType;
    //    latestMsgByCmdType[CommandType::Internal] = DataMessage();
    //    latestMsgByCmdType[CommandType::External] = DataMessage();

    //    for (const auto& msg : sameGroupMessages)
    //    {
    //        if (msg.channel() != targetChannel) continue;
    //        CommandType cmdType = msg.commandType();
    //        if (cmdType != CommandType::Internal && cmdType != CommandType::External) continue;

    //        // 空判断：直接用data().isEmpty()，无需messageId和isNull()
    //        bool storedMsgEmpty = latestMsgByCmdType[cmdType].data().isEmpty();
    //        if (storedMsgEmpty || msg.timestamp() > latestMsgByCmdType[cmdType].timestamp())
    //        {
    //            latestMsgByCmdType[cmdType] = msg;
    //        }
    //    }

    //    DataMessage innerMsg = latestMsgByCmdType[CommandType::Internal];
    //    DataMessage outerMsg = latestMsgByCmdType[CommandType::External];

    //    // 发射有效消息：判断data是否为空
    //    if (!innerMsg.data().isEmpty()) {
    //        emit dataSelected(innerMsg);
    //    }
    //    if (!outerMsg.data().isEmpty()) {
    //        emit dataSelected(outerMsg);
    //    }

    //    return;
    //}

    //// ===================== 普通消息逻辑（无messageId） =====================
    //QMap<CommunicationChannel, DataMessage> latestMessages;
    //for (const auto& msg : sameGroupMessages)
    //{
    //    if (!latestMessages.contains(msg.channel()) || msg.timestamp() > latestMessages[msg.channel()].timestamp())
    //    {
    //        latestMessages[msg.channel()] = msg;
    //    }
    //}

    //if (latestMessages.size() == 1)
    //{
    //    DataMessage validMsg = latestMessages.constBegin().value();
    //    if (!validMsg.data().isEmpty()) {
    //        emit dataSelected(validMsg);
    //    }
    //    return;
    //}

    //auto it = latestMessages.constBegin();
    //if (it == latestMessages.constEnd()) return;
    //DataMessage bestMessage = it.value();
    //++it;
    //for (; it != latestMessages.constEnd(); ++it)
    //{
    //    if (!isFirstBetter(bestMessage, it.value()))
    //    {
    //        bestMessage = it.value();
    //    }
    //}

    //if (!bestMessage.data().isEmpty()) {
    //    emit dataSelected(bestMessage);
    //}
    emit dataSelected(message);
}

bool DataSourceSelector::isFirstBetter(const DataMessage& a, const DataMessage& b)
{
    //相同通道无需比较
    if (a.channel() == b.channel())
        return true;

    //创建标准化的通道对
    auto channelPair = std::make_pair(
        std::min(a.channel(), b.channel()),
        std::max(a.channel(), b.channel())
    );

    //1.检查是否有冲突解决器
    auto resolverIt = m_m_conflictResolvers.find(channelPair);
    if (resolverIt != m_m_conflictResolvers.end())
    {
        //根据通道顺序调用解决器
        if (a.channel() == channelPair.first)
        {
            return resolverIt->second(a, b);
        }
        else
        {
            return !resolverIt->second(b, a);
        }
    }
    //2.使用通道优先级比较
    int prioA = m_channelPriorities.value(a.channel(), 0);
    int prioB = m_channelPriorities.value(b.channel(), 0);
    if (prioA != prioB)
    {
        return prioA > prioB;//优先级高的胜出
    }
    //比较消息自身优先级
    if (a.priority() != b.priority())
    {
        return a.priority() > b.priority();
    }

    //4.比较时间戳（更新者优先）
    if (a.timestamp()!= b.timestamp())
    {
        return a.timestamp() > b.timestamp();
    }
    //5.默认规则：枚举通道小的优先
    return a.channel() < b.channel();
}

size_t DataSourceSelector::PairHash::operator()(const std::pair<CommunicationChannel, CommunicationChannel>& p) const
{
    return static_cast<size_t>(p.first) * 1000 + static_cast<size_t>(p.second);
}
