#pragma once
#include <QByteArray>
#include <QVariant> 
#include <stdexcept>
#include "selfcheckenum.h"
#include<QJsonObject>
#include <QtEndian>
#include "controlparamdefs.h"
/**
 * @brief 命令参数基类
 *
 * 强类型封装命令参数，确保参数合法性
 */
class CommandParameter {
public:
    virtual ~CommandParameter() = default;

    /**
     * @brief 转换为字节流
     * @return 参数的字节表示
     */
    virtual QByteArray toBytes() const = 0;

    virtual QJsonObject toJson()const = 0;

    /**
     * @brief 获取参数描述
     * @return 参数的人类可读描述
     */
    virtual QString description() const = 0;
};


// 空参数（无参数命令专用）
class EmptyParameter : public CommandParameter {
public:
    QByteArray toBytes() const override { return QByteArray(); }
    QJsonObject toJson() const override { return { {"description", "无参数"} }; }
    QString description() const override { return "无参数"; }
};


/**
 * @brief 转速参数
 *
 * 确保转速在有效范围内（0-3000 RPM）
 */
class SpeedParameter : public CommandParameter {
public:
    explicit SpeedParameter(int16_t rpm) {
        if (rpm < -450 || rpm > 450) {
            throw std::invalid_argument("转速超出有效范围（-450-450 RPM）");
        }
        m_rpm = rpm;
    }

    QByteArray toBytes() const override {
        int16_t bigEndianVal = qToBigEndian(m_rpm); // 大端序转换
        return QByteArray(reinterpret_cast<const char*>(&bigEndianVal), 2); // 深拷贝
    }
    QJsonObject toJson() const override {
        QJsonObject obj;
        obj["value"] = static_cast<int16_t>(m_rpm);
        return obj;
    }
    QString description() const override {
        return QString("%1 RPM").arg(m_rpm);
    }

    int16_t rpm() const { return m_rpm; }

private:
    int16_t m_rpm;
};
// 推进器转速+运行时间组合参数（转速int16，运行时间uint8）
class ThrusterSpeedAndRunTimeParameter : public CommandParameter {
public:
    /**
     * @brief 构造函数
     * @param rpm 转速（int16，范围：-450~450 RPM，和SpeedParameter保持一致）
     * @param runTimeSec 运行时间（uint8，范围：1~255秒，uint8最大取值255，0无实际意义）
     */
    explicit ThrusterSpeedAndRunTimeParameter(int16_t rpm, uint8_t runTimeSec) {
        // 1. 转速合法性校验（复用SpeedParameter的范围）
        if (rpm < -450 || rpm > 450) {
            throw std::invalid_argument("转速超出有效范围（-450-450 RPM）");
        }
        // 2. 运行时间合法性校验（uint8，1~255秒）
        if (runTimeSec < 0 || runTimeSec > 255) {
            throw std::invalid_argument("运行时间超出有效范围（1~255秒）");
        }
        m_rpm = rpm;
        m_runTimeSec = runTimeSec;
    }

    /**
     * @brief 转换为字节流（协议格式：转速(2字节int16大端) + 运行时间(1字节uint8)，共3字节）
     */
    QByteArray toBytes() const override {
        QByteArray bytes;
        // 1. 转速：int16大端序转换（2字节）
        int16_t rpmBigEndian = qToBigEndian(m_rpm);
        bytes.append(reinterpret_cast<const char*>(&rpmBigEndian), 2);
        // 2. 运行时间：uint8直接拼接（1字节，无需要端序转换）
        bytes.append(static_cast<char>(m_runTimeSec));
        return bytes;
    }

    /**
     * @brief 转换为JSON对象（包含数值+单位，方便调试/UI展示）
     */
    QJsonObject toJson() const override {
        QJsonObject obj;
        obj["rpm"] = static_cast<int16_t>(m_rpm);
        obj["run_time_seconds"] = static_cast<int>(m_runTimeSec); // uint8转int避免JSON类型异常
        obj["run_time_desc"] = QString("%1秒").arg(m_runTimeSec);
        obj["rpm_desc"] = QString("%1 RPM").arg(m_rpm);
        return obj;
    }

    /**
     * @brief 参数描述（人类可读）
     */
    QString description() const override {
        return QString("转速：%1 RPM，运行时间：%2 秒")
            .arg(m_rpm)
            .arg(static_cast<int>(m_runTimeSec)); // uint8转int避免打印异常
    }

