#pragma once
#include<QByteArray>
#include<qDebug>
#include"selfcheckenum.h"
#include"commandparameter.h"
#include"selfcheckresult.h"
// 前置声明（解决模板类型识别问题）
class InternalProtocolAdapter;

// 1. 先声明智能指针类型（需在类定义后，但前置声明可解决顺序问题）
using InternalAdapterPtr = std::shared_ptr<InternalProtocolAdapter>;

// 内部协议适配器接口（所有内部设备需实现此接口）
class InternalProtocolAdapter {
public:
    virtual ~InternalProtocolAdapter() = default;

    // 1. 生成设备对应的内部指令字节流（如AT指令、USBL专属指令）
    virtual QByteArray generateCommand(CommandCode cmdCode, const std::shared_ptr<CommandParameter>& param) = 0;

    // 2. 解析设备返回的内部指令反馈
    virtual SelfCheckResult parseResponse(CommandCode cmdCode, const QByteArray& response) = 0;

    // 3. 获取适配器支持的设备ID
    virtual DeviceId supportedDeviceId() const = 0;
};

// 内部指令管理器（单例，统一管理所有适配器）
class InternalCommandManager {
public:
    static InternalCommandManager& instance() {
        static InternalCommandManager manager;
        return manager;
    }

    // 修复：参数改为shared_ptr（与m_adapters类型一致）
    void registerAdapter(InternalAdapterPtr adapter) {
        if (adapter) {
            DeviceId deviceId = adapter->supportedDeviceId();
            m_adapters[deviceId] = adapter; // shared_ptr支持拷贝赋值，无错误
            qDebug() << "注册内部指令适配器：" << EnumConverter::deviceIdToString(deviceId);
        }
    }

    // 重载：兼容unique_ptr入参（自动转换为shared_ptr）
    void registerAdapter(std::unique_ptr<InternalProtocolAdapter> adapter) {
        if (adapter) {
            // 将unique_ptr转换为shared_ptr（转移所有权）
            registerAdapter(InternalAdapterPtr(std::move(adapter)));
        }
    }

    // 获取设备对应的适配器（不存在返回nullptr）
    InternalProtocolAdapter* getAdapter(DeviceId deviceId) {
        // 修复：使用value避免创建空元素，contains判断存在性
        if (m_adapters.contains(deviceId)) {
            return m_adapters.value(deviceId).get();
        }
        return nullptr;
    }

private:
    InternalCommandManager() = default;
    QMap<DeviceId, InternalAdapterPtr> m_adapters; // 设备-适配器映射（shared_ptr类型）
};

