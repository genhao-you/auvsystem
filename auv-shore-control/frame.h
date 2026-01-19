#pragma once
#include <QDateTime>
#include <QByteArray>
#include "selfcheckenum.h"
#include "frameglobal.h"
#include<QJsonObject>
/**
 * @brief 数据帧结构封装
 *
 * 封装通信协议中的完整数据帧，提供结构化访问方式
 */
class Frame {
public:
    // 帧头（固定为"@@@"）
    QByteArray header = FrameConstants::FRAME_HEADER;

    // 数据长度：除帧头、帧尾外的字节数
    uint16_t dataLength = 0;

    // 时间戳：年月日时分秒（12字节）
    QDateTime timestamp;

    // 包序号：1-256循环
    uint8_t packetSeq = 0;

    // 通信信道：0表示不使用J方
    uint8_t channel = 0x00;

    // 发送设备：0x0001为岸基，0x0002为AUV
    uint16_t senderDevice = 0x0000;


    // 接收设备：0x0001为岸基，0x0002为AUV
    uint16_t receiverDevice = 0x0000;


    // 使用阶段：标识通信协议类型
    WorkPhase phase = WorkPhase::DeviceTest;

    // 数据体：配置的参数或传感器的输出数据
    QByteArray dataBody;

    // 校验位：CRC8校验结果
    uint8_t checksum = 0x00;

    // 帧尾（固定为"$$$"）
    QByteArray footer = FrameConstants::FRAME_FOOTER;

public:
    QJsonObject toJson() const;
    static Frame fromJson(const QJsonObject& json);
};
