#include "acousticcommhandler.h"
#include "widgetmanager.h"
#include <QMessageBox>
#include <QDebug>
#include "selfcheckenum.h"

AcousticCommHandler::AcousticCommHandler(SelfCheckModule* module, QWidget* parent)
    : DeviceHandlerBase(module, parent)
{
    initDefaultDeviceMapping();
    initAllUIControls();
    initializeUI();
}

AcousticCommHandler::~AcousticCommHandler()
{
    disconnectSignals();
}

// 初始化水声通信默认编号映射（15）
void AcousticCommHandler::initDefaultDeviceMapping()
{
    m_deviceNumberMap = { {DeviceId::AcousticComm, "15"} };
}

// 初始化水声通信UI控件
void AcousticCommHandler::AcousticCommUIControls::init(QWidget* parent, const QString& prefix)
{
    // 命令按钮
    powerOnBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_PowerOn").arg(prefix), parent);
    powerOffBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_PowerOff").arg(prefix), parent);
    selfCheckBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_SelfCheck").arg(prefix), parent);
    commTestBtn = WidgetManager::instance().getWidget<QPushButton>(QString("btn_%1_CommTest").arg(prefix), parent);

    // 状态图标
    powerOnIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_PowerOnIcon").arg(prefix), parent);
    powerOffIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_PowerOffIcon").arg(prefix), parent);
    selfCheckIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_SelfCheckIcon").arg(prefix), parent);
    commTestIcon = WidgetManager::instance().getWidget<QLabel>(QString("lbl_%1_CommTestIcon").arg(prefix), parent);

    // 反馈文本框
    powerOnFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_PowerOn_Feedback").arg(prefix), parent);
    powerOffFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_PowerOff_Feedback").arg(prefix), parent);
    selfCheckFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_SelfCheck_Feedback").arg(prefix), parent);
    commTestFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_CommTest_Feedback").arg(prefix), parent);
    sendResultFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_SendResult_Feedback").arg(prefix), parent);
    receiveResultFeedback = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReceiveResult_Feedback").arg(prefix), parent);

    // 数据显示框
    sendDataLe = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_SendData").arg(prefix), parent);
    recvDataLe = WidgetManager::instance().getWidget<QLineEdit>(QString("le_%1_ReceiveData").arg(prefix), parent);
}

void AcousticCommHandler::initAllUIControls()
{
    for (auto it = m_deviceNumberMap.begin(); it != m_deviceNumberMap.end(); ++it) {
        AcousticCommUIControls controls;
        controls.number = it.value();
        controls.init(m_parent, controls.number);
        m_uiControls[it.key()] = controls;
    }
}

// 初始化UI（清空反馈框）
void AcousticCommHandler::initializeUI()
{
    for (auto& controls : m_uiControls) {
        if (controls.powerOnFeedback) controls.powerOnFeedback->clear();
        if (controls.powerOffFeedback) controls.powerOffFeedback->clear();
        if (controls.selfCheckFeedback) controls.selfCheckFeedback->clear();
        if (controls.commTestFeedback) controls.commTestFeedback->clear();
        if (controls.sendResultFeedback) controls.sendResultFeedback->clear();
        if (controls.receiveResultFeedback) controls.receiveResultFeedback->clear();
        if (controls.sendDataLe) controls.sendDataLe->clear();
        if (controls.recvDataLe) controls.recvDataLe->clear();
    }
}

// 连接信号（先断后连，避免重复）
void AcousticCommHandler::connectSignals()
{
    disconnectSignals();

    for (auto it = m_uiControls.begin(); it != m_uiControls.end(); ++it) {
        DeviceId deviceId = it.key();
        AcousticCommUIControls& controls = it.value();

        qDebug() << "[水声通信信号连接] DeviceId:" << static_cast<int>(deviceId)
            << " 前缀:" << controls.number;

        // 连接按钮信号
        if (controls.powerOnBtn) {
            connect(controls.powerOnBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onPowerOnClicked(deviceId); });
        }
        if (controls.powerOffBtn) {
            connect(controls.powerOffBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onPowerOffClicked(deviceId); });
        }
        if (controls.selfCheckBtn) {
            connect(controls.selfCheckBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onSelfCheckClicked(deviceId); });
        }
        if (controls.commTestBtn) {
            connect(controls.commTestBtn, &QPushButton::clicked,
                this, [this, deviceId]() { onCommTestClicked(deviceId); });
        }
    }
}

// 断开所有信号
void AcousticCommHandler::disconnectSignals()
{
    for (auto& controls : m_uiControls) {
        if (controls.powerOnBtn) controls.powerOnBtn->disconnect();
        if (controls.powerOffBtn) controls.powerOffBtn->disconnect();
        if (controls.selfCheckBtn) controls.selfCheckBtn->disconnect();
        if (controls.commTestBtn) controls.commTestBtn->disconnect();
    }
}

