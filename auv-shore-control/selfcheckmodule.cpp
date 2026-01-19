#include "selfcheckmodule.h"
#include "communicationmanager.h"  // 假设存在通信管理器
#include "selfcheckenum.h"
#include"frameparser.h"
#include"framebuilder.h"
#include "commandparameter.h"
#include "cameramediaseq.h"  // 包含头文件
#include"insparse.h"
#include <QTimer>
#include <QDebug>
#include<QJsonDocument>
#include <QMutex>
#include<QPointer>
#include<QJsonArray>
// 定义全局序号计数器（初始化为1）
uint16_t g_cameraMediaSeq = 1;
// 定义线程安全锁
QMutex g_seqMutex;

/**
 * @brief 获取下一个唯一序号（实现）
 */
uint16_t getNextCameraMediaSeq() {
	QMutexLocker locker(&g_seqMutex);  // 线程安全
	uint16_t current = g_cameraMediaSeq;
	// 递增，超出范围后重置为1
	g_cameraMediaSeq = (g_cameraMediaSeq >= 65535) ? 1 : g_cameraMediaSeq + 1;
	return current;
}
SelfCheckModule::SelfCheckModule(QObject* parent)
	: BaseModule(ModuleType::SelfCheck, "SelfCheckModule", parent)
	, m_nextPacketSeq(1) // 初始化包序号从1开始（跳过0）
{
	initialize();
}

bool SelfCheckModule::initialize() {


	// 初始化器件列表
	initializeDevices();
	qDebug() << "自检模块初始化完成，加载了" << m_devices.size() << "个器件";
	
	// 订阅“发送结果”事件，绑定发送结果处理函数（确保lambda捕获this时使用弱指针，避免生命周期问题）
	subscribeEvent("selfcheck_send_result",
		[this](const DataMessage& msg)
		{
			QPointer<SelfCheckModule> weakThis = this;
			if (weakThis) {
				weakThis->onSendResultReceived(msg);
			}
		}
	);
	// 订阅“自检结果”事件，绑定自检结果处理函数
	subscribeEvent("selfcheck_receiveresult",
		[this](const DataMessage& msg)
		{
			QPointer<SelfCheckModule> weakThis = this;
			if (weakThis) {
				weakThis->onCheckResultReceived(msg);
			}
		}

	);
	// 2. 注册适配器
	auto beidouAdapter = std::make_shared<BeidouATAdapter>();
	InternalCommandManager::instance().registerAdapter(beidouAdapter);
	//定期清理残留状态（应对极端情况下的状态泄漏）
	m_cleanupTimer = new QTimer(this);
	m_cleanupTimer->setInterval(30000); // 30秒清理一次
	m_cleanupTimer->setSingleShot(false);
	connect(m_cleanupTimer, &QTimer::timeout, this, &SelfCheckModule::cleanupStaleStates);
	m_cleanupTimer->start();
	return true;
}

void SelfCheckModule::shutdown()
{
	qDebug() << "清理自检模块";

	// 停止清理定时器
	if (m_cleanupTimer) {
		m_cleanupTimer->stop();
		m_cleanupTimer->deleteLater();
		m_cleanupTimer = nullptr;
	}

	// 清理所有命令状态和定时器（加锁保护）
	QMutexLocker locker(&m_mutex);
	// 先停止所有定时器，再删除
	for (auto& state : m_commandStates) {
		if (state.timer) {
			state.timer->stop();
			state.timer->disconnect(); // 断开所有信号槽
			state.timer->deleteLater();
			state.timer = nullptr;
		}
	}

	// ：清理内部指令状态
	for (auto& state : m_internalCmdStates) {
		if (state.timer) {
			state.timer->stop();
			state.timer->disconnect();
			state.timer->deleteLater();
			state.timer = nullptr;
		}
	}

	m_commandStates.clear();
	m_internalCmdStates.clear(); // 清空内部指令状态
	m_cmdTimerMap.clear();
	m_observers.clear();
	
}

const SelfCheckDevice* SelfCheckModule::getDevice(DeviceId deviceId) const {
	for (const auto& device : m_devices) {
		if (device.deviceId() == deviceId) {
			return &device;
		}
	}
	return nullptr;
}

CheckError SelfCheckModule::sendCheckCommand(DeviceId deviceId,CommandCode cmdCode,
	const std::shared_ptr<CommandParameter>& param,WorkPhase parse, CommunicationChannel channel,
	 CommandType cmdType)
{

	qDebug() << "发送自检命令，通道类型：" << static_cast<int>(channel)
		<< " 设备ID：" << static_cast<int>(deviceId)
		<< " 命令码：" << static_cast<int>(cmdCode);
	// 1. 基础校验（器件存在性、命令支持性）
	const SelfCheckDevice* device = getDevice(deviceId);
	if (!device) {
		return CheckError(ErrorCode::DeviceNotFound,
			"器件不存在: " + EnumConverter::deviceIdToString(deviceId));
	}

	// 检查器件是否支持该命令
	if (!device->supportsCommand(cmdCode)) {
		return CheckError(ErrorCode::CommandNotSupported,
			QString("器件%1不支持命令%2")
			.arg(EnumConverter::deviceIdToString(deviceId))
			.arg(EnumConverter::commandCodeToString(deviceId, cmdCode)));
	}

	// 2. 生成命令元信息（统一机制核心）
	uint8_t packetSeq = getNextPacketSeq();// 包序号（硬件帧用）
	std::shared_ptr<InternalCommandParam> internalParam = std::dynamic_pointer_cast<InternalCommandParam>(param);
	uint64_t requestId = CommandIdGenerator::instance().nextId(); // 全局唯一ID（结果匹配用）

   // 3. 构建数据体（协议格式：器件标识(1字节) + 命令编码(1字节) + 参数(变长)）
	QByteArray dataBody;
	if (cmdType == CommandType::Internal && internalParam && deviceId == DeviceId::BeidouComm && cmdCode == CommandCode::BeidouComm_Test)
	{
		// 获取适配器
		InternalProtocolAdapter* adapter = InternalCommandManager::instance().getAdapter(deviceId);
		if (!adapter) {
			return CheckError(ErrorCode::DeviceNotFound,
				"设备" + EnumConverter::deviceIdToString(deviceId) + "无内部指令适配器");
		}

		// 核心：适配器直接使用 internalParam 中已构建的完整帧，无需任何帧处理
		dataBody = adapter->generateCommand(cmdCode, param);
		if (dataBody.isEmpty()) {
			return CheckError(ErrorCode::ParameterInvalid,
				QString("设备%1不支持该内部指令：%2")
				.arg(EnumConverter::deviceIdToString(deviceId))
				.arg(static_cast<int>(cmdCode)));
		}
	}
	else if (cmdType == CommandType::Internal)
	{
		// 其他内部指令：原有逻辑，无需修改
		InternalProtocolAdapter* adapter = InternalCommandManager::instance().getAdapter(deviceId);
		if (!adapter) {
			return CheckError(ErrorCode::DeviceNotFound,
				"设备" + EnumConverter::deviceIdToString(deviceId) + "无内部指令适配器");
		}
		dataBody = adapter->generateCommand(cmdCode, param);
		if (dataBody.isEmpty()) {
			return CheckError(ErrorCode::ParameterInvalid,
				QString("设备%1不支持该内部指令：%2")
				.arg(EnumConverter::deviceIdToString(deviceId))
				.arg(static_cast<int>(cmdCode)));
		}
	}
	else
	{
		// 外部指令：原逻辑（器件标识+命令码+参数）
		dataBody.append(static_cast<char>(static_cast<uint8_t>(deviceId)));
		dataBody.append(static_cast<char>(static_cast<uint8_t>(cmdCode)));
		if (param) dataBody.append(param->toBytes());
		qDebug() << "参数字节（大端序）：" << param->toBytes().toHex().toUpper();
	}
	
	// 4. 超时管理
	QTimer* timer = new QTimer(this);
	timer->setSingleShot(true);
	int timeoutMs = getDynamicTimeoutMs(deviceId, cmdCode);
	timer->setInterval(timeoutMs);

	connect(timer, &QTimer::timeout, this, [this, packetSeq, deviceId, cmdCode, timer,cmdType]() {
		// 第一步：先停止定时器（锁外，避免死锁）
		timer->stop();
		timer->disconnect(); // 断开所有信号槽

		// 第二步：加锁处理状态
		QMutexLocker locker(&m_mutex);
		if (m_commandStates.contains(packetSeq)) {
			CommandState& state = m_commandStates[packetSeq];
			if (state.deviceId == deviceId && state.cmdCode == cmdCode && !state.isProcessed
				&& state.cmdType == cmdType) {
				qDebug() << "[超时触发] 执行超时回调 - 器件:" << EnumConverter::deviceIdToString(deviceId)
					<< "命令:" << EnumConverter::commandCodeToString(deviceId, cmdCode)
					<< "包序号:" << static_cast<int>(packetSeq)
					<< "指令类型" << (cmdType == CommandType::Internal ? "内部" : "外部");
				// 调用超时处理
				this->onCommandTimeout(deviceId, cmdCode, packetSeq,cmdType);
			}
			else {
				qWarning() << "[超时回调] 状态不匹配/已处理 - packetSeq:" << static_cast<int>(packetSeq)
					<< "已处理:" << state.isProcessed
					<< "存储指令类型：" << (state.cmdType == CommandType::Internal ? "内部" : "外部")
					<< "当前指令类型：" << (cmdType == CommandType::Internal ? "内部" : "外部");
			}

			// 清理状态（仅移除，定时器删除在锁外）
			m_cmdTimerMap.remove(packetSeq);
			m_commandStates.remove(packetSeq);
		}
		else {
			qWarning() << "[超时回调] 未找到命令状态 - packetSeq:" << static_cast<int>(packetSeq);
		}

		// 第三步：锁外异步删除定时器（避免持有锁时触发事件循环）
		if (timer) {
			timer->deleteLater();
		}
		});
	timer->start();

	// 5. 存储命令状态（加锁）
	{
		QMutexLocker locker(&m_mutex);
		if (cmdType == CommandType::Internal) {
			// 内部指令：按设备+命令+类型存储（无packetSeq）
			QString cmdKey = generateInternalCmdKey(deviceId, cmdCode);
			m_internalCmdStates[cmdKey] = {
				deviceId, cmdCode, timer, false, packetSeq, QDateTime::currentDateTime(), cmdType
			};
		}
		else {
			// 外部指令：按packetSeq存储（原有逻辑）
			m_commandStates[packetSeq] = {
				deviceId, cmdCode, timer, false, packetSeq, QDateTime::currentDateTime(), cmdType
			};
		}
		m_cmdTimerMap[packetSeq] = QPointer<QTimer>(timer); // 保留定时器映射
	}

	// 6. 发送命令到通信模块（使用统一命令入口）
	QJsonObject cmdParams;
	cmdParams["source_module"] = static_cast<int>(ModuleType::SelfCheck);
	cmdParams["request_id"] = QString::number(requestId);
	cmdParams["cmd_type"] = static_cast<int>(cmdType); // 告知通信模块指令类型
	cmdParams["packet_seq"] = packetSeq;
	cmdParams["work_phase"] = static_cast<int>(parse);
	cmdParams["data_body"] = QString(dataBody.toBase64());
	cmdParams["timestamp"] = QDateTime::currentDateTime().toString("yyMMddhhmmss");
	// 标记BDS数据类型（AT/短报文）
	if (deviceId == DeviceId::BeidouComm) {
		cmdParams["device_id"] = static_cast<int>(deviceId);
		cmdParams["cmd_code"] = static_cast<int>(cmdCode);
		if (cmdCode == CommandCode::BeidouComm_Test) {
			// 测试命令=短报文
			cmdParams["bds_data_type"] = static_cast<int>(BDSDataType::ShortMessage);
		}
		else {
			// 其他北斗命令=AT指令
			cmdParams["bds_data_type"] = static_cast<int>(BDSDataType::ATCommand);
		}
	}
	sendCommand(
		"CommunicationModule",
		channel,
		"send_control_command",
		cmdParams
	);
	return CheckError();
}


