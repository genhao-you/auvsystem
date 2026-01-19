#ifndef OBSERVER_H
#define OBSERVER_H

#include "baseresult.h"
/**
 * @brief 观察者接口
 * 
 * 实现观察者模式，用于接收自检结果通知
 */
class IResultObserver {
   
public:
    virtual ~IResultObserver() = default;
    
    /**
     * @brief 当自检结果更新时调用
     * @param result 最新的自检结果
     */
    virtual void onResultUpdated(const BaseResult* result) = 0;
};

#endif // OBSERVER_H
    