#pragma once
#include<QString>
#include<qDebug>
// 1. 惯导工作状态（0-2位）
enum class INSWorkState : uint8_t {
    Ready = 0x00,        // 准备（0）
    Alignment = 0x02,    // 对准（2）
    Navigation = 0x04,   // 导航（4）
    Invalid = 0xFF       // 无效
};

// 2. 惯导组合状态（3-7位，低5位）
enum class INSCombinationState : uint8_t {
    PureINS = 0x00,          // 00000 纯惯导（0）
    GnssCombined = 0x01,     // 00001 卫导组合（1）
    DVLBottomCombined = 0x04,// 00100 DVL对底组合（4）
    GnssDVLCombined = 0x05,  // 00101 卫导+DVL（5）
    Invalid = 0xFF
};

// 3. 惯导自检状态（8-9位，低2位）
enum class INSSelfCheckState : uint8_t {
    Initial = 0x00,    // 初始（0）
    Checking = 0x01,   // 自检中（1）
    Success = 0x02,    // 自检成功（2）
    Fail = 0x03,       // 自检失败（3）
    Invalid = 0xFF
};

// 结构化存储解析结果
struct INS16StatusData {
    INSWorkState workState = INSWorkState::Invalid;
    INSCombinationState combinationState = INSCombinationState::Invalid;
    INSSelfCheckState selfCheckState = INSSelfCheckState::Invalid;
    uint16_t rawValue = 0; // 原始uint16值（调试用）

    bool isValid() const {
        return workState != INSWorkState::Invalid
            && combinationState != INSCombinationState::Invalid
            && selfCheckState != INSSelfCheckState::Invalid;
    }
};
/**
 * @brief 解析惯导返回的uint16状态码（大端序）
 * @param payload 2字节的原始数据（uint16大端序）
 * @return 结构化状态数据
 */
INS16StatusData parseINS16Status(const QByteArray& payload) {
    INS16StatusData result;
    if (payload.size() != 2) { // uint16固定2字节
        qWarning() << "惯导状态码长度错误（需2字节），实际：" << payload.size();
        return result;
    }

    // 1. 把2字节payload转为uint16（大端序，适配设备通信）
    uint16_t statusVal = qFromBigEndian<uint16_t>(reinterpret_cast<const uchar*>(payload.data()));
    result.rawValue = statusVal;

    // 2. 拆分0-2位：工作状态
    uint8_t workVal = statusVal & 0x0007; // 掩码保留低3位
    switch (workVal) {
    case 0x00: result.workState = INSWorkState::Ready; break;
    case 0x02: result.workState = INSWorkState::Alignment; break;
    case 0x04: result.workState = INSWorkState::Navigation; break;
    default: result.workState = INSWorkState::Invalid;
    }

    // 3. 拆分3-7位：组合状态（右移3位后取低5位）
    uint8_t comboVal = (statusVal >> 3) & 0x001F;
    switch (comboVal) {
    case 0x00: result.combinationState = INSCombinationState::PureINS; break;
    case 0x08: result.combinationState = INSCombinationState::GnssCombined; break;
    case 0x01: result.combinationState = INSCombinationState::DVLBottomCombined; break;
    case 0x09: result.combinationState = INSCombinationState::GnssDVLCombined; break;
    default: result.combinationState = INSCombinationState::Invalid;
    }

    // 4. 拆分8-9位：自检状态（右移8位后取低2位）
    uint8_t checkVal = (statusVal >> 8) & 0x0003;
    switch (checkVal) {
    case 0x00: result.selfCheckState = INSSelfCheckState::Initial; break;
    case 0x01: result.selfCheckState = INSSelfCheckState::Checking; break;
    case 0x02: result.selfCheckState = INSSelfCheckState::Success; break;
    case 0x03: result.selfCheckState = INSSelfCheckState::Fail; break;
    default: result.selfCheckState = INSSelfCheckState::Invalid;
    }

    return result;
}

// 状态转字符串（用于UI展示）
QString insWorkStateToString(INSWorkState state) {
    switch (state) {
    case INSWorkState::Ready: return "准备";
    case INSWorkState::Alignment: return "对准";
    case INSWorkState::Navigation: return "导航";
    default: return "无效";
    }
}

QString insCombinationStateToString(INSCombinationState state) {
    switch (state) {
    case INSCombinationState::PureINS: return "纯惯导（00000）";
    case INSCombinationState::GnssCombined: return "卫导组合（01000）";
    case INSCombinationState::DVLBottomCombined: return "DVL对底组合（00001）";
    case INSCombinationState::GnssDVLCombined: return "卫导+DVL组合（01001）";
    default: return "无效";
    }
}

QString insSelfCheckStateToString(INSSelfCheckState state) {
    switch (state) {
    case INSSelfCheckState::Initial: return "初始（0）";
    case INSSelfCheckState::Checking: return "自检中（1）";
    case INSSelfCheckState::Success: return "自检成功（2）";
    case INSSelfCheckState::Fail: return "自检失败（3）";
    default: return "无效";
    }
}