#include "recoverytaskeditor.h"

RecoveryTaskEditor::RecoveryTaskEditor(QWidget *parent)
	: ITaskEditor(parent)
{
	ui.setupUi(this);
}

RecoveryTaskEditor::~RecoveryTaskEditor()
{}

void RecoveryTaskEditor::setTask(Task * task)
{
}

Task* RecoveryTaskEditor::getTask() const
{
	return m_currentTask;
}

void RecoveryTaskEditor::clear()
{
}
