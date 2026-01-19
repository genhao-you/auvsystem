#include "communicationmodule.h"
#include "communicationmanager.h"
#include "configworker.h"
#include "commandparameter.h"
#include "selfcheckenum.h"
#include "enumconverter.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QThread>
#include <QFileInfo>
#include <memory>       // 智能指针
#include <cmath>        // llround/round
#include <stdexcept>    // invalid_argument
#include <QtEndian>     // 字节序转换

CommunicationModule& CommunicationModule::instance()
{
    static CommunicationModule instance;
    return instance;
}
CommunicationModule::CommunicationModule(QObject* parent)
    : BaseModule(ModuleType::Communication, "CommunicationModule", parent)
{
    m_commManager = &CommunicationManager::instance();
    initialize();
    
}

CommunicationModule::~CommunicationModule()
{
    shutdown();
}

bool CommunicationModule::initialize()
{
    if (!BaseModule::initialize()) return false;

    // 1. 初始化通道状态缓存
    m_channelStatusCache.insert(CommunicationChannel::WiredNetwork, CommunicationChannelStatus::Stopped);
    m_channelStatusCache.insert(CommunicationChannel::WirelessNetwork, CommunicationChannelStatus::Stopped);
    m_channelStatusCache.insert(CommunicationChannel::Radio, CommunicationChannelStatus::Stopped);
    m_channelStatusCache.insert(CommunicationChannel::BDS, CommunicationChannelStatus::Stopped);
    m_channelStatusCache.insert(CommunicationChannel::WaterAcoustic, CommunicationChannelStatus::Stopped);

    // 注册命令处理器（外部模块可通过sendCommand调用）
    registerCommandHandlers();
    registerDataReceivers();
    subscribeStatuses();
    // 连接CommunicationManager的信号（监控通道状态）
    connect(m_commManager, &CommunicationManager::channelStatusChanged,
        this, &CommunicationModule::onChannelStatusChanged);
    connect(m_commManager, &CommunicationManager::channelError,
        this, &CommunicationModule::onChannelError);
    connect(m_commManager, &CommunicationManager::responseReceived,
        this, &CommunicationModule::onHardwareRespond);
    connect(m_commManager, &CommunicationManager::ftpFileListFetched,
        this, &CommunicationModule::onFTPFileListRespond);
    connect(m_commManager, &CommunicationManager::ftpUploadProgress,
        this, &CommunicationModule::onFtpUploadProgress);
    connect(m_commManager, &CommunicationManager::ftpDownloadProgress,
        this, &CommunicationModule::onFtpDownloadProgress);
    qInfo() << "CommunicationModule初始化完成";
    return true;
}

void CommunicationModule::shutdown()
{
    qInfo() << "CommunicationModule shutdown：清理所有发送线程";
    // 遍历所有子对象，等待线程结束
    for (auto child : children()) {
        QThread* thread = qobject_cast<QThread*>(child);
        if (thread) {
            thread->quit();
            thread->wait(500); // 等待500ms
            if (thread->isRunning()) {
                thread->terminate();
            }
        }
    }
    BaseModule::shutdown();
    qInfo() << "CommunicationModule shutdown";
}


void CommunicationModule::registerCommandHandlers()
{
    //注册"send_selfcheck_Command"命令(自检模块调用此命令发送数据)
    registerCommandHandler("send_control_command", [this](const DataMessage& msg)
        {
            handleControlCommand(msg);
        });

    // 注册"switch_channel"命令的处理器
    registerCommandHandler("switch_channel",
        [this](const DataMessage& msg) {
            handleSwitchChannel(msg);
        });

    // 注册"set_config"命令的处理器
    registerCommandHandler("set_config",
        [this](const DataMessage& msg) {
            handleSetChannelConfig(msg);
        });

    //注册"close"命令的处理器
    registerCommandHandler("close_channel",
        [this](const DataMessage& msg) {
            handleChannelClose(msg);
        });
    // 注册"query_status"命令的处理器
    registerCommandHandler("query_status",
        [this](const DataMessage& msg) {
            handleQueryChannelStatus(msg);
        });
    registerCommandHandler("stop",
        [this](const DataMessage& msg) {
            handleStopChannel(msg);
        });

}

void CommunicationModule::registerDataReceivers()
{
    registerDataReceiver("missionIssue_transfer",
        [this](const DataMessage& msg)
        {
            handleMissionIssueTransfer(msg);
        });

    registerDataReceiver("file_download_transfer",
        [this](const DataMessage& msg)
        {
            handleFileDownloadTransfer(msg);
        });
    registerDataReceiver("file_upload_transfer",
        [this](const DataMessage& msg)
        {
            handleFileUploadTransfer(msg);
        });

    registerDataReceiver("filelist_get_transfer",
        [this](const DataMessage& msg)
        {
            handleFileListGetTransfer(msg);
        });

    registerDataReceiver("file_delete_transfer",
        [this](const DataMessage& msg)
        {
            handleFileDeleteTransfer(msg);
        });
}

void CommunicationModule::subscribeStatuses()
{
    subscribeStatus("channel_status",
        [this](const DataMessage& msg) {
            onChannelStatusRespond(msg);
        });
}

bool CommunicationModule::parseOuterJson(const QByteArray& data, QJsonObject& outJson)
{
    QJsonDocument outerDoc = QJsonDocument::fromJson(data);
    if (outerDoc.isNull()) {
        qWarning() << "JSON解析失败：" << data;
        return false;
    }
    outJson = outerDoc.object();
    return true;
}

void CommunicationModule::publishConfigResult(CommunicationChannel channel, bool success, const QString& msg)
{
    QJsonObject result;
    result["channel"] = static_cast<int>(channel);
    result["success"] = success;
    result["message"] = msg;

    publishEvent(
        channel,
        "channel_config_result",
        result
    );
}

void CommunicationModule::publishStopResult(CommunicationChannel channel, bool success, const QString& msg)
{
    QJsonObject result;
    result["channel"] = static_cast<int>(channel);
    result["success"] = success;
    result["message"] = msg;

    publishEvent(
        channel,
        "channel_stop_result",
        result
    );
}

void CommunicationModule::publishControlResult(CommunicationChannel channel, bool success, const QString& msg,
    DeviceId deviceId, CommandCode cmdCode, uint8_t packetSeq,
    ModuleType sourceModule, uint64_t requestId)
{
    QJsonObject result;
    result["success"] = success;
    result["message"] = msg;
    // 核心标识：供发送模块过滤和匹配
    result["source_module"] = static_cast<int>(sourceModule);
    result["request_id"] = QString::number(requestId);
    result["device_id"] = static_cast<int>(deviceId); // 元信息：设备ID
    result["cmd_code"] = static_cast<int>(cmdCode);   // 元信息：命令码
    result["packet_seq"] = packetSeq;                 // 元信息：包序号
    result["timestamp"] = QDateTime::currentDateTime().toString("yyMMddhhmmss");
    publishEvent(channel, "selfcheck_send_result", result); // 独立事件类型
}

void CommunicationModule::publishCommandError(const DataMessage& msg, const QString& errorMsg,
    DeviceId deviceId, CommandCode cmdCode, uint8_t packetSeq,
    ModuleType sourceModule)
{
    publishControlResult(msg.channel(), false, errorMsg,
        deviceId, cmdCode, packetSeq, sourceModule, 0); // requestId为0表示错误
}

void CommunicationModule::publishTransferResult(CommunicationChannel channel, 
    bool success, const QString& msg, ModuleType sourceModule, 
    uint64_t requestId, const QString& operationType)
{
    QJsonObject result;
    result["success"] = success;
    result["message"] = msg;
    // 核心：添加模块标识和请求ID
    result["source_module"] = static_cast<int>(sourceModule);
    result["request_id"] = QString::number(requestId);
    result["timestamp"] = QDateTime::currentDateTime().toString("yyMMddhhmmss");
    result["operation_type"] = operationType; // 操作类型：upload/download/mission
    publishEvent(channel, "file_transfer_result", result); // 独立事件类型
}

void CommunicationModule::sendFrameAsyncForControl(CommunicationChannel channel, const QByteArray& frame,
    DeviceId deviceId, CommandCode cmdCode, uint8_t packetSeq,
    ModuleType sourceModule, uint64_t requestId, BDSDataType bdsDataType)
{
    //调用物理层发送数据
    QThread* sendThread = new QThread(this);
    ConfigWorker* worker = new ConfigWorker(channel, frame, m_commManager, bdsDataType);
    qDebug() << "发送字节数" << frame.size();
    qDebug() << "发送：" << frame.toHex(' ');
    worker->moveToThread(sendThread);
    //连接线程启动信号到工作对象的配置函数
    connect(sendThread, &QThread::started, worker, &ConfigWorker::doSendData);
    //连接配置完成信号到主线程函数
    connect(worker, &ConfigWorker::dataSendFinished, this, [=](bool sendSuccess, CommunicationChannel channel)
        {
            QString resultMsg = sendSuccess ? "命令发送成功" : "命令发送失败（物理层超时或通道未运行）";
            publishControlResult(channel, sendSuccess, resultMsg,
                deviceId, cmdCode, packetSeq, sourceModule, requestId);

            // 先请求线程退出，再等待退出完成（关键：避免线程强制销毁）
            sendThread->quit();
            if (!sendThread->wait(1000)) { // 等待1秒，确保线程退出
                qWarning() << "发送线程退出超时，强制终止";
                sendThread->terminate();
            }
        }, Qt::QueuedConnection);

    // 线程结束后销毁资源（顺序：先销毁worker，再销毁线程）
    connect(sendThread, &QThread::finished, worker, &ConfigWorker::deleteLater);
    connect(sendThread, &QThread::finished, sendThread, &QThread::deleteLater);

    sendThread->start();
}


