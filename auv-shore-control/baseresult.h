#pragma once
#include<QDateTime>
#include"moduleenum.h"
#include"commonenum.h"
/**
* @brief 所有结果类的基类
*/
class BaseResult 
{
public:
	explicit BaseResult(ModuleType sourceType,ResultStatus status);
	virtual ~BaseResult() = default;

	//获取结果来源模块
	ModuleType sourceType() const;

	//获取结果状态
	ResultStatus status()const;
	void setStatus(ResultStatus status);

	//获取时间戳
	QDateTime timeStamp() const;

	//获取结果描述
	virtual QString description()const;

protected:
	ModuleType m_sourceModule;//结果来源模块
	ResultStatus m_status;
	QDateTime m_timestamp;

};
