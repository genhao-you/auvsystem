#include "beidoucommhandler.h"
#include "widgetmanager.h"
#include "framebuilder.h" // 帧构建工具
#include <QMessageBox>
#include <QDebug>
#include <algorithm>

BeidouCommHandler::BeidouCommHandler(SelfCheckModule* module, QWidget* parent)
    : DeviceHandlerBase(module, parent)
{
    initDefaultDeviceMapping();
    initAllUIControls();
    initializeUI();
}

BeidouCommHandler::~BeidouCommHandler()
{
    disconnectSignals();
}

// 初始化北斗默认编号映射（09）
void BeidouCommHandler::initDefaultDeviceMapping()
{
    m_deviceNumberMap = { {DeviceId::BeidouComm, "09"} };
}

// 初始化北斗UI控件（09前缀，含内外两套）
void BeidouCommHandler::BeidouCommUIControls::init(QWidget* parent, const QString& prefix)
{
    // ========== 外部控件 ==========
    // 按钮
    readSignalQualityBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_ReadSignalQuality").arg(prefix), parent);
    readIdentityBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_ReadIdentity").arg(prefix), parent);
    getTargetBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_ReadTarget").arg(prefix), parent); // 注意原命名是ReadTarget
    setTargetBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_SetTarget").arg(prefix), parent);
    getWhitelistBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_GetWhitelist").arg(prefix), parent);
    addWhitelistBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_AddWhitelist").arg(prefix), parent);
    delWhitelistBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_DelWhitelis").arg(prefix), parent); // 原命名笔误DelWhitelis
    getPositionBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_GetPosition").arg(prefix), parent);
    getSysInfoBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_GetSysInfo").arg(prefix), parent);
    factoryResetBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_FactoryReset").arg(prefix), parent);
    rebootBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_Reboot").arg(prefix), parent);
    commTestBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_CommTest").arg(prefix), parent);

    // 图标
    signalQualityIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_SignalQualityIcon").arg(prefix), parent);
    identityIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_IdentityIcon").arg(prefix), parent);
    getTargetIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_GetTargetIcon").arg(prefix), parent);
    setTargetIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_SetTargetIcon").arg(prefix), parent);
    getWhitelistIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_GetWhitelistIcon").arg(prefix), parent);
    addWhitelistIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_AddWhitelistIcon").arg(prefix), parent);
    delWhitelistIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_DelWhitelistIcon").arg(prefix), parent);
    getPositionIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_GetPositionIcon").arg(prefix), parent);
    getSysInfoIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_GetSysInfoIcon").arg(prefix), parent);
    factoryResetIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_FactoryResetIcon").arg(prefix), parent);
    rebootIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_RebootIcon").arg(prefix), parent);
    commTestIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_CommTestIcon").arg(prefix), parent);

    // 输入/反馈框
    setTargetInput = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_SetTarget_Input").arg(prefix), parent);
    addWhitelistInput = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_AddWhitelist_Input").arg(prefix), parent);
    delWhitelistInput = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_DelWhitelist_Input").arg(prefix), parent);
    signalQualityFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_SignalQuality_Feedback").arg(prefix), parent);
    identityFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_Identity_Feedback").arg(prefix), parent);
    getTargetFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_GetTarget_Feedback").arg(prefix), parent);
    setTargetFeedback = setTargetInput; // 反馈显示在输入框
    getPositionFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_GetPosition_Feedback").arg(prefix), parent);
    getSysInfoFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_GetSysInfo_Feedback").arg(prefix), parent);
    factoryResetFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_FactoryReset_Feedback").arg(prefix), parent);
    rebootFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_Reboot_Feedback").arg(prefix), parent);
    commTestFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_CommTest_Feedback").arg(prefix), parent);
    getWhitelistFeedback = WidgetManager::instance().getWidget<QComboBox>(QString("cb_%1_GetWhitelist_Feedback").arg(prefix), parent);
    sendDataLe = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_SendData").arg(prefix), parent);
    recvDataLe = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReceiveData").arg(prefix), parent);

    // ========== 内部控件 ==========
    // 按钮
    readSignalQualityBtnIn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_ReadSignalQuality_In").arg(prefix), parent);
    readIdentityBtnIn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_ReadIdentity_In").arg(prefix), parent);
    getTargetBtnIn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_GetTarget_In").arg(prefix), parent);
    setTargetBtnIn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_SetTarget_In").arg(prefix), parent);
    getWhitelistBtnIn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_GetWhitelist_In").arg(prefix), parent);
    addWhitelistBtnIn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_AddWhitelist_In").arg(prefix), parent);
    delWhitelistBtnIn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_DelWhitelis_In").arg(prefix), parent); // 原命名笔误
    getPositionBtnIn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_GetPosition_In").arg(prefix), parent);
    getSysInfoBtnIn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_GetSysInfo_In").arg(prefix), parent);
    factoryResetBtnIn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_FactoryReset_In").arg(prefix), parent);
    rebootBtnIn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_Reboot_In").arg(prefix), parent);

    // 图标
    signalQualityIconIn = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_SignalQualityIcon_In").arg(prefix), parent);
    identityIconIn = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_IdentityIcon_In").arg(prefix), parent);
    getTargetIconIn = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_GetTargetIcon_In").arg(prefix), parent);
    setTargetIconIn = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_SetTargetIcon_In").arg(prefix), parent);
    getWhitelistIconIn = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_GetWhitelistIcon_In").arg(prefix), parent);
    addWhitelistIconIn = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_AddWhitelistIcon_In").arg(prefix), parent);
    delWhitelistIconIn = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_DelWhitelistIcon_In").arg(prefix), parent);
    getPositionIconIn = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_GetPositionIcon_In").arg(prefix), parent);
    getSysInfoIconIn = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_GetSysInfoIcon_In").arg(prefix), parent);
    factoryResetIconIn = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_FactoryResetIcon_In").arg(prefix), parent);
    rebootIconIn = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_RebootIcon_In").arg(prefix), parent);

    // 输入/反馈框
    setTargetInputIn = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_SetTarget_Input_In").arg(prefix), parent);
    addWhitelistInputIn = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_AddWhitelist_Input_In").arg(prefix), parent);
    delWhitelistInputIn = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_DelWhitelist_Input_In").arg(prefix), parent);
    signalQualityFeedbackIn = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_SignalQuality_Feedback_In").arg(prefix), parent);
    identityFeedbackIn = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_Identity_Feedback_In").arg(prefix), parent);
    getTargetFeedbackIn = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_GetTarget_Feedback_In").arg(prefix), parent);
    setTargetFeedbackIn = setTargetInputIn; // 反馈显示在输入框
    getPositionFeedbackIn = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_GetPosition_Feedback_In").arg(prefix), parent);
    getSysInfoFeedbackIn = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_GetSysInfo_Feedback_In").arg(prefix), parent);
    factoryResetFeedbackIn = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_FactoryReset_Feedback_In").arg(prefix), parent);
    rebootFeedbackIn = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_Reboot_Feedback_In").arg(prefix), parent);
    getWhitelistFeedbackIn = WidgetManager::instance().getWidget<QComboBox>(QString("cb_%1_GetWhitelist_Feedback_In").arg(prefix), parent);
}

