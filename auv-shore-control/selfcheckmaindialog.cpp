#include "selfcheckmaindialog.h"
#include <QTreeWidget>
#include <QMessageBox>
#include <QFile>
#include <qDebug>
#include <functional> 
#include <QPixmap>
#include <QToolTip>
#include "cameramediaseq.h"  // 包含头文件
#include "thrusterhandler.h"
#include "rudderhandler.h"
#include "commandregistry.h"
#include "depthmeterhandler.h"
#include "altimeterhandler.h"
#include "usblhandler.h"
#include "camerahandler.h"
#include "inshandler.h"
#include "buoyancyhandler.h"
#include "sonarhandler.h"
#include "batteryhandler.h"
#include "dvlhandler.h"
#include "dropweighthandler.h"
#include "acousticcommhandler.h"
#include "beidoucommhandler.h"
#pragma execution_character_set("utf-8")
SelfCheckMainDialog_533::SelfCheckMainDialog_533(SelfCheckModule* module, QWidget* parent)
	: ModuleDialogBase(parent)
	, m_module(module)
{
	ui.setupUi(this);
	if (!m_module) {
		QMessageBox::critical(this, "错误", "自检模块未初始化");
		return;
	}

	initialize();
}

SelfCheckMainDialog_533::~SelfCheckMainDialog_533()
{
	// 清理资源
	for (auto& handler : m_deviceHandlers) {
		if (handler) {
			handler->disconnectSignals();
		}
	}
	m_deviceHandlers.clear();
}

void SelfCheckMainDialog_533::onResultUpdated(const BaseResult* result)
{
	if (!result)
		return;

	//根据来源模块区分结果类型
	switch (result->sourceType())
	{
	case ModuleType::SelfCheck:
		//更新自检结果ui
		if (const auto* sendResult = dynamic_cast<const SelfCheckSendResult*>(result)) {
			handleSendResult(sendResult);
		}
	else if (const auto* selfResult = dynamic_cast<const SelfCheckResult*>(result)) {
			handleSelfCheckResult(selfResult);
	}
		break;
	default:
		break;
	}

}


void SelfCheckMainDialog_533::initialize()
{
	// 1. 注册为观察者
	m_module->addObserver(this);

	// 2. 加载样式
	loadStyleSheet(STYLE_SELFCHECK);

	// 3. 初始化页面映射
	initPageMapping();

	// 4. 初始化树控件
	initTreeWidget();

	// 5. 初始化设备处理器
	initDeviceHandlers();

	// 6. 初始化命令注册表
	initCommandRegistry();

	// 7. 连接信号
	connect(ui.treeWidget_mission, &QTreeWidget::currentItemChanged,
		this, &SelfCheckMainDialog_533::on_missionTreeWidget_currentItemChanged);
}

