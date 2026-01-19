#pragma once
#include <QString>
#include <cstdint>
#include <unordered_set>
#include <unordered_map>
#include <array>
#include <type_traits>
#include <functional>
#include "baseresult.h"
#include "commonenum.h"
// 辅助宏：简化枚举到字符串的映射定义
#define ENUM_MAP_ENTRY(ENUM_VALUE, STR) {static_cast<uint8_t>(ENUM_VALUE), STR}
#define DEVICE_MAP_ENTRY(DEVICE_ID, SUBSYSTEM, STR) {DEVICE_ID, {SUBSYSTEM, STR}}

// 自检子系统
enum class Subsystem : uint8_t {
    Communication,    // 通信系统
    Navigation,       // 导航系统
    DataAcquisition,  // 数据采集
    Propulsion,       // 推进装置
    Steering,         // 转向装置
    Power,            // 能源与续航
    Buoyancy,         // 浮力调节
    Emergency         // 应急系统
};

// 器件ID
enum class DeviceId : uint8_t {
    Unknown = 0x00,
    // 推进装置（Propulsion）
    TailThruster = 0x01,  // 尾推
    FrontVerticalThruster = 0x16,  // 前垂推
    RearVerticalThruster = 0x17,
    FrontSideThruster = 0x18,      // 侧推
    RearSideThruster = 0x19,
    // 转向装置（Steering）
    Rudder1 = 0x02,   // 舵1
    Rudder2 = 0x03,   // 舵2
    Rudder3 = 0x04,   // 舵3
    Rudder4 = 0x05,   // 舵4
    AntennaFold = 0x08,    // 天线折叠机构
    // 浮力调节（Buoyancy）
    BuoyancyAdjust = 0x06,  // 浮调1
 
    // 应急系统（Emergency）
    DropWeight = 0x07,  // 抛载

    // 通信系统（Communication）
   
    BeidouComm = 0x09,     // 北斗（通信）
    AcousticComm = 0x15, // 水声通信

    // 导航系统（Navigation）
    DepthMeter = 0x0A,  // 深度计
    Altimeter = 0x0B,   // 高度计
    USBL = 0x0C,        // USBL
    INS = 0x0D,         // 惯导
    DVL = 0x0E,         // DVL

    // 数据采集（DataAcquisition）
    CameraLight = 0x0F,   // 摄像机与光源
    SideScanSonar = 0x12, // 侧扫声纳
    ForwardSonar = 0x13,  // 前视声纳
    DownwardSonar = 0x14, // 下视声呐

    // 能源与续航（Power）
    PowerBattery1 = 0x10,  // 动力电池1
    PowerBattery2 = 0x1A,  // 动力电池2
    MeterBattery = 0x11   // 仪表电池
};

// 命令编码
enum class CommandCode : uint8_t {
    Invalid = 0x00,
    // -------------------------------   通用命令（多器件共用编码）
    Common_PowerOn = 0x01,                // 上电（通用）
    Common_PowerOff = 0x02,               // 下电（通用）
    Common_Enable = 0x04,       // 使能/获取深度（推进器/深度计等）
    Common_Disable = 0x05,                // 停止工作
   
    // ------------------------------    推进器专属命令
    Thruster_ParamReset = 0x03,           // 参数重置
    Thruster_SetSpeed = 0x05,              // 设置转速
    Thruster_SetDuty = 0x06,              //设置占空比
    Thruster_SetRunTime = 0x07,           //设置连续运行时间
    Thruster_SetAccelTime = 0x08,         //设置加速时间
    Thruster_GetRunTime = 0x09,           //获取连续运行时间
    Thruster_GetDuty = 0x0A,              // 推进器获取占空比
    Thruster_GetCurrent = 0x0B,           // 推进器获取电流
    Thruster_GetTemperature = 0x0C,       // 推进器获取温度
    Thruster_GetSpeed = 0x0D,             // 推进器获取转速
    Thruster_GetHall = 0x0E,              // 推进器获取霍尔值
    Thruster_GetBusVoltage = 0x0F,        // 推进器获取母线电压
    Thruster_GetNoMaintainTime = 0x10,    // 推进器获取无维护时间
    Thruster_GetTotalRunTime = 0x11,      // 推进器获取总运行时间
    Thruster_SetSpeedWithTime = 0x12,          // 推进器设置反转
    
  

