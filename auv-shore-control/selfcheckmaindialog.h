#pragma once

#include <QDialog>
#include "ui_selfcheckmaindialog.h"
#include"devicepage.h"
#include"moduledialogbase.h"
#include"selfcheckmodule.h"
#include"baseresult.h"
class SelfCheckMainDialog_533 : public ModuleDialogBase,IResultObserver
{
	Q_OBJECT

public:
	SelfCheckMainDialog_533(SelfCheckModule* module, QWidget *parent = nullptr);
	~SelfCheckMainDialog_533() override;
	void onResultUpdated(const BaseResult* result) override;
private:
	void initialize()override;
	void setTreeWidgetItemData(QTreeWidget* treeWidget);
	void handleSendResult(const SelfCheckSendResult* result);
	void handleSelfCheckResult(const SelfCheckResult* result);

	void updateWidgetText(QWidget* widget, const QString& text, const QColor& color = Qt::black);
	QPixmap getStatusIcon(ResultStatus status);

	//设置状态图标
	void setIcon(QLabel* label, const QPixmap& icon);


	//处理不同器件返回自检结果
	void processThrusterCommand(const QString& number, CommandCode cmdCode,
		const SelfCheckResult* checkResult, const QPixmap& icon);
	void processRudderCommand(const QString& number, CommandCode cmdCode,
		const SelfCheckResult* checkResult, const QPixmap& icon);

	void handleThrusterResult(DeviceId deviceId, CommandCode cmdCode,
		const SelfCheckResult* checkResult);
	void handleRudderResult(DeviceId deviceId, CommandCode cmdCode,
		const SelfCheckResult* checkResult);

	void handleDropWeightResult(const SelfCheckResult* checkResult);
	void handleDepthMeterResult(const SelfCheckResult* checkResult);
	void handleAltimeterResult(const SelfCheckResult* checkResult);
	void handleUsblResult(const SelfCheckResult* checkResult);
	void handleInsResult(const SelfCheckResult* checkResult);
	void handleDvlResult(const SelfCheckResult* checkResult);
	void handleCameraResult(const SelfCheckResult* checkResult);
	void handleBuoyancyResult(DeviceId deviceId, CommandCode cmdCode,
		const SelfCheckResult* checkResult);
	void handleBatteryResult(DeviceId deviceId, CommandCode cmdCode,
		const SelfCheckResult* checkResult);
	void handleSonarResult(DeviceId deviceId, CommandCode cmdCode,
		const SelfCheckResult* checkResult);
	void handleAcousticCommResult(const SelfCheckResult* checkResult);
	void handleBeidouCommResult(const SelfCheckResult* checkResult);
	// ========== 保留原有7位卡号工具函数 ==========
	QString formatTo7DigitCard(uint32_t cardNum);
	QStringList filter7DigitCard(const QStringList& cardList);
	// 获取推进器对应的控件编号（如TailThruster1 → "02"）
	QString getThrusterNumber(DeviceId deviceId);

	QString getRudderNumber(DeviceId deviceId);

	QString getBatteryNumber(DeviceId deviceId);

	QString getSonarNumber(DeviceId deviceId);

	bool validateBeidouCardId(const QString& cardIdText, 
		QString& errorMsg, bool autoPadZero = true);

	//  通用更新状态文本函数（核心封装）
	void updateStatusWidget(QWidget* widget, ResultStatus status, const QString& customText = "");

private slots:
	void on_missionTreeWidget_currentItemChanged(QTreeWidgetItem* currentItem,QTreeWidgetItem * previous);


	//深度计
	void on_depMetPowerOnBtn_clicked();
	void on_depMetPowerOffBtn_clicked();
	void on_depMetCalibrateBtn_clicked();
	void on_depMetGetDepthBtn_clicked();

	//高度计
	void on_altimeterPowerOnBtn_clicked();
	void on_altimeterPowerOffBtn_clicked();
	void on_altimeterHeightBtn_clicked();

	//摄像机
	void on_cameraPowerOnBtn_clicked();
	void on_cameraPowerOffBtn_clicked();
	void on_cameraStartRecordBtn_clicked();
	void on_cameraStopRecordBtn_clicked();
	void on_cameraTakePhotoBtn_clicked();
	
	//USBL
	void on_usblPowerOnBtn_clicked();
	void on_usblPowerOffBtn_clicked();

	//惯导
	void on_insPowerOnBtn_clicked();
	void on_insPowerOffBtn_clicked();
	void on_insSetGnssBindBtn_clicked();
	void on_insCalibrationWithDVLBtn_clicked();
	void on_insCombinedWithDVLBtn_clicked();
	void on_insCombinedAutoBtn_clicked();
	void on_insGetUtcDateBtn_clicked();
	void on_insGetUtcTimeBtn_clicked();
	void on_insGetAttitudeBtn_clicked();
	void on_insGetBodySpeedBtn_clicked();
	void on_insGetGeoSpeedBtn_clicked();
	void on_insGetPositionBtn_clicked();
	void on_insGetGyroscopeSpeedBtn_clicked();
	void on_insGetAccelerationBtn_clicked();
	void on_insGetStatusBtn_clicked();

