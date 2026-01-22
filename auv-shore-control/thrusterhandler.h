#pragma once

#include <QObject>
#include "devicehandlerbase.h"
#include <QPushButton>
#include <QMap>
#include <QLineEdit>
// 前向声明
class SpeedParameter;
class ThrusterDutyParameter;
class ThrusterRunTimeParameter;
class AccelTimeParameter;
class ThrusterSpeedAndRunTimeParameter;
class EmptyParameter;
class SelfCheckResult;
class SelfCheckModule;

class ThrusterHandler  : public DeviceHandlerBase
{
	Q_OBJECT

public:
    explicit ThrusterHandler(SelfCheckModule* module, QWidget* parent = nullptr);
    ~ThrusterHandler() override;

    // DeviceHandlerBase 接口实现
    void handleResult(const SelfCheckResult* result) override;
    void initializeUI() override;
    void connectSignals() override;
    void disconnectSignals() override;
    QString getDeviceNumber(DeviceId deviceId) const override;
    bool supportsDevice(DeviceId deviceId) const override;

    // 推进器特定方法
    void setDeviceMapping(const QMap<DeviceId, QString>& mapping);
    void addDevice(DeviceId deviceId, const QString& number);

private:
    // UI控件组
    struct ThrusterUIControls {
        QString number;

        // 命令按钮
        QPushButton* powerOnBtn = nullptr;
        QPushButton* powerOffBtn = nullptr;
        QPushButton* paramResetBtn = nullptr;
        QPushButton* enableWorkBtn = nullptr;
        QPushButton* setSpeedBtn = nullptr;
        QPushButton* setDutyBtn = nullptr;
        QPushButton* setAccelTimeBtn = nullptr;
        QPushButton* setRunTimeBtn = nullptr;
        QPushButton* getRunTimeBtn = nullptr;
        QPushButton* getDutyBtn = nullptr;
        QPushButton* getCurrentBtn = nullptr;
        QPushButton* getTempBtn = nullptr;
        QPushButton* getSpeedBtn = nullptr;
        QPushButton* getHallBtn = nullptr;
        QPushButton* getVoltageBtn = nullptr;
        QPushButton* getNoMaintainTimeBtn = nullptr;
        QPushButton* getTotalRunTimeBtn = nullptr;
        QPushButton* setSpeedWithTimeBtn = nullptr;

        // 状态图标
        QLabel* powerOnIcon = nullptr;
        QLabel* powerOffIcon = nullptr;
        QLabel* paramResetIcon = nullptr;
        QLabel* enableWorkIcon = nullptr;
        QLabel* setSpeedIcon = nullptr;
        QLabel* setDutyIcon = nullptr;
        QLabel* setAccelTimeIcon = nullptr;
        QLabel* setRunTimeIcon = nullptr;
        QLabel* getRunTimeIcon = nullptr;
        QLabel* getDutyIcon = nullptr;
        QLabel* getCurrentIcon = nullptr;
        QLabel* getTempIcon = nullptr;
        QLabel* getSpeedIcon = nullptr;
        QLabel* getHallIcon = nullptr;
        QLabel* getVoltageIcon = nullptr;
        QLabel* getNoMaintainTimeIcon = nullptr;
        QLabel* getTotalRunTimeIcon = nullptr;
        QLabel* setSpeedWithTimeIcon = nullptr;

        // 反馈文本框
        QLineEdit* powerOnFeedback = nullptr;
        QLineEdit* powerOffFeedback = nullptr;
        QLineEdit* paramResetFeedback = nullptr;
        QLineEdit* enableWorkFeedback = nullptr;
        QLineEdit* setAccelTimeFeedback = nullptr;
        QLineEdit* setSpeedWithTimeFeedback = nullptr;
        QLineEdit* getRunTimeFeedback = nullptr;
        QLineEdit* getDutyFeedback = nullptr;
        QLineEdit* getCurrentFeedback = nullptr;
        QLineEdit* getTempFeedback = nullptr;
        QLineEdit* getSpeedFeedback = nullptr;
        QLineEdit* getHallFeedback = nullptr;
        QLineEdit* getVoltageFeedback = nullptr;
        QLineEdit* getNoMaintainTimeFeedback = nullptr;
        QLineEdit* getTotalRunTimeFeedback = nullptr;

