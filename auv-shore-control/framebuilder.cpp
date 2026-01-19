#include "framebuilder.h"
#include "crccalculator.h"
#include "frameglobal.h"
#include <QDateTime>
#include<qDebug>

// 构建通用帧
QByteArray FrameBuilder::buildFrame(const Frame& frame) {
    // 定义固定字段长度
    const int FIXED_FIELDS_LEN =
        FrameConstants::TIMESTAMP_LENGTH + //时间戳(6字节)
        1 +  // 包序号
        1 +  // 通信信道
        2 +  // 发送设备
        2 +  // 接收设备
        1 +  // 使用阶段
        1;  // 校验位
     
    //计算数据长度（除帧头、帧尾外的字节数)
    //数据长度 = 固定字段长度+数据体长度+数据长度自身(2字节)
    uint16_t dataLength = FIXED_FIELDS_LEN + frame.dataBody.size() + 2;

    //预分配内存，按顺序拼接字段
    QByteArray packet;
    packet.reserve(
        FrameConstants::FRAME_HEADER_LENGTH + //帧头(3)
        dataLength +  //数据长度
        FrameConstants::FRAME_FOOTER_LENGTH //帧尾(3)
    );

    // 帧头
    packet.append(frame.header);

    //数据长度
    packet.append(uint16ToBytes(dataLength));

    // 时间戳
    QByteArray timestamp = formatTimestamp(frame.timestamp);
    packet.append(timestamp);

    // 包序号
    packet.append(static_cast<char>(frame.packetSeq));

    // 通信信道
    packet.append(static_cast<char>(frame.channel));

    // 发送设备
    packet.append(uint16ToBytes(frame.senderDevice));

    // 接收设备
    packet.append(uint16ToBytes(frame.receiverDevice));

    // 使用阶段
    packet.append(static_cast<char>(frame.phase));

   
    // 数据体 器件ID + 命令码 + 参数
    packet.append(frame.dataBody);
    qDebug() << "bulider crc数据源：" << packet.toHex();
    // 计算校验位（帧头到数据体的所有数据）
    uint8_t crc = CrcCalculator::crc8(packet);
    packet.append(static_cast<char>(crc));

    // 帧尾
    packet.append(frame.footer);

    return packet;
}

// 构建自检命令帧
QByteArray FrameBuilder::buildCheckCommandFrame(DeviceId deviceId,
    CommandCode cmdCode, WorkPhase parse,
    const QByteArray& param,
    uint8_t packetSeq) {
    Frame frame;

    // 填充帧基本信息
    frame.timestamp = QDateTime::currentDateTime();
    frame.packetSeq = packetSeq;
    frame.senderDevice = FrameConstants::SHORE_STATION_DEVICE_ID;
    frame.receiverDevice = FrameConstants::AUV_DEVICE_ID;
    frame.phase = parse;
    // 构建数据体：命令编码(2字节) + 参数
    QByteArray dataBody;
    // 器件ID（1字节）
    dataBody.append(static_cast<char>(static_cast<uint8_t>(deviceId)));
    // 命令码（1字节）
    dataBody.append(static_cast<uint8_t>(cmdCode));
    // 参数（变长）
    dataBody.append(param);
    frame.dataBody = dataBody;

    // 构建完整帧
    return buildFrame(frame);
}

