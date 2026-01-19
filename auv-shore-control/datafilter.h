#pragma once
#include"datamessage.h"
// 数据过滤器接口
class DataFilter {
public:
    virtual ~DataFilter() {}

    // 过滤数据，返回true表示数据通过，false表示数据被过滤
    virtual bool filter(const DataMessage& message) = 0;
};