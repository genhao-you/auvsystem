#include "frameparser.h"
#include "crccalculator.h"
#include "frameglobal.h"
#include <QDateTime>
#include <QString>
#include<qDebug>
// 解析字节流为帧对象
bool FrameParser::parseFrame(const QByteArray& data, Frame& frame, CheckError& error) {
    const int headerLen = FrameConstants::FRAME_HEADER_LENGTH;
    const int footerLen = FrameConstants::FRAME_FOOTER_LENGTH;
    const int minFrameLen = headerLen + footerLen + 2 + 6 + (1 + 1 + 2 + 2 + 1) + 1; // 最小长度22

    // 1. 基础长度校验
    if (data.size() < minFrameLen) {
        error = CheckError(ErrorCode::FrameInvalid, "帧长度过短(小于最小长度)");
        return false;
    }

    // 2. 帧头校验（必须是前3字节）
    if (data.left(headerLen) != FrameConstants::FRAME_HEADER) {
        error = CheckError(ErrorCode::FrameInvalid, "帧头错误");
        return false;
    }

    // 3. 帧尾校验（必须是最后3字节，修复仅用endsWith的缺陷）
    if (data.right(footerLen) != FrameConstants::FRAME_FOOTER) {
        error = CheckError(ErrorCode::FrameInvalid, "帧尾错误（非最后3字节）");
        return false;
    }

    // 4. 提取帧内容（帧头后、帧尾前的所有字节）
    QByteArray content = data.mid(headerLen, data.size() - headerLen - footerLen);
    int offset = 0;

    // 5. 解析数据长度（2字节大端）
    if (content.size() < offset + 2) {
        error = CheckError(ErrorCode::FrameInvalid, "数据长度字段不完整");
        return false;
    }
    frame.dataLength = bytesToUint16(content, offset);
    offset += 2;

    // 6. 修复数据长度校验逻辑：允许微小冗余（兼容数据末尾1字节冗余）
    // 核心：数据长度字段应 ≤ content.size()（允许冗余，避免少量字节异常）
    if (frame.dataLength > content.size() || frame.dataLength < minFrameLen - headerLen - footerLen) {
        error = CheckError(ErrorCode::FrameInvalid,
            QString("数据长度不匹配（记录：%1, 实际：%2）").arg(frame.dataLength).arg(content.size()));
        return false;
    }
    // 截断content到数据长度字段指定的长度（丢弃冗余字节）
    content = content.left(frame.dataLength);

    // 7. 解析时间戳（6字节）
    if (content.size() < offset + FrameConstants::TIMESTAMP_LENGTH) {
        error = CheckError(ErrorCode::FrameInvalid,
            QString("时间戳不完整(需要%1字节)").arg(FrameConstants::TIMESTAMP_LENGTH));
        return false;
    }
    QByteArray timestampBytes = content.mid(offset, FrameConstants::TIMESTAMP_LENGTH);
    frame.timestamp = parseTimestamp(timestampBytes);
    offset += FrameConstants::TIMESTAMP_LENGTH;

    // 8. 解析包序号（1字节）
    if (content.size() < offset + 1) {
        error = CheckError(ErrorCode::FrameInvalid, "包序号字段不完整");
        return false;
    }
    frame.packetSeq = static_cast<uint8_t>(content[offset++]);

    // 9. 解析通信信道（1字节）
    if (content.size() < offset + 1) {
        error = CheckError(ErrorCode::FrameInvalid, "通信信道字段不完整");
        return false;
    }
    frame.channel = static_cast<uint8_t>(content[offset++]);

    // 10. 解析发送设备（2字节）
    if (content.size() < offset + 2) {
        error = CheckError(ErrorCode::FrameInvalid, "发送设备字段不完整");
        return false;
    }
    frame.senderDevice = bytesToUint16(content, offset);
    offset += 2;

    // 11. 解析接收设备（2字节）
    if (content.size() < offset + 2) {
        error = CheckError(ErrorCode::FrameInvalid, "接收设备字段不完整");
        return false;
    }
    frame.receiverDevice = bytesToUint16(content, offset);
    offset += 2;

    // 12. 解析使用阶段（1字节）
    if (content.size() < offset + 1) {
        error = CheckError(ErrorCode::FrameInvalid, "使用阶段字段不完整");
        return false;
    }
    uint8_t phaseValue = static_cast<uint8_t>(content[offset++]);
    frame.phase = static_cast<WorkPhase>(phaseValue);

    // 13. 解析数据体（变长）
    int checksumOffset = frame.dataLength - 1; // 校验位在content最后1字节
    if (offset > checksumOffset) {
        error = CheckError(ErrorCode::FrameInvalid, "数据体+校验位长度不足");
        return false;
    }
    frame.dataBody = content.mid(offset, checksumOffset - offset); // 数据体：offset到校验位前
    frame.checksum = static_cast<uint8_t>(content[checksumOffset]); // 校验位：最后1字节

    // 14. CRC校验（修复数据源：用原始数据的「帧头+content前checksumOffset字节」）
    QByteArray crcData = data.left(headerLen + checksumOffset); // 帧头 + content到校验位前
    uint8_t calculatedCrc = CrcCalculator::crc8(crcData);
    if (calculatedCrc != frame.checksum) {
        error = CheckError(ErrorCode::CrcMismatch,
            QString("CRC校验失败（计算值：0x%1，实际值：0x%2）")
            .arg(calculatedCrc, 2, 16, QLatin1Char('0'))
            .arg(frame.checksum, 2, 16, QLatin1Char('0')));
        return false;
    }

    error = CheckError();
    return true;

    //qDebug() << "parser crc数据源：" << crcData.toHex();
}

