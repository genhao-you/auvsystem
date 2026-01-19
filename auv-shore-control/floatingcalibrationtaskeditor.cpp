#include "floatingcalibrationtaskeditor.h"

FloatingCalibrationTaskEditor::FloatingCalibrationTaskEditor(QWidget *parent)
	: ITaskEditor(parent)
{
	ui.setupUi(this);
}

FloatingCalibrationTaskEditor::~FloatingCalibrationTaskEditor()
{}

void FloatingCalibrationTaskEditor::setTask(Task * task)
{
}

Task* FloatingCalibrationTaskEditor::getTask() const
{
	return m_currentTask;
}

void FloatingCalibrationTaskEditor::clear()
{
}
