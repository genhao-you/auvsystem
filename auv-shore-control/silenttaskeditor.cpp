#include "silenttaskeditor.h"
#pragma execution_character_set("utf-8")
SilentTaskEditor::SilentTaskEditor(QWidget *parent)
	: ITaskEditor(parent)
{
	ui.setupUi(this);
	initUI();
}

SilentTaskEditor::~SilentTaskEditor()
{}

void SilentTaskEditor::setTask(Task * task)
{
}

Task* SilentTaskEditor::getTask() const
{
	return m_currentTask;
}

void SilentTaskEditor::clear()
{
}

void SilentTaskEditor::initUI()
{
	//结束时刻
	m_timeEditEnd = new QDateTimeEdit(this);
	ui.tb_silentEnd->setCellWidget(3, 1, m_timeEditEnd);
	ui.tb_silentEnd->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tb_silentEnd->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	//预设结束复选框
	m_checkBoxIsEnd = new QCheckBox(this);
	QWidget* cbWidget = new QWidget(ui.tb_silentEnd);
	QHBoxLayout* hLayout = new QHBoxLayout(this);
	hLayout->addWidget(m_checkBoxIsEnd);
	hLayout->setMargin(0);
	hLayout->setAlignment(m_checkBoxIsEnd, Qt::AlignCenter);//居中
	hLayout->setMargin(0);
	cbWidget->setLayout(hLayout);
	ui.tb_silentEnd->setCellWidget(0, 1, cbWidget);
	m_comboBoxEndCondition = new QComboBox(this);
	m_comboBoxEndCondition->addItem("定时刻");
	m_comboBoxEndCondition->addItem("定时长");
	ui.tb_silentEnd->setCellWidget(1, 1, m_comboBoxEndCondition);
	//z单位
	m_comboBoxZUnit = new QComboBox(this);
	m_comboBoxZUnit->addItem("定深");
	m_comboBoxZUnit->addItem("定高");
	ui.tb_silentPosition->setCellWidget(1, 1, m_comboBoxZUnit);
	ui.tb_silentPosition->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tb_silentPosition->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
}