bool FrameParser::parseDataBody(const QByteArray& dataBody, DeviceId& deviceId, CommandCode& cmdCode, QByteArray& param)
{
    // 数据体至少需要：器件ID(1) + 命令码(1) = 2字节
    if (dataBody.size() < 2) {
        return false;
    }

    // 解析器件ID（1字节）
    uint8_t deviceIdValue = static_cast<uint8_t>(dataBody[0]);
    deviceId = static_cast<DeviceId>(deviceIdValue);

    // 解析命令码（1字节，大端模式）
    uint8_t cmdCodeValue = static_cast<uint8_t>(dataBody[1]); // 直接取1字节
    cmdCode = static_cast<CommandCode>(cmdCodeValue);

    // 解析参数（剩余字节，参数起始偏移从3→2，因命令码少1字节）
    if (dataBody.size() > 2) {
        param = dataBody.mid(2); // 从索引2开始（0:器件ID, 1:命令码）
    }
    else {
        param = QByteArray(); // 无参数
    }

    return true;
}

// 解析16位无符号整数（大端模式）
uint16_t FrameParser::bytesToUint16(const QByteArray& bytes, int offset) {
    if (offset + 1 >= bytes.size()) {
        return 0;
    }
    return (static_cast<uint16_t>(static_cast<uint8_t>(bytes[offset])) << 8) |
        static_cast<uint16_t>(static_cast<uint8_t>(bytes[offset + 1]));
}

