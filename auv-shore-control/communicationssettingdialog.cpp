#include"communicationssettingdialog.h"
#include<QSerialPort>
#include<QJsonObject>
#include<QJsonDocument>
#include<QPixmap>
#include"communicationmodule.h"
CommunicationsSettingDialog::CommunicationsSettingDialog(CommunicationModule* module, QWidget* parent )
    : ModuleDialogBase(parent),
    m_communicationModel(module)
{
	ui.setupUi(this);
    initialize();
}
CommunicationsSettingDialog::~CommunicationsSettingDialog()
{

}
void CommunicationsSettingDialog::onResultUpdated(const BaseResult* result)
{
    if (!result)
        return;
   
    //根据来源模块区分结果类型
    switch (result->sourceType())
    {
    case ModuleType::Communication:
        //更新自检结果ui
        handleChannelStatusResult(static_cast<const CommunicationChannelStatusResult*>(result));

        break;
    default:
        break;
    }

}
void CommunicationsSettingDialog::initialize()
{
    m_communicationModel->addObserver(this);
    connect(ui.btn_01_WiredNetActivate, &QPushButton::clicked, this,
        &CommunicationsSettingDialog::on_wiredNetworkActivateBtn_clicked);
    connect(ui.btn_01_WiredNetShut, &QPushButton::clicked, this,
        &CommunicationsSettingDialog::on_wiredNetworkShutBtn_clicked);
	connect(ui.btn_03_RadioActivate, &QPushButton::clicked, this, 
        &CommunicationsSettingDialog::on_radioActivateBtn_clicked);
	connect(ui.btn_03_RadioShut, &QPushButton::clicked, this, 
        &CommunicationsSettingDialog::on_radioShutBtn_clicked);
    connect(ui.btn_04_BDSatActivate, &QPushButton::clicked, this, 
        &CommunicationsSettingDialog::on_bdsActivateBtn_clicked);
    connect(ui.btn_04_BDSatShut, &QPushButton::clicked, this,
        &CommunicationsSettingDialog::on_bdsShutBtn_clicked);
    connect(ui.btn_06_WaterAcousticActivate, &QPushButton::clicked, this,
        &CommunicationsSettingDialog::on_waterAcousticActivateBtn_clicked);
    connect(ui.btn_06_WaterAcousticShut, &QPushButton::clicked, this, 
        &CommunicationsSettingDialog::on_waterAcousticShutBtn_clicked);
    // 加载当前模块的样式表（直接调用接口函数）
    loadStyleSheet("./qss/communicationsettingdlg.qss");
}


void CommunicationsSettingDialog::setConnectionState(QLabel* label, CommunicationChannelStatus state)
{
    QString imagePath;
    QString tooltip;
    switch (state)
    {
    case CommunicationChannelStatus::Initializing:
        imagePath = "./icon/communication/initializing.png";
        break;
    case CommunicationChannelStatus::Running:
        imagePath = "./icon/communication/connected.png";
        break;
    case CommunicationChannelStatus::Stopped:
        imagePath = "./icon/communication/disconnected.png";
        break;
    default:
        break;
    }
    label->setPixmap(QPixmap(imagePath));
}

void CommunicationsSettingDialog::handleChannelStatusResult(const CommunicationChannelStatusResult* result)
{
    CommunicationChannel channel = result->channel();
    CommunicationChannelStatus status = result->channelStatus();
    QString error = result->errorMsg();

    switch (channel)
    {
    case CommunicationChannel::Radio:
        setConnectionState(ui.lbl_03_RadioStatusIcon, status);
        break;
    case CommunicationChannel::WiredNetwork:
        setConnectionState(ui.lbl_01_WiredNetStatusIcon, status);
        break;
    case CommunicationChannel::BDS:
        setConnectionState(ui.lbl_04_BDSatStatusIcon, status);
        break;
    case CommunicationChannel::WaterAcoustic:
        setConnectionState(ui.lbl_06_WaterAcousticStatusIcon, status);
        break;
    default:
        break;
    }
}