void BeidouCommHandler::initAllUIControls()
{
    for (auto it = m_deviceNumberMap.begin(); it != m_deviceNumberMap.end(); ++it) {
        BeidouCommUIControls controls;
        controls.number = it.value();
        controls.init(m_parent, controls.number);
        m_uiControls[it.key()] = controls;
    }
}

// 初始化UI（清空所有输入/反馈框）
void BeidouCommHandler::initializeUI()
{
    for (auto& controls : m_uiControls) {
        // 外部输入框
        if (controls.setTargetInput) controls.setTargetInput->clear();
        if (controls.addWhitelistInput) controls.addWhitelistInput->clear();
        if (controls.delWhitelistInput) controls.delWhitelistInput->clear();
        // 外部反馈框
        if (controls.signalQualityFeedback) controls.signalQualityFeedback->clear();
        if (controls.identityFeedback) controls.identityFeedback->clear();
        if (controls.getTargetFeedback) controls.getTargetFeedback->clear();
        if (controls.getPositionFeedback) controls.getPositionFeedback->clear();
        if (controls.getSysInfoFeedback) controls.getSysInfoFeedback->clear();
        if (controls.factoryResetFeedback) controls.factoryResetFeedback->clear();
        if (controls.rebootFeedback) controls.rebootFeedback->clear();
        if (controls.commTestFeedback) controls.commTestFeedback->clear();
        if (controls.getWhitelistFeedback) controls.getWhitelistFeedback->clear();
        if (controls.sendDataLe) controls.sendDataLe->clear();
        if (controls.recvDataLe) controls.recvDataLe->clear();

        // 内部输入框
        if (controls.setTargetInputIn) controls.setTargetInputIn->clear();
        if (controls.addWhitelistInputIn) controls.addWhitelistInputIn->clear();
        if (controls.delWhitelistInputIn) controls.delWhitelistInputIn->clear();
        // 内部反馈框
        if (controls.signalQualityFeedbackIn) controls.signalQualityFeedbackIn->clear();
        if (controls.identityFeedbackIn) controls.identityFeedbackIn->clear();
        if (controls.getTargetFeedbackIn) controls.getTargetFeedbackIn->clear();
        if (controls.getPositionFeedbackIn) controls.getPositionFeedbackIn->clear();
        if (controls.getSysInfoFeedbackIn) controls.getSysInfoFeedbackIn->clear();
        if (controls.factoryResetFeedbackIn) controls.factoryResetFeedbackIn->clear();
        if (controls.rebootFeedbackIn) controls.rebootFeedbackIn->clear();
        if (controls.getWhitelistFeedbackIn) controls.getWhitelistFeedbackIn->clear();
    }
}

