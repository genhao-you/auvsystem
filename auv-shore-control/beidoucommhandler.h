#pragma once

#include <QObject>
#include "devicehandlerbase.h"
#include <QPushButton>
#include <QMap>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
class BeidouCommHandler  : public DeviceHandlerBase
{
	Q_OBJECT

public:
    explicit BeidouCommHandler(SelfCheckModule* module, QWidget* parent = nullptr);
    ~BeidouCommHandler() override;

    // DeviceHandlerBase 接口实现
    void handleResult(const SelfCheckResult* result) override;
    void initializeUI() override;
    void connectSignals() override;
    void disconnectSignals() override;
    QString getDeviceNumber(DeviceId deviceId) const override;
    bool supportsDevice(DeviceId deviceId) const override;

    // 北斗通信特定方法
    void setDeviceMapping(const QMap<DeviceId, QString>& mapping);

private:
    // 北斗通信UI控件结构体（匹配09前缀，含内外两套）
    struct BeidouCommUIControls {
        QString number; // 设备编号（09）

        // ========== 外部控件（无_In后缀） ==========
        // 按钮
        QPushButton* readSignalQualityBtn = nullptr;
        QPushButton* readIdentityBtn = nullptr;
        QPushButton* getTargetBtn = nullptr;
        QPushButton* setTargetBtn = nullptr;
        QPushButton* getWhitelistBtn = nullptr;
        QPushButton* addWhitelistBtn = nullptr;
        QPushButton* delWhitelistBtn = nullptr;
        QPushButton* getPositionBtn = nullptr;
        QPushButton* getSysInfoBtn = nullptr;
        QPushButton* factoryResetBtn = nullptr;
        QPushButton* rebootBtn = nullptr;
        QPushButton* commTestBtn = nullptr;

        // 图标
        QLabel* signalQualityIcon = nullptr;
        QLabel* identityIcon = nullptr;
        QLabel* getTargetIcon = nullptr;
        QLabel* setTargetIcon = nullptr;
        QLabel* getWhitelistIcon = nullptr;
        QLabel* addWhitelistIcon = nullptr;
        QLabel* delWhitelistIcon = nullptr;
        QLabel* getPositionIcon = nullptr;
        QLabel* getSysInfoIcon = nullptr;
        QLabel* factoryResetIcon = nullptr;
        QLabel* rebootIcon = nullptr;
        QLabel* commTestIcon = nullptr;

        // 输入/反馈框
        QLineEdit* setTargetInput = nullptr;
        QLineEdit* addWhitelistInput = nullptr;
        QLineEdit* delWhitelistInput = nullptr;
        QLineEdit* signalQualityFeedback = nullptr;
        QLineEdit* identityFeedback = nullptr;
        QLineEdit* getTargetFeedback = nullptr;
        QLineEdit* setTargetFeedback = nullptr;
        QLineEdit* getPositionFeedback = nullptr;
        QLineEdit* getSysInfoFeedback = nullptr;
        QLineEdit* factoryResetFeedback = nullptr;
        QLineEdit* rebootFeedback = nullptr;
        QLineEdit* commTestFeedback = nullptr;
        QComboBox* getWhitelistFeedback = nullptr;
        QLineEdit* sendDataLe = nullptr;    // 发送数据
        QLineEdit* recvDataLe = nullptr;    // 接收数据

        // ========== 内部控件（带_In后缀） ==========
        // 按钮
        QPushButton* readSignalQualityBtnIn = nullptr;
        QPushButton* readIdentityBtnIn = nullptr;
        QPushButton* getTargetBtnIn = nullptr;
        QPushButton* setTargetBtnIn = nullptr;
        QPushButton* getWhitelistBtnIn = nullptr;
        QPushButton* addWhitelistBtnIn = nullptr;
        QPushButton* delWhitelistBtnIn = nullptr;
        QPushButton* getPositionBtnIn = nullptr;
        QPushButton* getSysInfoBtnIn = nullptr;
        QPushButton* factoryResetBtnIn = nullptr;
        QPushButton* rebootBtnIn = nullptr;

