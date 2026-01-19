#pragma once
// 通信通道类型枚举
#include<QMetaType>
#include"baseresult.h"
enum class CommunicationChannel {
    WiredNetwork,     // 有线网络通信
    WirelessNetwork,  // 无线网络通信
    Radio,            // 无线电通信
    BDS,              // 北斗卫星通信
    WaterAcoustic, // 水声通信
    Unknown           // 未知类型
};
// 通信通道状态枚举（监控核心维度）
enum class CommunicationChannelStatus {
    Stopped,    // 已停止
    Running,    // 运行中
    Error,      // 异常
    Initializing// 初始化中
};

//注册枚举类型到qt元对象系统
Q_DECLARE_METATYPE(CommunicationChannel)