    // 获取转速值（供外部逻辑调用）
    int16_t rpm() const { return m_rpm; }
    // 获取运行时间值（供外部逻辑调用）
    uint8_t runTimeSec() const { return m_runTimeSec; }

private:
    int16_t m_rpm;         // 转速（int16，-450~450 RPM）
    uint8_t m_runTimeSec;  // 运行时间（uint8，1~255秒）
};
// 推进器占空比参数（开环控制，0-2000，uint16；1000停止，0-1000反转，1000-2000正转）
class ThrusterDutyParameter : public CommandParameter {
public:
    explicit ThrusterDutyParameter(uint16_t duty) {
        if (duty > 2000) throw std::invalid_argument("占空比超出0-2000范围");
        m_duty = duty;
    }
    QByteArray toBytes() const override {
        uint16_t bigEndianVal = qToBigEndian(m_duty); // 大端序转换
        return QByteArray(reinterpret_cast<const char*>(&bigEndianVal), 2); // 深拷贝
    }
    QJsonObject toJson() const override { return { {"duty", m_duty} }; }
    QString description() const override {
        if (m_duty == 1000) return "停止（占空比1000）";
        if (m_duty < 1000) return QString("反转（占空比%1）").arg(m_duty);
        return QString("正转（占空比%1）").arg(m_duty);
    }
    uint16_t duty() const { return m_duty; }
private:
    uint16_t m_duty;
};

// 推进器运行时间参数（单位秒，uint16_t，范围：1~65535秒，适配硬件2字节传输）
class ThrusterRunTimeParameter : public CommandParameter {
public:
    explicit ThrusterRunTimeParameter(uint16_t seconds) {

        // 可选：根据推进器实际需求限定合理上限（如1小时=3600秒）
        if (seconds > 60) {
            throw std::invalid_argument("运行时间超出推荐范围（1~3600秒）");
        }
        m_seconds = seconds;
    }

    QByteArray toBytes() const override {
        uint16_t bigEndianVal = qToBigEndian(m_seconds); // 16位大端序转换
        return QByteArray(reinterpret_cast<const char*>(&bigEndianVal), 2); // 2字节传输（原4字节改为2字节）
    }

    QJsonObject toJson() const override {
        QJsonObject obj;
        obj["seconds"] = static_cast<uint16_t>(m_seconds); // 显式标注uint16类型
        obj["minutes"] = m_seconds / 60.0; // 辅助显示分钟
        return obj;
    }

    QString description() const override {
        if (m_seconds < 60) {
            return QString("%1秒").arg(m_seconds);
        }
        else {
            return QString("%1秒（%2分钟）").arg(m_seconds).arg(m_seconds / 60.0, 0, 'f', 1);
        }
    }

    uint16_t seconds() const { return m_seconds; }

private:
    uint16_t m_seconds;
};


class RudderAngleParameter : public CommandParameter {
public:
    explicit RudderAngleParameter(int16_t angleTenthDegree) {
        // 恢复并修正范围校验：-900（-90度）~900（90度）
        if (angleTenthDegree < -900 || angleTenthDegree > 900) {
            throw std::invalid_argument(
                QString("舵角超出-900~900（*0.1度）范围，实际值：%1").arg(angleTenthDegree).toStdString()
            );
        }
        m_angleTenthDegree = angleTenthDegree;
    }

    QByteArray toBytes() const override {
        // 转换为大端序（int16_t版本的qToBigEndian）
        int16_t bigEndianVal = qToBigEndian(m_angleTenthDegree);
        // 按字节拷贝，确保二进制数据正确
        return QByteArray(reinterpret_cast<const char*>(&bigEndianVal), sizeof(int16_t));
    }

    QJsonObject toJson() const override {
        return {
            {"angle_tenth_degree", m_angleTenthDegree},
            {"angle_degree", m_angleTenthDegree / 10.0}
        };
    }

    QString description() const override {
        return QString("%1度").arg(m_angleTenthDegree / 10.0, 0, 'f', 1); // 保留1位小数更直观
    }

    int16_t angleTenthDegree() const { return m_angleTenthDegree; }

private:
    int16_t m_angleTenthDegree; // 例如30度→300，-30度→-300（int16支持负数）
};