// 连接信号（先断后连，避免重复）
void BeidouCommHandler::connectSignals()
{
    disconnectSignals();

    for (auto it = m_uiControls.begin(); it != m_uiControls.end(); ++it) {
        DeviceId deviceId = it.key();
        BeidouCommUIControls& controls = it.value();

        qDebug() << "[北斗通信信号连接] DeviceId:" << static_cast<int>(deviceId)
            << " 前缀:" << controls.number;

        // ========== 外部按钮信号 ==========
        if (controls.readSignalQualityBtn) {
            connect(controls.readSignalQualityBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onReadSignalQualityClicked(deviceId); });
        }
        if (controls.readIdentityBtn) {
            connect(controls.readIdentityBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onReadIdentityClicked(deviceId); });
        }
        if (controls.getTargetBtn) {
            connect(controls.getTargetBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetTargetClicked(deviceId); });
        }
        if (controls.setTargetBtn) {
            connect(controls.setTargetBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onSetTargetClicked(deviceId); });
        }
        if (controls.getWhitelistBtn) {
            connect(controls.getWhitelistBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetWhitelistClicked(deviceId); });
        }
        if (controls.addWhitelistBtn) {
            connect(controls.addWhitelistBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onAddWhitelistClicked(deviceId); });
        }
        if (controls.delWhitelistBtn) {
            connect(controls.delWhitelistBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onDelWhitelistClicked(deviceId); });
        }
        if (controls.getPositionBtn) {
            connect(controls.getPositionBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetPositionClicked(deviceId); });
        }
        if (controls.getSysInfoBtn) {
            connect(controls.getSysInfoBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetSysInfoClicked(deviceId); });
        }
        if (controls.factoryResetBtn) {
            connect(controls.factoryResetBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onFactoryResetClicked(deviceId); });
        }
        if (controls.rebootBtn) {
            connect(controls.rebootBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onRebootClicked(deviceId); });
        }
        if (controls.commTestBtn) {
            connect(controls.commTestBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onCommTestClicked(deviceId); });
        }

        // ========== 内部按钮信号 ==========
        if (controls.readSignalQualityBtnIn) {
            connect(controls.readSignalQualityBtnIn, &QPushButton::clicked,
                this, [this, deviceId]() { onReadSignalQualityBtnInClicked(deviceId); });
        }
        if (controls.readIdentityBtnIn) {
            connect(controls.readIdentityBtnIn, &QPushButton::clicked,
                this, [this, deviceId]() { onReadIdentityBtnInClicked(deviceId); });
        }
        if (controls.getTargetBtnIn) {
            connect(controls.getTargetBtnIn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetTargetBtnInClicked(deviceId); });
        }
        if (controls.setTargetBtnIn) {
            connect(controls.setTargetBtnIn, &QPushButton::clicked,
                this, [this, deviceId]() { onSetTargetBtnInClicked(deviceId); });
        }
        if (controls.getWhitelistBtnIn) {
            connect(controls.getWhitelistBtnIn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetWhitelistBtnInClicked(deviceId); });
        }
        if (controls.addWhitelistBtnIn) {
            connect(controls.addWhitelistBtnIn, &QPushButton::clicked,
                this, [this, deviceId]() { onAddWhitelistBtnInClicked(deviceId); });
        }
        if (controls.delWhitelistBtnIn) {
            connect(controls.delWhitelistBtnIn, &QPushButton::clicked,
                this, [this, deviceId]() { onDelWhitelistBtnInClicked(deviceId); });
        }
        if (controls.getPositionBtnIn) {
            connect(controls.getPositionBtnIn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetPositionBtnInClicked(deviceId); });
        }
        if (controls.getSysInfoBtnIn) {
            connect(controls.getSysInfoBtnIn, &QPushButton::clicked,
                this, [this, deviceId]() { onGetSysInfoBtnInClicked(deviceId); });
        }
        if (controls.factoryResetBtnIn) {
            connect(controls.factoryResetBtnIn, &QPushButton::clicked,
                this, [this, deviceId]() { onFactoryResetBtnInClicked(deviceId); });
        }
        if (controls.rebootBtnIn) {
            connect(controls.rebootBtnIn, &QPushButton::clicked,
                this, [this, deviceId]() { onRebootBtnInClicked(deviceId); });
        }
    }
}

// 断开所有信号
void BeidouCommHandler::disconnectSignals()
{
    for (auto& controls : m_uiControls) {
        // 外部按钮
        if (controls.readSignalQualityBtn) controls.readSignalQualityBtn->disconnect();
        if (controls.readIdentityBtn) controls.readIdentityBtn->disconnect();
        if (controls.getTargetBtn) controls.getTargetBtn->disconnect();
        if (controls.setTargetBtn) controls.setTargetBtn->disconnect();
        if (controls.getWhitelistBtn) controls.getWhitelistBtn->disconnect();
        if (controls.addWhitelistBtn) controls.addWhitelistBtn->disconnect();
        if (controls.delWhitelistBtn) controls.delWhitelistBtn->disconnect();
        if (controls.getPositionBtn) controls.getPositionBtn->disconnect();
        if (controls.getSysInfoBtn) controls.getSysInfoBtn->disconnect();
        if (controls.factoryResetBtn) controls.factoryResetBtn->disconnect();
        if (controls.rebootBtn) controls.rebootBtn->disconnect();
        if (controls.commTestBtn) controls.commTestBtn->disconnect();

        // 内部按钮
        if (controls.readSignalQualityBtnIn) controls.readSignalQualityBtnIn->disconnect();
        if (controls.readIdentityBtnIn) controls.readIdentityBtnIn->disconnect();
        if (controls.getTargetBtnIn) controls.getTargetBtnIn->disconnect();
        if (controls.setTargetBtnIn) controls.setTargetBtnIn->disconnect();
        if (controls.getWhitelistBtnIn) controls.getWhitelistBtnIn->disconnect();
        if (controls.addWhitelistBtnIn) controls.addWhitelistBtnIn->disconnect();
        if (controls.delWhitelistBtnIn) controls.delWhitelistBtnIn->disconnect();
        if (controls.getPositionBtnIn) controls.getPositionBtnIn->disconnect();
        if (controls.getSysInfoBtnIn) controls.getSysInfoBtnIn->disconnect();
        if (controls.factoryResetBtnIn) controls.factoryResetBtnIn->disconnect();
        if (controls.rebootBtnIn) controls.rebootBtnIn->disconnect();
    }
}