        // 图标
        QLabel* signalQualityIconIn = nullptr;
        QLabel* identityIconIn = nullptr;
        QLabel* getTargetIconIn = nullptr;
        QLabel* setTargetIconIn = nullptr;
        QLabel* getWhitelistIconIn = nullptr;
        QLabel* addWhitelistIconIn = nullptr;
        QLabel* delWhitelistIconIn = nullptr;
        QLabel* getPositionIconIn = nullptr;
        QLabel* getSysInfoIconIn = nullptr;
        QLabel* factoryResetIconIn = nullptr;
        QLabel* rebootIconIn = nullptr;

        // 输入/反馈框
        QLineEdit* setTargetInputIn = nullptr;
        QLineEdit* addWhitelistInputIn = nullptr;
        QLineEdit* delWhitelistInputIn = nullptr;
        QLineEdit* signalQualityFeedbackIn = nullptr;
        QLineEdit* identityFeedbackIn = nullptr;
        QLineEdit* getTargetFeedbackIn = nullptr;
        QLineEdit* setTargetFeedbackIn = nullptr;
        QLineEdit* getPositionFeedbackIn = nullptr;
        QLineEdit* getSysInfoFeedbackIn = nullptr;
        QLineEdit* factoryResetFeedbackIn = nullptr;
        QLineEdit* rebootFeedbackIn = nullptr;
        QComboBox* getWhitelistFeedbackIn = nullptr;

        // 初始化控件（按前缀匹配）
        void init(QWidget* parent, const QString& prefix);
    };

    // 成员变量
    QMap<DeviceId, BeidouCommUIControls> m_uiControls;
    QMap<DeviceId, QString> m_deviceNumberMap; // 北斗通信设备编号映射：BeidouComm→09

    // 工具函数（复用原有逻辑）
    template <typename T>
    T* getInternalExternalWidget(T* externalWidget, T* internalWidget, CommandType type);
    bool validateBeidouCardId(const QString& cardIdText, QString& errorMsg);
    QStringList filter7DigitCard(const QStringList& cardList);

    // 初始化函数
    void initDefaultDeviceMapping();
    void initAllUIControls();

    // 命令结果处理（按命令码分发）
    void processBeidouCommand(DeviceId deviceId, CommandCode cmdCode, const SelfCheckResult* result);

private slots:
    // ========== 外部按钮槽函数 ==========
    void onReadSignalQualityClicked(DeviceId deviceId);
    void onReadIdentityClicked(DeviceId deviceId);
    void onGetTargetClicked(DeviceId deviceId);
    void onSetTargetClicked(DeviceId deviceId);
    void onGetWhitelistClicked(DeviceId deviceId);
    void onAddWhitelistClicked(DeviceId deviceId);
    void onDelWhitelistClicked(DeviceId deviceId);
    void onGetPositionClicked(DeviceId deviceId);
    void onGetSysInfoClicked(DeviceId deviceId);
    void onFactoryResetClicked(DeviceId deviceId);
    void onRebootClicked(DeviceId deviceId);
    void onCommTestClicked(DeviceId deviceId);

    // ========== 内部按钮槽函数 ==========
    void onReadSignalQualityBtnInClicked(DeviceId deviceId);
    void onReadIdentityBtnInClicked(DeviceId deviceId);
    void onGetTargetBtnInClicked(DeviceId deviceId);
    void onSetTargetBtnInClicked(DeviceId deviceId);
    void onGetWhitelistBtnInClicked(DeviceId deviceId);
    void onAddWhitelistBtnInClicked(DeviceId deviceId);
    void onDelWhitelistBtnInClicked(DeviceId deviceId);
    void onGetPositionBtnInClicked(DeviceId deviceId);
    void onGetSysInfoBtnInClicked(DeviceId deviceId);
    void onFactoryResetBtnInClicked(DeviceId deviceId);
    void onRebootBtnInClicked(DeviceId deviceId);
};