void SelfCheckModule::onCheckResultReceived(const DataMessage& data)
{
	//解析从communicationmodule返回的json结果
	QJsonObject outerJson = QJsonDocument::fromJson(data.data()).object();
	QJsonObject dataJson = outerJson["data"].toObject();//提取内层参数

	if (CommunicationChannel::BDS == data.channel())
	{
		QString msgType = dataJson["msg_type"].toString();
		if (msgType == "at_response") {
			// 解析AT指令反馈
			parseBDSAtResponse(dataJson);
		}
		else if (msgType == "short_message") {
			// 解析短报文结果（函数）
			parseBDSShortMessageResponse(dataJson);
		}
		else {
			qWarning() << "未知的BDS消息类型：" << msgType;
		}
	}
	else if(CommunicationChannel::Radio == data.channel())
	{
		parseSelfcheckResult(dataJson);
	}
	else if (CommunicationChannel::WaterAcoustic == data.channel())
	{
		parseWaterAcousticResponse(dataJson);
	}
	
}
void SelfCheckModule::onSendResultReceived(const DataMessage& data)
{
	QJsonObject receiveJson = QJsonDocument::fromJson(data.data()).object();
	QJsonObject dataJson = receiveJson["data"].toObject();//提取内层参数

	// 2. 提取字段（默认值-1，用于后续有效性判断）
	int deviceIdInt = dataJson["device_id"].toInt(-1);
	int cmdCodeInt = dataJson["cmd_code"].toInt(-1);
	int packetSeqInt = dataJson["packet_seq"].toInt(-1);
	bool success = dataJson["success"].toBool(false);
	QString message = dataJson["message"].toString("未知原因");
	uint64_t requestId = dataJson["request_id"].toVariant().toULongLong();

	// 3. 字段有效性校验（避免非法值导致逻辑错误）
	QMutexLocker locker(&m_mutex); // 线程安全保护（与m_commandStates访问一致）
	bool isValid = true;
	DeviceId deviceId = static_cast<DeviceId>(deviceIdInt);
	CommandCode cmdCode = static_cast<CommandCode>(cmdCodeInt);
	uint8_t packetSeq = 0;

	if (deviceIdInt < 0 || deviceIdInt > 255) {
		qWarning() << "[发送结果无效] 器件ID超出范围（必须0~255）：" << deviceIdInt;
		isValid = false;
	}
	else if (!DeviceTypeHelper::isValidDeviceId(static_cast<uint8_t>(deviceIdInt))) {
		qWarning() << "[发送结果无效] 器件ID非法：" << deviceIdInt;
		isValid = false;
	}

	if (isValid) { // 仅当DeviceId有效时，校验CommandCode
		if (cmdCodeInt < 0 || cmdCodeInt > 255) {
			qWarning() << "[发送结果无效] 命令码超出范围（必须0~255）：" << cmdCodeInt;
			isValid = false;
		}
		else if (!DeviceTypeHelper::isValidCommandCode(static_cast<uint8_t>(cmdCodeInt))) {
			qWarning() << "[发送结果无效] 命令码非法：" << cmdCodeInt << "（器件ID：" << deviceIdInt << "）";
			isValid = false;
		}
	}

	// 校验packetSeq（uint8_t范围：0-255）
	if (isValid) {
		if (packetSeqInt < 0 || packetSeqInt > 255) {
			qWarning() << "[发送结果无效] 包序号超出uint8_t范围：" << packetSeqInt << "（合法范围0-255）";
			isValid = false;
		}
		else {
			packetSeq = static_cast<uint8_t>(packetSeqInt);
		}
	}

	// 创建发送结果对象
	SelfCheckSendResult sendResult(deviceId, cmdCode, success, packetSeq);
	sendResult.setMessage(message);
	sendResult.setRequestId(requestId);

	if (!isValid) {
		sendResult.setSuccess(false);
		sendResult.setMessage("发送结果无效：字段非法或超出范围");
		notifyObservers(&sendResult); // 通知发送结果
		return;
	}

	// 5. 日志打印（包含所有关键信息，格式清晰，便于调试）
	QString deviceName = EnumConverter::deviceIdToString(deviceId);
	QString cmdName = EnumConverter::commandCodeToString(deviceId, cmdCode);

	if (success)
	{
		qDebug() << "[自检命令发送成功] "
			<< "器件ID：0x" << QString::number(static_cast<uint8_t>(deviceId), 16).toUpper() << "（" << deviceName << "），"
			<< "命令码：0x" << QString::number(static_cast<uint8_t>(cmdCode), 16).toUpper() << "（" << cmdName << "），"
			<< "包序号：" << static_cast<int>(packetSeq) << "，"
			<< "描述：" << message;

		// 通知发送成功结果
		notifyObservers(&sendResult);

		// 发送成功后，可以单独通知Pending状态（可选）
		SelfCheckSendResult pendingResult(deviceId, cmdCode, true);
		pendingResult.setFeedbackDesc("命令已发送，等待响应...");
		notifyObservers(&pendingResult);
	}
	else
	{
		qWarning() << "[自检命令发送失败] "
			<< "器件ID：0x" << QString::number(static_cast<uint8_t>(deviceId), 16).toUpper() << "（" << deviceName << "），"
			<< "命令码：0x" << QString::number(static_cast<uint8_t>(cmdCode), 16).toUpper() << "（" << cmdName << "），"
			<< "包序号：" << static_cast<int>(packetSeq) << "，"
			<< "失败原因：" << message;

		// 通知发送失败结果
		notifyObservers(&sendResult);

		// 清理状态（优化：直接通过packetSeq删除，避免遍历）
		if (m_commandStates.contains(packetSeq)) {
			CommandState& state = m_commandStates[packetSeq];
			if (state.timer) {
				state.timer->stop();
				state.timer->disconnect();
				state.timer->deleteLater();
				state.timer = nullptr;
			}
			m_commandStates.remove(packetSeq);
		}
	}
}
void SelfCheckModule::onCommandTimeout(DeviceId deviceId, CommandCode cmdCode, uint8_t packetSeq
,  CommandType cmdType)
{
	
	// 1. 加锁校验状态是否已处理（核心：增加cmdType匹配）
	bool isAlreadyProcessed = false;
	{
		// 外部指令：检查packetSeq+cmdType
		if (cmdType == CommandType::External && m_commandStates.contains(packetSeq)) {
			CommandState& state = m_commandStates[packetSeq];
			isAlreadyProcessed = state.isProcessed || state.cmdType != cmdType;
		}
		// 内部指令：检查设备+命令+类型（避免内部指令触发超时）
		else if (cmdType == CommandType::Internal) {
			QString cmdKey = generateInternalCmdKey(deviceId, cmdCode);
			isAlreadyProcessed = !m_internalCmdStates.contains(cmdKey);
		}
	}

	// 2. 如果已处理/类型不匹配，直接返回，不执行超时逻辑
	if (isAlreadyProcessed) {
		qDebug() << "[超时忽略] 内部指令已处理，跳过超时逻辑 - 器件:" << EnumConverter::deviceIdToString(deviceId)
			<< "命令:" << EnumConverter::commandCodeToString(deviceId, cmdCode);
		return;
	}
	// 3. 构建超时结果并通知观察者
	SelfCheckResult result(deviceId, cmdCode, ResultStatus::Timeout);
	result.setFeedbackDesc("命令超时未收到响应（超时时间：" + QString::number(getDynamicTimeoutMs(deviceId, cmdCode)) + "ms）");
	result.setCommandType(cmdType);
	notifyObservers(&result);

	// 2. 此处无需清理状态，状态清理已在定时器回调中完成
	qDebug() << "[超时清理] 器件:" << EnumConverter::deviceIdToString(deviceId)
		<< "命令:" << EnumConverter::commandCodeToString(deviceId, cmdCode)
		<< "包序号:" << static_cast<int>(packetSeq)
		<< "指令类型" << (cmdType == CommandType::Internal ? "内部" : "外部");
}
/*
* 定期清理残留状态
*/
void SelfCheckModule::cleanupStaleStates()
{
	QList<uint8_t> staleSeqs;
	QList<QString> staleInternalKeys; // ：内部指令键列表
	{
		QMutexLocker locker(&m_mutex);
		QDateTime now = QDateTime::currentDateTime();

		// 清理外部指令（原有逻辑）
		for (auto it = m_commandStates.begin(); it != m_commandStates.end(); ++it) {
			bool isStale = it->isProcessed
				|| now > it->sendTime.addMSecs(getDynamicTimeoutMs(it->deviceId, it->cmdCode) * 2)
				|| (it->timer && !it->timer->isActive())
				|| (it->timer == nullptr);
			if (isStale) {
				staleSeqs.append(it.key());
			}
		}

		// ：清理内部指令
		for (auto it = m_internalCmdStates.begin(); it != m_internalCmdStates.end(); ++it) {
			bool isStale = it->isProcessed
				|| now > it->sendTime.addMSecs(getDynamicTimeoutMs(it->deviceId, it->cmdCode) * 2)
				|| (it->timer && !it->timer->isActive())
				|| (it->timer == nullptr);
			if (isStale) {
				staleInternalKeys.append(it.key());
			}
		}

		// 批量移除外部指令状态
		for (uint8_t seq : staleSeqs) {
			m_commandStates.remove(seq);
			m_cmdTimerMap.remove(seq);
		}

		// 批量移除内部指令状态
		for (const QString& key : staleInternalKeys) {
			if (m_internalCmdStates.contains(key)) {
				CommandState& state = m_internalCmdStates[key];
				m_cmdTimerMap.remove(state.packetSeq);
				m_internalCmdStates.remove(key);
			}
		}
	}

	// 锁外处理定时器删除（避免持有锁时触发事件循环）
	if (!staleSeqs.isEmpty()) {
		for (uint8_t seq : staleSeqs) {
			QPointer<QTimer> timer = m_cmdTimerMap.value(seq);
			if (!timer.isNull()) {
				timer->stop();
				timer->disconnect();
				timer->deleteLater();
			}
		}
		qDebug() << "清理残留命令状态：" << staleSeqs.size() << "个，剩余：" << m_commandStates.size();
	}
}

