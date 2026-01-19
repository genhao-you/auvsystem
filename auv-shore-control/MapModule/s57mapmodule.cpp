#include "s57mapmodule.h"
#include "cs57maptoolpan.h"
#include "cs57maptoolquery.h"
#include<QMessageBox>
#include<qDebug>
// 正确的 CHECK_PTR 宏：仅检查+日志+断言，不包含 return
#define CHECK_PTR(ptr, msg) \
    do { \
        if (!(ptr)) { \
            qCritical() << "[CHECK_PTR] " << msg << " (Pointer address: " << reinterpret_cast<void*>(ptr) << ")"; \
            Q_ASSERT_X(ptr != nullptr, __FUNCTION__, msg); \
        } \
    } while (0)
S57MapModule::S57MapModule(QObject *parent)
	: QObject(parent)
	,m_pMapCanvas(new CMapCanvas())
	
{
	
	// 创建工具
	createTools();
	// 初始化模块
	initialize();
	//doRender();
}

S57MapModule::~S57MapModule()
{
   if (m_pMapToolPan != nullptr)
		delete m_pMapToolPan;
	m_pMapToolPan = nullptr;

	if (m_pMapToolAuv != nullptr) 
		delete m_pMapToolAuv;
		m_pMapToolAuv = nullptr;
	
	if (m_pMapCanvas != nullptr)
	{
		m_pMapCanvas->setMapTool(nullptr);
		delete m_pMapCanvas;
		m_pMapCanvas = nullptr;
	}
		
}

void S57MapModule::doRender()
{
	m_pMapCanvas->fastInit();
	m_pMapCanvas->createCanvas();
	m_pMapCanvas->renderCanvas();
	m_pMapCanvas->setDisplayCategory(XT::DISPLAY_STANDARD);
	//视角跳转
	m_pMapCanvas->flyToCell();
}

CMapCanvas* S57MapModule::getMapCanvas()
{
	return m_pMapCanvas;
}

void S57MapModule::setAuvToolDependencies(AuvRenderer* renderer, AuvInfoPanel* infoPanel)
{
	m_pAuvRenderer = renderer;
	m_pAuvInfoPanel = infoPanel;

	// 如果AUV工具未创建且依赖满足，则创建
	if (!m_pMapToolAuv && checkAuvToolDependencies()) {
		m_pMapToolAuv = new MapToolAuv(
			m_pMapCanvas,
			m_pAuvRenderer,
			m_pAuvInfoPanel
		);
		CHECK_PTR(m_pMapToolAuv, "Failed to create MapToolAuv!");
		qDebug() << "[S57MapModule] MapToolAuv created successfully";
	}
}

void S57MapModule::initialize()
{
	m_pMapCanvas->setMapTool(m_pMapToolPan);
	m_pMapCanvas->setBorderVisible(false);
	connect(m_pMapCanvas, &CMapCanvas::moveGeo, this, &S57MapModule::updateGeo);
	connect(m_pMapCanvas, &CMapCanvas::changeScale, this, &S57MapModule::updateScale);
	
}
void S57MapModule::loadMaps(QString fileName)
{
	m_pMapCanvas->reloadEnc(fileName);
	//视角跳转
	m_pMapCanvas->flyToCell();
}
void S57MapModule::zoomIn()
{
	m_pMapCanvas->zoomIn();
}
void S57MapModule::zoomOut()
{
	m_pMapCanvas->zoomOut();
}

void S57MapModule::setPanTool()
{
	m_pMapCanvas->setOperateMode(XT::NORMAL);
	m_pMapCanvas->setQueryMode(XT::NO_QUERY);
	m_pMapCanvas->closeQuery();
	m_pMapCanvas->setMapTool(m_pMapToolPan);
}

void S57MapModule::setAuvQueryTool()
{
	m_pMapCanvas->setOperateMode(XT::AUVQUERY);
	m_pMapCanvas->setQueryMode(XT::NO_QUERY);
	m_pMapCanvas->closeQuery();
	m_pMapCanvas->setMapTool(m_pMapToolAuv);  // 直接使用交互工具
}

bool S57MapModule::checkAuvToolDependencies() const
{
	CHECK_PTR(m_pAuvRenderer, "AuvRenderer is not set!");
	CHECK_PTR(m_pAuvInfoPanel, "AuvInfoPanel is not set!");
	CHECK_PTR(m_pMapCanvas, "MapCanvas is null!");
	return true;
}

void S57MapModule::createTools()
{
	// 创建平移工具
	m_pMapToolPan = new CS57MapToolPan(m_pMapCanvas);
	CHECK_PTR(m_pMapToolPan, "Failed to create CS57MapToolPan!");

	// AUV工具暂时不创建，等待依赖注入后再创建
	m_pMapToolAuv = nullptr;
}




