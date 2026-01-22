#pragma once

#include <QObject>
#include <QObject>
#include "devicehandlerbase.h"
#include <QPushButton>
#include <QMap>
#include <QLineEdit>
#include <QLabel>
class AcousticCommHandler  : public DeviceHandlerBase
{
	Q_OBJECT

public:
    explicit AcousticCommHandler(SelfCheckModule* module, QWidget* parent = nullptr);
    ~AcousticCommHandler() override;

    // DeviceHandlerBase 接口实现
    void handleResult(const SelfCheckResult* result) override;
    void initializeUI() override;
    void connectSignals() override;
    void disconnectSignals() override;
    QString getDeviceNumber(DeviceId deviceId) const override;
    bool supportsDevice(DeviceId deviceId) const override;

    // 水声通信特定方法
    void setDeviceMapping(const QMap<DeviceId, QString>& mapping);

private:
    // 水声通信UI控件结构体（匹配15前缀）
    struct AcousticCommUIControls {
        QString number; // 设备编号（15）

        // 命令按钮
        QPushButton* powerOnBtn = nullptr;
        QPushButton* powerOffBtn = nullptr;
        QPushButton* selfCheckBtn = nullptr;
        QPushButton* commTestBtn = nullptr;

        // 状态图标
        QLabel* powerOnIcon = nullptr;
        QLabel* powerOffIcon = nullptr;
        QLabel* selfCheckIcon = nullptr;
        QLabel* commTestIcon = nullptr;

        // 反馈文本框
        QLineEdit* powerOnFeedback = nullptr;
        QLineEdit* powerOffFeedback = nullptr;
        QLineEdit* selfCheckFeedback = nullptr;
        QLineEdit* commTestFeedback = nullptr;
        QLineEdit* sendResultFeedback = nullptr;
        QLineEdit* receiveResultFeedback = nullptr;

        // 数据显示框
        QLineEdit* sendDataLe = nullptr;    // 发送数据
        QLineEdit* recvDataLe = nullptr;    // 接收数据

        // 初始化控件（按前缀匹配）
        void init(QWidget* parent, const QString& prefix);
    };

    // 成员变量
    QMap<DeviceId, AcousticCommUIControls> m_uiControls;
    QMap<DeviceId, QString> m_deviceNumberMap; // 水声通信设备编号映射：AcousticComm→15

    // 初始化函数
    void initDefaultDeviceMapping();
    void initAllUIControls();

    // 命令结果处理
    void processAcousticCommCommand(DeviceId deviceId, CommandCode cmdCode, const SelfCheckResult* result);

private slots:
    // 按钮槽函数
    void onPowerOnClicked(DeviceId deviceId);
    void onPowerOffClicked(DeviceId deviceId);
    void onSelfCheckClicked(DeviceId deviceId);
    void onCommTestClicked(DeviceId deviceId);
};