void CommunicationModule::handleControlCommand(const DataMessage& msg)
{
    QJsonObject outerJson;
    if (!parseOuterJson(msg.data(), outerJson)) {
        qWarning() << "控制命令外层JSON解析失败";
        return;
    }

    if (!outerJson.contains("params") || !outerJson["params"].isObject()) {
        qWarning() << "控制命令缺少params字段";
        return;
    }
    QJsonObject params = outerJson["params"].toObject();

    // 解析发送模块标识（核心区分依据）
    if (!params.contains("source_module")) {
        qWarning() << "控制命令缺少source_module字段";
        return;
    }
    // 从参数中提取发送模块标识（核心：区分来源的依据）
    int sourceModuleInt = params["source_module"].toInt();
    ModuleType sourceModule = static_cast<ModuleType>(sourceModuleInt);
   
    // 3. 根据发送模块分发到对应的处理函数
    switch (sourceModule) {
    case ModuleType::SelfCheck:
        handleSelfCheckSendCmd(msg.channel(), params); // 自检模块命令处理
        break;
    case ModuleType::TaskPlanning:
           // 监控模块命令处理
        break;
    case ModuleType::ControlParamDebugging:
        handleControlParamSendCmd(msg.channel(), params);
        break;
    case ModuleType::SailControl:
        handleSailControlSendCmd(msg.channel(), params);
        break;
    default:
        qWarning() << "未知发送模块，命令忽略：" << static_cast<int>(sourceModule);
        break;
    }
}