// 惯导对准时间参数（0x01=20min，0x02=60min）
class InsAlignTimeParameter : public CommandParameter {
public:
    explicit InsAlignTimeParameter(uint8_t type) {
        if (type != 0x01 && type != 0x02) throw std::invalid_argument("对准时间类型必须为0x01或0x02");
        m_type = type;
    }
    QByteArray toBytes() const override { return QByteArray(1, static_cast<char>(m_type)); }
    QJsonObject toJson() const override {
        return { {"type", m_type}, {"description", m_type == 0x01 ? "20分钟" : "60分钟"} };
    }
    QString description() const override { return m_type == 0x01 ? "20分钟对准" : "60分钟对准"; }
    uint8_t type() const { return m_type; }
private:
    uint8_t m_type;
};
/**
 * @brief 惯导GNSS参数（经纬度装订、UTC时间等）
 * 用于惯导系统的GNSS相关参数配置，包含纬度、经度（int32_t万分之一度）和UTC时间
 * 经纬度存储：int32_t（1度=100000单位），范围：
 * - 纬度：-9000000 ~ 9000000（对应-90.00000° ~ 90.00000°）
 * - 经度：-18000000 ~ 18000000（对应-180.00000° ~ 180.00000°）
 * 真实经纬度 = 存储值 / 100000
 */
class InsGnssParam : public CommandParameter {
public:

    explicit InsGnssParam(double latitude, double longitude, const QString& utcTime = "") {
        // 硬编码缩放系数，避免静态常量推导问题
        const int32_t SCALE = 100000;
        const int32_t LAT_MIN = -90 * SCALE;
        const int32_t LAT_MAX = 90 * SCALE;
        const int32_t LON_MIN = -180 * SCALE;
        const int32_t LON_MAX = 180 * SCALE;

        // 显式计算，无任何隐式转换
        long long latTemp = static_cast<long long>(latitude * SCALE + 0.5); // 替代round，避免模板
        long long lonTemp = static_cast<long long>(longitude * SCALE + 0.5);

        if (latTemp < LAT_MIN || latTemp > LAT_MAX) {
            throw std::invalid_argument("纬度超出-90.0~90.0°范围");
        }
        if (lonTemp < LON_MIN || lonTemp > LON_MAX) {
            throw std::invalid_argument("经度超出-180.0~180.0°范围");
        }

        m_lat = static_cast<int32_t>(latTemp);
        m_lon = static_cast<int32_t>(lonTemp);
        m_utc = utcTime;
    }

    // 解析用构造函数
    explicit  InsGnssParam(int32_t lat, int32_t lon, const QString& utc = "") : m_lat(lat), m_lon(lon), m_utc(utc) {}

    // 字节流：经度(4字节大端) + 纬度(4字节大端)
    QByteArray toBytes() const override {
        QByteArray bytes;
        int32_t lonBig = qToBigEndian(m_lon);
        bytes.append((char*)&lonBig, 4);
        int32_t latBig = qToBigEndian(m_lat);
        bytes.append((char*)&latBig, 4);
        return bytes;
    }

    QJsonObject toJson() const override {
        QJsonObject obj;
        obj["lon"] = m_lon / 100000.0;
        obj["lat"] = m_lat / 100000.0;
        return obj;
    }

    QString description() const override {
        return QString("经度：%1° 纬度：%2°").arg(m_lon / 100000.0, 0, 'f', 5).arg(m_lat / 100000.0, 0, 'f', 5);
    }

private:
    int32_t m_lat;
    int32_t m_lon;
    QString m_utc;
};
/**
 * @brief 加速时间参数
 * 用于配置设备（如推进器、舵机）的加速时间（从静止到目标状态的过渡时间）
 */
class AccelTimeParameter : public CommandParameter {
public:
    /**
     * @brief 构造函数
     * @param accelTimeMs 加速时间（单位：毫秒，范围：10~10000ms，避免过短导致冲击）
     */
    explicit AccelTimeParameter(uint16_t accelTimeMs) {
        if (accelTimeMs < 10 || accelTimeMs > 10000) {
            throw std::invalid_argument("加速时间超出范围（需10~10000毫秒）");
        }
        m_accelTimeMs = accelTimeMs;
    }

