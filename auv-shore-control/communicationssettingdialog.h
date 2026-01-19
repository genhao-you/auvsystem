#pragma once

#include <QDialog>
#include "ui_communicationssettingdialog.h"
#include"datamessage.h"
#include"moduledialogbase.h"
#include"communicationmodule.h"
class CommunicationsSettingDialog : public ModuleDialogBase, IResultObserver
{
	Q_OBJECT
		enum class ConnectionState
	{
		Disconnected,
		Connecting,
		Connected
	};
public:
	CommunicationsSettingDialog(CommunicationModule * module,QWidget *parent = nullptr);
	~CommunicationsSettingDialog();
	void onResultUpdated(const BaseResult* result) override;
private:
	void initialize()override;
	void setConnectionState(QLabel* label , CommunicationChannelStatus state);
	void handleChannelStatusResult(const CommunicationChannelStatusResult* result);
private slots:
	void on_radioActivateBtn_clicked();
	void on_radioShutBtn_clicked();
	
	//有线网配置相应
	void on_wiredNetworkActivateBtn_clicked();
	void on_wiredNetworkShutBtn_clicked();

	//北斗卫星配置
	void on_bdsActivateBtn_clicked();
	void on_bdsShutBtn_clicked();

	//水声通信配置
	void on_waterAcousticActivateBtn_clicked();
	void on_waterAcousticShutBtn_clicked();
private:
	Ui::CommunicationsSettingDialogClass ui;
	CommunicationModule* m_communicationModel;
};
