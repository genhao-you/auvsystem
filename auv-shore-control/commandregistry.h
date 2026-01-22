#pragma once
#include <QMap>
#include <QPair>
#include <functional>
#include "selfcheckenum.h"
#include "selfcheckresult.h"
class QWidget;

class CommandRegistry 
{
public:
    using CommandHandler = std::function<void(const SelfCheckResult*, QWidget*)>;

    static CommandRegistry& instance();

    // 注册命令处理器
    void registerHandler(DeviceId deviceId, CommandCode cmdCode, CommandHandler handler);

    // 获取命令处理器
    CommandHandler getHandler(DeviceId deviceId, CommandCode cmdCode) const;

    // 执行命令处理
    bool handleCommand(DeviceId deviceId, CommandCode cmdCode,
        const SelfCheckResult* result, QWidget* parent);

private:
    CommandRegistry() = default;
    ~CommandRegistry() = default;

    QMap<QPair<DeviceId, CommandCode>, CommandHandler> m_handlers;
};

// 注册宏，简化使用
#define REGISTER_COMMAND_HANDLER(deviceId, cmdCode, handler) \
    CommandRegistry::instance().registerHandler(deviceId, cmdCode, handler)