// 工具函数：区分内外控件
template <typename T>
T* BeidouCommHandler::getInternalExternalWidget(T* externalWidget, T* internalWidget, CommandType type)
{
    return (type == CommandType::Internal) ? internalWidget : externalWidget;
}

// 工具函数：验证北斗卡号（7位数字）
bool BeidouCommHandler::validateBeidouCardId(const QString& cardIdText, QString& errorMsg)
{
    errorMsg.clear();
    QString trimmedText = cardIdText.trimmed();

    if (trimmedText.isEmpty()) {
        errorMsg = "卡号不能为空！";
        return false;
    }

    bool isNumber = false;
    trimmedText.toUInt(&isNumber);
    if (!isNumber) {
        errorMsg = "卡号必须为纯数字！";
        return false;
    }

    if (trimmedText.length() > 7) {
        errorMsg = "卡号长度不能超过7位！";
        return false;
    }

    return true;
}

// 工具函数：过滤7位有效卡号
QStringList BeidouCommHandler::filter7DigitCard(const QStringList& cardList)
{
    QStringList validList;
    for (const QString& card : cardList) {
        QString trimmed = card.trimmed();
        bool isNumber = false;
        trimmed.toUInt(&isNumber);
        if (isNumber && trimmed.length() == 7) {
            validList.append(trimmed);
        }
    }
    return validList;
}

// 统一处理北斗命令结果
void BeidouCommHandler::handleResult(const SelfCheckResult* result)
{
    if (!result) return; // 空指针防护

    DeviceId deviceId = result->deviceId();
    if (!m_uiControls.contains(deviceId)) {
        qWarning() << "不支持的北斗通信设备:" << static_cast<uint8_t>(deviceId);
        return;
    }

    processBeidouCommand(deviceId, result->commandCode(), result);
}