void CommunicationModule::handleSelfCheckSendCmd(CommunicationChannel channel, const QJsonObject& params)
{   
    /* ------------------------------ 1. 解析的cmd_type字段（区分内部/外部指令）*/
    int cmdTypeInt = params["cmd_type"].toInt(static_cast<int>(CommandType::External));
    CommandType cmdType = static_cast<CommandType>(cmdTypeInt);

    /* ------------------------------ 2. 解析发送端传递的核心字段（与发送端对应）*/
    if (!params.contains("data_body")) {
        publishCommandError(DataMessage(), "自检命令缺少data_body字段",
            DeviceId::Unknown, CommandCode::Invalid, 0, ModuleType::SelfCheck);
        return;
    }
    QByteArray dataBody = QByteArray::fromBase64(params["data_body"].toString().toUtf8());
    // 解析包序号（用于硬件帧）
    uint8_t packetSeq = static_cast<uint8_t>(params["packet_seq"].toInt(0));
    if (packetSeq == 0) {
        publishCommandError(DataMessage(), "packet_seq无效（必须>0）",
            DeviceId::Unknown, CommandCode::Invalid, 0, ModuleType::SelfCheck);
        return;
    }
    uint64_t requestId = params["request_id"].toString().toULongLong(); // 解析全局唯一ID
    // 校验request_id有效性
    if (requestId == 0) {
        qWarning() << "控制命令request_id无效（必须>0）";
        return;
    }
    WorkPhase phase = static_cast<WorkPhase>(params["work_phase"].toInt(static_cast<int>(WorkPhase::ConnectivityTest)));


    // 2. 内部指令vs外部指令处理
    DeviceId deviceId = DeviceId::Unknown;
    CommandCode cmdCode = CommandCode::Invalid;
    BDSDataType bdsDataType = BDSDataType::ATCommand;
    if (channel == CommunicationChannel::BDS && params.contains("bds_data_type")) {
        bdsDataType = static_cast<BDSDataType>(params["bds_data_type"].toInt(static_cast<int>(BDSDataType::ATCommand)));
    }
    QByteArray sendData = dataBody;
    if (cmdType == CommandType::Internal) {
        // 内部指令：从params中获取deviceId（发送端已携带）
        deviceId = static_cast<DeviceId>(params["device_id"].toInt(static_cast<int>(DeviceId::Unknown)));
        cmdCode = static_cast<CommandCode>(params["cmd_code"].toInt(static_cast<int>(CommandCode::Invalid)));
        if (deviceId == DeviceId::Unknown || cmdCode == CommandCode::Invalid) {
            publishCommandError(DataMessage(), "内部指令设备ID或命令码无效",
                deviceId, cmdCode, packetSeq, ModuleType::SelfCheck);
            return;
        }
        // 内部指令无需构建帧，直接发送适配器生成的字节流
        qDebug() << "[内部指令] 设备：" << EnumConverter::deviceIdToString(deviceId)
            << " 命令：" << static_cast<int>(cmdCode)
            << " 字节流：" << sendData.toHex(' ');
    }
    else
    {
        // 外部指令：（解析器件ID+命令码+构建帧）
        if (dataBody.size() < 2) {
            publishCommandError(DataMessage(), "data_body长度不足（至少2字节）",
                DeviceId::Unknown, CommandCode::Invalid, packetSeq, ModuleType::SelfCheck);
            return;
        }
        // ------------------------------ 2. 从data_body中提取器件ID、命令码、参数
        deviceId = static_cast<DeviceId>(static_cast<uint8_t>(dataBody[0]));
        cmdCode = static_cast<CommandCode>(static_cast<uint8_t>(dataBody[1]));
        QByteArray paramBytes = dataBody.mid(2);
        // 基础校验
        if (deviceId == DeviceId::Unknown || cmdCode == CommandCode::Invalid) {
            publishCommandError(DataMessage(), "器件ID或命令码无效",
                deviceId, cmdCode, packetSeq, ModuleType::SelfCheck);
            return;
        }
        // ------------------------------ 4. 根据命令码+设备类型解析参数
        std::unique_ptr<CommandParameter> param;
        try {
            switch (cmdCode) {
                // ------------------------------ 通用命令（0x01-0x05，含北斗复用）
            case CommandCode::Common_PowerOn: // 0x01：通用上电 / 北斗获取信号质量
                if (deviceId == DeviceId::BeidouComm) {
                    // 北斗：获取信号质量（无参）
                    param = std::make_unique<EmptyParameter>();
                }
                else {
                    // 通用设备：上电（无参）
                    param = std::make_unique<EmptyParameter>();
                }
                break;

            case CommandCode::Common_PowerOff: // 0x02：通用下电 / 北斗身份查询
                if (deviceId == DeviceId::BeidouComm) {
                    // 北斗：身份查询（无参）
                    param = std::make_unique<EmptyParameter>();
                }
                else {
                    // 通用设备：下电（无参）
                    param = std::make_unique<EmptyParameter>();
                }
                break;

            case CommandCode::Common_Enable: // 0x04：通用使能 / 北斗设置目标
                if (deviceId == DeviceId::BeidouComm) {
                    // 北斗：设置目标卡号（有参，4字节uint32_t大端序）
                    if (paramBytes.size() != 4) {
                        throw std::invalid_argument("北斗目标卡号参数长度错误（应为4字节）");
                    }
                    uint32_t cardNo = qFromBigEndian(*reinterpret_cast<const uint32_t*>(paramBytes.data()));
                    param = std::make_unique<BeidouCardNoParam>(cardNo);
                }
                else {
                    // 通用设备：使能（无参）
                    param = std::make_unique<EmptyParameter>();
                }
                break;

            case CommandCode::Common_Disable: // 0x05：通用停止 / 推进器设置转速 / 北斗查询白名单
                if (deviceId == DeviceId::BeidouComm) {
                    // 北斗：查询白名单（无参）
                    param = std::make_unique<EmptyParameter>();
                }
                else if (DeviceTypeHelper::isThruster(deviceId)) {
                    // 推进器：设置转速（2字节int16_t）
                    if (paramBytes.size() != 2) {
                        throw std::invalid_argument("转速参数长度错误（应为2字节）");
                    }
                    int16_t rpm = qFromBigEndian(*reinterpret_cast<const int16_t*>(paramBytes.data()));
                    param = std::make_unique<SpeedParameter>(rpm);
                }
                else {
                    // 其他设备：停止工作（无参）
                    param = std::make_unique<EmptyParameter>();
                }
                break;

                // ------------------------------ 推进器/舵机/浮调命令（0x03，多设备复用）
            case CommandCode::Thruster_ParamReset: // 0x03：推进器参数重置 / 舵机设置角度 / 浮调归零 / 北斗目标查询
                if (deviceId == DeviceId::BeidouComm) {
                    // 北斗：目标查询（无参）
                    param = std::make_unique<EmptyParameter>();
                }
                else if (DeviceTypeHelper::isThruster(deviceId)) {
                    // 推进器：参数重置（无参）
                    param = std::make_unique<EmptyParameter>();
                }
                else if (DeviceTypeHelper::isRudder(deviceId)) {
                    // 舵机：设置角度（2字节uint16_t，*0.1度）
                    if (paramBytes.size() != 2) {
                        throw std::invalid_argument("舵机角度参数长度错误（应为2字节）");
                    }
                    int16_t angleTenthDegree = qFromBigEndian(*reinterpret_cast<const int16_t*>(paramBytes.data()));
                    param = std::make_unique<RudderAngleParameter>(angleTenthDegree);
                }

                else if (DeviceTypeHelper::isBuoyancy(deviceId)) {
                    // 浮调：设置浮力（2字节）
                    if (paramBytes.size() != 2) {
                        throw std::invalid_argument("浮调参数长度错误（应为2字节）");
                    }
                    uint16_t buoyancyValue = qFromBigEndian(*reinterpret_cast<const uint16_t*>(paramBytes.data()));
                    param = std::make_unique<BuoyancyVolumeParameter>(buoyancyValue);
                }
                else if (deviceId == DeviceId::CameraLight) {
                    // 相机：开始存储视频（2字节序号）
                    if (paramBytes.size() != 2) {
                        throw std::invalid_argument("视频序号参数长度错误（应为2字节）");
                    }
                    uint16_t seq = qFromBigEndian(*reinterpret_cast<const uint16_t*>(paramBytes.data()));
                    param = std::make_unique<CameraMediaParam>(seq);
                }
                else if (deviceId == DeviceId::INS) {
                    if (paramBytes.size() != 8)
                    {
                        throw std::invalid_argument(
                            QString("惯导GNSS参数长度错误：需8字节，实际%1字节").arg(paramBytes.size()).toStdString()
                        );
                    }

                    // 显式解析，无任何隐式转换
                    int32_t lon = qFromBigEndian<int32_t>((uchar*)paramBytes.data());
                    int32_t lat = qFromBigEndian<int32_t>((uchar*)paramBytes.data() + 4);

                    // 直接new，手动封装unique_ptr
                    CommandParameter* raw = new InsGnssParam(lat, lon);
                    param = std::unique_ptr<CommandParameter>(raw); // 显式指定基类
                }
                else {
                    param = std::make_unique<EmptyParameter>();
                }
                break;

                // ------------------------------ 推进器命令（0x06-0x11，含北斗复用）
            case CommandCode::Thruster_SetDuty: // 0x06：推进器设置占空比 / 北斗添加白名单
                if (deviceId == DeviceId::BeidouComm) {
                    // 北斗：添加白名单（有参，4字节uint32_t大端序）
                    if (paramBytes.size() != 4) {
                        throw std::invalid_argument("北斗白名单卡号参数长度错误（应为4字节）");
                    }
                    uint32_t cardNo = qFromBigEndian(*reinterpret_cast<const uint32_t*>(paramBytes.data()));
                    param = std::make_unique<BeidouCardNoParam>(cardNo);
                }
                else if (DeviceTypeHelper::isThruster(deviceId)) {
                    // 推进器：设置占空比（2字节uint16_t，0-2000）
                    if (paramBytes.size() != 2) {
                        throw std::invalid_argument("推进器占空比参数长度错误（应为2字节）");
                    }
                    uint16_t duty = qFromBigEndian(*reinterpret_cast<const uint16_t*>(paramBytes.data()));
                    param = std::make_unique<ThrusterDutyParameter>(duty);
                }
                else {
                    param = std::make_unique<EmptyParameter>();
                }
                break;

            case CommandCode::Thruster_SetRunTime: // 0x07：推进器设置运行时间 / 北斗删除白名单
                if (deviceId == DeviceId::BeidouComm) {
                    // 北斗：删除白名单（有参，4字节uint32_t大端序）
                    if (paramBytes.size() != 4) {
                        throw std::invalid_argument("北斗白名单卡号参数长度错误（应为4字节）");
                    }
                    uint32_t cardNo = qFromBigEndian(*reinterpret_cast<const uint32_t*>(paramBytes.data()));
                    param = std::make_unique<BeidouCardNoParam>(cardNo);
                }
                else if (DeviceTypeHelper::isThruster(deviceId)) {
                    // 推进器：设置运行时间（2字节uint16_t）
                    if (paramBytes.size() != 2) {
                        throw std::invalid_argument("推进器运行时间参数长度错误（应为4字节）");
                    }
                    uint16_t runSeconds = qFromBigEndian(*reinterpret_cast<const uint16_t*>(paramBytes.data()));
                    param = std::make_unique<ThrusterRunTimeParameter>(runSeconds);
                }
                else {
                    param = std::make_unique<EmptyParameter>();
                }
                break;

            case CommandCode::Thruster_SetAccelTime: // 0x08：推进器设置加速时间 / 北斗获取定位
                if (deviceId == DeviceId::BeidouComm) {
                    // 北斗：获取定位信息（无参）
                    param = std::make_unique<EmptyParameter>();
                }
                else if (DeviceTypeHelper::isThruster(deviceId)) {
                    // 推进器：设置加速时间（2字节uint16_t）
                    if (paramBytes.size() != 2) {
                        throw std::invalid_argument("加速时间参数长度错误（应为2字节）");
                    }
                    uint16_t accelTimeMs = qFromBigEndian<uint16_t>(paramBytes.data());
                    param = std::make_unique<AccelTimeParameter>(accelTimeMs);
                }
                else {
                    param = std::make_unique<EmptyParameter>();
                }
                break;

            case CommandCode::Thruster_GetRunTime: // 0x09：推进器获取运行时间 / 北斗获取系统信息
                if (deviceId == DeviceId::BeidouComm) {
                    // 北斗：获取系统信息（无参）
                    param = std::make_unique<EmptyParameter>();
                }
                else {
                    // 推进器/舵机：获取运行时间（无参）
                    param = std::make_unique<EmptyParameter>();
                }
                break;

            case CommandCode::Thruster_GetDuty: // 0x0A：推进器获取占空比 / 北斗恢复出厂
                if (deviceId == DeviceId::BeidouComm) {
                    // 北斗：恢复出厂设置（无参）
                    param = std::make_unique<EmptyParameter>();
                }
                else {
                    // 推进器：获取占空比（无参）
                    param = std::make_unique<EmptyParameter>();
                }
                break;

            case CommandCode::Thruster_GetCurrent: // 0x0B：推进器获取电流 / 北斗重启
                if (deviceId == DeviceId::BeidouComm) {
                    // 北斗：设备重启（无参）
                    param = std::make_unique<EmptyParameter>();
                }
                else {
                    // 推进器：获取电流（无参）
                    param = std::make_unique<EmptyParameter>();
                }
                break;

            case CommandCode::Thruster_GetTemperature: // 0x0C：推进器获取温度 / 北斗通讯测试
                if (deviceId == DeviceId::BeidouComm) {
                    // 北斗：通讯测试（无参）
                    param = std::make_unique<EmptyParameter>();
                }
                else {
                    // 推进器：获取温度（无参）
                    param = std::make_unique<EmptyParameter>();
                }
                break;
            case CommandCode::Thruster_SetSpeedWithTime:
                if (deviceId == DeviceId::TailThruster)
                {
                    // 1. 校验参数长度：转速(2字节int16) + 运行时间(1字节uint8) = 3字节
                    if (paramBytes.size() != 3) {
                        throw std::invalid_argument("推进器转速+运行时间参数长度错误（应为3字节）");
                    }
                    // 2. 解析前2字节：转速（int16，大端序）
                    int16_t rpm = qFromBigEndian<int16_t>(paramBytes.data()); // 从大端序解析int16
                    // 3. 解析第3字节：运行时间（uint8，直接取）
                    uint8_t runTimeSec = static_cast<uint8_t>(paramBytes.at(2));
                    // 4. 创建组合参数对象（替换原来的转向参数）
                    param = std::make_unique<ThrusterSpeedAndRunTimeParameter>(rpm, runTimeSec);
                }
                break;
                // ------------------------------ 其他推进器读命令（无参，无复用）
            case CommandCode::Thruster_GetSpeed:
            case CommandCode::Thruster_GetHall:
            case CommandCode::Thruster_GetBusVoltage:
            case CommandCode::Thruster_GetNoMaintainTime:
            case CommandCode::Thruster_GetTotalRunTime:
                param = std::make_unique<EmptyParameter>();
                break;
                // ------------------------------ 通用故障查询
            case CommandCode::Common_GetFault:
                param = std::make_unique<EmptyParameter>();
                break;


                // ------------------------------ 未支持的命令码
            default:
                publishCommandError(DataMessage(),
                    QString("不支持的自检命令码：0x%1").arg(static_cast<uint16_t>(cmdCode), 2, 16, QChar('0')),
                    deviceId, cmdCode, packetSeq, ModuleType::SelfCheck);
                return;
            }
        }
        catch (const std::invalid_argument& e) {
            publishCommandError(DataMessage(), QString("参数解析失败：%1").arg(e.what()),
                deviceId, cmdCode, packetSeq, ModuleType::SelfCheck);
            return;
        }
        catch (...) {
            publishCommandError(DataMessage(), "参数构建失败（未知错误）",
                deviceId, cmdCode, packetSeq, ModuleType::SelfCheck);
            return;
        }
        // ------------------------------ 按通道类型选择帧构建函数 ------------------------------
        if (channel == CommunicationChannel::WaterAcoustic) {
            // 水声通道：构建水声帧
            sendData = FrameBuilder::buildWaterAcousticCheckCommandFrame(deviceId, cmdCode, param->toBytes(), packetSeq);
            qDebug() << "[水声通道] 构建水声帧：" << sendData.toHex(' ');
        }
        else {
            // 其他通道：构建通用帧
            sendData = FrameBuilder::buildCheckCommandFrame(deviceId, cmdCode, phase,param->toBytes(), packetSeq);
            qDebug() << "[通用通道] 构建通用帧：" << sendData.toHex(' ');
        }
    }
  
    sendFrameAsyncForControl(channel, sendData, deviceId, cmdCode, packetSeq,
        ModuleType::SelfCheck, requestId, bdsDataType);
}

