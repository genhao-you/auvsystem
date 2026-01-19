#include "auvinfopanel.h"
#include<QScrollArea>
#include<QVariant>
#include<QMouseEvent>
#include<QStyle>
#pragma execution_character_set("utf-8")
AuvInfoPanel::AuvInfoPanel(QWidget* parent)
	: QWidget(parent)
	, m_isDragging(false)  // 初始化拖动状态为false
{


	//初始化信息标签
	initLabels();
	setObjectName("AuvInfoPanel");  // 给面板设置对象名，用于样式定位
	ModuleStyleHelper::loadStyleSheet(STYLE_AUVINFOPANEL, this);
}

AuvInfoPanel::~AuvInfoPanel()
{
	/*if (m_contentWidget != nullptr)
		delete m_contentWidget;
	m_contentWidget = nullptr;*/
}
// ------------------------------
// ：关闭按钮点击槽函数
// ------------------------------
void AuvInfoPanel::onCloseBtnClicked()
{
	clear();  // 关闭前清空内容
	emit closeClicked();  // 发送关闭信号，由外部决定隐藏还是销毁
}

// ------------------------------
// ：拖动功能实现（重写鼠标事件）
// ------------------------------
void AuvInfoPanel::mousePressEvent(QMouseEvent* event)
{
	// 仅当点击标题栏区域（且不是关闭按钮）时，才允许拖动
	if (event->button() == Qt::LeftButton &&
		event->pos().y() <= m_ititleHeight &&  // 点击y坐标在标题栏高度内
		event->pos().x() <= width() - 30)      // 点击x坐标不在关闭按钮区域
	{
		m_isDragging = true;
		// 记录“鼠标相对于窗口左上角”的起始位置（避免窗口跳变）
		m_dragStartPos = event->localPos();
	}
	QWidget::mousePressEvent(event);
}

void AuvInfoPanel::mouseMoveEvent(QMouseEvent* event)
{
	if (m_isDragging && event->buttons() & Qt::LeftButton)
	{
		// 计算窗口新位置：当前鼠标全局位置 - 拖动起始位置
		QPoint newWindowPos = (event->globalPos() - m_dragStartPos).toPoint();
		move(newWindowPos);  // 移动窗口
	}
	QWidget::mouseMoveEvent(event);
}

void AuvInfoPanel::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_isDragging = false;  // 释放鼠标，结束拖动
	}
	QWidget::mouseReleaseEvent(event);
}
void AuvInfoPanel::clear()
{
	m_titileLabel->setText("未选中任何元素");
	hideAllRows();
	updateContentHeight();
}

void AuvInfoPanel::showAuvInfo(const AuvRealTimeData& auv)
{
	clear();
	m_titileLabel->setText(QString("AUV信息:%1").arg(auv.auvId));

	showRow("id", auv.auvId);
	showRow("lon", QString("%1").arg(auv.geoPos.lon, 0, 'f', 6));
	showRow("lat", QString("%1").arg(auv.geoPos.lat, 0, 'f', 6));
	showRow("heading", QString("%1°").arg(auv.heading, 0, 'f', 1));
	showRow("speed", QString("%1 m/s").arg(auv.speed, 0, 'f', 1));
	showRow("status", auv.isConnected ? "<font color='#4CAF50'>在线</font>" : "<font color='#F44336'>离线</font>");
	showRow("time", auv.timestamp.toString("yyyy-MM-dd HH:mm:ss"));
	updateContentHeight();
}

void AuvInfoPanel::showTargetInfo(const DetectedTarget& target)
{
	clear();
	m_titileLabel->setText(QString("探测目标: - %1").arg(target.targetId));
	showRow("id", target.targetId);
	showRow("type", target.type);
	showRow("lon", QString("%1").arg(target.coord.lon, 0, 'f', 6));
	showRow("lat", QString("%1").arg(target.coord.lat, 0, 'f', 6));
	showRow("confidence", QString("%1").arg(target.confidence * 100, 0, 'f', 1));
	showRow("detectedTime", target.timestamp.toString("yyyy-MM-dd HH:mm:ss"));
	updateContentHeight();
}