// 分发处理不同北斗命令结果（完整保留原有逻辑）
void BeidouCommHandler::processBeidouCommand(DeviceId deviceId, CommandCode cmdCode, const SelfCheckResult* result)
{
    BeidouCommUIControls& controls = m_uiControls[deviceId];
    CommandCode cmdCodeResult = result->commandCode();
    ResultStatus status = result->status();
    QPixmap icon = getStatusIcon(status);
    QString feedbackDesc = result->feedbackDesc();
    CommandType type = result->commandType();

    // ====================== 1. 信号质量查询 ======================
    if (cmdCodeResult == CommandCode::BeidouComm_GetSignalQuality)
    {
        QLabel* signalIconLbl = getInternalExternalWidget(controls.signalQualityIcon,
            controls.signalQualityIconIn, type);
        QLineEdit* signalFeedbackLe = getInternalExternalWidget(controls.signalQualityFeedback,
            controls.signalQualityFeedbackIn, type);

        if (!signalIconLbl || !signalFeedbackLe) return;
        setIcon(signalIconLbl, icon);

        QString signalStrengthDesc = "未知等级";
        if (result->getParameter("signalQuality").isValid())
        {
            int signalLevel = result->getParameter("signalQuality").toInt();
            switch (static_cast<BeidouSignalQuality>(signalLevel))
            {
            case BeidouSignalQuality::NoSignal:    signalStrengthDesc = "无通信能力"; break;
            case BeidouSignalQuality::VeryWeak:    signalStrengthDesc = "可能丢包";   break;
            case BeidouSignalQuality::Weak:        signalStrengthDesc = "基本通信";   break;
            case BeidouSignalQuality::Good:        signalStrengthDesc = "稳定通信";   break;
            case BeidouSignalQuality::VeryGood:    signalStrengthDesc = "优质通信";   break;
            case BeidouSignalQuality::Excellent:   signalStrengthDesc = "最佳通信";   break;
            default:                                signalStrengthDesc = "未知等级";   break;
            }
        }
        updateStatusWidget(signalFeedbackLe, status, signalStrengthDesc);
    }

    // ====================== 2. 本机卡号查询 ======================
    else if (cmdCodeResult == CommandCode::BeidouComm_GetIdentity)
    {
        QLabel* identityIconLbl = getInternalExternalWidget(controls.identityIcon,
            controls.identityIconIn, type);
        QLineEdit* identityFeedbackLe = getInternalExternalWidget(controls.identityFeedback,
            controls.identityFeedbackIn, type);
        if (!identityIconLbl || !identityFeedbackLe) return;

        setIcon(identityIconLbl, icon);
        QString localCardStr = "未知卡号";
        if (result->getParameter("localCardNo").isValid())
        {
            uint32_t localCardNo = result->getParameter("localCardNo").toUInt();
            localCardStr = QString("%1").arg(localCardNo, 7, 10, QChar('0'));
        }
        updateStatusWidget(identityFeedbackLe, status, localCardStr);
    }

    // ====================== 3. 目标卡号查询 ======================
    else if (cmdCodeResult == CommandCode::BeidouComm_GetTarget)
    {
        QLabel* targetIconLbl = getInternalExternalWidget(controls.getTargetIcon,
            controls.getTargetIconIn, type);
        QLineEdit* targetFeedbackLe = getInternalExternalWidget(controls.getTargetFeedback,
            controls.getTargetFeedbackIn, type);
        if (!targetIconLbl || !targetFeedbackLe) return;

        setIcon(targetIconLbl, icon);
        if (result->getParameter("targetCardNo").isValid())
        {
            uint32_t targetCardNo = result->getParameter("targetCardNo").toUInt();
            QString targetCardStr = QString("%1").arg(targetCardNo, 7, 10, QChar('0'));
            updateStatusWidget(targetFeedbackLe, status, targetCardStr);
        }
        else
        {
            updateStatusWidget(targetFeedbackLe, status, feedbackDesc);
        }
    }

    // ====================== 4. 白名单查询 ======================
    else if (cmdCodeResult == CommandCode::BeidouComm_GetWhitelist)
    {
        QLabel* whitelistIconLbl = getInternalExternalWidget(controls.getWhitelistIcon,
            controls.getWhitelistIconIn, type);
        QComboBox* cbxWhitelist = getInternalExternalWidget(controls.getWhitelistFeedback,
            controls.getWhitelistFeedbackIn, type);
        if (!whitelistIconLbl || !cbxWhitelist) return;

        setIcon(whitelistIconLbl, icon);
        cbxWhitelist->clear();
        cbxWhitelist->setStyleSheet("");
        cbxWhitelist->setEditable(false);

        if (status == ResultStatus::Success)
        {
            QStringList whitelist = result->getParameter("whitelist").toStringList();
            QStringList valid7DigitWhitelist = filter7DigitCard(whitelist);

            if (!valid7DigitWhitelist.isEmpty())
            {
                cbxWhitelist->addItems(valid7DigitWhitelist);
                cbxWhitelist->setCurrentIndex(0);
            }
            else
            {
                cbxWhitelist->addItem("无符合7位规则的有效卡号");
                cbxWhitelist->setItemData(0, QBrush(Qt::gray), Qt::TextColorRole);
            }

            if (!whitelist.isEmpty() && whitelist.size() != valid7DigitWhitelist.size())
            {
                int insertIndex = valid7DigitWhitelist.isEmpty() ? 0 : 0;
                cbxWhitelist->insertItem(insertIndex, "⚠️ 部分卡号不符合7位规则已过滤");
                cbxWhitelist->setItemData(insertIndex, QBrush(QColor(255, 165, 0)), Qt::TextColorRole);
                cbxWhitelist->setCurrentIndex(insertIndex);
            }
        }
        else
        {
            QString errorText = QString("白名单查询失败：%1").arg(feedbackDesc);
            cbxWhitelist->addItem(errorText);
            cbxWhitelist->setItemData(0, QBrush(Qt::red), Qt::TextColorRole);
            cbxWhitelist->setCurrentIndex(0);
        }
    }

    // ====================== 5. 定位信息处理 ======================
    else if (cmdCodeResult == CommandCode::BeidouComm_GetPosition)
    {
        QLabel* positionIconLbl = getInternalExternalWidget(controls.getPositionIcon,
            controls.getPositionIconIn, type);
        QLineEdit* positionFeedbackLe = getInternalExternalWidget(controls.getPositionFeedback,
            controls.getPositionFeedbackIn, type);
        if (!positionIconLbl || !positionFeedbackLe) return;

        setIcon(positionIconLbl, icon);
        if (status == ResultStatus::Success)
        {
            bool isValidFix = result->getParameter("is_valid_fix").toBool();
            double latDegree = result->getParameter("latitude_degree").toDouble();
            double lonDegree = result->getParameter("longitude_degree").toDouble();

            QString positionText;
            if (isValidFix)
            {
                positionText = QString("%1°，%2°")
                    .arg(latDegree, 0, 'f', 6)
                    .arg(lonDegree, 0, 'f', 6);
            }
            else
            {
                positionText = "无效定位（无有效经纬度数据）";
            }
            updateStatusWidget(positionFeedbackLe, status, positionText);
        }
        else
        {
            updateStatusWidget(positionFeedbackLe, status, feedbackDesc);
        }
    }

    // ====================== 6. 系统信息处理 ======================
    else if (cmdCodeResult == CommandCode::BeidouComm_GetSysInfo)
    {
        QLabel* sysInfoIconLbl = getInternalExternalWidget(controls.getSysInfoIcon,
            controls.getSysInfoIconIn, type);
        QLineEdit* sysInfoFeedbackLe = getInternalExternalWidget(controls.getSysInfoFeedback,
            controls.getSysInfoFeedbackIn, type);
        if (!sysInfoIconLbl || !sysInfoFeedbackLe) return;

        setIcon(sysInfoIconLbl, icon);
        QString sysInfo = "未知系统信息";
        if (result->getParameter("systemInfo").isValid())
        {
            sysInfo = result->getParameter("systemInfo").toString();
        }
        updateStatusWidget(sysInfoFeedbackLe, status, sysInfo);
    }

    // ====================== 7. 有参设置命令（SetTarget/AddWhitelist/DelWhitelist） ======================
    else if (cmdCodeResult == CommandCode::BeidouComm_SetTarget ||
        cmdCodeResult == CommandCode::BeidouComm_AddWhitelist ||
        cmdCodeResult == CommandCode::BeidouComm_DelWhitelist)
    {
        QLabel* iconLbl = nullptr;
        QLineEdit* feedbackLe = nullptr;

        if (cmdCodeResult == CommandCode::BeidouComm_SetTarget)
        {
            iconLbl = getInternalExternalWidget(controls.setTargetIcon,
                controls.setTargetIconIn, type);
            feedbackLe = getInternalExternalWidget(controls.setTargetFeedback,
                controls.setTargetFeedbackIn, type);
        }
        else if (cmdCodeResult == CommandCode::BeidouComm_AddWhitelist)
        {
            iconLbl = getInternalExternalWidget(controls.addWhitelistIcon,
                controls.addWhitelistIconIn, type);
            feedbackLe = getInternalExternalWidget(controls.addWhitelistInput,
                controls.addWhitelistInputIn, type);
        }
        else if (cmdCodeResult == CommandCode::BeidouComm_DelWhitelist)
        {
            iconLbl = getInternalExternalWidget(controls.delWhitelistIcon,
                controls.delWhitelistIconIn, type);
            feedbackLe = getInternalExternalWidget(controls.delWhitelistInput,
                controls.delWhitelistInputIn, type);
        }

        if (iconLbl && feedbackLe)
        {
            setIcon(iconLbl, icon);
            QString tip = (cmdCodeResult == CommandCode::BeidouComm_AddWhitelist || cmdCodeResult == CommandCode::BeidouComm_DelWhitelist)
                ? "（仅支持7位数字卡号）" : "";
            updateStatusWidget(feedbackLe, status, feedbackDesc + tip);
        }
    }

    // ====================== 8. 控制命令（FactoryReset/Reboot/CommTest） ======================
    else if (cmdCodeResult == CommandCode::BeidouComm_FactoryReset ||
        cmdCodeResult == CommandCode::BeidouComm_Reboot ||
        cmdCodeResult == CommandCode::BeidouComm_Test)
    {
        QLabel* iconLbl = nullptr;
        QLineEdit* feedbackLe = nullptr;
        QComboBox* cbxWhitelist = getInternalExternalWidget(controls.getWhitelistFeedback,
            controls.getWhitelistFeedbackIn, type);

        if (cmdCodeResult == CommandCode::BeidouComm_FactoryReset)
        {
            iconLbl = getInternalExternalWidget(controls.factoryResetIcon,
                controls.factoryResetIconIn, type);
            feedbackLe = getInternalExternalWidget(controls.factoryResetFeedback,
                controls.factoryResetFeedbackIn, type);
        }
        else if (cmdCodeResult == CommandCode::BeidouComm_Reboot)
        {
            iconLbl = getInternalExternalWidget(controls.rebootIcon,
                controls.rebootIconIn, type);
            feedbackLe = getInternalExternalWidget(controls.rebootFeedback,
                controls.rebootFeedbackIn, type);
        }
        else if (cmdCodeResult == CommandCode::BeidouComm_Test)
        {
            iconLbl = controls.commTestIcon;
            feedbackLe = controls.commTestFeedback;
        }

        QString sendData = result->getParameter("sent_data_hex").isValid()
            ? result->getParameter("sent_data_hex").toString() : "无发送数据";
        QString recvData = result->getParameter("recv_data_hex").isValid()
            ? result->getParameter("recv_data_hex").toString() : "无接收数据";

        if (iconLbl && feedbackLe)
        {
            setIcon(iconLbl, icon);
            if (status == ResultStatus::Success)
            {
                QString enhancedDesc = feedbackDesc;
                if (cmdCodeResult == CommandCode::BeidouComm_Reboot)
                {
                    enhancedDesc += "（模块将重启，请勿断电）";
                }
                else if (cmdCodeResult == CommandCode::BeidouComm_FactoryReset)
                {
                    enhancedDesc += "（所有7位白名单已清空）";
                    if (cbxWhitelist)
                    {
                        cbxWhitelist->clear();
                        cbxWhitelist->addItem("白名单已清空");
                        cbxWhitelist->setItemData(0, QBrush(Qt::gray), Qt::TextColorRole);
                    }
                }
                updateStatusWidget(feedbackLe, status, enhancedDesc);

                if (cmdCodeResult == CommandCode::BeidouComm_Test)
                {
                    if (controls.sendDataLe) updateStatusWidget(controls.sendDataLe, status, sendData);
                    if (controls.recvDataLe) updateStatusWidget(controls.recvDataLe, status, recvData);
                }
            }
            else if (status == ResultStatus::Timeout)
            {
                updateStatusWidget(feedbackLe, status, feedbackDesc);
                if (cmdCodeResult == CommandCode::BeidouComm_Test)
                {
                    if (controls.sendDataLe) updateStatusWidget(controls.sendDataLe, status, sendData);
                    if (controls.recvDataLe) updateStatusWidget(controls.recvDataLe, status, recvData);
                }
            }
            else
            {
                updateStatusWidget(feedbackLe, status, feedbackDesc);
            }
        }
    }
}