void SelfCheckMainDialog_533::initDeviceHandlers()
{
	// 创建并注册所有设备处理器
	m_deviceHandlers.clear();

	// 推进器处理器
	auto thrusterHandler = QSharedPointer<ThrusterHandler>::create(m_module, this);
	m_deviceHandlers[DeviceId::TailThruster] = thrusterHandler;
	m_deviceHandlers[DeviceId::FrontVerticalThruster] = thrusterHandler;
	m_deviceHandlers[DeviceId::RearVerticalThruster] = thrusterHandler;
	m_deviceHandlers[DeviceId::FrontSideThruster] = thrusterHandler;
	m_deviceHandlers[DeviceId::RearSideThruster] = thrusterHandler;

	// 舵机处理器
	auto rudderHandler = QSharedPointer<RudderHandler>::create(m_module, this);
	m_deviceHandlers[DeviceId::Rudder1] = rudderHandler;
	m_deviceHandlers[DeviceId::Rudder2] = rudderHandler;
	m_deviceHandlers[DeviceId::Rudder3] = rudderHandler;
	m_deviceHandlers[DeviceId::Rudder4] = rudderHandler;
	m_deviceHandlers[DeviceId::AntennaFold] = rudderHandler;

	// 电池处理器
	auto batteryHandler = QSharedPointer<BatteryHandler>::create(m_module, this);
	m_deviceHandlers[DeviceId::PowerBattery1] = batteryHandler;
	m_deviceHandlers[DeviceId::PowerBattery2] = batteryHandler;
	m_deviceHandlers[DeviceId::MeterBattery] = batteryHandler;

	// 声呐处理器
	auto sonarHandler = QSharedPointer<SonarHandler>::create(m_module, this);
	m_deviceHandlers[DeviceId::SideScanSonar] = sonarHandler;
	m_deviceHandlers[DeviceId::ForwardSonar] = sonarHandler;
	m_deviceHandlers[DeviceId::DownwardSonar] = sonarHandler;

	// 北斗处理器
	auto beidouHandler = QSharedPointer<BeidouCommHandler>::create(m_module, this);
	m_deviceHandlers[DeviceId::BeidouComm] = beidouHandler;

	// INS处理器
	auto insHandler = QSharedPointer<INSHandler>::create(m_module, this);
	m_deviceHandlers[DeviceId::INS] = insHandler;

	// 摄像机处理器
	auto cameraHandler = QSharedPointer<CameraHandler>::create(m_module, this);
	m_deviceHandlers[DeviceId::CameraLight] = cameraHandler;

	// DVL处理器
	auto dvlHandler = QSharedPointer<DvlHandler>::create(m_module, this);
	m_deviceHandlers[DeviceId::DVL] = dvlHandler;

	// 深度计处理器
	auto depthHandler = QSharedPointer<DepthMeterHandler>::create(m_module, this);
	m_deviceHandlers[DeviceId::DepthMeter] = depthHandler;

	// 高度计处理器
	auto altimeterHandler = QSharedPointer<AltimeterHandler>::create(m_module, this);
	m_deviceHandlers[DeviceId::Altimeter] = altimeterHandler;

	// USBL处理器
	auto usblHandler = QSharedPointer<USBLHandler>::create(m_module, this);
	m_deviceHandlers[DeviceId::USBL] = usblHandler;

	// 浮调处理器
	auto buoyancyHandler = QSharedPointer<BuoyancyHandler>::create(m_module, this);
	m_deviceHandlers[DeviceId::BuoyancyAdjust] = buoyancyHandler;

	// 抛载处理器
	auto dropWeightHandler = QSharedPointer<DropWeightHandler>::create(m_module, this);
	m_deviceHandlers[DeviceId::DropWeight] = dropWeightHandler;

	// 声通处理器
	auto acousticHandler = QSharedPointer<AcousticCommHandler>::create(m_module, this);
	m_deviceHandlers[DeviceId::AcousticComm] = acousticHandler;

	// 初始化所有处理器的UI
	if (thrusterHandler) {
		thrusterHandler->initializeUI();
		thrusterHandler->connectSignals();
	}

	if (rudderHandler) {
		rudderHandler->initializeUI();        // 初始化舵机UI（角度输入验证器等）
		rudderHandler->connectSignals();      // 连接舵机所有按钮信号（仅一次）
	}
	// 深度计
	if (depthHandler) {
		depthHandler->initializeUI();
		depthHandler->connectSignals();
	}

	// 高度计
	if (altimeterHandler) {
		altimeterHandler->initializeUI();
		altimeterHandler->connectSignals();
	}

	// USBL
	if (usblHandler) {
		usblHandler->disconnectSignals();
		usblHandler->initializeUI();
		usblHandler->connectSignals();
	}
	// 摄像头
	if (cameraHandler) {
		cameraHandler->initializeUI();
		cameraHandler->connectSignals();
	}

	// INS
	if (insHandler) {
		insHandler->initializeUI();
		insHandler->connectSignals();
	}

	// DVL
	if (dvlHandler) {
		dvlHandler->initializeUI();
		dvlHandler->connectSignals();
	}

	// 电池
	if (batteryHandler) {
		batteryHandler->initializeUI();
		batteryHandler->connectSignals();
	}

	//声呐
	if (sonarHandler)
	{
		sonarHandler->initializeUI();
		sonarHandler->connectSignals();
	}

	if (buoyancyHandler)
	{
		buoyancyHandler->initializeUI();
		buoyancyHandler->connectSignals();
	}
	// 抛载
	if (dropWeightHandler) {
		dropWeightHandler->initializeUI();
		dropWeightHandler->connectSignals();
	}

	// 水声通信
	if (acousticHandler) {
		acousticHandler->initializeUI();
		acousticHandler->connectSignals();
	}

	//北斗
	if (beidouHandler) {
		beidouHandler->initializeUI();
		beidouHandler->connectSignals();
	}
}