void CommunicationsSettingDialog::on_radioActivateBtn_clicked()
{
    //设置数传电台参数
    int baudRate = ui.cb_03_RadioBaudRate->currentText().toInt();
    QString portName = ui.cb_03_RadioPort->currentText();
    QSerialPort::DataBits dataBits = static_cast<QSerialPort::DataBits>(QSerialPort::Data8);
    QSerialPort::Parity parity = static_cast<QSerialPort::Parity>(QSerialPort::NoParity);
    QSerialPort::StopBits stopBits = static_cast<QSerialPort::StopBits>(QSerialPort::OneStop);
    QSerialPort::FlowControl flowControl = static_cast<QSerialPort::FlowControl>(QSerialPort::NoFlowControl);

    //构建配置参数json
    QJsonObject radioConfig;
    radioConfig["baudRate"] = baudRate;
    radioConfig["portName"] = portName;
    radioConfig["dataBits"] = static_cast<int>(dataBits);
    radioConfig["parity"] = static_cast<int>(parity);
    radioConfig["stopBits"] = static_cast<int>(stopBits);
    radioConfig["flowControl"] = static_cast<int>(flowControl);
    //构建命令参数
    QJsonObject cmdParams;
    cmdParams["channel"] = static_cast<int>(CommunicationChannel::Radio);
    cmdParams["config"] = radioConfig;

    //发送命令到CommunicationModule
    m_communicationModel->sendCommand(
        "CommunicationModule",
        CommunicationChannel::Radio,
        "set_config",
        cmdParams);
}
void CommunicationsSettingDialog::on_radioShutBtn_clicked()
{

    //构建命令参数
    QJsonObject cmdParams;
    cmdParams["channel"] = static_cast<int>(CommunicationChannel::Radio);
    //发送命令到CommunicationModule
    m_communicationModel->sendCommand(
        "CommunicationModule",
        CommunicationChannel::Radio,
        "close_channel",
        cmdParams);
}

void CommunicationsSettingDialog::on_wiredNetworkActivateBtn_clicked()
{
    // 从UI获取有线网络配置参数
    QString host = ui.le_01_WiredNetIp->text(); // 主机地址（IP或域名）
    int port = ui.le_01_WiredNetPort->text().toInt();    // 端口号（如FTP默认21，HTTP默认80）
    int timeout = 50000; // 超时时间（毫秒）

    // FTP相关配置（从UI获取）
    QString ftpUsername = ui.le_01_WiredNetUserName->text();       // FTP用户名
    QString ftpPassword = ui.le_01_WiredNetPwd->text();       // FTP密码
    bool ftpPassiveMode = true; // FTP被动模式（推荐开启）

    // 构建有线网络配置JSON（复用WiredNetworkConfig类）
    WiredNetworkConfig wiredConfig;
    wiredConfig.m_host = host;
    wiredConfig.m_port = port;
    wiredConfig.m_timeout = timeout;
    wiredConfig.m_ftpUsername = ftpUsername;
    wiredConfig.m_ftpPassword = ftpPassword;
    wiredConfig.m_ftpPassiveMode = ftpPassiveMode;
    QJsonObject wiredConfigJson = wiredConfig.toJson(); // 利用已有toJson()方法

    // 构建命令参数（与Radio格式一致）
    QJsonObject cmdParams;
    cmdParams["channel"] = static_cast<int>(CommunicationChannel::WiredNetwork); // 假设枚举有WiredNetwork
    cmdParams["config"] = wiredConfigJson;

    // 发送配置命令到CommunicationModule（与Radio逻辑一致）
    CommunicationModule::instance().sendCommand(
        "CommunicationModule",          // 目标模块名（与Radio保持一致）
        CommunicationChannel::WiredNetwork, // 有线网络通道
        "set_config",                   // 命令：设置配置（与Radio一致）
        cmdParams);
}

void CommunicationsSettingDialog::on_wiredNetworkShutBtn_clicked()
{
    // 构建关闭命令参数（仅需指定通道，无需详细配置）
    QJsonObject cmdParams;
    cmdParams["channel"] = static_cast<int>(CommunicationChannel::WiredNetwork);

    // 发送关闭命令到CommunicationModule
    CommunicationModule::instance().sendCommand(
        "CommunicationModule",
        CommunicationChannel::WiredNetwork,
        "close_channel",                        // 命令：关闭通道（与Radio关闭逻辑对应）
        cmdParams);
}