QByteArray FrameBuilder::buildWaterAcousticFrame(const Frame& frame)
{
    // 1. 先构建完整通用帧，用于提取核心数据（去头尾）
    QByteArray fullCommonFrame = buildFrame(frame);
    qDebug() << "完整通用帧（含通用帧头/尾）：" << fullCommonFrame.toHex(' ');

    // 2. 剥离通用帧头（3字节）和尾（3字节），获取核心数据（原通用帧去头尾）
    QByteArray commonFrameCore;
    if (fullCommonFrame.size() > FrameConstants::FRAME_HEADER_LENGTH + FrameConstants::FRAME_FOOTER_LENGTH) {
        commonFrameCore = fullCommonFrame.mid(
            FrameConstants::FRAME_HEADER_LENGTH,
            fullCommonFrame.size() - FrameConstants::FRAME_HEADER_LENGTH - FrameConstants::FRAME_FOOTER_LENGTH
        );
        qDebug() << "剥离通用帧头/尾后核心数据（含原校验位）：" << commonFrameCore.toHex(' ');

        if (commonFrameCore.size() >= 2) {
            // 2.1 提取通用帧的dataLength（前2字节，大端）
            quint16 bigEndianValue; // 定义quint16变量存储大端数据
            // 步骤1：从QByteArray前2字节拷贝到quint16变量
            memcpy(&bigEndianValue, commonFrameCore.constData(), sizeof(quint16));
            // 步骤2：大端字节序 → 主机字节序（解析出真实数值）
            uint16_t commonDataLen = qFromBigEndian(bigEndianValue);

            // 2.2 计算水声协议的真实长度：通用帧dataLength - 2（去掉自身2字节）
            uint16_t waDataLen = commonDataLen - 2;
            // 2.3 替换核心数据开头的2字节为水声真实长度
            QByteArray waDataLenBytes = uint16ToBytes(waDataLen);
            commonFrameCore.replace(0, 2, waDataLenBytes);
            //qDebug() << "修正长度字段：通用帧长度(" << commonDataLen << ") → 水声长度(" << waDataLen << ")，字节：" << waDataLenBytes.toHex();
        }
        // 3. 移除原通用帧的校验位（最后1字节），保留修正长度后的核心数据
        if (!commonFrameCore.isEmpty()) {
            // 原核心数据最后1字节是基于通用帧头的校验位，无效，需剔除
            commonFrameCore.chop(1);
            //qDebug() << "剔除原无效校验位后核心数据：" << commonFrameCore.toHex(' ');
        }
    }
    else {
        qWarning() << "通用帧长度不足，无法剥离头尾：" << fullCommonFrame.size() << "字节";
        commonFrameCore = QByteArray();
    }

    // 4. 截断核心数据（最大114字节，符合水声协议）
    QByteArray content = truncateWaterAcousticContent(commonFrameCore);
    //qDebug() << "截断后水声帧核心数据：" << content.toHex(' ');

    // 5. 构建水声帧头字节数组（关键修改：单独提取水声帧头，用于CRC计算）
    QByteArray waHeader(reinterpret_cast<const char*>(FrameConstants::WATER_ACOUSTIC_HEADER),
        FrameConstants::WATER_ACOUSTIC_HEADER_LENGTH);
   

    // 6. 计算校验位（关键修改：数据源 = 水声帧头 + 截断后的核心数据）
    QByteArray crcSource = waHeader + content; // 校验范围包含水声帧头
    uint8_t waCrc = CrcCalculator::crc8(crcSource);
    qDebug() << "CRC计算数据源（水声帧头+核心数据）：" << crcSource.toHex(' ');
    qDebug() << "计算出的水声帧校验位：" << QString::number(waCrc, 16).toUpper();

    // 7. 构建最终水声帧（水声帧头 + 核心数据 + 新校验位 + 水声帧尾）
    QByteArray waFrame;
    waFrame.reserve(FrameConstants::WATER_ACOUSTIC_HEADER_LENGTH +
        content.size() +
        1 + // 新增的校验位
        FrameConstants::WATER_ACOUSTIC_FOOTER_LENGTH);

    // 7.1 水声帧头（参与CRC计算）
    waFrame.append(waHeader);
    // 7.2 核心数据
    waFrame.append(content);
    // 7.3 新校验位（基于水声帧头+核心数据计算）
    waFrame.append(static_cast<char>(waCrc));
    // 7.4 水声帧尾（不参与CRC计算）
    waFrame.append(reinterpret_cast<const char*>(FrameConstants::WATER_ACOUSTIC_FOOTER),
        FrameConstants::WATER_ACOUSTIC_FOOTER_LENGTH);

    qDebug() << "最终水声帧（水声帧头+核心数据+新校验位+水声帧尾）：" << waFrame.toHex(' ');
    return waFrame;
}

QByteArray FrameBuilder::buildWaterAcousticCheckCommandFrame(DeviceId deviceId, CommandCode cmdCode, const QByteArray& param,uint8_t packetSeq)
{
    Frame frame;

    // 填充帧基本信息
    frame.timestamp = QDateTime::currentDateTime();
    frame.packetSeq = packetSeq;
    frame.senderDevice = FrameConstants::SHORE_STATION_DEVICE_ID;
    frame.receiverDevice = FrameConstants::AUV_DEVICE_ID;
    frame.phase = WorkPhase::DeviceTest;
    frame.header = QByteArray(reinterpret_cast<const char*>(FrameConstants::FRAME_HEADER), FrameConstants::FRAME_HEADER_LENGTH);
    frame.footer = QByteArray(reinterpret_cast<const char*>(FrameConstants::FRAME_FOOTER), FrameConstants::FRAME_FOOTER_LENGTH);
    // 构建数据体：命令编码(2字节) + 参数
    QByteArray dataBody;
    // 器件ID（1字节）
    dataBody.append(static_cast<char>(static_cast<uint8_t>(deviceId)));
    // 命令码（1字节）
    dataBody.append(static_cast<uint8_t>(cmdCode));
    // 参数（变长）
    dataBody.append(param);
    frame.dataBody = dataBody;

    // 3. 构建水声帧
    return buildWaterAcousticFrame(frame);
}

