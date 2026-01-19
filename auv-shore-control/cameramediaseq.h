#pragma once
#include <QMutex>
#include <cstdint>

// 声明全局序号计数器（extern表示仅声明，定义在.cpp中）
extern uint16_t g_cameraMediaSeq;
// 声明线程安全锁
extern QMutex g_seqMutex;

/**
 * @brief 获取下一个唯一序号（仅递增，不关联日期）
 */
uint16_t getNextCameraMediaSeq();