	//浮调
	void on_buoyancyAdjustingPowerOnBtn_clicked();
	void on_buoyancyAdjustingPowerOffBtn_clicked();
	void on_buoyancyAdjustingGetbuoyancyValue_clicked();
	void on_buoyancyAdjustingSetbuoyancyValue_clicked();
	void on_buoyancyAdjustingStop_clicked();

	//前视声呐
	void on_forwardSonarPowerOnBtn_clicked();
	void on_forwardSonarPowerOffBtn_clicked();
	void on_forwardSonarEnabelBtn_clicked();
	void on_forwardSonarDisableBtn_clicked();

	//下视声呐
	void on_downwardSonarPowerOnBtn_clicked();
	void on_downwardSonarPowerOffBtn_clicked();
	void on_downwardSonarEnabelBtn_clicked();
	void on_downwardSonarDisableBtn_clicked();

	//前视声呐
	void on_sideScanSonarPowerOnBtn_clicked();
	void on_sideScanSonarPowerOffBtn_clicked();
	void on_sideScanSonarEnabelBtn_clicked();
	void on_sideScanSonarDisableBtn_clicked();



	//推进器
	void on_tailThrusterPowerOnBtn_clicked();
	void on_tailThrusterPowerOffBtn_clicked();
	void on_tailThrusterParamResetBtn_clicked();
	void on_tailThrusterEnableWorkBtn_clicked();
	void on_tailThrusterSetSpeedBtn_clicked();
	void on_tailThrusterSetDutyBtn_clicked();
	void on_tailThrusterSetRunTimeBtn_clicked();
	void on_tailThrusterSetAccelTimeBtn_clicked();
	void on_tailThrusterGetRunTimeBtn_clicked();
	void on_tailThrusterGetDutyBtn_clicked();
	void on_tailThrusterGetCurrentBtn_clicked();
	void on_tailThrusterGetTemperatureBtn_clicked();
	void on_tailThrusterGetSpeedBtn_clicked();
	void on_tailThrusterGetHallBtn_clicked();
	void on_tailThrusterGetBusVoltageBtn_clicked();
	void on_tailThrusterGetNoMaintainTimeBtn_clicked();
	void on_tailThrusterGetTotalRunTimeBtn_clicked();
	void on_tailThrusterSetSpeedWithTimeBtn_clicked();

	//天线折叠机构
	void on_antennaFoldPowerOnBtn_clicked();
	void on_antennaFoldPowerOffBtn_clicked();
	void on_antennaFoldSetAngle_clicked();
	void on_antennaFoldGetRunTime_clicked();
	void on_antennaFoldGetVoltage_clicked();
	void on_antennaFoldGetAngle_clicked();
	void on_antennaFoldGetCurrent_clicked();
	void on_antennaFoldGetTemp_clicked();
	void on_antennaFoldGetFault_clicked();

	//舵机
	void on_rudder1PowerOnBtn_clicked();
	void on_rudder1PowerOffBtn_clicked();
	void on_rudder1SetAngleBtn_clicked();
	void on_rudder1GetRunTimeBtn_clicked();
	void on_rudder1GetVoltageBtn_clicked();
	void on_rudder1GetAngleBtn_clicked();
	void on_rudder1GetCurrentBtn_clicked();
	void on_rudder1GetTempBtn_clicked();
    void on_rudder1SetZeroBtn_clicked();
	void on_rudder1GetFaultBtn_clicked();
	

	void on_rudder2PowerOnBtn_clicked();
	void on_rudder2PowerOffBtn_clicked();
	void on_rudder2SetAngleBtn_clicked();
	void on_rudder2GetRunTimeBtn_clicked();
	void on_rudder2GetVoltageBtn_clicked();
	void on_rudder2GetAngleBtn_clicked();
	void on_rudder2GetCurrentBtn_clicked();
	void on_rudder2GetTempBtn_clicked();
	void on_rudder2SetZeroBtn_clicked();
	void on_rudder2GetFaultBtn_clicked();

	void on_rudder3PowerOnBtn_clicked();
	void on_rudder3PowerOffBtn_clicked();
	void on_rudder3SetAngleBtn_clicked();
	void on_rudder3GetRunTimeBtn_clicked();
	void on_rudder3GetVoltageBtn_clicked();
	void on_rudder3GetAngleBtn_clicked();
	void on_rudder3GetCurrentBtn_clicked();
	void on_rudder3GetTempBtn_clicked();
	void on_rudder3SetZeroBtn_clicked();
	void on_rudder3GetFaultBtn_clicked();

