#include "selfcheckmaindialog.h"
#include<QTreeWidget>
#include<QMessageBox>
#include<QFile>
#include<qDebug>
#include <functional> 
#include<QPixmap>
#include<QToolTip>
#include "cameramediaseq.h"  // 包含头文件
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
{}

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
	m_module->addObserver(this);
	// 加载QSS样式
	loadStyleSheet(STYLE_SELFCHECK);
	m_currentDevicePage = DevicePage::CORRESPOND;
	setTreeWidgetItemData(ui.treeWidget_mission);
	connect(ui.treeWidget_mission, &QTreeWidget::currentItemChanged,
		this, &SelfCheckMainDialog_533::on_missionTreeWidget_currentItemChanged);
	connect(ui.btn_01_PowerOn, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_tailThrusterPowerOnBtn_clicked);
	connect(ui.btn_01_PowerOff, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_tailThrusterPowerOffBtn_clicked);
	connect(ui.btn_01_ParamReset, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_tailThrusterParamResetBtn_clicked);
	connect(ui.btn_01_EnableWork, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_tailThrusterEnableWorkBtn_clicked);
	connect(ui.btn_01_SetTargetSpeed, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_tailThrusterSetSpeedBtn_clicked);
	connect(ui.btn_01_SetTargetDuty, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_tailThrusterSetDutyBtn_clicked);
	connect(ui.btn_01_SetAccelTime, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_tailThrusterSetAccelTimeBtn_clicked);
	connect(ui.btn_01_SetRunTime, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_tailThrusterSetRunTimeBtn_clicked);
	connect(ui.btn_01_ReadRunTime, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_tailThrusterGetRunTimeBtn_clicked);
	connect(ui.btn_01_ReadNoMaintainTime, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_tailThrusterGetNoMaintainTimeBtn_clicked);
	connect(ui.btn_01_ReadTargetDuty, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_tailThrusterGetDutyBtn_clicked);
	connect(ui.btn_01_ReadCurrent, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_tailThrusterGetCurrentBtn_clicked);
	connect(ui.btn_01_ReadTemp, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_tailThrusterGetTemperatureBtn_clicked);
	connect(ui.btn_01_ReadTargetSpeed, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_tailThrusterGetSpeedBtn_clicked);
	connect(ui.btn_01_ReadHall, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_tailThrusterGetHallBtn_clicked);
	connect(ui.btn_01_ReadVoltage, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_tailThrusterGetBusVoltageBtn_clicked);
	connect(ui.btn_01_ReadTotalRunTime, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_tailThrusterGetTotalRunTimeBtn_clicked);
	connect(ui.btn_01_SetSpeedWithTime, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_tailThrusterSetSpeedWithTimeBtn_clicked);
	connect(ui.btn_02_PowerOn, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder1PowerOnBtn_clicked);
	connect(ui.btn_02_PowerOff, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder1PowerOffBtn_clicked);
	connect(ui.btn_02_SetAngle, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder1SetAngleBtn_clicked);
	connect(ui.btn_02_ReadRunTime, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder1GetRunTimeBtn_clicked);
	connect(ui.btn_02_ReadVoltage, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder1GetVoltageBtn_clicked);
	connect(ui.btn_02_ReadCurrent, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder1GetCurrentBtn_clicked);
	connect(ui.btn_02_ReadTemp, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder1GetTempBtn_clicked);
	connect(ui.btn_02_ReadAngle, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder1GetAngleBtn_clicked);
	connect(ui.btn_02_SetZero, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder1SetZeroBtn_clicked);
	connect(ui.btn_03_PowerOn, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder2PowerOnBtn_clicked);
	connect(ui.btn_03_PowerOff, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder2PowerOffBtn_clicked);
	connect(ui.btn_03_SetAngle, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder2SetAngleBtn_clicked);
	connect(ui.btn_03_ReadRunTime, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder2GetRunTimeBtn_clicked);
	connect(ui.btn_03_ReadVoltage, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder2GetVoltageBtn_clicked);
	connect(ui.btn_03_ReadCurrent, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder2GetCurrentBtn_clicked);
	connect(ui.btn_03_ReadTemp, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder2GetTempBtn_clicked);
	connect(ui.btn_03_ReadAngle, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder2GetAngleBtn_clicked);
	connect(ui.btn_03_SetZero, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder2SetZeroBtn_clicked);

	connect(ui.btn_04_PowerOn, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder3PowerOnBtn_clicked);
	connect(ui.btn_04_PowerOff, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder3PowerOffBtn_clicked);
	connect(ui.btn_04_SetAngle, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder3SetAngleBtn_clicked);
	connect(ui.btn_04_ReadRunTime, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder3GetRunTimeBtn_clicked);
	connect(ui.btn_04_ReadVoltage, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder3GetVoltageBtn_clicked);
	connect(ui.btn_04_ReadCurrent, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder3GetCurrentBtn_clicked);
	connect(ui.btn_04_ReadTemp, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder3GetTempBtn_clicked);
	connect(ui.btn_04_ReadAngle, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder3GetAngleBtn_clicked);
	connect(ui.btn_04_SetZero, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder3SetZeroBtn_clicked);

	connect(ui.btn_05_PowerOn, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder4PowerOnBtn_clicked);
	connect(ui.btn_05_PowerOff, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder4PowerOffBtn_clicked);
	connect(ui.btn_05_SetAngle, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder4SetAngleBtn_clicked);
	connect(ui.btn_05_ReadRunTime, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder4GetRunTimeBtn_clicked);
	connect(ui.btn_05_ReadVoltage, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder4GetVoltageBtn_clicked);
	connect(ui.btn_05_ReadCurrent, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder4GetCurrentBtn_clicked);
	connect(ui.btn_05_ReadTemp, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder4GetTempBtn_clicked);
	connect(ui.btn_05_ReadAngle, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder4GetAngleBtn_clicked);
	connect(ui.btn_05_SetZero, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_rudder4SetZeroBtn_clicked);

	connect(ui.btn_06_PowerOn, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_buoyancyAdjustingPowerOnBtn_clicked);
	connect(ui.btn_06_PowerOff, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_buoyancyAdjustingPowerOffBtn_clicked);
	connect(ui.btn_06_SetBuoyancyValue, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_buoyancyAdjustingSetbuoyancyValue_clicked);
	connect(ui.btn_06_ReadBuoyancyValue, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_buoyancyAdjustingGetbuoyancyValue_clicked);
	connect(ui.btn_06_Stop, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_buoyancyAdjustingStop_clicked);
	connect(ui.btn_07_PowerOn, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_dropWeightPowerOnBtn_clicked);
	connect(ui.btn_07_PowerOff, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_dropWeightPowerOffBtn_clicked);
	connect(ui.btn_07_Release, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_dropWeighReleaseBtn_clicked);
	connect(ui.btn_07_Reset, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_dropWeightResetBtn_clicked);
	connect(ui.btn_07_StatusCheck, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_dropWeightGetFaultBtn_clicked);
	connect(ui.btn_08_PowerOn, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_antennaFoldPowerOnBtn_clicked);
	connect(ui.btn_08_PowerOff, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_antennaFoldPowerOffBtn_clicked);
	connect(ui.btn_08_SetAngle, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_antennaFoldSetAngle_clicked);
	connect(ui.btn_08_ReadRunTime, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_antennaFoldGetRunTime_clicked);
	connect(ui.btn_08_ReadVoltage, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_antennaFoldGetVoltage_clicked);
	connect(ui.btn_08_ReadCurrent, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_antennaFoldGetCurrent_clicked);
	connect(ui.btn_08_ReadTemp, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_antennaFoldGetTemp_clicked);
	connect(ui.btn_08_ReadAngle, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_antennaFoldGetAngle_clicked);

	connect(ui.btn_09_ReadSignalQuality_In, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_beidouGetSignalQualityBtnIn_clicked);
	connect(ui.btn_09_ReadIdentity_In, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_beidouGetIdentityBtnIn_clicked);
	connect(ui.btn_09_GetTarget_In, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_beidouGetTargetBtnIn_clicked);
	connect(ui.btn_09_SetTarget_In, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_beidouSetTargetBtnIn_clicked);
	connect(ui.btn_09_GetWhitelist_In, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_beidouGetWhitelistBtnIn_clicked);
	connect(ui.btn_09_AddWhitelist_In, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_beidouAddWhitelistBtnIn_clicked);
	connect(ui.btn_09_DelWhitelis_In, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_beidouDelWhitelistBtnIn_clicked);
	connect(ui.btn_09_GetPosition_In, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_beidouGetPositionBtnIn_clicked);
	connect(ui.btn_09_GetSysInfo_In, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_beidouGetSysInfoBtnIn_clicked);
	connect(ui.btn_09_FactoryReset_In, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_beidouFactoryResetBtnIn_clicked);
	connect(ui.btn_09_Reboot_In, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_beidouRebootBtnIn_clicked);

	connect(ui.btn_09_ReadSignalQuality, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_beidouGetSignalQualityBtn_clicked);
	connect(ui.btn_09_ReadIdentity, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_beidouGetIdentityBtn_clicked);
	connect(ui.btn_09_ReadTarget, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_beidouGetTargetBtn_clicked);
	connect(ui.btn_09_SetTarget, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_beidouSetTargetBtn_clicked);
	connect(ui.btn_09_GetWhitelist, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_beidouGetWhitelistBtn_clicked);
	connect(ui.btn_09_AddWhitelist, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_beidouAddWhitelistBtn_clicked);
	connect(ui.btn_09_DelWhitelis, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_beidouDelWhitelistBtn_clicked);
	connect(ui.btn_09_GetPosition, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_beidouGetPositionBtn_clicked);
	connect(ui.btn_09_GetSysInfo, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_beidouGetSysInfoBtn_clicked);
	connect(ui.btn_09_FactoryReset, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_beidouFactoryResetBtn_clicked);
	connect(ui.btn_09_Reboot, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_beidouRebootBtn_clicked);
	connect(ui.btn_09_CommTest, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_beidouCommTestBtn_clicked);
	connect(ui.btn_0A_PowerOn, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_depMetPowerOnBtn_clicked);
	connect(ui.btn_0A_PowerOff, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_depMetPowerOffBtn_clicked);
	connect(ui.btn_0A_SetReference, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_depMetCalibrateBtn_clicked);
	connect(ui.btn_0A_ReadDepth, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_depMetGetDepthBtn_clicked);
	connect(ui.btn_0B_PowerOn, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_altimeterPowerOnBtn_clicked);
	connect(ui.btn_0B_PowerOff, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_altimeterPowerOffBtn_clicked);
	connect(ui.btn_0B_ReadHeight, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_altimeterHeightBtn_clicked);
	connect(ui.btn_0C_PowerOn, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_usblPowerOnBtn_clicked);
	connect(ui.btn_0C_PowerOff, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_usblPowerOffBtn_clicked);
	connect(ui.btn_0D_PowerOn, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_insPowerOnBtn_clicked);
	connect(ui.btn_0D_PowerOff, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_insPowerOffBtn_clicked);
	connect(ui.btn_0D_SetGnssBind, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_insSetGnssBindBtn_clicked);
	connect(ui.btn_0D_SetCalibWithDvl, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_insCalibrationWithDVLBtn_clicked);
	connect(ui.btn_0D_SetCombinedWithDvl, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_insCombinedWithDVLBtn_clicked);
	connect(ui.btn_0D_SetCombinedAuto, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_insCombinedAutoBtn_clicked);
	connect(ui.btn_0D_GetUtcDate, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_insGetUtcDateBtn_clicked);
	connect(ui.btn_0D_GetUtcTime, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_insGetUtcTimeBtn_clicked);
	connect(ui.btn_0D_GetPosition, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_insGetPositionBtn_clicked);
	connect(ui.btn_0D_GetAttitude, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_insGetAttitudeBtn_clicked);
	connect(ui.btn_0D_GetBodySpeed, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_insGetBodySpeedBtn_clicked);
	connect(ui.btn_0D_GetGeoSpeed, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_insGetGeoSpeedBtn_clicked);
	connect(ui.btn_0D_GetGyroSpeed, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_insGetGyroscopeSpeedBtn_clicked);
	connect(ui.btn_0D_GetAccel, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_insGetAccelerationBtn_clicked);
	connect(ui.btn_0D_GetStatus, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_insGetStatusBtn_clicked);
	connect(ui.btn_0E_PowerOn, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_dvlPowerOnBtn_clicked);
	connect(ui.btn_0E_PowerOff, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_dvlPowerOffBtn_clicked);
	connect(ui.btn_0E_ReadBottomSpeed, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_dvlGetBottomSpeed_clicked);
	connect(ui.btn_0E_ReadWaterSpeed, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_dvlGetWaterSpeed_clicked);
	connect(ui.btn_0E_ReadStatus, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_dvlGetStatus_clicked);
	connect(ui.btn_0F_PowerOn, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_cameraPowerOnBtn_clicked);
	connect(ui.btn_0F_PowerOff, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_cameraPowerOffBtn_clicked);
	connect(ui.btn_0F_StartRecord, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_cameraStartRecordBtn_clicked);
	connect(ui.btn_0F_StopRecord, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_cameraStopRecordBtn_clicked);
	connect(ui.btn_0F_TakePhoto, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_cameraTakePhotoBtn_clicked);
	connect(ui.btn_10_ReadSOC, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_powerBattery1GetSoc_clicked);
	connect(ui.btn_10_ReadVoltage, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_powerBattery1GetTotalVoltage_clicked);
	connect(ui.btn_10_ReadCurrent, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_powerBattery1GetCurrent_clicked);
	connect(ui.btn_10_ReadAvgTemp, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_powerBattery1GetAvgTemp_clicked);
	connect(ui.btn_10_HVOn, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_powerBattery1HVOn_clicked);
	connect(ui.btn_10_HVOff, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_powerBattery1HVOff_clicked);
	connect(ui.btn_10_Lock, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_powerBattery1Lock_clicked);
	connect(ui.btn_10_Unlock, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_powerBattery1Unlock_clicked);

	connect(ui.btn_1A_ReadSOC, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_powerBattery2GetSoc_clicked);
	connect(ui.btn_1A_ReadVoltage, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_powerBattery2GetTotalVoltage_clicked);
	connect(ui.btn_1A_ReadCurrent, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_powerBattery2GetCurrent_clicked);
	connect(ui.btn_1A_ReadAvgTemp, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_powerBattery2GetAvgTemp_clicked);
	connect(ui.btn_1A_HVOn, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_powerBattery2HVOn_clicked);
	connect(ui.btn_1A_HVOff, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_powerBattery2HVOff_clicked);
	connect(ui.btn_1A_Lock, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_powerBattery2Lock_clicked);
	connect(ui.btn_1A_Unlock, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_powerBattery2Unlock_clicked);

	connect(ui.btn_11_ReadSOC, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_meterBatteryGetSoc_clicked);
	connect(ui.btn_11_ReadVoltage, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_meterBatteryGetTotalVoltage_clicked);
	connect(ui.btn_11_ReadCurrent, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_meterBatteryGetCurrent_clicked);
	connect(ui.btn_11_ReadAvgTemp, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_meterBatteryGetAvgTemp_clicked);
	connect(ui.btn_11_HVOn, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_meterBatteryHVOn_clicked);
	connect(ui.btn_11_HVOff, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_meterBatteryHVOff_clicked);
	connect(ui.btn_11_Lock, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_meterBatteryLock_clicked);
	connect(ui.btn_11_Unlock, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_meterBatteryUnlock_clicked);

	connect(ui.btn_12_PowerOn, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_sideScanSonarPowerOnBtn_clicked);
	connect(ui.btn_12_PowerOff, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_sideScanSonarPowerOffBtn_clicked);
	connect(ui.btn_12_EnableWork, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_sideScanSonarEnabelBtn_clicked);
	connect(ui.btn_12_DisableWork, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_sideScanSonarDisableBtn_clicked);
	connect(ui.btn_13_PowerOn, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_forwardSonarPowerOnBtn_clicked);
	connect(ui.btn_13_PowerOff, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_forwardSonarPowerOffBtn_clicked);
	connect(ui.btn_13_EnableWork, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_forwardSonarEnabelBtn_clicked);
	connect(ui.btn_13_DisableWork, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_forwardSonarDisableBtn_clicked);
	connect(ui.btn_14_PowerOn, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_downwardSonarPowerOnBtn_clicked);
	connect(ui.btn_14_PowerOff, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_downwardSonarPowerOffBtn_clicked);
	connect(ui.btn_14_EnableWork, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_downwardSonarEnabelBtn_clicked);
	connect(ui.btn_14_DisableWork, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_downwardSonarDisableBtn_clicked);
	connect(ui.btn_15_PowerOn, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_acousticCommPowerOn_clicked);
	connect(ui.btn_15_PowerOff, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_acousticCommPowerOff_clicked);
	connect(ui.btn_15_SelfCheck, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_acousticCommSelfCheck_clicked);
	connect(ui.btn_15_CommTest, &QPushButton::clicked,
		this, &SelfCheckMainDialog_533::on_acousticCommtTest_clicked);
	// 初始化页面映射表（仅一次）
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


