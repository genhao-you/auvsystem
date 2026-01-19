#pragma once
#include <QList>
#include <QString>
#include "selfcheckenum.h"
#include "commandparameter.h"

/**
 * @brief 器件支持的命令
 */
struct SupportedCommand {
    CommandCode code;              // 命令编码
    QString name;                  // 命令名称
    std::shared_ptr<CommandParameter> defaultParam;  // 默认参数
    QString description;           // 命令描述
};

/**
 * @brief 自检器件类
 *
 * 封装一个可自检器件的所有信息，包括其支持的命令
 */
class SelfCheckDevice {
public:
    SelfCheckDevice() = default;

    SelfCheckDevice(Subsystem subsystem, DeviceId deviceId, const QString& name)
        : m_subsystem(subsystem), m_deviceId(deviceId), m_name(name) {}

    /**
     * @brief 添加支持的命令
     * @param command 命令信息
     */
    void addSupportedCommand(const SupportedCommand& command) {
        m_supportedCommands.append(command);
    }

    /**
     * @brief 检查是否支持某个命令
     * @param code 命令编码
     * @return 是否支持
     */
    bool supportsCommand(CommandCode code) const {
        for (const auto& cmd : m_supportedCommands) {
            if (cmd.code == code) {
                return true;
            }
        }
        return false;
    }

    /**
     * @brief 获取命令信息
     * @param code 命令编码
     * @return 命令信息（如果存在）
     */
    const SupportedCommand* getCommand(CommandCode code) const {
        for (const auto& cmd : m_supportedCommands) {
            if (cmd.code == code) {
                return &cmd;
            }
        }
        return nullptr;
    }

    // Getters
    Subsystem subsystem() const { return m_subsystem; }
    DeviceId deviceId() const { return m_deviceId; }
    QString name() const { return m_name; }
    const QList<SupportedCommand>& supportedCommands() const { return m_supportedCommands; }

private:
    Subsystem m_subsystem;                     // 所属子系统
    DeviceId m_deviceId;                       // 器件ID
    QString m_name;                            // 器件名称
    QList<SupportedCommand> m_supportedCommands; // 支持的命令列表
};