void SelfCheckMainDialog_533::initTreeWidget()
{
	// 1. 用std::function声明递归函数类型
	std::function<void(QTreeWidgetItem*, DevicePage)> setItemDataRecursive;

	// 2. 实现递归逻辑（此时可引用自身）
	setItemDataRecursive = [this, &setItemDataRecursive](QTreeWidgetItem* item, DevicePage parentPage)
	{
		QString itemText = item->text(0);

		// 根据节点文本和父节点类型，设置当前节点的DevicePage
		DevicePage currentPage = parentPage; // 默认继承父节点类型，再根据文本细化

		// 1. 处理“通信”节点及其子节点
		if (parentPage == DevicePage::CORRESPOND)
		{
			if (itemText == "有线") currentPage = DevicePage::CORRESPOND_WIRED;
			else if (itemText == "数传电台") currentPage = DevicePage::CORRESPOND_RADIO;
			else if (itemText == "WIFI") currentPage = DevicePage::CORRESPOND_WIFI;
			else if (itemText == "卫星通信") currentPage = DevicePage::CORRESPOND_BEIDOU_COM;
			else if (itemText == "北斗") currentPage = DevicePage::CORRESPOND_BEIDOU_COM;
			else if (itemText == "铱星") currentPage = DevicePage::CORRESPOND_IRIDIUM;
			else if (itemText == "水声通信")currentPage = DevicePage::CORRESPOND_WATERACOUSTIC;
		}
		// 2. 处理“导航定位”节点及其子节点
		else if (parentPage == DevicePage::NAVIGATIONWITHLOCATION)
		{
			if (itemText == "北斗") currentPage = DevicePage::NAV_BEIDOU;
			else if (itemText == "定位") currentPage = DevicePage::NAV_BEIDOU_POS;
			else if (itemText == "惯导") currentPage = DevicePage::NAV_INERTIAL;
			else if (itemText == "USBL") currentPage = DevicePage::NAV_USBL;
			else if (itemText == "DVL") currentPage = DevicePage::NAV_DVL;
			else if (itemText == "高度计") currentPage = DevicePage::NAV_ALTIMETER;
			else if (itemText == "深度计") currentPage = DevicePage::NAV_DEPTHMETER;
		}
		// 3. 处理“数据采集”节点及其子节点
		else if (parentPage == DevicePage::DATAACQUISITION) {
			if (itemText == "侧扫声呐") currentPage = DevicePage::DATA_SIDESCAN;
			else if (itemText == "前视声呐") currentPage = DevicePage::DATA_FORWARDSCAN;
			else if (itemText == "下视声呐") currentPage = DevicePage::DATA_DOWNWARDSCAN;
			else if (itemText == "摄像机与光源") currentPage = DevicePage::DATA_CAMERA;

		}
		// 4. 处理“推进装置”节点及其子节点
		else if (parentPage == DevicePage::PROMOTINGDEVICE) {
			if (itemText == "尾推1") currentPage = DevicePage::PROMOTE_TAIL;
			else if (itemText == "垂推") currentPage = DevicePage::PROMOTE_VERTICAL;
			else if (itemText == "前垂推") currentPage = DevicePage::PROMOTE_VERTICAL_FRONT;
			else if (itemText == "后垂推") currentPage = DevicePage::PROMOTE_VERTICAL_BACK;
			else if (itemText == "侧推") currentPage = DevicePage::PROMOTE_SIDE;
			else if (itemText == "前侧推") currentPage = DevicePage::PROMOTE_SIDE_FRONT;
			else if (itemText == "后侧推") currentPage = DevicePage::PROMOTE_SIDE_BACK;

		}
		// ：处理垂推父节点的子节点
		else if (parentPage == DevicePage::PROMOTE_VERTICAL) {
			if (itemText == "前垂推") currentPage = DevicePage::PROMOTE_VERTICAL_FRONT;
			else if (itemText == "后垂推") currentPage = DevicePage::PROMOTE_VERTICAL_BACK;
		}
		// ：处理侧推父节点的子节点
		else if (parentPage == DevicePage::PROMOTE_SIDE) {
			if (itemText == "前侧推") currentPage = DevicePage::PROMOTE_SIDE_FRONT;
			else if (itemText == "后侧推") currentPage = DevicePage::PROMOTE_SIDE_BACK;
		}
		// 5. 处理“转向装置”节点及其子节点
		else if (parentPage == DevicePage::STEERINGDEVICE) {
			if (itemText == "舵1") currentPage = DevicePage::STEER_RUDDER1;
			else if (itemText == "舵2") currentPage = DevicePage::STEER_RUDDER2;
			else if (itemText == "舵3") currentPage = DevicePage::STEER_RUDDER3;
			else if (itemText == "舵4") currentPage = DevicePage::STEER_RUDDER4;
			else if (itemText == "天线折叠机构") currentPage = DevicePage::PROMOTE_ANTENNA;
		}
		// 6. 处理“能源与续航”节点及其子节点
		else if (parentPage == DevicePage::ENERGYWITHENDURANCE) {
			if (itemText == "动力电池1") currentPage = DevicePage::ENERGY_POWERBAT1;
			else if (itemText == "动力电池2") currentPage = DevicePage::ENERGY_POWERBAT2;
			else if (itemText == "仪表电池") currentPage = DevicePage::ENERGY_METERBAT;
		}
		// 7. 处理“浮力调节”节点及其子节点
		else if (parentPage == DevicePage::BUOYANCYREGULATION) {
			if (itemText == "浮调1") currentPage = DevicePage::BUOYANCY_REG;

		}
		// 8. 处理“应急系统”节点及其子节点
		else if (parentPage == DevicePage::EMERGENCYSYSTEM) {
			if (itemText == "漏水传感器") currentPage = DevicePage::EMERGENCY_WATER;
			else if (itemText == "抛载") currentPage = DevicePage::EMERGENCY_JETTISON;
		}

		// 存储当前节点的DevicePage
		item->setData(0, Qt::UserRole, currentPage);

		// 递归处理子节点（传入当前节点的Page作为父Page）
		for (int i = 0; i < item->childCount(); ++i) {
			setItemDataRecursive(item->child(i), currentPage);
		}
	};

	// 处理顶级节点（父节点）
	for (int i = 0; i < ui.treeWidget_mission->topLevelItemCount(); ++i) {
		QTreeWidgetItem* topItem = ui.treeWidget_mission->topLevelItem(i);
		QString topText = topItem->text(0);
		DevicePage topPage;

		// 确定顶级节点的DevicePage
		if (topText == "通信") topPage = DevicePage::CORRESPOND;
		else if (topText == "导航定位") topPage = DevicePage::NAVIGATIONWITHLOCATION;
		else if (topText == "数据采集") topPage = DevicePage::DATAACQUISITION;
		else if (topText == "推进装置") topPage = DevicePage::PROMOTINGDEVICE;
		else if (topText == "转向装置") topPage = DevicePage::STEERINGDEVICE;
		else if (topText == "能源与续航") topPage = DevicePage::ENERGYWITHENDURANCE;
		else if (topText == "浮力调节") topPage = DevicePage::BUOYANCYREGULATION;
		else if (topText == "应急系统") topPage = DevicePage::EMERGENCYSYSTEM;

		// 为顶级节点设置数据，并递归处理其子节点
		topItem->setData(0, Qt::UserRole, topPage);
		setItemDataRecursive(topItem, topPage);
	}
}

