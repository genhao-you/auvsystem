#pragma once
#include <cstdint>

// ========== 全项目通用枚举（所有模块都可能用到） ==========
/**
 * @brief 通用结果状态
 */
enum class ResultStatus : uint8_t {
    Success = 0x00,  // 成功
    Pending = 0x01,  // 等待中
    Failed = 0x02,   // 失败
    Timeout = 0x03,  // 超时
    Unknown = 0xFF   // 未知
};

// ========== 全项目通用常量 ==========
constexpr int16_t COMMON_PARAM_SCALE = 1000;  // 通用参数缩放系数

// 使用阶段
enum class WorkPhase : uint8_t {
    ConnectivityTest = 0x01,    // 连通性测试
    DeviceTest = 0x02,          // 器件测试（自检模块主要使用）
    VirtualWorkflow = 0x03,     // 虚拟工作流程测试
    TaskCommand = 0x04,         // 任务指令
    ControlCommand = 0x05,      // 控制参数指令
    MissionCommand = 0x06       // 使命指令
};