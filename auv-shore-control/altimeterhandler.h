#pragma once

#include <QObject>
#include "devicehandlerbase.h"
#include <QPushButton>
#include <QMap>
#include <QLineEdit>
#include <QLabel>

class AltimeterHandler  : public DeviceHandlerBase
{
	Q_OBJECT

public:
    explicit AltimeterHandler(SelfCheckModule* module, QWidget* parent = nullptr);
    ~AltimeterHandler() override;

    // DeviceHandlerBase 接口实现
    void handleResult(const SelfCheckResult* result) override;
    void initializeUI() override;
    void connectSignals() override;
    void disconnectSignals() override;
    QString getDeviceNumber(DeviceId deviceId) const override;
    bool supportsDevice(DeviceId deviceId) const override;

    // 高度计特定方法
    void setDeviceMapping(const QMap<DeviceId, QString>& mapping);

private:
    // 高度计UI控件结构体
    struct AltimeterUIControls {
        QString number; // 设备编号（0B）

        // 命令按钮
        QPushButton* powerOnBtn = nullptr;
        QPushButton* powerOffBtn = nullptr;
        QPushButton* getHeightBtn = nullptr;

        // 状态图标
        QLabel* powerOnIcon = nullptr;
        QLabel* powerOffIcon = nullptr;
        QLabel* getHeightIcon = nullptr;

        // 反馈文本框
        QLineEdit* powerOnFeedback = nullptr;
        QLineEdit* powerOffFeedback = nullptr;
        QLineEdit* getHeightFeedback = nullptr;

        // 初始化控件
        void init(QWidget* parent, const QString& prefix);
    };

    // 成员变量
    QMap<DeviceId, AltimeterUIControls> m_uiControls;
    QMap<DeviceId, QString> m_deviceNumberMap;

    // 初始化函数
    void initDefaultDeviceMapping();
    void initAllUIControls();

    // 命令结果处理
    void processAltimeterCommand(DeviceId deviceId, CommandCode cmdCode, const SelfCheckResult* result);

private slots:
    // 按钮槽函数
    void onPowerOnClicked(DeviceId deviceId);
    void onPowerOffClicked(DeviceId deviceId);
    void onGetHeightClicked(DeviceId deviceId);
};