void SelfCheckModule::initializeDevices()
{
	// ------------------------------ 尾推1（0x02）
	{
		SelfCheckDevice device(Subsystem::Propulsion, DeviceId::TailThruster, "尾推1");
		device.addSupportedCommand({ CommandCode::Common_PowerOn, "上电",
								  std::make_shared<EmptyParameter>(), "响应0x00成功/0xFF失败" });
		device.addSupportedCommand({ CommandCode::Common_PowerOff, "下电",
								  std::make_shared<EmptyParameter>(), "响应0x00成功/0xFF失败" });
		device.addSupportedCommand({ CommandCode::Thruster_ParamReset, "参数重置",
								  std::make_shared<EmptyParameter>(), "转速与运行时间归0，响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::Common_Enable, "使能",
								  std::make_shared<EmptyParameter>(), "响应0x00成功/0xFF失败" });
		device.addSupportedCommand({ CommandCode::Thruster_SetSpeed, "设置转速（闭环）",
								  std::make_shared<SpeedParameter>(300), "目标转速(int16)，响应0x1F/0x2F" });
		device.addSupportedCommand({ CommandCode::Thruster_SetDuty, "设置占空比（开环）",
								  std::make_shared<ThrusterDutyParameter>(1500), "0-2000(uint16)，响应0x1F/0x2F" });
		device.addSupportedCommand({ CommandCode::Thruster_SetRunTime, "设置运行时间",
								  std::make_shared<ThrusterRunTimeParameter>(60), "秒(uint32)，响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::Thruster_SetAccelTime, "设置加速时间",
								  std::make_shared<AccelTimeParameter>(1000), "毫秒(uint16)，响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::Thruster_GetRunTime, "获取运行时间",
								  std::make_shared<EmptyParameter>(), "反馈秒数(uint16)" });
		device.addSupportedCommand({ CommandCode::Thruster_GetDuty, "获取占空比",
								  std::make_shared<EmptyParameter>(), "反馈(uint16)" });
		device.addSupportedCommand({ CommandCode::Thruster_GetCurrent, "获取电流",
								  std::make_shared<EmptyParameter>(), "反馈值/100=A(uint16)" });
		device.addSupportedCommand({ CommandCode::Thruster_GetTemperature, "获取温度",
								  std::make_shared<EmptyParameter>(), "反馈值/10=℃(uint16)" });
		device.addSupportedCommand({ CommandCode::Thruster_GetSpeed, "获取转速",
								  std::make_shared<EmptyParameter>(), "反馈(int16)" });
		device.addSupportedCommand({ CommandCode::Thruster_GetHall, "获取霍尔值",
								  std::make_shared<EmptyParameter>(), "1-6正常，0/7故障(uint8)" });
		device.addSupportedCommand({ CommandCode::Common_GetFault, "获取故障码",
								  std::make_shared<EmptyParameter>(), "不定字节" });
		device.addSupportedCommand({ CommandCode::Thruster_GetBusVoltage, "获取母线电压",
								  std::make_shared<EmptyParameter>(), "反馈值/10=V(uint16)" });
		device.addSupportedCommand({ CommandCode::Thruster_GetNoMaintainTime, "获取无维护时间",
								  std::make_shared<EmptyParameter>(), "小时(uint16)" });
		device.addSupportedCommand({ CommandCode::Thruster_GetTotalRunTime, "获取总运行时间",
								  std::make_shared<EmptyParameter>(), "秒(uint32)" });
		device.addSupportedCommand({ CommandCode::Thruster_SetSpeedWithTime, "设置转速及时间",
						  std::make_shared<ThrusterSpeedAndRunTimeParameter>(0,0), "反馈成功/失败" });
		m_devices.append(device);
	}

	// 添加所有舵设备
	createRudderDevice(DeviceId::Rudder1, "舵1");
	createRudderDevice(DeviceId::Rudder2, "舵2");
	createRudderDevice(DeviceId::Rudder3, "舵3");
	createRudderDevice(DeviceId::Rudder4, "舵4");
	createRudderDevice(DeviceId::AntennaFold, "天线折叠机构");

	// ------------------------------ 浮调1（0x06）
	{
		SelfCheckDevice device(Subsystem::Buoyancy, DeviceId::BuoyancyAdjust, "浮调1");
		// 原有通用命令（上电、下电等）
		device.addSupportedCommand({ CommandCode::Common_PowerOn, "上电",
								  std::make_shared<EmptyParameter>(), "响应0x00成功/0xFF失败" });
		device.addSupportedCommand({ CommandCode::Common_PowerOff, "下电",
								  std::make_shared<EmptyParameter>(), "响应0x00成功/0xFF失败" });
		//设置浮力
		device.addSupportedCommand({
			CommandCode::Buoyancy_SetValue,
			"设置浮力值",
			std::make_shared<EmptyParameter>(),  
			"响应0x00成功/0xFF失败"
			});

		// 读取当前浮力
		device.addSupportedCommand({
			CommandCode::Buoyancy_ReadValue,
			"读取当前浮力",
			std::make_shared<EmptyParameter>(),  // 无参数（硬件使用预设绝对零位值）
			"响应浮力值"
			});
		device.addSupportedCommand({
			CommandCode::Buoyancy_Stop,
			"急停",
			std::make_shared<EmptyParameter>(),  // 无参数（硬件使用预设绝对零位值）
			"响应0x00成功/0xFF失败"
			});
		m_devices.append(device);
	}

	// ------------------------------ 抛载（0x07）
	{
		SelfCheckDevice device(Subsystem::Emergency, DeviceId::DropWeight, "抛载");
		device.addSupportedCommand({ CommandCode::Common_PowerOn, "上电",
							  std::make_shared<EmptyParameter>(), "响应0x00成功/0xFF失败" });
		device.addSupportedCommand({ CommandCode::Common_PowerOff, "下电",
								  std::make_shared<EmptyParameter>(), "响应0x00成功/0xFF失败" });
		device.addSupportedCommand({ CommandCode::DropWeight_Release, "抛载释放",
								  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::DropWeight_Reset, "抛载复位",
								  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::Common_GetFault, "自检状态",
								  std::make_shared<EmptyParameter>(), "0x00正常/0xFF故障" });
		m_devices.append(device);
	}

	// ------------------------------ 北斗（0x08）
	{
		SelfCheckDevice device(Subsystem::Navigation, DeviceId::BeidouComm, "北斗");
		device.addSupportedCommand({ CommandCode::BeidouComm_GetSignalQuality, "获取信号质量",
							  std::make_shared<EmptyParameter>(), "响应1，2，3，4，5" });
		device.addSupportedCommand({ CommandCode::BeidouComm_GetIdentity, "身份查询",
								  std::make_shared<EmptyParameter>(), "响应7位数字卡号" });
		device.addSupportedCommand({ CommandCode::BeidouComm_GetTarget, "目标查询（远端设备号）",
								  std::make_shared<EmptyParameter>(), "响应远端7位数字卡号" });
		device.addSupportedCommand({ CommandCode::BeidouComm_SetTarget, "修改远端设备号",
								  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::BeidouComm_GetWhitelist, "查询白名单",
								  std::make_shared<EmptyParameter>(), "响应白名单序列" });
		device.addSupportedCommand({ CommandCode::BeidouComm_AddWhitelist, "添加白名单",
								  std::make_shared<EmptyParameter>(), "响应" });
		device.addSupportedCommand({ CommandCode::BeidouComm_DelWhitelist, "删除白名单成员",
								  std::make_shared<EmptyParameter>(), "响应" });
		device.addSupportedCommand({ CommandCode::BeidouComm_GetPosition, "获取定位信息",
								  std::make_shared<EmptyParameter>(), "响应定位信息" });
		device.addSupportedCommand({ CommandCode::BeidouComm_GetSysInfo, "获取系统信息",
								  std::make_shared<EmptyParameter>(), "响应系统信息" });
		device.addSupportedCommand({ CommandCode::BeidouComm_FactoryReset, "恢复出厂设置并重启",
								  std::make_shared<EmptyParameter>(), "响应" });
		device.addSupportedCommand({ CommandCode::BeidouComm_Reboot, "设备重启",
						  std::make_shared<EmptyParameter>(), "响应" });
		device.addSupportedCommand({ CommandCode::BeidouComm_Test, "通讯测试",
						  std::make_shared<EmptyParameter>(), "响应" });
		device.addSupportedCommand({ CommandCode::BeidouComm_GetConnectStatus, "获取连接状态",
				  std::make_shared<EmptyParameter>(), "响应" });
		m_devices.append(device);
	}

	// 深度计（0x18）
	{
		SelfCheckDevice device(Subsystem::Navigation, DeviceId::DepthMeter, "深度计");
		device.addSupportedCommand({ CommandCode::Common_PowerOn, "上电",
								  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::Common_PowerOff, "下电",
								  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::Depthgauge_Calibration, "设置基准值",
								  std::make_shared<EmptyParameter>(), "校准/重置" });
		device.addSupportedCommand({ CommandCode::Depthgauge_GetDepth, "获取深度值",
								  std::make_shared<EmptyParameter>(), "米，小数点后四位(uint16)" });
		m_devices.append(device);
	}

	// 高度计（0x19）
	{
		SelfCheckDevice device(Subsystem::Navigation, DeviceId::Altimeter, "高度计");
		device.addSupportedCommand({ CommandCode::Common_PowerOn, "上电",
								  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::Common_PowerOff, "下电",
								  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::Altimeter_GetHeight, "获取高度值",
								  std::make_shared<EmptyParameter>(), "离底高度，小数点后四位(uint16)" });
		m_devices.append(device);
	}

	// USBL（0x20）
	{
		SelfCheckDevice device(Subsystem::Navigation, DeviceId::USBL, "USBL");
		device.addSupportedCommand({ CommandCode::Common_PowerOn, "上电",
								  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::Common_PowerOff, "下电",
								  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::USBL_GetBeaconVersion, "获取信标版本号",
								  std::make_shared<EmptyParameter>(), "string格式" });
		m_devices.append(device);
	}
	// 惯导（0x21）
	{
		SelfCheckDevice device(Subsystem::Navigation, DeviceId::INS, "惯导");
		device.addSupportedCommand({ CommandCode::Common_PowerOn, "上电",
								  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::Common_PowerOff, "下电",
								  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::INS_SetGnssBind, "设置装订对准",
								  std::make_shared<InsGnssParam>(30, 120), "经纬度(string)，响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::INS_CalibrationWithDVL, "设置与DVL标定",
								  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::INS_GetUtcDate, "获取UTC日期",
								  std::make_shared<EmptyParameter>(), "YYMMDD(string)" });
		device.addSupportedCommand({ CommandCode::INS_GetUtcTime, "获取UTC时间",
								  std::make_shared<EmptyParameter>(), "HHMMSS.SS(string)" });
		device.addSupportedCommand({ CommandCode::INS_GetAttitude, "获取姿态",
								  std::make_shared<EmptyParameter>(), "航向,俯仰,横滚(string)" });
		device.addSupportedCommand({ CommandCode::INS_GetBodySpeed, "获取载体系速度",
								  std::make_shared<EmptyParameter>(), "右前上速度(string)" });
		device.addSupportedCommand({ CommandCode::INS_GetGeoSpeed, "获取地理系速度",
								  std::make_shared<EmptyParameter>(), "东北天速度(string)" });
		device.addSupportedCommand({ CommandCode::INS_GetPosition, "获取位置",
								  std::make_shared<EmptyParameter>(), "经纬度(string)" });
		device.addSupportedCommand({ CommandCode::INS_GetGyroscopeSpeed, "获取陀螺仪角速度",
						  std::make_shared<EmptyParameter>(), "航向,俯仰,横滚(string)" });
		device.addSupportedCommand({ CommandCode::INS_GetAcceleration, "获取加速度",
				  std::make_shared<EmptyParameter>(), "右,前,上(string)" });
		device.addSupportedCommand({ CommandCode::INS_GetStatus, "获取状态码",
								  std::make_shared<EmptyParameter>(), "状态信息" });
		device.addSupportedCommand({ CommandCode::Common_GetFault, "获取故障类型",
								  std::make_shared<EmptyParameter>(), "0x00正常/0xFF故障+故障码" });
		device.addSupportedCommand({ CommandCode::INS_CombinedWithDVL, "强制DVL组合",
								  std::make_shared<EmptyParameter>(), "0x00成功/0xFF失败" });
		device.addSupportedCommand({ CommandCode::INS_CombinedAuto, "自由组合(默认)",
						  std::make_shared<EmptyParameter>(), "0x00成功/0xFF失败" });
		m_devices.append(device);
	}

	// DVL（0x22）
	{
		SelfCheckDevice device(Subsystem::Navigation, DeviceId::DVL, "DVL");
		device.addSupportedCommand({ CommandCode::Common_PowerOn, "上电",
								  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::Common_PowerOff, "下电",
								  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::DVL_GetBottomSpeed, "获取对底速度",
								  std::make_shared<EmptyParameter>(), "右前上速度(mm/s, string)" });
		device.addSupportedCommand({ CommandCode::DVL_GetWaterSpeed, "获取对水速度",
								  std::make_shared<EmptyParameter>(), "右前上速度(mm/s, string)" });
		device.addSupportedCommand({ CommandCode::DVL_GetStatus, "获取状态码",
								  std::make_shared<EmptyParameter>(), "自检状态" });
		m_devices.append(device);
	}

	// 摄像机与光源（0x23）
	{
		SelfCheckDevice device(Subsystem::DataAcquisition, DeviceId::CameraLight, "摄像机与光源");
		device.addSupportedCommand({ CommandCode::Common_PowerOn, "上电",
								  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::Common_PowerOff, "下电",
								  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::CameraLight_StartVideoSave, "开始存储视频",
								  std::make_shared<CameraMediaParam>(getNextCameraMediaSeq()), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::CameraLight_StopVideoSave, "停止存储视频",
								  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::CameraLight_TakePhoto, "拍照",
								  std::make_shared<CameraMediaParam>(getNextCameraMediaSeq()), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::Common_GetFault, "获取设备状态",
								  std::make_shared<EmptyParameter>(), "0x00正常/0xFF故障+故障码" });
		m_devices.append(device);
	}

	// 动力电池1（0x24）
	{
		SelfCheckDevice device(Subsystem::Power, DeviceId::PowerBattery1, "动力电池1");
		device.addSupportedCommand({ CommandCode::Battery_GetSoc, "获取SOC",
								  std::make_shared<EmptyParameter>(), "值*0.1=%(uint8)" });
		device.addSupportedCommand({ CommandCode::Battery_GetTotalVoltage, "获取总电压",
								  std::make_shared<EmptyParameter>(), "值*0.1=V(uint16)" });
		device.addSupportedCommand({ CommandCode::Battery_GetCurrent, "获取电池电流",
								  std::make_shared<EmptyParameter>(), "(值-500)*0.1=A(int16)" });
		device.addSupportedCommand({ CommandCode::Battery_GetAvgTemp, "获取平均温度",
								  std::make_shared<EmptyParameter>(), "值-40=℃(int16)" });
		device.addSupportedCommand({ CommandCode::Battery_HVOn, "高压上电",
								  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::Battery_HVOff, "高压下电",
								  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::Battery_Lock, "锁止",
							  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::Battery_Unlock, "解锁",
					  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::Common_GetFault, "获取故障类型",
								  std::make_shared<EmptyParameter>(), "故障码" });
		m_devices.append(device);
	}

	// 动力电池2
	{
		SelfCheckDevice device(Subsystem::Power, DeviceId::PowerBattery2, "动力电池2");
		device.addSupportedCommand({ CommandCode::Battery_GetSoc, "获取SOC",
								  std::make_shared<EmptyParameter>(), "值*0.1=%(uint8)" });
		device.addSupportedCommand({ CommandCode::Battery_GetTotalVoltage, "获取总电压",
								  std::make_shared<EmptyParameter>(), "值*0.1=V(uint16)" });
		device.addSupportedCommand({ CommandCode::Battery_GetCurrent, "获取电池电流",
								  std::make_shared<EmptyParameter>(), "(值-500)*0.1=A(int16)" });
		device.addSupportedCommand({ CommandCode::Battery_GetAvgTemp, "获取平均温度",
								  std::make_shared<EmptyParameter>(), "值-40=℃(int16)" });
		device.addSupportedCommand({ CommandCode::Battery_HVOn, "高压上电",
								  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::Battery_HVOff, "高压下电",
								  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::Battery_Lock, "锁止",
							  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::Battery_Unlock, "解锁",
					  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::Common_GetFault, "获取故障类型",
								  std::make_shared<EmptyParameter>(), "故障码" });
		m_devices.append(device);
	}

	// 动力电池（0x24）
	{
		SelfCheckDevice device(Subsystem::Power, DeviceId::MeterBattery, "仪表电池");
		device.addSupportedCommand({ CommandCode::Battery_GetSoc, "获取SOC",
								  std::make_shared<EmptyParameter>(), "值*0.1=%(uint8)" });
		device.addSupportedCommand({ CommandCode::Battery_GetTotalVoltage, "获取总电压",
								  std::make_shared<EmptyParameter>(), "值*0.1=V(uint16)" });
		device.addSupportedCommand({ CommandCode::Battery_GetCurrent, "获取电池电流",
								  std::make_shared<EmptyParameter>(), "(值-500)*0.1=A(int16)" });
		device.addSupportedCommand({ CommandCode::Battery_GetAvgTemp, "获取平均温度",
								  std::make_shared<EmptyParameter>(), "值-40=℃(int16)" });
		device.addSupportedCommand({ CommandCode::Battery_HVOn, "高压上电",
								  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::Battery_HVOff, "高压下电",
								  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::Battery_Lock, "锁止",
							  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::Battery_Unlock, "解锁",
					  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::Common_GetFault, "获取故障类型",
								  std::make_shared<EmptyParameter>(), "故障码" });
		m_devices.append(device);
	}
	// 侧扫声纳（0x26）、前视声纳（0x27）、下视声呐（0x28）
	{
		auto addSonar = [&](DeviceId id, const QString& name) {
			SelfCheckDevice device(Subsystem::DataAcquisition, id, name);
			device.addSupportedCommand({ CommandCode::Common_PowerOn, "上电",
									  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
			device.addSupportedCommand({ CommandCode::Common_PowerOff, "下电",
									  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
			device.addSupportedCommand({ CommandCode::Sonar_SetParameter, "设置参数",
									  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
			device.addSupportedCommand({ CommandCode::Common_Enable, "使能（开始工作）",
									  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
			device.addSupportedCommand({ CommandCode::Common_Disable, "停止工作",
						  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
			device.addSupportedCommand({ CommandCode::Common_GetFault, "获取设备状态",
									  std::make_shared<EmptyParameter>(), "0x00正常/0xFF故障" });
			m_devices.append(device);
		};
		addSonar(DeviceId::SideScanSonar, "侧扫声纳");
		addSonar(DeviceId::ForwardSonar, "前视声纳");
		addSonar(DeviceId::DownwardSonar, "下视声呐");
	}

	// 水声通信（0x29）
	{
		SelfCheckDevice device(Subsystem::Communication, DeviceId::AcousticComm, "水声通信");
		device.addSupportedCommand({ CommandCode::Common_PowerOn, "上电",
								  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::Common_PowerOff, "下电",
								  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::AcousticComm_SelfCheck, "自检",
								  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
		device.addSupportedCommand({ CommandCode::AcousticComm_Test, "通信测试",
							  std::make_shared<EmptyParameter>(), "响应发送数据" });
		device.addSupportedCommand({ CommandCode::Common_GetFault, "获取设备状态",
								  std::make_shared<EmptyParameter>(), "0x00正常/0xFF故障" });
		m_devices.append(device);
	}
}

void SelfCheckModule::parseSelfcheckResult(const QJsonObject& frameJson)
{
	Frame frame = Frame::fromJson(frameJson);
	QByteArray dataBody = frame.dataBody;
	if (dataBody.size() < 2) { // 至少1字节器件ID + 1字节命令码
		qWarning() << "数据体长度不足（需≥2字节），原始数据:" << dataBody.toHex();
		return;
	}
	// 解析器件ID和命令码
	DeviceId deviceId = static_cast<DeviceId>(static_cast<uint8_t>(dataBody[0]));
	CommandCode cmdCode = static_cast<CommandCode>(static_cast<uint8_t>(dataBody[1]));
	QByteArray payload = dataBody.mid(2); // 参数部分

	// 关键：从响应中提取发送时的packetSeq（假设在frameJson的"packet_seq"字段）
	uint8_t respPacketSeq = frameJson["packet_seq"].toInt(-1);
	if (respPacketSeq == static_cast<uint8_t>(-1)) {
		qWarning() << "响应中未包含packetSeq，无法匹配命令";
		return;
	}

	QList<QTimer*> timersToDelete; // 收集需要删除的定时器
	{
		QMutexLocker locker(&m_mutex);
		if (!m_commandStates.contains(respPacketSeq)) {
			qWarning() << "未匹配命令 - 器件:" << static_cast<uint8_t>(deviceId)
				<< "命令:" << static_cast<uint8_t>(cmdCode)
				<< "包序号:" << static_cast<int>(respPacketSeq);
			return;
		}

		CommandState& matchState = m_commandStates[respPacketSeq];
		if (matchState.deviceId != deviceId
			|| matchState.cmdCode != cmdCode
			|| matchState.packetSeq != respPacketSeq
			|| matchState.isProcessed) {
			qWarning() << "命令匹配失败（信息不匹配） - 器件:" << static_cast<uint8_t>(deviceId)
				<< "命令:" << static_cast<uint8_t>(cmdCode)
				<< "包序号:" << static_cast<int>(respPacketSeq);
			return;
		}

		// 收集定时器，锁外删除
		if (matchState.timer) {
			timersToDelete.append(matchState.timer);
			matchState.timer = nullptr; // 置空，避免野指针
		}

		// 标记状态并清理
		matchState.isProcessed = true;
		m_cmdTimerMap.remove(respPacketSeq);
		m_commandStates.remove(respPacketSeq);
	}

	// 锁外删除定时器（关键修复）
	for (QTimer* timer : timersToDelete) {
		if (timer) {
			timer->stop();
			timer->disconnect();
			timer->deleteLater();
		}
	}

	// 解析响应（按器件和命令分类）

	QString feedbackDesc;
	ResultStatus status = ResultStatus::Unknown;
	SelfCheckResult result(deviceId, cmdCode, status);
	result.setCommandType(CommandType::External);
	if (DeviceTypeHelper::isThruster(deviceId)) {
		// 推进器响应解析
		switch (cmdCode) {
			// 上电/下电/参数重置/使能（响应为PayloadResult：0x00成功/0xFF失败）
		case CommandCode::Common_PowerOn:
		case CommandCode::Common_PowerOff:
		case CommandCode::Thruster_ParamReset:
		case CommandCode::Common_Enable:
			if (payload.size() == 1) {
				PayloadResult payloadRes = static_cast<PayloadResult>(static_cast<uint8_t>(payload[0]));
				status = (payloadRes == PayloadResult::Success) ? ResultStatus::Success : ResultStatus::Failed;
				feedbackDesc = EnumConverter::payloadResultToString(payloadRes);
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("响应长度异常（期望1字节，实际%1字节）").arg(payload.size());
			}
			break;
			// 设置转速（闭环）
		case CommandCode::Thruster_SetSpeed: // 设置转速
			if (payload.size() == 1) {
				PayloadResult payloadRes = static_cast<PayloadResult>(static_cast<uint8_t>(payload[0]));
				status = (payloadRes == PayloadResult::Success) ? ResultStatus::Success : ResultStatus::Failed;
				feedbackDesc = EnumConverter::payloadResultToString(payloadRes);
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("转速响应长度异常（期望1字节，实际%1字节）").arg(payload.size());
			}
			break;
			// 设置占空比（开环）：响应0x1F接收/0x2F到达
		case CommandCode::Thruster_SetDuty: // 设置占空比
			if (payload.size() == 1) {
				PayloadResult payloadRes = static_cast<PayloadResult>(static_cast<uint8_t>(payload[0]));
				status = (payloadRes == PayloadResult::Success) ? ResultStatus::Success : ResultStatus::Failed;
				feedbackDesc = EnumConverter::payloadResultToString(payloadRes);
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望1字节，实际%1字节").arg(payload.size());
			}
			break;
		case CommandCode::Thruster_SetRunTime: // 设置运行时间
		case CommandCode::Thruster_SetAccelTime: // 设置加速时间
			if (payload.size() == 1) {
				PayloadResult res = static_cast<PayloadResult>(static_cast<uint8_t>(payload[0]));
				status = (res == PayloadResult::Success) ? ResultStatus::Success : ResultStatus::Failed;
				feedbackDesc = EnumConverter::payloadResultToString(res);
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望1字节，实际%1字节").arg(payload.size());
			}
			break;
		case CommandCode::Thruster_GetRunTime: // 获取运行时间
			if (payload.size() == 2) {
				uint16_t time = FrameParser::bytesToUint16(payload);
				feedbackDesc = QString("连续运行时间：%1秒").arg(time);
				result.setParameter("runtime", time);
				status = ResultStatus::Success;
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望2字节，实际%1字节").arg(payload.size());
			}
			break;

		case CommandCode::Thruster_GetDuty: // 获取占空比
			if (payload.size() == 2) {
				uint16_t duty = FrameParser::bytesToUint16(payload);
				feedbackDesc = QString("占空比：%1（1000=停止，<1000=反转，>1000=正转）").arg(duty);
				result.setParameter("duty", duty);
				status = ResultStatus::Success;
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望2字节，实际%1字节").arg(payload.size());
			}
			break;
		case CommandCode::Thruster_GetCurrent: // 获取电流
			if (payload.size() == 2) {
				uint16_t curr = FrameParser::bytesToUint16(payload);
				feedbackDesc = QString("电流：%1A").arg(curr / 100.0);
				result.setParameter("current", curr/100.0);
				status = ResultStatus::Success;
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望2字节，实际%1字节").arg(payload.size());
			}
			break;
		case CommandCode::Thruster_GetTemperature: // 获取温度
			if (payload.size() == 2) {
				uint16_t temp = FrameParser::bytesToUint16(payload);
				feedbackDesc = QString("温度：%1℃").arg(temp / 10.0);
				result.setParameter("temperature", temp);
				status = ResultStatus::Success;
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望2字节，实际%1字节").arg(payload.size());
			}
			break;
		case CommandCode::Thruster_GetSpeed: // 获取转速
			if (payload.size() == 2) {
				const uchar* dataPtr = reinterpret_cast<const uchar*>(payload.constData());
				int16_t speed = qFromBigEndian<int16_t>(dataPtr);
				feedbackDesc = QString("转速：%1RPM").arg(speed);
				result.setParameter("speed", speed);
				status = ResultStatus::Success;
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望2字节，实际%1字节").arg(payload.size());
			}
			break;
		case CommandCode::Thruster_GetHall: // 获取霍尔值
			if (payload.size() == 1) {
				uint8_t hall = static_cast<uint8_t>(payload[0]);
				if (hall == 0 || hall == 7) {
				feedbackDesc = QString("霍尔值%1（故障）").arg(hall);
				}
				else {
				feedbackDesc = QString("霍尔值%1（正常）").arg(hall);
				result.setParameter("hall", hall);
				}
				status = ResultStatus::Success;
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望1字节，实际%1字节").arg(payload.size());
			}
			break;
		case CommandCode::Common_GetFault: // 获取故障码
			if (!payload.isEmpty()) {
				feedbackDesc = QString("故障码：0x%1").arg(QString(payload.toHex().toUpper()));
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = "故障码响应为空";
			}
			break;
		case CommandCode::Thruster_GetBusVoltage: // 获取母线电压
			if (payload.size() == 2) {
				const uchar* dataPtr = reinterpret_cast<const uchar*>(payload.constData());
				uint16_t volt = qFromBigEndian<uint16_t>(dataPtr);
				feedbackDesc = QString("母线电压：%1V").arg(volt / 10.0);
				result.setParameter("volt", volt/10.0);
				status = ResultStatus::Success;
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望2字节，实际%1字节").arg(payload.size());
			}
			break;
		case CommandCode::Thruster_GetNoMaintainTime: // 无维护时间
			if (payload.size() == 2) {
				uint16_t hours = FrameParser::bytesToUint16(payload);
				feedbackDesc = QString("无维护运行时间：%1小时").arg(hours);
				result.setParameter("noMaintainTime", hours);
				status = ResultStatus::Success;
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望2字节，实际%1字节").arg(payload.size());
			}
			break;
		case CommandCode::Thruster_GetTotalRunTime: // 总运行时间
			if (payload.size() == 4) {
				const uchar* dataPtr = reinterpret_cast<const uchar*>(payload.constData());
				uint32_t sec = qFromBigEndian<uint32_t>(dataPtr);
				feedbackDesc = QString("总运行时间：%1秒").arg(sec);
				result.setParameter("totalRunTime", sec);
				status = ResultStatus::Success;
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望4字节，实际%1字节").arg(payload.size());
			}
			break;
		case CommandCode::Thruster_SetSpeedWithTime:
			if (payload.size() == 1) {
				PayloadResult payloadRes = static_cast<PayloadResult>(static_cast<uint8_t>(payload[0]));
				status = (payloadRes == PayloadResult::Success) ? ResultStatus::Success : ResultStatus::Failed;
				feedbackDesc = EnumConverter::payloadResultToString(payloadRes);
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("转速及运行时间响应长度异常（期望1字节，实际%1字节）").arg(payload.size());
			}
			break;
		
		default:
			feedbackDesc = "推进器未知命令响应";
			status = ResultStatus::Unknown;
		}
	}
	else if (DeviceTypeHelper::isRudder(deviceId)) {
		// 舵机响应解析
		switch (cmdCode) {
		case CommandCode::Common_PowerOn:
		case CommandCode::Common_PowerOff:
		case CommandCode::Rudder_SetZero:
			if (payload.size() == 1) {
				PayloadResult res = static_cast<PayloadResult>(static_cast<uint8_t>(payload[0]));
				status = (res == PayloadResult::Success) ? ResultStatus::Success : ResultStatus::Failed;
				feedbackDesc = EnumConverter::payloadResultToString(res);
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望1字节，实际%1字节").arg(payload.size());
			}
			break;
		case CommandCode::Rudder_SetAngle: // 设置舵角
			if (payload.size() == 1) {
				PayloadResult res = static_cast<PayloadResult>(static_cast<uint8_t>(payload[0]));
				status = (res == PayloadResult::Success) ? ResultStatus::Success : ResultStatus::Failed;
				feedbackDesc = EnumConverter::payloadResultToString(res);
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望1字节，实际%1字节").arg(payload.size());
			}
			break;
		case CommandCode::Rudder_GetRunTime: // 读取运行时间
			if (payload.size() == 4) {
				uint32_t sec = qFromBigEndian(*reinterpret_cast<const uint32_t*>(payload.data()));
				feedbackDesc = QString("运行时间：%1秒").arg(sec);
				result.setParameter("runtime", static_cast<qlonglong>(sec)); // 显式转换为qlonglong
				status = ResultStatus::Success;
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望4字节，实际%1字节").arg(payload.size());
			}
			break;
		case CommandCode::Rudder_GetVoltage: // 霍尔电压
			if (payload.size() == 2) {
				uint16_t volt = FrameParser::bytesToUint16(payload);
				feedbackDesc = QString("霍尔电压：%1V").arg(volt / 1000.0);
				result.setParameter("voltage", volt/1000.0);
				status = ResultStatus::Success;
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望2字节，实际%1字节").arg(payload.size());
			}
			break;
		case CommandCode::Rudder_GetAngle: // 获取舵角
			if (payload.size() == 2) {
				int16_t angle = qFromBigEndian<int16_t>(reinterpret_cast<const uchar*>(payload.data()));
				feedbackDesc = QString("当前舵角：%1度").arg(angle / 10.0);
				result.setParameter("angle", angle/10.0);
				status = ResultStatus::Success;
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望2字节，实际%1字节").arg(payload.size());
			}
			break;
		case CommandCode::Rudder_GetCurrent: // 舵机电流
			if (payload.size() == 2) {
				uint16_t curr = FrameParser::bytesToUint16(payload);
				feedbackDesc = QString("电流：%1A").arg(curr / 100.0);
				result.setParameter("current", curr);
				status = ResultStatus::Success;
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望2字节，实际%1字节").arg(payload.size());
			}
			break;
		case CommandCode::Rudder_GetTemp: // 舵机温度
			if (payload.size() == 2) {
				uint16_t temp = FrameParser::bytesToUint16(payload);
				feedbackDesc = QString("温度：%1℃").arg(temp / 10.0);
				result.setParameter("temperature", temp);
				status = ResultStatus::Success;
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望2字节，实际%1字节").arg(payload.size());
			}
			break;
		case CommandCode::Common_GetFault: // 舵机故障
			if (!payload.isEmpty()) {
				feedbackDesc = QString("故障码：0x%1").arg(QString(payload.toHex().toUpper()));
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = "故障码响应为空";
			}
			break;
		default:
			feedbackDesc = "舵机未知命令响应";
			status = ResultStatus::Unknown;
			break;
		}
	}
	// 浮调（BuoyancyAdjust1/2）解析（分支）
	else if (DeviceTypeHelper::isBuoyancy(deviceId)) {
	switch (cmdCode) {
		// 上电/下电命令解析（原有逻辑）
	case CommandCode::Common_PowerOn:
	case CommandCode::Common_PowerOff:
	case CommandCode::Buoyancy_SetValue:
	case CommandCode::Buoyancy_Stop:
		if (payload.size() == 1)
		{
			PayloadResult res = static_cast<PayloadResult>(static_cast<uint8_t>(payload[0]));
			status = (res == PayloadResult::Success) ? ResultStatus::Success : ResultStatus::Failed;
			feedbackDesc = EnumConverter::payloadResultToString(res);
		}
		else
		{
			status = ResultStatus::Failed;
			feedbackDesc = QString("期望1字节，实际%1字节").arg(payload.size());
		}
		break;
	case CommandCode::Buoyancy_ReadValue:  
		if (payload.size() == 2) { 
			uint16_t buoyancyValue = FrameParser::bytesToUint16(payload);
			feedbackDesc = QString("浮力值：%1L").arg(buoyancyValue / 100.0);
			result.setParameter("buoyancyValue", buoyancyValue);
			status = ResultStatus::Success;
		}
		else {
			status = ResultStatus::Failed;
			feedbackDesc = QString("电位计响应异常（期望4字节，实际%1字节）").arg(payload.size());
		}
		break;

		// 其他命令（如未来扩展）
	default:
		feedbackDesc = "浮调未知命令";
		status = ResultStatus::Unknown;
	}
	}
	else if (deviceId == DeviceId::DropWeight) {
		// 抛载响应解析
		switch (cmdCode) {
		case CommandCode::Common_PowerOn:
		case CommandCode::Common_PowerOff:
		case CommandCode::DropWeight_Release:
		case CommandCode::DropWeight_Reset:
		case CommandCode::Common_GetFault:
			if (payload.size() == 1) {
				PayloadResult res = static_cast<PayloadResult>(static_cast<uint8_t>(payload[0]));
				status = (res == PayloadResult::Success) ? ResultStatus::Success : ResultStatus::Failed;
				feedbackDesc = (cmdCode == CommandCode::Common_GetFault) ?
					(status == ResultStatus::Success ? "抛载状态正常" : "抛载存在故障") :
					EnumConverter::payloadResultToString(res);
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望1字节，实际%1字节").arg(payload.size());
			}
			break;
		default:
			feedbackDesc = "抛载未知命令";
			status = ResultStatus::Unknown;
		}
	}
	// 深度计解析
	else if (deviceId == DeviceId::DepthMeter) {
		switch (cmdCode) {
		case CommandCode::Common_PowerOn:
		case CommandCode::Common_PowerOff:
			if (payload.size() == 1) {
				PayloadResult res = static_cast<PayloadResult>(static_cast<uint8_t>(payload[0]));
				status = (res == PayloadResult::Success) ? ResultStatus::Success : ResultStatus::Failed;
				feedbackDesc = EnumConverter::payloadResultToString(res);
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望1字节，实际%1字节").arg(payload.size());
			}
			break;
		case CommandCode::Depthgauge_Calibration: // 设置基准值
			if (payload.size() == 1) {
				PayloadResult res = static_cast<PayloadResult>(static_cast<uint8_t>(payload[0]));
				status = (res == PayloadResult::Success) ? ResultStatus::Success : ResultStatus::Failed;
				feedbackDesc = EnumConverter::payloadResultToString(res);
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望1字节，实际%1字节").arg(payload.size());
			}
			break;
		case CommandCode::Depthgauge_GetDepth: // 获取深度值
			if (payload.size() == 2) {

				uint16_t depthRaw = FrameParser::bytesToUint16(payload);
				double depth = depthRaw / 100.0; // 转换为实际深度（根据协议调整比例尺）
				feedbackDesc = QString("深度：%1米（小数点后四位）").arg(depth);
				// 存入具体参数（键名建议统一规范，如"depth"）
				result.setParameter("depth", depth);
				status = ResultStatus::Success;
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望2字节，实际%1字节").arg(payload.size());
			}
			break;
		default:
			feedbackDesc = "深度计未知命令";
			status = ResultStatus::Unknown;
		}
	}
	// 高度计解析
	else if (deviceId == DeviceId::Altimeter) {
		switch (cmdCode) {
		case CommandCode::Common_PowerOn:
		case CommandCode::Common_PowerOff:
			if (payload.size() == 1) {
				PayloadResult res = static_cast<PayloadResult>(static_cast<uint8_t>(payload[0]));
				status = (res == PayloadResult::Success) ? ResultStatus::Success : ResultStatus::Failed;
				feedbackDesc = EnumConverter::payloadResultToString(res);
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望1字节，实际%1字节").arg(payload.size());
			}
			break;
		case CommandCode::Altimeter_GetHeight: // 获取高度值
			if (payload.size() == 2) {
				uint16_t height = FrameParser::bytesToUint16(payload);
				feedbackDesc = QString("离底高度：%1米（小数点后四位）").arg(height / 100.0);
				result.setParameter("height", height/100.0);
				status = ResultStatus::Success;
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望2字节，实际%1字节").arg(payload.size());
			}
			break;
		default:
			feedbackDesc = "高度计未知命令";
			status = ResultStatus::Unknown;
		}
	}
	// USBL解析
	else if (deviceId == DeviceId::USBL) {
		switch (cmdCode) {
		case CommandCode::Common_PowerOn:
		case CommandCode::Common_PowerOff:
			if (payload.size() == 1) {
				PayloadResult res = static_cast<PayloadResult>(static_cast<uint8_t>(payload[0]));
				status = (res == PayloadResult::Success) ? ResultStatus::Success : ResultStatus::Failed;
				feedbackDesc = EnumConverter::payloadResultToString(res);
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望1字节，实际%1字节").arg(payload.size());
			}
			break;
		case CommandCode::USBL_GetBeaconVersion: // 获取信标版本号
			feedbackDesc = QString("信标版本号：%1").arg(QString(payload));
			break;
		default:
			feedbackDesc = "USBL未知命令";
			status = ResultStatus::Unknown;
		}
	}

	// 惯导解析
	else if (deviceId == DeviceId::INS) {
		switch (cmdCode) {
		case CommandCode::Common_PowerOn:
		case CommandCode::Common_PowerOff:
		case CommandCode::INS_SetGnssBind:
		case CommandCode::INS_CalibrationWithDVL:
			if (payload.size() == 1) {
				PayloadResult res = static_cast<PayloadResult>(static_cast<uint8_t>(payload[0]));
				status = (res == PayloadResult::Success) ? ResultStatus::Success : ResultStatus::Failed;
				feedbackDesc = EnumConverter::payloadResultToString(res);
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望1字节，实际%1字节").arg(payload.size());
			}
			break;
		case CommandCode::INS_GetUtcDate: // UTC日期
			if (payload.size() == 6) {
				QString dateStr = QString(payload); // 直接转字符串（YYMMDD）
				feedbackDesc = QString("UTC日期：%1（格式YYMMDD）").arg(dateStr);
				status = ResultStatus::Success;
				// 设置UI所需参数：date
				result.setParameter("date", dateStr);
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望6字节日期，实际%1字节").arg(payload.size());
			}
			break;
		case CommandCode::INS_GetUtcTime: // UTC时间
			if (payload.size() == 8) {
				QString timeStr = QString(payload); // 直接转字符串（HHMMSS.SS）
				feedbackDesc = QString("UTC时间：%1（格式HHMMSS.SS）").arg(timeStr);
				status = ResultStatus::Success;
				// 设置UI所需参数：time
				result.setParameter("time", timeStr);
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望8字节时间，实际%1字节").arg(payload.size());
			}
			break;
		case CommandCode::INS_GetAttitude: // 姿态
		{
			// 拆分payload字符串（去除空格，按逗号分割）
			QString attitudeStr = QString(payload).simplified().replace(" ", "");
			QStringList attitudeParts = attitudeStr.split(",");
			if (attitudeParts.size() == 3) {
				// 解析为double（保留4位小数，UI展示1位）
				double yaw = attitudeParts[0].toDouble();    // 航向（x.xxxx）
				double pitch = attitudeParts[1].toDouble();  // 俯仰（y.yyyy）
				double roll = attitudeParts[2].toDouble();   // 横滚（z.zzzz）
				feedbackDesc = QString("姿态（航向,俯仰,横滚）：%1°,%2°,%3°")
					.arg(yaw, 0, 'f', 2)
					.arg(pitch, 0, 'f', 2)
					.arg(roll, 0, 'f', 2);
				status = ResultStatus::Success;
				// 设置UI所需参数：yaw/pitch/roll
				result.setParameter("yaw", yaw);
				result.setParameter("pitch", pitch);
				result.setParameter("roll", roll);
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("姿态数据格式错误，实际：%1").arg(attitudeStr);
			}
			break;
		}
		case CommandCode::INS_GetBodySpeed: // 载体系速度
		{
			QString bodySpeedStr = QString(payload).simplified().replace(" ", "");
			QStringList bodySpeedParts = bodySpeedStr.split(",");
			if (bodySpeedParts.size() == 3) {
				double vx = bodySpeedParts[0].toDouble(); // 右向速度（x.xxx）
				double vy = bodySpeedParts[1].toDouble(); // 前向速度（y.xxx）
				double vz = bodySpeedParts[2].toDouble(); // 上向速度（z.xxx）
				feedbackDesc = QString("载体系速度（右,前,上）：%1,%2,%3 m/s")
					.arg(vx, 0, 'f', 2)
					.arg(vy, 0, 'f', 2)
					.arg(vz, 0, 'f', 2);
				status = ResultStatus::Success;
				// 设置UI所需参数：vx/vy/vz
				result.setParameter("vx", vx);
				result.setParameter("vy", vy);
				result.setParameter("vz", vz);
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("载体系速度格式错误，实际：%1").arg(bodySpeedStr);
			}
			break;
		}
		case CommandCode::INS_GetGeoSpeed: // 地理系速度
		{
			QString geoSpeedStr = QString(payload).simplified().replace(" ", "");
			QStringList geoSpeedParts = geoSpeedStr.split(",");
			if (geoSpeedParts.size() == 3) {
				double ex = geoSpeedParts[0].toDouble(); // 东向速度（x.xxx）
				double ey = geoSpeedParts[1].toDouble(); // 北向速度（y.xxx）
				double ez = geoSpeedParts[2].toDouble(); // 天向速度（z.xxx）
				feedbackDesc = QString("地理系速度（东,北,天）：%1,%2,%3 m/s")
					.arg(ex, 0, 'f', 2)
					.arg(ey, 0, 'f', 2)
					.arg(ez, 0, 'f', 2);
				status = ResultStatus::Success;
				// 设置UI所需参数：ex/ey/ez
				result.setParameter("ex", ex);
				result.setParameter("ey", ey);
				result.setParameter("ez", ez);
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("地理系速度格式错误，实际：%1").arg(geoSpeedStr);
			}
			break;
		}
		case CommandCode::INS_GetPosition: // 位置
		{
			QString posStr = QString(payload).simplified().replace(" ", "");
			QStringList posParts = posStr.split(",");
			if (posParts.size() == 2) {
				QString lat = posParts[0]; // 纬度（x.xxxxxx）
				QString lon = posParts[1]; // 经度（y.yyyyyy）
				feedbackDesc = QString("位置（纬度,经度）：%1°,%2°")
					.arg(lat)
					.arg(lon);
				status = ResultStatus::Success;
				// 设置UI所需参数：lat/lon
				result.setParameter("lat", lat);
				result.setParameter("lon", lon);
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("位置数据格式错误，实际：%1").arg(posStr);
			}
			break;
		}
		// ------------------------------获取陀螺仪角速度 ------------------------------
		case CommandCode::INS_GetGyroscopeSpeed: {
			QString gyroStr = QString(payload).simplified().replace(" ", "");
			QStringList gyroParts = gyroStr.split(",");
			if (gyroParts.size() == 3) {
				// 解析三轴角速度
				double yawGyro = gyroParts[0].toDouble();    // 航向角（x.xxxx，°/s，小数点后2位）
				double pitchGyro = gyroParts[1].toDouble();  // 俯仰角（y.yyyy，°/s，小数点后4位）
				double rollGyro = gyroParts[2].toDouble();   // 横滚角（z.zzzz，°/s，小数点后2位）

				feedbackDesc = QString("陀螺仪角速度（航向,俯仰,横滚）：%1°/s,%2°/s,%3°/s")
					.arg(yawGyro, 0, 'f', 2)    // 航向保留2位小数
					.arg(pitchGyro, 0, 'f', 4)  // 俯仰保留4位小数
					.arg(rollGyro, 0, 'f', 2);  // 横滚保留2位小数

				// 存入参数（供UI使用）
				result.setParameter("yaw_gyro", yawGyro);
				result.setParameter("pitch_gyro", pitchGyro);
				result.setParameter("roll_gyro", rollGyro);
				status = ResultStatus::Success;
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("陀螺仪角速度格式错误，实际：%1").arg(gyroStr);
			}
			break;
		}

											   // ------------------------------ ：获取加速度 ------------------------------
		case CommandCode::INS_GetAcceleration: {
			QString accelStr = QString(payload).simplified().replace(" ", "");
			QStringList accelParts = accelStr.split(",");
			if (accelParts.size() == 3) {
				// 解析三轴加速度
				double xAccel = accelParts[0].toDouble();  // 右向（x.xxx，m/s?，小数点后2位）
				double yAccel = accelParts[1].toDouble();  // 前向（y.yyy，m/s?，小数点后2位）
				double zAccel = accelParts[2].toDouble();  // 垂向（z.zzz，m/s?，小数点后2位）

				feedbackDesc = QString("加速度（右,前,垂）：%1m/s?,%2m/s?,%3m/s?")
					.arg(xAccel, 0, 'f', 2)
					.arg(yAccel, 0, 'f', 2)
					.arg(zAccel, 0, 'f', 2);

				// 存入参数（供UI使用）
				result.setParameter("x_accel", xAccel);
				result.setParameter("y_accel", yAccel);
				result.setParameter("z_accel", zAccel);
				status = ResultStatus::Success;
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("加速度格式错误，实际：%1").arg(accelStr);
			}
			break;
		}

		case CommandCode::INS_GetStatus: // 状态码
			  if (payload.size() == 2) { // uint16固定2字节
            INS16StatusData statusData = parseINS16Status(payload);
            if (statusData.isValid()) {
                // 拼接结构化描述（用于UI展示）
                feedbackDesc = QString(
                    "工作状态：%1\n组合状态：%2\n自检状态：%3"
                ).arg(insWorkStateToString(statusData.workState))
                 .arg(insCombinationStateToString(statusData.combinationState))
                 .arg(insSelfCheckStateToString(statusData.selfCheckState));
                
                // 存储各状态参数（供UI精细化展示）
                result.setParameter("ins_work_state", insWorkStateToString(statusData.workState));
                result.setParameter("ins_combination_state", insCombinationStateToString(statusData.combinationState));
                result.setParameter("ins_selfcheck_state", insSelfCheckStateToString(statusData.selfCheckState));
                status = ResultStatus::Success;
            } else {
                status = ResultStatus::Failed;
                feedbackDesc = "惯导状态码解析失败（位值非法）";
            }
        } else if (!payload.isEmpty()) {
            status = ResultStatus::Failed;
            feedbackDesc = QString("惯导状态码长度错误（需2字节，实际%1字节）").arg(payload.size());
        } else {
            status = ResultStatus::Failed;
            feedbackDesc = "惯导状态码响应为空";
        }
        break;
		case CommandCode::Common_GetFault: // 故障类型
			if (payload.size() >= 1) {
				PayloadResult res = static_cast<PayloadResult>(static_cast<uint8_t>(payload[0]));
				status = (res == PayloadResult::Success) ? ResultStatus::Success : ResultStatus::Failed;
				QString faultCode = (payload.size() > 1) ?
					QString(", 故障码:0x%1").arg(QString(payload.mid(1).toHex().toUpper())) : "";
				feedbackDesc = (status == ResultStatus::Success ? "正常" : "故障") + faultCode;
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = "故障响应为空";
			}
			break;
		case CommandCode::INS_CombinedWithDVL: // 强制dvl组合
			if (payload.size() == 1) {
				PayloadResult res = static_cast<PayloadResult>(static_cast<uint8_t>(payload[0]));
				status = (res == PayloadResult::Success) ? ResultStatus::Success : ResultStatus::Failed;
				feedbackDesc = EnumConverter::payloadResultToString(res);
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望1字节，实际%1字节").arg(payload.size());
			}
			break;
		case CommandCode::INS_CombinedAuto: // 自由组合
			if (payload.size() == 1) {
				PayloadResult res = static_cast<PayloadResult>(static_cast<uint8_t>(payload[0]));
				status = (res == PayloadResult::Success) ? ResultStatus::Success : ResultStatus::Failed;
				feedbackDesc = EnumConverter::payloadResultToString(res);
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望1字节，实际%1字节").arg(payload.size());
			}
			break;
		default:
			feedbackDesc = "惯导未知命令";
			status = ResultStatus::Unknown;
		}
	}
	// DVL解析
	else if (deviceId == DeviceId::DVL) {
		switch (cmdCode) {
		case CommandCode::Common_PowerOn:
		case CommandCode::Common_PowerOff:
			if (payload.size() == 1) {
				PayloadResult res = static_cast<PayloadResult>(static_cast<uint8_t>(payload[0]));
				status = (res == PayloadResult::Success) ? ResultStatus::Success : ResultStatus::Failed;
				feedbackDesc = EnumConverter::payloadResultToString(res);
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望1字节，实际%1字节").arg(payload.size());
			}
			break;
		case CommandCode::DVL_GetBottomSpeed: // 对底速度
		{
			QString speedStr = QString(payload).simplified().replace(" ", ""); // 清理空格
			QStringList speedParts = speedStr.split(",");
			if (speedParts.size() == 3) {
				// 解析三轴速度（右/前/上，mm/s，保留2位小数）
				double xSpeed = speedParts[0].toDouble();
				double ySpeed = speedParts[1].toDouble();
				double zSpeed = speedParts[2].toDouble();

				// 拼接结构化反馈描述
				feedbackDesc = QString("对底速度（右,前,上 mm/s）：%1,%2,%3")
					.arg(xSpeed, 0, 'f', 2)
					.arg(ySpeed, 0, 'f', 2)
					.arg(zSpeed, 0, 'f', 2);

				// 存储参数（供UI精细化展示，命名区分对底/对水）
				result.setParameter("dvl_bottom_x", xSpeed);
				result.setParameter("dvl_bottom_y", ySpeed);
				result.setParameter("dvl_bottom_z", zSpeed);
				status = ResultStatus::Success;
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("对底速度格式错误（需3轴数值），实际：%1").arg(speedStr);
			}
			break;
		}
		case CommandCode::DVL_GetWaterSpeed: // 对水速度
		{
			QString speedStr = QString(payload).simplified().replace(" ", ""); // 清理空格
			QStringList speedParts = speedStr.split(",");
			if (speedParts.size() == 3) {
				// 解析三轴速度（右/前/上，mm/s，保留2位小数）
				double xSpeed = speedParts[0].toDouble();
				double ySpeed = speedParts[1].toDouble();
				double zSpeed = speedParts[2].toDouble();

				// 拼接结构化反馈描述
				feedbackDesc = QString("对水速度（右,前,上 mm/s）：%1,%2,%3")
					.arg(xSpeed, 0, 'f', 2)
					.arg(ySpeed, 0, 'f', 2)
					.arg(zSpeed, 0, 'f', 2);

				// 存储参数（供UI精细化展示）
				result.setParameter("dvl_water_x", xSpeed);
				result.setParameter("dvl_water_y", ySpeed);
				result.setParameter("dvl_water_z", zSpeed);
				status = ResultStatus::Success;
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("对水速度格式错误（需3轴数值），实际：%1").arg(speedStr);
			}
			break;
		}
		case CommandCode::DVL_GetStatus:
			if (payload.size() == 1) { // 状态码是uint8，固定1字节
		// 1. 提取uint8状态字节
				uint8_t statusByte = static_cast<uint8_t>(payload[0]);

				// 2. 解析0-2bit（自检+存储卡状态，掩码0x07=00000111）
				uint8_t selfCheckCardState = statusByte & 0x07;
				QString selfCheckCardDesc;
				switch (selfCheckCardState) {
				case 0x00: // 000
					selfCheckCardDesc = "初始状态";
					break;
				case 0x01: // 001
					selfCheckCardDesc = "自检失败，存储卡读取成功";
					break;
				case 0x02: // 010
					selfCheckCardDesc = "自检失败，存储卡读取失败";
					break;
				case 0x03: // 011
					selfCheckCardDesc = "自检成功，存储卡读取成功";
					break;
				case 0x04: // 100
					selfCheckCardDesc = "自检成功，存储卡读取失败";
					break;
				default: // 101/110/111（非法值）
					selfCheckCardDesc = "非法状态（0-2bit值：0x" + QString::number(selfCheckCardState, 16) + "）";
					break;
				}

				// 3. 解析bit3（DVL输出状态，掩码0x08=00001000，右移3位）
				uint8_t outputState = (statusByte & 0x08) >> 3;
				QString outputDesc = (outputState == 0) ? "DVL无输出" : "DVL有输出";

				// 4. 校验状态合法性（仅0-4为合法值）
				bool isStateValid = (selfCheckCardState <= 0x04);
				if (isStateValid) {
					// 拼接结构化描述（和惯导风格一致）
					feedbackDesc = QString(
						"自检+存储卡状态：%1\n输出状态：%2"
					).arg(selfCheckCardDesc).arg(outputDesc);

					// 存储精细化参数（供UI单独展示）
					result.setParameter("dvl_selfcheck_card_state", selfCheckCardDesc);
					result.setParameter("dvl_output_state", outputDesc);
					result.setParameter("dvl_status_byte", QString("0x%1").arg(QString::number(statusByte, 16).toUpper()));
					status = ResultStatus::Success;
				}
				else {
					status = ResultStatus::Failed;
					feedbackDesc = QString("DVL状态码解析失败（0-2bit位值非法：0x%1）").arg(QString::number(selfCheckCardState, 16));
				}
			}
			else if (!payload.isEmpty()) {
				status = ResultStatus::Failed;
				feedbackDesc = QString("DVL状态码长度错误（需1字节，实际%1字节）").arg(payload.size());
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = "DVL状态码响应为空";
			}
			break;
		default:
			feedbackDesc = "DVL未知命令";
			status = ResultStatus::Unknown;
		}
	}

	// 摄像机与光源解析
	else if (deviceId == DeviceId::CameraLight) {
		switch (cmdCode) {
		case CommandCode::Common_PowerOn:
		case CommandCode::Common_PowerOff:
		case CommandCode::CameraLight_StartVideoSave:
		case CommandCode::CameraLight_StopVideoSave:
		case CommandCode::CameraLight_TakePhoto:
			if (payload.size() == 1) {
				PayloadResult res = static_cast<PayloadResult>(static_cast<uint8_t>(payload[0]));
				status = (res == PayloadResult::Success) ? ResultStatus::Success : ResultStatus::Failed;
				feedbackDesc = EnumConverter::payloadResultToString(res);
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望1字节，实际%1字节").arg(payload.size());
			}
			break;
		case CommandCode::Common_GetFault: // 设备状态
			if (payload.size() >= 1) {
				PayloadResult res = static_cast<PayloadResult>(static_cast<uint8_t>(payload[0]));
				status = (res == PayloadResult::Success) ? ResultStatus::Success : ResultStatus::Failed;
				QString faultCode = (payload.size() > 1) ?
					QString(", 故障码:0x%1").arg(QString(payload.mid(1).toHex().toUpper())) : "";
				feedbackDesc = (status == ResultStatus::Success ? "正常" : "故障") + faultCode;
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = "状态响应为空";
			}
			break;
		default:
			feedbackDesc = "摄像机未知命令";
			status = ResultStatus::Unknown;
		}
	}
	// 电池（动力/仪表）解析
	else if (DeviceTypeHelper::isBattery(deviceId)) {
		switch (cmdCode) {
		case CommandCode::Battery_GetSoc: // SOC
			if (deviceId == DeviceId::MeterBattery) {
				if (payload.size() == 2) {
					uint16_t socRaw = FrameParser::bytesToUint16(payload); // 解析2字节uint16（大端序）
					double soc = socRaw * 0.1; // 比例尺0.1
					feedbackDesc = QString("电量SOC：%1%").arg(soc, 0, 'f', 1); // 保留1位小数
					result.setParameter("soc", socRaw); // 存储原始值，UI层统一换算
					status = ResultStatus::Success;
				}
				else {
					status = ResultStatus::Failed;
					feedbackDesc = QString("仪表电池SOC期望2字节，实际%1字节").arg(payload.size());
				}
			}
			// 2. 动力电池（1/2）：uint8（1字节），比例尺1
			else if (deviceId == DeviceId::PowerBattery1 || deviceId == DeviceId::PowerBattery2) {
				if (payload.size() == 1) {
					uint8_t soc = static_cast<uint8_t>(payload[0]);
					feedbackDesc = QString("电量SOC：%1%").arg(soc);
					result.setParameter("soc", soc); // 直接存储原始值
					status = ResultStatus::Success;
				}
				else {
					status = ResultStatus::Failed;
					feedbackDesc = QString("动力电池SOC期望1字节，实际%1字节").arg(payload.size());
				}
			}
			// 3. 未知电池类型
			else {
				status = ResultStatus::Unknown;
				feedbackDesc = QString("未知电池类型（DeviceId：%1），无法解析SOC").arg(static_cast<uint8_t>(deviceId));
			}
			break;
		case CommandCode::Battery_GetTotalVoltage: // 总电压
			if (payload.size() == 2) {
				uint16_t voltRaw = FrameParser::bytesToUint16(payload); // 大端序解析
				double volt = voltRaw * 0.1;
				feedbackDesc = QString("总电压：%1V").arg(volt, 0, 'f', 1);
				result.setParameter("volt", voltRaw); // 存储原始值，UI层再换算
				status = ResultStatus::Success;
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望2字节，实际%1字节").arg(payload.size());
			}
			break;
		case CommandCode::Battery_GetCurrent: // 电流
			if (payload.size() == 2) {
				// 修正：先解析为uint16，再强转为int16（适配协议的有符号数）
				uint16_t currUint = FrameParser::bytesToUint16(payload);
				int16_t currRaw = static_cast<int16_t>(currUint);
				double curr = currRaw * 0.1; // 协议比例尺0.1，原偏移逻辑错误，删除-500
				feedbackDesc = QString("电流：%1A（%2电）")
					.arg(curr, 0, 'f', 1)
					.arg(curr > 0 ? "放" : (curr < 0 ? "充" : "无"));
				result.setParameter("current", currRaw); // 存储原始int16值
				status = ResultStatus::Success;
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望2字节，实际%1字节").arg(payload.size());
			}
			break;
		case CommandCode::Battery_GetAvgTemp: // 平均温度（协议：int16，单位℃，比例尺1）
			if (payload.size() == 2) {
				uint16_t tempUint = FrameParser::bytesToUint16(payload);
				int16_t tempRaw = static_cast<int16_t>(tempUint);
				feedbackDesc = QString("平均温度：%1℃").arg(tempRaw);
				result.setParameter("avgTemp", tempRaw); // 存储原始int16值
				status = ResultStatus::Success;
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望2字节，实际%1字节").arg(payload.size());
			}
			break;
		case CommandCode::Common_GetFault: // 故障类型
			if (!payload.isEmpty()) {
				feedbackDesc = QString("故障码：0x%1").arg(QString(payload.toHex().toUpper()));
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = "故障码响应为空";
			}
			break;
		case CommandCode::Battery_HVOn:
		case CommandCode::Battery_HVOff:
		case CommandCode::Battery_Lock:
		case CommandCode::Battery_Unlock:
			if (payload.size() == 1) {
				PayloadResult res = static_cast<PayloadResult>(static_cast<uint8_t>(payload[0]));
				status = (res == PayloadResult::Success) ? ResultStatus::Success : ResultStatus::Failed;
				feedbackDesc = EnumConverter::payloadResultToString(res);
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望1字节，实际%1字节").arg(payload.size());
			}
			break;
		default:
			feedbackDesc = "电池未知命令";
			status = ResultStatus::Unknown;
		}
	}
	// 声纳（侧扫/前视/下视）解析
	else if (DeviceTypeHelper::isSonar(deviceId)) {
		switch (cmdCode) {
		case CommandCode::Common_PowerOn:
		case CommandCode::Common_PowerOff:
		case CommandCode::Sonar_SetParameter:
		case CommandCode::Common_Enable:
		case CommandCode::Common_Disable:
			if (payload.size() == 1) {
				PayloadResult res = static_cast<PayloadResult>(static_cast<uint8_t>(payload[0]));
				status = (res == PayloadResult::Success) ? ResultStatus::Success : ResultStatus::Failed;
				feedbackDesc = EnumConverter::payloadResultToString(res);
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望1字节，实际%1字节").arg(payload.size());
			}
			break;

		case CommandCode::Common_GetFault: // 设备状态
			if (payload.size() == 1) {
				PayloadResult res = static_cast<PayloadResult>(static_cast<uint8_t>(payload[0]));
				status = (res == PayloadResult::Success) ? ResultStatus::Success : ResultStatus::Failed;
				feedbackDesc = status == ResultStatus::Success ? "设备正常" : "设备故障";
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望1字节，实际%1字节").arg(payload.size());
			}
			break;
		default:
			feedbackDesc = "声纳未知命令";
			status = ResultStatus::Unknown;
		}
	}
	// 水声通信解析
	else if (deviceId == DeviceId::AcousticComm) {
		switch (cmdCode) {
		case CommandCode::Common_PowerOn:
		case CommandCode::Common_PowerOff:
		case CommandCode::AcousticComm_SelfCheck:
			if (payload.size() == 1) {
				PayloadResult res = static_cast<PayloadResult>(static_cast<uint8_t>(payload[0]));
				status = (res == PayloadResult::Success) ? ResultStatus::Success : ResultStatus::Failed;
				feedbackDesc = EnumConverter::payloadResultToString(res);
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望1字节，实际%1字节").arg(payload.size());
			}
			break;
		case CommandCode::Common_GetFault: // 设备状态
			if (payload.size() == 1) {
				PayloadResult res = static_cast<PayloadResult>(static_cast<uint8_t>(payload[0]));
				status = (res == PayloadResult::Success) ? ResultStatus::Success : ResultStatus::Failed;
				feedbackDesc = status == ResultStatus::Success ? "设备正常" : "设备故障";
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = QString("期望1字节，实际%1字节").arg(payload.size());
			}
			break;
		default:
			feedbackDesc = "水声通信未知命令";
			status = ResultStatus::Unknown;
		}
	}
	// ------------------------------ ：北斗通信（BeidouComm）解析分支 ------------------------------
	else if (deviceId == DeviceId::BeidouComm) {
	switch (cmdCode) {
		// 1. 无参查询命令（返回具体数据）
	case CommandCode::BeidouComm_GetSignalQuality: // 获取信号质量（1字节）
		if (payload.size() == 1) {
			BeidouSignalQuality quality = static_cast<BeidouSignalQuality>(static_cast<uint8_t>(payload[0]));
			switch (quality) {
			case BeidouSignalQuality::NoSignal: feedbackDesc = "无信号（0x00）"; break;
			case BeidouSignalQuality::VeryWeak: feedbackDesc = "信号极弱（0x01）"; break;
			case BeidouSignalQuality::Weak: feedbackDesc = "信号弱（0x02）"; break;
			case BeidouSignalQuality::Good: feedbackDesc = "信号良好（0x03）"; break;
			case BeidouSignalQuality::VeryGood: feedbackDesc = "信号很好（0x04）"; break;
			case BeidouSignalQuality::Excellent: feedbackDesc = "信号极佳（0x05）"; break;
			default: feedbackDesc = QString("未知信号质量（0x%1）").arg(static_cast<uint8_t>(quality), 2, 16, QChar('0'));
			}
			result.setParameter("signalQuality", static_cast<int>(quality));
			status = ResultStatus::Success;
		}
		else {
			status = ResultStatus::Failed;
			feedbackDesc = QString("信号质量响应长度异常（期望1字节，实际%1字节）").arg(payload.size());
		}
		break;

	case CommandCode::BeidouComm_GetIdentity: // 获取远程卡号（4字节uint32_t大端序）
		if (payload.size() == 4) {
			uint32_t cardNo = qFromBigEndian(*reinterpret_cast<const uint32_t*>(payload.data()));
			feedbackDesc = QString("远端北斗卡号：%1").arg(cardNo);
			result.setParameter("localCardNo", cardNo);
			status = ResultStatus::Success;
		}
		else {
			status = ResultStatus::Failed;
			feedbackDesc = QString("本机卡号响应长度异常（期望4字节，实际%1字节）").arg(payload.size());
		}
		break;

	case CommandCode::BeidouComm_GetTarget: // 获取目标卡号（4字节uint32_t大端序）
		if (payload.size() == 4) {
			uint32_t targetNo = qFromBigEndian(*reinterpret_cast<const uint32_t*>(payload.data()));
			feedbackDesc = QString("目标北斗卡号：%1").arg(targetNo);
			result.setParameter("targetCardNo", targetNo);
			status = ResultStatus::Success;
		}
		else if (payload.isEmpty()) {
			feedbackDesc = "未设置目标卡号";
			status = ResultStatus::Success;
		}
		else {
			status = ResultStatus::Failed;
			feedbackDesc = QString("目标卡号响应长度异常（期望4字节，实际%1字节）").arg(payload.size());
		}
		break;

	case CommandCode::BeidouComm_GetWhitelist: // 获取白名单（直接接收逗号分隔的7位卡号字符串）
	{
		// 1. 将payload转为字符串（编码根据实际调整）
		QString whitelistRawStr = QString::fromUtf8(payload);
		// 若为GBK编码，改为：QString whitelistRawStr = QString::fromLocal8Bit(payload);

		QStringList whitelistStr; // 存储有效7位卡号
		bool hasInvalidCard = false;

		// 2. 分割字符串
		QStringList rawCardList = whitelistRawStr.split(',', Qt::SkipEmptyParts);
		QStringList rawCardListCn = whitelistRawStr.split('，', Qt::SkipEmptyParts);
		rawCardList.append(rawCardListCn);

		// 3. 遍历校验
		for (const QString& rawCard : rawCardList) {
			QString cardNo = rawCard.trimmed();

			bool isAllDigit = true;
			for (const QChar& c : cardNo) {
				if (!c.isDigit()) {
					isAllDigit = false;
					break;
				}
			}

			if (cardNo.length() == 7 && isAllDigit) {
				if (cardNo != "0000000") {
					whitelistStr.append(cardNo);
				}
				else {
					hasInvalidCard = true;
					qWarning() << "北斗白名单无效卡号（全0）：" << cardNo;
				}
			}
			else {
				hasInvalidCard = true;
				qWarning() << "北斗白名单非法卡号（非7位数字）：" << cardNo;
			}
		}

		// 4. 去重（关键修改：用QSet手动去重，兼容所有Qt版本）
		QSet<QString> cardSet;
		for (const QString& card : whitelistStr) {
			cardSet.insert(card);
		}
		whitelistStr.clear();
		whitelistStr = cardSet.values();

		// 5. 构建反馈描述
		if (whitelistStr.isEmpty()) {
			if (hasInvalidCard) {
				feedbackDesc = "北斗白名单无有效卡号（仅包含非法/全0卡号）";
			}
			else {
				feedbackDesc = "北斗白名单为空";
			}
		}
		else {
			feedbackDesc = QString("北斗白名单（%1个有效）：%2")
				.arg(whitelistStr.size())
				.arg(whitelistStr.join(","));
		}

		// 6. 存储参数（QStringList支持QVariant转换）
		result.setParameter("whitelist", whitelistStr);
		status = ResultStatus::Success;

		// 空payload处理
		if (payload.isEmpty()) {
			feedbackDesc = "北斗白名单为空";
			status = ResultStatus::Success;
		}
	}
	break;

	case CommandCode::BeidouComm_GetPosition: // 获取定位信息（NMEA字符串，UTF-8编码）
		if (!payload.isEmpty()) {
			QString nmeaStr;
			bool parseSuccess = false;
			// 定义普通变量，用于接收经纬度等数据
			double lat = 0.0;
			double lon = 0.0;
			bool isValidFix = false;
			QString fixDesc = "";

			// 按行分割查找GNRMC语句
			QStringList lines = splitBufferByLine(payload);
			for (const QString& line : lines) {
				QString trimmedLine = line.trimmed();
				if (trimmedLine.isEmpty()) continue;
				qDebug() << "[BDSChannel] 原始AT反馈：" << trimmedLine;

				if (trimmedLine.startsWith("$GNRMC")) {
					nmeaStr = trimmedLine;
					if (trimmedLine.length() < 10 || !trimmedLine.contains('*')) {
						qWarning() << "[GNRMC解析] 无效的GNRMC语句：" << trimmedLine;
						continue;
					}
					// 直接调用改造后的parseGNRMC，经纬度等数据赋值给普通变量
					parseSuccess = parseGNRMC(trimmedLine, lat, lon, isValidFix, fixDesc);
					break;
				}
			}

			// 使用获取到的变量值，存入result供UI使用
			if (parseSuccess) {
				// 直接将普通变量的值存入result，无需JSON解析
				result.setParameter("latitude_degree", lat);
				result.setParameter("longitude_degree", lon);
				result.setParameter("is_valid_fix", isValidFix);
				result.setParameter("fixStatusDesc", fixDesc);
				result.setParameter("nmeaData", nmeaStr);

				if (isValidFix) {
					feedbackDesc = QString("北斗定位成功 - 纬度：%1°，经度：%2°，状态：%3")
						.arg(lat, 6, 'f', 4)
						.arg(lon, 6, 'f', 4)
						.arg(fixDesc);
				}
				else {
					feedbackDesc = QString("北斗定位无效 - 状态：%1").arg(fixDesc);
				}
				status = ResultStatus::Success;
			}
			else {
				status = ResultStatus::Failed;
				feedbackDesc = "未提取到有效定位数据";
			}
		}
		else {
			status = ResultStatus::Failed;
			feedbackDesc = "定位信息响应为空";
		}
		break;

	case CommandCode::BeidouComm_GetSysInfo: // 获取系统信息（固件版本，UTF-8字符串）
		if (!payload.isEmpty()) {
			QString sysInfo = QString::fromUtf8(payload);
			feedbackDesc = QString("北斗系统信息（固件版本）：%1").arg(sysInfo);
			result.setParameter("systemInfo", sysInfo);
			status = ResultStatus::Success;
		}
		else {
			status = ResultStatus::Failed;
			feedbackDesc = "系统信息响应为空";
		}
		break;

		// 2. 有参设置命令（返回成功/失败）
	case CommandCode::BeidouComm_SetTarget: // 设置目标卡号
	case CommandCode::BeidouComm_AddWhitelist: // 添加白名单
	case CommandCode::BeidouComm_DelWhitelist: // 删除白名单
		if (payload.size() == 1) {
			PayloadResult res = static_cast<PayloadResult>(static_cast<uint8_t>(payload[0]));
			status = (res == PayloadResult::Success) ? ResultStatus::Success : ResultStatus::Failed;
			feedbackDesc = QString("%1%2").arg(EnumConverter::commandCodeToString(deviceId, cmdCode)).arg(EnumConverter::payloadResultToString(res));
		}
		else {
			status = ResultStatus::Failed;
			feedbackDesc = QString("%1响应长度异常（期望1字节，实际%1字节）").arg(EnumConverter::commandCodeToString(deviceId, cmdCode)).arg(payload.size());
		}
		break;

		// 3. 控制命令（返回成功/失败）
	case CommandCode::BeidouComm_FactoryReset: // 恢复出厂设置
	case CommandCode::BeidouComm_Reboot: // 重启设备
	case CommandCode::BeidouComm_Test: // 通讯测试
		if (payload.size() == 1) {
			PayloadResult res = static_cast<PayloadResult>(static_cast<uint8_t>(payload[0]));
			status = (res == PayloadResult::Success) ? ResultStatus::Success : ResultStatus::Failed;
			feedbackDesc = QString("%1%2").arg(EnumConverter::commandCodeToString(deviceId, cmdCode)).arg(EnumConverter::payloadResultToString(res));
		}
		else {
			status = ResultStatus::Failed;
			feedbackDesc = QString("%1响应长度异常（期望1字节，实际%1字节）").arg(EnumConverter::commandCodeToString(deviceId, cmdCode)).arg(payload.size());
		}
		break;

		// 故障查询（通用命令）
	case CommandCode::Common_GetFault:
		if (payload.size() == 1) {
			PayloadResult res = static_cast<PayloadResult>(static_cast<uint8_t>(payload[0]));
			status = (res == PayloadResult::Success) ? ResultStatus::Success : ResultStatus::Failed;
			feedbackDesc = status == ResultStatus::Success ? "北斗模块状态正常" : "北斗模块存在故障";
		}
		else {
			status = ResultStatus::Failed;
			feedbackDesc = QString("北斗故障响应长度异常（期望1字节，实际%1字节）").arg(payload.size());
		}
		break;

		// 未知北斗命令
	default:
		feedbackDesc = QString("北斗未知命令（编码0x%1）").arg(static_cast<uint8_t>(cmdCode), 2, 16, QChar('0'));
		status = ResultStatus::Unknown;
		break;
	}
	}
	// 其他未覆盖器件
	else {
		feedbackDesc = QString("未处理器件（ID:0x%1）命令（0x%2），payload:0x%3")
			.arg(static_cast<uint8_t>(deviceId), 2, 16, QChar('0'))
			.arg(static_cast<uint8_t>(cmdCode), 2, 16, QChar('0'))
			.arg(QString(payload.toHex().toUpper()));
		status = ResultStatus::Unknown;
	}
	// 6. 生成结果并通知观察者
	result.setStatus(status);
	result.setFeedbackData(payload);
	result.setFeedbackDesc(feedbackDesc);
	notifyObservers(&result);



	// 输出日志
	qDebug() << "收到自检结果 - 器件：" << EnumConverter::deviceIdToString(deviceId)
		<< "命令：" << EnumConverter::commandCodeToString(deviceId,cmdCode)
		<< "状态：" << static_cast<int>(status)
		<< "描述：" << feedbackDesc;
	
}

void SelfCheckModule::parseBDSShortMessageResponse(const QJsonObject& shortMsgJson)
{
	// 1. 提取短报文核心字段
	DeviceId deviceId = static_cast<DeviceId>(shortMsgJson["device_id"].toInt(static_cast<int>(DeviceId::BeidouComm)));
	BDSShortMsgResult result = static_cast<BDSShortMsgResult>(shortMsgJson["result"].toInt());
	QByteArray sentData = QByteArray::fromHex(shortMsgJson["sent_data"].toString().toUtf8());
	QByteArray recvData = QByteArray::fromHex(shortMsgJson["recv_data"].toString().toUtf8());
	QString timestamp = shortMsgJson["timestamp"].toString();

	// 2. 构建自检结果
	SelfCheckResult checkResult(deviceId, CommandCode::BeidouComm_Test,
		(result == BDSShortMsgResult::Success) ? ResultStatus::Success : ResultStatus::Failed,
		CommandType::Internal);

	// 3. 补充结果详情
	checkResult.setFeedbackData(sentData); // 发送的数据作为反馈数据
	switch (result) {
	case BDSShortMsgResult::Success:
		checkResult.setFeedbackDesc(QString("短报文发送成功（发送：%1，回复：%2）")
			.arg(QString(sentData)).arg(QString(recvData)));
		break;
	case BDSShortMsgResult::Failed:
		checkResult.setFeedbackDesc(QString("短报文发送失败（发送：%1，回复不匹配：%2）")
			.arg(QString(sentData)).arg(QString(recvData)));
		break;
	case BDSShortMsgResult::Timeout:
		checkResult.setFeedbackDesc(QString("短报文发送超时（发送：%1，30秒未收到回复）")
			.arg(QString(sentData)));
		break;
	}

	// 4. 补充参数（供UI展示）
	checkResult.setParameter("sent_data_hex", shortMsgJson["sent_data"].toString());
	checkResult.setParameter("recv_data_hex", shortMsgJson["recv_data"].toString());
	checkResult.setParameter("timestamp", timestamp);

	// 5. 通知观察者（如UI模块更新状态）
	notifyObservers(&checkResult);
	qDebug() << "北斗短报文自检结果：" << checkResult.feedbackDesc();
}

void SelfCheckModule::parseWaterAcousticResponse(const QJsonObject& waJson)
{
	// 1. 提取基础字段
	int rawDeviceId = waJson["device_id"].toInt(-1); // 先取原始值，默认-1（无效值）
	DeviceId deviceId;

	if (rawDeviceId == -1) {
		// JSON缺失device_id字段 → 兜底为水声，并打印警告
		qWarning() << "[水声解析] JSON缺失device_id字段，兜底为水声设备";
		deviceId = DeviceId::AcousticComm;
	}
	else {
		// JSON有device_id → 转换为枚举，并校验是否为水声设备
		deviceId = static_cast<DeviceId>(rawDeviceId);
		if (deviceId != DeviceId::AcousticComm) {
			// 异常：非水声设备的JSON误传到该函数 → 打印警告，仍兜底为水声（保证逻辑不崩溃）
			qWarning() << QString("[水声解析] 异常：JSON的device_id是%1（非水声设备），强制兜底为水声").arg(rawDeviceId);
			deviceId = DeviceId::AcousticComm;
		}
	}
	CommandCode cmdCode = static_cast<CommandCode>(waJson["cmd_code"].toInt(static_cast<int>(CommandCode::Invalid)));
	WaterAcousticResult result = static_cast<WaterAcousticResult>(waJson["result"].toInt());
	QString timestamp = waJson["timestamp"].toString();

	// 2. 提取指令类型字段（核心）
	int cmdTypeInt = waJson["cmd_type"].toInt(static_cast<int>(WaterAcousticUplinkCmd::Unknown));
	WaterAcousticUplinkCmd cmdType = static_cast<WaterAcousticUplinkCmd>(cmdTypeInt);
	QString cmdTypeDesc = waJson["cmd_type_desc"].toString(WaterAcousticChannel::waUplinkCmdToString(cmdType));

	// 3. 区分消息类型（数据类/状态类），解析对应字段
	QString msgType = waJson["msg_type"].toString("water_acoustic_data");
	QByteArray sentData;
	QByteArray recvData;
	QString statusDesc;

	// 2. 构建自检结果（状态：Success/Failed）
	if (msgType == "water_acoustic_data") {
		// 数据类消息：解析发送/接收数据
		sentData = QByteArray::fromHex(waJson["sent_data"].toString().toUtf8());
		recvData = QByteArray::fromHex(waJson["recv_data"].toString().toUtf8());
	}
	else if (msgType == "water_acoustic_status") {
		// 状态类消息：解析状态描述
		statusDesc = waJson["status_desc"].toString();
	}

	// 4. 构建自检结果（状态：Success/Failed/Pending 适配）
	ResultStatus checkStatus = ResultStatus::Failed;
	if (result == WaterAcousticResult::Success) {
		checkStatus = ResultStatus::Success;
	}
	else if (result == WaterAcousticResult::Pending) {
		checkStatus = ResultStatus::Pending; // 需确保ResultStatus枚举包含Pending
	}

	SelfCheckResult checkResult(
		deviceId,
		CommandCode::AcousticComm_Test,
		checkStatus,
		CommandType::Internal
	);

	// 5. 补充结果详情（融合指令类型+不同场景描述）
	QString feedbackDesc;
	switch (result) {
	case WaterAcousticResult::Success:
		if (msgType == "water_acoustic_data") {
			feedbackDesc = QString("水声通信成功【指令：%1】（发送：%2，回复：%3）")
				.arg(cmdTypeDesc)
				.arg(QString(sentData.toHex()))
				.arg(QString(recvData.toHex()));
		}
		else {
			feedbackDesc = QString("水声通信状态成功【指令：%1】：%2")
				.arg(cmdTypeDesc)
				.arg(statusDesc);
		}
		break;
	case WaterAcousticResult::Failed:
		if (msgType == "water_acoustic_data") {
			feedbackDesc = QString("水声通信失败【指令：%1】（发送：%2，回复不匹配：%3）")
				.arg(cmdTypeDesc)
				.arg(QString(sentData.toHex()))
				.arg(QString(recvData.toHex()));
		}
		else {
			feedbackDesc = QString("水声通信状态失败【指令：%1】：%2")
				.arg(cmdTypeDesc)
				.arg(statusDesc);
		}
		break;
	case WaterAcousticResult::Pending:
		feedbackDesc = QString("水声通信等待中【指令：%1】：%2")
			.arg(cmdTypeDesc)
			.arg(statusDesc);
		break;
	case WaterAcousticResult::Timeout:
		feedbackDesc = QString("水声通信超时【指令：%1】（发送：%2，30秒未收到回复）")
			.arg(cmdTypeDesc)
			.arg(QString(sentData.toHex()));
		break;
	}

	// 6. 设置核心反馈信息
	checkResult.setFeedbackDesc(feedbackDesc);
	if (!sentData.isEmpty()) {
		checkResult.setFeedbackData(sentData); // 发送数据作为反馈（数据类消息）
	}
	else {
		checkResult.setFeedbackData(statusDesc.toUtf8()); // 状态描述作为反馈（状态类消息）
	}

	// 7. 补充UI展示参数（含指令类型）

	checkResult.setParameter("sent_data_hex", waJson["sent_data"].toString());
	checkResult.setParameter("recv_data_hex", waJson["recv_data"].toString());
	checkResult.setParameter("timestamp", timestamp);
	checkResult.setParameter("channel_type", "WaterAcoustic");
	checkResult.setParameter("cmd_type", static_cast<int>(cmdType)); // 指令枚举值
	checkResult.setParameter("cmd_type_desc", cmdTypeDesc); // 指令字符串描述
	checkResult.setParameter("status_desc", statusDesc); // 状态描述（状态类消息）
	checkResult.setParameter("msg_type", msgType); // 消息类型（区分数据/状态）

	// 8. 通知观察者（UI更新/结果存储）
	notifyObservers(&checkResult);
	qDebug() << "水声通信自检结果：" << checkResult.feedbackDesc();
}

void SelfCheckModule::parseBDSAtResponse(const QJsonObject& bdsJson)
{
	// 1. 提取北斗核心字段（增加字段存在性判断，避免空值异常）
	DeviceId deviceId = DeviceId::BeidouComm; // 固定北斗设备ID，兼容字段缺失场景
	if (bdsJson.contains("device_id")) {
		deviceId = static_cast<DeviceId>(bdsJson["device_id"].toInt(static_cast<int>(DeviceId::BeidouComm)));
	}

	CommandCode cmdCode = CommandCode::Invalid;
	if (bdsJson.contains("cmd_code")) {
		cmdCode = static_cast<CommandCode>(bdsJson["cmd_code"].toInt(static_cast<int>(CommandCode::Invalid)));
	}

	// 提取原始AT反馈（优先使用聚合后的反馈，兼容普通AT和白名单场景）
	QString rawAT = bdsJson.contains("raw_at_response") ? bdsJson["raw_at_response"].toString().trimmed() : "";
	QByteArray payload = rawAT.toUtf8(); // 原始AT反馈作为payload，保持数据完整性

	// 提取可选字段（兼容AT封装的专属数据）
	QString atResult = bdsJson.contains("result") ? bdsJson["result"].toString() : "unknown";
	QString atErrorMsg = bdsJson.contains("error_msg") ? bdsJson["error_msg"].toString() : "未知错误";
	QJsonArray whitelistCardsArray = bdsJson.contains("whitelist_cards") ? bdsJson["whitelist_cards"].toArray() : QJsonArray();
	QJsonObject gnrmcData = bdsJson.contains("gnrmc_data") ? bdsJson["gnrmc_data"].toObject() : QJsonObject();

	// 2. 获取适配器（增强容错，增加日志输出）
	InternalProtocolAdapter* adapter = InternalCommandManager::instance().getAdapter(deviceId);
	if (!adapter) {
		SelfCheckResult result(deviceId, cmdCode, ResultStatus::Failed, CommandType::Internal);
		result.setFeedbackData(payload);
		QString errorDesc = QString("未找到北斗设备适配器，设备ID=%1（0x%2）")
			.arg(static_cast<int>(deviceId))
			.arg(static_cast<int>(deviceId), 2, 16, QChar('0'));
		result.setFeedbackDesc(errorDesc);
		notifyObservers(&result);
		qWarning() << "北斗解析失败：" << errorDesc;
		return;
	}

	// 3. 核心修改：清理内部指令状态（增加日志，优化容错）
	QList<QTimer*> timersToDelete;
	{
		QMutexLocker locker(&m_mutex);
		QString cmdKey = generateInternalCmdKey(deviceId, cmdCode);
		qDebug() << "[北斗解析] 生成内部指令Key：" << cmdKey << "，设备ID：" << static_cast<int>(deviceId) << "，命令码：" << static_cast<int>(cmdCode);

		// 容错判断：仅当存在对应状态时才清理
		if (m_internalCmdStates.contains(cmdKey)) {
			CommandState& state = m_internalCmdStates[cmdKey];
			// 安全清理定时器
			if (state.timer) {
				timersToDelete.append(state.timer);
				state.timer = nullptr; // 置空避免野指针
			}
			state.isProcessed = true;
			m_internalCmdStates.remove(cmdKey); // 移除指令状态
			// 容错：仅当packetSeq有效时才清理定时器映射
			if (state.packetSeq != static_cast<uint8_t>(-1)) {
				m_cmdTimerMap.remove(state.packetSeq);
			}
			qDebug() << "[北斗解析] 成功清理内部指令状态，Key：" << cmdKey;
		}
		else {
			qDebug() << "[北斗解析] 未找到对应内部指令状态，Key：" << cmdKey;
		}
	}

	// 4. 锁外安全停止并删除定时器（无差别容错，避免空指针）
	for (QTimer* timer : timersToDelete) {
		if (timer) {
			// 双重判断：确保定时器未被销毁
			if (timer->parent() ) {
				timer->stop();
				timer->disconnect(); // 断开所有信号槽，避免野回调
				timer->deleteLater();
				qDebug() << "[北斗解析] 成功删除内部指令定时器";
			}
		}
	}

	// 5. 调用适配器解析（严格遵循协议）
	SelfCheckResult result = adapter->parseResponse(cmdCode, payload);

	// 6. 核心修改1：优先使用AT封装的结果状态（覆盖适配器默认状态）
	result.setDeviceId(deviceId);
	result.setCommandType(CommandType::Internal);
	result.setFeedbackData(payload);

	// 覆盖结果状态和错误描述
	if (atResult == "failed") {
		result.setStatus(ResultStatus::Failed);
		// 优先使用AT封装的错误信息，无则保留适配器描述
		if (!atErrorMsg.isEmpty()) {
			result.setFeedbackDesc(atErrorMsg);
		}
	}
	else if (atResult == "success" && result.status() == ResultStatus::Unknown) {
		// 适配器返回未知状态时，优先标记为成功
		result.setStatus(ResultStatus::Success);
	}

	// 7. 增强白名单解析（提取卡号列表，补充参数和反馈描述）
	if ((cmdCode == CommandCode::BeidouComm_AddWhitelist ||
		cmdCode == CommandCode::BeidouComm_DelWhitelist ||
		cmdCode == CommandCode::BeidouComm_GetWhitelist) &&
		!whitelistCardsArray.isEmpty()) {
		// 转换QJsonArray为QStringList（方便UI使用）
		QStringList whitelistCards;
		for (const QJsonValue& val : whitelistCardsArray) {
			whitelistCards.append(val.toString().trimmed());
		}

		// 补充白名单参数到结果中
		result.setParameter("whitelist_cards", whitelistCards);
		result.setParameter("whitelist_count", whitelistCards.count());

		// 优化白名单反馈描述
		QString originalDesc = result.feedbackDesc();
		QString cmdName = EnumConverter::commandCodeToString(deviceId, cmdCode);
		QString enhancedWhitelistDesc = QString("%1 | 卡号数量：%2 | 卡号列表：%3")
			.arg(originalDesc)
			.arg(whitelistCards.count())
			.arg(whitelistCards.join(","));
		result.setFeedbackDesc(enhancedWhitelistDesc);

		qDebug() << "[北斗解析] 白名单解析完成：" << cmdName << "，卡号数量：" << whitelistCards.count();
	}

	// 8. 优化GNRMC定位解析（增强容错，处理无效定位）
	if (cmdCode == CommandCode::BeidouComm_GetPosition) {
		bool isValidFix = false;
		double latDegree = 0.0;
		double lonDegree = 0.0;
		QString fixStatusDesc = "未知定位状态";

		// 先判断GNRMC是否存在错误
		if (gnrmcData.contains("error")) {
			// 无效定位：强制标记为失败，更新描述
			result.setStatus(ResultStatus::Failed);
			QString gnrmcError = gnrmcData["error"].toString("无效的GNRMC语句");
			QString errorDesc = QString("定位解析失败：%1 | 原始NMEA：%2").arg(gnrmcError).arg(rawAT);
			result.setFeedbackDesc(errorDesc);
			qWarning() << "[北斗解析] 定位失败：" << gnrmcError;
		}
		else {
			// 有效GNRMC：提取字段（增加存在性判断）
			isValidFix = gnrmcData.contains("is_valid_fix") ? gnrmcData["is_valid_fix"].toBool() : false;
			latDegree = gnrmcData.contains("latitude_degree") ? gnrmcData["latitude_degree"].toDouble() : 0.0;
			lonDegree = gnrmcData.contains("longitude_degree") ? gnrmcData["longitude_degree"].toDouble() : 0.0;
			fixStatusDesc = isValidFix ? "有效定位" : "无效定位";

			// 补充定位参数
			result.setParameter("latitude_degree", latDegree);
			result.setParameter("longitude_degree", lonDegree);
			result.setParameter("is_valid_fix", isValidFix);

			// 优化反馈描述：优先显示直观经纬度
			QString originalDesc = result.feedbackDesc();
			QString enhancedDesc = QString("%1 | %2：纬度%3°（精度6位），经度%4°（精度6位）")
				.arg(originalDesc)
				.arg(fixStatusDesc)
				.arg(latDegree, 0, 'f', 6)
				.arg(lonDegree, 0, 'f', 6);
			result.setFeedbackDesc(enhancedDesc);

			qDebug() << "[北斗解析] 定位解析完成：" << fixStatusDesc << "，纬度：" << latDegree << "，经度：" << lonDegree;
		}
	}

	// 9. 通知观察者并输出详细日志
	notifyObservers(&result);

	// 优化日志输出：显示设备名称、命令名称、状态名称（更直观）
	QString deviceName = EnumConverter::deviceIdToString(deviceId);
	QString cmdName = EnumConverter::commandCodeToString(deviceId, cmdCode);
	qDebug() << "北斗自检结果 - 设备：" << deviceName
		<< "（ID：0x" << QString::number(static_cast<int>(deviceId), 16).toUpper() << "）"
		<< "命令：" << cmdName
		<< "（编码：0x" << QString::number(static_cast<int>(cmdCode), 16).toUpper() << "）"
		<< "（编码：" << static_cast<int>(result.status()) << "）"
		<< "描述：" << result.feedbackDesc();
}

uint8_t SelfCheckModule::getNextPacketSeq() {
	// 原子自增，确保多线程环境下序号唯一（0-255循环）
	uint8_t seq = m_nextPacketSeq.fetchAndAddRelaxed(1);
	// 跳过0（避免与默认无效值冲突），循环到1
	if (seq == 0 || seq > FrameConstants::MAX_PACKET_SEQ) {
		m_nextPacketSeq.storeRelaxed(1);
		return 1;
	}
	return seq;
}

/**
 * @brief 根据设备类型和命令动态调整超时时间
 * @param deviceId 设备id
 * @param cmdCode 命令码
 * @return 超时时间ms
*/
int SelfCheckModule::getDynamicTimeoutMs(DeviceId deviceId, CommandCode cmdCode)
{
	// 水声通信/卫星通信设备：延长超时（10-30秒）
	if (deviceId == DeviceId::AcousticComm || deviceId == DeviceId::BeidouComm || deviceId == DeviceId::DropWeight) {
		return 60000; // 30秒
	}
	else if (DeviceTypeHelper::isThruster(deviceId)){
		if(CommandCode::Thruster_SetSpeedWithTime == cmdCode)
			return 6000000;
		else 
			return FrameConstants::DEFAULT_TIMEOUT_MS;
	}

	// 浮调/声纳：中等超时（5-10秒）
	else if (DeviceTypeHelper::isBuoyancy(deviceId) || DeviceTypeHelper::isSonar(deviceId)) {
		return 600000; 
	}
	else if (DeviceTypeHelper::isRudder(deviceId))
	{
		return 15000;
	}
	else if (DeviceId::INS == deviceId)
	{
		return 20000;
	}
	else if (DeviceId::DVL == deviceId)
	{
		return 60000;
	}

	else if (DeviceId::DropWeight == deviceId)
	{
		return 20000;
	}
	else if (DeviceTypeHelper::isBattery(deviceId))
	{
		return 60000;
	}
	// 上电/下电命令：延长超时（20-30秒）
	else if (cmdCode == CommandCode::Common_PowerOn) {
		return FrameConstants::POWER_ON_TIMEOUT_MS; // 20秒
	}
	else if (cmdCode == CommandCode::Common_PowerOff) {
		return FrameConstants::POWER_OFF_TIMEOUT_MS; // 10秒
	}
	// 其他设备/命令：默认超时（3秒）
	else {
		return FrameConstants::DEFAULT_TIMEOUT_MS;
	}
}

void SelfCheckModule::createRudderDevice(DeviceId id, const QString& name)
{
	SelfCheckDevice device(Subsystem::Steering, id, name);
	device.addSupportedCommand({ CommandCode::Common_PowerOn, "上电",
							  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
	device.addSupportedCommand({ CommandCode::Common_PowerOff, "下电",
							  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
	device.addSupportedCommand({ CommandCode::Rudder_SetAngle, "设置舵角",
							  std::make_shared<RudderAngleParameter>(300), "*10度(uint16)，反馈实际值" });
	device.addSupportedCommand({ CommandCode::Rudder_GetRunTime, "读取运行时间",
							  std::make_shared<EmptyParameter>(), "秒(uint32)" });
	device.addSupportedCommand({ CommandCode::Rudder_GetVoltage, "获取霍尔电压",
							  std::make_shared<EmptyParameter>(), "值/10=V(uint16)" });
	device.addSupportedCommand({ CommandCode::Rudder_GetAngle, "获取舵机角度",
							  std::make_shared<EmptyParameter>(), "值/10=度(uint16)" });
	device.addSupportedCommand({ CommandCode::Rudder_GetCurrent, "获取舵机电流",
							  std::make_shared<EmptyParameter>(), "值/100=A(uint16)" });
	device.addSupportedCommand({ CommandCode::Rudder_GetTemp, "获取舵机温度",
							  std::make_shared<EmptyParameter>(), "值/10=℃(uint16)" });
	device.addSupportedCommand({ CommandCode::Rudder_SetZero, "设置零位",
						  std::make_shared<EmptyParameter>(), "响应0x00/0xFF" });
	device.addSupportedCommand({ CommandCode::Common_GetFault, "获取舵机故障",
							  std::make_shared<EmptyParameter>(), "不定字节故障码" });
;
	m_devices.append(device);
}

/**
 * @brief 通用命令状态清理函数（移除状态+停止销毁定时器）
 * @param packetSeq 包序号
 */
void SelfCheckModule::cleanupCommandState(uint8_t packetSeq)
{
	QList<QTimer*> timersToDelete; // 收集待删除定时器（锁外处理）
	{
		QMutexLocker locker(&m_mutex);
		// 1. 移除命令状态
		if (m_commandStates.contains(packetSeq)) {
			CommandState& state = m_commandStates[packetSeq];
			// 收集定时器，避免锁内操作事件循环
			if (state.timer) {
				timersToDelete.append(state.timer);
				state.timer = nullptr;
			}
			m_commandStates.remove(packetSeq);
		}
		// 2. 移除定时器映射
		if (m_cmdTimerMap.contains(packetSeq)) {
			m_cmdTimerMap.remove(packetSeq);
		}
	}

	// 3. 锁外停止并销毁定时器（避免死锁）
	for (QTimer* timer : timersToDelete) {
		if (timer) {
			timer->stop();
			timer->disconnect();
			timer->deleteLater();
		}
	}
}

bool SelfCheckModule::parseGNRMC(const QString& gnrmcStr,
	double& latitudeDegree,  // 十进制纬度（引用传出）
	double& longitudeDegree, // 十进制经度（引用传出）
	bool& isValidFix,        // 是否有效定位（引用传出）
	QString& fixStatusDesc)
{
	latitudeDegree = 0.0;
	longitudeDegree = 0.0;
	isValidFix = false;
	fixStatusDesc = "未知状态";

	// 按逗号分割GNRMC字段
	QStringList fields = gnrmcStr.split(',', Qt::KeepEmptyParts);
	if (fields.size() > 14) {
		fields = fields.mid(0, 14);
	}
	while (fields.size() < 14) {
		fields.append("");
	}

	// 提取核心字段（无需存入JSON，直接赋值给临时变量）
	QString fixStatus = fields[2];
	QString latitude = fields[3];
	QString latHemisphere = fields[4];
	QString longitude = fields[5];
	QString lonHemisphere = fields[6];

	// 经纬度转换逻辑（保持不变，直接赋值给引用参数）
	auto convertNMEAToDegree = [](const QString& nmeaStr, const QString& hemisphere) -> double {
		if (nmeaStr.isEmpty()) return 0.0;
		int degreeLen = (hemisphere == "N" || hemisphere == "S") ? 2 : 3;
		if (nmeaStr.length() < degreeLen) return 0.0;

		QString degreeStr = nmeaStr.left(degreeLen);
		QString minuteStr = nmeaStr.mid(degreeLen);
		bool degreeOk = false;
		bool minuteOk = false;
		double degree = degreeStr.toDouble(&degreeOk);
		double minute = minuteStr.toDouble(&minuteOk);
		if (!degreeOk || !minuteOk) return 0.0;

		return degree + (minute / 60.0);
	};

	// 直接计算经纬度，赋值给引用参数
	latitudeDegree = convertNMEAToDegree(latitude, latHemisphere);
	longitudeDegree = convertNMEAToDegree(longitude, lonHemisphere);
	if (latHemisphere == "S") latitudeDegree = -latitudeDegree;
	if (lonHemisphere == "W") longitudeDegree = -longitudeDegree;

	// 直接设置定位有效性和描述
	isValidFix = (fixStatus == "A") && !latitude.isEmpty() && !longitude.isEmpty()
		&& !latHemisphere.isEmpty() && !lonHemisphere.isEmpty();
	fixStatusDesc = (fixStatus == "A") ? "有效定位" : (fixStatus == "V") ? "无效定位" : "未知状态";

	// 日志打印
	if (!isValidFix) {
		qWarning() << "[GNRMC解析] 当前无有效定位，状态：" << fixStatusDesc;
		return false;
	}
	else {
		qInfo() << "[GNRMC解析] 有效定位 - 纬度：" << latitudeDegree << "°，经度：" << longitudeDegree << "°";
		return true;
	}
}

QStringList SelfCheckModule::splitBufferByLine(QByteArray recevBuffer)
{
	QStringList lines;
	QString bufferStr = QString::fromUtf8(recevBuffer).replace("\r\n", "\n");
	int splitIdx = bufferStr.lastIndexOf('\n');
	if (splitIdx == -1) return lines; // 无完整行

	// 提取所有完整行
	QString completeLines = bufferStr.left(splitIdx);
	lines = completeLines.split('\n', Qt::SkipEmptyParts);

	// 保留未完成的行到缓冲区
	recevBuffer = bufferStr.mid(splitIdx + 1).toUtf8();
	return lines;
}

QString SelfCheckModule::generateInternalCmdKey(DeviceId deviceId, CommandCode cmdCode)
{
	return QString("%1_%2_Internal").arg(static_cast<int>(deviceId)).arg(static_cast<int>(cmdCode));
}

std::shared_ptr<InternalCommandParam> SelfCheckModule::createBeidouSNDBParam(uint8_t packetSeq,
	DeviceId deviceId, CommandCode cmdCode, 
	const std::shared_ptr<CommandParameter>& extParam)
{
	auto internalParam = std::make_shared<InternalCommandParam>();

	// 1. 构建外部硬件帧（复用现有FrameBuilder，仅生成一次）
	QByteArray extFrame = FrameBuilder::buildCheckCommandFrame(
		deviceId, cmdCode, WorkPhase::ConnectivityTest,extParam ? extParam->toBytes() : QByteArray(), packetSeq
	);

	// 2. 存储核心参数（移除旧协议的长度字段X1，仅保留必要数据）
	internalParam->m_extraParams["packetSeq"] = packetSeq;       // 透传包序号
	internalParam->m_extraParams["snd_data"] = extFrame;          // 核心：AT+SND 后跟随的原始数据（明文/HEX）
	internalParam->m_extraParams["snd_data_hex"] = extFrame.toHex().toUpper(); // 可选：数据的HEX格式（若需要）
	internalParam->m_extraParams["deviceId"] = static_cast<int>(deviceId);
	internalParam->m_extraParams["cmdCode"] = static_cast<int>(cmdCode);

	return internalParam;
}

uint8_t SelfCheckModule::getUniquePacketSeq()
{
	QMutexLocker locker(&m_mutex); // 加锁保证线程安全（与原有逻辑一致）
	return getNextPacketSeq(); // 调用原有内部序号生成函数
}
