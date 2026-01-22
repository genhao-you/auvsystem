#include "commandregistry.h"
#include <QDebug>

CommandRegistry& CommandRegistry::instance()
{
    static CommandRegistry instance;
    return instance;
}

void CommandRegistry::registerHandler(DeviceId deviceId, CommandCode cmdCode, CommandHandler handler)
{
    m_handlers[{deviceId, cmdCode}] = handler;
}

CommandRegistry::CommandHandler CommandRegistry::getHandler(DeviceId deviceId, CommandCode cmdCode) const
{
    return m_handlers.value({ deviceId, cmdCode }, nullptr);
}

bool CommandRegistry::handleCommand(DeviceId deviceId, CommandCode cmdCode,
    const SelfCheckResult* result, QWidget* parent)
{
    auto handler = getHandler(deviceId, cmdCode);
    if (handler) {
        handler(result, parent);
        return true;
    }
    return false;
}