void SelfCheckMainDialog_533::handleSelfCheckResult(const SelfCheckResult* checkResult)
{
	if (!checkResult) {
		qWarning() << "checkResult为空，跳过处理";
		return;
	}

	DeviceId deviceId = checkResult->deviceId();
	CommandCode cmdCode = checkResult->commandCode();
	QString deviceName = checkResult->deviceName();
	QString feedbackDesc = checkResult->feedbackDesc();
	ResultStatus status = checkResult->status();

	//QToolTip::showText(this->mapToGlobal(QPoint(100, 100)),
	//	QString("%1：%2").arg(deviceName).arg(feedbackDesc),
	//	this, QRect(), 3000); // 3秒后消失
	qDebug() << QString("[%1] 命令0x%2处理结果：%3")
		.arg(deviceName)
		.arg(static_cast<uint8_t>(cmdCode), 2, 16, QChar('0'))
		.arg(feedbackDesc);

	
	if (DeviceTypeHelper::isThruster(deviceId)) {
		handleThrusterResult(deviceId, cmdCode, checkResult);
	}
	else if (DeviceTypeHelper::isRudder(deviceId) || deviceId == DeviceId::AntennaFold) {
		handleRudderResult(deviceId, cmdCode, checkResult);
	}
	else if (DeviceTypeHelper::isBuoyancy(deviceId)) {
		handleBuoyancyResult(deviceId, cmdCode, checkResult);
	}
	else if (deviceId == DeviceId::DropWeight) {
		handleDropWeightResult(checkResult);
	}
	else if (deviceId == DeviceId::DepthMeter) {
		handleDepthMeterResult(checkResult);
	}
	else if (deviceId == DeviceId::Altimeter) {
		handleAltimeterResult(checkResult);
	}
	else if (deviceId == DeviceId::USBL) {
		handleUsblResult(checkResult);
	}
	else if (deviceId == DeviceId::INS) {
		handleInsResult(checkResult);
	}
	else if (deviceId == DeviceId::DVL) {
		handleDvlResult(checkResult);
	}
	else if (deviceId == DeviceId::CameraLight) {
		handleCameraResult(checkResult);
	}
	else if (DeviceTypeHelper::isBattery(deviceId)) {
		handleBatteryResult(deviceId, cmdCode, checkResult);
	}
	else if (DeviceTypeHelper::isSonar(deviceId)) {
		handleSonarResult(deviceId, cmdCode, checkResult);
	}
	else if (deviceId == DeviceId::AcousticComm) {
		handleAcousticCommResult(checkResult);
	}
	else if (deviceId == DeviceId::BeidouComm) {
		handleBeidouCommResult(checkResult);
	}
	else {
		qWarning() << "未定义器件处理逻辑：" << EnumConverter::deviceIdToString(deviceId);
	}
}

void SelfCheckMainDialog_533::updateWidgetText(QWidget* widget, const QString& text, const QColor& color)
{
	if (!widget) {
		qWarning() << "UI控件为空，无法更新文本";
		return;
	}

	// 设置文本和颜色
	if (auto lineEdit = qobject_cast<QLineEdit*>(widget)) {
		lineEdit->setText(text);
		QPalette palette = lineEdit->palette();
		palette.setColor(QPalette::Text, color); // 设置文本颜色
		lineEdit->setPalette(palette);
	}
	else if (auto label = qobject_cast<QLabel*>(widget)) {
		label->setText(text);
		QPalette palette = label->palette();
		palette.setColor(QPalette::WindowText, color); // 设置标签文本颜色
		label->setPalette(palette);
	}
}

QPixmap SelfCheckMainDialog_533::getStatusIcon(ResultStatus status)
{
	QPixmap icon;
	switch (status) {
	case ResultStatus::Success: icon.load(ICON_SUCCESS); break;
	case ResultStatus::Pending: icon.load(ICON_WAIT); break;
	default: icon.load(ICON_FAIL); break; // 失败/超时/未知统一用失败图标
	}
	return icon;
}

