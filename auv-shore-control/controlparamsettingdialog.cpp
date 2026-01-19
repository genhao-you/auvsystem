#include "controlparamsettingdialog.h"
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QToolTip>
ControlParamSettingDialog::ControlParamSettingDialog(ControlParamModule* module, QWidget *parent)
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

ControlParamSettingDialog::~ControlParamSettingDialog()
{

}

void ControlParamSettingDialog::onResultUpdated(const BaseResult * result)
{
	// 区分发送结果和控制结果
	const ControlParamSendResult* sendResult = dynamic_cast<const ControlParamSendResult*>(result);
	if (sendResult) { handleSendResult(sendResult); return; }

	const ControlParamResult* controlResult = dynamic_cast<const ControlParamResult*>(result);
	if (controlResult) { handleControlResult(controlResult); return; }
}

void ControlParamSettingDialog::initialize()
{
	m_module->addObserver(this);
	// 加载QSS样式
	loadStyleSheet(STYLE_CONTROLDEBUGGING);

	// ---------------------- 初始化深度参数SpinBox映射 ----------------------
	m_depthSpinBoxes["deep_pid_kp"] =ui.dsb_deep_pid_kp;
	m_depthSpinBoxes["deep_pid_ki"] =ui.dsb_deep_pid_ki;
	m_depthSpinBoxes["deep_pid_kd"] =ui.dsb_deep_pid_kd;
	m_depthSpinBoxes["deep_sita_pid_kp"] =ui.dsb_deep_sita_pid_kp;
	m_depthSpinBoxes["deep_sita_pid_ki"] =ui.dsb_deep_sita_pid_ki;
	m_depthSpinBoxes["deep_sita_pid_kd"] =ui.dsb_deep_sita_pid_kd;
	m_depthSpinBoxes["sita_xf_max"] =ui.dsb_sita_xf_max;
	m_depthSpinBoxes["sita_xf_min"] =ui.dsb_sita_xf_min;
	m_depthSpinBoxes["dleta_xf_max"] =ui.dsb_dleta_xf_max;
	m_depthSpinBoxes["dleta_xf_min"] =ui.dsb_dleta_xf_min;
	m_depthSpinBoxes["dynamic_sp_dleta_number"] =ui.dsb_dynamic_sp_dleta_number;
	m_depthSpinBoxes["dleta_sp_angle"] =ui.dsb_dleta_sp_angle;
	m_depthSpinBoxes["deep_dleta_rate_max"] =ui.dsb_deep_dleta_rate_max;
	m_depthSpinBoxes["deep_dleta_rate_min"] =ui.dsb_deep_dleta_rate_min;
	m_depthSpinBoxes["deep_lb_number"] =ui.dsb_deep_lb_number;
	m_depthSpinBoxes["deep_sita_lb_number"] =ui.dsb_deep_sita_lb_number;
	m_depthSpinBoxes["dleta_sp_rate_lb_number"] =ui.dsb_dleta_sp_rate_lb_number;
	m_depthSpinBoxes["deep_control_period"] =ui.dsb_deep_control_period;

	// ---------------------- 初始化高度参数SpinBox映射 ----------------------
	m_heightSpinBoxes["high_pid_kp"] =ui.dsb_high_pid_kp;
	m_heightSpinBoxes["high_pid_ki"] =ui.dsb_high_pid_ki;
	m_heightSpinBoxes["high_pid_kd"] =ui.dsb_high_pid_kd;
	m_heightSpinBoxes["high_sita_kp"] =ui.dsb_high_sita_kp;
	m_heightSpinBoxes["high_sita_ki"] =ui.dsb_high_sita_ki;
	m_heightSpinBoxes["high_sita_kd"] =ui.dsb_high_sita_kd;
	m_heightSpinBoxes["high_sita_max"] =ui.dsb_high_sita_max;
	m_heightSpinBoxes["high_sita_min"] =ui.dsb_high_sita_min;
	m_heightSpinBoxes["high_dleta_max"] =ui.dsb_high_dleta_max;
	m_heightSpinBoxes["high_dleta_min"] =ui.dsb_high_dleta_min;
	m_heightSpinBoxes["dynamic_sp_dleta_number_h"] =ui.dsb_dynamic_sp_dleta_number_h;
	m_heightSpinBoxes["dleta_sp_angle_h"] =ui.dsb_dleta_sp_angle_h;
	m_heightSpinBoxes["dleta_sp_rate_max_h"] =ui.dsb_dleta_sp_rate_max_h;
	m_heightSpinBoxes["dleta_sp_rate_min_h"] =ui.dsb_dleta_sp_rate_min_h;
	m_heightSpinBoxes["act_h_lb_number"] =ui.dsb_act_h_lb_number;
	m_heightSpinBoxes["act_h_sita_lb_number"] =ui.dsb_act_h_sita_lb_number;
	m_heightSpinBoxes["high_dleta_rate_lb_number"] =ui.dsb_high_dleta_rate_lb_number;
	m_heightSpinBoxes["high_control_period"] =ui.dsb_high_control_period;

	// ---------------------- 初始化航向参数SpinBox映射 ----------------------
	m_headingSpinBoxes["hx_pid_kp"] =ui.dsb_hx_pid_kp;
	m_headingSpinBoxes["hx_pid_ki"] =ui.dsb_hx_pid_ki;
	m_headingSpinBoxes["hx_pid_kd"] =ui.dsb_hx_pid_kd;
	m_headingSpinBoxes["cz_dleta_max"] =ui.dsb_cz_dleta_max;
	m_headingSpinBoxes["cz_dleta_min"] =ui.dsb_cz_dleta_min;
	m_headingSpinBoxes["dynamic_cz_dleta_number"] =ui.dsb_dynamic_cz_dleta_number;
	m_headingSpinBoxes["dleta_cz_angle"] =ui.dsb_dleta_cz_angle;
	m_headingSpinBoxes["dleta_cz_rate_max"] =ui.dsb_dleta_cz_rate_max;
	m_headingSpinBoxes["dleta_cz_rate_min"] =ui.dsb_dleta_cz_rate_min;
	m_headingSpinBoxes["circle_r"] =ui.dsb_circle_r;
	m_headingSpinBoxes["psi_lb_number"] =ui.dsb_psi_lb_number;
	m_headingSpinBoxes["dleta_cz_lb_number"] =ui.dsb_dleta_cz_lb_number;
	m_headingSpinBoxes["hx_control_period"] =ui.dsb_hx_control_period;

	// ---------------------- 关联按钮点击信号 ----------------------
	connect(ui.btn_SendDepthParam, &QPushButton::clicked, this, &ControlParamSettingDialog::on_btn_SendDepthParam_clicked);
	connect(ui.btn_SendHeightParam, &QPushButton::clicked, this, &ControlParamSettingDialog::on_btn_SendHeightParam_clicked);
	connect(ui.btn_SendHeadingParam, &QPushButton::clicked, this, &ControlParamSettingDialog::on_btn_SendHeadingParam_clicked);
}