    /**
     * @brief 转换为字节流（uint16_t类型，大端存储）
     */
    QByteArray toBytes() const override {
        uint16_t bigEndianVal = qToBigEndian(m_accelTimeMs); // 大端序转换
        return QByteArray(reinterpret_cast<const char*>(&bigEndianVal), 2); // 深拷贝
    }

    /**
     * @brief 转换为JSON对象
     */
    QJsonObject toJson() const override {
        QJsonObject obj;
        obj["accel_time_ms"] = m_accelTimeMs;
        obj["accel_time_s"] = m_accelTimeMs / 1000.0;  // 同时提供秒级单位
        return obj;
    }

    /**
     * @brief 参数描述（人类可读）
     */
    QString description() const override {
        return QString("加速时间：%1毫秒（%2秒）")
            .arg(m_accelTimeMs)
            .arg(m_accelTimeMs / 1000.0, 0, 'f', 1);
    }

    // 获取加速时间（毫秒）
    uint16_t accelTimeMs() const { return m_accelTimeMs; }

private:
    uint16_t m_accelTimeMs;  // 加速时间（毫秒）
};

// 字符串参数（用于惯导经纬度装订、北斗定位值等）
class StringParameter : public CommandParameter {
public:
    explicit StringParameter(const QString& str) : m_str(str) {}
    QByteArray toBytes() const override { return m_str.toUtf8(); }
    QJsonObject toJson() const override { return { {"value", m_str} }; }
    QString description() const override { return m_str; }
    QString str() const { return m_str; }
private:
    QString m_str;
};

class CameraMediaParam : public CommandParameter {
public:
    /**
     * @brief 构造函数（仅需序号，时间戳由下位机拼接）
     * @param seq 媒体序号（范围：1~65535，不能为0，确保唯一性）
     */
    explicit CameraMediaParam(uint16_t seq) : m_seq(seq) {
        // 仅校验序号合法性
        if (seq == 0) {
            throw std::invalid_argument("媒体序号不能为0");
        }
    }

    /**
     * @brief 转换为字节流（协议格式：仅序号，uint16_t大端）
     */
    QByteArray toBytes() const override {
        // 仅传输序号（大端字节序）
        uint16_t seqBigEndian = qToBigEndian(m_seq);
        return QByteArray(reinterpret_cast<const char*>(&seqBigEndian), 2);
    }

    QJsonObject toJson() const override {
        QJsonObject obj;
        obj["media_sequence"] = static_cast<int>(m_seq);
        obj["description"] = QString("媒体序号：%1（时间戳由下位机拼接）").arg(m_seq);
        return obj;
    }

    QString description() const override {
        return QString("媒体参数：序号%1（时间戳由下位机处理）").arg(m_seq);
    }

    // 获取序号
    uint16_t sequence() const { return m_seq; }

private:
    uint16_t m_seq; // 仅保留序号（1~65535）
};

/**
 * @brief 北斗设备卡号参数（适用于：修改目标卡号、添加/删除白名单）
 * 存储：uint32_t（7位数字，0-9999999）
 * 适配命令：BeidouComm_SetObjective、BeidouComm_AddWhitelist、BeidouComm_DelWhitelist
 */
class BeidouCardNoParam : public CommandParameter {
public:
    // 构造1：直接传入uint32_t数值（内部逻辑调用用）
    explicit BeidouCardNoParam(uint32_t cardNo) {
        validate(cardNo);
        m_cardNo = cardNo;
    }

    // 构造2：传入字符串（UI输入/AT指令解析用）
    explicit BeidouCardNoParam(const QString& cardNoStr) {
        bool convertOk = false;
        uint32_t cardNo = cardNoStr.toUInt(&convertOk);
        if (!convertOk) {
            throw std::invalid_argument("设备卡号必须为数字");
        }
        validate(cardNo);
        m_cardNo = cardNo;
    }

    // 转换为下位机传输字节流（4字节大端序）
    QByteArray toBytes() const override {
        uint32_t bigEndianVal = qToBigEndian(m_cardNo);
        return QByteArray(reinterpret_cast<const char*>(&bigEndianVal), 4);
    }

    // 转换为JSON（同时包含数值和7位字符串，方便日志/UI使用）
    QJsonObject toJson() const override {
        return {
            {"card_no", static_cast<qint64>(m_cardNo)},
            {"card_no_str", to7DigitStr()} // 补零为7位字符串
        };
    }