	void on_rudder4PowerOnBtn_clicked();
	void on_rudder4PowerOffBtn_clicked();
	void on_rudder4SetAngleBtn_clicked();
	void on_rudder4GetRunTimeBtn_clicked();
	void on_rudder4GetVoltageBtn_clicked();
	void on_rudder4GetAngleBtn_clicked();
	void on_rudder4GetCurrentBtn_clicked();
	void on_rudder4GetTempBtn_clicked();
	void on_rudder4SetZeroBtn_clicked();
	void on_rudder4GetFaultBtn_clicked();

	//北斗
	void on_beidouGetSignalQualityBtn_clicked();
	void on_beidouGetIdentityBtn_clicked();
	void on_beidouGetTargetBtn_clicked();
	void on_beidouSetTargetBtn_clicked();
	void on_beidouGetWhitelistBtn_clicked();
	void on_beidouAddWhitelistBtn_clicked();
	void on_beidouDelWhitelistBtn_clicked();
	void on_beidouGetPositionBtn_clicked();
	void on_beidouGetSysInfoBtn_clicked();
	void on_beidouFactoryResetBtn_clicked();
	void on_beidouRebootBtn_clicked();
	void on_beidouCommTestBtn_clicked();

	void on_beidouGetSignalQualityBtnIn_clicked();
	void on_beidouGetIdentityBtnIn_clicked();
	void on_beidouGetTargetBtnIn_clicked();
	void on_beidouSetTargetBtnIn_clicked();
	void on_beidouGetWhitelistBtnIn_clicked();
	void on_beidouAddWhitelistBtnIn_clicked();
	void on_beidouDelWhitelistBtnIn_clicked();
	void on_beidouGetPositionBtnIn_clicked();
	void on_beidouGetSysInfoBtnIn_clicked();
	void on_beidouFactoryResetBtnIn_clicked();
	void on_beidouRebootBtnIn_clicked();
	//抛载
	void on_dropWeightPowerOnBtn_clicked();
	void on_dropWeightPowerOffBtn_clicked();
	void on_dropWeighReleaseBtn_clicked();
	void on_dropWeightResetBtn_clicked();
	void on_dropWeightGetFaultBtn_clicked();

	//声通
	void on_acousticCommPowerOn_clicked();
	void on_acousticCommPowerOff_clicked();
	void on_acousticCommSelfCheck_clicked();
	void on_acousticCommtTest_clicked();

	//dvl
	void on_dvlPowerOnBtn_clicked();
	void on_dvlPowerOffBtn_clicked();
	void on_dvlGetBottomSpeed_clicked();
	void on_dvlGetWaterSpeed_clicked();
	void on_dvlGetStatus_clicked();

	//电池
	void on_powerBattery1GetSoc_clicked();
	void on_powerBattery1GetTotalVoltage_clicked();
	void on_powerBattery1GetCurrent_clicked();
	void on_powerBattery1GetAvgTemp_clicked();
	void on_powerBattery1HVOn_clicked();
	void on_powerBattery1HVOff_clicked();
	void on_powerBattery1Lock_clicked();
	void on_powerBattery1Unlock_clicked();

	void on_powerBattery2GetSoc_clicked();
	void on_powerBattery2GetTotalVoltage_clicked();
	void on_powerBattery2GetCurrent_clicked();
	void on_powerBattery2GetAvgTemp_clicked();
	void on_powerBattery2HVOn_clicked();
	void on_powerBattery2HVOff_clicked();
	void on_powerBattery2Lock_clicked();
	void on_powerBattery2Unlock_clicked();

	void on_meterBatteryGetSoc_clicked();
	void on_meterBatteryGetTotalVoltage_clicked();
	void on_meterBatteryGetCurrent_clicked();
	void on_meterBatteryGetAvgTemp_clicked();
	void on_meterBatteryHVOn_clicked();
	void on_meterBatteryHVOff_clicked();
	void on_meterBatteryLock_clicked();
	void on_meterBatteryUnlock_clicked();
private:
	Ui::SelfCheckMainDialogClass ui;
	DevicePage m_currentDevicePage;
	SelfCheckModule* m_module;
	const QString STYLE_SELFCHECK = "./qss/selfcheckdlg.qss";
	const QString COMMON_ICON = "./icon/selfcheck/";
	// 图标路径常量（避免硬编码）
	const QString ICON_SUCCESS = COMMON_ICON + "success.png";
	const QString ICON_FAIL = COMMON_ICON + "fail.png";
	const QString ICON_WAIT = COMMON_ICON + "wait.png";
	QMap<DevicePage, int> m_pageIndexMap; // 声明为成员变量，存储页面映射关系
		// 封装状态映射配置（统一管理文本+颜色）
	const QMap<ResultStatus, QPair<QString, QColor>> m_statusConfig = {
		{ResultStatus::Success,  {"成功", Qt::green}},
		{ResultStatus::Pending,  {"等待中", Qt::yellow}},
		{ResultStatus::Failed,   {"失败", Qt::red}},
		{ResultStatus::Timeout,  {"超时", QColor(255, 165, 0)}}, // 橙色（Qt无内置，用RGB）
		{ResultStatus::Unknown,  {"未知状态", Qt::gray}}        // 兜底防崩溃
	};
};