void SelfCheckMainDialog_533::setIcon(QLabel* label, const QPixmap& icon)
{
	if (label && !icon.isNull()) {
		label->setPixmap(icon.scaled(label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
	}
}


void SelfCheckMainDialog_533::processThrusterCommand(const QString& number, CommandCode cmdCode, const SelfCheckResult* checkResult, const QPixmap& icon)
{
	ResultStatus status = checkResult->status();
	qDebug() << "SelfCheckMainDialog_533舵机参数设置结果"
		<< (status == ResultStatus::Success ? "Success" :
			status == ResultStatus::Failed ? "Failed" :
			status == ResultStatus::Pending ? "Pending" :
			status == ResultStatus::Timeout ? "Timeout" : "Unknown");
	QString feedbackDesc = checkResult->feedbackDesc(); // 获取反馈描述

	// 1. 处理图标更新（根据命令码+编号定位图标控件）
	if (cmdCode == CommandCode::Common_PowerOn) {
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_PowerOnIcon").arg(number)), icon);
		updateStatusWidget(this->findChild<QLineEdit*>(QString("le_%1_PowerOn_Feedback").arg(number)),
			status,feedbackDesc);
			
	}
	else if (cmdCode == CommandCode::Common_PowerOff) {
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_PowerOffIcon").arg(number)), icon);
		updateStatusWidget(this->findChild<QLineEdit*>(QString("le_%1_PowerOff_Feedback").arg(number)),
			status, feedbackDesc);
	}
	else if (cmdCode == CommandCode::Thruster_ParamReset) {
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_ParamResetIcon").arg(number)), icon);
		updateStatusWidget(this->findChild<QLineEdit*>(QString("le_%1_ParamReset_Feedback").arg(number)),
			status, feedbackDesc);
	}
	else if (cmdCode == CommandCode::Common_Enable) {
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_EnableWorkIcon").arg(number)), icon);
		updateStatusWidget(this->findChild<QLineEdit*>(QString("le_%1_EnableWork_Feedback").arg(number)),
			status, feedbackDesc);
	}
	else if (cmdCode == CommandCode::Thruster_SetSpeed) {
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_SetSpeedIcon").arg(number)), icon);
		
	}
	else if (cmdCode == CommandCode::Thruster_SetDuty) {
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_SetDutyIcon").arg(number)), icon);
	}
	else if (cmdCode == CommandCode::Thruster_SetAccelTime) {
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_SetAccelTimeIcon").arg(number)), icon);
		updateStatusWidget(this->findChild<QLineEdit*>(QString("le_%1_SetAccelTime_Feedback").arg(number)),
			status, feedbackDesc);
	}
	else if (cmdCode == CommandCode::Thruster_SetRunTime) {
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_SetRunTimeIcon").arg(number)), icon);
	}
	// 2. 处理参数更新（根据命令码+编号定位LineEdit控件）
	if (cmdCode == CommandCode::Thruster_GetSpeed) {
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_ReadSpeedIcon").arg(number)), icon);
		uint16_t speed = checkResult->getParameter("speed").toUInt();
		updateStatusWidget(this->findChild<QLineEdit*>(QString("le_%1_ReadSpeed_Feedback").arg(number)),status,
			QString("%1 RPM").arg(speed));
	}
	else if (cmdCode == CommandCode::Thruster_GetDuty) {
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_ReadDutyIcon").arg(number)), icon);
		uint16_t duty = checkResult->getParameter("duty").toUInt();
		updateStatusWidget(this->findChild<QLineEdit*>(QString("le_%1_ReadDuty_Feedback").arg(number)),status,
			QString("%1（1000=停止）").arg(duty));
	}
	else if (cmdCode == CommandCode::Thruster_GetCurrent) {
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_ReadCurrentIcon").arg(number)), icon);
		double current = checkResult->getParameter("current").toDouble();
		updateStatusWidget(this->findChild<QLineEdit*>(QString("le_%1_ReadCurrent_Feedback").arg(number)),status,
			QString("%1 A").arg(current, 0, 'f', 2));
	}
	else if (cmdCode == CommandCode::Thruster_GetTemperature) {
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_ReadTempIcon").arg(number)), icon);
		uint16_t temp = checkResult->getParameter("temperature").toUInt();
		updateStatusWidget(this->findChild<QLineEdit*>(QString("le_%1_ReadTemp_Feedback").arg(number)),status,
			QString("%1 ℃").arg(temp / 10.0, 0, 'f', 1));
	}
	else if (cmdCode == CommandCode::Thruster_GetBusVoltage) {
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_ReadVoltageIcon").arg(number)), icon);
		double volt = checkResult->getParameter("volt").toDouble();
		updateStatusWidget(this->findChild<QLineEdit*>(QString("le_%1_ReadVoltage_Feedback").arg(number)),status,
			QString("%1 V").arg(volt, 0, 'f', 1));
	}
	else if (cmdCode == CommandCode::Thruster_GetRunTime) {
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_ReadRunTimeIcon").arg(number)), icon);
		uint16_t runtime = checkResult->getParameter("runtime").toUInt();
		updateStatusWidget(this->findChild<QLineEdit*>(QString("le_%1_ReadRunTime_Feedback").arg(number)),status,
			QString("%1 秒").arg(runtime));
	}
	else if (cmdCode == CommandCode::Thruster_GetNoMaintainTime) {
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_ReadNoMaintainTimeIcon").arg(number)), icon);
		uint16_t hours = checkResult->getParameter("noMaintainTime").toUInt();
		updateStatusWidget(this->findChild<QLineEdit*>(QString("le_%1_ReadNoMaintainTime_Feedback").arg(number)),
			status, QString("%1 小时").arg(hours));
	}
	else if (cmdCode == CommandCode::Thruster_GetTotalRunTime) {
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_ReadTotalRunTimeIcon").arg(number)), icon);
		uint32_t totalSec = checkResult->getParameter("totalRunTime").toUInt();
		updateStatusWidget(this->findChild<QLineEdit*>(QString("le_%1_ReadTotalRunTime_Feedback").arg(number)),
			status , QString("%1 秒").arg(totalSec));
	}
	else if (cmdCode == CommandCode::Thruster_GetHall) {
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_ReadHallIcon").arg(number)), icon);
		uint8_t hall = checkResult->getParameter("hall").toUInt();
		QString hallDesc = (hall == 0 || hall == 7) ?
			QString("%1（故障）").arg(hall) : QString("%1（正常）").arg(hall);
		updateStatusWidget(this->findChild<QLineEdit*>(QString("le_%1_ReadHall_Feedback").arg(number)), 
			status,hallDesc);

	}
	else if (cmdCode == CommandCode::Thruster_SetSpeedWithTime) {
	
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_SetSpeedWithTimeIcon").arg(number)), icon);
		updateStatusWidget(this->findChild<QLineEdit*>(QString("le_%1_SetSpeedWithTime_Feedback").
			arg(number)),status, feedbackDesc);
	}
}

void SelfCheckMainDialog_533::processRudderCommand(const QString& number, CommandCode cmdCode, const SelfCheckResult* checkResult, const QPixmap& icon)
{

}

void SelfCheckMainDialog_533::handleThrusterResult(DeviceId deviceId, CommandCode cmdCode, const SelfCheckResult* checkResult)
{
	QString number = getThrusterNumber(deviceId);
	if (number.isEmpty()) {
		qWarning() << "未定义推进器编号：" << static_cast<uint8_t>(deviceId);
		return;
	}

	ResultStatus status = checkResult->status();
	QPixmap icon = getStatusIcon(status);
	processThrusterCommand(number, cmdCode, checkResult, icon);
}

void SelfCheckMainDialog_533::handleRudderResult(DeviceId deviceId, CommandCode cmdCode, const SelfCheckResult* checkResult)
{
	QString number = getRudderNumber(deviceId);
	if (number.isEmpty()) {
		qWarning() << "未定义舵机编号：" << static_cast<uint8_t>(deviceId);
		return;
	}

	ResultStatus status = checkResult->status();
	qDebug() << "SelfCheckMainDialog_533舵机参数设置结果"
		<< (status == ResultStatus::Success ? "Success" :
			status == ResultStatus::Failed ? "Failed" :
			status == ResultStatus::Pending ? "Pending" :
			status == ResultStatus::Timeout ? "Timeout" : "Unknown");
	QPixmap icon = getStatusIcon(status);
	QString feedbackDesc = checkResult->feedbackDesc(); // 获取反馈描述
	// 1. 图标更新（使用通用控件获取函数）
	if (cmdCode == CommandCode::Common_PowerOn)
	{
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_PowerOnIcon").arg(number)), icon);
		updateStatusWidget(
			this->findChild<QLineEdit*>(QString("le_%1_PowerOn_Feedback").arg(number)),status,feedbackDesc);
	}
	else if (cmdCode == CommandCode::Common_PowerOff) {
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_PowerOffIcon").arg(number)), icon);
		updateStatusWidget(
			this->findChild<QLineEdit*>(QString("le_%1_PowerOff_Feedback").arg(number)),status, feedbackDesc);
	}
	// 2. 设置舵角状态更新
	else if (cmdCode == CommandCode::Rudder_SetAngle) {
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_SetAngleIcon").arg(number)), icon);
	}
	// 3. 读取运行时间更新
	else if (cmdCode == CommandCode::Rudder_GetRunTime)
	{
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_ReadRunTimeIcon").arg(number)), icon);
		qlonglong runtimeVal = checkResult->getParameter("runtime").toLongLong();
		updateStatusWidget(
			this->findChild<QLineEdit*>(QString("le_%1_ReadRunTime_Feedback").arg(number)),
			status, QString("%1秒").arg(static_cast<uint32_t>(runtimeVal)));

	}
	// 4. 读取霍尔电压更新
	else if (cmdCode == CommandCode::Rudder_GetVoltage)
	{
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_ReadVoltageIcon").arg(number)), icon);
		double voltage = checkResult->getParameter("voltage").toDouble();
		updateStatusWidget(
			this->findChild<QLineEdit*>(QString("le_%1_ReadVoltage_Feedback").arg(number)), status,
			QString("%1V").arg(voltage, 0, 'f', 1));
	}
	// 5. 读取当前舵角更新
	else if (cmdCode == CommandCode::Rudder_GetAngle)
	{
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_ReadAngleIcon").arg(number)), icon);
		double angle = checkResult->getParameter("angle").toDouble();
		updateStatusWidget(
			this->findChild<QLineEdit*>(QString("le_%1_ReadAngle_Feedback").arg(number)),status,
			QString("%1°").arg(angle, 0, 'f', 1));
	}
	// 6. 读取舵机电流更新
	else if (cmdCode == CommandCode::Rudder_GetCurrent)
	{
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_ReadCurrentIcon").arg(number)), icon);
		qlonglong currRaw = checkResult->getParameter("current").toLongLong();
		double current = static_cast<uint16_t>(currRaw) / 100.0; // 转换为uint16_t再计算
		updateStatusWidget(
			this->findChild<QLineEdit*>(QString("le_%1_ReadCurrent_Feedback").arg(number)),status,
			QString("%1A").arg(current, 0, 'f', 2));
	}
	// 7. 读取舵机温度更新
	else if (cmdCode == CommandCode::Rudder_GetTemp)
	{
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_ReadTempIcon").arg(number)), icon);
		qlonglong tempRaw = checkResult->getParameter("temperature").toLongLong();
		double temperature = static_cast<uint16_t>(tempRaw) / 10.0; // 转换为uint16_t再计算
		updateStatusWidget(
			this->findChild<QLineEdit*>(QString("le_%1_ReadTemp_Feedback").arg(number)),status,
			QString("%1℃").arg(temperature, 0, 'f', 1));
	}
}

void SelfCheckMainDialog_533::handleBuoyancyResult(DeviceId deviceId, CommandCode cmdCode, const SelfCheckResult* checkResult)
{
	ResultStatus status = checkResult->status();
	QPixmap icon = getStatusIcon(status);
	QString feedbackDesc = checkResult->feedbackDesc(); // 获取反馈描述
	// 1. 图标更新（使用通用控件获取函数）
	if (cmdCode == CommandCode::Common_PowerOn)
	{
		setIcon(ui.lbl_06_PowerOnIcon, icon);
		updateStatusWidget(ui.le_06_PowerOn_Feedback, status, feedbackDesc);
		
	}
	else if (cmdCode == CommandCode::Common_PowerOff)
	{
		setIcon(ui.lbl_06_PowerOffIcon, icon);
		updateStatusWidget(ui.le_06_PowerOff_Feedback, status, feedbackDesc);
	}
	else if (cmdCode == CommandCode::Buoyancy_SetValue)
	{
		setIcon(ui.lbl_06_SetBuoyancyValueIcon, icon);
		updateStatusWidget(ui.le_06_SetBuoyancyValue_Feedback, status); // 设置反馈文本
	}
	else if (cmdCode == CommandCode::Buoyancy_ReadValue)
	{
		setIcon(ui.lbl_06_ReadBuoyancyValueIcon, icon);
		uint16_t buoyancyRaw = checkResult->getParameter("buoyancyValue").toUInt();
		double buoyancy = buoyancyRaw / 100.0;
		QString buoyancyStr = QString("%1 L").arg(buoyancy, 0, 'f', 2);
		updateStatusWidget(ui.le_06_ReadBuoyancyValue_Feedback, status,buoyancyStr);
	}
	else if (cmdCode == CommandCode::Buoyancy_Stop)
	{
		setIcon(ui.lbl_06_StopIcon, icon);
		updateStatusWidget(ui.le_06_Stop_Feedback, status, feedbackDesc);
	}
}

void SelfCheckMainDialog_533::handleBatteryResult(DeviceId deviceId, CommandCode cmdCode, const SelfCheckResult* checkResult)
{
	QString number = getBatteryNumber(deviceId);
	if (number.isEmpty()) {
		qWarning() << "未定义电池编号：" << static_cast<uint8_t>(deviceId);
		return;
	}

	ResultStatus status = checkResult->status();
	QPixmap icon = getStatusIcon(status);
	QString feedbackDesc = checkResult->feedbackDesc(); // 获取反馈描述
	// 1. 图标更新
	if (cmdCode == CommandCode::Battery_GetSoc) {
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_ReadSOCIcon").arg(number)), icon);
		if (deviceId == DeviceId::MeterBattery) {
			// 仪表电池：原始值是uint16，比例尺0.1
			uint16_t socRaw = checkResult->getParameter("soc").toUInt();
			double soc = socRaw * 0.1;
			updateStatusWidget(this->findChild<QLineEdit*>(QString("le_%1_ReadSOC_Feedback").arg(number)),
				status,QString("%1%").arg(soc, 0, 'f', 1));
		}
		else {
			// 动力电池：原始值是uint8，比例尺1
			uint8_t soc = checkResult->getParameter("soc").toUInt();
			updateStatusWidget(this->findChild<QLineEdit*>(QString("le_%1_ReadSOC_Feedback").arg(number)),
				status,QString("%1%").arg(soc));
		}
		
	}
	else if (cmdCode == CommandCode::Battery_GetTotalVoltage) {
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_ReadVoltageIcon").arg(number)), icon);
		uint16_t voltRaw = checkResult->getParameter("volt").toUInt();
		double volt = voltRaw * 0.1; // 协议比例尺0.1
		updateStatusWidget(this->findChild<QLineEdit*>(QString("le_%1_ReadVoltage_Feedback").arg(number)),
			status, QString("%1V").arg(volt, 0, 'f', 1));
	}
	else if (cmdCode == CommandCode::Battery_GetCurrent) {
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_ReadCurrentIcon").arg(number)), icon);
		int16_t currRaw = checkResult->getParameter("current").toInt();
		double curr = currRaw * 0.1; // 协议比例尺0.1
		updateStatusWidget(this->findChild<QLineEdit*>(QString("le_%1_ReadCurrent_Feedback").arg(number)),
			status,QString("%1A").arg(curr, 0, 'f', 1));
	}
	else if (cmdCode == CommandCode::Battery_GetAvgTemp) { // 替换原MaxTemp为平均温度
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_ReadAvgTempIcon").arg(number)), icon);
		int16_t tempRaw = checkResult->getParameter("avgTemp").toInt();
		updateStatusWidget(this->findChild<QLineEdit*>(QString("le_%1_ReadAvgTemp_Feedback").arg(number)),
			status,QString("%1℃").arg(tempRaw)); // 比例尺1，无需换算
	}
	else if (cmdCode == CommandCode::Battery_HVOn) {
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_HVOnIcon").arg(number)), icon);
		updateStatusWidget(this->findChild<QLineEdit*>(QString("le_%1_HVOn_Feedback").arg(number)),
			status, feedbackDesc);
	}
	else if (cmdCode == CommandCode::Battery_HVOff) {
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_HVOffIcon").arg(number)), icon);
		updateStatusWidget(this->findChild<QLineEdit*>(QString("le_%1_HVOff_Feedback").arg(number)),
			status, feedbackDesc);
	}
	else if (cmdCode == CommandCode::Battery_Lock) {
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_LockIcon").arg(number)), icon);
		updateStatusWidget(this->findChild<QLineEdit*>(QString("le_%1_Lock_Feedback").arg(number)),
			status, feedbackDesc);
	}
	else if (cmdCode == CommandCode::Battery_Unlock) {
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_UnlockIcon").arg(number)), icon);
		updateStatusWidget(this->findChild<QLineEdit*>(QString("le_%1_Unlock_Feedback").arg(number)),
			status, feedbackDesc);
	}
	
}

void SelfCheckMainDialog_533::handleSonarResult(DeviceId deviceId, CommandCode cmdCode, const SelfCheckResult* checkResult)
{
	QString number = getSonarNumber(deviceId);
	if (number.isEmpty()) {
		qWarning() << "未定义声呐编号：" << static_cast<uint8_t>(deviceId);
		return;
	}

	ResultStatus status = checkResult->status();
	QPixmap icon = getStatusIcon(status);
	QString feedbackDesc = checkResult->feedbackDesc(); // 获取反馈描述
	// 1. 图标更新（协议中声纳功能）
	if (cmdCode == CommandCode::Common_PowerOn) {
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_PowerOnIcon").arg(number)), icon);
		updateStatusWidget(
			this->findChild<QLineEdit*>(QString("le_%1_PowerOn_Feedback").arg(number)), status, feedbackDesc);
	}
	else if (cmdCode == CommandCode::Common_PowerOff) {
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_PowerOffIcon").arg(number)), icon);
		updateStatusWidget(
			this->findChild<QLineEdit*>(QString("le_%1_PowerOff_Feedback").arg(number)), status, feedbackDesc);
		
	}
	else if (cmdCode == CommandCode::Common_Enable) { // 开始工作
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_EnableWorkIcon").arg(number)), icon);
		updateStatusWidget(
			this->findChild<QLineEdit*>(QString("le_%1_EnableWork_Feedback").arg(number)), status, feedbackDesc);
	}
	else if (cmdCode == CommandCode::Common_Disable) { // 停止工作
		setIcon(this->findChild<QLabel*>(QString("lbl_%1_DisableWorkIcon").arg(number)), icon);
		updateStatusWidget(
			this->findChild<QLineEdit*>(QString("le_%1_DisableWork_Feedback").arg(number)),status,feedbackDesc);
	}

}

void SelfCheckMainDialog_533::handleAcousticCommResult(const SelfCheckResult* checkResult)
{

	if (!checkResult) return; // 空指针防护

	CommandCode cmdCode = checkResult->commandCode();
	ResultStatus status = checkResult->status();
	QPixmap icon = getStatusIcon(status);
	QString feedbackDesc = checkResult->feedbackDesc(); // 之前构建的反馈描述
	if (cmdCode == CommandCode::Common_PowerOn)
	{
		setIcon(ui.lbl_15_PowerOnIcon, icon);
		updateStatusWidget(ui.le_15_PowerOn_Feedback, status, feedbackDesc);

	}
	else if (cmdCode == CommandCode::Common_PowerOff)
	{
		setIcon(ui.lbl_15_PowerOffIcon, icon);
		updateStatusWidget(ui.le_15_PowerOff_Feedback, status, feedbackDesc);
	}
	else if (CommandCode::AcousticComm_SelfCheck == cmdCode)
	{
		setIcon(ui.lbl_15_SelfCheckIcon, icon);
		updateStatusWidget(ui.le_15_SelfCheck_Feedback, status, feedbackDesc);
	}
	else if (CommandCode::AcousticComm_Test == cmdCode  )
	{
		QLabel* acousticIconLbl = ui.lbl_15_CommTestIcon;  // 外部水声测试图标
		
		// 反馈描述文本框（内外）
		QLineEdit* acousticFeedbackLe = ui.le_15_CommTest_Feedback;
		QLineEdit* sendResultFeedbackLe = ui.le_15_SendResult_Feedback;
		QLineEdit* receiveResultFeedbackLe = ui.le_15_ReceiveResult_Feedback;
		// 发送/接收数据显示框
		QLineEdit* sendDataLe = ui.le_15_SendData;    // 水声发送数据框
		QLineEdit* recvDataLe = ui.le_15_ReceiveData; // 水声接收数据框


		 // 1. 提取指令类型参数
		QString cmdTypeDesc = checkResult->getParameter("cmd_type_desc").toString();
		QString msgType = checkResult->getParameter("msg_type").toString();


		int cmdType = checkResult->getParameter("cmd_type").toInt();
	
		switch (static_cast<WaterAcousticUplinkCmd>(cmdType)) 
		{
		case WaterAcousticUplinkCmd::SendFinish: 
		{
			sendResultFeedbackLe->setText(feedbackDesc);
			break;
		}
			
		case WaterAcousticUplinkCmd::RecvNotify:
		{
			receiveResultFeedbackLe->setText(feedbackDesc);
			break;
		}
		case WaterAcousticUplinkCmd::DataReceived:
		{
			// 3. 设置状态图标（成功√/失败×/超时⚠️）
			setIcon(acousticIconLbl, icon);

			// 4. 提取发送/接收的十六进制数据（与北斗短报文参数名一致）
			QString sentData = checkResult->getParameter("sent_data_hex").toString();
			QString recvData = checkResult->getParameter("recv_data_hex").toString();

			updateStatusWidget(acousticFeedbackLe, status, feedbackDesc);
			updateStatusWidget(sendDataLe, status, sentData);   // 发送数据白色显示
			updateStatusWidget(recvDataLe, status, recvData);   // 接收数据白色显示
			break;
		}
		default: 
			break;
		}
		

	}
}

template <typename T>
T* getInternalExternalWidget(T* externalWidget, T* internalWidget, CommandType type)
{
	return (type == CommandType::External) ? externalWidget : internalWidget;
}

void SelfCheckMainDialog_533::handleBeidouCommResult(const SelfCheckResult* checkResult)
{
	if (!checkResult) return; // 空指针防护

	// 提取核心参数，统一存储
	CommandCode cmdCode = checkResult->commandCode();
	ResultStatus status = checkResult->status();
	QPixmap icon = getStatusIcon(status);
	QString feedbackDesc = checkResult->feedbackDesc();
	CommandType type = checkResult->commandType();

	// ====================== 1. 信号质量查询（优化参数有效性判断） ======================
	if (CommandCode::BeidouComm_GetSignalQuality == cmdCode)
	{
		// 利用工具函数简化内外控件区分（消除重复代码）
		QLabel* signalIconLbl = getInternalExternalWidget(ui.lbl_09_SignalQualityIcon,
			ui.lbl_09_SignalQualityIcon_In, type);
		QLineEdit* signalFeedbackLe = getInternalExternalWidget(ui.le_09_SignalQuality_Feedback,
			ui.le_09_SignalQuality_Feedback_In, type);

		if (!signalIconLbl || !signalFeedbackLe) return; // 空指针防护
		setIcon(signalIconLbl, icon);

		// 增强参数有效性判断，避免无效转换
		QString signalStrengthDesc = "未知等级";
		if (checkResult->getParameter("signalQuality").isValid())
		{
			int signalLevel = checkResult->getParameter("signalQuality").toInt();
			// 信号强度描述映射
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
		// 统一使用updateStatusWidget，保持状态处理一致
		updateStatusWidget(signalFeedbackLe, status, signalStrengthDesc);
	}

	// ====================== 2. 本机卡号查询（优化参数有效性判断） ======================
	else if (cmdCode == CommandCode::BeidouComm_GetIdentity)
	{
		QLabel* identityIconLbl = getInternalExternalWidget(ui.lbl_09_IdentityIcon,
			ui.lbl_09_IdentityIcon_In, type);
		QLineEdit* identityFeedbackLe = getInternalExternalWidget(ui.le_09_Identity_Feedback,
			ui.le_09_Identity_Feedback_In, type);
		if (!identityIconLbl || !identityFeedbackLe) return;

		setIcon(identityIconLbl, icon);
		QString localCardStr = "未知卡号";
		// 增强参数有效性判断
		if (checkResult->getParameter("localCardNo").isValid())
		{
			uint32_t localCardNo = checkResult->getParameter("localCardNo").toUInt();
			localCardStr = QString("%1").arg(localCardNo, 7, 10, QChar('0')); // 7位补零
		}
		updateStatusWidget(identityFeedbackLe, status, localCardStr);
	}

	// ====================== 3. 目标卡号查询（修正命名笔误+优化逻辑） ======================
	else if (cmdCode == CommandCode::BeidouComm_GetTarget)
	{
		// 修正命名笔误：GeTarget -> GetTarget（与UI控件名一致）
		QLabel* targetIconLbl = getInternalExternalWidget(ui.lbl_09_GetTargetIcon,
			ui.lbl_09_GetTargetIcon_In, type);
		QLineEdit* targetFeedbackLe = getInternalExternalWidget(ui.le_09_GetTarget_Feedback,
			ui.le_09_GetTarget_Feedback_In, type);
		if (!targetIconLbl || !targetFeedbackLe) return;

		setIcon(targetIconLbl, icon);
		if (checkResult->getParameter("targetCardNo").isValid())
		{
			uint32_t targetCardNo = checkResult->getParameter("targetCardNo").toUInt();
			QString targetCardStr = QString("%1").arg(targetCardNo, 7, 10, QChar('0'));
			updateStatusWidget(targetFeedbackLe, status, targetCardStr);
		}
		else
		{
			// 未设置目标卡号：统一用updateStatusWidget处理样式
			updateStatusWidget(targetFeedbackLe, status, feedbackDesc);
		}
	}

	// ====================== 4. 白名单查询（优化逻辑+样式+容错） ======================
	else if (cmdCode == CommandCode::BeidouComm_GetWhitelist)
	{
		QLabel* whitelistIconLbl = getInternalExternalWidget(ui.lbl_09_GetWhitelistIcon,
			ui.lbl_09_GetWhitelistIcon_In, type);
		QComboBox* cbxWhitelist = getInternalExternalWidget(ui.cb_09_GetWhitelist_Feedback,
			ui.cb_09_GetWhitelist_Feedback_In, type);
		if (!whitelistIconLbl || !cbxWhitelist) return;

		setIcon(whitelistIconLbl, icon);
		// 彻底重置ComboBox：清空数据+样式+编辑状态
		cbxWhitelist->clear();
		cbxWhitelist->setStyleSheet("");
		cbxWhitelist->setEditable(false);

		if (status == ResultStatus::Success)
		{
			QStringList whitelist = checkResult->getParameter("whitelist").toStringList();
			QStringList valid7DigitWhitelist = filter7DigitCard(whitelist);

			// 优化1：先添加有效卡号，再判断是否插入提示项（避免索引错位）
			if (!valid7DigitWhitelist.isEmpty())
			{
				cbxWhitelist->addItems(valid7DigitWhitelist);
				cbxWhitelist->setCurrentIndex(0);
			}
			else
			{
				cbxWhitelist->addItem("无符合7位规则的有效卡号");
				// 无效状态设置灰色文字
				cbxWhitelist->setItemData(0, QBrush(Qt::gray), Qt::TextColorRole);
			}

			// 优化2：仅当存在非法卡号时插入提示项，且判断有效卡号是否为空
			if (!whitelist.isEmpty() && whitelist.size() != valid7DigitWhitelist.size())
			{
				int insertIndex = valid7DigitWhitelist.isEmpty() ? 0 : 0; // 插入到首位，避免错位
				cbxWhitelist->insertItem(insertIndex, "⚠️ 部分卡号不符合7位规则已过滤");
				cbxWhitelist->setItemData(insertIndex, QBrush(QColor(255, 165, 0)), Qt::TextColorRole);
				cbxWhitelist->setCurrentIndex(insertIndex); // 默认显示提示项
			}
		}
		else
		{
			// 优化3：失败场景设置红色文字样式，视觉更直观
			QString errorText = QString("白名单查询失败：%1").arg(feedbackDesc);
			cbxWhitelist->addItem(errorText);
			cbxWhitelist->setItemData(0, QBrush(Qt::red), Qt::TextColorRole);
			cbxWhitelist->setCurrentIndex(0);
		}
	}

	// ====================== 5. 定位信息处理（优化无效定位提示） ======================
	else if (cmdCode == CommandCode::BeidouComm_GetPosition)
	{
		QLabel* positionIconLbl = getInternalExternalWidget(ui.lbl_09_GetPositionIcon,
			ui.lbl_09_GetPositionIcon_In, type);
		QLineEdit* positionFeedbackLe = getInternalExternalWidget(ui.le_09_GetPosition_Feedback,
			ui.le_09_GetPosition_Feedback_In, type);
		if (!positionIconLbl || !positionFeedbackLe) return;

		setIcon(positionIconLbl, icon);
		if (status == ResultStatus::Success)
		{
			bool isValidFix = checkResult->getParameter("is_valid_fix").toBool();
			double latDegree = checkResult->getParameter("latitude_degree").toDouble();
			double lonDegree = checkResult->getParameter("longitude_degree").toDouble();

			// 优化：无效定位时提示文字，而非显示0.000000°
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

	// ====================== 6. 系统信息处理（增强参数有效性判断） ======================
	else if (cmdCode == CommandCode::BeidouComm_GetSysInfo)
	{
		QLabel* sysInfoIconLbl = getInternalExternalWidget(ui.lbl_09_GetSysInfoIcon,
			ui.lbl_09_GetSysInfoIcon_In, type);
		QLineEdit* sysInfoFeedbackLe = getInternalExternalWidget(ui.le_09_GetSysInfo_Feedback,
			ui.le_09_GetSysInfo_Feedback_In, type);
		if (!sysInfoIconLbl || !sysInfoFeedbackLe) return;

		setIcon(sysInfoIconLbl, icon);
		QString sysInfo = "未知系统信息";
		if (checkResult->getParameter("systemInfo").isValid())
		{
			sysInfo = checkResult->getParameter("systemInfo").toString();
		}
		updateStatusWidget(sysInfoFeedbackLe, status, sysInfo);
	}

	// ====================== 7. 有参设置命令（修正命名笔误+统一处理） ======================
	else if (cmdCode == CommandCode::BeidouComm_SetTarget ||
		cmdCode == CommandCode::BeidouComm_AddWhitelist ||
		cmdCode == CommandCode::BeidouComm_DelWhitelist)
	{
		QLabel* iconLbl = nullptr;
		QLineEdit* feedbackLe = nullptr;

		if (cmdCode == CommandCode::BeidouComm_SetTarget)
		{
			iconLbl = getInternalExternalWidget(ui.lbl_09_SetTargetIcon,
				ui.lbl_09_SetTargetIcon_In, type);
			feedbackLe = getInternalExternalWidget(ui.le_09_SetTarget_Input,
				ui.le_09_SetTarget_Input_In, type);
		}
		else if (cmdCode == CommandCode::BeidouComm_AddWhitelist)
		{
			iconLbl = getInternalExternalWidget(ui.lbl_09_AddWhitelistIcon,
				ui.lbl_09_AddWhitelistIcon_In, type);
			feedbackLe = getInternalExternalWidget(ui.le_09_AddWhitelist_Input,
				ui.le_09_AddWhitelist_Input_In, type);
		}
		else if (cmdCode == CommandCode::BeidouComm_DelWhitelist)
		{
			// 修正命名笔误：DelWhitelis -> DelWhitelist（与UI控件名一致）
			iconLbl = getInternalExternalWidget(ui.lbl_09_DelWhitelistIcon,
				ui.lbl_09_DelWhitelistIcon_In, type);
			feedbackLe = getInternalExternalWidget(ui.le_09_DelWhitelist_Input,
				ui.le_09_DelWhitelist_Input_In, type);
		}

		if (iconLbl && feedbackLe)
		{
			setIcon(iconLbl, icon);
			QString tip = (cmdCode == CommandCode::BeidouComm_AddWhitelist || cmdCode == CommandCode::BeidouComm_DelWhitelist)
				? "（仅支持7位数字卡号）" : "";
			// 统一使用updateStatusWidget，避免样式不一致
			updateStatusWidget(feedbackLe, status, feedbackDesc + tip);
		}
	}

	// ====================== 8. 控制命令（增强容错+优化逻辑） ======================
	else if (cmdCode == CommandCode::BeidouComm_FactoryReset ||
		cmdCode == CommandCode::BeidouComm_Reboot ||
		cmdCode == CommandCode::BeidouComm_Test)
	{
		QLabel* iconLbl = nullptr;
		QLineEdit* feedbackLe = nullptr;
		// 增强空指针防护：先判断控件是否存在
		QLineEdit* sendDataLe = ui.le_09_SendData;
		QLineEdit* recvDataLe = ui.le_09_ReceiveData;
		QComboBox* cbxWhitelist = getInternalExternalWidget(ui.cb_09_GetWhitelist_Feedback,
			ui.cb_09_GetWhitelist_Feedback_In, type);

		if (cmdCode == CommandCode::BeidouComm_FactoryReset)
		{
			iconLbl = getInternalExternalWidget(ui.lbl_09_FactoryResetIcon,
				ui.lbl_09_FactoryResetIcon_In, type);
			feedbackLe = getInternalExternalWidget(ui.le_09_FactoryReset_Feedback,
				ui.le_09_FactoryReset_Feedback_In, type);
		}
		else if (cmdCode == CommandCode::BeidouComm_Reboot)
		{
			iconLbl = getInternalExternalWidget(ui.lbl_09_RebootIcon,
				ui.lbl_09_RebootIcon_In, type);
			feedbackLe = getInternalExternalWidget(ui.le_09_Reboot_Feedback,
				ui.le_09_Reboot_Feedback_In, type);
		}
		else if (cmdCode == CommandCode::BeidouComm_Test)
		{
			iconLbl = ui.lbl_09_CommTestIcon;
			feedbackLe = ui.le_09_CommTest_Feedback;
		}

		// 提取发送/接收数据，增强有效性判断
		QString sendData = checkResult->getParameter("sent_data_hex").isValid()
			? checkResult->getParameter("sent_data_hex").toString() : "无发送数据";
		QString recvData = checkResult->getParameter("recv_data_hex").isValid()
			? checkResult->getParameter("recv_data_hex").toString() : "无接收数据";

		if (iconLbl && feedbackLe)
		{
			setIcon(iconLbl, icon);
			// 统一状态处理逻辑，消除不一致
			if (status == ResultStatus::Success)
			{
				QString enhancedDesc = feedbackDesc;
				if (cmdCode == CommandCode::BeidouComm_Reboot)
				{
					enhancedDesc += "（模块将重启，请勿断电）";
				}
				else if (cmdCode == CommandCode::BeidouComm_FactoryReset)
				{
					enhancedDesc += "（所有7位白名单已清空）";
					// 清空白名单ComboBox并设置提示
					if (cbxWhitelist)
					{
						cbxWhitelist->clear();
						cbxWhitelist->addItem("白名单已清空");
						cbxWhitelist->setItemData(0, QBrush(Qt::gray), Qt::TextColorRole);
					}
				}
				updateStatusWidget(feedbackLe, status, enhancedDesc);

				// 通信测试：更新发送/接收数据，增加空指针判断
				if (cmdCode == CommandCode::BeidouComm_Test)
				{
					if (sendDataLe) updateStatusWidget(sendDataLe, status, sendData);
					if (recvDataLe) updateStatusWidget(recvDataLe, status, recvData);
				}
			}
			else if (status == ResultStatus::Timeout)
			{
				updateStatusWidget(feedbackLe,status, feedbackDesc);
				if (sendDataLe) updateStatusWidget(sendDataLe, status, sendData);
				if (recvDataLe) updateStatusWidget(recvDataLe, status, recvData);
			}
			else
			{
				updateStatusWidget(feedbackLe, status, feedbackDesc);
			}
		}
	}
}

QString SelfCheckMainDialog_533::formatTo7DigitCard(uint32_t cardNum)
{
	QString cardStr = QString::number(cardNum);
	if (cardStr.length() < 7) {
		cardStr = cardStr.rightJustified(7, '0');
	}
	else if (cardStr.length() > 7) {
		cardStr = cardStr.right(7);
	}
	return cardStr;
}

QStringList SelfCheckMainDialog_533::filter7DigitCard(const QStringList& cardList)
{
	QStringList validList;
	// 正则表达式：严格匹配7位数字（^表示开头，$表示结尾，[0-9]{7}表示7个数字）
	QRegExp sevenDigitReg("^[0-9]{7}$");
	sevenDigitReg.setPatternSyntax(QRegExp::RegExp); // 显式指定正则语法（兼容旧Qt版本）

	for (const QString& card : cardList) {
		// 校验逻辑：7位纯数字 + 非全0（保留原需求）
		if (card.contains(sevenDigitReg) && card != "0000000") {
			validList.append(card);
		}
	}
	return validList;
}

void SelfCheckMainDialog_533::handleDropWeightResult(const SelfCheckResult* checkResult)
{
	CommandCode cmdCode = checkResult->commandCode();
	ResultStatus status = checkResult->status();
	QPixmap icon = getStatusIcon(status);

	if (cmdCode == CommandCode::Common_PowerOn)
	{
		setIcon(ui.lbl_07_PowerOnIcon, icon);
		updateStatusWidget(ui.le_07_PowerOn_Feedback, status);
	}
	else if (CommandCode::Common_PowerOff == cmdCode)
	{
		setIcon(ui.lbl_07_PowerOffIcon, icon);
		updateStatusWidget(ui.le_07_PowerOff_Feedback, status);
	}
	else if (cmdCode == CommandCode::DropWeight_Release) 
	{
		setIcon(ui.lbl_07_ReleaseIcon, icon);
		updateStatusWidget(ui.le_07_Release_Feedback, status);
	}
	else if (cmdCode == CommandCode::DropWeight_Reset)
	{
		setIcon(ui.lbl_07_ResetIcon, icon);
		updateStatusWidget(ui.le_07_Reset_Feedback, status);
	}
	else if (cmdCode == CommandCode::Common_GetFault)
	{
		setIcon(ui.lbl_07_StatusCheckIcon, icon);
		updateStatusWidget(ui.le_07_StatusCheck_Feedback, status);
	}
	
}

void SelfCheckMainDialog_533::handleDepthMeterResult(const SelfCheckResult* checkResult)
{
	CommandCode cmdCode = checkResult->commandCode();
	ResultStatus status = checkResult->status();
	QPixmap icon = getStatusIcon(status);
	QString feedbackDesc = checkResult->feedbackDesc();
	if (cmdCode == CommandCode::Common_PowerOn) {
		setIcon(ui.lbl_0A_PowerOnIcon, icon);
		updateStatusWidget(ui.le_0A_PowerOn_Feedback, status, feedbackDesc);
	}
	else if (cmdCode == CommandCode::Common_PowerOff) {
		setIcon(ui.lbl_0A_PowerOffIcon, icon);
		updateStatusWidget(ui.le_0A_PowerOff_Feedback, status, feedbackDesc);
	}
	else if (cmdCode == CommandCode::Depthgauge_GetDepth) {
		setIcon(ui.lbl_0A_ReadDepthIcon, icon);
		double depth = checkResult->getParameter("depth").toDouble();
		updateStatusWidget(ui.le_0A_ReadDepth_Feedback,status, QString("%1 米").arg(depth, 0, 'f', 4));
	}
	else if (cmdCode == CommandCode::Depthgauge_Calibration) {
		setIcon(ui.lbl_0A_ReferenceIcon, icon);
		updateStatusWidget(ui.le_0A_Reference_Feedback, status, feedbackDesc);
	
	}
}

void SelfCheckMainDialog_533::handleAltimeterResult(const SelfCheckResult* checkResult)
{
	CommandCode cmdCode = checkResult->commandCode();
	ResultStatus status = checkResult->status();
	QPixmap icon = getStatusIcon(status);
	QString feedbackDesc = checkResult->feedbackDesc();
	if (cmdCode == CommandCode::Common_PowerOn) {
		setIcon(ui.lbl_0B_PowerOnIcon, icon);
		updateStatusWidget(ui.le_0B_PowerOn_Feedback, status, feedbackDesc);
	}
	else if (cmdCode == CommandCode::Common_PowerOff) {
		setIcon(ui.lbl_0B_PowerOffIcon, icon);
		updateStatusWidget(ui.le_0B_PowerOff_Feedback, status, feedbackDesc);
	}
	else if (cmdCode == CommandCode::Altimeter_GetHeight) {
		setIcon(ui.lbl_0B_ReadHeightIcon, icon);
		double height = checkResult->getParameter("height").toDouble();
		updateStatusWidget(ui.le_0B_ReadHeight_Feedback, status,QString("%1 米").arg(height, 0, 'f', 4));
	}
}

void SelfCheckMainDialog_533::handleUsblResult(const SelfCheckResult* checkResult)
{
	CommandCode cmdCode = checkResult->commandCode();
	ResultStatus status = checkResult->status();
	QPixmap icon = getStatusIcon(status);
	QString feedbackDesc = checkResult->feedbackDesc();
	if (cmdCode == CommandCode::Common_PowerOn) {
		setIcon(ui.lbl_0C_PowerOnIcon, icon);
		updateStatusWidget(ui.le_0C_PowerOn_Feedback, status, feedbackDesc);
	}
	else if (cmdCode == CommandCode::Common_PowerOff) {
		setIcon(ui.lbl_0C_PowerOffIcon, icon);
		updateStatusWidget(ui.le_0C_PowerOff_Feedback, status, feedbackDesc);
	}
	
}

void SelfCheckMainDialog_533::handleInsResult(const SelfCheckResult* checkResult)
{
	CommandCode cmdCode = checkResult->commandCode();
	ResultStatus status = checkResult->status();
	QPixmap icon = getStatusIcon(status);
	QString feedbackDesc = checkResult->feedbackDesc();
	if (cmdCode == CommandCode::Common_PowerOn) {
		setIcon(ui.lbl_0D_PowerOnIcon, icon);
		updateStatusWidget(ui.le_0D_PowerOn_Feedback, status ,feedbackDesc);

	}
	else if (cmdCode == CommandCode::Common_PowerOff) {
		setIcon(ui.lbl_0D_PowerOffIcon, icon);
		updateStatusWidget(ui.le_0D_PowerOff_Feedback, status, feedbackDesc);
	}
	else if (cmdCode == CommandCode::INS_SetGnssBind)
	{
		setIcon(ui.lbl_0D_SetGnssBindIcon, icon);
		updateStatusWidget(ui.le_0D_SetGnssBind_Feedback, status, feedbackDesc);

	}
	else if (cmdCode == CommandCode::INS_CalibrationWithDVL)
	{
		setIcon(ui.lbl_0D_CalibWithDvlIcon, icon);
		updateStatusWidget(ui.le_0D_CalibWithDvl_Feedback, status, feedbackDesc);

	}
	else if (cmdCode == CommandCode::INS_GetUtcDate) {
		setIcon(ui.lbl_0D_GetUtcDateIcon,icon);
		updateStatusWidget(ui.le_0D_ReadUtcDate_Feedback, status, checkResult->getParameter("date").toString());
	}
	else if (cmdCode == CommandCode::INS_GetUtcTime) {
		setIcon(ui.lbl_0D_GetUtcTimeIcon, icon);
		updateStatusWidget(ui.le_0D_ReadUtcTime_Feedback, status, checkResult->getParameter("time").toString());
	}
	else if (cmdCode == CommandCode::INS_GetAttitude) {
		setIcon(ui.lbl_0D_GetAttitudeIcon, icon);
		QString attitude = QString("航向：%1° 俯仰：%2° 横滚：%3°")
			.arg(checkResult->getParameter("yaw").toDouble(), 0, 'f', 2)
			.arg(checkResult->getParameter("pitch").toDouble(), 0, 'f', 2)
			.arg(checkResult->getParameter("roll").toDouble(), 0, 'f', 2);
		double yaw = checkResult->getParameter("yaw").toDouble();
		double pitch = checkResult->getParameter("pitch").toDouble();
		double roll = checkResult->getParameter("roll").toDouble();
		updateStatusWidget(ui.le_0D_ReadYaw_Feedback, status, QString("%1°").arg(yaw, 0, 'f', 2));
		updateStatusWidget(ui.le_0D_ReadPitch_Feedback, status, QString("%1°").arg(pitch, 0, 'f', 2));
		updateStatusWidget(ui.le_0D_ReadRoll_Feedback, status, QString("%1°").arg(roll, 0, 'f', 2));
	}
	else if (cmdCode == CommandCode::INS_GetBodySpeed) {
		setIcon(ui.lbl_0D_GetBodySpeedIcon, icon);
		QString speed = QString("右：%1 前：%2 上：%3")
			.arg(checkResult->getParameter("vx").toDouble(), 0, 'f', 2)
			.arg(checkResult->getParameter("vy").toDouble(), 0, 'f', 2)
			.arg(checkResult->getParameter("vz").toDouble(), 0, 'f', 2);
		double vx = checkResult->getParameter("vx").toDouble();
		double vy = checkResult->getParameter("vy").toDouble();
		double vz = checkResult->getParameter("vz").toDouble();
		updateStatusWidget(ui.le_0D_ReadBodyX_Feedback, status, QString("%1m/s").arg(vx, 0, 'f', 2));
		updateStatusWidget(ui.le_0D_ReadBodyY_Feedback, status, QString("%1m/s").arg(vy, 0, 'f', 2));
		updateStatusWidget(ui.le_0D_ReadBodyZ_Feedback, status, QString("%1m/s").arg(vz, 0, 'f', 2));

	}
	else if (cmdCode == CommandCode::INS_GetGeoSpeed) {
		setIcon(ui.lbl_0D_GetGeoSpeedIcon, icon);
		QString speed = QString("东：%1 北：%2 天：%3")
			.arg(checkResult->getParameter("ex").toDouble(), 0, 'f', 2)
			.arg(checkResult->getParameter("ey").toDouble(), 0, 'f', 2)
			.arg(checkResult->getParameter("ez").toDouble(), 0, 'f', 2);
		double ex = checkResult->getParameter("ex").toDouble();
		double ey = checkResult->getParameter("ey").toDouble();
		double ez = checkResult->getParameter("ez").toDouble();
		updateStatusWidget(ui.le_0D_ReadGeoX_Feedback, status,QString("%1m/s").arg(ex, 0, 'f', 2));
		updateStatusWidget(ui.le_0D_ReadGeoY_Feedback, status,QString("%1m/s").arg(ey, 0, 'f', 2));
		updateStatusWidget(ui.le_0D_ReadGeoZ_Feedback, status,QString("%1m/s").arg(ez, 0, 'f', 2));
	}
	else if (cmdCode == CommandCode::INS_GetPosition) {
		setIcon(ui.lbl_0D_GetPositionIcon, icon);
		QString pos = QString("纬度：%1 经度：%2")
			.arg(checkResult->getParameter("lat").toString())
			.arg(checkResult->getParameter("lon").toString());
		QString lat = checkResult->getParameter("lat").toString() + "°";
		QString lon = checkResult->getParameter("lon").toString() + "°";
		updateStatusWidget(ui.le_0D_ReadLatitude_Feedback, status, lat);
		updateStatusWidget(ui.le_0D_ReadLongitude_Feedback, status, lon);
	}
	// ------------------------------ 陀螺仪角速度UI处理 ------------------------------
	else if (cmdCode == CommandCode::INS_GetGyroscopeSpeed) {
		setIcon(ui.lbl_0D_GetGyroIcon, icon);  // 需确保UI有该控件（可根据实际命名调整）
		// 解析三轴角速度参数
		double yawGyro = checkResult->getParameter("yaw_gyro").toDouble();
		double pitchGyro = checkResult->getParameter("pitch_gyro").toDouble();
		double rollGyro = checkResult->getParameter("roll_gyro").toDouble();
		// 更新UI控件（需确保控件命名与实际UI一致）
		updateStatusWidget(ui.le_0D_ReadGyroYaw_Feedback, status, QString("%1°/s").arg(yawGyro, 0, 'f', 2));
		updateStatusWidget(ui.le_0D_ReadGyroPitch_Feedback, status, QString("%1°/s").arg(pitchGyro, 0, 'f', 4));
		updateStatusWidget(ui.le_0D_ReadGyroRoll_Feedback, status, QString("%1°/s").arg(rollGyro, 0, 'f', 2));
	}

	//// ------------------------------加速度UI处理 ------------------------------
	else if (cmdCode == CommandCode::INS_GetAcceleration) {
		setIcon(ui.lbl_0D_GetAccelIcon, icon);  // 需确保UI有该控件（可根据实际命名调整）
		// 解析三轴加速度参数
		double xAccel = checkResult->getParameter("x_accel").toDouble();
		double yAccel = checkResult->getParameter("y_accel").toDouble();
		double zAccel = checkResult->getParameter("z_accel").toDouble();
		// 更新UI控件（需确保控件命名与实际UI一致）
		updateStatusWidget(ui.le_0D_ReadAccelX_Feedback, status, QString("%1m/s²").arg(xAccel, 0, 'f', 2));
		updateStatusWidget(ui.le_0D_ReadAccelY_Feedback, status, QString("%1m/s²").arg(yAccel, 0, 'f', 2));
		updateStatusWidget(ui.le_0D_ReadAccelZ_Feedback, status, QString("%1m/s²").arg(zAccel, 0, 'f', 2));
	}

	else if (cmdCode == CommandCode::INS_GetStatus)
	{
		setIcon(ui.lbl_0D_GetStatusIcon, icon);
		QString workState = checkResult->getParameter("ins_work_state").toString();
        QString comboState = checkResult->getParameter("ins_combination_state").toString();
        QString checkState = checkResult->getParameter("ins_selfcheck_state").toString();
        updateStatusWidget(ui.le_0D_ReadWorkStatus_Feedback, status, workState);    // 工作状态文本框
        updateStatusWidget(ui.le_0D_CombinationState_Feedback, status, comboState);  // 组合状态文本框
        updateStatusWidget(ui.le_0D_SelfCheck_Feedback, status, checkState);  // 自检状态文本框
	}
	else if (CommandCode::INS_CombinedWithDVL == cmdCode)
	{
	    setIcon(ui.lbl_0D_CombinedWithDvlIcon, icon);
	    updateStatusWidget(ui.le_0D_CombinedWithDvl_Feedback, status, feedbackDesc);
	}
	else if (CommandCode::INS_CombinedAuto == cmdCode)
	{
		setIcon(ui.lbl_0D_CombinedAutoIcon, icon);
		updateStatusWidget(ui.le_0D_CombinedAuto_Feedback, status, feedbackDesc);
	}
}

void SelfCheckMainDialog_533::handleDvlResult(const SelfCheckResult* checkResult)
{
	CommandCode cmdCode = checkResult->commandCode();
	ResultStatus status = checkResult->status();
	QPixmap icon = getStatusIcon(status);
	QString feedbackDesc = checkResult->feedbackDesc();
	if (cmdCode == CommandCode::Common_PowerOn) {
		setIcon(ui.lbl_0E_PowerOnIcon, icon);
		updateStatusWidget(ui.le_0E_PowerOn_Feedback, status, feedbackDesc);
	}
	else if (cmdCode == CommandCode::Common_PowerOff) {
		setIcon(ui.lbl_0E_PowerOffIcon, icon);
		updateStatusWidget(ui.le_0E_PowerOff_Feedback, status, feedbackDesc);
	}
	else if (cmdCode == CommandCode::DVL_GetBottomSpeed) {
		setIcon(ui.lbl_0E_ReadBottomSpeedIcon, icon);
		// 读取解析后的参数（带默认值，避免空值报错）
		double x = checkResult->getParameter("dvl_bottom_x").toDouble();
		double y = checkResult->getParameter("dvl_bottom_y").toDouble();
		double z = checkResult->getParameter("dvl_bottom_z").toDouble();

		// 更新三轴速度UI（保留2位小数）
		updateStatusWidget(ui.le_0E_ReadBottomX_Feedback, status, QString("%1").arg(x, 0, 'f', 2));
		updateStatusWidget(ui.le_0E_ReadBottomY_Feedback, status, QString("%1").arg(y, 0, 'f', 2));
		updateStatusWidget(ui.le_0E_ReadBottomZ_Feedback, status, QString("%1").arg(z, 0, 'f', 2));
	}
	else if (cmdCode == CommandCode::DVL_GetWaterSpeed) {
		setIcon(ui.lbl_0E_ReadWaterSpeedIcon, icon);
		// 读取解析后的参数（带默认值，避免空值报错）
		double x = checkResult->getParameter("dvl_water_x").toDouble();
		double y = checkResult->getParameter("dvl_water_y").toDouble();
		double z = checkResult->getParameter("dvl_water_z").toDouble();

		// 更新三轴速度UI（保留2位小数）
		updateStatusWidget(ui.le_0E_ReadWaterX_Feedback, status, QString("%1").arg(x, 0, 'f', 2));
		updateStatusWidget(ui.le_0E_ReadWaterY_Feedback, status, QString("%1").arg(y, 0, 'f', 2));
		updateStatusWidget(ui.le_0E_ReadWaterZ_Feedback, status, QString("%1").arg(z, 0, 'f', 2));

	}
	else if (CommandCode::DVL_GetStatus == cmdCode)
	{
		// 1. 设置状态图标（成功/失败/未知）
		setIcon(ui.lbl_0E_ReadStatusIcon, icon);

		// 2. 读取解析后的状态参数（无参数时赋默认值）
		QString selfCheckCardState = checkResult->getParameter("dvl_selfcheck_card_state").toString();
		QString outputState = checkResult->getParameter("dvl_output_state").toString();

		// 3. 更新自检+存储卡状态文本框
		updateStatusWidget(ui.le_0E_SelfCheckCard_Feedback, status, selfCheckCardState);
		// 4. 更新输出状态文本框（仅更新文本，状态样式和自检状态一致）
		updateStatusWidget(ui.le_0E_OutputState_Feedback, status, outputState);
	}

}

void SelfCheckMainDialog_533::handleCameraResult(const SelfCheckResult* checkResult)
{
	CommandCode cmdCode = checkResult->commandCode();
	ResultStatus status = checkResult->status();
	QPixmap icon = getStatusIcon(status);
	QString feedbackDesc = checkResult->feedbackDesc();
	if (cmdCode == CommandCode::Common_PowerOn) {
		setIcon(ui.lbl_0F_PowerOnIcon, icon);
		updateStatusWidget(ui.le_0F_PowerOn_Feedback, status, feedbackDesc);
	}
	else if (cmdCode == CommandCode::Common_PowerOff) {
		setIcon(ui.lbl_0F_PowerOffIcon, icon);
		updateStatusWidget(ui.le_0F_PowerOff_Feedback, status, feedbackDesc);
	}
	else if (cmdCode == CommandCode::CameraLight_StartVideoSave) {
		setIcon(ui.lbl_0F_StartRecordIcon, icon);
		updateStatusWidget(ui.le_0F_StartRecord_Feedback, status, feedbackDesc);
	}
	else if (cmdCode == CommandCode::CameraLight_StopVideoSave) {
		setIcon(ui.lbl_0F_StopRecordIcon, icon);
		updateStatusWidget(ui.le_0F_StopRecord_Feedback, status, feedbackDesc);
	}
	else if (cmdCode == CommandCode::CameraLight_TakePhoto) {
		setIcon(ui.lbl_0F_TakePhotoIcon, icon);
		QString photoPath = checkResult->getParameter("path").toString();
		updateStatusWidget(ui.le_0F_Photo_Feedback, status, feedbackDesc);
	}
	
}

QString SelfCheckMainDialog_533::getThrusterNumber(DeviceId deviceId)
{
	switch (deviceId) {
	case DeviceId::TailThruster: return "01";
	case DeviceId::FrontVerticalThruster: return "16";
	case DeviceId::RearVerticalThruster: return "17";
	case DeviceId::FrontSideThruster: return "18";
	case DeviceId::RearSideThruster: return "0B";

	default: return "";
	}
}

QString SelfCheckMainDialog_533::getRudderNumber(DeviceId deviceId)
{
	switch (deviceId) {
	case DeviceId::Rudder1: return "02";
	case DeviceId::Rudder2: return "03";
	case DeviceId::Rudder3: return "04";
	case DeviceId::Rudder4: return "05";
	case DeviceId::AntennaFold:return "08";
	default: return "";
	}
}


QString SelfCheckMainDialog_533::getBatteryNumber(DeviceId deviceId)
{
	switch (deviceId) {
	case DeviceId::PowerBattery1: return "10";  // 动力电池1
	case DeviceId::MeterBattery: return "11";   // 仪表电池
	case DeviceId::PowerBattery2: return "1A";  // ：动力电池2
	default: return "";
	}
}

QString SelfCheckMainDialog_533::getSonarNumber(DeviceId deviceId)
{
	switch (deviceId) {
	case DeviceId::SideScanSonar:return "12";
	case DeviceId::ForwardSonar: return "13";
	case DeviceId::DownwardSonar: return "14";
	default: return "";
	}
}
/**
 * @brief 增强版：校验并返回补零后的卡号
 * @param cardIdText 输入文本
 * @param outCardId 输出参数：校验通过后的uint32_t卡号
 * @param errorMsg 输出参数：错误信息
 * @param autoPadZero 是否自动补零
 * @return bool 校验结果
 */
bool SelfCheckMainDialog_533::validateBeidouCardId(const QString& cardIdText,
	 QString& errorMsg, bool autoPadZero)
{
	// 步骤1：去除首尾空格，避免用户误输入
	QString cardIdStr = cardIdText.trimmed();

	// 步骤2：空值校验
	if (cardIdStr.isEmpty()) {
		errorMsg = "卡号不能为空！";
		return false;
	}

	// 步骤3：纯数字校验
	for (QChar c : cardIdStr) {
		if (!c.isDigit()) {
			errorMsg = "卡号必须为纯数字（不能包含字母、符号或空格）！";
			return false;
		}
	}

	// 步骤4：自动补零（可选，根据业务需求）
	if (autoPadZero && cardIdStr.length() < 7) {
		cardIdStr = cardIdStr.rightJustified(7, '0'); // 左侧补零，如"123"→"0000123"
	}

	// 步骤5：长度校验（严格7位）
	if (cardIdStr.length() != 7) {
		errorMsg = QString("卡号必须为7位数字！当前长度：%1位（已自动补零：%2）")
			.arg(cardIdText.trimmed().length())
			.arg(cardIdStr);
		return false;
	}

	// 步骤6：数值范围校验（0~9999999，兜底校验）
	bool convertOk = false;
	uint32_t cardId = cardIdStr.toUInt(&convertOk);
	if (!convertOk || cardId > 9999999) {
		errorMsg = QString("卡号超出有效范围（0~9999999）！当前值：%1").arg(cardIdStr);
		return false;
	}

	// 所有校验通过
	errorMsg = "校验通过";
	return true;
}

void SelfCheckMainDialog_533::updateStatusWidget(QWidget* widget, ResultStatus status, const QString& customText)
{
	// 从配置中获取默认文本和颜色（兜底到Unknown）
	QPair<QString, QColor> statusData = m_statusConfig.value(status, m_statusConfig[ResultStatus::Unknown]);

	// 优先使用自定义文本（支持特殊场景，如“失败：设备未响应”）
	QString showText = customText.isEmpty() ? statusData.first : customText;

	// 复用原有函数更新控件
	updateWidgetText(widget, showText, statusData.second);
}


void SelfCheckMainDialog_533::on_depMetPowerOnBtn_clicked()
{
	if (!m_module) return;

	// 上电命令无参数，使用EmptyParameter
	auto params = std::make_shared<EmptyParameter>();
	// 发送上电命令（Common_PowerOn）
	m_module->sendCheckCommand(DeviceId::DepthMeter, 
		CommandCode::Common_PowerOn, params);
}

void SelfCheckMainDialog_533::on_depMetPowerOffBtn_clicked()
{
	if (!m_module) return;

	// 下电命令无参数，使用EmptyParameter
	auto params = std::make_shared<EmptyParameter>();
	// 发送下电命令（Common_PowerOn）
	m_module->sendCheckCommand(DeviceId::DepthMeter, 
		CommandCode::Common_PowerOff, params);
}

void SelfCheckMainDialog_533::on_depMetCalibrateBtn_clicked()
{
	if (!m_module) return;

	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::DepthMeter,
		CommandCode::Depthgauge_Calibration, params);
}

void SelfCheckMainDialog_533::on_depMetGetDepthBtn_clicked()
{
	if (!m_module) return;

	// 获取深度值无参数，使用EmptyParameter
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::DepthMeter, 
		CommandCode::Depthgauge_GetDepth, params);

}

void SelfCheckMainDialog_533::on_altimeterPowerOnBtn_clicked()
{
	if (!m_module) return;

	// 上电命令无参数，使用EmptyParameter
	auto params = std::make_shared<EmptyParameter>();
	// 发送上电命令（Common_PowerOn）
	m_module->sendCheckCommand(DeviceId::Altimeter, 
		CommandCode::Common_PowerOn, params);
}

void SelfCheckMainDialog_533::on_altimeterPowerOffBtn_clicked()
{
	if (!m_module) return;

	// 上电命令无参数，使用EmptyParameter
	auto params = std::make_shared<EmptyParameter>();
	// 发送上电命令（Common_PowerOn）
	m_module->sendCheckCommand(DeviceId::Altimeter, 
		CommandCode::Common_PowerOff, params);
}

void SelfCheckMainDialog_533::on_altimeterHeightBtn_clicked()
{
	if (!m_module) return;

	// 获取高度值无参数，使用EmptyParameter
	auto params = std::make_shared<EmptyParameter>();

	m_module->sendCheckCommand(DeviceId::Altimeter, 
		CommandCode::Altimeter_GetHeight, params);
}

void SelfCheckMainDialog_533::on_cameraPowerOnBtn_clicked()
{
	// 上电命令无参数，使用EmptyParameter
	auto params = std::make_shared<EmptyParameter>();
	// 发送上电命令（Common_PowerOn）
	m_module->sendCheckCommand(DeviceId::CameraLight, 
		CommandCode::Common_PowerOn, params);
}

void SelfCheckMainDialog_533::on_cameraPowerOffBtn_clicked()
{

	if (!m_module) return;

	// 下电命令无参数，使用EmptyParameter
	auto params = std::make_shared<EmptyParameter>();
	// 发送下电命令（Common_PowerOn）
	m_module->sendCheckCommand(DeviceId::CameraLight, 
		CommandCode::Common_PowerOff, params);
}

void SelfCheckMainDialog_533::on_cameraStartRecordBtn_clicked()
{
	if (!m_module) return;

	auto params = std::make_shared<CameraMediaParam>(getNextCameraMediaSeq());
	m_module->sendCheckCommand(DeviceId::CameraLight,
		CommandCode::CameraLight_StartVideoSave, params);
}

void SelfCheckMainDialog_533::on_cameraStopRecordBtn_clicked()
{
	if (!m_module) return;

	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::CameraLight, 
		CommandCode::CameraLight_StopVideoSave, params);
}

void SelfCheckMainDialog_533::on_cameraTakePhotoBtn_clicked()
{
	if (!m_module) return;

	auto params = std::make_shared<CameraMediaParam>(getNextCameraMediaSeq());
	m_module->sendCheckCommand(DeviceId::CameraLight, 
		CommandCode::CameraLight_TakePhoto, params);

}

void SelfCheckMainDialog_533::on_usblPowerOnBtn_clicked()
{
	// 上电命令无参数，使用EmptyParameter
	auto params = std::make_shared<EmptyParameter>();
	// 发送上电命令（Common_PowerOn）
	m_module->sendCheckCommand(DeviceId::USBL, CommandCode::Common_PowerOn, 
		params);
}

void SelfCheckMainDialog_533::on_usblPowerOffBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::USBL, CommandCode::Common_PowerOff,
		params);
}

