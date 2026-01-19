#pragma once
#include <QByteArray>
#include <cstdint>

/**
 * @brief CRC计算器
 *
 * 实现协议规定的CRC8计算：
 * - 多项式：0x07
 * - 初值：0x00
 * - 输入输出不反转
 * - 余数不异或
 */
class CrcCalculator {
public:
    /**
     * @brief 计算CRC8校验值
     * @param data 待计算的数据
     * @return 计算得到的CRC8值
     */
    static uint8_t calculateCrc8(const QByteArray& data);
    static uint8_t crc8(const QByteArray& data);

private:
    // 预计算CRC8表，提高计算效率
    static const uint8_t crc8Table[256];

    // 初始化CRC表（仅内部使用）
    static void initCrc8Table();

    // 静态初始化，确保CRC表只初始化一次
    class Initializer {
    public:
        Initializer() {
            CrcCalculator::initCrc8Table();
        }
    };
    static Initializer initializer;
};