void CommunicationModule::handleControlParamSendCmd(CommunicationChannel channelType, const QJsonObject& params)
{

    // ========== 1. 核心参数校验（仅保留控制参数必要字段） ==========
    const QStringList requiredFields = { "request_id", "packet_seq", "data_body", "control_type" };
    for (const QString& field : requiredFields) {
        if (!params.contains(field)) {
            qWarning() << "[控制参数命令] 缺少必要字段：" << field;
            return;
        }
    }

    // ========== 2. 解析基础参数（去掉cmd_type/器件ID相关） ==========
    // 全局请求ID
    uint64_t requestId = params["request_id"].toString().toULongLong();
    if (requestId == 0) {
        qWarning() << "[控制参数命令] request_id非法：" << params["request_id"].toString();
        return;
    }

    // 包序号
    uint8_t packetSeq = static_cast<uint8_t>(params["packet_seq"].toInt());
    if (packetSeq > 0xFF) {
        qWarning() << "[控制参数命令] packet_seq超出范围：" << params["packet_seq"].toInt();
        return;
    }

    // 控制类型（核心）
    int controlTypeInt = params["control_type"].toInt();
    ControlType controlType = static_cast<ControlType>(controlTypeInt);
    if (controlType < ControlType::DepthControl || controlType > ControlType::HeadingControl) {
        qWarning() << "[控制参数命令] 非法控制类型：" << controlTypeInt;
        return;
    }

    // ========== 3. 解码数据体（控制类型+参数，无器件ID） ==========
    QByteArray dataBody = QByteArray::fromBase64(params["data_body"].toString().toUtf8());
    if (dataBody.isEmpty()) {
        qWarning() << "[控制参数命令] data_body Base64解码失败：" << params["data_body"].toString();
        return;
    }
    qDebug() << "[控制参数命令] 原始数据体（控制类型+参数）：" << dataBody.toHex(' ');

    // ========== 4. 拆分控制类型和参数（可选：校验数据体长度） ==========
    if (dataBody.size() < 1) { // 至少包含1字节控制类型
        qWarning() << "[控制参数命令] 数据体过短，无控制类型：" << dataBody.size() << "字节";
        return;
    }
    // 提取控制参数（去掉前1字节控制类型，因为FrameBuilder会重新拼接）
    QByteArray controlParam = dataBody.mid(1);

    // ========== 5. 构建控制参数帧（区分通用通道/水声通道） ==========
    QByteArray frame;
    frame = FrameBuilder::buildControlParamFrame(controlType, controlParam, packetSeq);
    qDebug() << "[控制参数命令] 最终发送帧：" << frame.toHex(' ');

    // ========== 6. 调用物理层异步发送（去掉BDSDataType，通用发送） ==========
    sendFrameAsyncForControl(
        channelType,
        frame,
        DeviceId::Unknown,       // 无器件ID，设为Unknown
        CommandCode::Invalid,       // 无命令码，设为None（需在CommandCode枚举中新增None）
        packetSeq,
        ModuleType::ControlParamDebugging,
        requestId    // 控制参数无BDS类型区分
    );
}

void CommunicationModule::handleSailControlSendCmd(CommunicationChannel channelType, const QJsonObject& params)
{
    qDebug() << "[航行控制] 开始处理航行控制命令...";

    // ========== 1. 核心参数校验 ==========
    const QStringList requiredFields = { "request_id", "packet_seq", "data_body", "sail_cmd_code" };
    for (const QString& field : requiredFields) {
        if (!params.contains(field)) {
            qWarning() << "[航行控制命令] 缺少必要字段：" << field;
            publishCommandError(DataMessage(),
                QString("缺少必要字段：%1").arg(field),
                DeviceId::Unknown, CommandCode::Invalid, 0,
                ModuleType::SailControl);
            return;
        }
    }

    // ========== 2. 解析基础参数 ==========
    // 全局请求ID
    uint64_t requestId = params["request_id"].toString().toULongLong();
    if (requestId == 0) {
        qWarning() << "[航行控制命令] request_id非法：" << params["request_id"].toString();
        publishCommandError(DataMessage(), "request_id非法",
            DeviceId::Unknown, CommandCode::Invalid, 0,
            ModuleType::SailControl);
        return;
    }

    // 包序号
    uint8_t packetSeq = static_cast<uint8_t>(params["packet_seq"].toInt());
    if (packetSeq > 0xFF) {
        qWarning() << "[航行控制命令] packet_seq超出范围：" << params["packet_seq"].toInt();
        publishCommandError(DataMessage(), "packet_seq超出范围",
            DeviceId::Unknown, CommandCode::Invalid, packetSeq,
            ModuleType::SailControl);
        return;
    }

    // 航行控制命令码（核心）
    int sailCmdCodeInt = params["sail_cmd_code"].toInt();
    uint8_t sailCmdCode = static_cast<uint8_t>(sailCmdCodeInt);

    // 验证命令码有效性（只能是AA或BB）
    if (sailCmdCode != 0xAA && sailCmdCode != 0xBB) {
        qWarning() << "[航行控制命令] 非法命令码：" << QString::number(sailCmdCodeInt, 16).toUpper();
        publishCommandError(DataMessage(),
            QString("非法航行控制命令码：0x%1").arg(sailCmdCodeInt, 2, 16, QChar('0')),
            DeviceId::Unknown, CommandCode::Invalid, packetSeq,
            ModuleType::SailControl);
        return;
    }

    // ========== 3. 解码数据体（航行控制只有命令码，1字节） ==========
    QByteArray dataBody = QByteArray::fromBase64(params["data_body"].toString().toUtf8());
    if (dataBody.isEmpty()) {
        qWarning() << "[航行控制命令] data_body Base64解码失败";
        publishCommandError(DataMessage(), "data_body解码失败",
            DeviceId::Unknown, CommandCode::Invalid, packetSeq,
            ModuleType::SailControl);
        return;
    }

    // 验证数据体长度（必须为1字节）
    if (dataBody.size() != 1) {
        qWarning() << "[航行控制命令] 数据体长度错误，应为1字节，实际：" << dataBody.size() << "字节";
        publishCommandError(DataMessage(),
            QString("数据体长度错误（应为1字节，实际%1字节）").arg(dataBody.size()),
            DeviceId::Unknown, CommandCode::Invalid, packetSeq,
            ModuleType::SailControl);
        return;
    }

    // 验证数据体内容是否与命令码一致
    uint8_t dataCmdCode = static_cast<uint8_t>(dataBody[0]);
    if (dataCmdCode != sailCmdCode) {
        qWarning() << "[航行控制命令] 数据体命令码不匹配，参数：0x"
            << QString::number(sailCmdCode, 16).toUpper()
            << "，数据体：0x" << QString::number(dataCmdCode, 16).toUpper();
        publishCommandError(DataMessage(), "命令码与数据体不匹配",
            DeviceId::Unknown, CommandCode::Invalid, packetSeq,
            ModuleType::SailControl);
        return;
    }

    qDebug() << "[航行控制命令] 命令码：0x" << QString::number(sailCmdCode, 16).toUpper()
        << " 包序号：" << packetSeq
        << " 数据体：" << dataBody.toHex(' ');

    // ========== 4. 构建航行控制帧 ==========
    QByteArray frame;
    frame = FrameBuilder::buildSailControlFrame(sailCmdCode, packetSeq);

    qDebug() << "[航行控制命令] 最终发送帧：" << frame.toHex(' ');

    // ========== 5. 调用物理层异步发送 ==========
    sendFrameAsyncForControl(
        channelType,
        frame,
        DeviceId::Unknown,           // 无器件ID
        CommandCode::Invalid,           // 使用转换后的CommandCode
        packetSeq,
        ModuleType::SailControl,
        requestId,
        BDSDataType::ATCommand      // 默认使用AT指令类型
    );
}



// 处理切换通道命令
void CommunicationModule::handleSwitchChannel(const DataMessage& msg) {
    // 解析命令参数
    QJsonObject params = QJsonDocument::fromJson(msg.data()).object();
    CommunicationChannel channel = static_cast<CommunicationChannel>(params["channel"].toInt());

    bool activate = params["active"].toBool(true);
    //调用物理层接口切换通道状态
    bool success = false;
    QString errorMsg;
    if (activate)
    {
        //激活通道
        if (!m_commManager->channelStatus(channel).contains("运行中"))
        {
            success = m_commManager->startChannel(channel);
        }
        else
        {
            success = true;
        }
    }
    else
    {
        m_commManager->stopChannel(channel);
    }

    // 发布命令结果事件
    QJsonObject result;
    result["channel"] = static_cast<int>(channel);
    result["activated"] = activate;
    result["success"] = success;
    result["message"] = success ?
        (activate ? "通道已激活" : "通道已停用") :
        "操作失败：" + errorMsg;

    publishEvent(
        defaultCommunicationChannel(),
        "channel_switch_result",
        result
    );
}