void SelfCheckMainDialog_533::on_insPowerOnBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::INS, CommandCode::Common_PowerOn,
		params);
}

void SelfCheckMainDialog_533::on_insPowerOffBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::INS, CommandCode::Common_PowerOff,
		params);
}

void SelfCheckMainDialog_533::on_insSetGnssBindBtn_clicked()
{
	// 1. 强制转换输入，无任何歧义
	double lon = ui.le_0D_SetGnssBindLon->text().toDouble();
	double lat = ui.le_0D_SetGnssBindLat->text().toDouble();

	// 2. 手动创建原始指针，再转为基类指针
	CommandParameter* rawParam = nullptr;
	try {
		rawParam = new InsGnssParam(lat, lon); // 直接new，无模板
		// 手动封装为shared_ptr（显式指定基类，避免推导）
		std::shared_ptr<CommandParameter> params(rawParam);

		// 3. 发送命令（确保sendCheckCommand接收的是shared_ptr<CommandParameter>）
		m_module->sendCheckCommand(DeviceId::INS, CommandCode::INS_SetGnssBind, params);

		QMessageBox::information(this, "成功", "发送成功");
	}
	catch (...) {
		delete rawParam; // 异常时释放原始指针
		QMessageBox::critical(this, "错误", "参数错误");
	}
}