void SelfCheckMainDialog_533::initPageMapping()
{
	m_pageIndexMap = {
		// 通信节点
		{DevicePage::CORRESPOND, 0},               // 通信父页面
		{DevicePage::CORRESPOND_WIRED, 0},         // 有线页面
		{DevicePage::CORRESPOND_RADIO, 0},         // 数传电台页面
		{DevicePage::CORRESPOND_WIFI, 0},          // WIFI页面
		{DevicePage::CORRESPOND_SATELLITE, 0},     // 卫星通信页面
		{DevicePage::CORRESPOND_BEIDOU_COM, 17},        // 北斗页面
		{DevicePage::CORRESPOND_IRIDIUM, 0},       // 铱星页面
		{DevicePage::CORRESPOND_WATERACOUSTIC, 30},//水声通信
	   // 推进装置节点
		{DevicePage::PROMOTINGDEVICE, 1},          // 推进装置父节点（默认显示尾推1）
		{DevicePage::PROMOTE_TAIL, 1},            // 尾推1

		{DevicePage::PROMOTE_VERTICAL, 5},         // 垂推（含子节点）
		{DevicePage::PROMOTE_VERTICAL_FRONT, 5},   // 前垂推（共用垂推页面）
		{DevicePage::PROMOTE_VERTICAL_BACK, 4},    // 后垂推（共用垂推页面）
		{DevicePage::PROMOTE_SIDE, 6},             // 侧推（含子节点）
		{DevicePage::PROMOTE_SIDE_FRONT, 6},       // 前侧推（共用侧推页面）
		{DevicePage::PROMOTE_SIDE_BACK, 3},        // 后侧推（共用侧推页面）


	   // 转向装置节点
		{DevicePage::STEERINGDEVICE, 7},           // 转向装置父节点（默认显示舵1）
		{DevicePage::STEER_RUDDER1, 7},            // 舵1
		{DevicePage::STEER_RUDDER2, 8},            // 舵2
		{DevicePage::STEER_RUDDER3, 9},            // 舵3
		{DevicePage::STEER_RUDDER4, 10},           // 舵4
		{DevicePage::PROMOTE_ANTENNA, 15},         // 天线折叠机构

		// 浮力调节节点
		{DevicePage::BUOYANCYREGULATION, 11},      // 浮力调节父节点（默认显示浮调1）
		{DevicePage::BUOYANCY_REG, 11},           // 浮调1


		// 应急系统节点
		{DevicePage::EMERGENCYSYSTEM, 13},         // 应急系统父节点（默认显示抛载）
		{DevicePage::EMERGENCY_WATER, 13},         // 漏水传感器（暂用抛载页面，可调整）
		{DevicePage::EMERGENCY_JETTISON, 13},      // 抛载

	   // 导航定位节点
		{DevicePage::NAVIGATIONWITHLOCATION, 16},  // 导航定位父节点（默认显示北斗定位）
		{DevicePage::NAV_BEIDOU, 16},              // 北斗（父节点，默认定位）
		{DevicePage::NAV_BEIDOU_POS, 16},          // 北斗->定位
		{DevicePage::NAV_DEPTHMETER, 18},          // 深度计
		{DevicePage::NAV_ALTIMETER, 19},           // 高度计
		{DevicePage::NAV_USBL, 20},                // USBL
		{DevicePage::NAV_INERTIAL, 21},            // 惯导
		{DevicePage::NAV_DVL, 22},                 // DVL

		// 数据采集节点
		{DevicePage::DATAACQUISITION, 23},         // 数据采集父节点（默认显示摄像机与光源）
		{DevicePage::DATA_CAMERA, 23},             // 摄像机

		{DevicePage::DATA_SIDESCAN, 27},           // 侧扫声呐
		{DevicePage::DATA_FORWARDSCAN, 28},       // 前视声呐（第一个）
		{DevicePage::DATA_DOWNWARDSCAN, 29},       //下视声呐（第二个，对应下视声呐）

		// 能源与续航节点
		{DevicePage::ENERGYWITHENDURANCE, 24},     // 能源父节点（默认显示动力电池）
		{DevicePage::ENERGY_POWERBAT1, 24},         // 动力电池1
		{DevicePage::ENERGY_POWERBAT2, 25},         // 动力电池2
		{DevicePage::ENERGY_METERBAT, 26}          // 仪表电池
	};
}

