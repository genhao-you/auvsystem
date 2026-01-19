#include "beaconcommunciationtaskediotor.h"

BeaconCommunciationTaskEdiotor::BeaconCommunciationTaskEdiotor(QWidget *parent)
	: ITaskEditor(parent)
{
	ui.setupUi(this);
	setupUI();
	connectSignals();
	clear();
}

BeaconCommunciationTaskEdiotor::~BeaconCommunciationTaskEdiotor()
{}

void BeaconCommunciationTaskEdiotor::setTask(Task * task)
{
	if (m_currentTask == task)
		return;

	m_currentTask = task;
	updateFields();
}

Task* BeaconCommunciationTaskEdiotor::getTask() const
{
	return m_currentTask;
}

void BeaconCommunciationTaskEdiotor::clear()
{
	m_currentTask = nullptr;
}

void BeaconCommunciationTaskEdiotor::connectSignals()
{
	auto updateTaskData = [this]() {
		if (!m_currentTask)
			return;

		m_currentTask->setParam("id", ui.tb_beaconId->item(0, 1)->text());
		emit dataChanged();
	};
}

void BeaconCommunciationTaskEdiotor::setupUI()
{
}

void BeaconCommunciationTaskEdiotor::updateFields()
{
	if (!m_currentTask)
	{
		clear();
		return;
	}
	//¸üÐÂui from task
	ui.tb_beaconId->item(0, 1)->setText(m_currentTask->getParam("id").toString());
}
