#pragma once
#include "frame.h"
#include "frameglobal.h"
#include"wateracousticchannel.h"
#include <QByteArray>

/**
 * @brief 帧解析器
 *
 * 负责将原始字节流解析为Frame对象，
 * 并验证帧的有效性（帧头、帧尾、CRC校验）
 */
class FrameParser {
public:
    /**
     * @brief 解析字节流为帧对象
     * @param data 原始字节流
     * @param frame 输出的帧对象
     * @param error 错误信息（输出）
     * @return 解析是否成功
     */
    static bool parseFrame(const QByteArray& data, Frame& frame, CheckError& error);
    // 解析数据体为：器件ID + 命令码 + 参数
    static bool parseDataBody(const QByteArray& dataBody,
        DeviceId& deviceId,
        CommandCode& cmdCode,
        QByteArray& param);
 // 从字节数组解析时间戳
    static QDateTime parseTimestamp(const QByteArray& timestampBytes);
    // 从字节数组解析16位无符号整数（大端模式）
    static uint16_t bytesToUint16(const QByteArray& bytes, int offset = 0);

    
    // 水声帧解析（映射到原有Frame结构）
    static bool parseWaterAcousticFrame(const QByteArray& frameData, Frame& frame, CheckError& error);
   
    // 识别水声上行指令
    static WaterAcousticCmd parseWaterAcousticCmd(const Frame& frame);

    // 解析水声帧内的通用核心数据（无通用帧头/尾）
    static bool parseWaterAcousticCoreData(const QByteArray& frameData, 
        const QByteArray& coreData, Frame& frame, CheckError& error, WaterAcousticUplinkCmd cmd = WaterAcousticUplinkCmd::Unknown);
};