QByteArray FrameBuilder::buildControlParamFrame(ControlType controlType, const QByteArray& param, uint8_t packetSeq, WorkPhase phase)
{
    Frame frame;
    // 填充帧基础信息（和自检帧一致，仅数据体不同）
    frame.timestamp = QDateTime::currentDateTime();
    frame.packetSeq = packetSeq;
    frame.senderDevice = FrameConstants::SHORE_STATION_DEVICE_ID;
    frame.receiverDevice = FrameConstants::AUV_DEVICE_ID;
    frame.phase = phase;
    frame.header = QByteArray(reinterpret_cast<const char*>(FrameConstants::FRAME_HEADER), FrameConstants::FRAME_HEADER_LENGTH);
    frame.footer = QByteArray(reinterpret_cast<const char*>(FrameConstants::FRAME_FOOTER), FrameConstants::FRAME_FOOTER_LENGTH);

    // 核心修改：数据体 = 控制类型(1字节) + 控制参数（无器件ID、无命令码）
    QByteArray dataBody;
    dataBody.append(static_cast<char>(static_cast<uint8_t>(controlType))); // 控制类型
    dataBody.append(param);                                              // 控制参数
    frame.dataBody = dataBody;

    // 调用通用帧构建逻辑（复用原有CRC/长度计算）
    return buildFrame(frame);
}

QByteArray FrameBuilder::buildSailControlFrame(uint8_t sailCmdCode, uint8_t packetSeq, WorkPhase phase)
{
    Frame frame;

    // 填充帧基本信息
    frame.timestamp = QDateTime::currentDateTime();
    frame.packetSeq = packetSeq;
    frame.senderDevice = FrameConstants::SHORE_STATION_DEVICE_ID;
    frame.receiverDevice = FrameConstants::AUV_DEVICE_ID;
    frame.phase = phase;

    // 航行控制数据体：只有命令码（1字节）
    QByteArray dataBody;
    dataBody.append(static_cast<char>(sailCmdCode));
    frame.dataBody = dataBody;

    qDebug() << "构建航行控制帧，命令码：0x" << QString::number(sailCmdCode, 16).toUpper()
        << "，数据体：" << dataBody.toHex();

    // 构建完整通用帧
    return buildFrame(frame);
}

// 格式化时间戳
QByteArray FrameBuilder::formatTimestamp(const QDateTime& timestamp) {
    // 确保使用UTC时间
    QDateTime utcTime = timestamp.toUTC();
    QDate date = utcTime.date();
    QTime time = utcTime.time();

    QByteArray timestampBytes;
    // 年（取后两位，如2024→24，uint8）
    timestampBytes.append(static_cast<char>(static_cast<uint8_t>(date.year() % 100)));
    // 月（1~12，uint8）
    timestampBytes.append(static_cast<char>(static_cast<uint8_t>(date.month())));
    // 日（1~31，uint8）
    timestampBytes.append(static_cast<char>(static_cast<uint8_t>(date.day())));
    // 时（0~23，uint8）
    timestampBytes.append(static_cast<char>(static_cast<uint8_t>(time.hour())));
    // 分（0~59，uint8）
    timestampBytes.append(static_cast<char>(static_cast<uint8_t>(time.minute())));
    // 秒（0~59，uint8）
    timestampBytes.append(static_cast<char>(static_cast<uint8_t>(time.second())));

    // 确保长度为6字节（防止异常情况）
    return timestampBytes.left(FrameConstants::TIMESTAMP_LENGTH);
}

// 转换uint16为大端字节数组
QByteArray FrameBuilder::uint16ToBytes(uint16_t value) {
    QByteArray bytes;
    bytes.append(static_cast<char>((value >> 8) & 0xFF));  // 高8位
    bytes.append(static_cast<char>(value & 0xFF));         // 低8位
    return bytes;
}
//截断水声报文内容（最大16字节）
QByteArray FrameBuilder::truncateWaterAcousticContent(const QByteArray& content)
{
    if (content.size() > FrameConstants::WATER_ACOUSTIC_MAX_CONTENT_LEN) {
        qWarning() << "水声报文内容过长（" << content.size() << "字节），已截断为16字节";
        return content.left(FrameConstants::WATER_ACOUSTIC_MAX_CONTENT_LEN);
    }
    return content;
}