void SelfCheckMainDialog_533::initCommandRegistry()
{
	// 注册通用命令处理器
	auto& registry = CommandRegistry::instance();

	// 推进器命令
	REGISTER_COMMAND_HANDLER(DeviceId::TailThruster, CommandCode::Common_PowerOn,
		[this](const SelfCheckResult* result, QWidget* parent) {
			if (auto handler = m_deviceHandlers.value(result->deviceId())) {
				handler->handleResult(result);
			}
		});

	// 北斗命令
	REGISTER_COMMAND_HANDLER(DeviceId::BeidouComm, CommandCode::BeidouComm_GetSignalQuality,
		[this](const SelfCheckResult* result, QWidget* parent) {
			if (auto handler = m_deviceHandlers.value(result->deviceId())) {
				handler->handleResult(result);
			}
		});
	// ... 注册其他命令
}

void SelfCheckMainDialog_533::switchToPage(DevicePage page)
{
	if (m_pageIndexMap.contains(page)) {
		ui.sw_DevicePages->setCurrentIndex(m_pageIndexMap[page]);
	}
	else {
		qWarning() << "未找到页面映射：" << static_cast<int>(page);
	}
}

void SelfCheckMainDialog_533::setTreeWidgetItemData(QTreeWidget* treeWidget)
{
	// 1. 用std::function声明递归函数类型
	std::function<void(QTreeWidgetItem*, DevicePage)> setItemDataRecursive;

	// 2. 实现递归逻辑（此时可引用自身）
	setItemDataRecursive = [this, &setItemDataRecursive](QTreeWidgetItem* item, DevicePage parentPage)
	{
		QString itemText = item->text(0);

		// 根据节点文本和父节点类型，设置当前节点的DevicePage
		DevicePage currentPage = parentPage; // 默认继承父节点类型，再根据文本细化

		// 1. 处理“通信”节点及其子节点
		if (parentPage == DevicePage::CORRESPOND)
		{
			if (itemText == "有线") currentPage = DevicePage::CORRESPOND_WIRED;
			else if (itemText == "数传电台") currentPage = DevicePage::CORRESPOND_RADIO;
			else if (itemText == "WIFI") currentPage = DevicePage::CORRESPOND_WIFI;
			else if (itemText == "卫星通信") currentPage = DevicePage::CORRESPOND_BEIDOU_COM;
			else if (itemText == "北斗") currentPage = DevicePage::CORRESPOND_BEIDOU_COM;
			else if (itemText == "铱星") currentPage = DevicePage::CORRESPOND_IRIDIUM;
			else if (itemText == "水声通信")currentPage = DevicePage::CORRESPOND_WATERACOUSTIC;
		}
		// 2. 处理“导航定位”节点及其子节点
		else if (parentPage == DevicePage::NAVIGATIONWITHLOCATION)
		{
			if (itemText == "北斗") currentPage = DevicePage::NAV_BEIDOU;
			else if (itemText == "定位") currentPage = DevicePage::NAV_BEIDOU_POS;
			else if (itemText == "惯导") currentPage = DevicePage::NAV_INERTIAL;
			else if (itemText == "USBL") currentPage = DevicePage::NAV_USBL;
			else if (itemText == "DVL") currentPage = DevicePage::NAV_DVL;
			else if (itemText == "高度计") currentPage = DevicePage::NAV_ALTIMETER;
			else if (itemText == "深度计") currentPage = DevicePage::NAV_DEPTHMETER;
		}
		// 3. 处理“数据采集”节点及其子节点
		else if (parentPage == DevicePage::DATAACQUISITION) {
			if (itemText == "侧扫声呐") currentPage = DevicePage::DATA_SIDESCAN;
			else if (itemText == "前视声呐") currentPage = DevicePage::DATA_FORWARDSCAN;
			else if (itemText == "下视声呐") currentPage = DevicePage::DATA_DOWNWARDSCAN;
			else if (itemText == "摄像机与光源") currentPage = DevicePage::DATA_CAMERA;

		}
		// 4. 处理“推进装置”节点及其子节点
		else if (parentPage == DevicePage::PROMOTINGDEVICE) {
			if (itemText == "尾推1") currentPage = DevicePage::PROMOTE_TAIL;
			else if (itemText == "垂推") currentPage = DevicePage::PROMOTE_VERTICAL;
			else if (itemText == "前垂推") currentPage = DevicePage::PROMOTE_VERTICAL_FRONT;
			else if (itemText == "后垂推") currentPage = DevicePage::PROMOTE_VERTICAL_BACK;
			else if (itemText == "侧推") currentPage = DevicePage::PROMOTE_SIDE;
			else if (itemText == "前侧推") currentPage = DevicePage::PROMOTE_SIDE_FRONT;
			else if (itemText == "后侧推") currentPage = DevicePage::PROMOTE_SIDE_BACK;
			
		}
		// ：处理垂推父节点的子节点
		else if (parentPage == DevicePage::PROMOTE_VERTICAL) {
			if (itemText == "前垂推") currentPage = DevicePage::PROMOTE_VERTICAL_FRONT;
			else if (itemText == "后垂推") currentPage = DevicePage::PROMOTE_VERTICAL_BACK;
		}
		// ：处理侧推父节点的子节点
		else if (parentPage == DevicePage::PROMOTE_SIDE) {
			if (itemText == "前侧推") currentPage = DevicePage::PROMOTE_SIDE_FRONT;
			else if (itemText == "后侧推") currentPage = DevicePage::PROMOTE_SIDE_BACK;
		}
		// 5. 处理“转向装置”节点及其子节点
		else if (parentPage == DevicePage::STEERINGDEVICE) {
			if (itemText == "舵1") currentPage = DevicePage::STEER_RUDDER1;
			else if (itemText == "舵2") currentPage = DevicePage::STEER_RUDDER2;
			else if (itemText == "舵3") currentPage = DevicePage::STEER_RUDDER3;
			else if (itemText == "舵4") currentPage = DevicePage::STEER_RUDDER4;
			else if (itemText == "天线折叠机构") currentPage = DevicePage::PROMOTE_ANTENNA;
		}
		// 6. 处理“能源与续航”节点及其子节点
		else if (parentPage == DevicePage::ENERGYWITHENDURANCE) {
			if (itemText == "动力电池1") currentPage = DevicePage::ENERGY_POWERBAT1;
			else if (itemText == "动力电池2") currentPage = DevicePage::ENERGY_POWERBAT2;
			else if (itemText == "仪表电池") currentPage = DevicePage::ENERGY_METERBAT;
		}
		// 7. 处理“浮力调节”节点及其子节点
		else if (parentPage == DevicePage::BUOYANCYREGULATION) {
			if (itemText == "浮调1") currentPage = DevicePage::BUOYANCY_REG;
			
		}
		// 8. 处理“应急系统”节点及其子节点
		else if (parentPage == DevicePage::EMERGENCYSYSTEM) {
			if (itemText == "漏水传感器") currentPage = DevicePage::EMERGENCY_WATER;
			else if (itemText == "抛载") currentPage = DevicePage::EMERGENCY_JETTISON;
		}

		// 存储当前节点的DevicePage
		item->setData(0, Qt::UserRole, currentPage);

		// 递归处理子节点（传入当前节点的Page作为父Page）
		for (int i = 0; i < item->childCount(); ++i) {
			setItemDataRecursive(item->child(i), currentPage);
		}
	};

	// 处理顶级节点（父节点）
	for (int i = 0; i < treeWidget->topLevelItemCount(); ++i) {
		QTreeWidgetItem* topItem = treeWidget->topLevelItem(i);
		QString topText = topItem->text(0);
		DevicePage topPage;

		// 确定顶级节点的DevicePage
		if (topText == "通信") topPage = DevicePage::CORRESPOND;
		else if (topText == "导航定位") topPage = DevicePage::NAVIGATIONWITHLOCATION;
		else if (topText == "数据采集") topPage = DevicePage::DATAACQUISITION;
		else if (topText == "推进装置") topPage = DevicePage::PROMOTINGDEVICE;
		else if (topText == "转向装置") topPage = DevicePage::STEERINGDEVICE;
		else if (topText == "能源与续航") topPage = DevicePage::ENERGYWITHENDURANCE;
		else if (topText == "浮力调节") topPage = DevicePage::BUOYANCYREGULATION;
		else if (topText == "应急系统") topPage = DevicePage::EMERGENCYSYSTEM;

		// 为顶级节点设置数据，并递归处理其子节点
		topItem->setData(0, Qt::UserRole, topPage);
		setItemDataRecursive(topItem, topPage);
	}
}