// 获取设备编号
QString BeidouCommHandler::getDeviceNumber(DeviceId deviceId) const
{
    return m_deviceNumberMap.value(deviceId, "");
}

// 支持的设备
bool BeidouCommHandler::supportsDevice(DeviceId deviceId) const
{
    return m_deviceNumberMap.contains(deviceId);
}

// 设置设备映射
void BeidouCommHandler::setDeviceMapping(const QMap<DeviceId, QString>& mapping)
{
    m_deviceNumberMap = mapping;
    initAllUIControls();
}

// -------------------- 外部按钮槽函数实现 --------------------
void BeidouCommHandler::onReadSignalQualityClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId,
            CommandCode::BeidouComm_GetSignalQuality,
            params,
            WorkPhase::ConnectivityTest);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("北斗信号质量查询命令失败: %1").arg(e.what()));
    }
}

void BeidouCommHandler::onReadIdentityClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId,
            CommandCode::BeidouComm_GetIdentity,
            params,
            WorkPhase::ConnectivityTest);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("北斗本机卡号查询命令失败: %1").arg(e.what()));
    }
}

void BeidouCommHandler::onGetTargetClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId,
            CommandCode::BeidouComm_GetTarget,
            params,
            WorkPhase::ConnectivityTest);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("北斗目标卡号查询命令失败: %1").arg(e.what()));
    }
}