void SelfCheckMainDialog_533::on_insCalibrationWithDVLBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::INS, CommandCode::INS_CalibrationWithDVL,
		params);
}

void SelfCheckMainDialog_533::on_insCombinedWithDVLBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::INS, CommandCode::INS_CombinedWithDVL,
		params);
}

void SelfCheckMainDialog_533::on_insCombinedAutoBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::INS, CommandCode::INS_CombinedAuto,
		params);
}

void SelfCheckMainDialog_533::on_insGetUtcDateBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::INS, CommandCode::INS_GetUtcDate,
		params);
}

void SelfCheckMainDialog_533::on_insGetUtcTimeBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::INS, CommandCode::INS_GetUtcTime,
		params);
}

void SelfCheckMainDialog_533::on_insGetAttitudeBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::INS, CommandCode::INS_GetAttitude,
		params);
}

void SelfCheckMainDialog_533::on_insGetBodySpeedBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::INS, CommandCode::INS_GetBodySpeed,
		params);
}

void SelfCheckMainDialog_533::on_insGetGeoSpeedBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::INS, CommandCode::INS_GetGeoSpeed,
		params);
}

void SelfCheckMainDialog_533::on_insGetPositionBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::INS, CommandCode::INS_GetPosition,
		params);
}

void SelfCheckMainDialog_533::on_insGetGyroscopeSpeedBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::INS, CommandCode::INS_GetGyroscopeSpeed,
		params);
}

