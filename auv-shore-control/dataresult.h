#pragma once
#include "baseresult.h"
#include "moduleenum.h"

/**
 * @brief 数据模块结果基类
 * 所有数据模块相关结果的父类
 */
class DataResult : public BaseResult {
public:
    explicit DataResult(ModuleType sourceType, ResultStatus status)
        : BaseResult(sourceType, status) {}

    virtual ~DataResult() = default;
};