    // ------------------------------      舵机/天线折叠机构专属命令
    Rudder_SetAngle = 0x03,               //设置舵角
    Rudder_GetRunTime = 0x04,             //读取运行时间
    Rudder_GetVoltage = 0x05,             // 舵机获取电压
    Rudder_GetAngle = 0x06,               // 舵机获取角度
    Rudder_GetCurrent = 0x07,             // 舵机获取电流
    Rudder_GetTemp = 0x08,                // 舵机获取温度
    Rudder_SetZero = 0x09,                //舵机归零

    // ------------------------------      浮调专属命令
    Buoyancy_SetValue = 0x03,
    Buoyancy_ReadValue = 0x04,
    Buoyancy_Stop = 0x05,

    // ------------------------------       抛载专属命令
    DropWeight_Release = 0x03,             // 抛载释放
    DropWeight_Reset = 0x04,               // 抛载复位

    // ------------------------------       北斗通信专属命令（新协议）
    BeidouComm_GetSignalQuality = 0x01,        // 获取信号质量
    BeidouComm_GetIdentity = 0x02,             // 身份查询（卡号）
    BeidouComm_GetTarget = 0x03,               // 目标查询（远端设备号）
    BeidouComm_SetTarget = 0x04,               // 修改远端设备号
    BeidouComm_GetWhitelist = 0x05,            // 查询白名单
    BeidouComm_AddWhitelist = 0x06,            // 添加白名单成员
    BeidouComm_DelWhitelist = 0x07,            // 删除白名单成员
    BeidouComm_GetPosition = 0x08,             // 获取定位信息（NMEA字符串）
    BeidouComm_GetSysInfo = 0x09,              // 获取系统信息（固件版本）
    BeidouComm_FactoryReset = 0x0A,            // 恢复出厂设置并重启
    BeidouComm_Reboot = 0x0B,                  // 设备重启
    BeidouComm_Test = 0x0C,                    // 通讯测试
    BeidouComm_GetConnectStatus = 0x0D,        //获取连接状态
    //---------------------------------     深度计
    Depthgauge_Calibration = 0x03,
    Depthgauge_GetDepth = 0x04,

    //--------------------------------     高度计
    Altimeter_GetHeight = 0x03,

    //---------------------------------     USBL
    USBL_GetBeaconVersion = 0x03,         // USBL获取信标版本号
    //---------------------------------      惯导
    INS_SetGnssBind = 0x03,                // 惯导设置装订对准
    INS_CalibrationWithDVL = 0x04,         //与DVL标定
    INS_GetUtcDate = 0x05,                 // 惯导获取UTC日期
    INS_GetUtcTime = 0x06,                 // 惯导获取UTC时间
    INS_GetAttitude = 0x07,                // 惯导获取姿态
    INS_GetBodySpeed = 0x08,               // 惯导获取载体系速度
    INS_GetGeoSpeed = 0x09,                // 惯导获取地理系速度
    INS_GetPosition = 0x0A,                // 惯导获取位置
    INS_GetGyroscopeSpeed = 0x0B,          // 惯导获取陀螺仪角速度
    INS_GetAcceleration = 0x0C,            // 惯导获取加速度
    INS_GetStatus = 0x0D,                  // 惯导获取状态码
    INS_CombinedWithDVL = 0x0E,             // 强制与DVL组合
    INS_CombinedAuto = 0x0F,                // 自由组合
    //---------------------------------      DVL
    DVL_GetBottomSpeed = 0x03,              //DVL获取对底速度
    DVL_GetWaterSpeed = 0x04,              // DVL获取对水速度
    DVL_GetStatus = 0x05,                  //获取状态码
    //---------------------------------      摄像机与光源
    CameraLight_StartVideoSave = 0x03,     // 摄像机开始存储视频
    CameraLight_StopVideoSave = 0x04,      // 摄像机停止存储视频
    CameraLight_TakePhoto = 0x05,           //拍照