void ControlParamSettingDialog::handleSendResult(const ControlParamSendResult* result)
{
	QString timeStr = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
	QString typeStr;
	QLineEdit* statusLineEdit = nullptr;
	QLabel* iconLabel = nullptr;

	// 根据控制类型匹配对应的状态标签
	switch (result->controlType()) {
	case ControlType::DepthControl:
		typeStr = "深度控制";
		statusLineEdit = ui.le_DepthControl_Feedback;
		iconLabel = ui.lbl_DepthIcon;
		break;
	case ControlType::HeightControl:
		typeStr = "高度控制";
		statusLineEdit = ui.le_HeightControl_Feedback;
		iconLabel = ui.lbl_HeightIcon;
		break;
	case ControlType::HeadingControl:
		typeStr = "航向控制";
		statusLineEdit = ui.le_HeadingControl_Feedback;
		iconLabel = ui.lbl_HeadingIcon;
		break;
	default:
		typeStr = "未知控制类型";
		break;
	}

	// 显示ToolTip提示
	QToolTip::showText(this->mapToGlobal(QPoint(100, 100)),
		QString("【%1】%2 命令%3：%4（包序号：%5）")
		.arg(timeStr, typeStr, result->isSuccess() ? "发送成功" : "发送失败",
			result->message(), QString::number(result->packetSeq())),
		this, QRect(), 3000); // 3秒后消失

	// 更新界面状态标签
	/*if (statusLabel && iconLabel) {
		updateSendStatusLabel(statusLabel, iconLabel, result->isSuccess(),
			result->isSuccess() ? "发送成功" : "发送失败：" + result->message());
	}*/
}