void CommunicationModule::handleSetChannelConfig(const DataMessage& msg)
{
    //1.解析外层json(basemodule封装的格式)
    QJsonObject outerJson = QJsonDocument::fromJson(msg.data()).object();
    QJsonObject cmdParams = outerJson["params"].toObject();//提取内层参数

    //2.获取目标通道和配置json
    CommunicationChannel targetChannel = msg.channel();
    QJsonObject configJson = cmdParams["config"].toObject();

    //3.校验通道有效性
    if (configJson.isEmpty())
    {
        qWarning() << "配置参数无效：通道或配置为空";
        publishConfigResult(targetChannel, false, "参数无效");
        return;
    }


    //4.转换为为物理层需要的channelconfig对象
    ChannelConfigPtr config;
    switch (targetChannel)
    {
    case CommunicationChannel::Radio:
        // 用Qt智能指针创建RadioConfig实例（匹配ChannelConfigPtr类型）
        config = QSharedPointer<RadioConfig>::create();
        break;
    case CommunicationChannel::BDS:
        config = QSharedPointer<BDSConfig>::create();
        break;
    case CommunicationChannel::WiredNetwork:
        config = QSharedPointer<WiredNetworkConfig>::create();
        break;
    case CommunicationChannel::WirelessNetwork:
        config = QSharedPointer<WirelessNetworkConfig>::create();
        break;
    case CommunicationChannel::WaterAcoustic:
        config = QSharedPointer<WaterAcousticConfig>::create();
        break;
    default:
        // 默认值：避免config未初始化（可选，根据业务需求）
        config = nullptr;
        break;
    }
    //根据通道类型设置具体参数
    config->fromJson(configJson);

    QThread* configThread = new QThread(this);
    ConfigWorker* worker = new ConfigWorker(targetChannel, config, m_commManager);
    worker->moveToThread(configThread);
    //连接线程启动信号到工作对象的配置函数
    connect(configThread, &QThread::started, worker, &ConfigWorker::doConfig);
    //连接配置完成信号到主线程函数
    connect(worker, &ConfigWorker::configFinished, this, [=](bool configSuccess, CommunicationChannel channel)
        {
            if (!configSuccess)
            {
                publishConfigResult(channel, false, "配置失败");
                configThread->quit();
                return;
            }

            // 关键优化：配置成功后，通过信号触发启动，而非直接调用
            // 监听启动结果，避免与配置回调嵌套
            CommunicationChannelBase* channelInst = m_commManager->getChannelInstance(channel);
            if (!channelInst) {
                publishConfigResult(channel, false, "配置成功，但通道实例丢失");
                configThread->quit();
                return;
            }

            // 单独连接启动结果回调
            QMetaObject::Connection startConn = connect(
                channelInst, &CommunicationChannelBase::startFinished,
                this, [=](bool startSuccess) {
                    QString msg = startSuccess ? "配置成功且通道启动" : "配置成功，但通道启动失败";
                    publishConfigResult(channel, startSuccess, msg);

                    // 关键修复：断开连接前先检查有效性，或通过三要素断开
                    QObject::disconnect(
                        channelInst, &CommunicationChannelBase::startFinished,
                        this, nullptr // 第三个参数为this，第四个参数为nullptr表示断开该信号与this的所有连接
                    );
                },
                Qt::QueuedConnection
                    );

            // 触发启动（不依赖配置回调的嵌套）
            m_commManager->startChannel(channel);
            configThread->quit();
        }, Qt::QueuedConnection);


    connect(configThread, &QThread::finished, worker, &ConfigWorker::deleteLater);
    connect(configThread, &QThread::finished, configThread, &QThread::deleteLater);
    configThread->start();
}

void CommunicationModule::handleChannelClose(const DataMessage& msg)
{
    //获取目标通道
    CommunicationChannel targetChannel = msg.channel();
    // 启动线程执行关闭（复用 ConfigWorker 的 doStop）
    QThread* stopThread = new QThread(this);
    ConfigWorker* worker = new ConfigWorker(targetChannel, &CommunicationManager::instance());
    worker->moveToThread(stopThread);

    // 连接信号
    connect(stopThread, &QThread::started, worker, &ConfigWorker::doStop);
    connect(worker, &ConfigWorker::stopFinished, this, [=](bool stopSuccess, CommunicationChannel channel) {
        QString msg = stopSuccess ? "通道关闭成功" : "通道关闭失败";
        publishConfigResult(channel, stopSuccess, msg);
        stopThread->quit();
        }, Qt::QueuedConnection);
    connect(stopThread, &QThread::finished, worker, &ConfigWorker::deleteLater);
    connect(stopThread, &QThread::finished, stopThread, &QThread::deleteLater);

    stopThread->start();
}

void CommunicationModule::handleQueryChannelStatus(const DataMessage& msg)
{
    //解析参数:若指定了channel则查询单个，否则查询所有
    QJsonObject params = QJsonDocument::fromJson(msg.data()).object();
    bool queryAll = !params.contains("channel");//是否查询所有通道
    CommunicationChannel targetChannel = static_cast<CommunicationChannel>(params["channel"].toInt(-1));

    QJsonObject statusData;
    if (queryAll)
    {
        //查询所有通道状态
        QJsonArray channelsArray;
        //遍历所有通道类型
        for (int i = static_cast<int>(CommunicationChannel::WiredNetwork);
            i<static_cast<int>(CommunicationChannel::WaterAcoustic);++i)
        {
            CommunicationChannel channel = static_cast<CommunicationChannel>(i);
            QJsonObject channelObj;
            channelObj["channel"] = i;
            channelObj["status"] = m_commManager->channelStatus(channel);
            channelsArray.append(channelObj);
        }
        statusData["channels"] = channelsArray;

    }
    else
    {
        //查询单个通道状态
        statusData["channel"] = static_cast<int>(targetChannel);
        statusData["status"] = m_commManager->channelStatus(targetChannel);
    }

    //发布状态查询结果(通过时间反馈给请求模块)
    publishEvent(
        defaultCommunicationChannel(),
        "channel_status_result",
        statusData
    );
}

void CommunicationModule::handleMissionIssueTransfer(const DataMessage& msg)
{
    // 1. 解析外层JSON（BaseModule::transferData封装的格式）
    QByteArray outerData = msg.data(); // 获取消息原始数据
    QJsonObject outerJson = QJsonDocument::fromJson(outerData).object();
    if (outerJson.isEmpty()) {
        qWarning() << "任务下发：外层JSON解析失败";
        publishTransferResult(msg.channel(), false, "外层数据格式错误（非JSON）",
            ModuleType::Unknown, 0,"upload");
        return;
    }

    // 2. 提取并解码内层data（Base64编码的元数据）
    if (!outerJson.contains("data") || outerJson["data"].type() != QJsonValue::String) {
        qWarning() << "任务下发：外层JSON缺少有效data字段";
        publishTransferResult(msg.channel(), false, "外层数据缺少data字段",
            ModuleType::Unknown, 0, "upload");
        return;
    }
    QString base64Data = outerJson["data"].toString();
    QByteArray metaData = QByteArray::fromBase64(base64Data.toUtf8()); // 解码Base64
    if (metaData.isEmpty()) {
        qWarning() << "任务下发：内层data解码失败（无效Base64）";
        publishTransferResult(msg.channel(), false, "数据解码失败",
            ModuleType::Unknown, 0, "upload");
        return;
    }

    // 3. 解析真实元数据（MissionController封装的meta）
    QJsonObject metaJson = QJsonDocument::fromJson(metaData).object();
    if (metaJson.isEmpty()) {
        qWarning() << "任务下发：元数据解析失败（非JSON）";
        publishTransferResult(msg.channel(), false, "元数据格式错误",
            ModuleType::Unknown, 0, "upload");
        return;
    }

    // 4. 提取核心字段（与MissionController封装的字段对应）
    ModuleType sourceModule = static_cast<ModuleType>(metaJson["source_module"].toInt());
    uint64_t requestId = metaJson["request_id"].toString().toULongLong();
    QString localPath = metaJson["localPath"].toString();
    QString remotePath = metaJson["remotePath"].toString();

    // 3. 校验字段有效性
    if (sourceModule == ModuleType::Unknown || requestId == 0)
    {
        publishTransferResult(msg.channel(), false, "元数据缺少source_module或request_id",
            ModuleType::Unknown, 0, "upload");
        return;
    }
    if (localPath.isEmpty() || remotePath.isEmpty()) {
        publishTransferResult(msg.channel(), false, "本地路径或远程路径为空",
            sourceModule, requestId, "upload");
        return;
    }
    if (!QFile::exists(localPath)) {
        publishTransferResult(msg.channel(), false, QString("本地文件不存在：%1").arg(localPath),
            sourceModule, requestId, "upload");
        return;
    }

    // 4. 启动异步文件上传线程（使用的ConfigWorker构造函数）
    QThread* sendThread = new QThread(this);
    // 初始化Worker：传入路径、模块标识、请求ID等参数
    ConfigWorker* worker = new ConfigWorker(
        msg.channel(),          // 通信通道（如WiredNetwork）
        localPath,              // 本地文件路径
        remotePath,             // 远程文件路径
        sourceModule,           // 来源模块（如TaskPlanning）
        requestId,              // 任务请求ID
        m_commManager           // 通信管理器
    );
    worker->moveToThread(sendThread);

    // 5. 关联Worker结果信号（任务文件上传完成后反馈）
    connect(worker, &ConfigWorker::TaskFileSendFinished, this,
        [=](bool success, CommunicationChannel channel, uint64_t reqId) {
            QString fileName = QFileInfo(localPath).fileName();
            QString resultMsg = success
                ? QString("任务文件 [%1] 下发成功").arg(fileName)
                : QString("任务文件 [%1] 下发失败").arg(fileName);
            publishTransferResult(channel, success, resultMsg, sourceModule, 
                reqId, "upload");
            sendThread->quit(); // 完成后退出线程
        }, Qt::QueuedConnection);

    // 6. 启动线程和任务
    connect(sendThread, &QThread::started, worker, &ConfigWorker::doSendTasksFile);
    // 线程结束后释放资源
    connect(sendThread, &QThread::finished, worker, &ConfigWorker::deleteLater);
    connect(sendThread, &QThread::finished, sendThread, &QThread::deleteLater);

    qDebug() << "任务下发：启动上传线程，本地路径=" << localPath << "，远程路径=" << remotePath;
    sendThread->start();
   
}