void AuvInfoPanel::showTrajectoryPointInfo(const QString& auvId, const ActualTrajectoryPoint& traj)
{
	clear();
	m_titileLabel->setText(QString("轨迹点- AUV:%1").arg(auvId));

	showRow("id", auvId);
	showRow("lon", QString("%1").arg(traj.coord.lon, 0, 'f', 6));
	showRow("lat", QString("%1").arg(traj.coord.lat, 0, 'f', 6));
	showRow("heading", QString("%1°").arg(traj.heading, 0, 'f', 1));
	showRow("speed", QString("%1 m/s").arg(traj.speed, 0, 'f', 1));
	showRow("time", traj.timestamp.toString("yyyy-MM-dd HH:mm:ss"));
	updateContentHeight();
}

void AuvInfoPanel::showMissionPointInfo(const QString& auvId, const MissionWaypoint& missionPoint)
{
	clear();
	m_titileLabel->setText(QString("使命航点- AUV:%1").arg(auvId));

	showRow("id", auvId);
	showRow("lon", QString("%1").arg(missionPoint.coord.lon, 0, 'f', 6));
	showRow("lat", QString("%1").arg(missionPoint.coord.lat, 0, 'f', 6));
	showRow("stayTime", QString("%1 s").arg(missionPoint.stayTime, 0, 'f', 1));
	showRow("radius", QString("%1 m").arg(missionPoint.radius, 0, 'f', 2));
	showRow("taskDesc", missionPoint.taskDesc.isEmpty() ? "无" : missionPoint.taskDesc);  // 优化：空值显示“无”，避免空白
	updateContentHeight();
	//showRow("type", );
}



void AuvInfoPanel::initLabels()
{
	QScrollArea* scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(false);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //禁用水平滚动
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded); // 内容过长时显示垂直滚动条

	//内容容器
	m_contentWidget = new QWidget(scrollArea);

	//创建表单布局
	m_layout = new QGridLayout(m_contentWidget);

	// 布局参数保留，微调标题栏高度以容纳关闭按钮
	m_iRowHeight = 24;
	m_iRowSpacing = 15;
	m_ititleHeight = 36;  // 标题栏高度增加8px，给关闭按钮留空间
	m_iMargins = 12;
	m_iMaxContentHeight = 400;  // 最大高度适当增加，提升显示容量

	m_layout->setColumnStretch(0, 1);
	m_layout->setColumnStretch(1, 2);
	m_layout->setHorizontalSpacing(25);
	m_layout->setVerticalSpacing(m_iRowSpacing);
	m_layout->setContentsMargins(m_iMargins, m_iMargins, m_iMargins, m_iMargins);

	// ------------------------------
		// ：标题栏容器（包含标题标签 + 关闭按钮）
		// ------------------------------
	QWidget* titleBarWidget = new QWidget(m_contentWidget);
	QHBoxLayout* titleBarLayout = new QHBoxLayout(titleBarWidget);
	titleBarLayout->setContentsMargins(8, 4, 8, 4);
	titleBarLayout->setSpacing(10);

	// 标题标签（原有）
	m_titileLabel = new QLabel("未选中任何元素", titleBarWidget);
	m_titileLabel->setObjectName("titleLabel");
	m_titileLabel->setMinimumWidth(450); // 最小宽度，避免过窄
	m_titileLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	m_titileLabel->setFixedHeight(m_ititleHeight);
	titleBarLayout->addWidget(m_titileLabel);

	// ：关闭按钮（用系统默认图标，适配不同主题）
	m_closeBtn = new QPushButton(titleBarWidget);
	m_closeBtn->setObjectName("CloseBtn");
	// 使用Qt内置关闭图标（避免手动传图，兼容性更好）
	m_closeBtn->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
	m_closeBtn->setIconSize(QSize(16, 16));
	m_closeBtn->setToolTip("关闭面板");  // 鼠标悬浮提示
	m_closeBtn->setFixedSize(24, 24);    // 固定按钮大小
	titleBarLayout->addWidget(m_closeBtn, 0, Qt::AlignRight);  // 右对齐

	// 将标题栏容器加入主布局（占1行2列）
	m_layout->addWidget(titleBarWidget, 0, 0, 1, 2);

	// 连接关闭按钮信号（）
	connect(m_closeBtn, &QPushButton::clicked, this, &AuvInfoPanel::onCloseBtnClicked);

	// 预定义信息行（原有逻辑保留）
	addInfoRow(1, "id", "ID:");
	addInfoRow(2, "auvId", "所属AUV:");
	addInfoRow(3, "type", "类型:");
	addInfoRow(4, "lon", "经度:");
	addInfoRow(5, "lat", "纬度:");
	addInfoRow(6, "heading", "航向:");
	addInfoRow(7, "speed", "速度:");
	addInfoRow(8, "status", "状态:");
	addInfoRow(9, "time", "时间:");
	addInfoRow(10, "confidence", "置信度(%):");  // 优化：补充单位，更直观
	addInfoRow(11, "detectedTime", "探测时间:");
	addInfoRow(12, "stayTime", "停留时间(s):");  // 优化：补充单位
	addInfoRow(13, "radius", "到达半径(m):");    // 优化：补充单位
	addInfoRow(14, "taskDesc", "任务描述:");
	addInfoRow(15, "pointType", "点类型:");      // 优化：键名更清晰，避免与type重复

	m_contentWidget->setFixedWidth(450);  // 面板宽度适当增加，避免文本换行
	scrollArea->setWidget(m_contentWidget);

	// 最外布局（原有）
	QVBoxLayout* outerLayout = new QVBoxLayout(this);
	outerLayout->setContentsMargins(0, 0, 0, 0);
	outerLayout->addWidget(scrollArea);

	clear();
	
}