void ControlParamSettingDialog::handleControlResult(const ControlParamResult* result)
{
	QString timeStr = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
	QString typeStr;
	QLineEdit* statusLineEdit = nullptr;
	QLabel* iconLabel = nullptr;

	// 根据控制类型匹配对应的状态标签
	switch (result->controlType()) {
	case ControlType::DepthControl:
		typeStr = "深度控制";
		statusLineEdit = ui.le_DepthControl_Feedback;
		iconLabel = ui.lbl_DepthIcon;
		break;
	case ControlType::HeightControl:
		typeStr = "高度控制";
		statusLineEdit = ui.le_HeightControl_Feedback;
		iconLabel = ui.lbl_HeightIcon;
		break;
	case ControlType::HeadingControl:
		typeStr = "航向控制";
		statusLineEdit = ui.le_HeadingControl_Feedback;
		iconLabel = ui.lbl_HeadingIcon;
		break;
	default:
		typeStr = "未知控制类型";
		break;
	}

	QString statusStr;
	QColor color;
	switch (result->status()) {
	case ResultStatus::Success:
		statusStr = "执行成功";
		color = Qt::green;
		break;
	case ResultStatus::Failed:
		statusStr = "执行失败";
		color = Qt::red;
		break;
	case ResultStatus::Timeout:
		statusStr = "执行超时";
		color = QColorConstants::Svg::orange;
		break;
	case ResultStatus::Pending:
		statusStr = "待执行";
		color = Qt::gray;
		break;
	default:
		statusStr = "未知状态";
		color = Qt::black;
		break;
	}

	// 恢复并补全文本更新逻辑（假设ui.txtResult是结果显示的PlainTextEdit，若UI中无此控件可创建或调整）
	// 注：若UI中没有txtResult，需在UI里添加QPlainTextEdit并命名为txtResult，或注释此行后调整显示逻辑
	//updateWidgetText(ui.txtResult,
	//	QString("【%1】%2 %3：%4（包序号：%5）")
	//	.arg(timeStr, typeStr, statusStr, result->feedbackDesc(), QString::number(result->packetSeq())),
	//	color);

	// 更新状态标签
	/*if (statusLabel && iconLabel) {
		updateSendStatusLabel(statusLabel, iconLabel, result->status() == ResultStatus::Success, statusStr);
	}*/
}

void ControlParamSettingDialog::updateWidgetText(QWidget* widget, const QString& text, const QColor& color)
{
	QPlainTextEdit* edit = qobject_cast<QPlainTextEdit*>(widget);
	if (edit) {
		QTextCursor cursor = edit->textCursor();
		cursor.movePosition(QTextCursor::End);
		QTextCharFormat format;
		format.setForeground(color);
		cursor.insertText(text + "\n", format);
		edit->setTextCursor(cursor);
		// 滚动到末尾
		edit->ensureCursorVisible();
		return;
	}

	// 兼容QLabel的情况
	QLabel* label = qobject_cast<QLabel*>(widget);
	if (label) {
		label->setText(text);
		label->setStyleSheet(QString("color: %1;").arg(color.name()));
		return;
	}
}