    // 人类可读描述（显示7位字符串）
    QString description() const override {
        return QString("北斗设备卡号：%1").arg(to7DigitStr());
    }

    // 获取原始数值（供内部逻辑计算）
    uint32_t cardNo() const { return m_cardNo; }

    // 获取7位补零字符串（供UI显示/AT指令反馈）
    QString to7DigitStr() const {
        return QString("%1").arg(m_cardNo, 7, 10, QChar('0'));
    }

private:
    // 校验：必须是0-9999999的7位数字
    void validate(uint32_t cardNo) {
        if (cardNo > 9999999) {
            throw std::invalid_argument("设备卡号无效（需7位数字，0-9999999）");
        }
    }

    uint32_t m_cardNo; // 核心存储：uint32_t（统一承载目标/白名单卡号）
};

/**
 * @brief 浮调浮力体积参数（适配0~10.06L范围）
 * 核心逻辑：用uint16_t存储缩放后的整数值（1L = 1000单位），确保精度且兼容硬件传输
 * 适配命令：Buoyancy_SetExpectedValue（设置期望浮力体积）、Buoyancy_ReadCurrentValue（读取当前浮力体积）
 */
class BuoyancyVolumeParameter : public CommandParameter {
public:
    /**
     * @brief 构造函数（输入体积，自动缩放为uint16_t存储）
     * @param volumeL 浮力体积（单位：L，范围：0.00~10.06L，精度0.001L）
     */
    explicit BuoyancyVolumeParameter(double volumeL) {
        // 1. 范围校验（严格限制0~10.06L）
        if (volumeL < 0.00 || volumeL > 1006) {
            throw std::invalid_argument(QString("浮力体积超出有效范围（0.00~10.06L），当前值：%1L").arg(volumeL, 0, 'f', 3).toStdString());
        }

        // 2. 缩放转换（1L = 1000单位，uint16_t最大支持65535单位→65.535L，满足10.06L需求）
        m_scaledValue = static_cast<uint16_t>(qRound(volumeL * 1000)); // 四舍五入保留3位小数精度
    }

    /**
     * @brief 构造函数（输入硬件传输的uint16_t缩放值，反向转换为体积）
     * @param scaledValue 硬件传输的缩放值（0~10060 → 对应0.00~10.06L）
     */
    explicit BuoyancyVolumeParameter(uint16_t scaledValue) {
        // 1. 缩放值范围校验（对应0~10.06L）
        if (scaledValue > 10060) {
            throw std::invalid_argument(QString("浮力缩放值超出范围（0~10060），当前值：%1").arg(scaledValue).toStdString());
        }
        m_scaledValue = scaledValue;
    }

    /**
     * @brief 转换为硬件传输字节流（uint16_t大端序，2字节）
     */
    QByteArray toBytes() const override {
        uint16_t bigEndianVal = qToBigEndian(m_scaledValue); // 大端序适配硬件协议
        return QByteArray(reinterpret_cast<const char*>(&bigEndianVal), 2); // 深拷贝2字节
    }

    /**
     * @brief 转换为JSON对象（同时包含原始体积、缩放值，方便调试/UI展示）
     */
    QJsonObject toJson() const override {
        QJsonObject obj;
        obj["volume_L"] = volumeL();          // 实际体积（L，保留3位小数）
        obj["scaled_value"] = static_cast<uint16_t>(m_scaledValue); // 硬件传输的缩放值
        obj["range"] = "0.00~10.06L（精度0.001L）";
        return obj;
    }

    /**
     * @brief 参数描述（人类可读，直接显示体积单位L）
     */
    QString description() const override {
        return QString("浮调浮力体积：%1L（缩放值：%2）")
            .arg(volumeL(), 0, 'f', 3) // 保留3位小数，如10.060L
            .arg(m_scaledValue);
    }

    /**
     * @brief 获取实际浮力体积（单位：L）
     */
    double volumeL() const {
        return m_scaledValue / 1000.0; // 缩放值→体积（1000单位=1L）
    }

    /**
     * @brief 获取硬件传输的缩放值（uint16_t）
     */
    uint16_t scaledValue() const {
        return m_scaledValue;
    }

private:
    uint16_t m_scaledValue; // 核心存储：缩放后的整数值（1单位=0.001L）
};

class InternalCommandParam : public CommandParameter {
public:
    explicit InternalCommandParam(const QVariantMap& extraParams = {})
        : m_extraParams(extraParams) {}