   // ------------------------------         电池专属命令
    Battery_GetSoc = 0x01,                 // 电池获取SOC
    Battery_GetTotalVoltage = 0x02,        // 电池获取总电压
    Battery_GetCurrent = 0x03,             // 电池获取电流
    Battery_GetAvgTemp = 0x04,             // 电池获取平均温度
    Battery_HVOn = 0x05,                   // 电池高压上电
    Battery_HVOff = 0x06,                  // 电池高压下电
    Battery_Lock = 0x07,                   // 电池锁止
    Battery_Unlock = 0x08,                 // 电池解锁

   // ------------------------------         声呐
   Sonar_SetParameter = 0x03,            //设置参数

   // ------------------------------         水声通信
   AcousticComm_SelfCheck = 0x03,
   AcousticComm_Test = 0x04,
   
   // ------------------------------       通用故障查询命令
   Common_GetFault = 0xAA                  // 获取故障（所有器件通用）
};

// 指令类型枚举（区分内部/外部）
enum class CommandType {
    External, // 外部指令（需构建硬件帧）
    Internal  // 内部指令（AT指令，无需帧封装）
};

// 北斗信号质量枚举
enum class BeidouSignalQuality : uint8_t {
    NoSignal = 0x00,
    VeryWeak = 0x01,
    Weak = 0x02,
    Good = 0x03,
    VeryGood = 0x04,
    Excellent = 0x05
};

// 水声通信上行指令类型（协议定义）
enum class WaterAcousticUplinkCmd {
    Unknown = 0,
    SendFinish = 1,    // 我方发送完成（01 00）
    RecvNotify = 2,    // 对方通知发送（02 02 00）
    DataReceived = 3   // 对方有效数据
};

enum class PayloadResult :uint8_t {
    Success = 0x00,
    Failure = 0xFF
};



// 命令参数枚举
enum class PowerState : uint8_t {
    PowerOn = 0x00,    // 上电
    PowerOff = 0xFF      // 下电
};

enum class EnableState : uint8_t {
    Disable = 0x00,
    Enable = 0xFF
};


// 类型安全的枚举工具类
template<typename EnumType>
class EnumUtils {
    static_assert(std::is_enum_v<EnumType>, "EnumUtils only works with enum types");
    using UnderlyingType = std::underlying_type_t<EnumType>;

public:
    // 将整数安全转换为枚举，如果值无效返回默认值
    static EnumType fromInt(UnderlyingType value, EnumType defaultValue) {
        if (isValid(value)) {
            return static_cast<EnumType>(value);
        }
        return defaultValue;
    }

    // 检查整数值是否为有效的枚举值
    static bool isValid(UnderlyingType value) {
        return getValidValues().count(value) > 0;
    }

private:
    // 获取所有有效的枚举值集合（由特化实现）
    static const std::unordered_set<UnderlyingType>& getValidValues();
};

// 器件信息结构体
struct DeviceInfo {
    Subsystem subsystem;
    QString name;
};

// 命令描述结构体
struct CommandDescription {
    std::function<QString(DeviceId)> getDescription;
};

// 核心数据存储（使用constexpr和const确保不可变性）
namespace DeviceData {
    // 子系统名称映射
    static const std::array<std::pair<uint8_t, const char*>, 8> subsystemNames = { {
        {static_cast<uint8_t>(Subsystem::Communication), "通信系统"},
        {static_cast<uint8_t>(Subsystem::Navigation), "导航系统"},
        {static_cast<uint8_t>(Subsystem::DataAcquisition), "数据采集"},
        {static_cast<uint8_t>(Subsystem::Propulsion), "推进装置"},
        {static_cast<uint8_t>(Subsystem::Steering), "转向装置"},
        {static_cast<uint8_t>(Subsystem::Power), "能源与续航"},
        {static_cast<uint8_t>(Subsystem::Buoyancy), "浮力调节"},
        {static_cast<uint8_t>(Subsystem::Emergency), "应急系统"}
    } };

