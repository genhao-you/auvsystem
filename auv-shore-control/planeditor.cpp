#include "planeditor.h"

PlanEditor::PlanEditor(QWidget* parent)
	:ITaskEditor(parent), m_currentPlan(nullptr)
{
	connectSignals();
	clear();
}

PlanEditor::~PlanEditor()
{
}

Task* PlanEditor::getTask() const
{
	return m_currentTask;
}

void PlanEditor::setPlan(Plan* plan)
{
	if (m_currentPlan == plan)
		return;

	m_currentPlan = plan;

}

void PlanEditor::clear()
{
	m_currentPlan = nullptr;
}

void PlanEditor::connectSignals()
{
	//connect(ui.lb_planName,QLineEdit)
}

void PlanEditor::updateFields()
{
	if (!m_currentPlan)
	{
		clear();
		return;
	}
	//更新ui顯示
	ui.lb_planName->setText(m_currentPlan->getPlanName());
	ui.lb_planMode->setText([this]() {
		switch (m_currentPlan->getPlanMode())
		{
		case PlanMode::TEST_DEBUG: return tr("试验调试"); break;
		case PlanMode::SURFACE_RECON:return tr("水面侦察"); break;
		case PlanMode::UNDERWATER_RECON:return tr("水下侦察"); break;
		case PlanMode::SURFACE_STRIKE:return tr("水面打击"); break;
		case PlanMode::UNDERWATER_STRIKE:return tr("水下打击"); break;
		case PlanMode::LAND_STRIKE:return tr("岸基打击"); break;
		default:
			return tr("未知模式");
			break;
		}
		}());
	ui.lb_planDistance->setText(QString::number(m_currentPlan->getDistance()));
	ui.lb_planTime->setText(QString::number(m_currentPlan->getEstimatedTime()));
	ui.lb_planMinDepth->setText(QString::number(m_currentPlan->getMinDepth()));
	ui.lb_planMaxDepth->setText(QString::number(m_currentPlan->getMaxDepth()));
	ui.lb_taskCount->setText(QString::number(m_currentPlan->getTaskCount()));

}