    QByteArray toBytes() const override {
        QString paramStr;
        for (auto it = m_extraParams.constBegin(); it != m_extraParams.constEnd(); ++it) {
            if (!paramStr.isEmpty()) {
                paramStr += ";"; // 键值对分隔符
            }
            // 适配不同类型的值（数值/字符串/布尔）
            if (it.value().type() == QVariant::UInt || it.value().type() == QVariant::Int) {
                paramStr += QString("%1=%2").arg(it.key()).arg(it.value().toUInt());
            }
            else if (it.value().type() == QVariant::Double) {
                paramStr += QString("%1=%2").arg(it.key()).arg(it.value().toDouble(), 0, 'f', 3);
            }
            else {
                paramStr += QString("%1=%2").arg(it.key()).arg(it.value().toString());
            }
        }
        return paramStr.toUtf8();
    }
    /**
  * @brief 转换为JSON对象（包含所有额外参数）
  */
    QJsonObject toJson() const override {
        QJsonObject json;
        // 将QVariantMap转换为QJsonObject（适配常见类型）
        for (auto it = m_extraParams.constBegin(); it != m_extraParams.constEnd(); ++it) {
            if (it.value().type() == QVariant::UInt) {
                json[it.key()] = static_cast<qint64>(it.value().toUInt());
            }
            else if (it.value().type() == QVariant::Int) {
                json[it.key()] = it.value().toInt();
            }
            else if (it.value().type() == QVariant::Double) {
                json[it.key()] = it.value().toDouble();
            }
            else if (it.value().type() == QVariant::Bool) {
                json[it.key()] = it.value().toBool();
            }
            else {
                json[it.key()] = it.value().toString();
            }
        }
        return json;
    }

    /**
     * @brief 生成人类可读的参数描述
     */
    QString description() const override {
        if (m_extraParams.isEmpty()) {
            return "内部指令：无额外参数";
        }

        QString desc = "内部指令参数：";
        for (auto it = m_extraParams.constBegin(); it != m_extraParams.constEnd(); ++it) {
            desc += QString("\n  %1: %2").arg(it.key()).arg(it.value().toString());
        }
        return desc;
    }
    QVariantMap extraParams() const { return m_extraParams; }

    // 设置单个额外参数（解决E0135错误）
    void setExtraParam(const QString& key, const QVariant& value) {
        m_extraParams.insert(key, value);
    }

    // 批量设置额外参数
    void setExtraParams(const QVariantMap& params) {
        m_extraParams.unite(params);
    }
public:
    QVariantMap m_extraParams; // 存储设备专属参数（如卡号、波特率、数据内容等）
};

class ControlParamParameter : public CommandParameter {
public:
    explicit ControlParamParameter(const ControlParams& params) : m_params(params) {}

    // 序列化：按协议转为字节流（仅参数，无命令码）
    QByteArray toBytes() const override {
        return m_params.toDataSegment();
    }

    // 实现基类纯虚函数（适配JSON序列化）
    QJsonObject toJson() const override {
        QJsonObject json;
        json["control_type"] = static_cast<int>(m_params.type);

        // 按类型添加具体参数
        switch (m_params.type) {
        case ControlType::DepthControl:
            json["param_type"] = "DepthControlParams";
            json["description"] = "深度控制参数";
            break;
        case ControlType::HeightControl:
            json["param_type"] = "HeightControlParams";
            json["description"] = "高度控制参数";
            break;
        case ControlType::HeadingControl:
            json["param_type"] = "HeadingControlParams";
            json["description"] = "航向控制参数";
            break;
        default:
            json["param_type"] = "UnknownControlParams";
            json["description"] = "未知控制参数";
            break;
        }
        return json;
    }

    // 实现基类纯虚函数（人类可读描述）
    QString description() const override {
        switch (m_params.type) {
        case ControlType::DepthControl:
            return "深度控制参数";
        case ControlType::HeightControl:
            return "高度控制参数";
        case ControlType::HeadingControl:
            return "航向控制参数";
        default:
            return QString("未知控制参数（类型：%1）").arg(static_cast<int>(m_params.type));
        }
    }

    // 获取原始参数
    ControlParams getParams() const { return m_params; }

private:
    ControlParams m_params; // 控制参数组合体（深度/高度/航向）
};