// 统一处理水声通信命令结果
void AcousticCommHandler::handleResult(const SelfCheckResult* result)
{
    if (!result) return; // 空指针防护

    DeviceId deviceId = result->deviceId();
    if (!m_uiControls.contains(deviceId)) {
        qWarning() << "不支持的水声通信设备:" << static_cast<uint8_t>(deviceId);
        return;
    }

    processAcousticCommCommand(deviceId, result->commandCode(), result);
}

// 分发处理不同水声通信命令结果
void AcousticCommHandler::processAcousticCommCommand(DeviceId deviceId, CommandCode cmdCode, const SelfCheckResult* result)
{
    const AcousticCommUIControls& controls = m_uiControls[deviceId];
    ResultStatus status = result->status();
    QPixmap icon = getStatusIcon(status);
    QString feedbackDesc = result->feedbackDesc();

    switch (cmdCode) {
    case CommandCode::Common_PowerOn:{
        setIcon(controls.powerOnIcon, icon);
        updateStatusWidget(controls.powerOnFeedback, status, feedbackDesc);
        break;
    }
    case CommandCode::Common_PowerOff:{
        setIcon(controls.powerOffIcon, icon);
        updateStatusWidget(controls.powerOffFeedback, status, feedbackDesc);
        break;
    }
    case CommandCode::AcousticComm_SelfCheck:{
        setIcon(controls.selfCheckIcon, icon);
        updateStatusWidget(controls.selfCheckFeedback, status, feedbackDesc);
        break;
    }
    case CommandCode::AcousticComm_Test:
    {
        // 1. 提取指令类型参数
        int cmdType = result->getParameter("cmd_type").toInt();
        QString feedbackDesc = result->feedbackDesc();

        switch (static_cast<WaterAcousticUplinkCmd>(cmdType)) {
        case WaterAcousticUplinkCmd::SendFinish:
            if (controls.sendResultFeedback) {
                controls.sendResultFeedback->setText(feedbackDesc);
            }
            break;
        case WaterAcousticUplinkCmd::RecvNotify:
            if (controls.receiveResultFeedback) {
                controls.receiveResultFeedback->setText(feedbackDesc);
            }
            break;
        case WaterAcousticUplinkCmd::DataReceived: {

            // 设置状态图标
            setIcon(controls.commTestIcon, icon);
            // 提取发送/接收的十六进制数据
            QString sentData = result->getParameter("sent_data_hex").toString();
            QString recvData = result->getParameter("recv_data_hex").toString();
            // 更新UI
            updateStatusWidget(controls.commTestFeedback, status, feedbackDesc);
            updateStatusWidget(controls.sendDataLe, status, sentData);   // 发送数据
            updateStatusWidget(controls.recvDataLe, status, recvData);   // 接收数据
            break; 
        }
        default:
            break;
        }
        break;
    }
    default:
        qDebug() << "未处理的水声通信命令:" << static_cast<int>(cmdCode);
        break;
    }
}

// 获取设备编号
QString AcousticCommHandler::getDeviceNumber(DeviceId deviceId) const
{
    return m_deviceNumberMap.value(deviceId, "");
}

// 支持的设备
bool AcousticCommHandler::supportsDevice(DeviceId deviceId) const
{
    return m_deviceNumberMap.contains(deviceId);
}

// 设置设备映射
void AcousticCommHandler::setDeviceMapping(const QMap<DeviceId, QString>& mapping)
{
    m_deviceNumberMap = mapping;
    initAllUIControls();
}

// -------------------- 槽函数实现 --------------------
void AcousticCommHandler::onPowerOnClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        // 带WorkPhase参数的命令调用
        m_module->sendCheckCommand(deviceId, CommandCode::Common_PowerOn, params, WorkPhase::ConnectivityTest);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("水声通信上电命令失败: %1").arg(e.what()));
    }
}

void AcousticCommHandler::onPowerOffClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        // 带WorkPhase参数的命令调用
        m_module->sendCheckCommand(deviceId, CommandCode::Common_PowerOff, params, WorkPhase::ConnectivityTest);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("水声通信下电命令失败: %1").arg(e.what()));
    }
}

void AcousticCommHandler::onSelfCheckClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        // 带WorkPhase参数的命令调用
        m_module->sendCheckCommand(deviceId, CommandCode::AcousticComm_SelfCheck, params, WorkPhase::ConnectivityTest);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("水声通信自检命令失败: %1").arg(e.what()));
    }
}

void AcousticCommHandler::onCommTestClicked(DeviceId deviceId)
{
    if (!m_module) return;
    try {
        auto params = std::make_shared<EmptyParameter>();
        // 带WorkPhase和CommunicationChannel参数的命令调用
        m_module->sendCheckCommand(deviceId, CommandCode::AcousticComm_Test, params,
            WorkPhase::ConnectivityTest,
            CommunicationChannel::WaterAcoustic);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(m_parent, "错误", QString("水声通信测试命令失败: %1").arg(e.what()));
    }
}
