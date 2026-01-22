#pragma once

#include <QObject>
#include "devicehandlerbase.h"
#include <QPushButton>
#include <QMap>
#include <QLineEdit>
#include <QLabel>
class DropWeightHandler  : public DeviceHandlerBase
{
	Q_OBJECT

public:
    explicit DropWeightHandler(SelfCheckModule* module, QWidget* parent = nullptr);
    ~DropWeightHandler() override;

    // DeviceHandlerBase 接口实现
    void handleResult(const SelfCheckResult* result) override;
    void initializeUI() override;
    void connectSignals() override;
    void disconnectSignals() override;
    QString getDeviceNumber(DeviceId deviceId) const override;
    bool supportsDevice(DeviceId deviceId) const override;

    // 抛载特定方法
    void setDeviceMapping(const QMap<DeviceId, QString>& mapping);

private:
    // 抛载UI控件结构体（匹配07前缀）
    struct DropWeightUIControls {
        QString number; // 设备编号（07）

        // 命令按钮
        QPushButton* powerOnBtn = nullptr;
        QPushButton* powerOffBtn = nullptr;
        QPushButton* releaseBtn = nullptr;
        QPushButton* resetBtn = nullptr;
        QPushButton* getFaultBtn = nullptr;

        // 状态图标
        QLabel* powerOnIcon = nullptr;
        QLabel* powerOffIcon = nullptr;
        QLabel* releaseIcon = nullptr;
        QLabel* resetIcon = nullptr;
        QLabel* getFaultIcon = nullptr;

        // 反馈文本框
        QLineEdit* powerOnFeedback = nullptr;
        QLineEdit* powerOffFeedback = nullptr;
        QLineEdit* releaseFeedback = nullptr;
        QLineEdit* resetFeedback = nullptr;
        QLineEdit* getFaultFeedback = nullptr;

        // 初始化控件（按前缀匹配）
        void init(QWidget* parent, const QString& prefix);
    };

    // 成员变量
    QMap<DeviceId, DropWeightUIControls> m_uiControls;
    QMap<DeviceId, QString> m_deviceNumberMap; // 抛载设备编号映射：DropWeight→07

    // 初始化函数
    void initDefaultDeviceMapping();
    void initAllUIControls();

    // 命令结果处理
    void processDropWeightCommand(DeviceId deviceId, CommandCode cmdCode, const SelfCheckResult* result);

private slots:
    // 按钮槽函数
    void onPowerOnClicked(DeviceId deviceId);
    void onPowerOffClicked(DeviceId deviceId);
    void onReleaseClicked(DeviceId deviceId);
    void onResetClicked(DeviceId deviceId);
    void onGetFaultClicked(DeviceId deviceId);
};
