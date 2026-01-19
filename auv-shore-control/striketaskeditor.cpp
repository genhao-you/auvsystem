#include "striketaskeditor.h"

StrikeTaskEditor::StrikeTaskEditor(QWidget *parent)
	: ITaskEditor(parent)
{
	ui.setupUi(this);
}

StrikeTaskEditor::~StrikeTaskEditor()
{}

void StrikeTaskEditor::setTask(Task * task)
{
}

Task* StrikeTaskEditor::getTask() const
{
	return m_currentTask;
}

void StrikeTaskEditor::clear()
{
}
