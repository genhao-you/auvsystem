#pragma once

#include <QObject>
#include "devicehandlerbase.h"
#include <QPushButton>
#include <QMap>
#include <QLineEdit>
#include <QLabel>
class BatteryHandler  : public DeviceHandlerBase
{
	Q_OBJECT

public:
    explicit BatteryHandler(SelfCheckModule* module, QWidget* parent = nullptr);
    ~BatteryHandler() override;

    // DeviceHandlerBase 接口实现
    void handleResult(const SelfCheckResult* result) override;
    void initializeUI() override;
    void connectSignals() override;
    void disconnectSignals() override;
    QString getDeviceNumber(DeviceId deviceId) const override;
    bool supportsDevice(DeviceId deviceId) const override;

    // 电池特定方法
    void setDeviceMapping(const QMap<DeviceId, QString>& mapping);

private:
    // 电池UI控件结构体（适配10/1A/11前缀）
    struct BatteryUIControls {
        QString number; // 设备编号（10/1A/11）

        // 命令按钮
        QPushButton* getSocBtn = nullptr;
        QPushButton* getTotalVoltageBtn = nullptr;
        QPushButton* getCurrentBtn = nullptr;
        QPushButton* getAvgTempBtn = nullptr;
        QPushButton* hvOnBtn = nullptr;
        QPushButton* hvOffBtn = nullptr;
        QPushButton* lockBtn = nullptr;
        QPushButton* unlockBtn = nullptr;

        // 状态图标
        QLabel* getSocIcon = nullptr;
        QLabel* getTotalVoltageIcon = nullptr;
        QLabel* getCurrentIcon = nullptr;
        QLabel* getAvgTempIcon = nullptr;
        QLabel* hvOnIcon = nullptr;
        QLabel* hvOffIcon = nullptr;
        QLabel* lockIcon = nullptr;
        QLabel* unlockIcon = nullptr;

        // 反馈文本框
        QLineEdit* getSocFeedback = nullptr;
        QLineEdit* getTotalVoltageFeedback = nullptr;
        QLineEdit* getCurrentFeedback = nullptr;
        QLineEdit* getAvgTempFeedback = nullptr;
        QLineEdit* hvOnFeedback = nullptr;
        QLineEdit* hvOffFeedback = nullptr;
        QLineEdit* lockFeedback = nullptr;
        QLineEdit* unlockFeedback = nullptr;

        // 初始化控件（按前缀匹配）
        void init(QWidget* parent, const QString& prefix);
    };

    // 成员变量
    QMap<DeviceId, BatteryUIControls> m_uiControls;
    QMap<DeviceId, QString> m_deviceNumberMap; // 电池设备编号映射：PowerBattery1→10, PowerBattery2→1A, MeterBattery→11

    // 初始化函数
    void initDefaultDeviceMapping();
    void initAllUIControls();

    // 命令结果处理
    void processBatteryCommand(DeviceId deviceId, CommandCode cmdCode, const SelfCheckResult* result);

private slots:
    // 按钮槽函数
    void onGetSocClicked(DeviceId deviceId);
    void onGetTotalVoltageClicked(DeviceId deviceId);
    void onGetCurrentClicked(DeviceId deviceId);
    void onGetAvgTempClicked(DeviceId deviceId);
    void onHVOnClicked(DeviceId deviceId);
    void onHVOffClicked(DeviceId deviceId);
    void onLockClicked(DeviceId deviceId);
    void onUnlockClicked(DeviceId deviceId);
};