void SelfCheckMainDialog_533::on_insGetAccelerationBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::INS, CommandCode::INS_GetAcceleration,
		params);
}

void SelfCheckMainDialog_533::on_insGetStatusBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::INS, CommandCode::INS_GetStatus,
		params);

	/*工作状态：0：准备，2：对准 ，4：导航
	  组合状态:00000：纯惯导，00001：卫导组合，00100：dvl对底组合，
	  00101：卫导组合+dvl组合
	  dvl自检：uint8 0-2bit：000初始，001：自检失败存储卡读取成功，010：自检失败且存储卡读取失败
	  011：自检成功存储卡读取成功，100：自检成功存储卡读取失败    
	  bit3:0：dvl无输出，1：dvl有输出 
	  惯导自检：uint8 bit0-1:0：初始，1：自检中，2：自检成功,3:自检失败

	  故障位：0：正常，1：故障
	*/
}

void SelfCheckMainDialog_533::on_buoyancyAdjustingPowerOnBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::BuoyancyAdjust, 
		CommandCode::Common_PowerOn, params);
}

void SelfCheckMainDialog_533::on_buoyancyAdjustingPowerOffBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::BuoyancyAdjust,
		CommandCode::Common_PowerOff, params);
}

void SelfCheckMainDialog_533::on_buoyancyAdjustingGetbuoyancyValue_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::BuoyancyAdjust,
		CommandCode::Buoyancy_ReadValue, params);
}

void SelfCheckMainDialog_533::on_buoyancyAdjustingSetbuoyancyValue_clicked()
{
	// 1. 获取用户输入的浮力值文本并去空
	QString volumeText = ui.le_06_SetBuoyancyValue_input->text().trimmed();
	if (volumeText.isEmpty()) {
		QMessageBox::warning(this, "输入错误", "请输入浮力值");
		return;
	}

	// 2. 文本转double（附带有效性校验，避免非数字输入）
	bool convertOk = false;
	double volumeD = volumeText.toDouble(&convertOk);
	// 校验转换是否成功
	if (!convertOk) {
		QMessageBox::warning(this, "输入错误", "请输入有效的数字格式浮力值");
		return;
	}

	// 3. 核心校验：浮力值不允许超过10.06，同时校验非负（物理意义上浮力值不应为负）
	const double MAX_BUOYANCY_VALUE = 10.06; // 定义最大允许浮力值（常量，便于维护）
	if (volumeD < 0 || volumeD > MAX_BUOYANCY_VALUE) {
		QString warningMsg = QString("浮力值输入无效！\n请输入0到%1之间的数值").arg(MAX_BUOYANCY_VALUE);
		QMessageBox::warning(this, "输入错误", warningMsg);
		return;
	}

	// 4. 合法值：转换为uint16_t类型（放大100倍）
	uint16_t volume = static_cast<uint16_t>(volumeD * 100); // 显式类型转换，更安全
	auto params = std::make_shared<BuoyancyVolumeParameter>(volume);
	m_module->sendCheckCommand(DeviceId::BuoyancyAdjust,
		CommandCode::Buoyancy_SetValue, params);
}

void SelfCheckMainDialog_533::on_buoyancyAdjustingStop_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::BuoyancyAdjust,
		CommandCode::Buoyancy_Stop, params);
}



void SelfCheckMainDialog_533::on_forwardSonarPowerOnBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::ForwardSonar, 
		CommandCode::Common_PowerOn, params);
}

void SelfCheckMainDialog_533::on_forwardSonarPowerOffBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::ForwardSonar, 
		CommandCode::Common_PowerOff, params);
}

void SelfCheckMainDialog_533::on_forwardSonarEnabelBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::ForwardSonar, 
		CommandCode::Common_Enable, params);
}

void SelfCheckMainDialog_533::on_forwardSonarDisableBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::ForwardSonar, 
		CommandCode::Common_Disable, params);
}

void SelfCheckMainDialog_533::on_downwardSonarPowerOnBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::DownwardSonar, 
		CommandCode::Common_PowerOn, params);
}

void SelfCheckMainDialog_533::on_downwardSonarPowerOffBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::DownwardSonar, 
		CommandCode::Common_PowerOff, params);
}

void SelfCheckMainDialog_533::on_downwardSonarEnabelBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::DownwardSonar,
		CommandCode::Common_Enable, params);
}

void SelfCheckMainDialog_533::on_downwardSonarDisableBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::DownwardSonar,
		CommandCode::Common_Disable, params);
}

void SelfCheckMainDialog_533::on_sideScanSonarPowerOnBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::SideScanSonar,
		CommandCode::Common_PowerOn, params);
}

void SelfCheckMainDialog_533::on_sideScanSonarPowerOffBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::SideScanSonar,
		CommandCode::Common_PowerOff, params);
}

void SelfCheckMainDialog_533::on_sideScanSonarEnabelBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::SideScanSonar,
		CommandCode::Common_Enable, params);
}

void SelfCheckMainDialog_533::on_sideScanSonarDisableBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::SideScanSonar,
		CommandCode::Common_Disable, params);
}

void SelfCheckMainDialog_533::on_antennaFoldPowerOnBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::AntennaFold, 
		CommandCode::Common_PowerOn, params);
}

void SelfCheckMainDialog_533::on_antennaFoldPowerOffBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::AntennaFold,
		CommandCode::Common_PowerOff, params);
}

void SelfCheckMainDialog_533::on_antennaFoldSetAngle_clicked()
{
	if (!m_module) return;
	// 1. 获取用户输入（支持正负角度，例如：30 → 30°，-15 → -15°）
	QString angleText = ui.le_08_TargetAngle_Input->text().trimmed();
	if (angleText.isEmpty()) {
		QMessageBox::warning(this, "输入错误", "请输入目标角度（-90~90度）");
		return;
	}
	// 2. 转换为double类型（支持小数输入，例如30.5 → 305十分度）
	bool ok = false;
	double angleDegree = angleText.toDouble(&ok);
	if (!ok) {
		QMessageBox::warning(this, "输入错误", "角度必须为数字（支持正负）");
		return;
	}

	// 3. 校验角度范围（-90° ~ 90°）
	if (angleDegree < -90.0 || angleDegree > 90.0) {
		QMessageBox::warning(this, "输入错误", "角度超出范围（-90~90度）");
		return;
	}

	// 4. 转换为十分度（*10），并转换为int16_t（避免溢出）
	uint16_t angleTenthDegree = static_cast<uint16_t>(angleDegree );

	try {
		// 5. 创建参数对象（内部会自动转换为uint16_t并校验范围）
		auto params = std::make_shared<RudderAngleParameter>(angleTenthDegree);
		// 6. 发送命令
		m_module->sendCheckCommand(DeviceId::AntennaFold, CommandCode::Rudder_SetAngle, params);

		QMessageBox::information(this, "提示", QString("已发送舵角设置命令：%1度（十分度：%2）")
			.arg(angleDegree, 0, 'f', 1)
			.arg(angleTenthDegree));
	}
	catch (const std::invalid_argument& e) {
		QMessageBox::critical(this, "参数错误", e.what());
	}
}

void SelfCheckMainDialog_533::on_antennaFoldGetRunTime_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::AntennaFold,
		CommandCode::Rudder_GetRunTime, params);
}

void SelfCheckMainDialog_533::on_antennaFoldGetVoltage_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::AntennaFold,
		CommandCode::Rudder_GetVoltage, params);
}

void SelfCheckMainDialog_533::on_antennaFoldGetAngle_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::AntennaFold,
		CommandCode::Rudder_GetAngle, params);
}

void SelfCheckMainDialog_533::on_antennaFoldGetCurrent_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::AntennaFold,
		CommandCode::Rudder_GetCurrent, params);
}

void SelfCheckMainDialog_533::on_antennaFoldGetTemp_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::AntennaFold,
		CommandCode::Rudder_GetTemp, params);
}

void SelfCheckMainDialog_533::on_antennaFoldGetFault_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::AntennaFold,
		CommandCode::Common_GetFault, params);
}

void SelfCheckMainDialog_533::on_rudder1PowerOnBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder1,
		CommandCode::Common_PowerOn, params);
}

void SelfCheckMainDialog_533::on_rudder1PowerOffBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder1,
		CommandCode::Common_PowerOff, params);
}

void SelfCheckMainDialog_533::on_rudder1SetAngleBtn_clicked()
{
	if (!m_module) return;
	// 1. 获取用户输入（支持正负角度，例如：30 → 30°，-15 → -15°）
	QString angleText = ui.le_02_TargetAngle_Input->text().trimmed();
	if (angleText.isEmpty()) {
		QMessageBox::warning(this, "输入错误", "请输入目标角度（-90~90度）");
		return;
	}
	// 2. 转换为double类型（支持小数输入，例如30.5 → 305十分度）
	bool ok = false;
	double angleDegree = angleText.toDouble(&ok);
	if (!ok) {
		QMessageBox::warning(this, "输入错误", "角度必须为数字（支持正负）");
		return;
	}

	// 3. 校验角度范围（-90° ~ 90°）
	if (angleDegree < -90.0 || angleDegree > 90.0) {
		QMessageBox::warning(this, "输入错误", "角度超出范围（-90~90度）");
		return;
	}

	// 4. 转换为十分度（*10），并转换为int16_t（避免溢出）
	int16_t angleTenthDegree = static_cast<int16_t>(angleDegree);

	try {
		// 5. 创建参数对象（内部会自动转换为int16_t并校验范围）
		auto params = std::make_shared<RudderAngleParameter>(angleTenthDegree);
		// 6. 发送命令
		m_module->sendCheckCommand(DeviceId::Rudder1, CommandCode::Rudder_SetAngle, params);

		QMessageBox::information(this, "提示", QString("已发送舵角设置命令：%1度（十分度：%2）")
			.arg(angleDegree, 0, 'f', 1)
			.arg(angleTenthDegree));
	}
	catch (const std::invalid_argument& e) {
		QMessageBox::critical(this, "参数错误", e.what());
	}
}

void SelfCheckMainDialog_533::on_rudder1GetRunTimeBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder1,
		CommandCode::Rudder_GetRunTime, params);

}

void SelfCheckMainDialog_533::on_rudder1GetVoltageBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder1,
		CommandCode::Rudder_GetVoltage, params);
}

void SelfCheckMainDialog_533::on_rudder1GetAngleBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder1,
		CommandCode::Rudder_GetAngle, params);
}

void SelfCheckMainDialog_533::on_rudder1GetCurrentBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder1,
		CommandCode::Rudder_GetCurrent, params);
}

void SelfCheckMainDialog_533::on_rudder1GetTempBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder1,
		CommandCode::Rudder_GetTemp, params);
}

void SelfCheckMainDialog_533::on_rudder1SetZeroBtn_clicked()
{
}

void SelfCheckMainDialog_533::on_rudder1GetFaultBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder1,
		CommandCode::Common_GetFault, params);
}

void SelfCheckMainDialog_533::on_rudder2PowerOnBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder2,
		CommandCode::Common_PowerOn, params);
}

void SelfCheckMainDialog_533::on_rudder2PowerOffBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder2,
		CommandCode::Common_PowerOff, params);
}