    // 器件信息映射（包含所属子系统和名称）
    static const std::array<std::pair<DeviceId, DeviceInfo>, 30> deviceInfos = { {
       {DeviceId::Unknown, {Subsystem::Communication, "未知器件"}},
       // 推进装置
       {DeviceId::TailThruster, {Subsystem::Propulsion, "尾推"}},
       {DeviceId::FrontVerticalThruster, {Subsystem::Propulsion, "前垂推"}},
       {DeviceId::RearVerticalThruster, {Subsystem::Propulsion, "后垂推"}},
       {DeviceId::FrontSideThruster, {Subsystem::Propulsion, "前侧推"}},
       {DeviceId::RearSideThruster, {Subsystem::Propulsion, "后侧推"}},
       // 转向装置
       {DeviceId::Rudder1, {Subsystem::Steering, "舵1"}},
       {DeviceId::Rudder2, {Subsystem::Steering, "舵2"}},
       {DeviceId::Rudder3, {Subsystem::Steering, "舵3"}},
       {DeviceId::Rudder4, {Subsystem::Steering, "舵4"}},
       {DeviceId::AntennaFold, {Subsystem::Steering, "天线折叠机构"}},
       // 浮力调节
       {DeviceId::BuoyancyAdjust, {Subsystem::Buoyancy, "浮调"}},
       // 应急系统
       {DeviceId::DropWeight, {Subsystem::Emergency, "抛载"}},
       // 通信系统
       {DeviceId::BeidouComm, {Subsystem::Communication, "北斗（通信）"}},
       {DeviceId::AcousticComm, {Subsystem::Communication, "水声通信"}},
       // 导航系统
       {DeviceId::DepthMeter, {Subsystem::Navigation, "深度计"}},
       {DeviceId::Altimeter, {Subsystem::Navigation, "高度计"}},
       {DeviceId::USBL, {Subsystem::Navigation, "USBL"}},
       {DeviceId::INS, {Subsystem::Navigation, "惯导"}},
       {DeviceId::DVL, {Subsystem::Navigation, "DVL"}},
       // 数据采集
       {DeviceId::CameraLight, {Subsystem::DataAcquisition, "摄像机与光源"}},
       {DeviceId::SideScanSonar, {Subsystem::DataAcquisition, "侧扫声纳"}},
       {DeviceId::ForwardSonar, {Subsystem::DataAcquisition, "前视声纳"}},
       {DeviceId::DownwardSonar, {Subsystem::DataAcquisition, "下视声呐"}},
       // 能源与续航
       {DeviceId::PowerBattery1, {Subsystem::Power, "动力电池1"}},
       {DeviceId::PowerBattery2, {Subsystem::Power, "动力电池2"}},
       {DeviceId::MeterBattery, {Subsystem::Power, "仪表电池"}}
   } };
}

// 器件类型辅助类（使用数据驱动，而非硬编码判断）
class DeviceTypeHelper {
public:
    // 预初始化的有效ID集合
    static const std::unordered_set<uint8_t>& getValidDeviceIds() {
        static const std::unordered_set<uint8_t> ids = []() {
            std::unordered_set<uint8_t> set;
            for (const auto& entry : DeviceData::deviceInfos) {
                set.insert(static_cast<uint8_t>(entry.first));
            }
            return set;
        }();
        return ids;
    }

