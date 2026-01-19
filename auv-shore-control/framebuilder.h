#pragma once

#include "frame.h"
#include "selfcheckenum.h"
#include "controlparamdefs.h"
#include <QByteArray>
#include <QtEndian>
/**
 * @brief 帧构建器
 *
 * 负责将Frame对象转换为符合协议的字节流，
 * 采用工厂模式设计，提供多种帧的构建方法
 */
class FrameBuilder {
public:
    /**
     * @brief 构建通用帧
     * @param frame 帧结构对象
     * @return 构建好的字节流
     */
    static QByteArray buildFrame(const Frame& frame);

    /**
     * @brief 构建自检命令帧（常用快捷方法）
     * @param deviceId 目标器件ID
     * @param cmdCode 命令编码
     * @param param 命令参数
     * @param packetSeq 包序号
     * @return 构建好的字节流
     */
    static QByteArray buildCheckCommandFrame(DeviceId deviceId,
        CommandCode cmdCode, WorkPhase parse,
        const QByteArray& param,
        uint8_t packetSeq);
    /**
  * @brief 构建通用水声帧（A5A5A5D501帧头 + A5D4帧尾）
  * @param frame 帧结构对象（仅使用dataBody作为报文内容）
  * @return 符合水声协议的字节流
  */
    static QByteArray buildWaterAcousticFrame(const Frame& frame);

    /**
     * @brief 构建水声自检命令帧（快捷方法）
     * @param deviceId 目标器件ID
     * @param cmdCode 命令编码
     * @param param 命令参数
     * @return 符合水声协议的自检命令帧
     */
    static QByteArray buildWaterAcousticCheckCommandFrame(DeviceId deviceId,
        CommandCode cmdCode,
        const QByteArray& param, uint8_t packetSeq);

    /**
 * @brief 构建控制参数通用帧（无器件ID，数据体=控制类型+参数）
 * @param controlType 控制类型（深度/高度/航向）
 * @param param 控制参数字节流
 * @param packetSeq 包序号
 * @param phase 工作阶段
 * @return 构建好的通用帧字节流
 */
    static QByteArray buildControlParamFrame(ControlType controlType,
        const QByteArray& param,
        uint8_t packetSeq,
        WorkPhase phase = WorkPhase::ControlCommand);

    /**
 * @brief 构建航行控制帧（通用帧格式）
 * @param sailCmdCode 航行控制命令码（0xAA/0xBB）
 * @param packetSeq 包序号
 * @param phase 工作阶段
 * @return 构建好的通用帧字节流
 */
    static QByteArray buildSailControlFrame(uint8_t sailCmdCode,
        uint8_t packetSeq,
        WorkPhase phase = WorkPhase::TaskCommand);
private:
    // 转换时间戳为协议要求的6字节格式（yyMMddhhmmss）
    static QByteArray formatTimestamp(const QDateTime& timestamp);

    // 将16位无符号整数转换为大端模式的字节数组
    static QByteArray uint16ToBytes(uint16_t value);
    // ：截断水声报文内容（最大16字节）
    static QByteArray truncateWaterAcousticContent(const QByteArray& content);
    static const int FIXED_FIELDS_LEN;
};
