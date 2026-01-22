#pragma once

#include <QObject>
#include "devicehandlerbase.h"
#include <QPushButton>
#include <QMap>
#include <QLineEdit>
#include <QLabel>

class DepthMeterHandler  : public DeviceHandlerBase
{
	Q_OBJECT

public:
    explicit DepthMeterHandler(SelfCheckModule* module, QWidget* parent = nullptr);
    ~DepthMeterHandler() override;

    // DeviceHandlerBase 接口实现
    void handleResult(const SelfCheckResult* result) override;
    void initializeUI() override;
    void connectSignals() override;
    void disconnectSignals() override;
    QString getDeviceNumber(DeviceId deviceId) const override;
    bool supportsDevice(DeviceId deviceId) const override;

    // 深度计特定方法
    void setDeviceMapping(const QMap<DeviceId, QString>& mapping);

private:
    // 深度计UI控件结构体
    struct DepthMeterUIControls {
        QString number; // 设备编号（0A）

        // 命令按钮
        QPushButton* powerOnBtn = nullptr;
        QPushButton* powerOffBtn = nullptr;
        QPushButton* calibrateBtn = nullptr;
        QPushButton* getDepthBtn = nullptr;

        // 状态图标
        QLabel* powerOnIcon = nullptr;
        QLabel* powerOffIcon = nullptr;
        QLabel* calibrateIcon = nullptr;
        QLabel* getDepthIcon = nullptr;

        // 反馈文本框
        QLineEdit* powerOnFeedback = nullptr;
        QLineEdit* powerOffFeedback = nullptr;
        QLineEdit* calibrateFeedback = nullptr;
        QLineEdit* getDepthFeedback = nullptr;

        // 初始化控件（按前缀匹配）
        void init(QWidget* parent, const QString& prefix);
    };

    // 成员变量
    QMap<DeviceId, DepthMeterUIControls> m_uiControls;
    QMap<DeviceId, QString> m_deviceNumberMap;

    // 初始化函数
    void initDefaultDeviceMapping();
    void initAllUIControls();

    // 命令结果处理
    void processDepthMeterCommand(DeviceId deviceId, CommandCode cmdCode, const SelfCheckResult* result);

private slots:
    // 按钮槽函数
    void onPowerOnClicked(DeviceId deviceId);
    void onPowerOffClicked(DeviceId deviceId);
    void onCalibrateClicked(DeviceId deviceId);
    void onGetDepthClicked(DeviceId deviceId);
};
