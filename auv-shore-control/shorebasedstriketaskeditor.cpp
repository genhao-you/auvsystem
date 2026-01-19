#include "shorebasedstriketaskeditor.h"

ShoreBasedStrikeTaskEditor::ShoreBasedStrikeTaskEditor(QWidget *parent)
	: ITaskEditor(parent)
{
	ui.setupUi(this);
}

ShoreBasedStrikeTaskEditor::~ShoreBasedStrikeTaskEditor()
{}

void ShoreBasedStrikeTaskEditor::setTask(Task * task)
{
}

Task* ShoreBasedStrikeTaskEditor::getTask() const
{
	return m_currentTask;
}

void ShoreBasedStrikeTaskEditor::clear()
{
}
