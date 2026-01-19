#pragma once

#include <QObject>
#include"communicationchannel.h"
#include"datamessage.h"
#include<QMutex>
#include<QQueue>
#include<QHash>
#include<QMap>
#include<unordered_map>
#include<utility>
class DataSourceSelector  : public QObject
{
	Q_OBJECT
public:
    explicit DataSourceSelector(QObject* parent = nullptr);

    // 设置通道优先级
    void setChannelPriority(CommunicationChannel channel, int priority);

    // 添加数据冲突解决器
    void addConflictResolver(CommunicationChannel channelA, CommunicationChannel channelB,
        std::function<bool(const DataMessage&, const DataMessage&)> resolver);

    // 接口：注册“需按CommandType（内外）拆分保留”的北斗指令配置
    void registerBDSInnerOuterKeep(MessageType targetMsgType, CommunicationChannel targetChannel);

    // 选择最佳数据源
    void selectBestData(const DataMessage& message);

signals:
    void dataSelected(const DataMessage& message);
    
private:
    //比较两个消息的优先级
   bool isFirstBetter(const DataMessage& a, const DataMessage& b);
   //哈希函数定义
   struct PairHash
   {
       size_t operator()(const std::pair<CommunicationChannel, CommunicationChannel>& p)const;
   };
   static constexpr int MAX_QUEUE_SIZE = 1000;
   QMutex m_mutex;
    QMap<CommunicationChannel, int> m_channelPriorities;
    using ChannelPair = std::pair<CommunicationChannel, CommunicationChannel>;
    std::unordered_map<
        ChannelPair,
        std::function<bool(const DataMessage&, const DataMessage&)>,
        PairHash
    >m_m_conflictResolvers;
    QQueue<DataMessage> m_recentMessages;
    // 存储“需按CommandType拆分”的配置（msgId -> 目标通道（BDS））
    std::unordered_map<int, CommunicationChannel> m_bdsInnerOuterKeepConfig;
   
};
