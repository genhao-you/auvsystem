#pragma once

#include <QObject>
#include<QWidget>
#include<QFormLayout>
#include<QLabel>
#include<QMap>
#include<QPushButton>
#include"mission_monitor_types.h"
#include"moduledialogbase.h"
struct InfoRow
{
	QLabel* label = nullptr;
	QLabel* value = nullptr;
};

class AuvInfoPanel  : public QWidget
{
	Q_OBJECT

public:
	explicit AuvInfoPanel(QWidget *parent = nullptr);
	~AuvInfoPanel();

	//清除显示内容
	void clear();

	//显示AUV详细信息
	void showAuvInfo(const AuvRealTimeData& auv);

	//显示目标详细信息
	void showTargetInfo(const DetectedTarget& target);

	//显示轨迹点详细信息
	void showTrajectoryPointInfo(const QString& auvId,const ActualTrajectoryPoint& traj);

	//显示使命航点详细信息
	void showMissionPointInfo(const QString& auvId, const MissionWaypoint& missionPoint);
signals:
	// ：关闭按钮点击信号（供外部处理隐藏/销毁逻辑）
	void closeClicked();

private slots:
	// ：关闭按钮点击槽函数
	void onCloseBtnClicked();
private:
	//初始化标签
	void initLabels();

	//添加信息行到布局
	void addInfoRow(int row,const QString& key, const QString& labelText);

	//显式指定行并设置值
	void showRow(const QString& key, const QString& value);

	void hideAllRows();

	int  countVisibleRows();

	void updateContentHeight();

	int getTitleBarActualHeight();

	// ：拖动相关方法
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
private:
	QWidget* m_contentWidget;
	QGridLayout* m_layout;
	QLabel* m_titileLabel;
	QMap<QString, InfoRow>m_infoRows;
	int m_iRowHeight;
	int m_iRowSpacing;
	int m_ititleHeight;
	int m_iMargins;
	int m_iMaxContentHeight;

	// ：拖动相关成员
	bool m_isDragging;          // 是否处于拖动状态
	QPointF m_dragStartPos;      // 拖动起始位置（相对于窗口左上角）

	// ：关闭按钮
	QPushButton* m_closeBtn;
	const QString STYLE_AUVINFOPANEL = "./qss/auvinfopanel.qss";
};
