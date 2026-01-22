#pragma once
#include "selfcheckenum.h"
#include "controlparamdefs.h"
// 枚举与字符串转换工具（数据驱动）
class EnumConverter {
public:
    // 子系统转字符串
    static QString subsystemToString(Subsystem subsystem) {
        for (const auto& entry : DeviceData::subsystemNames) {
            if (entry.first == static_cast<uint8_t>(subsystem)) {
                return entry.second;
            }
        }
        return "未知子系统";
    }

    // 器件ID转字符串
    static QString deviceIdToString(DeviceId deviceId) {
        for (const auto& entry : DeviceData::deviceInfos) {
            if (entry.first == deviceId) {
                return entry.second.name;
            }
        }
        return "未知器件";
    }

    // 命令码转字符串（使用函数映射表，避免冗长的switch）
    static QString commandCodeToString(DeviceId deviceId, CommandCode code) {
        static const std::unordered_map<uint8_t, CommandDescription> commandMap = initCommandMap();

        uint8_t codeValue = static_cast<uint8_t>(code);
        auto it = commandMap.find(codeValue);

        if (it != commandMap.end()) {
            return it->second.getDescription(deviceId);
        }

        return QString("未知命令（编码0x%1）").arg(codeValue, 2, 16, QChar('0')).toUpper();
    }

    static QString payloadResultToString(PayloadResult result) {
        switch (result) {
        case PayloadResult::Success: return "成功（0x00）";
        case PayloadResult::Failure: return "失败（0xFF）";
        default: return "未知状态";
        }
    }

    static QString ResultStatusToString(ResultStatus status) {
        switch (status) {
        case ResultStatus::Success: return "成功";
        case ResultStatus::Failed: return "失败";
        case ResultStatus::Timeout: return "超时";
        case ResultStatus::Pending: return "等待中";
        default: return "未知状态";
        }
    }

