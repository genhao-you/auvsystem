#pragma once

#include <QObject>
#include "devicehandlerbase.h"
#include <QPushButton>
#include <QMap>
#include <QLineEdit>
#include <QLabel>
class INSHandler  : public DeviceHandlerBase
{
	Q_OBJECT

public:
    explicit INSHandler(SelfCheckModule* module, QWidget* parent = nullptr);
    ~INSHandler() override;

    // DeviceHandlerBase 接口实现
    void handleResult(const SelfCheckResult* result) override;
    void initializeUI() override;
    void connectSignals() override;
    void disconnectSignals() override;
    QString getDeviceNumber(DeviceId deviceId) const override;
    bool supportsDevice(DeviceId deviceId) const override;

    // INS特定方法
    void setDeviceMapping(const QMap<DeviceId, QString>& mapping);

private:
    // INS UI控件结构体（匹配0D前缀）
    struct INSUIControls {
        QString number; // 设备编号（0D）

        // 命令按钮
        QPushButton* powerOnBtn = nullptr;
        QPushButton* powerOffBtn = nullptr;
        QPushButton* setGnssBindBtn = nullptr;
        QPushButton* calibrationWithDVLBtn = nullptr;
        QPushButton* combinedWithDVLBtn = nullptr;
        QPushButton* combinedAutoBtn = nullptr;
        QPushButton* getUtcDateBtn = nullptr;
        QPushButton* getUtcTimeBtn = nullptr;
        QPushButton* getAttitudeBtn = nullptr;
        QPushButton* getBodySpeedBtn = nullptr;
        QPushButton* getGeoSpeedBtn = nullptr;
        QPushButton* getPositionBtn = nullptr;
        QPushButton* getGyroscopeSpeedBtn = nullptr;
        QPushButton* getAccelerationBtn = nullptr;
        QPushButton* getStatusBtn = nullptr;

        // 参数输入框（GNSS绑定）
        QLineEdit* setGnssBindLonEdit = nullptr;
        QLineEdit* setGnssBindLatEdit = nullptr;

        // 状态图标
        QLabel* powerOnIcon = nullptr;
        QLabel* powerOffIcon = nullptr;
        QLabel* setGnssBindIcon = nullptr;
        QLabel* calibrationWithDVLIcon = nullptr;
        QLabel* combinedWithDVLIcon = nullptr;
        QLabel* combinedAutoIcon = nullptr;
        QLabel* getUtcDateIcon = nullptr;
        QLabel* getUtcTimeIcon = nullptr;
        QLabel* getAttitudeIcon = nullptr;
        QLabel* getBodySpeedIcon = nullptr;
        QLabel* getGeoSpeedIcon = nullptr;
        QLabel* getPositionIcon = nullptr;
        QLabel* getGyroscopeSpeedIcon = nullptr;
        QLabel* getAccelerationIcon = nullptr;
        QLabel* getStatusIcon = nullptr;

        // 反馈文本框
        QLineEdit* powerOnFeedback = nullptr;
        QLineEdit* powerOffFeedback = nullptr;
        QLineEdit* setGnssBindFeedback = nullptr;
        QLineEdit* calibrationWithDVLFeedback = nullptr;
        QLineEdit* combinedWithDVLFeedback = nullptr;
        QLineEdit* combinedAutoFeedback = nullptr;
        QLineEdit* getUtcDateFeedback = nullptr;
        QLineEdit* getUtcTimeFeedback = nullptr;
        QLineEdit* getYawFeedback = nullptr;
        QLineEdit* getPitchFeedback = nullptr;
        QLineEdit* getRollFeedback = nullptr;
        QLineEdit* getBodyXFeedback = nullptr;
        QLineEdit* getBodyYFeedback = nullptr;
        QLineEdit* getBodyZFeedback = nullptr;
        QLineEdit* getGeoXFeedback = nullptr;
        QLineEdit* getGeoYFeedback = nullptr;
        QLineEdit* getGeoZFeedback = nullptr;
        QLineEdit* getLatitudeFeedback = nullptr;
        QLineEdit* getLongitudeFeedback = nullptr;
        QLineEdit* getGyroYawFeedback = nullptr;
        QLineEdit* getGyroPitchFeedback = nullptr;
        QLineEdit* getGyroRollFeedback = nullptr;
        QLineEdit* getAccelXFeedback = nullptr;
        QLineEdit* getAccelYFeedback = nullptr;
        QLineEdit* getAccelZFeedback = nullptr;
        QLineEdit* getWorkStatusFeedback = nullptr;
        QLineEdit* getCombinationStateFeedback = nullptr;
        QLineEdit* getSelfCheckFeedback = nullptr;

        // 初始化控件（按前缀匹配）
        void init(QWidget* parent, const QString& prefix);
    };

    // 成员变量
    QMap<DeviceId, INSUIControls> m_uiControls;
    QMap<DeviceId, QString> m_deviceNumberMap;

    // 初始化函数
    void initDefaultDeviceMapping();
    void initAllUIControls();

    // 命令结果处理
    void processINSCommand(DeviceId deviceId, CommandCode cmdCode, const SelfCheckResult* result);

private slots:
    // 按钮槽函数
    void onPowerOnClicked(DeviceId deviceId);
    void onPowerOffClicked(DeviceId deviceId);
    void onSetGnssBindClicked(DeviceId deviceId);
    void onCalibrationWithDVLClicked(DeviceId deviceId);
    void onCombinedWithDVLClicked(DeviceId deviceId);
    void onCombinedAutoClicked(DeviceId deviceId);
    void onGetUtcDateClicked(DeviceId deviceId);
    void onGetUtcTimeClicked(DeviceId deviceId);
    void onGetAttitudeClicked(DeviceId deviceId);
    void onGetBodySpeedClicked(DeviceId deviceId);
    void onGetGeoSpeedClicked(DeviceId deviceId);
    void onGetPositionClicked(DeviceId deviceId);
    void onGetGyroscopeSpeedClicked(DeviceId deviceId);
    void onGetAccelerationClicked(DeviceId deviceId);
    void onGetStatusClicked(DeviceId deviceId);
};
