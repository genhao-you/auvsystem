#pragma once
#include<QString>
#pragma execution_character_set("utf-8")
//模块类型
enum class ModuleType
{
	Unknown = 0,
	SelfCheck,//自检模块
	ControlParamDebugging,
	SailControl,
	TaskPlanning,//任务规划模块
	Simulation,//仿真验证模块
	Monitoring,//监控模块
	Data,//数据模块
	Playback,
	Communication
};
//辅助函数
inline QString moduleTypeToString(ModuleType type)
{
	switch (type)
	{
	case ModuleType::SelfCheck:
		return QString("自检");
		break;
	case ModuleType::ControlParamDebugging:
		return QString("控制参数调试");
		break;
	case ModuleType::TaskPlanning:
		return QString("任务规划");
		break;
	case ModuleType::Simulation:
		return QString("仿真验证");
		break;
	case ModuleType::Monitoring:
		return QString("监控模块");
		break;
	case ModuleType::Data:
		return QString("数据模块");
		break;
	case ModuleType::Playback:
		return QString("回访模块");
		break;
	default:
		return QString("未知");
		break;
	}
}