void CommunicationModule::handleFileDownloadTransfer(const DataMessage& msg)
{
    // 1. 解析外层JSON（BaseModule::transferData封装的格式）
    QByteArray outerData = msg.data(); // 获取消息原始数据
    QJsonObject outerJson = QJsonDocument::fromJson(outerData).object();
    if (outerJson.isEmpty()) {
        qWarning() << "文件下载：外层JSON解析失败";
        publishTransferResult(msg.channel(), false, "外层数据格式错误（非JSON）",
            ModuleType::Unknown, 0, "download");
        return;
    }

    // 2. 提取并解码内层data（Base64编码的元数据）
    if (!outerJson.contains("data") || outerJson["data"].type() != QJsonValue::String) {
        qWarning() << "文件下载：外层JSON缺少有效data字段";
        publishTransferResult(msg.channel(), false, "外层数据缺少data字段",
            ModuleType::Unknown, 0, "download");
        return;
    }
    QString base64Data = outerJson["data"].toString();
    QByteArray metaData = QByteArray::fromBase64(base64Data.toUtf8()); // 解码Base64
    if (metaData.isEmpty()) {
        qWarning() << "文件下载：内层data解码失败（无效Base64）";
        publishTransferResult(msg.channel(), false, "数据解码失败",
            ModuleType::Unknown, 0, "download");
        return;
    }

    // 3. 解析真实元数据
    QJsonObject metaJson = QJsonDocument::fromJson(metaData).object();
    if (metaJson.isEmpty()) {
        qWarning() << "文件下载：元数据解析失败（非JSON）";
        publishTransferResult(msg.channel(), false, "元数据格式错误", 
            ModuleType::Unknown, 0, "download");
        return;
    }

    // 4. 提取核心字段（与MissionController封装的字段对应）
    ModuleType sourceModule = static_cast<ModuleType>(metaJson["source_module"].toInt());
    uint64_t requestId = metaJson["request_id"].toString().toULongLong();
    QString localPath = metaJson["localPath"].toString();
    QString remotePath = metaJson["remotePath"].toString();

    // 3. 校验字段有效性
    if (sourceModule == ModuleType::Unknown || requestId == 0)
    {
        publishTransferResult(msg.channel(), false, "元数据缺少source_module或request_id",
            ModuleType::Unknown, 0, "download");
        return;
    }
    if (localPath.isEmpty() || remotePath.isEmpty()) {
        publishTransferResult(msg.channel(), false, "本地路径或远程路径为空",
            sourceModule, requestId, "download");
        return;
    }


    // 4. 启动异步文件上传线程（使用的ConfigWorker构造函数）
    QThread* sendThread = new QThread(this);
    // 初始化Worker：传入路径、模块标识、请求ID等参数
    ConfigWorker* worker = new ConfigWorker(
        msg.channel(),          // 通信通道（如WiredNetwork）
        localPath,              // 本地文件路径
        remotePath,             // 远程文件路径
        sourceModule,           // 来源模块（如TaskPlanning）
        requestId,              // 文件下载请求ID
        m_commManager           // 通信管理器
    );
    worker->moveToThread(sendThread);

    // 5. 关联Worker结果信号（文件下载完成后反馈）
    connect(worker, &ConfigWorker::FileDownloadFinished, this,
        [=](bool success, CommunicationChannel channel, uint64_t reqId) {
            QString fileName = QFileInfo(localPath).fileName();
            QString resultMsg = success
                ? QString("文件[%1]下载  成功").arg(fileName)
                : QString("文件[%1]下载  失败").arg(fileName);
            publishTransferResult(channel, success, resultMsg, sourceModule, 
                reqId, "download");
            sendThread->quit(); // 完成后退出线程
        }, Qt::QueuedConnection);

    // 6. 启动线程和任务
    connect(sendThread, &QThread::started, worker, &ConfigWorker::doDownloadFile);
    // 线程结束后释放资源
    connect(sendThread, &QThread::finished, worker, &ConfigWorker::deleteLater);
    connect(sendThread, &QThread::finished, sendThread, &QThread::deleteLater);

    qDebug() << "文件下载：启动下载线程，本地路径=" << localPath << "，远程路径=" << remotePath;
    sendThread->start();
}

void CommunicationModule::handleFileUploadTransfer(const DataMessage& msg)
{
    // 1. 解析外层JSON（BaseModule::transferData封装的格式）
    QByteArray outerData = msg.data(); // 获取消息原始数据
    QJsonObject outerJson = QJsonDocument::fromJson(outerData).object();
    if (outerJson.isEmpty()) {
        qWarning() << "文件下发：外层JSON解析失败";
        publishTransferResult(msg.channel(), false, "外层数据格式错误（非JSON）",
            ModuleType::Unknown, 0, "upload");
        return;
    }

    // 2. 提取并解码内层data（Base64编码的元数据）
    if (!outerJson.contains("data") || outerJson["data"].type() != QJsonValue::String) {
        qWarning() << "文件下发：外层JSON缺少有效data字段";
        publishTransferResult(msg.channel(), false, "外层数据缺少data字段",
            ModuleType::Unknown, 0, "upload");
        return;
    }
    QString base64Data = outerJson["data"].toString();
    QByteArray metaData = QByteArray::fromBase64(base64Data.toUtf8()); // 解码Base64
    if (metaData.isEmpty()) {
        qWarning() << "文件下发：内层data解码失败（无效Base64）";
        publishTransferResult(msg.channel(), false, "数据解码失败",
            ModuleType::Unknown, 0, "upload");
        return;
    }

    // 3. 解析真实元数据（MissionController封装的meta）
    QJsonObject metaJson = QJsonDocument::fromJson(metaData).object();
    if (metaJson.isEmpty()) {
        qWarning() << "文件下发：元数据解析失败（非JSON）";
        publishTransferResult(msg.channel(), false, "元数据格式错误", 
            ModuleType::Unknown, 0, "upload");
        return;
    }

    // 4. 提取核心字段（与MissionController封装的字段对应）
    ModuleType sourceModule = static_cast<ModuleType>(metaJson["source_module"].toInt());
    uint64_t requestId = metaJson["request_id"].toString().toULongLong();
    QString localPath = metaJson["localPath"].toString();
    QString remotePath = metaJson["remotePath"].toString();

    // 3. 校验字段有效性
    if (sourceModule == ModuleType::Unknown || requestId == 0)
    {
        publishTransferResult(msg.channel(), false, "元数据缺少source_module或request_id",
            ModuleType::Unknown, 0, "upload");
        return;
    }
    if (localPath.isEmpty() || remotePath.isEmpty()) {
        publishTransferResult(msg.channel(), false, "本地路径或远程路径为空",
            sourceModule, requestId, "upload");
        return;
    }
    if (!QFile::exists(localPath)) {
        publishTransferResult(msg.channel(), false, QString("本地文件不存在：%1").arg(localPath),
            sourceModule, requestId, "upload");
        return;
    }

    // 4. 启动异步文件上传线程（使用的ConfigWorker构造函数）
    QThread* sendThread = new QThread(this);
    // 初始化Worker：传入路径、模块标识、请求ID等参数
    ConfigWorker* worker = new ConfigWorker(
        msg.channel(),          // 通信通道（如WiredNetwork）
        localPath,              // 本地文件路径
        remotePath,             // 远程文件路径
        sourceModule,           // 来源模块（如TaskPlanning）
        requestId,              // 任务请求ID
        m_commManager           // 通信管理器
    );
    worker->moveToThread(sendThread);

    // 5. 关联Worker结果信号（文件上传完成后反馈）
    connect(worker, &ConfigWorker::FileUploadFinished, this,
        [=](bool success, CommunicationChannel channel, uint64_t reqId) {
            QString fileName = QFileInfo(localPath).fileName();
            QString resultMsg = success
                ? QString("文件 [%1] 下发成功").arg(fileName)
                : QString("文件 [%1] 下发失败").arg(fileName);
            publishTransferResult(channel, success, resultMsg, sourceModule,
                reqId, "upload");
            sendThread->quit(); // 完成后退出线程
        }, Qt::QueuedConnection);

    // 6. 启动线程和任务
    connect(sendThread, &QThread::started, worker, &ConfigWorker::doUploadFile);
    // 线程结束后释放资源
    connect(sendThread, &QThread::finished, worker, &ConfigWorker::deleteLater);
    connect(sendThread, &QThread::finished, sendThread, &QThread::deleteLater);

    qDebug() << "文件下发：启动上传线程，本地路径=" << localPath << "，远程路径=" << remotePath;
    sendThread->start();
}