bool FrameParser::parseWaterAcousticFrame(const QByteArray& frameData, Frame& frame, CheckError& error)
{
    // 1. 基础长度校验（帧头4 + 至少1字节内容 + 帧尾2 = 最小7字节）
    int minWaFrameLen = FrameConstants::WATER_ACOUSTIC_HEADER_LENGTH + FrameConstants::WATER_ACOUSTIC_FOOTER_LENGTH + 1;
    if (frameData.size() < minWaFrameLen) {
        error = CheckError::fromWaterAcousticError(QString("帧长度不足（最小%1字节，实际%2字节）").arg(minWaFrameLen).arg(frameData.size()));
        return false;
    }

    // 2. 校验帧头（前4字节：A5 A5 A5 D5）
    QByteArray header = frameData.left(FrameConstants::WATER_ACOUSTIC_HEADER_LENGTH);
    QByteArray expectedHeader = QByteArray(reinterpret_cast<const char*>(FrameConstants::WATER_ACOUSTIC_HEADER), FrameConstants::WATER_ACOUSTIC_HEADER_LENGTH);
    if (header != expectedHeader) {
        error = CheckError::fromWaterAcousticError(QString("帧头错误（实际：%1，期望：%2）").
            arg(QString(header.toHex(' '))).arg(QString(expectedHeader.toHex(' '))));
        return false;
    }

    // 3. 校验帧尾（最后2字节：A5 D4）
    QByteArray footer = frameData.right(FrameConstants::WATER_ACOUSTIC_FOOTER_LENGTH);
    QByteArray expectedFooter = QByteArray(reinterpret_cast<const char*>(FrameConstants::WATER_ACOUSTIC_FOOTER), FrameConstants::WATER_ACOUSTIC_FOOTER_LENGTH);
    if (footer != expectedFooter) {
        error = CheckError::fromWaterAcousticError(QString("帧尾错误（实际：%1，期望：%2）").
            arg(QString(footer.toHex(' '))).arg(QString(expectedFooter.toHex(' '))));
        return false;
    }

    // 4. 提取核心内容（帧头后 → 帧尾前）
    QByteArray content = frameData.mid(
        FrameConstants::WATER_ACOUSTIC_HEADER_LENGTH,
        frameData.size() - FrameConstants::WATER_ACOUSTIC_HEADER_LENGTH - FrameConstants::WATER_ACOUSTIC_FOOTER_LENGTH
    );

    // 5. 映射到Frame结构
    frame.header = header;
    frame.footer = footer;
    frame.dataLength = content.size(); // 核心内容长度
    frame.timestamp = QDateTime::currentDateTime();
    frame.packetSeq = 0;
    frame.channel = 0x00;
    frame.senderDevice = FrameConstants::AUV_DEVICE_ID;
    frame.receiverDevice = FrameConstants::SHORE_STATION_DEVICE_ID;
    frame.phase = WorkPhase::DeviceTest;
    frame.dataBody = content; // 核心内容存入dataBody
    frame.checksum = 0x00; // 水声帧外层无CRC

    error = CheckError(ErrorCode::Success);
    return true;
}

WaterAcousticCmd FrameParser::parseWaterAcousticCmd(const Frame& frame)
{
    // 1. 校验水声帧头（前5字节：A5 A5 A5 D5 01 / 02）
    // 兼容帧头第5字节为01/02的情况（仅校验前4字节核心帧头）
    QByteArray coreHeader = QByteArray(reinterpret_cast<const char*>(FrameConstants::WATER_ACOUSTIC_HEADER), 4); // 取前4字节：A5 A5 A5 D5
    if (frame.header.size() < 4 || frame.header.left(4) != coreHeader) {
        return WaterAcousticCmd::Unknown;
    }

    QByteArray content = frame.dataBody;
    int contentLen = content.size();

    // 2. 匹配指令1：通信机收到数据准备发送（帧内容：00，1字节）
    // 对应帧：A5 A5 A5 D5 01 00 A5 D4
    if (contentLen == 1 && content == QByteArray::fromHex("00")) {
        return WaterAcousticCmd::RecvConfirm;
    }
    // 3. 匹配指令2：通信机收到新数据（帧内容：02 00，2字节）
    // 对应帧：A5 A5 A5 D5 02 02 00 A5 D4
    else if (contentLen == 2 && content == QByteArray::fromHex("0200")) {
        return WaterAcousticCmd::RecvNotify;
    }
    // 5. 匹配指令4：通信机转发的有效数据
    else if (contentLen > 0 && contentLen <= FrameConstants::WATER_ACOUSTIC_MAX_CONTENT_LEN) {
        return WaterAcousticCmd::DataReceived;
    }
    // 6. 未知指令
    else {
        return WaterAcousticCmd::Unknown;
    }
}

