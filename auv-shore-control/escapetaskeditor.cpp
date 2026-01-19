#include "escapetaskeditor.h"

EscapeTaskEditor::EscapeTaskEditor(QWidget *parent)
	: ITaskEditor(parent)
{
	ui.setupUi(this);
	//速度单位
	m_comboBoxSpeedUnit = new QComboBox(this);
	m_comboBoxSpeedUnit->addItem("m/s");
	m_comboBoxSpeedUnit->addItem("kn");
	m_comboBoxSpeedUnit->addItem("km/s");
	m_comboBoxSpeedUnit->addItem("RPM");
	ui.tb_escape->setCellWidget(2, 1, m_comboBoxSpeedUnit);
}

EscapeTaskEditor::~EscapeTaskEditor()
{}

void EscapeTaskEditor::setTask(Task * task)
{
}

Task* EscapeTaskEditor::getTask() const
{
	return m_currentTask;
}

void EscapeTaskEditor::clear()
{
}
