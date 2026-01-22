#pragma once

#include <QObject>
#include "devicehandlerbase.h"
#include <QPushButton>
#include <QMap>
#include <QLineEdit>
#include <QLabel>
class CameraHandler  : public DeviceHandlerBase
{
	Q_OBJECT

public:
    explicit CameraHandler(SelfCheckModule* module, QWidget* parent = nullptr);
    ~CameraHandler() override;

    // DeviceHandlerBase 接口实现
    void handleResult(const SelfCheckResult* result) override;
    void initializeUI() override;
    void connectSignals() override;
    void disconnectSignals() override;
    QString getDeviceNumber(DeviceId deviceId) const override;
    bool supportsDevice(DeviceId deviceId) const override;

    // 摄像头特定方法
    void setDeviceMapping(const QMap<DeviceId, QString>& mapping);
    // 获取下一个媒体序列（复用原有逻辑）
    int getNextCameraMediaSeq();

private:
    // 摄像头UI控件结构体（匹配0F前缀）
    struct CameraUIControls {
        QString number; // 设备编号（0F）

        // 命令按钮
        QPushButton* powerOnBtn = nullptr;
        QPushButton* powerOffBtn = nullptr;
        QPushButton* startRecordBtn = nullptr;
        QPushButton* stopRecordBtn = nullptr;
        QPushButton* takePhotoBtn = nullptr;

        // 状态图标
        QLabel* powerOnIcon = nullptr;
        QLabel* powerOffIcon = nullptr;
        QLabel* startRecordIcon = nullptr;
        QLabel* stopRecordIcon = nullptr;
        QLabel* takePhotoIcon = nullptr;

        // 反馈文本框
        QLineEdit* powerOnFeedback = nullptr;
        QLineEdit* powerOffFeedback = nullptr;
        QLineEdit* startRecordFeedback = nullptr;
        QLineEdit* stopRecordFeedback = nullptr;
        QLineEdit* takePhotoFeedback = nullptr;

        // 初始化控件（按前缀匹配）
        void init(QWidget* parent, const QString& prefix);
    };

    // 成员变量
    QMap<DeviceId, CameraUIControls> m_uiControls;
    QMap<DeviceId, QString> m_deviceNumberMap;
    int m_mediaSeq = 0; // 媒体序列计数器

    // 初始化函数
    void initDefaultDeviceMapping();
    void initAllUIControls();

    // 命令结果处理
    void processCameraCommand(DeviceId deviceId, CommandCode cmdCode, const SelfCheckResult* result);

private slots:
    // 按钮槽函数
    void onPowerOnClicked(DeviceId deviceId);
    void onPowerOffClicked(DeviceId deviceId);
    void onStartRecordClicked(DeviceId deviceId);
    void onStopRecordClicked(DeviceId deviceId);
    void onTakePhotoClicked(DeviceId deviceId);
};