bool FrameParser::parseWaterAcousticCoreData(const QByteArray& frameData, 
    const QByteArray& coreData, Frame& frame,
    CheckError& error, WaterAcousticUplinkCmd cmd)
{
    // 非DataReceived指令
    if (cmd != WaterAcousticUplinkCmd::DataReceived) {
        if (coreData.isEmpty()) {
            error = CheckError(ErrorCode::FrameInvalid, QString("%1指令核心数据为空").arg(static_cast<int>(cmd)));
            return false;
        }
        frame.timestamp = QDateTime::currentDateTime();
        frame.dataBody = coreData;
        error = CheckError();
        return true;
    }

    // ========== DataReceived指令：修复核心逻辑 ==========
    // 1. 最小长度校验：通用帧字段（包序号+信道+设备+阶段+时间戳）+ 数据体 + 校验位
    // 包序号(1)+信道(1)+发送设备(2)+接收设备(2)+阶段(1)+时间戳(6) = 13字节，至少+1字节数据体+1字节校验位 = 15字节
    const int MIN_CORE_DATA_LEN = 15;
    if (coreData.size() < MIN_CORE_DATA_LEN) {
        error = CheckError(ErrorCode::FrameInvalid,
            QString("DataReceived核心数据过短（需≥%1字节，实际%2字节）").arg(MIN_CORE_DATA_LEN).arg(coreData.size()));
        return false;
    }

    // 2. 修正字段偏移定义（匹配实际帧结构）
    const int OFFSET_DATA_LEN = 0;        // 数据长度（2字节，大端）
    const int OFFSET_TIMESTAMP = 2;       // 时间戳（6字节：2-7）
    const int OFFSET_PACKET_SEQ = 8;      // 包序号（1字节）
    const int OFFSET_CHANNEL = 9;         // 通信信道（1字节）
    const int OFFSET_SENDER_DEV = 10;     // 发送设备（2字节，大端）
    const int OFFSET_RECEIVER_DEV = 12;   // 接收设备（2字节，大端）
    const int OFFSET_PHASE = 14;          // 使用阶段（1字节）
    const int OFFSET_DATA_BODY = 15;      // 数据体起始
    const int OFFSET_CHECKSUM = coreData.size() - 1; // 校验位（最后1字节）

    // 3. 提取字段（严格按协议顺序）
    // 3.1 数据长度（大端序）
    uint16_t dataLen = (static_cast<uint16_t>(coreData[OFFSET_DATA_LEN]) << 8) | coreData[OFFSET_DATA_LEN + 1];
    frame.dataLength = dataLen;

    // 3.2 时间戳（6字节：年、月、日、时、分、秒）
    QByteArray timestampBytes = coreData.mid(OFFSET_TIMESTAMP, FrameConstants::TIMESTAMP_LENGTH);
    frame.timestamp = parseTimestamp(timestampBytes);

    // 3.3 包序号
    frame.packetSeq = static_cast<uint8_t>(coreData[OFFSET_PACKET_SEQ]);

    // 3.4 通信信道
    frame.channel = static_cast<uint8_t>(coreData[OFFSET_CHANNEL]);

    // 3.5 发送/接收设备（核心：下位机和上位机相反，互换值）
    // 先解析下位机的原始值
    uint16_t senderDevLower = (static_cast<uint16_t>(coreData[OFFSET_SENDER_DEV]) << 8) | coreData[OFFSET_SENDER_DEV + 1];
    uint16_t receiverDevLower = (static_cast<uint16_t>(coreData[OFFSET_RECEIVER_DEV]) << 8) | coreData[OFFSET_RECEIVER_DEV + 1];
    // 互换：上位机的发送设备 = 下位机的接收设备，上位机的接收设备 = 下位机的发送设备
    frame.senderDevice = receiverDevLower;
    frame.receiverDevice = senderDevLower;

    // 3.6 使用阶段
    frame.phase = static_cast<WorkPhase>(coreData[OFFSET_PHASE]);

    // 3.7 数据体（从15字节到校验位前）
    frame.dataBody = coreData.mid(OFFSET_DATA_BODY, OFFSET_CHECKSUM - OFFSET_DATA_BODY);

    // 3.8 校验位
    frame.checksum = static_cast<uint8_t>(coreData[OFFSET_CHECKSUM]);

    // 4. CRC计算：包含水声帧头（已修复，保留）
    QByteArray waHeader = frameData.left(FrameConstants::WATER_ACOUSTIC_HEADER_LENGTH);
    QByteArray coreDataBeforeChecksum = coreData.left(OFFSET_CHECKSUM);
    QByteArray crcSource = waHeader + coreDataBeforeChecksum;
    uint8_t calculatedCrc = CrcCalculator::crc8(crcSource);

    // 调试日志（新增：输出解析后的字段，方便验证）
    qDebug() << "[字段解析调试] 数据长度：0x" << QString::number(dataLen, 16);
    qDebug() << "[字段解析调试] 包序号：0x" << QString::number(frame.packetSeq, 16);
    qDebug() << "[字段解析调试] 通信信道：0x" << QString::number(frame.channel, 16);
    qDebug() << "[字段解析调试] 发送设备（互换后）：0x" << QString::number(frame.senderDevice, 16);
    qDebug() << "[字段解析调试] 接收设备（互换后）：0x" << QString::number(frame.receiverDevice, 16);
    qDebug() << "[字段解析调试] 使用阶段：0x" << QString::number(static_cast<uint8_t>(frame.phase), 16);
    qDebug() << "[字段解析调试] 数据体：" << frame.dataBody.toHex(' ');
    qDebug() << "[CRC调试] 完整数据源（帧头+coreData校验位前）：" << crcSource.toHex(' ');
    qDebug() << "[CRC调试] 计算值：0x" << QString::number(calculatedCrc, 16).rightJustified(2, '0');
    qDebug() << "[CRC调试] 实际值：0x" << QString::number(frame.checksum, 16).rightJustified(2, '0');

    if (calculatedCrc != frame.checksum) {
        error = CheckError(ErrorCode::CrcMismatch,
            QString("CRC校验失败（计算值：0x%1，实际值：0x%2）")
            .arg(calculatedCrc, 2, 16, QLatin1Char('0'))
            .arg(frame.checksum, 2, 16, QLatin1Char('0')));
        return false;
    }

    // 5. 基础信息赋值
    frame.header = waHeader;
    frame.footer = QByteArray();

    error = CheckError(); 
    return true;
}

// 解析时间戳
QDateTime FrameParser::parseTimestamp(const QByteArray& timestampBytes) {
    if (timestampBytes.size() != FrameConstants::TIMESTAMP_LENGTH) {
        return QDateTime();
    }

    // 提取各字段（uint8转整数）
    uint8_t year = static_cast<uint8_t>(timestampBytes[0]);
    uint8_t month = static_cast<uint8_t>(timestampBytes[1]);
    uint8_t day = static_cast<uint8_t>(timestampBytes[2]);
    uint8_t hour = static_cast<uint8_t>(timestampBytes[3]);
    uint8_t minute = static_cast<uint8_t>(timestampBytes[4]);
    uint8_t second = static_cast<uint8_t>(timestampBytes[5]);

    // 校验字段合法性（避免无效时间）
    if (month < 1 || month > 12 || day < 1 || day > 31 || hour > 23 || minute > 59 || second > 59) {
        return QDateTime();
    }

    // 年份补全为4位（默认20xx，可根据实际需求调整）
    int fullYear = 2000 + year;
    QDate date(fullYear, month, day);
    QTime time(hour, minute, second);

    return QDateTime(date, time, Qt::UTC); 
}