void BeidouCommHandler::onSetTargetClicked(DeviceId deviceId)
{
    if (!m_module) return;
    BeidouCommUIControls& controls = m_uiControls[deviceId];
    if (!controls.setTargetInput) return;

    QString cardID = controls.setTargetInput->text();
    auto params = std::make_shared<BeidouCardNoParam>(cardID);
    try {
        m_module->sendCheckCommand(deviceId,
            CommandCode::BeidouComm_SetTarget,
            params,
            WorkPhase::ConnectivityTest);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("北斗设置目标卡号命令失败: %1").arg(e.what()));
    }
}

void BeidouCommHandler::onGetWhitelistClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId,
            CommandCode::BeidouComm_GetWhitelist,
            params,
            WorkPhase::ConnectivityTest);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("北斗白名单查询命令失败: %1").arg(e.what()));
    }
}

void BeidouCommHandler::onAddWhitelistClicked(DeviceId deviceId)
{
    if (!m_module) return;
    BeidouCommUIControls& controls = m_uiControls[deviceId];
    if (!controls.addWhitelistInput) return;

    QString cardID = controls.addWhitelistInput->text();
    if (cardID.isEmpty()) return;

    auto params = std::make_shared<BeidouCardNoParam>(cardID);
    try {
        m_module->sendCheckCommand(deviceId,
            CommandCode::BeidouComm_AddWhitelist,
            params,
            WorkPhase::ConnectivityTest);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("北斗添加白名单命令失败: %1").arg(e.what()));
    }
}

void BeidouCommHandler::onDelWhitelistClicked(DeviceId deviceId)
{
    if (!m_module) return;
    BeidouCommUIControls& controls = m_uiControls[deviceId];
    if (!controls.delWhitelistInput) return;

    QString cardID = controls.delWhitelistInput->text();
    auto params = std::make_shared<BeidouCardNoParam>(cardID);
    try {
        m_module->sendCheckCommand(deviceId,
            CommandCode::BeidouComm_DelWhitelist,
            params,
            WorkPhase::ConnectivityTest);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("北斗删除白名单命令失败: %1").arg(e.what()));
    }
}

void BeidouCommHandler::onGetPositionClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId,
            CommandCode::BeidouComm_GetPosition,
            params,
            WorkPhase::ConnectivityTest);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("北斗定位信息查询命令失败: %1").arg(e.what()));
    }
}

void BeidouCommHandler::onGetSysInfoClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId,
            CommandCode::BeidouComm_GetSysInfo,
            params,
            WorkPhase::ConnectivityTest);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("北斗系统信息查询命令失败: %1").arg(e.what()));
    }
}

void BeidouCommHandler::onFactoryResetClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId,
            CommandCode::BeidouComm_FactoryReset,
            params,
            WorkPhase::ConnectivityTest);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("北斗恢复出厂设置命令失败: %1").arg(e.what()));
    }
}

void BeidouCommHandler::onRebootClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId,
            CommandCode::BeidouComm_Reboot,
            params,
            WorkPhase::ConnectivityTest);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("北斗重启命令失败: %1").arg(e.what()));
    }
}

void BeidouCommHandler::onCommTestClicked(DeviceId deviceId)
{
    if (!m_module) return;
    DeviceId devId = DeviceId::BeidouComm;
    CommandCode cmdCode = CommandCode::BeidouComm_Test;
    WorkPhase workPhase = WorkPhase::ConnectivityTest;

    // 提前获取packetSeq
    uint8_t packetSeq = m_module->getUniquePacketSeq();
    auto emptyParam = std::make_shared<EmptyParameter>();
    QByteArray businessParamBytes = emptyParam ? emptyParam->toBytes() : QByteArray();

    // 构建InternalCommandParam
    auto sndParam = std::make_shared<InternalCommandParam>();
    {
        QByteArray fullBusinessFrame = FrameBuilder::buildCheckCommandFrame(
            devId,
            cmdCode,
            workPhase,
            businessParamBytes,
            packetSeq
        );

        sndParam->m_extraParams["packetSeq"] = packetSeq;
        sndParam->m_extraParams["original_frame_data"] = fullBusinessFrame;
        sndParam->m_extraParams["original_frame_hex"] = fullBusinessFrame.toHex().toUpper();
        sndParam->m_extraParams["deviceId"] = static_cast<int>(devId);
        sndParam->m_extraParams["cmdCode"] = static_cast<int>(cmdCode);
    }

    // 发送命令
    CheckError err = m_module->sendCheckCommand(
        devId,
        cmdCode,
        sndParam,
        workPhase,
        CommunicationChannel::BDS,
        CommandType::Internal
    );
    if (err.code() != ErrorCode::Success) {
        qWarning() << "发送北斗测试指令失败：" << err.description();
    }
}

// -------------------- 内部按钮槽函数实现 --------------------
void BeidouCommHandler::onReadSignalQualityBtnInClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId,
            CommandCode::BeidouComm_GetSignalQuality,
            params,
            WorkPhase::ConnectivityTest,
            CommunicationChannel::BDS,
            CommandType::Internal);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("北斗内部信号质量查询命令失败: %1").arg(e.what()));
    }
}

