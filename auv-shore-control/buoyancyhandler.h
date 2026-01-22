#pragma once

#include <QObject>
#include <QObject>
#include "devicehandlerbase.h"
#include <QPushButton>
#include <QMap>
#include <QLineEdit>
#include <QLabel>

class BuoyancyHandler  : public DeviceHandlerBase
{
	Q_OBJECT

public:
    explicit BuoyancyHandler(SelfCheckModule* module, QWidget* parent = nullptr);
    ~BuoyancyHandler() override;

    // DeviceHandlerBase 接口实现
    void handleResult(const SelfCheckResult* result) override;
    void initializeUI() override;
    void connectSignals() override;
    void disconnectSignals() override;
    QString getDeviceNumber(DeviceId deviceId) const override;
    bool supportsDevice(DeviceId deviceId) const override;

    // 浮力调节特定方法
    void setDeviceMapping(const QMap<DeviceId, QString>& mapping);

private:
    // 浮力调节UI控件结构体（匹配06前缀）
    struct BuoyancyUIControls {
        QString number; // 设备编号（06）

        // 命令按钮
        QPushButton* powerOnBtn = nullptr;
        QPushButton* powerOffBtn = nullptr;
        QPushButton* getBuoyancyValueBtn = nullptr;
        QPushButton* setBuoyancyValueBtn = nullptr;
        QPushButton* stopBtn = nullptr;

        // 参数输入框
        QLineEdit* setBuoyancyValueInput = nullptr;

        // 状态图标
        QLabel* powerOnIcon = nullptr;
        QLabel* powerOffIcon = nullptr;
        QLabel* getBuoyancyValueIcon = nullptr;
        QLabel* setBuoyancyValueIcon = nullptr;
        QLabel* stopIcon = nullptr;

        // 反馈文本框
        QLineEdit* powerOnFeedback = nullptr;
        QLineEdit* powerOffFeedback = nullptr;
        QLineEdit* getBuoyancyValueFeedback = nullptr;
        QLineEdit* setBuoyancyValueFeedback = nullptr;
        QLineEdit* stopFeedback = nullptr;

        // 初始化控件（按前缀匹配）
        void init(QWidget* parent, const QString& prefix);
    };

    // 成员变量
    QMap<DeviceId, BuoyancyUIControls> m_uiControls;
    QMap<DeviceId, QString> m_deviceNumberMap;
    const double MAX_BUOYANCY_VALUE = 10.06; // 最大浮力值常量

    // 初始化函数
    void initDefaultDeviceMapping();
    void initAllUIControls();

    // 命令结果处理
    void processBuoyancyCommand(DeviceId deviceId, CommandCode cmdCode, const SelfCheckResult* result);

private slots:
    // 按钮槽函数
    void onPowerOnClicked(DeviceId deviceId);
    void onPowerOffClicked(DeviceId deviceId);
    void onGetBuoyancyValueClicked(DeviceId deviceId);
    void onSetBuoyancyValueClicked(DeviceId deviceId);
    void onStopClicked(DeviceId deviceId);
};