    static QString ResultStatusToStyle(ResultStatus status) {
        switch (status) {
        case ResultStatus::Success: return "color: green;";
        case ResultStatus::Failed: return "color: red;";
        case ResultStatus::Timeout: return "color: orange;";
        case ResultStatus::Pending: return "color: blue;";
        default: return "";
        }
    }
    static QString controlTypeToString(ControlType type) {
        switch (type) {
        case ControlType::DepthControl:    return "深度控制";
        case ControlType::HeightControl:   return "高度控制";
        case ControlType::HeadingControl:  return "航向控制";
        default:                           return QString("无效类型(%1)").arg(static_cast<int>(type));
        }
    }

private:
    // 初始化命令映射表（完整版本）
    static std::unordered_map<uint8_t, CommandDescription> initCommandMap() {
        std::unordered_map<uint8_t, CommandDescription> map;

        // ------------------------------ 通用命令
        map[static_cast<uint8_t>(CommandCode::Invalid)] = { [](DeviceId) {
            return "无效命令";
        } };

        map[static_cast<uint8_t>(CommandCode::Common_PowerOn)] = { [](DeviceId) {
            return "上电";
        } };

        map[static_cast<uint8_t>(CommandCode::Common_PowerOff)] = { [](DeviceId) {
            return "下电";
        } };

        map[static_cast<uint8_t>(CommandCode::Common_Enable)] = { [](DeviceId) {
            return "使能";
        } };

        map[static_cast<uint8_t>(CommandCode::Common_Disable)] = { [](DeviceId) {
            return "停止工作";
        } };

        // ------------------------------ 编码0x03（多场景复用）
        map[0x03] = { [](DeviceId id) {
            if (DeviceTypeHelper::isThruster(id))
                return "参数重置（转速/运行时间归0）";
            if (DeviceTypeHelper::isRudder(id))
                return "设置舵角";
            if (DeviceTypeHelper::isBuoyancy(id))
                return "设置浮力";
            if (id == DeviceId::DropWeight)
                return"抛载释放";
            if (id == DeviceId::BeidouComm)
                return "目标查询";
            if (id == DeviceId::DepthMeter)
                return "设置深度计基准值（校准）";
            if (id == DeviceId::Altimeter)
                return "获取高度值";
            if (id == DeviceId::INS)
                return "设置装订对准";
            if (id == DeviceId::DVL)
                return "获取对底速度";
            if (id == DeviceId::CameraLight)
                return "开始拍摄存储视频";
            if (DeviceTypeHelper::isBattery(id))
                return "获取电池电流";
            if (DeviceTypeHelper::isSonar(id))
                return "设置声纳参数";
            if (id == DeviceId::USBL)
                return "获取USBL信标版本号";
            if (id == DeviceId::AcousticComm)
                return "水声通信自检";
            return "参数重置/校准（未知场景）";
        } };

        // ------------------------------ 编码0x04（多场景复用）
        map[0x04] = { [](DeviceId id) {
            if (DeviceTypeHelper::isThruster(id))
                return "使能（开始工作）";
            if (DeviceTypeHelper::isRudder(id))
                return "读取运行时间";
            if (DeviceTypeHelper::isBuoyancy(id))
                return "读取当前浮力";
            if (id == DeviceId::DropWeight)
                return "抛载复位";
            if (id == DeviceId::BeidouComm)
                return "目标修改";
            if (id == DeviceId::DepthMeter)
                return "获取深度值（小数点后四位，单位米）";
            if (id == DeviceId::INS)
                return "设置惯导与DVL标定";
            if (id == DeviceId::DVL)
                return "获取DVL对水速度";
            if (id == DeviceId::CameraLight)
                return "关闭存储拍摄视频";
            if (DeviceTypeHelper::isBattery(id))
                return "获取平均温度";
            if (DeviceTypeHelper::isSonar(id))
                return "开始工作";
            return "使能/获取深度（未知场景）";
            if (id == DeviceId::AcousticComm)
                return "水声通信通讯测试";
        } };

        // ------------------------------ 编码0x05（多场景复用）
        map[0x05] = { [](DeviceId id) {
            if (DeviceTypeHelper::isThruster(id))
                return "设置推进器转速（闭环控制，uint16）";
            if (DeviceTypeHelper::isRudder(id))
                return "获取舵机电压值";
            if (DeviceTypeHelper::isBuoyancy(id))
                return "急停";
            if (id == DeviceId::BeidouComm)
                return "查询白名单";
            if (id == DeviceId::INS)
                return "获取惯导UTC日期（YYMMDD）";
            if (id == DeviceId::CameraLight)
                return "拍照";
            if (DeviceTypeHelper::isBattery(id))
                return "高压上电";
            if (DeviceTypeHelper::isSonar(id))
                return "停止工作";
            return "设置转速（未知场景）";
        } };

        // ------------------------------ 编码0x06（多场景复用）
        map[0x06] = { [](DeviceId id) {
            if (DeviceTypeHelper::isThruster(id))
                return "设置推进器占空比（开环控制，0-2000）";
            if (DeviceTypeHelper::isRudder(id))
                return "获取舵机角（*10度，uint16）";
            if (id == DeviceId::BeidouComm)
                return "添加白名单成员";
            if (id == DeviceId::INS)
                return "获取惯导UTC时间（HHMMSS.SS）";
            if (DeviceTypeHelper::isBattery(id))
                return "高压下电";
            if (DeviceTypeHelper::isBuoyancy(id))
                return "读取当前电位计";
            return "设置占空比/舵机角（未知场景）";
        } };

        // ------------------------------ 编码0x07（多场景复用）
        map[0x07] = { [](DeviceId id) {
            if (DeviceTypeHelper::isThruster(id))
                return "设置推进器连续运行时间（单位秒，uint16）";
            if (DeviceTypeHelper::isRudder(id))
                return "获取舵机电流";
            if (id == DeviceId::BeidouComm)
                return "删除白名单成员";
            if (DeviceTypeHelper::isBattery(id))
                return "锁止";
            if (id == DeviceId::INS)
                return "获取惯导姿态（航向/俯仰/横滚角）";
            return "设置运行时间/反馈频率（未知场景）";
        } };

        // ------------------------------ 编码0x08（多场景复用）
        map[0x08] = { [](DeviceId id) {
            if (DeviceTypeHelper::isThruster(id))
                return "设置加速时间（uint16）";
            if (DeviceTypeHelper::isRudder(id))
                return "获取舵机温度";
            if (id == DeviceId::BeidouComm)
                return "获取定位信息";
            if (DeviceTypeHelper::isBattery(id))
                return "解锁";
            if (id == DeviceId::INS)
                return "获取惯导载体系速度（右前上）";
            return "获取运行时间/反馈频率（未知场景）";
        } };

        // ------------------------------ 编码0x09（多场景复用）
        map[0x09] = { [](DeviceId id) {
            if (DeviceTypeHelper::isThruster(id))
                return "获取推进器连续运行时间（uint16）";
            if (DeviceTypeHelper::isRudder(id))
                return "设置舵角零位";
            if (id == DeviceId::BeidouComm)
                return "查看固件版本信息";
            if (id == DeviceId::INS)
                return "获取惯导地理系速度（东北天）";
            return "获取运行时间（未知场景）";
        } };

        // ------------------------------ 编码0x0A（多场景复用）
        map[0x0A] = { [](DeviceId id) {
            if (DeviceTypeHelper::isThruster(id))
                return "获取推进器占空比";
            if (id == DeviceId::BeidouComm)
                return "恢复出厂设置并重启";
            if (id == DeviceId::INS)
                return "获取惯导位置";
            return "命令编码0x0A（未知场景）";
        } };

        // ------------------------------ 编码0x0B（多场景复用）
        map[0x0B] = { [](DeviceId id) {
            if (DeviceTypeHelper::isThruster(id))
                return "获取推进器电流";
            if (id == DeviceId::BeidouComm)
                return "设备重启";
            if (id == DeviceId::INS)
                return "获取陀螺仪角速度（x.xxxx航向角(°/s，小数点后2位)、y.yyyy俯仰角(°/s，小数点后2位)、z.zzzz横滚角(°/s，小数点后2位)）";
            return "命令编码0x0B（未知场景）";
        } };

        // ------------------------------ 编码0x0C（推进器专属）
        map[0x0C] = { [](DeviceId id) {
            if (DeviceTypeHelper::isThruster(id))
                return "获取推进器温度（uint16，需除10）";
            if (id == DeviceId::INS)
                return "获取加速度（x.xxx右向加速度(m/s?，小数点后2位)、y.yyy前向加速度(m/s?，小数点后2位)、z.zzz垂向加速度(m/s?，小数点后2位)）";
            if (id == DeviceId::BeidouComm)
                return "北斗通信测试";
            return "命令编码0x0C（未知场景）";
        } };

        // ------------------------------ 编码0x0D（推进器专属）
        map[0x0D] = { [](DeviceId id) {

            if (DeviceTypeHelper::isThruster(id))
                return "获取推进器转速（uint16）";
            if (id == DeviceId::INS)
                return "获取惯导状态码";
            return "命令编码0x0D（未知场景）";
        } };

        // ------------------------------ 编码0x0E（推进器专属）
        map[0x0E] = { [](DeviceId id) {
            if (DeviceTypeHelper::isThruster(id))
                return "获取推进器霍尔值（1-7正常，0/7故障）";
            if (DeviceId::INS == id)
                return "强制DVL组合";
            return "命令编码0x0E（未知场景）";
        } };

        // ------------------------------ 编码0x0F（推进器专属）
        map[0x0F] = { [](DeviceId id) {
            if (DeviceTypeHelper::isThruster(id))
                return "获取推进器母线电压（uint16，需除10）";
            if (DeviceId::INS == id)
                return "自由组合（默认值）";
            return "命令编码0x0F（未知场景）";
        } };

        // ------------------------------ 编码0x10（多场景复用）
        map[0x10] = { [](DeviceId id) {
            if (DeviceTypeHelper::isThruster(id))
                return "读取无维护运行时间(h)";
            return "命令编码0x10（未知场景）";
        } };

        // ------------------------------ 编码0x11（推进器专属）
        map[0x11] = { [](DeviceId id) {
            if (DeviceTypeHelper::isThruster(id))
                return "读取运行时间";
            return "命令编码0x11（未知场景）";
        } };
        // ------------------------------ 编码0x12（推进器专属）
        map[0x12] = { [](DeviceId id) {
            if (DeviceTypeHelper::isThruster(id))
                return "设置转速及时间";
            return "命令编码0x12（未知场景）";
        } };

        // ------------------------------ 编码0xAA（通用故障查询）
        map[0xAA] = { [](DeviceId id) {
            if (DeviceTypeHelper::isThruster(id))
                return "获取推进器故障码（不定字节）";
            if (DeviceTypeHelper::isRudder(id))
                return "获取舵机故障码（不定字节）";
            if (id == DeviceId::DropWeight)
                return "抛载自检并反馈状态";
            if (id == DeviceId::INS)
                return "获取惯导故障类型（0x00正常/0xFF故障）";
            if (id == DeviceId::CameraLight)
                return "获取摄像机与光源故障码";
            if (DeviceTypeHelper::isBattery(id))
                return "获取电池故障类型";
            if (DeviceTypeHelper::isSonar(id))
                return "获取声纳设备状态（故障）";
            if (id == DeviceId::AcousticComm)
                return "获取水声通信设备状态（故障）";
            return "获取故障码（通用）";
        } };

        return map;
    }
};