void SelfCheckMainDialog_533::on_rudder2SetAngleBtn_clicked()
{
	if (!m_module) return;
	// 1. 获取用户输入（支持正负角度，例如：30 → 30°，-15 → -15°）
	QString angleText = ui.le_03_TargetAngle_Input->text().trimmed();
	if (angleText.isEmpty()) {
		QMessageBox::warning(this, "输入错误", "请输入目标角度（-90~90度）");
		return;
	}
	// 2. 转换为double类型（支持小数输入，例如30.5 → 305十分度）
	bool ok = false;
	double angleDegree = angleText.toDouble(&ok);
	if (!ok) {
		QMessageBox::warning(this, "输入错误", "角度必须为数字（支持正负）");
		return;
	}

	// 3. 校验角度范围（-90° ~ 90°）
	if (angleDegree < -90.0 || angleDegree > 90.0) {
		QMessageBox::warning(this, "输入错误", "角度超出范围（-90~90度）");
		return;
	}

	// 4. 转换为十分度（*10），并转换为int16_t（避免溢出）
	int16_t angleTenthDegree = static_cast<int16_t>(angleDegree);

	try {
		// 5. 创建参数对象（内部会自动转换为int16_t并校验范围）
		auto params = std::make_shared<RudderAngleParameter>(angleTenthDegree);
		// 6. 发送命令
		m_module->sendCheckCommand(DeviceId::Rudder2, CommandCode::Rudder_SetAngle, params);

		QMessageBox::information(this, "提示", QString("已发送舵角设置命令：%1度（十分度：%2）")
			.arg(angleDegree, 0, 'f', 1)
			.arg(angleTenthDegree));
	}
	catch (const std::invalid_argument& e) {
		QMessageBox::critical(this, "参数错误", e.what());
	}
}

void SelfCheckMainDialog_533::on_rudder2GetRunTimeBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder2,
		CommandCode::Rudder_GetRunTime, params);
}

void SelfCheckMainDialog_533::on_rudder2GetVoltageBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder2,
		CommandCode::Rudder_GetVoltage, params);
}

void SelfCheckMainDialog_533::on_rudder2GetAngleBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder2,
		CommandCode::Rudder_GetAngle, params);
}

void SelfCheckMainDialog_533::on_rudder2GetCurrentBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder2,
		CommandCode::Rudder_GetCurrent, params);
}

void SelfCheckMainDialog_533::on_rudder2GetTempBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder2,
		CommandCode::Rudder_GetTemp, params);
}

void SelfCheckMainDialog_533::on_rudder2SetZeroBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder2,
		CommandCode::Rudder_SetZero, params);
}

void SelfCheckMainDialog_533::on_rudder2GetFaultBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder2,
		CommandCode::Common_GetFault, params);
}

void SelfCheckMainDialog_533::on_rudder3PowerOnBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder3,
		CommandCode::Common_PowerOn, params);
}

void SelfCheckMainDialog_533::on_rudder3PowerOffBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder3,
		CommandCode::Common_PowerOff, params);
}

void SelfCheckMainDialog_533::on_rudder3SetAngleBtn_clicked()
{
	if (!m_module) return;
	// 1. 获取用户输入（支持正负角度，例如：30 → 30°，-15 → -15°）
	QString angleText = ui.le_04_TargetAngle_Input->text().trimmed();
	if (angleText.isEmpty()) {
		QMessageBox::warning(this, "输入错误", "请输入目标角度（-90~90度）");
		return;
	}
	// 2. 转换为double类型（支持小数输入，例如30.5 → 305十分度）
	bool ok = false;
	double angleDegree = angleText.toDouble(&ok);
	if (!ok) {
		QMessageBox::warning(this, "输入错误", "角度必须为数字（支持正负）");
		return;
	}

	// 3. 校验角度范围（-90° ~ 90°）
	if (angleDegree < -90.0 || angleDegree > 90.0) {
		QMessageBox::warning(this, "输入错误", "角度超出范围（-90~90度）");
		return;
	}

	// 4. 转换为十分度（*10），并转换为int16_t（避免溢出）
	int16_t angleTenthDegree = static_cast<int16_t>(angleDegree);

	try {
		// 5. 创建参数对象（内部会自动转换为uint16_t并校验范围）
		auto params = std::make_shared<RudderAngleParameter>(angleTenthDegree);
		// 6. 发送命令
		m_module->sendCheckCommand(DeviceId::Rudder3, CommandCode::Rudder_SetAngle, params);

		QMessageBox::information(this, "提示", QString("已发送舵角设置命令：%1度（十分度：%2）")
			.arg(angleDegree, 0, 'f', 1)
			.arg(angleTenthDegree));
	}
	catch (const std::invalid_argument& e) {
		QMessageBox::critical(this, "参数错误", e.what());
	}
}

void SelfCheckMainDialog_533::on_rudder3GetRunTimeBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder3,
		CommandCode::Rudder_GetRunTime, params);
}

void SelfCheckMainDialog_533::on_rudder3GetVoltageBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder3,
		CommandCode::Rudder_GetVoltage, params);
}

void SelfCheckMainDialog_533::on_rudder3GetAngleBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder3,
		CommandCode::Rudder_GetAngle, params);
}

void SelfCheckMainDialog_533::on_rudder3GetCurrentBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder3,
		CommandCode::Rudder_GetCurrent, params);
}

void SelfCheckMainDialog_533::on_rudder3GetTempBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder3,
		CommandCode::Rudder_GetTemp, params);
}

void SelfCheckMainDialog_533::on_rudder3SetZeroBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder3,
		CommandCode::Rudder_SetZero, params);
}

void SelfCheckMainDialog_533::on_rudder3GetFaultBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder3,
		CommandCode::Common_GetFault, params);
}

void SelfCheckMainDialog_533::on_rudder4PowerOnBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder4,
		CommandCode::Common_PowerOn, params);
}

void SelfCheckMainDialog_533::on_rudder4PowerOffBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder4,
		CommandCode::Common_PowerOff, params);
}

void SelfCheckMainDialog_533::on_rudder4SetAngleBtn_clicked()
{
	if (!m_module) return;
	// 1. 获取用户输入（支持正负角度，例如：30 → 30°，-15 → -15°）
	QString angleText = ui.le_05_TargetAngle_Input->text().trimmed();
	if (angleText.isEmpty()) {
		QMessageBox::warning(this, "输入错误", "请输入目标角度（-90~90度）");
		return;
	}
	// 2. 转换为double类型（支持小数输入，例如30.5 → 305十分度）
	bool ok = false;
	double angleDegree = angleText.toDouble(&ok);
	if (!ok) {
		QMessageBox::warning(this, "输入错误", "角度必须为数字（支持正负）");
		return;
	}

	// 3. 校验角度范围（-90° ~ 90°）
	if (angleDegree < -90.0 || angleDegree > 90.0) {
		QMessageBox::warning(this, "输入错误", "角度超出范围（-90~90度）");
		return;
	}

	// 4. 转换为十分度（*10），并转换为int16_t（避免溢出）
	int16_t angleTenthDegree = static_cast<int16_t>(angleDegree);

	try {
		// 5. 创建参数对象（内部会自动转换为int16_t并校验范围）
		auto params = std::make_shared<RudderAngleParameter>(angleTenthDegree);
		// 6. 发送命令
		m_module->sendCheckCommand(DeviceId::Rudder4, CommandCode::Rudder_SetAngle, params);

		QMessageBox::information(this, "提示", QString("已发送舵角设置命令：%1度（十分度：%2）")
			.arg(angleDegree, 0, 'f', 1)
			.arg(angleTenthDegree));
	}
	catch (const std::invalid_argument& e) {
		QMessageBox::critical(this, "参数错误", e.what());
	}
}

void SelfCheckMainDialog_533::on_rudder4GetRunTimeBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder4,
		CommandCode::Rudder_GetRunTime, params);
}

void SelfCheckMainDialog_533::on_rudder4GetVoltageBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder4,
		CommandCode::Rudder_GetVoltage, params);
}

void SelfCheckMainDialog_533::on_rudder4GetAngleBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder4,
		CommandCode::Rudder_GetAngle, params);
}

void SelfCheckMainDialog_533::on_rudder4GetCurrentBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder4,
		CommandCode::Rudder_GetCurrent, params);
}

void SelfCheckMainDialog_533::on_rudder4GetTempBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder4,
		CommandCode::Rudder_GetTemp, params);
}

void SelfCheckMainDialog_533::on_rudder4SetZeroBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder4,
		CommandCode::Rudder_SetZero, params);
}

void SelfCheckMainDialog_533::on_rudder4GetFaultBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::Rudder4,
		CommandCode::Common_GetFault, params);
}

void SelfCheckMainDialog_533::on_tailThrusterPowerOnBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::TailThruster,
		CommandCode::Common_PowerOn, params);
}

void SelfCheckMainDialog_533::on_tailThrusterPowerOffBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::TailThruster,
		CommandCode::Common_PowerOff, params);
}

void SelfCheckMainDialog_533::on_tailThrusterParamResetBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::TailThruster,
		CommandCode::Thruster_ParamReset, params);
}

void SelfCheckMainDialog_533::on_tailThrusterEnableWorkBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::TailThruster,
		CommandCode::Common_Enable, params);
}

void SelfCheckMainDialog_533::on_tailThrusterSetSpeedBtn_clicked()
{
	// 1. 获取用户输入的转速文本
	QString speedText = ui.le_01_TargetSpeed_Input->text().trimmed();
	if (speedText.isEmpty()) {
		QMessageBox::warning(this, "输入错误", "请输入目标转速");
		return;
	}

	// 2. 将文本转换为int16_t类型（转速）
	bool ok = false;
	int16_t speed = speedText.toInt(&ok); // 仅支持非负整数
	if (!ok) {
		QMessageBox::warning(this, "输入错误", "转速必须为整数");
		return;
	}

	// 3. 校验转速范围（-450-450 RPM，与SpeedParameter构造函数保持一致）
	if (speed > 450 ||speed <-450) {
		QMessageBox::warning(this, "输入错误", "转速超出范围（-450-450 RPM）");
		return;
	}

	try {
		// 5. 创建转速参数（SpeedParameter会二次校验，确保安全）
		auto params = std::make_shared<SpeedParameter>(speed);

		// 6. 发送转速设置命令（需使用对应设备的"设置转速"命令码，如Common_SetSpeed）
		// 注意：CommandCode::Common_SetSpeed需替换为项目中实际的转速命令码
		m_module->sendCheckCommand(DeviceId::TailThruster, CommandCode::Thruster_SetSpeed, params);

		QMessageBox::information(this, "提示", QString("已发送转速设置命令：%1 RPM").arg(speed));
	}
	catch (const std::invalid_argument& e) {
		// 捕获参数校验异常（如极端情况的范围错误）
		QMessageBox::critical(this, "参数错误", e.what());
	}
}

void SelfCheckMainDialog_533::on_tailThrusterSetDutyBtn_clicked()
{
	QString dutyText = ui.le_01_TargetDuty_Input->text().trimmed();
	if (dutyText.isEmpty()) {
		QMessageBox::warning(this, "输入错误", "请输入目标占空比");
		return;
	}

	bool ok = false;
	uint16_t duty = dutyText.toUInt(&ok); // 仅支持非负整数
	if (!ok) {
		QMessageBox::warning(this, "输入错误", "目标占空比必须为非负整数");
		return;
	}

	if (duty > 2000) {
		QMessageBox::warning(this, "输入错误", "目标占空比超出范围（0-2000）");
		return;
	}

	try {
		auto params = std::make_shared<ThrusterDutyParameter>(duty);
		m_module->sendCheckCommand(DeviceId::TailThruster, CommandCode::Thruster_SetDuty, params);

		QMessageBox::information(this, "提示", QString("已发送目标占空比设置命令：%1 RPM").arg(duty));
	}
	catch (const std::invalid_argument& e) {
		QMessageBox::critical(this, "参数错误", e.what());
	}
}

void SelfCheckMainDialog_533::on_tailThrusterSetRunTimeBtn_clicked()
{
	QString runTimeText = ui.le_01_SetRunTime_Input->text().trimmed();
	if (runTimeText.isEmpty()) {
		QMessageBox::warning(this, "输入错误", "请输入连续运行时间");
		return;
	}

	bool ok = false;
	uint16_t runTime = runTimeText.toUInt(&ok); // 仅支持非负整数
	if (!ok) {
		QMessageBox::warning(this, "输入错误", "连续运行时间必须为非负整数");
		return;
	}

	if (runTime < 0) {
		QMessageBox::warning(this, "输入错误", "连续运行时间超出范围（0-2000）");
		return;
	}

	try {
		auto params = std::make_shared<ThrusterRunTimeParameter>(runTime);
		m_module->sendCheckCommand(DeviceId::TailThruster, CommandCode::Thruster_SetRunTime, params);

		QMessageBox::information(this, "提示", QString("已发送连续运行时间设置命令：%1 秒").arg(runTime));
	}
	catch (const std::invalid_argument& e) {
		QMessageBox::critical(this, "参数错误", e.what());
	}
}

void SelfCheckMainDialog_533::on_tailThrusterSetAccelTimeBtn_clicked()
{
	QString accelTimeText = ui.le_01_AccelTime_Input->text().trimmed();
	if (accelTimeText.isEmpty()) {
		QMessageBox::warning(this, "输入错误", "请输入加速时间");
		return;
	}

	bool ok = false;
	uint16_t accelTime = accelTimeText.toUInt(&ok); // 仅支持非负整数
	if (!ok) {
		QMessageBox::warning(this, "输入错误", "加速时间必须为非负整数");
		return;
	}

	if (accelTime < 0) {
		QMessageBox::warning(this, "输入错误", "加速时间超出范围（0-2000）");
		return;
	}

	try {
		auto params = std::make_shared<AccelTimeParameter>(accelTime);
		m_module->sendCheckCommand(DeviceId::TailThruster, CommandCode::Thruster_SetAccelTime, params);

		QMessageBox::information(this, "提示", QString("已发送加速时间设置命令：%1 RPM").arg(accelTime));
	}
	catch (const std::invalid_argument& e) {
		QMessageBox::critical(this, "参数错误", e.what());
	}
}

void SelfCheckMainDialog_533::on_tailThrusterGetRunTimeBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::TailThruster,
		CommandCode::Thruster_GetRunTime, params);
}

void SelfCheckMainDialog_533::on_tailThrusterGetDutyBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::TailThruster,
		CommandCode::Thruster_GetDuty, params);
}

void SelfCheckMainDialog_533::on_tailThrusterGetCurrentBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::TailThruster,
		CommandCode::Thruster_GetCurrent, params);
}

void SelfCheckMainDialog_533::on_tailThrusterGetTemperatureBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::TailThruster,
		CommandCode::Thruster_GetTemperature, params);
}

void SelfCheckMainDialog_533::on_tailThrusterGetSpeedBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::TailThruster,
		CommandCode::Thruster_GetSpeed, params);
}

void SelfCheckMainDialog_533::on_tailThrusterGetHallBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::TailThruster,
		CommandCode::Thruster_GetHall, params);
}

void SelfCheckMainDialog_533::on_tailThrusterGetBusVoltageBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::TailThruster,
		CommandCode::Thruster_GetBusVoltage, params);
}

void SelfCheckMainDialog_533::on_tailThrusterGetNoMaintainTimeBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::TailThruster,
		CommandCode::Thruster_GetNoMaintainTime, params);
}

void SelfCheckMainDialog_533::on_tailThrusterGetTotalRunTimeBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::TailThruster,
		CommandCode::Thruster_GetTotalRunTime, params);
}


void SelfCheckMainDialog_533::on_tailThrusterSetSpeedWithTimeBtn_clicked()
{
	// 1. 获取用户输入的转速文本
	QString speedText = ui.le_01_Speed->text().trimmed();
	if (speedText.isEmpty()) {
		QMessageBox::warning(this, "输入错误", "请输入目标转速");
		return;
	}

	// 2. 将文本转换为int16_t类型（转速）
	bool ok = false;
	int16_t speed = speedText.toInt(&ok); // 仅支持非负整数
	if (!ok) {
		QMessageBox::warning(this, "输入错误", "转速必须为整数");
		return;
	}

	// 3. 校验转速范围（-450-450 RPM，与SpeedParameter构造函数保持一致）
	if (speed > 450 || speed < -450) {
		QMessageBox::warning(this, "输入错误", "转速超出范围（-450-450 RPM）");
		return;
	}

	QString runTimeText = ui.le_01_Time->text().trimmed();
	if (runTimeText.isEmpty()) {
		QMessageBox::warning(this, "输入错误", "请输入运行时间");
		return;
	}
	ok = false;
	uint8_t runTime = runTimeText.toInt(&ok); // 仅支持非负整数
	if (!ok) {
		QMessageBox::warning(this, "输入错误", "时间必须为整数");
		return;
	}

	// 3. 校验时间范围（min，0-255）
	if (runTime > 255 || runTime < 0) {
		QMessageBox::warning(this, "输入错误", "运行时间超出范围（0-255 RPM）");
		return;
	}

	try {
	
		auto params = std::make_shared<ThrusterSpeedAndRunTimeParameter>(speed,runTime);
		m_module->sendCheckCommand(DeviceId::TailThruster, CommandCode::Thruster_SetSpeedWithTime,
			params);

		QMessageBox::information(this, "提示", QString("已发送转速设置命令：%1 RPM").arg(speed));
	}
	catch (const std::invalid_argument& e) {
		// 捕获参数校验异常（如极端情况的范围错误）
		QMessageBox::critical(this, "参数错误", e.what());
	}
}