void CommunicationsSettingDialog::on_bdsActivateBtn_clicked()
{
    //设置bds参数
    int baudRate = ui.cb_04_BDSatBaudRate->currentText().toInt();
    QString portName = ui.cb_04_BDSatPort->currentText();
    QSerialPort::DataBits dataBits = static_cast<QSerialPort::DataBits>(QSerialPort::Data8);
    QSerialPort::Parity parity = static_cast<QSerialPort::Parity>(QSerialPort::NoParity);
    QSerialPort::StopBits stopBits = static_cast<QSerialPort::StopBits>(QSerialPort::OneStop);
    QSerialPort::FlowControl flowControl = static_cast<QSerialPort::FlowControl>(QSerialPort::NoFlowControl);

    //构建配置参数json
    QJsonObject bdsConfig;
    bdsConfig["baudRate"] = baudRate;
    bdsConfig["portName"] = portName;
    bdsConfig["dataBits"] = static_cast<int>(dataBits);
    bdsConfig["parity"] = static_cast<int>(parity);
    bdsConfig["stopBits"] = static_cast<int>(stopBits);
    bdsConfig["flowControl"] = static_cast<int>(flowControl);
    //构建命令参数
    QJsonObject cmdParams;
    cmdParams["channel"] = static_cast<int>(CommunicationChannel::BDS);
    cmdParams["config"] = bdsConfig;

    //发送命令到CommunicationModule
    m_communicationModel->sendCommand(
        "CommunicationModule",
        CommunicationChannel::BDS,
        "set_config",
        cmdParams);
}

void CommunicationsSettingDialog::on_bdsShutBtn_clicked()
{
    //构建命令参数
    QJsonObject cmdParams;
    cmdParams["channel"] = static_cast<int>(CommunicationChannel::BDS);
 
    //发送命令到CommunicationModule
    m_communicationModel->sendCommand(
        "CommunicationModule",
        CommunicationChannel::BDS,
        "close_channel",
        cmdParams);
}

void CommunicationsSettingDialog::on_waterAcousticActivateBtn_clicked()
{
    //设置bds参数
    int baudRate = ui.cb_06_WaterAcousticBaudRate->currentText().toInt();
    QString portName = ui.cb_06_WaterAcousticPort->currentText();
    QSerialPort::DataBits dataBits = static_cast<QSerialPort::DataBits>(QSerialPort::Data8);
    QSerialPort::Parity parity = static_cast<QSerialPort::Parity>(QSerialPort::NoParity);
    QSerialPort::StopBits stopBits = static_cast<QSerialPort::StopBits>(QSerialPort::OneStop);
    QSerialPort::FlowControl flowControl = static_cast<QSerialPort::FlowControl>(QSerialPort::NoFlowControl);

    //构建配置参数json
    QJsonObject waterAcousticConfig;
    waterAcousticConfig["baudRate"] = baudRate;
    waterAcousticConfig["portName"] = portName;
    waterAcousticConfig["dataBits"] = static_cast<int>(dataBits);
    waterAcousticConfig["parity"] = static_cast<int>(parity);
    waterAcousticConfig["stopBits"] = static_cast<int>(stopBits);
    waterAcousticConfig["flowControl"] = static_cast<int>(flowControl);
    //构建命令参数
    QJsonObject cmdParams;
    cmdParams["channel"] = static_cast<int>(CommunicationChannel::WaterAcoustic);
    cmdParams["config"] = waterAcousticConfig;

    //发送命令到CommunicationModule
    m_communicationModel->sendCommand(
        "CommunicationModule",
        CommunicationChannel::WaterAcoustic,
        "set_config",
        cmdParams);
}

void CommunicationsSettingDialog::on_waterAcousticShutBtn_clicked()
{
    //构建命令参数
    QJsonObject cmdParams;
    cmdParams["channel"] = static_cast<int>(CommunicationChannel::WaterAcoustic);
    //发送命令到CommunicationModule
    m_communicationModel->sendCommand(
        "CommunicationModule",
        CommunicationChannel::WaterAcoustic,
        "close_channel",
        cmdParams);
}
