#pragma once

#include <QObject>
#include "devicehandlerbase.h"
#include <QPushButton>
#include <QMap>
#include <QLineEdit>
#include <QLabel>

class SonarHandler  : public DeviceHandlerBase
{
	Q_OBJECT

public:
    explicit SonarHandler(SelfCheckModule* module, QWidget* parent = nullptr);
    ~SonarHandler() override;

    // DeviceHandlerBase 接口实现
    void handleResult(const SelfCheckResult* result) override;
    void initializeUI() override;
    void connectSignals() override;
    void disconnectSignals() override;
    QString getDeviceNumber(DeviceId deviceId) const override;
    bool supportsDevice(DeviceId deviceId) const override;

    // 声呐特定方法
    void setDeviceMapping(const QMap<DeviceId, QString>& mapping);

private:
    // 声呐UI控件结构体（适配12/13/14前缀）
    struct SonarUIControls {
        QString number; // 设备编号（12/13/14）

        // 命令按钮
        QPushButton* powerOnBtn = nullptr;
        QPushButton* powerOffBtn = nullptr;
        QPushButton* enableWorkBtn = nullptr;
        QPushButton* disableWorkBtn = nullptr;

        // 状态图标
        QLabel* powerOnIcon = nullptr;
        QLabel* powerOffIcon = nullptr;
        QLabel* enableWorkIcon = nullptr;
        QLabel* disableWorkIcon = nullptr;

        // 反馈文本框
        QLineEdit* powerOnFeedback = nullptr;
        QLineEdit* powerOffFeedback = nullptr;
        QLineEdit* enableWorkFeedback = nullptr;
        QLineEdit* disableWorkFeedback = nullptr;

        // 初始化控件（按前缀匹配）
        void init(QWidget* parent, const QString& prefix);
    };

    // 成员变量
    QMap<DeviceId, SonarUIControls> m_uiControls;
    QMap<DeviceId, QString> m_deviceNumberMap; // 声呐设备编号映射：SideScan(12)/Forward(13)/Downward(14)

    // 初始化函数
    void initDefaultDeviceMapping();
    void initAllUIControls();

    // 命令结果处理
    void processSonarCommand(DeviceId deviceId, CommandCode cmdCode, const SelfCheckResult* result);

private slots:
    // 按钮槽函数
    void onPowerOnClicked(DeviceId deviceId);
    void onPowerOffClicked(DeviceId deviceId);
    void onEnableWorkClicked(DeviceId deviceId);
    void onDisableWorkClicked(DeviceId deviceId);
};