void SelfCheckMainDialog_533::on_beidouGetSignalQualityBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::BeidouComm,
		CommandCode::BeidouComm_GetSignalQuality, params,WorkPhase::ConnectivityTest);
}

void SelfCheckMainDialog_533::on_beidouGetIdentityBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::BeidouComm,
		CommandCode::BeidouComm_GetIdentity, params, WorkPhase::ConnectivityTest);
}

void SelfCheckMainDialog_533::on_beidouGetTargetBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::BeidouComm,
		CommandCode::BeidouComm_GetTarget, params, WorkPhase::ConnectivityTest);
}

void SelfCheckMainDialog_533::on_beidouSetTargetBtn_clicked()
{
	QString cardID = ui.le_09_SetTarget_Input->text();
	auto params = std::make_shared<BeidouCardNoParam>(cardID);
	m_module->sendCheckCommand(DeviceId::BeidouComm,
		CommandCode::BeidouComm_SetTarget, params, WorkPhase::ConnectivityTest);
}

void SelfCheckMainDialog_533::on_beidouGetWhitelistBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::BeidouComm,
		CommandCode::BeidouComm_GetWhitelist, params, WorkPhase::ConnectivityTest);
}

void SelfCheckMainDialog_533::on_beidouAddWhitelistBtn_clicked()
{
	QString cardID = ui.le_09_AddWhitelist_Input->text();
	if (cardID.isEmpty())
		return;
	auto params = std::make_shared<BeidouCardNoParam>(cardID);
	m_module->sendCheckCommand(DeviceId::BeidouComm,
		CommandCode::BeidouComm_AddWhitelist, params, WorkPhase::ConnectivityTest);
}

void SelfCheckMainDialog_533::on_beidouDelWhitelistBtn_clicked()
{
	QString cardID = ui.le_09_DelWhitelist_Input->text();
	auto params = std::make_shared<BeidouCardNoParam>(cardID);
	m_module->sendCheckCommand(DeviceId::BeidouComm,
		CommandCode::BeidouComm_DelWhitelist, params, WorkPhase::ConnectivityTest);
}

void SelfCheckMainDialog_533::on_beidouGetPositionBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::BeidouComm,
		CommandCode::BeidouComm_GetPosition, params, WorkPhase::ConnectivityTest);
}

void SelfCheckMainDialog_533::on_beidouGetSysInfoBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::BeidouComm,
		CommandCode::BeidouComm_GetSysInfo, params, WorkPhase::ConnectivityTest);
}

void SelfCheckMainDialog_533::on_beidouFactoryResetBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::BeidouComm,
		CommandCode::BeidouComm_FactoryReset, params, WorkPhase::ConnectivityTest);
}

void SelfCheckMainDialog_533::on_beidouRebootBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::BeidouComm,
		CommandCode::BeidouComm_Reboot, params, WorkPhase::ConnectivityTest);
}

void SelfCheckMainDialog_533::on_beidouCommTestBtn_clicked()
{
	DeviceId deviceId = DeviceId::BeidouComm;
	CommandCode cmdCode = CommandCode::BeidouComm_Test;
	WorkPhase workPhase = WorkPhase::ConnectivityTest;

	// 2. 提前获取 SelfCheckModule 的全局唯一 packetSeq（解决帧构建依赖）
	uint8_t packetSeq = m_module->getUniquePacketSeq();
	// 3. 复用原有业务参数（空参数，与原代码一致）
	auto emptyParam = std::make_shared<EmptyParameter>();
	QByteArray businessParamBytes = emptyParam ? emptyParam->toBytes() : QByteArray();
	// 4. 直接创建 InternalCommandParam 并【一次性构建完整业务帧】
	auto sndParam = std::make_shared<InternalCommandParam>();
	{
		// 关键：在创建 sndParam 时，直接构建完整业务帧（所有参数齐全，无后续补全）
		QByteArray fullBusinessFrame = FrameBuilder::buildCheckCommandFrame(
			deviceId,        // 设备ID
			cmdCode,         // 命令码
			workPhase,       // 工作阶段
			businessParamBytes, // 业务参数字节流（原有数据）
			packetSeq        // 全局唯一包序号（已提前获取）
		);

		// 将完整帧数据及核心参数存储到 InternalCommandParam 中（供后续适配器使用）
		sndParam->m_extraParams["packetSeq"] = packetSeq;
		sndParam->m_extraParams["original_frame_data"] = fullBusinessFrame; // 完整业务帧（核心数据）
		sndParam->m_extraParams["original_frame_hex"] = fullBusinessFrame.toHex().toUpper(); // 帧HEX（日志/展示用）
		sndParam->m_extraParams["deviceId"] = static_cast<int>(deviceId);
		sndParam->m_extraParams["cmdCode"] = static_cast<int>(cmdCode);
	}

	// 5. 直接调用 sendCheckCommand，参数已携带完整帧数据，无需后续处理
	CheckError err = m_module->sendCheckCommand(
		deviceId,
		cmdCode,
		sndParam, // 已构建完整帧的参数
		workPhase,
		CommunicationChannel::BDS,
		CommandType::Internal
	);
	// 错误日志
	if (err.code() != ErrorCode::Success) {
		qWarning() << "发送北斗测试指令失败：" << err.description();
	}
}

void SelfCheckMainDialog_533::on_beidouGetSignalQualityBtnIn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::BeidouComm,
		CommandCode::BeidouComm_GetSignalQuality, params,
		WorkPhase::ConnectivityTest,
		CommunicationChannel::BDS,
		CommandType::Internal);
}

void SelfCheckMainDialog_533::on_beidouGetIdentityBtnIn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::BeidouComm,
		CommandCode::BeidouComm_GetIdentity, params,
		WorkPhase::ConnectivityTest,
		CommunicationChannel::BDS, 
		CommandType::Internal);
}

void SelfCheckMainDialog_533::on_beidouGetTargetBtnIn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::BeidouComm,
		CommandCode::BeidouComm_GetTarget, params,
		WorkPhase::ConnectivityTest,
		CommunicationChannel::BDS,
		CommandType::Internal);
}

void SelfCheckMainDialog_533::on_beidouSetTargetBtnIn_clicked()
{
	
	if (!m_module) return;
	QString cardIdText = ui.le_09_SetTarget_Input_In->text();
	QString cardErrorMsg;
	if (!validateBeidouCardId(cardIdText, cardErrorMsg)) {
		QMessageBox::warning(this, "卡号输入错误", cardErrorMsg);
		return;
	}
	QString cardIdStr = cardIdText.trimmed().rightJustified(7, '0');
	uint32_t cardId = cardIdStr.toUInt();
	auto params = std::make_shared<BeidouCardNoParam>(cardId);
	m_module->sendCheckCommand(DeviceId::BeidouComm,
		CommandCode::BeidouComm_SetTarget, params,
		WorkPhase::ConnectivityTest,
		CommunicationChannel::BDS,
		CommandType::Internal);
}

void SelfCheckMainDialog_533::on_beidouGetWhitelistBtnIn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::BeidouComm,
		CommandCode::BeidouComm_GetWhitelist, params,
		WorkPhase::ConnectivityTest,
		CommunicationChannel::BDS,
		CommandType::Internal);
}

void SelfCheckMainDialog_533::on_beidouAddWhitelistBtnIn_clicked()
{
	if (!m_module) return;
	QString cardIdText = ui.le_09_AddWhitelist_Input_In->text();
	QString cardErrorMsg;
	if (!validateBeidouCardId(cardIdText, cardErrorMsg)) {
		QMessageBox::warning(this, "卡号输入错误", cardErrorMsg);
		return;
	}
	QString cardIdStr = cardIdText.trimmed().rightJustified(7, '0');
	uint32_t cardId = cardIdStr.toUInt();
	auto params = std::make_shared<BeidouCardNoParam>(cardId);
	m_module->sendCheckCommand(DeviceId::BeidouComm,
		CommandCode::BeidouComm_AddWhitelist, params,
		WorkPhase::ConnectivityTest,
		CommunicationChannel::BDS,
		CommandType::Internal);
}

void SelfCheckMainDialog_533::on_beidouDelWhitelistBtnIn_clicked()
{
	if (!m_module) return;
	QString cardIdText = ui.le_09_DelWhitelist_Input_In->text();
	QString cardErrorMsg;
	if (!validateBeidouCardId(cardIdText, cardErrorMsg)) {
		QMessageBox::warning(this, "卡号输入错误", cardErrorMsg);
		return;
	}
	QString cardIdStr = cardIdText.trimmed().rightJustified(7, '0');
	uint32_t cardId = cardIdStr.toUInt();
	auto params = std::make_shared<BeidouCardNoParam>(cardId);
	m_module->sendCheckCommand(DeviceId::BeidouComm,
		CommandCode::BeidouComm_DelWhitelist, params,
		WorkPhase::ConnectivityTest,
		CommunicationChannel::BDS,
		CommandType::Internal);
}

void SelfCheckMainDialog_533::on_beidouGetPositionBtnIn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::BeidouComm,
		CommandCode::BeidouComm_GetPosition, params,
		WorkPhase::ConnectivityTest,
		CommunicationChannel::BDS,
		CommandType::Internal);
}

void SelfCheckMainDialog_533::on_beidouGetSysInfoBtnIn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::BeidouComm,
		CommandCode::BeidouComm_GetSysInfo, params, 
		WorkPhase::ConnectivityTest,
		CommunicationChannel::BDS,
		CommandType::Internal);
}

void SelfCheckMainDialog_533::on_beidouFactoryResetBtnIn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::BeidouComm,
		CommandCode::BeidouComm_FactoryReset, params, 
		WorkPhase::ConnectivityTest,
		CommunicationChannel::BDS,
		CommandType::Internal);
}

void SelfCheckMainDialog_533::on_beidouRebootBtnIn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::BeidouComm,
		CommandCode::BeidouComm_Reboot, params,
		WorkPhase::ConnectivityTest,
		CommunicationChannel::BDS,
		CommandType::Internal);
}

void SelfCheckMainDialog_533::on_dropWeightPowerOnBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::DropWeight,
		CommandCode::Common_PowerOn, params);
}

void SelfCheckMainDialog_533::on_dropWeightPowerOffBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::DropWeight,
		CommandCode::Common_PowerOff, params);
}

void SelfCheckMainDialog_533::on_dropWeighReleaseBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::DropWeight,
		CommandCode::DropWeight_Release, params);
}

void SelfCheckMainDialog_533::on_dropWeightResetBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::DropWeight,
		CommandCode::DropWeight_Reset, params);
}

void SelfCheckMainDialog_533::on_dropWeightGetFaultBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::DropWeight,
		CommandCode::Common_GetFault, params);
}

void SelfCheckMainDialog_533::on_acousticCommPowerOn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::AcousticComm,
		CommandCode::Common_PowerOn, params, WorkPhase::ConnectivityTest);
}

void SelfCheckMainDialog_533::on_acousticCommPowerOff_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::AcousticComm,
		CommandCode::Common_PowerOff, params, WorkPhase::ConnectivityTest);
}

void SelfCheckMainDialog_533::on_acousticCommSelfCheck_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::AcousticComm,
		CommandCode::AcousticComm_SelfCheck, params ,WorkPhase::ConnectivityTest);
}

void SelfCheckMainDialog_533::on_acousticCommtTest_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::AcousticComm,
		CommandCode::AcousticComm_Test, params,
		WorkPhase::ConnectivityTest,
		CommunicationChannel::WaterAcoustic);
}

void SelfCheckMainDialog_533::on_dvlPowerOnBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::DVL,
		CommandCode::Common_PowerOn, params);
}

void SelfCheckMainDialog_533::on_dvlPowerOffBtn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::DVL,
		CommandCode::Common_PowerOff, params);
}

void SelfCheckMainDialog_533::on_dvlGetBottomSpeed_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::DVL,
		CommandCode::DVL_GetBottomSpeed, params);
}

void SelfCheckMainDialog_533::on_dvlGetWaterSpeed_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::DVL,
		CommandCode::DVL_GetWaterSpeed, params);
}

void SelfCheckMainDialog_533::on_dvlGetStatus_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::DVL,
		CommandCode::DVL_GetStatus, params);
}

void SelfCheckMainDialog_533::on_powerBattery1GetSoc_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::PowerBattery1,
		CommandCode::Battery_GetSoc, params);
}

void SelfCheckMainDialog_533::on_powerBattery1GetTotalVoltage_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::PowerBattery1,
		CommandCode::Battery_GetTotalVoltage, params);
}

void SelfCheckMainDialog_533::on_powerBattery1GetCurrent_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::PowerBattery1,
		CommandCode::Battery_GetCurrent, params);
}

void SelfCheckMainDialog_533::on_powerBattery1GetAvgTemp_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::PowerBattery1,
		CommandCode::Battery_GetAvgTemp, params);
}

void SelfCheckMainDialog_533::on_meterBatteryGetSoc_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::MeterBattery,
		CommandCode::Battery_GetSoc, params);
}

void SelfCheckMainDialog_533::on_meterBatteryGetTotalVoltage_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::MeterBattery,
		CommandCode::Battery_GetTotalVoltage, params);
}

void SelfCheckMainDialog_533::on_meterBatteryGetCurrent_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::MeterBattery,
		CommandCode::Battery_GetCurrent, params);
}

void SelfCheckMainDialog_533::on_meterBatteryGetAvgTemp_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::MeterBattery,
		CommandCode::Battery_GetAvgTemp, params);
}

void SelfCheckMainDialog_533::on_meterBatteryHVOn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::MeterBattery,
		CommandCode::Battery_HVOn, params);
}

void SelfCheckMainDialog_533::on_meterBatteryHVOff_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::MeterBattery,
		CommandCode::Battery_HVOff, params);
}

void SelfCheckMainDialog_533::on_meterBatteryLock_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::MeterBattery,
		CommandCode::Battery_Lock, params);
}

void SelfCheckMainDialog_533::on_meterBatteryUnlock_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::MeterBattery,
		CommandCode::Battery_Unlock, params);
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

void SelfCheckMainDialog_533::on_powerBattery1HVOn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::PowerBattery1,
		CommandCode::Battery_HVOn, params);
}

void SelfCheckMainDialog_533::on_powerBattery1HVOff_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::PowerBattery1,
		CommandCode::Battery_HVOff, params);
}

void SelfCheckMainDialog_533::on_powerBattery1Lock_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::PowerBattery1,
		CommandCode::Battery_Lock, params);
}

void SelfCheckMainDialog_533::on_powerBattery1Unlock_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::PowerBattery1,
		CommandCode::Battery_Unlock, params);
}

void SelfCheckMainDialog_533::on_powerBattery2GetSoc_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::PowerBattery2,
		CommandCode::Battery_GetSoc, params);
}

void SelfCheckMainDialog_533::on_powerBattery2GetTotalVoltage_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::PowerBattery2,
		CommandCode::Battery_GetTotalVoltage, params);
}

void SelfCheckMainDialog_533::on_powerBattery2GetCurrent_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::PowerBattery2,
		CommandCode::Battery_GetCurrent, params);
}

void SelfCheckMainDialog_533::on_powerBattery2GetAvgTemp_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::PowerBattery2,
		CommandCode::Battery_GetAvgTemp, params);
}

void SelfCheckMainDialog_533::on_powerBattery2HVOn_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::PowerBattery2,
		CommandCode::Battery_HVOn, params);
}

void SelfCheckMainDialog_533::on_powerBattery2HVOff_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::PowerBattery2,
		CommandCode::Battery_HVOff, params);
}

void SelfCheckMainDialog_533::on_powerBattery2Lock_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::PowerBattery2,
		CommandCode::Battery_Lock, params);
}

void SelfCheckMainDialog_533::on_powerBattery2Unlock_clicked()
{
	auto params = std::make_shared<EmptyParameter>();
	m_module->sendCheckCommand(DeviceId::PowerBattery2,
		CommandCode::Battery_Unlock, params);
}