void BeidouCommHandler::onReadIdentityBtnInClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId,
            CommandCode::BeidouComm_GetIdentity,
            params,
            WorkPhase::ConnectivityTest,
            CommunicationChannel::BDS,
            CommandType::Internal);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("北斗内部本机卡号查询命令失败: %1").arg(e.what()));
    }
}

void BeidouCommHandler::onGetTargetBtnInClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId,
            CommandCode::BeidouComm_GetTarget,
            params,
            WorkPhase::ConnectivityTest,
            CommunicationChannel::BDS,
            CommandType::Internal);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("北斗内部目标卡号查询命令失败: %1").arg(e.what()));
    }
}

void BeidouCommHandler::onSetTargetBtnInClicked(DeviceId deviceId)
{
    if (!m_module) return;
    BeidouCommUIControls& controls = m_uiControls[deviceId];
    if (!controls.setTargetInputIn) return;

    QString cardIdText = controls.setTargetInputIn->text();
    QString cardErrorMsg;
    if (!validateBeidouCardId(cardIdText, cardErrorMsg)) {
        QMessageBox::warning(m_parent, "卡号输入错误", cardErrorMsg);
        return;
    }
    QString cardIdStr = cardIdText.trimmed().rightJustified(7, '0');
    uint32_t cardId = cardIdStr.toUInt();
    auto params = std::make_shared<BeidouCardNoParam>(cardId);

    try {
        m_module->sendCheckCommand(deviceId,
            CommandCode::BeidouComm_SetTarget,
            params,
            WorkPhase::ConnectivityTest,
            CommunicationChannel::BDS,
            CommandType::Internal);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("北斗内部设置目标卡号命令失败: %1").arg(e.what()));
    }
}

void BeidouCommHandler::onGetWhitelistBtnInClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId,
            CommandCode::BeidouComm_GetWhitelist,
            params,
            WorkPhase::ConnectivityTest,
            CommunicationChannel::BDS,
            CommandType::Internal);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("北斗内部白名单查询命令失败: %1").arg(e.what()));
    }
}

void BeidouCommHandler::onAddWhitelistBtnInClicked(DeviceId deviceId)
{
    if (!m_module) return;
    BeidouCommUIControls& controls = m_uiControls[deviceId];
    if (!controls.addWhitelistInputIn) return;

    QString cardIdText = controls.addWhitelistInputIn->text();
    QString cardErrorMsg;
    if (!validateBeidouCardId(cardIdText, cardErrorMsg)) {
        QMessageBox::warning(m_parent, "卡号输入错误", cardErrorMsg);
        return;
    }
    QString cardIdStr = cardIdText.trimmed().rightJustified(7, '0');
    uint32_t cardId = cardIdStr.toUInt();
    auto params = std::make_shared<BeidouCardNoParam>(cardId);

    try {
        m_module->sendCheckCommand(deviceId,
            CommandCode::BeidouComm_AddWhitelist,
            params,
            WorkPhase::ConnectivityTest,
            CommunicationChannel::BDS,
            CommandType::Internal);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("北斗内部添加白名单命令失败: %1").arg(e.what()));
    }
}

void BeidouCommHandler::onDelWhitelistBtnInClicked(DeviceId deviceId)
{
    if (!m_module) return;
    BeidouCommUIControls& controls = m_uiControls[deviceId];
    if (!controls.delWhitelistInputIn) return;

    QString cardIdText = controls.delWhitelistInputIn->text();
    QString cardErrorMsg;
    if (!validateBeidouCardId(cardIdText, cardErrorMsg)) {
        QMessageBox::warning(m_parent, "卡号输入错误", cardErrorMsg);
        return;
    }
    QString cardIdStr = cardIdText.trimmed().rightJustified(7, '0');
    uint32_t cardId = cardIdStr.toUInt();
    auto params = std::make_shared<BeidouCardNoParam>(cardId);

    try {
        m_module->sendCheckCommand(deviceId,
            CommandCode::BeidouComm_DelWhitelist,
            params,
            WorkPhase::ConnectivityTest,
            CommunicationChannel::BDS,
            CommandType::Internal);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("北斗内部删除白名单命令失败: %1").arg(e.what()));
    }
}

void BeidouCommHandler::onGetPositionBtnInClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId,
            CommandCode::BeidouComm_GetPosition,
            params,
            WorkPhase::ConnectivityTest,
            CommunicationChannel::BDS,
            CommandType::Internal);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("北斗内部定位信息查询命令失败: %1").arg(e.what()));
    }
}

void BeidouCommHandler::onGetSysInfoBtnInClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId,
            CommandCode::BeidouComm_GetSysInfo,
            params,
            WorkPhase::ConnectivityTest,
            CommunicationChannel::BDS,
            CommandType::Internal);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("北斗内部系统信息查询命令失败: %1").arg(e.what()));
    }
}

void BeidouCommHandler::onFactoryResetBtnInClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId,
            CommandCode::BeidouComm_FactoryReset,
            params,
            WorkPhase::ConnectivityTest,
            CommunicationChannel::BDS,
            CommandType::Internal);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("北斗内部恢复出厂设置命令失败: %1").arg(e.what()));
    }
}

void BeidouCommHandler::onRebootBtnInClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        m_module->sendCheckCommand(deviceId,
            CommandCode::BeidouComm_Reboot,
            params,
            WorkPhase::ConnectivityTest,
            CommunicationChannel::BDS,
            CommandType::Internal);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("北斗内部重启命令失败: %1").arg(e.what()));
    }
}