        // 输入文本框
        QLineEdit* targetSpeedInput = nullptr;
        QLineEdit* targetDutyInput = nullptr;
        QLineEdit* accelTimeInput = nullptr;
        QLineEdit* setRunTimeInput = nullptr;
        QLineEdit* speedInput = nullptr;
        QLineEdit* timeInput = nullptr;

        // 初始化所有控件
        void init(QWidget* parent, const QString& prefix);
    };

    QMap<DeviceId, ThrusterUIControls> m_uiControls;
    QMap<DeviceId, QString> m_deviceNumberMap;

    // 初始化
    void initDefaultDeviceMapping();
    void initAllUIControls();

    // 命令处理
    void processThrusterCommand(DeviceId deviceId, CommandCode cmdCode,
        const SelfCheckResult* result);

    // 特定命令处理
    void handlePowerCommand(const ThrusterUIControls& controls, CommandCode cmdCode,
        const SelfCheckResult* result);
    void handleSpeedCommand(const ThrusterUIControls& controls, const SelfCheckResult* result);
    void handleDutyCommand(const ThrusterUIControls& controls, const SelfCheckResult* result);
    void handleCurrentCommand(const ThrusterUIControls& controls, const SelfCheckResult* result);
    void handleTemperatureCommand(const ThrusterUIControls& controls, const SelfCheckResult* result);
    void handleVoltageCommand(const ThrusterUIControls& controls, const SelfCheckResult* result);
    void handleRunTimeCommand(const ThrusterUIControls& controls, const SelfCheckResult* result);
    void handleHallCommand(const ThrusterUIControls& controls, const SelfCheckResult* result);
    void handleNoMaintainTimeCommand(const ThrusterUIControls& controls, const SelfCheckResult* result);
    void handleTotalRunTimeCommand(const ThrusterUIControls& controls, const SelfCheckResult* result);
    void handleSetSpeedWithTimeCommand(const ThrusterUIControls& controls, const SelfCheckResult* result);

    // 参数创建辅助函数
    std::shared_ptr<SpeedParameter> createSpeedParameter(const QString& inputText,
        const QString& paramName) const;
    std::shared_ptr<ThrusterDutyParameter> createDutyParameter(const QString& inputText,
        const QString& paramName) const;
    std::shared_ptr<ThrusterRunTimeParameter> createRunTimeParameter(const QString& inputText,
        const QString& paramName) const;
    std::shared_ptr<AccelTimeParameter> createAccelTimeParameter(const QString& inputText,
        const QString& paramName) const;
    std::shared_ptr<ThrusterSpeedAndRunTimeParameter> createSpeedAndTimeParameter(
        const QString& speedInput, const QString& timeInput, const QString& paramName) const;

    // 验证辅助函数
    bool validateInput(const QString& inputText, const QString& paramName,
        int minVal, int maxVal, QString& errorMsg) const;

private slots:
    void onPowerOnClicked(DeviceId deviceId);
    void onPowerOffClicked(DeviceId deviceId);
    void onParamResetClicked(DeviceId deviceId);
    void onEnableWorkClicked(DeviceId deviceId);
    void onSetSpeedClicked(DeviceId deviceId, const ThrusterUIControls& controls);
    void onSetDutyClicked(DeviceId deviceId, const ThrusterUIControls& controls);
    void onSetAccelTimeClicked(DeviceId deviceId, const ThrusterUIControls& controls);
    void onSetRunTimeClicked(DeviceId deviceId, const ThrusterUIControls& controls);
    void onSetSpeedWithTimeClicked(DeviceId deviceId, const ThrusterUIControls& controls);
    void onGetRunTimeClicked(DeviceId deviceId);
    void onGetDutyClicked(DeviceId deviceId);
    void onGetCurrentClicked(DeviceId deviceId);
    void onGetTempClicked(DeviceId deviceId);
    void onGetSpeedClicked(DeviceId deviceId);
    void onGetHallClicked(DeviceId deviceId);
    void onGetVoltageClicked(DeviceId deviceId);
    void onGetNoMaintainTimeClicked(DeviceId deviceId);
    void onGetTotalRunTimeClicked(DeviceId deviceId);

};
