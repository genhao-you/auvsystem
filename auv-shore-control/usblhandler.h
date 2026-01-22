#pragma once

#include <QObject>
#include "devicehandlerbase.h"
#include <QPushButton>
#include <QMap>
#include <QLineEdit>
#include <QLabel>
class USBLHandler  : public DeviceHandlerBase
{
	Q_OBJECT

public:
    explicit USBLHandler(SelfCheckModule* module, QWidget* parent = nullptr);
    ~USBLHandler() override;

    // DeviceHandlerBase 接口实现
    void handleResult(const SelfCheckResult* result) override;
    void initializeUI() override;
    void connectSignals() override;
    void disconnectSignals() override;
    QString getDeviceNumber(DeviceId deviceId) const override;
    bool supportsDevice(DeviceId deviceId) const override;

    // USBL特定方法
    void setDeviceMapping(const QMap<DeviceId, QString>& mapping);

private:
    // USBL UI控件结构体
    struct USBLUIControls {
        QString number; // 设备编号（0C）

        // 命令按钮
        QPushButton* powerOnBtn = nullptr;
        QPushButton* powerOffBtn = nullptr;

        // 状态图标
        QLabel* powerOnIcon = nullptr;
        QLabel* powerOffIcon = nullptr;

        // 反馈文本框
        QLineEdit* powerOnFeedback = nullptr;
        QLineEdit* powerOffFeedback = nullptr;

        // 初始化控件
        void init(QWidget* parent, const QString& prefix);
    };

    // 成员变量
    QMap<DeviceId, USBLUIControls> m_uiControls;
    QMap<DeviceId, QString> m_deviceNumberMap;

    // 初始化函数
    void initDefaultDeviceMapping();
    void initAllUIControls();

    // 命令结果处理
    void processUSBLCommand(DeviceId deviceId, CommandCode cmdCode, const SelfCheckResult* result);

private slots:
    // 按钮槽函数
    void onPowerOnClicked(DeviceId deviceId);
    void onPowerOffClicked(DeviceId deviceId);
};
