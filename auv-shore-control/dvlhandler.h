#pragma once

#include <QObject>
#include "devicehandlerbase.h"
#include <QPushButton>
#include <QMap>
#include <QLineEdit>
#include <QLabel>

class DvlHandler  : public DeviceHandlerBase
{
	Q_OBJECT

public:
    explicit DvlHandler(SelfCheckModule* module, QWidget* parent = nullptr);
    ~DvlHandler() override;

    // DeviceHandlerBase 接口实现
    void handleResult(const SelfCheckResult* result) override;
    void initializeUI() override;
    void connectSignals() override;
    void disconnectSignals() override;
    QString getDeviceNumber(DeviceId deviceId) const override;
    bool supportsDevice(DeviceId deviceId) const override;

    // DVL特定方法
    void setDeviceMapping(const QMap<DeviceId, QString>& mapping);

private:
    // DVL UI控件结构体（匹配0E前缀）
    struct DvlUIControls {
        QString number; // 设备编号（0E）

        // 命令按钮
        QPushButton* powerOnBtn = nullptr;
        QPushButton* powerOffBtn = nullptr;
        QPushButton* getBottomSpeedBtn = nullptr;
        QPushButton* getWaterSpeedBtn = nullptr;
        QPushButton* getStatusBtn = nullptr;

        // 状态图标
        QLabel* powerOnIcon = nullptr;
        QLabel* powerOffIcon = nullptr;
        QLabel* getBottomSpeedIcon = nullptr;
        QLabel* getWaterSpeedIcon = nullptr;
        QLabel* getStatusIcon = nullptr;

        // 反馈文本框
        QLineEdit* powerOnFeedback = nullptr;
        QLineEdit* powerOffFeedback = nullptr;
        // 底速三轴反馈
        QLineEdit* getBottomXFeedback = nullptr;
        QLineEdit* getBottomYFeedback = nullptr;
        QLineEdit* getBottomZFeedback = nullptr;
        // 水速三轴反馈
        QLineEdit* getWaterXFeedback = nullptr;
        QLineEdit* getWaterYFeedback = nullptr;
        QLineEdit* getWaterZFeedback = nullptr;
        // 状态反馈
        QLineEdit* selfCheckCardFeedback = nullptr;
        QLineEdit* outputStateFeedback = nullptr;

        // 初始化控件（按前缀匹配）
        void init(QWidget* parent, const QString& prefix);
    };

    // 成员变量
    QMap<DeviceId, DvlUIControls> m_uiControls;
    QMap<DeviceId, QString> m_deviceNumberMap; // DVL设备编号映射：DVL→0E

    // 初始化函数
    void initDefaultDeviceMapping();
    void initAllUIControls();

    // 命令结果处理
    void processDvlCommand(DeviceId deviceId, CommandCode cmdCode, const SelfCheckResult* result);

private slots:
    // 按钮槽函数
    void onPowerOnClicked(DeviceId deviceId);
    void onPowerOffClicked(DeviceId deviceId);
    void onGetBottomSpeedClicked(DeviceId deviceId);
    void onGetWaterSpeedClicked(DeviceId deviceId);
    void onGetStatusClicked(DeviceId deviceId);
};
