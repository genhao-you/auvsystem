#include "missionview.h"
#include <QVBoxLayout>
#include<QContextMenuEvent>
#pragma execution_character_set("utf-8")
MissionView::MissionView(QWidget* parent) : QWidget(parent),
m_model(nullptr),
m_missionMenu(new QMenu(this)),
m_planMenu(new QMenu(this)),
m_taskMenu(new QMenu(this)),
m_emptyAreaMenu(new QMenu(this))
{
	setupUi();
	createActions();
	createTaskMenu();
	createMissionMenu();
	createPlanMenu();
	createEmptyAreaMenu();
}

MissionView::~MissionView()
{
	if (m_missionMenu != nullptr)
		delete m_missionMenu;
	m_missionMenu = nullptr;

	if (m_planMenu != nullptr)
		delete m_planMenu;
	m_planMenu = nullptr;

	if (m_taskMenu != nullptr)
		delete m_taskMenu;
	m_taskMenu = nullptr;

	if (m_emptyAreaMenu != nullptr)
		delete m_emptyAreaMenu;
	m_emptyAreaMenu = nullptr;

	if (m_specificTaskMenu != nullptr)
		delete m_specificTaskMenu;
	m_specificTaskMenu = nullptr;

	qDeleteAll(m_taskActions);
	m_taskActions.clear();

	qDeleteAll(m_planActions);
	m_planActions.clear();

	
}

void MissionView::setModel(MissionModel* model) {
	if (m_model == model)
		return;

	m_model = model;
	m_treeView->setModel(model);
	connect(m_treeView->selectionModel(), &QItemSelectionModel::currentChanged,
		this, &MissionView::on_selection_changed);
}

MissionModel* MissionView::model() const {
	return m_model;
}

QModelIndex MissionView::currentIndex() const {
	return m_treeView->currentIndex();
}

void MissionView::setCurrentIndex(const QModelIndex& index) {
	m_treeView->setCurrentIndex(index);
}

void MissionView::edit(const QModelIndex& index) {
	m_treeView->edit(index);
}

void MissionView::setupUi() {
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);

	m_treeView = new QTreeView(this);
	m_treeView->setHeaderHidden(true);
	m_treeView->setSelectionMode(QAbstractItemView::SingleSelection);
	m_treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_treeView->setAnimated(true);
	m_treeView->setIndentation(15);



	layout->addWidget(m_treeView);
}

void MissionView::createActions() {

	//使命相关动作
	m_addPlanAction = new QAction(tr("新建计划"), this);
	connect(m_addPlanAction, &QAction::triggered, [this]() {
		emit addPlanRequested(m_treeView->currentIndex(),PlanMode::TEST_DEBUG);
		});

	//计划相关动作
	m_addTaskAction = new  QAction(tr("新建任务"), this);
	connect(m_addTaskAction, &QAction::triggered, [this]() {
		emit addTaskRequested(m_treeView->currentIndex(), TaskType::SAIL);
		});

	//空白区域

	m_addMissionAction = new QAction(tr("新建使命"), this);
	connect(m_addMissionAction, &QAction::triggered, this, &MissionView::addMissionRequested);

	//公共动作
	m_removeNodeAction = new QAction(tr("删除"), this);
	connect(m_removeNodeAction, &QAction::triggered, [this]() {
		emit removeNodeRequested(m_treeView->currentIndex());
		});
	m_newAction = new QAction(tr("新建"), this);
}

void MissionView::createTaskMenu()
{

	m_taskMenu->addAction(m_removeNodeAction);
}
void MissionView::createPlanMenu()
{
	
	m_specificTaskMenu = new QMenu(tr("新建任务"));
	QMap<TaskType, QString>taskTypes = {
		{ TaskType::SAIL,taskTypeToString(TaskType::SAIL)},
		{ TaskType::SILENT,taskTypeToString(TaskType::SILENT)},
		{ TaskType::PATROL_DETECT,taskTypeToString(TaskType::PATROL_DETECT)},
		{ TaskType::STRIKE,taskTypeToString(TaskType::STRIKE)},
		{ TaskType::SHORE_BASED_STRIKE,taskTypeToString(TaskType::SHORE_BASED_STRIKE)},
		{ TaskType::ESCAPE,taskTypeToString(TaskType::ESCAPE)},
		{ TaskType::FLOATING_CALIBRATION,taskTypeToString(TaskType::FLOATING_CALIBRATION)},
		{ TaskType::FLOATING_COMMUNICATION,taskTypeToString(TaskType::FLOATING_COMMUNICATION)},
		{ TaskType::BEACON_COMMUNICATION,taskTypeToString(TaskType::BEACON_COMMUNICATION)},
		{ TaskType::RECOVERY,taskTypeToString(TaskType::RECOVERY)}
	};
	
	for (auto it = taskTypes.constBegin(); it != taskTypes.constEnd(); ++it)
	{
		QAction* action = m_specificTaskMenu->addAction(it.value());
		connect(action, &QAction::triggered, [this, type = it.key()]()
		{
			emit addTaskRequested(m_treeView->currentIndex(), type);
		});

	}
	m_planMenu->addMenu(m_specificTaskMenu);//新建任务
	m_planMenu->addSeparator();
	m_planMenu->addAction(m_removeNodeAction);// 删除计划


}

void MissionView::createMissionMenu()
{
	//计划模式子菜单
	m_planModeMenu = new QMenu(tr("新建计划"), this);
	QMap<PlanMode, QString> planModes = {
		{PlanMode::TEST_DEBUG, planModeToString(PlanMode::TEST_DEBUG)},
		{PlanMode::SURFACE_RECON, planModeToString(PlanMode::SURFACE_RECON)},
		{PlanMode::UNDERWATER_RECON,planModeToString(PlanMode::UNDERWATER_RECON)},
		{PlanMode::SURFACE_STRIKE, planModeToString(PlanMode::SURFACE_STRIKE)},
		{PlanMode::UNDERWATER_STRIKE, planModeToString(PlanMode::UNDERWATER_STRIKE)},
		{PlanMode::LAND_STRIKE, planModeToString(PlanMode::LAND_STRIKE)}
	};

	for (auto it = planModes.constBegin(); it != planModes.constEnd(); ++it)
	{
		QAction* action = m_planModeMenu->addAction(it.value());
		connect(action, &QAction::triggered, [this, mode = it.key()]()
		{
			emit addPlanRequested(m_treeView->currentIndex(), mode);
		});


	}
	m_missionMenu->addMenu(m_planModeMenu);
	m_missionMenu->addSeparator();
	//m_missionMenu->addAction(m_addPlanAction);//使命节点下新建计划、
	//m_missionMenu->addSeparator();
	m_missionMenu->addAction(m_removeNodeAction);

}

void MissionView::createEmptyAreaMenu()
{
	m_emptyAreaMenu->addAction(m_addMissionAction);
}

void MissionView::contextMenuEvent(QContextMenuEvent* event)
{
	QModelIndex index = m_treeView->indexAt(event->pos());
	if (index.isValid())
	{
		int nodeType = m_model->data(index, MissionModel::NodeTypeRole).toInt();
		switch (nodeType)
		{
		case MissionModel::MISSION:
			m_missionMenu->exec(event->globalPos());
			break;
		case MissionModel::PLAN:
			m_planMenu->exec(event->globalPos());
			break;
		case MissionModel::TASK:
			m_taskMenu->exec(event->globalPos());
			break;
		default:
			return;
		}
	}
	else
	{
		m_emptyAreaMenu->exec(event->globalPos());
	}
	


}
void MissionView::on_selection_changed(const QModelIndex& current)
{
	emit selectionChanged(current);
}