void CommunicationModule::handleFileListGetTransfer(const DataMessage& msg)
{
    // 1. 解析外层JSON（与下载/上传逻辑完全一致）
    QByteArray outerData = msg.data();
    QJsonObject outerJson = QJsonDocument::fromJson(outerData).object();
    if (outerJson.isEmpty()) {
        qWarning() << "文件列表：外层JSON解析失败";
        publishTransferResult(msg.channel(), false, "外层数据格式错误（非JSON）",
            ModuleType::Unknown, 0, "listget");
        return;
    }

    // 2. 提取并解码内层data（Base64编码的元数据）
    if (!outerJson.contains("data") || outerJson["data"].type() != QJsonValue::String) {
        qWarning() << "文件列表：外层JSON缺少有效data字段";
        publishTransferResult(msg.channel(), false, "外层数据缺少data字段",
            ModuleType::Unknown, 0, "listget");
        return;
    }
    QString base64Data = outerJson["data"].toString();
    QByteArray metaData = QByteArray::fromBase64(base64Data.toUtf8());
    if (metaData.isEmpty()) {
        qWarning() << "文件列表：内层data解码失败（无效Base64）";
        publishTransferResult(msg.channel(), false, "数据解码失败", 
            ModuleType::Unknown, 0, "listget");
        return;
    }

    // 3. 解析真实元数据（与DataModule封装的字段对应）
    QJsonObject metaJson = QJsonDocument::fromJson(metaData).object();
    if (metaJson.isEmpty()) {
        qWarning() << "文件列表：元数据解析失败（非JSON）";
        publishTransferResult(msg.channel(), false, "元数据格式错误",
            ModuleType::Unknown, 0, "listget");
        return;
    }

    // 4. 提取核心字段（与下载/上传逻辑一致）
    ModuleType sourceModule = static_cast<ModuleType>(metaJson["source_module"].toInt());
    uint64_t requestId = metaJson["request_id"].toString().toULongLong();
    QString remotePath = metaJson["remotePath"].toString();

    // 5. 校验字段有效性
    if (sourceModule == ModuleType::Unknown || requestId == 0) {
        publishTransferResult(msg.channel(), false, "元数据缺少source_module或request_id",
            ModuleType::Unknown, 0, "listget");
        return;
    }
    if (remotePath.isEmpty()) {
        publishTransferResult(msg.channel(), false, "远程路径为空",
            sourceModule, requestId, "listget");
        return;
    }

    // 6. 启动异步获取文件列表线程（复用ConfigWorker，与下载/上传线程逻辑一致）
    QThread* listThread = new QThread(this);
    ConfigWorker* worker = new ConfigWorker(
        msg.channel(),          // 通信通道（WiredNetwork）
        "",                     // 本地路径为空（获取列表不需要）
        remotePath,             // 远程路径（要获取列表的目录）
        sourceModule,           // 来源模块（Data/Monitoring等）
        requestId,              // 列表请求ID
        m_commManager           // 通信管理器
    );
    worker->moveToThread(listThread);

    // 7. 关联Worker结果信号（文件列表获取完成后反馈）
    connect(worker, &ConfigWorker::fileListFetched, this,
        [=](bool success, const QStringList& files, CommunicationChannel channel, uint64_t reqId) {
            QString resultMsg = success
                ? QString("获取文件列表成功（%1个项目）").arg(files.size())
                : "获取文件列表失败";
            publishTransferResult(channel, success, resultMsg, sourceModule,
                reqId, "listget");
            listThread->quit(); // 完成后退出线程
        }, Qt::QueuedConnection);

    // 8. 启动线程和任务（与下载/上传逻辑一致）
    connect(listThread, &QThread::started, worker, &ConfigWorker::doListFiles);
    // 线程结束后释放资源（避免内存泄漏）
    connect(listThread, &QThread::finished, worker, &ConfigWorker::deleteLater);
    connect(listThread, &QThread::finished, listThread, &QThread::deleteLater);

    qDebug() << "文件列表：启动获取线程，远程路径=" << remotePath;
    listThread->start();
}

void CommunicationModule::handleFileDeleteTransfer(const DataMessage& msg)
{
    // 1. 解析外层JSON（与下载/上传/列表获取逻辑完全一致）
    QByteArray outerData = msg.data(); // 获取消息原始数据
    QJsonObject outerJson = QJsonDocument::fromJson(outerData).object();
    if (outerJson.isEmpty()) {
        qWarning() << "文件删除：外层JSON解析失败";
        publishTransferResult(msg.channel(), false, "外层数据格式错误（非JSON）",
            ModuleType::Unknown, 0, "delete"); // operationType设为delete
        return;
    }

    // 2. 提取并解码内层data（Base64编码的元数据，与现有逻辑一致）
    if (!outerJson.contains("data") || outerJson["data"].type() != QJsonValue::String) {
        qWarning() << "文件删除：外层JSON缺少有效data字段";
        publishTransferResult(msg.channel(), false, "外层数据缺少data字段",
            ModuleType::Unknown, 0, "delete");
        return;
    }
    QString base64Data = outerJson["data"].toString();
    QByteArray metaData = QByteArray::fromBase64(base64Data.toUtf8()); // 解码Base64
    if (metaData.isEmpty()) {
        qWarning() << "文件删除：内层data解码失败（无效Base64）";
        publishTransferResult(msg.channel(), false, "数据解码失败",
            ModuleType::Unknown, 0, "delete");
        return;
    }

    // 3. 解析真实元数据（与下载/上传逻辑一致）
    QJsonObject metaJson = QJsonDocument::fromJson(metaData).object();
    if (metaJson.isEmpty()) {
        qWarning() << "文件删除：元数据解析失败（非JSON）";
        publishTransferResult(msg.channel(), false, "元数据格式错误",
            ModuleType::Unknown, 0, "delete");
        return;
    }

    // 4. 提取核心字段（对齐下载/上传，新增deleteType）
    ModuleType sourceModule = static_cast<ModuleType>(metaJson["source_module"].toInt());
    uint64_t requestId = metaJson["request_id"].toString().toULongLong();
    QString remotePath = metaJson["remotePath"].toString();
    FtpDeleteResult::DeleteType delType = FtpDeleteResult::DeleteType::File;
    if (metaJson.contains("deleteType")) {
        // 假设deleteType在JSON中是整数（0=文件，1=目录，可根据实际枚举值调整）
        int typeInt = metaJson["deleteType"].toInt();
        // 校验类型合法性，避免无效值
        if (typeInt == static_cast<int>(FtpDeleteResult::DeleteType::File) ||
            typeInt == static_cast<int>(FtpDeleteResult::DeleteType::Directory)) {
            delType = static_cast<FtpDeleteResult::DeleteType>(typeInt);
        }
        else {
            qWarning() << "文件删除：无效的deleteType值，默认按文件处理，值=" << typeInt;
        }
    }
    else {
        qWarning() << "文件删除：元数据缺少deleteType字段，默认按文件处理";
        publishTransferResult(msg.channel(), false, "元数据缺少deleteType字段",
            sourceModule, requestId, "delete");
        return;
    }
    
    // 5. 校验字段有效性（与下载/上传逻辑一致，新增根目录防护）
    if (sourceModule == ModuleType::Unknown || requestId == 0)
    {
        publishTransferResult(msg.channel(), false, "元数据缺少source_module或request_id",
            ModuleType::Unknown, 0, "delete");
        return;
    }
    if (remotePath.isEmpty()) {
        publishTransferResult(msg.channel(), false, "远程删除路径为空",
            sourceModule, requestId, "delete");
        return;
    }
    if (remotePath == "/") { // 禁止删除根目录
        publishTransferResult(msg.channel(), false, "禁止删除远程根目录",
            sourceModule, requestId, "delete");
        return;
    }

    // 6. 启动异步删除线程（完全对齐下载/上传的线程逻辑）
    QThread* deleteThread = new QThread(this);
    // 注意：ConfigWorker构造函数参数顺序为「本地路径、远程路径」，删除时本地路径传空
    ConfigWorker* worker = new ConfigWorker(
        msg.channel(),          // 通信通道（如WiredNetwork）
        "",                     // 本地路径（删除无需，传空）
        remotePath,             // 远程删除路径
        sourceModule,           // 来源模块（如Monitoring）
        requestId,              // 删除请求ID
        m_commManager           // 通信管理器
    );
    worker->setDeleteType(delType); // 设置删除类型（文件/目录）
    worker->moveToThread(deleteThread);

    // 7. 关联删除完成信号（对齐下载的FileDownloadFinished）
    connect(worker, &ConfigWorker::FileDeleteFinished, this,
        [=](bool success, CommunicationChannel channel, uint64_t reqId) {
            QString typeStr = (delType == FtpDeleteResult::DeleteType::File) ? "文件" : "目录";
            QString resultMsg = success
                ? QString("删除远程%1成功：%2").arg(typeStr).arg(remotePath)
                : QString("删除远程%1失败：%2").arg(typeStr).arg(remotePath);
            // 复用publishTransferResult发布结果，operationType设为delete
            publishTransferResult(channel, success, resultMsg, sourceModule,
                reqId, "delete");
            deleteThread->quit(); // 完成后退出线程
        }, Qt::QueuedConnection);

    // 8. 启动线程和任务（与下载/上传逻辑完全一致）
    connect(deleteThread, &QThread::started, worker, &ConfigWorker::doDeleteFile);
    // 线程结束后释放资源（避免内存泄漏）
    connect(deleteThread, &QThread::finished, worker, &ConfigWorker::deleteLater);
    connect(deleteThread, &QThread::finished, deleteThread, &QThread::deleteLater);

    qDebug() << "文件删除：启动删除线程，远程路径=" << remotePath << "，类型=" << (int)delType;
    deleteThread->start();
}

