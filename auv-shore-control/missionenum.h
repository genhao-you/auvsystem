#pragma once
#pragma execution_character_set("utf-8")
//任务类型
enum class TaskType {
    SAIL,//航行任务
    SILENT,//静默任务
    PATROL_DETECT,//游曳探测任务
    STRIKE,//打击任务
    SHORE_BASED_STRIKE,//岸基打击
    ESCAPE,//逃跑任务
    FLOATING_CALIBRATION,//上浮校准
    FLOATING_COMMUNICATION,//上浮通信
    BEACON_COMMUNICATION,//浮标通信
    RECOVERY//回收任务
};

// 航行模式（定深/定高，用于细化航行任务）
enum class NavigationMode {
	DepthKeeping,  // 定深航行
	HeightKeeping  // 定高航行
};

enum class PlanMode {
    TEST_DEBUG,//试验调试
    SURFACE_RECON,//水面侦察
    UNDERWATER_RECON,//水下侦察
    SURFACE_STRIKE,//水面打击
    UNDERWATER_STRIKE,//水下打击
    LAND_STRIKE//岸基打击
};

inline QString taskTypeToString(TaskType type)
{
	switch (type)
	{
	case TaskType::SAIL:
		return QString("航行任务");
		break;
	case TaskType::SILENT:
		return QString("静默任务");
		break;
	case TaskType::PATROL_DETECT:
		return QString("游曳探测任务");
		break;
	case TaskType::STRIKE:
		return QString("打击任务");
		break;
	case TaskType::SHORE_BASED_STRIKE:
		return QString("岸基打击任务");
		break;
	case TaskType::ESCAPE:
		return QString("逃跑任务");
		break;
	case TaskType::FLOATING_CALIBRATION:
		return QString("上浮校准任务");
		break;
	case TaskType::FLOATING_COMMUNICATION:
		return QString("上浮通信任务");
		break;	
	case TaskType::BEACON_COMMUNICATION:
		return QString("浮标通信任务");
		break;
	case TaskType::RECOVERY:
		return QString("回收任务");
		break;
	default:
		return QString("未知");
		break;
	}
}
inline QString planModeToString(PlanMode mode)
{
	switch (mode)
	{
	case PlanMode::TEST_DEBUG:
		return QString("试验调试");
		break;
	case PlanMode::SURFACE_RECON:
		return QString("水面侦察");
		break;
	case PlanMode::UNDERWATER_RECON:
		return QString("水下侦察");
		break;
	case PlanMode::SURFACE_STRIKE:
		return QString("水面打击");
		break;
	case PlanMode::UNDERWATER_STRIKE:
		return QString("水下打击");
		break;
	case PlanMode::LAND_STRIKE:
		return QString("岸基打击");
		break;
	default:
		return QString("未知");
		break;
	}
}

// 航行模式与字符串转换
inline QString navigationModeToString(NavigationMode mode) {
	switch (mode) {
	case NavigationMode::DepthKeeping: return "定深航行";
	case NavigationMode::HeightKeeping: return "定高航行";
	default: return "未知航行模式";
	}
}