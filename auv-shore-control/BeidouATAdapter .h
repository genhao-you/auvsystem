#pragma once
#include"internalprotocoladapter.h"
#include"framebuilder.h"
#include <memory>
class BeidouATAdapter : public InternalProtocolAdapter {
public:
    DeviceId supportedDeviceId() const override {
        return DeviceId::BeidouComm; // 支持北斗设备
    }

    // 生成北斗AT指令（按协议映射）
    QByteArray generateCommand(CommandCode cmdCode, const std::shared_ptr<CommandParameter>& param) override {
        auto internalParam = std::dynamic_pointer_cast<InternalCommandParam>(param);
        QVariantMap extraParams = internalParam ? internalParam->extraParams() : QVariantMap();
        QString atCmd;

        switch (cmdCode) {
        case CommandCode::BeidouComm_GetConnectStatus:
            atCmd = "AT+STACNT\r\n"; 
            break;
        case CommandCode::BeidouComm_GetSignalQuality: 
            atCmd = "AT+CSQ\r\n"; 
            break;
        case CommandCode::BeidouComm_GetIdentity: 
            atCmd = "AT+LNO\r\n"; 
            break;
        case CommandCode::BeidouComm_GetTarget: 
            atCmd = "AT+PNO\r\n";
            break;
        case CommandCode::BeidouComm_SetTarget: 
        {
            auto cardParam = std::dynamic_pointer_cast<BeidouCardNoParam>(param);
            if (!cardParam) {
                qWarning() << "[北斗AT] 设置目标卡号失败：参数类型不是BeidouCardNoParam";
                return QByteArray();
            }
            // 直接从BeidouCardNoParam获取7位补零字符串（无需手动补零）
            QString cardStr = cardParam->to7DigitStr();
            atCmd = QString("AT+PNO %1\r\n").arg(cardStr);
            break;
        }
        case CommandCode::BeidouComm_GetWhitelist: 
            atCmd = "AT+LISTPD\r\n";
            break;
        case CommandCode::BeidouComm_AddWhitelist: 
        {
            // 动态转型为BeidouCardNoParam，获取正确卡号
            auto cardParam = std::dynamic_pointer_cast<BeidouCardNoParam>(param);
            if (!cardParam) {
                qWarning() << "[北斗AT] 添加白名单失败：参数类型不是BeidouCardNoParam";
                return QByteArray();
            }
            // 直接从BeidouCardNoParam获取7位补零字符串（无需手动补零）
            QString cardStr = cardParam->to7DigitStr();
            atCmd = QString("AT+ADDPD %1\r\n").arg(cardStr);
            break;
        }
        case CommandCode::BeidouComm_DelWhitelist: 
        {
            // 核心修改：动态转型为BeidouCardNoParam，获取正确卡号
            auto cardParam = std::dynamic_pointer_cast<BeidouCardNoParam>(param);
            if (!cardParam) {
                qWarning() << "[北斗AT] 删除白名单失败：参数类型不是BeidouCardNoParam";
                return QByteArray();
            }
            // 直接从BeidouCardNoParam获取7位补零字符串（无需手动补零）
            QString cardStr = cardParam->to7DigitStr();
            atCmd = QString("AT+DELPD %1\r\n").arg(cardStr);
            break;
        }
        case CommandCode::BeidouComm_GetPosition:
            atCmd = "AT+GPS\r\n"; 
            break;
        case CommandCode::BeidouComm_GetSysInfo: 
            atCmd = "AT+VER\r\n";
            break;
        case CommandCode::BeidouComm_FactoryReset:
            atCmd = "AT+DEF\r\n"; 
            break;
        case CommandCode::BeidouComm_Reboot:
            atCmd = "AT+RESET\r\n";
            break;
        case CommandCode::BeidouComm_Test:
        {
            // 1. 从参数中获取核心帧信息（移除旧协议长度相关冗余参数，仅保留必要数据）
            uint8_t packetSeq = extraParams["packetSeq"].toUInt();
            QString frameHexStr = extraParams["original_frame_hex"].toString();// 帧HEX（日志用）
            QByteArray frameRawBytes = extraParams["original_frame_data"].toByteArray(); // 核心：原始帧字节流（新协议直接使用）
            // 2. 简化参数有效性校验（仅校验核心帧数据非空，移除旧协议长度范围校验）
            if (frameRawBytes.isEmpty() && frameHexStr.isEmpty()) {
                qWarning() << "[北斗AT] 帧信息为空，无法构建AT+SND指令";
                return QByteArray();
            }

            // 3. 拼接新协议指令：AT+SND + 空格 + 原始帧数据 + \r\n（摒弃旧协议X1长度字段）
            // 优先使用原始字节流（更贴合硬件发送），若原始流为空则使用HEX字符串
            QByteArray sndData = frameRawBytes.isEmpty() ? frameHexStr.toUtf8() : frameRawBytes;
            atCmd = QString("AT+SND %1\r\n").arg(QString::fromUtf8(sndData));
            qDebug() << "[北斗AT] 新协议最终指令：" << atCmd.trimmed();

            // 4. 保留原有参数保存逻辑（不影响后续解析和日志，无需修改）
            if (internalParam) {
                internalParam->setExtraParam("sent_data_hex", frameHexStr);          // 帧HEX字符串
                internalParam->setExtraParam("sent_data_byte_len", frameRawBytes.size()); // 字节长度（仅日志用，非协议必需）
                internalParam->setExtraParam("sent_x2_raw", frameRawBytes);          // 原始字节流
                internalParam->setExtraParam("recv_data_hex", "");                   // 初始化接收数据
            }
            return atCmd.toUtf8();
            break;
        }
        default: atCmd = ""; break;
        }

        qDebug() << "[北斗AT指令] 生成：" << atCmd.trimmed();
        return atCmd.toUtf8(); // AT指令字节流（含\r\n）
    }