void SelfCheckMainDialog_533::handleSendResult(const SelfCheckSendResult* result)
{
	if (!result) {
		qWarning() << "checkResult为空，跳过处理";
		return;
	}

	DeviceId deviceId = result->deviceId();
	CommandCode cmdCode = result->commandCode();
	QString deviceName = result->deviceName();
	QString feedbackDesc = result->feedbackDesc();
	ResultStatus status = result->status();

	QToolTip::showText(this->mapToGlobal(QPoint(100, 100)),
		QString("%1：%2").arg(deviceName).arg(feedbackDesc),
		this, QRect(), 3000); // 3秒后消失
	qDebug() << QString("[%1] 命令0x%2处理结果：%3")
		.arg(deviceName)
		.arg(static_cast<uint8_t>(cmdCode), 2, 16, QChar('0'))
		.arg(feedbackDesc);
}


void SelfCheckMainDialog_533::handleSelfCheckResult(const SelfCheckResult* result)
{
	if (!result) return;

	DeviceId deviceId = result->deviceId();
	CommandCode cmdCode = result->commandCode();

	// 1. 首先尝试使用设备处理器
	if (m_deviceHandlers.contains(deviceId)) {
		auto handler = m_deviceHandlers[deviceId];
		if (handler->supportsDevice(deviceId)) {
			handler->handleResult(result);
			return;
		}
	}

	// 2. 其次尝试使用命令注册表
	if (CommandRegistry::instance().handleCommand(deviceId, cmdCode, result, this)) {
		return;
	}

	// 3. 最后使用默认处理
	qWarning() << "未定义器件处理逻辑：" << EnumConverter::deviceIdToString(deviceId);
}

void SelfCheckMainDialog_533::on_missionTreeWidget_currentItemChanged(QTreeWidgetItem* currentItem, QTreeWidgetItem* previous)
{
	if (!currentItem)
		return;
	//获取节点的自定义数据
	DevicePage targetDevicePage = static_cast<DevicePage>(currentItem->data(0, Qt::UserRole).toInt());
	if (targetDevicePage == m_currentDevicePage)
	{
		return;
	}
	m_currentDevicePage = targetDevicePage;
	if (m_pageIndexMap.contains(targetDevicePage))
	{
		ui.sw_DevicePages->setCurrentIndex(m_pageIndexMap[targetDevicePage]);
	}
	else 
	{
		qWarning() << "未找到页面映射：" << currentItem->text(0);
	}
}

