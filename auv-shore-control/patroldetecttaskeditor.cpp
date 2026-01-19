#include "patroldetecttaskeditor.h"
#pragma execution_character_set("utf-8")
PatrolDetectTaskEditor::PatrolDetectTaskEditor(QWidget *parent)
	: ITaskEditor(parent)
{
	ui.setupUi(this);
	initUI();
}

PatrolDetectTaskEditor::~PatrolDetectTaskEditor()
{}

void PatrolDetectTaskEditor::setTask(Task * task)
{
}

Task* PatrolDetectTaskEditor::getTask() const
{
	return m_currentTask;
}

void PatrolDetectTaskEditor::clear()
{
}

void PatrolDetectTaskEditor::initUI()
{
	//结束时刻
	m_timeEditEnd = new QDateTimeEdit(this);
	ui.tb_patroDetectEnd->setCellWidget(3, 1, m_timeEditEnd);
	ui.tb_patroDetectEnd->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tb_patroDetectEnd->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	//预设结束复选框
	m_checkBoxIsEnd = new QCheckBox(this);
	ui.tb_patroDetectEnd->setCellWidget(0, 1, createCheckBox(m_checkBoxIsEnd, ui.tb_patroDetectEnd));
	m_comboBoxEndCondition = new QComboBox(this);
	m_comboBoxEndCondition->addItem("定时刻");
	m_comboBoxEndCondition->addItem("定时长");
	ui.tb_patroDetectEnd->setCellWidget(1, 1, m_comboBoxEndCondition);
	//z单位
	m_comboBoxZUnit = new QComboBox(this);
	m_comboBoxZUnit->addItem("定深");
	m_comboBoxZUnit->addItem("定高");
	ui.tb_patroDetectPosition->setCellWidget(1, 1, m_comboBoxZUnit);
	ui.tb_patroDetectPosition->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tb_patroDetectPosition->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	//速度单位
	m_comboBoxSpeedUnit = new QComboBox(this);
	m_comboBoxSpeedUnit->addItem("m/s");
	m_comboBoxSpeedUnit->addItem("kn");
	m_comboBoxSpeedUnit->addItem("km/s");
	m_comboBoxSpeedUnit->addItem("RPM");
	ui.tb_patroDetectPosition->setCellWidget(3, 1, m_comboBoxSpeedUnit);

	//游曳类型
	m_comboBoxPatrolType = new QComboBox(this);
	m_comboBoxPatrolType->addItem("外环游曳");
	m_comboBoxPatrolType->addItem("环形填充");
	m_comboBoxPatrolType->addItem("弓形填充");
	m_comboBoxPatrolType->addItem("手动填充");
	ui.tb_patroDetectTask->setCellWidget(1, 1, m_comboBoxPatrolType);
	ui.tb_patroDetectTask->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tb_patroDetectTask->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	//目标处理
	m_comboBoxTargetProcess = new QComboBox(this);
	m_comboBoxTargetProcess->addItem("主动打击");
	m_comboBoxTargetProcess->addItem("手动打击");
	m_comboBoxTargetProcess->addItem("不打击");
	ui.tb_patroDetectTask->setCellWidget(3, 1, m_comboBoxTargetProcess);

	//探测手段
	m_checkBoxGunwale = new QCheckBox(this);
	ui.tb_patroDetectWay->setCellWidget(0, 1, createCheckBox(m_checkBoxGunwale, ui.tb_patroDetectWay));
	m_checkBoxElectromagnetism = new QCheckBox(this);
	ui.tb_patroDetectWay->setCellWidget(1, 1, createCheckBox(m_checkBoxElectromagnetism, ui.tb_patroDetectWay));
	m_checkBoxGunwale = new QCheckBox(this);
	ui.tb_patroDetectWay->setCellWidget(2, 1, createCheckBox(m_checkBoxGunwale, ui.tb_patroDetectWay));
	m_checkBoxDrag = new QCheckBox(this);
	ui.tb_patroDetectWay->setCellWidget(3, 1, createCheckBox(m_checkBoxDrag, ui.tb_patroDetectWay));
	ui.tb_patroDetectWay->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tb_patroDetectWay->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
}

//QWidget* PatrolDetectTaskEditor::createCheckBox(QCheckBox* checkBox, QWidget* widget)
//{
//	QWidget* cbWidget = new QWidget(widget);
//	QHBoxLayout* hLayout = new QHBoxLayout(this);
//	hLayout->addWidget(checkBox);
//	hLayout->setMargin(0);
//	hLayout->setAlignment(checkBox, Qt::AlignCenter);//居中
//	hLayout->setMargin(0);
//	cbWidget->setLayout(hLayout);
//	return cbWidget;
//}
