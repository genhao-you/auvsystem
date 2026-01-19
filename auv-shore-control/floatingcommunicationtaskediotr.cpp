#include "floatingcommunicationtaskediotr.h"

FloatingCommunicationTaskEdiotr::FloatingCommunicationTaskEdiotr(QWidget *parent)
	: ITaskEditor(parent)
{
	ui.setupUi(this);
}

FloatingCommunicationTaskEdiotr::~FloatingCommunicationTaskEdiotr()
{}

void FloatingCommunicationTaskEdiotr::setTask(Task * task)
{
}

void FloatingCommunicationTaskEdiotr::clear()
{
}

Task* FloatingCommunicationTaskEdiotr::getTask() const
{
	return m_currentTask;
}
