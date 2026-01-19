#pragma once

#include <QObject>
#include <QScopedPointer>
#include "cmapcanvas.h"
#include "cs57maptool.h"
#include"../maptoolauv.h"
#include"../auvrenderer.h"
#include"../auvinfopanel.h"
class S57MapModule  : public QObject
{
	Q_OBJECT

public:
	S57MapModule(QObject *parent);
	~S57MapModule();
	//渲染
	void doRender();
	CMapCanvas* getMapCanvas();
	// 依赖注入接口 - 为AUV工具设置必要依赖
	void setAuvToolDependencies(AuvRenderer* renderer, AuvInfoPanel* infoPanel);
	void initialize();
	void loadMaps(QString fileName);
signals:
	void updateGeo(const QString& text);
	void updateScale(int scale);
	void canvasRefreshed();
public slots:
	//更新显示比例尺
	//放大
	void zoomIn();
	//缩小
	void zoomOut();
	//平移
	void setPanTool();
	void setAuvQueryTool();
private:
	bool checkAuvToolDependencies() const;
	// 创建工具实例
	void createTools();
private:

	CMapCanvas  *  m_pMapCanvas = nullptr;
	CS57MapTool * m_pMapToolPan = nullptr;
	CS57MapTool * m_pMapToolAuv = nullptr;
	// AUV工具的依赖项（外部注入，不负责销毁）
	AuvRenderer* m_pAuvRenderer = nullptr;
	AuvInfoPanel* m_pAuvInfoPanel = nullptr;
};