ControlParams ControlParamSettingDialog::buildControlParamsFromUI()
{
	// 1. 初始化ControlParams
	ControlParams params;
	int currentTab = ui.tabWidget->currentIndex();

	// 2. 根据当前标签页读取对应参数
	switch (currentTab) {
	case 0: // 深度控制标签页
		params.type = ControlType::DepthControl;
		// 读取所有深度参数
		params.depth.deep_pid_kp = m_depthSpinBoxes["deep_pid_kp"]->value();
		params.depth.deep_pid_ki = m_depthSpinBoxes["deep_pid_ki"]->value();
		params.depth.deep_pid_kd = m_depthSpinBoxes["deep_pid_kd"]->value();
		params.depth.deep_sita_pid_kp = m_depthSpinBoxes["deep_sita_pid_kp"]->value();
		params.depth.deep_sita_pid_ki = m_depthSpinBoxes["deep_sita_pid_ki"]->value();
		params.depth.deep_sita_pid_kd = m_depthSpinBoxes["deep_sita_pid_kd"]->value();
		params.depth.sita_xf_max = m_depthSpinBoxes["sita_xf_max"]->value();
		params.depth.sita_xf_min = m_depthSpinBoxes["sita_xf_min"]->value();
		params.depth.dleta_xf_max = m_depthSpinBoxes["dleta_xf_max"]->value();
		params.depth.dleta_xf_min = m_depthSpinBoxes["dleta_xf_min"]->value();
		params.depth.dynamic_sp_dleta_number = m_depthSpinBoxes["dynamic_sp_dleta_number"]->value();
		params.depth.dleta_sp_angle = m_depthSpinBoxes["dleta_sp_angle"]->value();
		params.depth.deep_dleta_rate_max = m_depthSpinBoxes["deep_dleta_rate_max"]->value();
		params.depth.deep_dleta_rate_min = m_depthSpinBoxes["deep_dleta_rate_min"]->value();
		params.depth.deep_lb_number = m_depthSpinBoxes["deep_lb_number"]->value();
		params.depth.deep_sita_lb_number = m_depthSpinBoxes["deep_sita_lb_number"]->value();
		params.depth.dleta_sp_rate_lb_number = m_depthSpinBoxes["dleta_sp_rate_lb_number"]->value();
		params.depth.deep_control_period = m_depthSpinBoxes["deep_control_period"]->value();
		break;

	case 1: // 高度控制标签页
		params.type = ControlType::HeightControl;
		// 读取所有高度参数
		params.height.high_pid_kp = m_heightSpinBoxes["high_pid_kp"]->value();
		params.height.high_pid_ki = m_heightSpinBoxes["high_pid_ki"]->value();
		params.height.high_pid_kd = m_heightSpinBoxes["high_pid_kd"]->value();
		params.height.high_sita_kp = m_heightSpinBoxes["high_sita_kp"]->value();
		params.height.high_sita_ki = m_heightSpinBoxes["high_sita_ki"]->value();
		params.height.high_sita_kd = m_heightSpinBoxes["high_sita_kd"]->value();
		params.height.high_sita_max = m_heightSpinBoxes["high_sita_max"]->value();
		params.height.high_sita_min = m_heightSpinBoxes["high_sita_min"]->value();
		params.height.high_dleta_max = m_heightSpinBoxes["high_dleta_max"]->value();
		params.height.high_dleta_min = m_heightSpinBoxes["high_dleta_min"]->value();
		params.height.dynamic_sp_dleta_number_h = m_heightSpinBoxes["dynamic_sp_dleta_number_h"]->value();
		params.height.dleta_sp_angle_h = m_heightSpinBoxes["dleta_sp_angle_h"]->value();
		params.height.dleta_sp_rate_max_h = m_heightSpinBoxes["dleta_sp_rate_max_h"]->value();
		params.height.dleta_sp_rate_min_h = m_heightSpinBoxes["dleta_sp_rate_min_h"]->value();
		params.height.act_h_lb_number = m_heightSpinBoxes["act_h_lb_number"]->value();
		params.height.act_h_sita_lb_number = m_heightSpinBoxes["act_h_sita_lb_number"]->value();
		params.height.high_dleta_rate_lb_number = m_heightSpinBoxes["high_dleta_rate_lb_number"]->value();
		params.height.high_control_period = m_heightSpinBoxes["high_control_period"]->value();
		break;

	case 2: // 航向控制标签页
		params.type = ControlType::HeadingControl;
		// 读取所有航向参数
		params.heading.hx_pid_kp = m_headingSpinBoxes["hx_pid_kp"]->value();
		params.heading.hx_pid_ki = m_headingSpinBoxes["hx_pid_ki"]->value();
		params.heading.hx_pid_kd = m_headingSpinBoxes["hx_pid_kd"]->value();
		params.heading.cz_dleta_max = m_headingSpinBoxes["cz_dleta_max"]->value();
		params.heading.cz_dleta_min = m_headingSpinBoxes["cz_dleta_min"]->value();
		params.heading.dynamic_cz_dleta_number = m_headingSpinBoxes["dynamic_cz_dleta_number"]->value();
		params.heading.dleta_cz_angle = m_headingSpinBoxes["dleta_cz_angle"]->value();
		params.heading.dleta_cz_rate_max = m_headingSpinBoxes["dleta_cz_rate_max"]->value();
		params.heading.dleta_cz_rate_min = m_headingSpinBoxes["dleta_cz_rate_min"]->value();
		params.heading.circle_r = m_headingSpinBoxes["circle_r"]->value();
		params.heading.psi_lb_number = m_headingSpinBoxes["psi_lb_number"]->value();
		params.heading.dleta_cz_lb_number = m_headingSpinBoxes["dleta_cz_lb_number"]->value();
		params.heading.hx_control_period = m_headingSpinBoxes["hx_control_period"]->value();
		break;

	default:
		params.type = ControlType::UnknownControl;
		QMessageBox::warning(this, "警告", "未选择有效的控制类型标签页");
		break;
	}

	return params;
}
// ========== 深度参数发送按钮点击槽函数 ==========
void ControlParamSettingDialog::on_btn_SendDepthParam_clicked()
{
	// 切换到深度标签页
	ui.tabWidget->setCurrentIndex(0);

	// 构建深度参数
	ControlParams params = buildControlParamsFromUI();
	if (params.type != ControlType::DepthControl) {
		QMessageBox::warning(this, "警告", "无法获取深度控制参数");
		return;
	}

	// 调用模块发送参数
	if (m_module) {
		// 创建ControlParamParameter的智能指针
		auto paramPtr = std::make_shared<ControlParamParameter>(params);
		// 传入智能指针，并补充缺失的CommunicationChannel参数（根据实际场景选择，示例用默认值）
		m_module->sendControlParamCommand(
			ControlType::DepthControl,
			paramPtr,
			WorkPhase::ControlCommand
		);
	}
	else {
		QMessageBox::critical(this, "错误", "控制参数模块未初始化，无法发送参数");
	}
}
void ControlParamSettingDialog::on_btn_SendHeightParam_clicked()
{
}

void ControlParamSettingDialog::on_btn_SendHeadingParam_clicked()
{
}