    // 解析北斗AT指令反馈
    SelfCheckResult parseResponse(CommandCode cmdCode, const QByteArray& response) override {
        // 核心：直接调用带参构造，避免默认构造的坑
        // 参数：设备ID、命令码、默认状态（失败）、指令类型（内部）
        SelfCheckResult result(DeviceId::BeidouComm, cmdCode, ResultStatus::Failed, CommandType::Internal);

        QString respStr = QString::fromUtf8(response).trimmed();
        // 保存原始反馈数据
        result.setFeedbackData(response);

        if (respStr.isEmpty()) {
            // 特殊处理：测试指令成功时无返回
            if (cmdCode == CommandCode::BeidouComm_Test) {
                result.setStatus(ResultStatus::Success);
                result.setFeedbackDesc("数据发送成功（无返回表示指令执行正常）");
            }
            else {
                result.setStatus(ResultStatus::Failed);
                result.setFeedbackDesc("无反馈数据");
            }
            return result;
        }

        // 按命令码解析反馈
        switch (cmdCode) {
        case CommandCode::BeidouComm_GetConnectStatus:
            if (respStr.startsWith("STACNT")) {
                QStringList parts = respStr.split(" ", Qt::SkipEmptyParts);
                if (parts.size() >= 2) {
                    bool ok;
                    int status = parts[1].toInt(&ok);
                    // 优化：增加转换成功校验
                    if (ok && (status == 0 || status == 1)) { // 协议规定0=已连接，1=未连接
                        result.setParameter("connectStatus", status);
                        result.setFeedbackDesc(QString("连接状态：%1（0=已连接，1=未连接）").arg(status));
                        result.setStatus(ResultStatus::Success);
                    }
                    else {
                        result.setFeedbackDesc("连接状态值无效（必须为0或1）：" + respStr);
                    }
                }
                else {
                    result.setFeedbackDesc("连接状态反馈格式错误（应为STACNT [X]）：" + respStr);
                }
            }
            else {
                result.setFeedbackDesc("连接状态反馈格式错误（应为STACNT [X]）：" + respStr);
            }
            break;

        case CommandCode::BeidouComm_GetSignalQuality:
            if (respStr.startsWith("CSQ:")) { // 协议反馈：CSQ:[X]
                // 优化：增加字符串长度校验，避免mid越界
                if (respStr.length() < 4) {
                    result.setFeedbackDesc("信号质量反馈格式错误（长度不足）：" + respStr);
                    break;
                }
                bool ok;
                int signalLevel = respStr.mid(4).toInt(&ok);
                // 优化：增加转换成功校验
                if (ok && signalLevel >= 0 && signalLevel <= 5) { // 协议规定0-5级
                    result.setParameter("signalQuality", signalLevel);
                    result.setFeedbackDesc(QString("信号质量：%1（0-5级）").arg(signalLevel));
                    result.setStatus(ResultStatus::Success);
                }
                else {
                    result.setFeedbackDesc("信号质量值无效（必须为0-5的整数）：" + respStr);
                }
            }
            else {
                result.setFeedbackDesc("信号质量反馈格式错误（应为CSQ:X）：" + respStr);
            }
            break;

        case CommandCode::BeidouComm_GetIdentity:
            if (respStr.startsWith("LNO:")) { // 协议反馈：LNO:[X]
                // 优化：增加字符串长度校验
                if (respStr.length() < 4) {
                    result.setFeedbackDesc("身份查询反馈格式错误（长度不足）：" + respStr);
                    break;
                }
                QString cardStr = respStr.mid(4).trimmed();
                bool ok;
                uint32_t cardNo = cardStr.toUInt(&ok);
                // 优化：增加纯数字判断
                bool isAllDigit = true;
                for (QChar c : cardStr) {
                    if (!c.isDigit()) {
                        isAllDigit = false;
                        break;
                    }
                }
                if (ok && isAllDigit && cardStr.length() == 7 && cardNo <= 9999999) { // 协议规定7位数字
                    result.setParameter("localCardNo", cardNo);
                    result.setFeedbackDesc(QString("本机卡号：%1（7位有效）").arg(cardStr));
                    result.setStatus(ResultStatus::Success);
                }
                else {
                    result.setFeedbackDesc("本机卡号格式错误（必须为7位纯数字）：" + respStr);
                }
            }
            else {
                result.setFeedbackDesc("身份查询反馈格式错误（应为LNO:XXXXXXX）：" + respStr);
            }
            break;

        case CommandCode::BeidouComm_GetTarget:
        case CommandCode::BeidouComm_SetTarget:
            if (respStr.startsWith("PNO:")) { // 协议反馈：PNO:[X]
                // 优化：增加字符串长度校验
                if (respStr.length() < 4) {
                    result.setFeedbackDesc("目标卡号反馈格式错误（长度不足）：" + respStr);
                    break;
                }
                QString cardStr = respStr.mid(4).trimmed();
                if (cardStr.isEmpty() || cardStr == "0000000") {
                    result.setFeedbackDesc(cmdCode == CommandCode::BeidouComm_SetTarget
                        ? "目标卡号设置成功（未指定具体卡号）"
                        : "未设置目标卡号");
                    result.setStatus(ResultStatus::Success);
                }
                else {
                    bool ok;
                    uint32_t cardNo = cardStr.toUInt(&ok);
                    // 优化：增加纯数字判断
                    bool isAllDigit = true;
                    for (QChar c : cardStr) {
                        if (!c.isDigit()) {
                            isAllDigit = false;
                            break;
                        }
                    }
                    if (ok && isAllDigit && cardStr.length() == 7 && cardNo <= 9999999) {
                        result.setParameter("targetCardNo", cardNo);
                        result.setFeedbackDesc(cmdCode == CommandCode::BeidouComm_SetTarget
                            ? QString("目标卡号设置成功：%1（7位）").arg(cardStr)
                            : QString("目标卡号查询成功：%1（7位）").arg(cardStr));
                        result.setStatus(ResultStatus::Success);
                    }
                    else {
                        result.setFeedbackDesc("目标卡号格式错误（必须为7位纯数字）：" + respStr);
                    }
                }
            }
            else {
                result.setFeedbackDesc("目标卡号反馈格式错误（应为PNO:XXXXXXX）：" + respStr);
            }
            break;

        case CommandCode::BeidouComm_GetWhitelist:
        case CommandCode::BeidouComm_AddWhitelist:
        case CommandCode::BeidouComm_DelWhitelist:
            if (respStr.startsWith("White List:")) { // 协议反馈：White List:<CR><LF>[X]<CR><LF>
                // 修正：兼容换行和逗号分隔，跳过"White List:"后处理
                QString listContent = respStr.mid(11).trimmed();
                QStringList whitelist = listContent.split(QRegExp("[\r\n,]"), Qt::SkipEmptyParts);

                bool hasInvalid = false;
                QStringList validWhitelist;
                for (const QString& card : whitelist) {
                    QString trimmedCard = card.trimmed();
                    // 优化：增加纯数字判断，更严谨
                    bool isAllDigit = true;
                    for (QChar c : trimmedCard) {
                        if (!c.isDigit()) {
                            isAllDigit = false;
                            break;
                        }
                    }
                    if (isAllDigit && trimmedCard.length() == 7 && trimmedCard.toUInt(nullptr, 10) <= 9999999) {
                        validWhitelist.append(trimmedCard);
                    }
                    else {
                        hasInvalid = true;
                    }
                }

                result.setParameter("whitelist", validWhitelist);
                QString cmdDesc = cmdCode == CommandCode::BeidouComm_GetWhitelist ? "查询"
                    : (cmdCode == CommandCode::BeidouComm_AddWhitelist ? "添加" : "删除");
                if (hasInvalid) {
                    result.setFeedbackDesc(QString("白名单%1成功（部分卡号无效已过滤，当前有效：%2个）")
                        .arg(cmdDesc).arg(validWhitelist.size()));
                }
                else {
                    result.setFeedbackDesc(QString("白名单%1成功（当前有效：%2个）").arg(cmdDesc).arg(validWhitelist.size()));
                }
                result.setStatus(ResultStatus::Success);
            }
            else {
                result.setFeedbackDesc("白名单反馈格式错误（应为White List:...）：" + respStr);
            }
            break;

        case CommandCode::BeidouComm_GetPosition:
            // 必须修改：修正GNRMC开头判断（带$，无冒号）
            if (respStr.startsWith("$GNRMC")) {
                QString nmeaData = respStr.trimmed();
                result.setParameter("nmeaData", nmeaData);
                // 优化：保留空字段，避免索引错位
                QStringList nmeaParts = nmeaData.split(',', Qt::KeepEmptyParts);
                // 优化：增加索引校验，避免越界
                bool isValidFix = false;
                QString fixStatus = "";
                double latDegree = 0.0;
                double lonDegree = 0.0;

                if (nmeaParts.size() >= 13) {
                    fixStatus = nmeaParts[2]; // 定位状态：A=有效，V=无效
                    isValidFix = (fixStatus == "A");
                    // 存储原始字段
                    result.setParameter("latitude_raw", nmeaParts[3]);
                    result.setParameter("latitude_dir", nmeaParts[4]);
                    result.setParameter("longitude_raw", nmeaParts[5]);
                    result.setParameter("longitude_dir", nmeaParts[6]);
                    result.setParameter("fix_status", fixStatus);
                    result.setParameter("is_valid_fix", isValidFix);

                    // 可选：转换为十进制经纬度，提升实用性
                    auto convertNMEAToDegree = [](const QString& nmeaStr, const QString& hemisphere) -> double {
                        if (nmeaStr.isEmpty()) return 0.0;
                        int degreeLen = (hemisphere == "N" || hemisphere == "S") ? 2 : 3;
                        if (nmeaStr.length() < degreeLen) return 0.0;

                        bool degOk = false;
                        bool minOk = false;
                        double degree = nmeaStr.left(degreeLen).toDouble(&degOk);
                        double minute = nmeaStr.mid(degreeLen).toDouble(&minOk);
                        if (!degOk || !minOk) return 0.0;

                        return degree + (minute / 60.0);
                    };

                    // 安全转换经纬度
                    QString latRaw = nmeaParts[3];
                    QString latDir = nmeaParts[4];
                    QString lonRaw = nmeaParts[5];
                    QString lonDir = nmeaParts[6];

                    latDegree = convertNMEAToDegree(latRaw, latDir);
                    lonDegree = convertNMEAToDegree(lonRaw, lonDir);

                    if (latDir == "S") latDegree = -latDegree;
                    if (lonDir == "W") lonDegree = -lonDegree;

                    result.setParameter("latitude_degree", latDegree);
                    result.setParameter("longitude_degree", lonDegree);
                }

                // 优化：根据定位有效性更新反馈描述
                if (isValidFix) {
                    result.setFeedbackDesc(QString("定位信息获取成功（有效定位）：纬度%1°，经度%2°，原始数据：%3")
                        .arg(latDegree, 0, 'f', 6).arg(lonDegree, 0, 'f', 6).arg(nmeaData));
                }
                else {
                    result.setFeedbackDesc(QString("定位信息获取成功（无效定位）：状态%1，原始数据：%2")
                        .arg(fixStatus).arg(nmeaData));
                }
                result.setStatus(ResultStatus::Success);
            }
            else {
                result.setFeedbackDesc("定位信息反馈格式错误（应为$GNRMC开头的NMEA语句）：" + respStr);
            }
            break;

        case CommandCode::BeidouComm_GetSysInfo: // 补充系统信息解析
            if (respStr.startsWith("VER:")) { // 协议反馈：VER:[X]
                // 优化：增加字符串长度校验
                if (respStr.length() < 4) {
                    result.setFeedbackDesc("系统信息反馈格式错误（长度不足）：" + respStr);
                    break;
                }
                QString sysVersion = respStr.mid(4).trimmed();
                result.setParameter("systemInfo", sysVersion);
                result.setFeedbackDesc(QString("固件版本信息获取成功：%1").arg(sysVersion));
                result.setStatus(ResultStatus::Success);
            }
            else {
                result.setFeedbackDesc("系统信息反馈格式错误（应为VER:...）：" + respStr);
            }
            break;

        case CommandCode::BeidouComm_FactoryReset:
            if (respStr == "DEF:OK") { // 协议反馈：DEF:OK<CR><LF>
                result.setStatus(ResultStatus::Success);
                result.setFeedbackDesc("恢复出厂设置成功（设备将重启，所有配置清空）");
            }
            else {
                result.setFeedbackDesc("恢复出厂设置反馈错误（应为DEF:OK）：" + respStr);
            }
            break;

        case CommandCode::BeidouComm_Reboot:
            if (respStr == "RESET:OK") { // 协议反馈：RESET:OK
                result.setStatus(ResultStatus::Success);
                result.setFeedbackDesc("模块重启指令发送成功（设备正在重启）");
            }
            else {
                result.setFeedbackDesc("重启反馈错误（应为RESET:OK）：" + respStr);
            }
            break;

        case CommandCode::BeidouComm_Test:
        {
            // 核心修改：兼容新协议返回值，补充SND:ONGOING和SND:SUCCESS解析
            if (respStr == "NO SIM!") {
                result.setStatus(ResultStatus::Failed);
                result.setFeedbackDesc("数据发送失败：无SIM卡");
            }
            else if (respStr.startsWith("SND:ONGOING")) {
                result.setStatus(ResultStatus::Success); // 发送中视为成功（正常流程）
                result.setFeedbackDesc(QString("数据发送中：%1").arg(respStr));
            }
            else if (respStr.startsWith("SND:SUCCESS")) {
                result.setStatus(ResultStatus::Success);
                result.setFeedbackDesc(QString("数据发送成功：%1").arg(respStr));
            }
            else if (respStr == "SND:ERROR,BUSY") {
                result.setStatus(ResultStatus::Failed);
                result.setFeedbackDesc("数据发送失败：上次发送未结束，设备忙");
            }
            else if (respStr == "SND:ERROR,BUFOVF") {
                result.setStatus(ResultStatus::Failed);
                result.setFeedbackDesc("数据发送失败：缓冲区溢出");
            }
            else if (respStr == "SND:ERROR,NODATA") {
                result.setStatus(ResultStatus::Failed);
                result.setFeedbackDesc("数据发送失败：发送数据为空");
            }
            else {
                result.setStatus(ResultStatus::Unknown);
                result.setFeedbackDesc("数据发送反馈未知：" + respStr);
            }
            break;
        }


        default:
            result.setStatus(ResultStatus::Unknown);
            result.setFeedbackDesc("北斗未知命令，原始反馈：" + respStr);
            break;
        }

        return result;
    }
};