void AuvInfoPanel::addInfoRow(int row, const QString& key, const QString& labelText)
{
	InfoRow infoRow;
	infoRow.label = new QLabel(labelText, m_contentWidget);
	infoRow.label->setProperty("label", QVariant(true));
	//infoRow.label->setAlignment(Qt::AlignRight | Qt::AlignVCenter); //强制右对齐
	infoRow.label->setFixedHeight(m_iRowHeight);
	infoRow.label->setMinimumWidth(100);
	infoRow.label->setWordWrap(false);
	infoRow.label->hide(); // 默认隐藏

	infoRow.value = new QLabel(m_contentWidget);
	infoRow.value->setProperty("value", true);
	//infoRow.value->setAlignment(Qt::AlignLeft | Qt::AlignVCenter); // 强制左对齐
	infoRow.value->setFixedHeight(m_iRowHeight);
	infoRow.value->setWordWrap(true); //长文本自动换行
	infoRow.value->setTextInteractionFlags(Qt::TextSelectableByMouse);
	infoRow.value->hide();  //默认隐藏


	//添加到网络布局的固定行
	m_layout->addWidget(infoRow.label, row, 0);
	m_layout->addWidget(infoRow.value, row, 1);
	m_infoRows[key] = infoRow;
}

void AuvInfoPanel::showRow(const QString& key, const QString& value)
{
	if (m_infoRows.contains(key))
	{
		m_infoRows[key].value->setText(value);
		m_infoRows[key].label->show();
		m_infoRows[key].value->show();

		//强制刷新尺寸，解决隐藏后显示的尺寸偏差
		m_infoRows[key].label->updateGeometry();
		m_infoRows[key].value->updateGeometry();
	}
}

void AuvInfoPanel::hideAllRows()
{
	for (auto& pair : m_infoRows)
	{
		pair.label->hide();
		pair.value->hide();
		pair.value->setText("");
	}
}

int AuvInfoPanel::countVisibleRows()
{
	int count = 0;
	for (auto& pair : m_infoRows)
	{
		if (pair.label->isVisible())
		{
			count++;
		}
	}
	return count;
}

void AuvInfoPanel::updateContentHeight()
{
	int visibleRows = countVisibleRows();
	if (visibleRows == 0)
	{
		// 无内容时，预留足够高度避免标题挤压
		m_contentWidget->setMinimumHeight(m_ititleHeight + m_iMargins * 2 + 20);
		m_contentWidget->setMaximumHeight(QWIDGETSIZE_MAX); // 取消最大高度限制
		return;
	}

	// 高度计算公式：包含行间距和内边距
	int contentHeight = m_ititleHeight // 标题高度
		+ (m_iRowHeight * visibleRows) // 所有可见行的高度
		+ (m_iRowSpacing * visibleRows) // 行间距（每行都有间距）
		+ (m_iMargins * 2); // 上下内边距
	    + 20;                            // 额外缓冲空间（关键）
// 限制最大高度，超过则显示滚动条
	if (contentHeight > m_iMaxContentHeight)
	{
		m_contentWidget->setMaximumHeight(m_iMaxContentHeight);
		m_contentWidget->setMinimumHeight(m_ititleHeight + m_iMargins * 2);
	}
	else
	{
		m_contentWidget->setFixedHeight(contentHeight);
	}

}