void CommunicationModule::handleStopChannel(const DataMessage& msg)
{
    //1.解析外层json(basemodule封装的格式)
    QJsonObject outerJson = QJsonDocument::fromJson(msg.data()).object();
    QJsonObject cmdParams = outerJson["params"].toObject();//提取内层参数

    //2.获取目标通道
    CommunicationChannel targetChannel = msg.channel();
    // 创建停止Worker
    QThread* stopThread = new QThread(this);
    ConfigWorker* worker = new ConfigWorker(targetChannel, m_commManager);
    worker->moveToThread(stopThread);

    // 连接信号
    connect(stopThread, &QThread::started, worker, &ConfigWorker::doStop);

    connect(worker, &ConfigWorker::stopFinished, this, [=](bool stopSuccess, CommunicationChannel channel) {
        // 处理停止结果
        publishStopResult(channel, stopSuccess, stopSuccess ? "通道停止成功" : "通道停止失败");

        // 清理线程
        stopThread->quit();
        }, Qt::QueuedConnection);

    // 线程清理
    connect(stopThread, &QThread::finished, worker, &ConfigWorker::deleteLater);
    connect(stopThread, &QThread::finished, stopThread, &QThread::deleteLater);

    // 启动线程
    stopThread->start();
}

void CommunicationModule::onChannelStatusRespond(const DataMessage& msg)
{
    QJsonObject outerJson;
    if (!parseOuterJson(msg.data(), outerJson)) {
        qWarning() << "状态外层JSON解析失败";
        return;
    }

    if (!outerJson.contains("data") || !outerJson["data"].isObject()) {
        qWarning() << "状态信息缺少data字段";
        return;
    }
    QJsonObject data = outerJson["data"].toObject();
    CommunicationChannel channel = static_cast<CommunicationChannel>(data["channel"].toInt());
    QString statusDesc = data["status"].toString();

    CommunicationChannelStatus newStatus = CommunicationChannelStatus::Stopped;
    if (statusDesc == "运行中") {
        newStatus = CommunicationChannelStatus::Running;
    }
    else if (statusDesc == "初始化中") {
        newStatus = CommunicationChannelStatus::Initializing;
    }
    // 更新缓存并生成结果
    m_channelStatusCache[channel] = newStatus;
    CommunicationChannelStatusResult result(channel, newStatus);
    notifyObservers(&result);
    qInfo() << result.description();
}



// 处理通道状态变化（来自CommunicationManager）
void CommunicationModule::onChannelStatusChanged(CommunicationChannel channel, const QString& status)
{

    // 1. 先做空值防护，避免无效状态处理
    QString trimmedStatus = status.trimmed();
    if (trimmedStatus.isEmpty()) {
        qWarning() << "[CommunicationModule] 无效的通道状态字符串，通道：" << static_cast<int>(channel);
        return;
    }

    // 2. 优化状态映射逻辑（精准匹配，按“具体→通用”顺序判断，避免误判）
    CommunicationChannelStatus newStatus = CommunicationChannelStatus::Stopped;
    if (trimmedStatus.contains("运行中", Qt::CaseInsensitive) ||  // 通道正常运行的核心标识
        trimmedStatus.contains("已连接", Qt::CaseInsensitive))   // 串口连接成功的标识（兼容statusInfo返回值）
    {
        newStatus = CommunicationChannelStatus::Running;
    }
    else if (trimmedStatus.contains("初始化", Qt::CaseInsensitive))  // 初始化状态
    {
        newStatus = CommunicationChannelStatus::Initializing;
    }
    else if (trimmedStatus.contains("已停止", Qt::CaseInsensitive) ||  // 主动停止的核心标识
        trimmedStatus.contains("停止完成", Qt::CaseInsensitive)) // 可选：若有停止完成提示
    {
        newStatus = CommunicationChannelStatus::Stopped;
    }
    else if (trimmedStatus.contains("错误", Qt::CaseInsensitive) ||   // 通用错误
        trimmedStatus.contains("失败", Qt::CaseInsensitive) ||   // 操作失败
        trimmedStatus.contains("断开", Qt::CaseInsensitive) ||   // 被动断开（硬件移除/网络中断）
        trimmedStatus.contains("移除", Qt::CaseInsensitive) ||   // 硬件意外移除
        trimmedStatus.contains("超时", Qt::CaseInsensitive) ||   // 通信超时（可选，按需添加）
        trimmedStatus.contains("无法打开", Qt::CaseInsensitive)) // 串口无法打开等初始化失败
    {
        newStatus = CommunicationChannelStatus::Error;
    }
    // 兜底：若以上都不匹配，仍按Error处理（避免未知状态漏判）
    else
    {
        newStatus = CommunicationChannelStatus::Error;
        qWarning() << "[CommunicationModule] 未知通道状态字符串，默认标记为Error，通道：" << static_cast<int>(channel) << "，状态：" << trimmedStatus;
    }

    // 2. 防抖：仅当状态变化时，才更新缓存并发射信号（原有逻辑保留，无需修改）
    if (m_channelStatusCache.value(channel) == newStatus) {
        return; // 状态未变，跳过后续操作，避免重复发布
    }

    // 3. 更新缓存（复用已有 m_channelStatusCache，原有逻辑保留）
    m_channelStatusCache[channel] = newStatus;

    // 发布通道状态（供外部模块订阅）
    QJsonObject statusData;
    statusData["channel"] = static_cast<int>(channel);
    statusData["status"] = status;
    statusData["timestamp"] = QDateTime::currentDateTime().toString("yyMMddhhmmss");

    publishStatus(
        channel,
        "channel_status",
        statusData
    );
   
}
void CommunicationModule::onChannelError(CommunicationChannel channel, const QString& error)
{
    //构建错误事件数据
    QJsonObject errorData;
    errorData["channel"] = static_cast<int>(channel);
    errorData["error"] = error;
    errorData["timestamp"] = QDateTime::currentDateTime().toString("yyMMddhhmmss");
    
    //发布错误事件
    publishEvent(
        defaultCommunicationChannel(),
        "channel_error",
        errorData
    );
    //打印错误日志
    qWarning() << "通道错误[" << static_cast<int>(channel) << "]:" << error;
}


void CommunicationModule::onHardwareRespond(const DataMessage& message)
{
    //1.解析DataMessage中的frame
    QJsonObject frameJson = QJsonDocument::fromJson(message.data()).object();
    //发布事件，供selfcheckmodule订阅
    publishEvent(message.channel(), "selfcheck_receiveresult", frameJson);
}

void CommunicationModule::onFTPFileListRespond(CommunicationChannel channel, bool success, const QStringList& files, const QString& msg)
{
    // 1. 构建标准化的FTP文件列表结果数据
    QJsonObject resultData;
    resultData["channel"] = static_cast<int>(channel);           // 通道类型
    resultData["success"] = success;                             // 操作是否成功
    resultData["fileList"] = QJsonArray::fromStringList(files);  // 文件列表（JSON数组）
    resultData["fileCount"] = files.size();                      // 文件数量
    resultData["message"] = msg;                                 // 结果描述/错误信息
    resultData["timestamp"] = QDateTime::currentDateTime().toString("yyMMddhhmmss"); // 时间戳
    resultData["operationType"] = "listFiles";                   // 操作类型标识

    // 2. 发布FTP文件列表事件（供DataModule订阅）
    publishEvent(
        channel,                     // 所属通道
        "ftp_file_list_result",      // 事件名称（唯一标识）
        resultData                   // 事件数据
    );


    // 4. 日志输出
    if (success) {
        qInfo() << QString("FTP文件列表获取成功[通道%1]：%2个文件").arg(static_cast<int>(channel)).arg(files.size());
    }
    else {
        qWarning() << QString("FTP文件列表获取失败[通道%1]：%2").arg(static_cast<int>(channel)).arg(msg);
    }
}

void CommunicationModule::onFtpUploadProgress(CommunicationChannel channel, int progress)
{
    QJsonObject progressData;
    progressData["channel"] = static_cast<int>(channel);       // 通道类型
    progressData["progress"] = progress;                       // 进度值(0-100)
    progressData["operationType"] = "uploadProgress";          // 进度类型标识
    progressData["timestamp"] = QDateTime::currentDateTime().toString("yyMMddhhmmss");

    publishEvent(
        channel,                     // 所属通道
        "ftp_transfer_progress",     // 进度事件名称（唯一标识）
        progressData                 // 进度数据
    );

    qInfo() << QString("FTP上传进度[通道%1]：%2%").arg(static_cast<int>(channel)).arg(progress);
}

void CommunicationModule::onFtpDownloadProgress(CommunicationChannel channel, int progress)
{
    QJsonObject progressData;
    progressData["channel"] = static_cast<int>(channel);
    progressData["progress"] = progress;
    progressData["operationType"] = "downloadProgress"; // 区分下载进度
    progressData["timestamp"] = QDateTime::currentDateTime().toString("yyMMddhhmmss");

    // 发布进度事件
    publishEvent(channel, "ftp_transfer_progress", progressData);

    qInfo() << QString("FTP下载进度[通道%1]：%2%").arg(static_cast<int>(channel)).arg(progress);
}
