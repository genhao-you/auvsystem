#include"frame.h"
#include"frameparser.h"
#include"communicationchannel.h"
QJsonObject Frame::toJson() const {
    QJsonObject json;
    json["header"] = QString(header);
    json["data_length"] = dataLength;
    json["timestamp"] = timestamp.toString("yyMMddhhmmss"); // 按协议格式存储
    json["packet_seq"] = static_cast<int>(packetSeq);
    json["channel"] = static_cast<int>(channel);
    json["sender_device"] = senderDevice;
    json["receiver_device"] = receiverDevice;
    json["phase"] =  static_cast<uint8_t>(phase);
    json["data_body"] = QString(dataBody.toBase64());
    json["checksum"] = static_cast<int>(checksum);
    json["footer"] = QString(footer);
    return json;
}

Frame Frame::fromJson(const QJsonObject& json) {
    Frame frame;
    frame.header = json["header"].toString().toUtf8();
    frame.dataLength = json["data_length"].toInt();
    // 解析时间戳（协议格式：yyMMddhhmmss）
    frame.timestamp = FrameParser::parseTimestamp(json["timestamp"].toString().toUtf8());
    frame.packetSeq = static_cast<uint8_t>(json["packet_seq"].toInt());
    frame.channel = static_cast<uint8_t>(json["channel"].toInt());
    frame.senderDevice = json["sender_device"].toInt();
    frame.receiverDevice = json["receiver_device"].toInt();
    frame.phase = static_cast<WorkPhase>(static_cast<uint8_t>(json["phase"].toInt(-1)));
    frame.dataBody = QByteArray::fromBase64(json["data_body"].toString().toUtf8());
    frame.checksum = static_cast<uint8_t>(json["checksum"].toInt());
     frame.footer = json["footer"].toString().toUtf8();
    return frame;
}