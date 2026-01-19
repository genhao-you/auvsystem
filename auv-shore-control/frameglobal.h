#pragma once
#include <cstdint>
#include <QByteArray>
// 通用帧字段在水声帧content中的固定偏移
#define GF_DATA_LEN_OFFSET     0   // 数据长度偏移（2字节）
#define GF_TIMESTAMP_OFFSET    2   // 时间戳偏移（6字节）
#define GF_PACKET_SEQ_OFFSET   8   // 包序号偏移（1字节）
#define GF_CHANNEL_OFFSET      9   // 通信信道偏移（1字节）
#define GF_SENDER_DEV_OFFSET   10  // 发送设备偏移（2字节）
#define GF_RECEIVER_DEV_OFFSET 12  // 接收设备偏移（2字节）
#define GF_PHASE_OFFSET        14  // 使用阶段偏移（1字节）
#define GF_DATA_BODY_OFFSET    15  // 数据体起始偏移
// 通用帧字段最小长度（到数据体起始前）
#define GF_MIN_FIELD_LEN       15
// 全局常量定义
namespace FrameConstants {
    // 帧格式常量
    constexpr char FRAME_HEADER[] = "@@@";          // 帧头
    constexpr char FRAME_FOOTER[] = "$$$";          // 帧尾
    constexpr int FRAME_HEADER_LENGTH = 3;          // 帧头长度
    constexpr int FRAME_FOOTER_LENGTH = 3;          // 帧尾长度
    constexpr int TIMESTAMP_LENGTH = 6;            // 时间戳长度(yyMMddhhmmss)
    constexpr int MAX_PACKET_SEQ = 256;             // 最大包序号
    constexpr int POWER_ON_TIMEOUT_MS = 20000;    // 上电命令超时（20秒）
    constexpr int POWER_OFF_TIMEOUT_MS = 10000;   // 下电命令超时（10秒）
    constexpr int DEFAULT_TIMEOUT_MS = 5000;      // 默认超时（获取/设置值等，3秒）
    constexpr int MAX_RETRY_COUNT = 3;              // 最大重试次数

    // 设备ID常量
    constexpr uint16_t SHORE_STATION_DEVICE_ID = 0x0001;  // 岸基显控软件
    constexpr uint16_t AUV_DEVICE_ID = 0x0002;            // AUV设备

      // 水声通信帧常量
    constexpr uint8_t WATER_ACOUSTIC_HEADER[] = { 0xA5, 0xA5, 0xA5, 0xD5, 0x01 }; // 水声帧头
    constexpr uint8_t WATER_ACOUSTIC_FOOTER[] = { 0xA5, 0xD4 };                  // 水声帧尾
    constexpr int WATER_ACOUSTIC_HEADER_LENGTH = 5;                            // 水声帧头长度
    constexpr int WATER_ACOUSTIC_FOOTER_LENGTH = 2;                            // 水声帧尾长度
    constexpr int WATER_ACOUSTIC_MAX_CONTENT_LEN = 114;                         // 水声报文内容最大长度
    constexpr int WATER_ACOUSTIC_SEND_INTERVAL_MS = 3000;                      // 水声发送间隔≥3s
    constexpr int WATER_ACOUSTIC_LEN_FIELD_LENGTH = 2; 
    
    // 水声报文长度字段（2字节大端）
}
// 水声上行指令枚举（复用原有错误码风格）
enum class WaterAcousticCmd {
    Unknown = 0,
    RecvConfirm = 1,   // 通信机收到数据，准备发送（00 00）
    SendFinish = 2,    // 通信机发送完毕（00 01）
    RecvNotify = 3,    // 通信机收到新数据（02 02 00）
    DataReceived = 4   // 通信机转发的有效数据
};
// 错误码定义
enum class ErrorCode {
    Success = 0,               // 成功
    FrameInvalid = 1001,       // 帧格式错误
    CrcMismatch = 1002,        // CRC校验失败
    DeviceNotFound = 2001,     // 器件不存在
    CommandNotSupported = 2002,// 器件不支持该命令
    CommandTimeout = 2003,     // 命令超时
    ParameterInvalid = 2004,    // 参数无效
    FileWriteFailed = 3001,
    FileNotExists = 3002,
    FileOpenFailed = 3003,
    FileEmpty = 3004
};

// 错误信息类
class CheckError {
public:
    CheckError(ErrorCode code = ErrorCode::Success, const QString& desc = "")
        : m_errorCode(code), m_description(desc) {}

    bool isSuccess() const { return m_errorCode == ErrorCode::Success; }
    ErrorCode code() const { return m_errorCode; }
    QString description() const { return m_description; }
    static CheckError fromWaterAcousticError(const QString& desc) {
        return CheckError(ErrorCode::FrameInvalid, "[水声通信] " + desc);
    }
private:
    ErrorCode m_errorCode;
    QString m_description;
};

// 控制参数错误类（对齐CheckError，无命令码相关）
class ControlParamError {
public:
    ControlParamError(ErrorCode code = ErrorCode::Success, const QString& desc = "")
        : m_errorCode(code), m_description(desc) {}

    bool isSuccess() const { return m_errorCode == ErrorCode::Success; }
    ErrorCode code() const { return m_errorCode; }
    QString description() const { return m_description; }

private:
    ErrorCode m_errorCode;
    QString m_description;
};