    static const std::unordered_set<uint8_t>& getValidCommandCodes() {
        static const std::unordered_set<uint8_t> codes = []() {
            std::unordered_set<uint8_t> set;

            // 添加所有CommandCode枚举值
            set.insert(static_cast<uint8_t>(CommandCode::Invalid));
            set.insert(static_cast<uint8_t>(CommandCode::Common_PowerOn));
            set.insert(static_cast<uint8_t>(CommandCode::Common_PowerOff));
            set.insert(static_cast<uint8_t>(CommandCode::Common_Enable));
            set.insert(static_cast<uint8_t>(CommandCode::Common_Disable));

            // 推进器命令
            set.insert(static_cast<uint8_t>(CommandCode::Thruster_ParamReset));
            set.insert(static_cast<uint8_t>(CommandCode::Thruster_SetSpeed));
            set.insert(static_cast<uint8_t>(CommandCode::Thruster_SetDuty));
            set.insert(static_cast<uint8_t>(CommandCode::Thruster_SetRunTime));
            set.insert(static_cast<uint8_t>(CommandCode::Thruster_SetAccelTime));
            set.insert(static_cast<uint8_t>(CommandCode::Thruster_GetRunTime));
            set.insert(static_cast<uint8_t>(CommandCode::Thruster_GetDuty));
            set.insert(static_cast<uint8_t>(CommandCode::Thruster_GetCurrent));
            set.insert(static_cast<uint8_t>(CommandCode::Thruster_GetTemperature));
            set.insert(static_cast<uint8_t>(CommandCode::Thruster_GetSpeed));
            set.insert(static_cast<uint8_t>(CommandCode::Thruster_GetHall));
            set.insert(static_cast<uint8_t>(CommandCode::Thruster_GetBusVoltage));
            set.insert(static_cast<uint8_t>(CommandCode::Thruster_GetNoMaintainTime));
            set.insert(static_cast<uint8_t>(CommandCode::Thruster_GetTotalRunTime));
            set.insert(static_cast<uint8_t>(CommandCode::Thruster_SetSpeedWithTime));
          
            // 舵机命令
            set.insert(static_cast<uint8_t>(CommandCode::Rudder_SetAngle));
            set.insert(static_cast<uint8_t>(CommandCode::Rudder_GetRunTime));
            set.insert(static_cast<uint8_t>(CommandCode::Rudder_GetVoltage));
            set.insert(static_cast<uint8_t>(CommandCode::Rudder_GetAngle));
            set.insert(static_cast<uint8_t>(CommandCode::Rudder_GetCurrent));
            set.insert(static_cast<uint8_t>(CommandCode::Rudder_GetTemp));
            set.insert(static_cast<uint8_t>(CommandCode::Rudder_SetZero));
            // 浮调命令
            set.insert(static_cast<uint8_t>(CommandCode::Buoyancy_ReadValue));
            set.insert(static_cast<uint8_t>(CommandCode::Buoyancy_SetValue));
            set.insert(static_cast<uint8_t>(CommandCode::Buoyancy_Stop));

            // 抛载命令
            set.insert(static_cast<uint8_t>(CommandCode::DropWeight_Release));
            set.insert(static_cast<uint8_t>(CommandCode::DropWeight_Reset));

            // 北斗命令
            set.insert(static_cast<uint8_t>(CommandCode::BeidouComm_GetSignalQuality));
            set.insert(static_cast<uint8_t>(CommandCode::BeidouComm_GetIdentity));
            set.insert(static_cast<uint8_t>(CommandCode::BeidouComm_GetTarget));
            set.insert(static_cast<uint8_t>(CommandCode::BeidouComm_SetTarget));
            set.insert(static_cast<uint8_t>(CommandCode::BeidouComm_GetWhitelist));
            set.insert(static_cast<uint8_t>(CommandCode::BeidouComm_AddWhitelist));
            set.insert(static_cast<uint8_t>(CommandCode::BeidouComm_DelWhitelist));
            set.insert(static_cast<uint8_t>(CommandCode::BeidouComm_GetPosition));
            set.insert(static_cast<uint8_t>(CommandCode::BeidouComm_GetSysInfo));
            set.insert(static_cast<uint8_t>(CommandCode::BeidouComm_FactoryReset));
            set.insert(static_cast<uint8_t>(CommandCode::BeidouComm_Reboot));
            set.insert(static_cast<uint8_t>(CommandCode::BeidouComm_Test));
            set.insert(static_cast<uint8_t>(CommandCode::BeidouComm_GetConnectStatus));

            // 深度计命令
            set.insert(static_cast<uint8_t>(CommandCode::Depthgauge_GetDepth));

            // 高度计命令
            set.insert(static_cast<uint8_t>(CommandCode::Altimeter_GetHeight));

            // USBL命令
            set.insert(static_cast<uint8_t>(CommandCode::USBL_GetBeaconVersion));

            // 惯导命令
            set.insert(static_cast<uint8_t>(CommandCode::INS_SetGnssBind));
            set.insert(static_cast<uint8_t>(CommandCode::INS_CalibrationWithDVL));
            set.insert(static_cast<uint8_t>(CommandCode::INS_GetUtcDate));
            set.insert(static_cast<uint8_t>(CommandCode::INS_GetUtcTime));
            set.insert(static_cast<uint8_t>(CommandCode::INS_GetAttitude));
            set.insert(static_cast<uint8_t>(CommandCode::INS_GetBodySpeed));
            set.insert(static_cast<uint8_t>(CommandCode::INS_GetGeoSpeed));
            set.insert(static_cast<uint8_t>(CommandCode::INS_GetPosition));
            set.insert(static_cast<uint8_t>(CommandCode::INS_GetGyroscopeSpeed));
            set.insert(static_cast<uint8_t>(CommandCode::INS_GetAcceleration));
            set.insert(static_cast<uint8_t>(CommandCode::INS_GetStatus));
            set.insert(static_cast<uint8_t>(CommandCode::INS_CombinedWithDVL));
            set.insert(static_cast<uint8_t>(CommandCode::INS_CombinedAuto));

            // DVL命令
            set.insert(static_cast<uint8_t>(CommandCode::DVL_GetBottomSpeed));
            set.insert(static_cast<uint8_t>(CommandCode::DVL_GetWaterSpeed));

            // 摄像机命令
            set.insert(static_cast<uint8_t>(CommandCode::CameraLight_StartVideoSave));
            set.insert(static_cast<uint8_t>(CommandCode::CameraLight_StopVideoSave));
            set.insert(static_cast<uint8_t>(CommandCode::CameraLight_TakePhoto));

            // 电池命令
            set.insert(static_cast<uint8_t>(CommandCode::Battery_GetSoc));
            set.insert(static_cast<uint8_t>(CommandCode::Battery_GetTotalVoltage));
            set.insert(static_cast<uint8_t>(CommandCode::Battery_GetCurrent));
            set.insert(static_cast<uint8_t>(CommandCode::Battery_GetAvgTemp));
            set.insert(static_cast<uint8_t>(CommandCode::Battery_HVOn));
            set.insert(static_cast<uint8_t>(CommandCode::Battery_HVOff));
            set.insert(static_cast<uint8_t>(CommandCode::Battery_Lock));
            set.insert(static_cast<uint8_t>(CommandCode::Battery_Unlock));

            // 声呐命令
            set.insert(static_cast<uint8_t>(CommandCode::Sonar_SetParameter));

            // 水声通信命令
            set.insert(static_cast<uint8_t>(CommandCode::AcousticComm_SelfCheck));
            set.insert(static_cast<uint8_t>(CommandCode::AcousticComm_Test));

            // 故障查询命令
            set.insert(static_cast<uint8_t>(CommandCode::Common_GetFault));

            return set;
        }();
        return codes;
    }

