#pragma once

#include <QObject>
#include "devicehandlerbase.h"
#include <QPushButton>
#include <QMap>
#include <QLineEdit>
#include <QLabel>

class RudderHandler  : public DeviceHandlerBase
{
	Q_OBJECT

public:
    explicit RudderHandler(SelfCheckModule* module, QWidget* parent = nullptr);
    ~RudderHandler() override;

    // DeviceHandlerBase 接口实现
    void handleResult(const SelfCheckResult* result) override;
    void initializeUI() override;
    void connectSignals() override;
    void disconnectSignals() override;
    QString getDeviceNumber(DeviceId deviceId) const override;
    bool supportsDevice(DeviceId deviceId) const override;

    // 舵机特定方法
    void setDeviceMapping(const QMap<DeviceId, QString>& mapping);
    void addDevice(DeviceId deviceId, const QString& number);

private:
    // 单个舵机的UI控件结构体（对应推进器的ThrusterUIControls）
    struct RudderUIControls {
        QString number;

        // 命令按钮
        QPushButton* powerOnBtn = nullptr;
        QPushButton* powerOffBtn = nullptr;
        QPushButton* setAngleBtn = nullptr;
        QPushButton* getRunTimeBtn = nullptr;
        QPushButton* getVoltageBtn = nullptr;
        QPushButton* getAngleBtn = nullptr;
        QPushButton* getCurrentBtn = nullptr;
        QPushButton* getTempBtn = nullptr;
        QPushButton* setZeroBtn = nullptr;
        QPushButton* getFaultBtn = nullptr;

        // 状态图标
        QLabel* powerOnIcon = nullptr;
        QLabel* powerOffIcon = nullptr;
        QLabel* setAngleIcon = nullptr;
        QLabel* getRunTimeIcon = nullptr;
        QLabel* getVoltageIcon = nullptr;
        QLabel* getAngleIcon = nullptr;
        QLabel* getCurrentIcon = nullptr;
        QLabel* getTempIcon = nullptr;
        QLabel* getFaultIcon = nullptr;

        // 反馈文本框
        QLineEdit* powerOnFeedback = nullptr;
        QLineEdit* powerOffFeedback = nullptr;
        QLineEdit* getRunTimeFeedback = nullptr;
        QLineEdit* getVoltageFeedback = nullptr;
        QLineEdit* getAngleFeedback = nullptr;
        QLineEdit* getCurrentFeedback = nullptr;
        QLineEdit* getTempFeedback = nullptr;
        QLineEdit* getFaultFeedback = nullptr;

        // 输入文本框
        QLineEdit* targetAngleInput = nullptr;

        // 初始化所有控件（按编号前缀获取）
        void init(QWidget* parent, const QString& prefix);
    };

    // 成员变量（和推进器handler对齐）
    QMap<DeviceId, RudderUIControls> m_uiControls;
    QMap<DeviceId, QString> m_deviceNumberMap;

    // 初始化函数
    void initDefaultDeviceMapping();  // 初始化4个舵机的默认编号映射
    void initAllUIControls();         // 初始化所有舵机的UI控件

    // 命令处理函数
    void processRudderCommand(DeviceId deviceId, CommandCode cmdCode, const SelfCheckResult* result);

    // 特定命令结果处理
    void handlePowerCommand(const RudderUIControls& controls, CommandCode cmdCode, const SelfCheckResult* result);
    void handleSetAngleCommand(const RudderUIControls& controls, const SelfCheckResult* result);
    void handleGetRunTimeCommand(const RudderUIControls& controls, const SelfCheckResult* result);
    void handleGetVoltageCommand(const RudderUIControls& controls, const SelfCheckResult* result);
    void handleGetAngleCommand(const RudderUIControls& controls, const SelfCheckResult* result);
    void handleGetCurrentCommand(const RudderUIControls& controls, const SelfCheckResult* result);
    void handleGetTempCommand(const RudderUIControls& controls, const SelfCheckResult* result);
    void handleGetFaultCommand(const RudderUIControls& controls, const SelfCheckResult* result);

    // 参数创建辅助函数（封装参数验证逻辑）
    std::shared_ptr<RudderAngleParameter> createAngleParameter(const QString& inputText, const QString& paramName) const;

    // 输入验证辅助函数
    bool validateAngleInput(const QString& inputText, QString& errorMsg, double& angleDegree) const;

private slots:
    // 按钮点击槽函数（按DeviceId区分不同舵机）
    void onPowerOnClicked(DeviceId deviceId);
    void onPowerOffClicked(DeviceId deviceId);
    void onSetAngleClicked(DeviceId deviceId, const RudderUIControls& controls);
    void onGetRunTimeClicked(DeviceId deviceId);
    void onGetVoltageClicked(DeviceId deviceId);
    void onGetAngleClicked(DeviceId deviceId);
    void onGetCurrentClicked(DeviceId deviceId);
    void onGetTempClicked(DeviceId deviceId);
    void onSetZeroClicked(DeviceId deviceId);
    void onGetFaultClicked(DeviceId deviceId);
};
