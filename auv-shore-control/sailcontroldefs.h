#pragma once
#include <cstdint>
#include <QByteArray>
#include <QString>

/**
 * @brief 航行控制命令码
 */
enum class SailCommandCode : uint8_t {
    Sail_Start = 0xAA,       // 开始航行 (0xAA)
    Sail_Stop = 0xBB,       // 停止航行 (0xBB)
    Sail_Unknown = 0x00      // 未知命令
};

// 航行控制常量
constexpr int SAIL_CMD_TIMEOUT_MS = 3000;      // 命令超时时间
constexpr uint8_t SAIL_DEFAULT_SEQ = 0x01;     // 默认包序号

/**
 * @brief 航行控制参数（只有命令码）
 */
struct SailControlParam {
    SailCommandCode cmdCode;

    explicit SailControlParam(SailCommandCode code) : cmdCode(code) {}

    QByteArray toBytes() const {
        QByteArray bytes;
        bytes.append(static_cast<char>(static_cast<uint8_t>(cmdCode)));
        return bytes;
    }

    static SailControlParam fromBytes(const QByteArray& bytes) {
        if (bytes.isEmpty()) {
            return SailControlParam(SailCommandCode::Sail_Unknown);
        }
        return SailControlParam(static_cast<SailCommandCode>(static_cast<uint8_t>(bytes[0])));
    }

    QString toString() const {
        switch (cmdCode) {
        case SailCommandCode::Sail_Start: return "开始航行";
        case SailCommandCode::Sail_Stop: return "停止航行";
        default: return "未知命令";
        }
    }
};