    // 基于器件所属子系统进行判断
    static bool isThruster(DeviceId id) {
        return getDeviceSubsystem(id) == Subsystem::Propulsion;
    }

    static bool isRudder(DeviceId id) {
        return getDeviceSubsystem(id) == Subsystem::Steering;
    }

    static bool isBattery(DeviceId id) {
        return id == DeviceId::PowerBattery1 || id == DeviceId::PowerBattery2 || id == DeviceId::MeterBattery;
    }

    static bool isBuoyancy(DeviceId id) {
        return getDeviceSubsystem(id) == Subsystem::Buoyancy;
    }

    static bool isSonar(DeviceId id) {
        if (getDeviceSubsystem(id) != Subsystem::DataAcquisition) return false;
        return id == DeviceId::SideScanSonar || id == DeviceId::ForwardSonar || id == DeviceId::DownwardSonar;
    }

    static bool isValidDeviceId(uint8_t deviceIdInt) {
        return getValidDeviceIds().count(deviceIdInt) > 0;
    }

    static bool isValidCommandCode(uint8_t cmdCodeInt) {
        return getValidCommandCodes().count(cmdCodeInt) > 0;
    }

private:
    // 获取器件所属子系统
    static Subsystem getDeviceSubsystem(DeviceId id) {
        for (const auto& entry : DeviceData::deviceInfos) {
            if (entry.first == id) {
                return entry.second.subsystem;
            }
        }
        return static_cast<Subsystem>(0);
    }
};



// EnumUtils特化实现
template<>
inline const std::unordered_set<uint8_t>& EnumUtils<DeviceId>::getValidValues() {
    static const std::unordered_set<uint8_t> values = []() {
        std::unordered_set<uint8_t> set;
        for (const auto& entry : DeviceData::deviceInfos) {
            set.insert(static_cast<uint8_t>(entry.first));
        }
        return set;
    }();
    return values;
}

template<>
inline const std::unordered_set<uint8_t>& EnumUtils<CommandCode>::getValidValues() {
    static const std::unordered_set<uint8_t> values = DeviceTypeHelper::getValidCommandCodes();
    return values;
}

