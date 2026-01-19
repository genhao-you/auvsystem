#include "stdafx.h"
#include "dpi.h"
#include "common.h"
#include "cs57control.h"
#include "cs57settings.h"
#include "cs57transform.h"
#include "cs57routemanager.h"
#include <QMessageBox>
#include <QPrintDialog>
#include <QPagedPaintDevice>
#include <QPrinter>
#include <QDebug>
#include <QDateTime>
#include <QFileInfo>


using namespace Core;
using namespace Doc;
#define Min_Scale 500.
#define Max_Scale 100000000.
#pragma execution_character_set("utf-8")
CS57Control::CS57Control(QWidget* parent)
	: QGraphicsView(parent)
	, m_pS57Scene(new QGraphicsScene())
	, m_pS57DocManager(CS57DocManager::instance())
	, m_pS57Analysis(CS57Analysis::instance())
	, m_pS57CanvasImage(nullptr)
	, m_pS57CanvasMap(nullptr)
	, m_pLayerTask(new CS57LayerTask())
	, m_pCellLayer(new CS57CellLayer())
	, m_pHighlightLayer(new CS57HighlightLayer())
	, m_pCustomLayer(new CS57CustomLayer())
	, m_pMcovrRegionLayer(new CS57McovrRegionLayer())
	, m_pTransform(CS57Transform::instance())
	, m_pRouteManager(new CS57RouteManager(this))
	, m_dDpi(0.0)
	, m_dDpiX(0.0)
	, m_dDpiY(0.0)
	, m_GeoMouseMove(QPointF(0.0,0.0))
	, m_bMousePress(false)
	, m_eCurOperateMode(XT::NORMAL)
	, m_eCurQueryMode(XT::NO_QUERY)
	, m_eCurSymbolScaleMode(XT::ABS_SCALE)
	, m_eCurSelectChartType(CS57DocManager::SELECT_DISPLAY)
	, m_CanvasColor(212, 234, 238)
	, m_pMapTool(nullptr)
	, m_bCellLayerVisible(true)
	, m_pS57McovrRegionLegend(new CS57McovrRegionLegend(this))
{
	init();
	initCanvas();
	initConnects();
}
CS57Control::~CS57Control()
{
	if (m_pS57Analysis != nullptr)
		delete m_pS57Analysis;
	m_pS57Analysis = nullptr;

	if (m_pCellLayer != nullptr)
		delete m_pCellLayer;
	m_pCellLayer = nullptr;

	if (m_pHighlightLayer != nullptr)
		delete m_pHighlightLayer;
	m_pHighlightLayer = nullptr;

	if (m_pCustomLayer != nullptr)
		delete m_pCustomLayer;
	m_pCustomLayer = nullptr;

	if (m_pMcovrRegionLayer != nullptr)
		delete m_pMcovrRegionLayer;
	m_pMcovrRegionLayer = nullptr;

	if (m_pLayerTask)
		delete m_pLayerTask;
	m_pLayerTask = nullptr;

	destoryImage();

	if (m_pRouteManager != nullptr)
		delete m_pRouteManager;
	m_pRouteManager = nullptr;

	if (m_pS57DocManager != nullptr)
		delete m_pS57DocManager;
	m_pS57DocManager = nullptr;

	if (m_pS57CanvasMap != nullptr)
		delete m_pS57CanvasMap;
	m_pS57CanvasMap = nullptr;

	if (m_pS57Scene != nullptr)
		delete m_pS57Scene;
	m_pS57Scene = nullptr;

	if (m_pS57McovrRegionLegend != nullptr)
		delete m_pS57McovrRegionLegend;
	m_pS57McovrRegionLegend = nullptr;
}

//************************************
// 返回值:  void 
// 简介:	初始化
// 日期：	2021/06/01
//************************************
void CS57Control::init()
{
    #if defined(_WIN32_WINNT_WIN7)
	HWND hWnd = (HWND)this->winId();
	HDC hdc = GetDC(hWnd);
	m_dDpi = Dpi::getDPI(hdc);
	m_dDpiX = Dpi::getXDPI(hdc);
	m_dDpiY = Dpi::getYDPI(hdc);
	Dpi::setCurrentDpi(m_dDpi);
	#else
	m_dDpi = Dpi::screenDpi(XT::DPI);
	m_dDpiX = Dpi::screenDpi(XT::DPI_X);
	m_dDpiY = Dpi::screenDpi(XT::DPI_Y);
	Dpi::setCurrentDpi(m_dDpi);
	#endif
	m_pCellLayer->setPresLib(m_pS57DocManager->getPresLib());

	m_pS57Analysis->setDoc(m_pS57DocManager);
	m_pS57Analysis->setPresLib(m_pS57DocManager->getPresLib());
}

//************************************
// 返回值:  void 
// 简介:	初始化画布
// 日期：	2021/06/01
//************************************
void CS57Control::initCanvas()
{
	setCursor(Qt::OpenHandCursor);
	setScene(m_pS57Scene);
	setAlignment(Qt::AlignLeft | Qt::AlignTop);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setMouseTracking(true);
	setFocusPolicy(Qt::StrongFocus);
	setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
	QSize size = viewport()->size();
	setSceneRect(0, 0, size.width(), size.height());
	m_pS57Scene->setSceneRect(QRectF(0, 0, size.width(), size.height()));

	QBrush defaultBrush(QColor(255, 255, 255));
	m_pS57Scene->setBackgroundBrush(defaultBrush);

	m_pS57CanvasMap = new CS57CanvasMap(this);
	setMcovrRegionLegendVisible(false);
}

//************************************
// Method:    initConnects
// Brief:	  初始化连接
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57Control::initConnects()
{
	connect(m_pS57DocManager, &CS57DocManager::updateLoadCellProgress, this, &CS57Control::updateLoadCellProgress);
}

//************************************
// 返回值:  void 
// 简介:	更新画布
// 日期：	2021/06/01
//************************************
void CS57Control::updateCanvas()
{
	if (m_pS57CanvasImage)
	{
		QSize lastSize = viewport()->size();
		QImage* canvasImg = m_pS57CanvasImage->canvasImage();
		m_pS57CanvasMap->setContent(canvasImg, QRectF(0,0,lastSize.width(),lastSize.height()));
		m_pRouteManager->renderAllRoute(CS57RouteManager::Normal);
		m_pRouteManager->renderAllRoute(CS57RouteManager::Temp);
	}
}

//************************************
// Method:    setLocation
// Brief:	  设置初始跳转点
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: double lon
// Parameter: double lat
// Parameter: double display
//************************************
void CS57Control::setLocation(double lon, double lat, double display)
{
	CS57AppConfig* pConfig = CS57AppConfig::instance();
	pConfig->setLocation(lon, lat, display);
	pConfig->updateXml();
}

//************************************
// Method:    closeQuery
// Brief:	  关闭查询
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57Control::closeQuery()
{
	m_pS57Analysis->getFeatureQueryPtr()->closeFeatureQuery();
}

//************************************
// 返回值:  void 
// 简介:	设置ENC目录
// 参数:	const QString & dir 
// 日期：	2021/06/01
//************************************
void CS57Control::setEncDir(const QString& dir)
{
	CS57AppConfig* pConfig = CS57AppConfig::instance();
	QString strDir = dir;
	pConfig->setEncDir(strDir);
	pConfig->updateXml();

	m_pS57DocManager->setEncDir(dir);
}

//************************************
// 返回值:  void 
// 简介:	加载ENC
// 日期：	2021/06/01
//************************************
void CS57Control::loadEnc()
{
	m_pS57DocManager->loadEnc();
}

//************************************
// 返回值:  void 
// 简介:	设置投影名称
// 参数:	int proj_id 
// 日期：	2021/06/01
//************************************
void CS57Control::setProjectionName(XT::ProjType type)
{
	m_pTransform->m_Proj.setProjectionName(type);
}

//************************************
// 返回值:  void 
// 简介:	设置基准线
// 参数:	double baseLine 
// 日期：	2021/06/01
//************************************
void CS57Control::setBaseLine(double baseLine)
{
	m_pTransform->m_Proj.setBaseLine(baseLine);

	CS57AppConfig* pConfig = CS57AppConfig::instance();
	pConfig->setBaseLine(fun_radTodmmss(baseLine));
	pConfig->updateXml();
}

//************************************
// 返回值:  void 
// 简介:	设置视口中心点
// 参数:	double geo_lon 
// 参数:	double geo_lat 
// 日期：	2021/06/01
//************************************
void CS57Control::setViewportCenterPt(double geo_lon, double geo_lat)
{
	double vpx = 0.0, vpy = 0.0;
	double lon = 0.0, lat = 0.0;
	lon = geo_lon * PI / 180.;
	lat = geo_lat * PI / 180.;

	m_pTransform->m_Proj.getXY(lat, lon, &vpy, &vpx);
	m_pTransform->m_Viewport.setCenterPtXY(vpx, vpy);
}

//************************************
// 返回值:  void 
// 简介:	设置视口显示比例尺
// 参数:	double displayScale 
// 日期：	2021/06/01
//************************************
void CS57Control::setViewportDisplayScale(double displayScale)
{
	m_pTransform->m_Viewport.setDisplayScale(displayScale);
}

//************************************
// 返回值:  void 
// 简介:	设置视口
// 参数:	int dpi_x 
// 参数:	int dpi_y 
// 参数:	int x 
// 参数:	int y 
// 参数:	int w 
// 参数:	int h 
// 日期：	2021/06/01
//************************************
void CS57Control::setViewport(int dpi_x, int dpi_y, int x, int y, int w, int h)
{
	m_pTransform->m_Viewport.setViewPort(dpi_x, dpi_y, QRectF(x, y, w, h));
	QPointF pt[4];
	m_pTransform->m_Viewport.getBufferRange(pt);
	pt[0] = m_pTransform->vp2Geo(pt[0]);
	pt[1] = m_pTransform->vp2Geo(pt[1]);
	pt[2] = m_pTransform->vp2Geo(pt[2]);
	pt[3] = m_pTransform->vp2Geo(pt[3]);
	m_pS57DocManager->createSelectPolygon(pt);
}

//************************************
// 返回值:  void 
// 简介:	设置视口
// 参数:	int dpi_x 
// 参数:	int dpi_y 
// 参数:	QRectF rect 
// 日期：	2021/06/01
//************************************
void CS57Control::setViewport(int dpi_x, int dpi_y, QRectF rect)
{
	m_pTransform->m_Viewport.setViewPort(dpi_x, dpi_y, rect);
	QPointF pt[4];
	m_pTransform->m_Viewport.getBufferRange(pt);
	pt[0] = m_pTransform->vp2Geo(pt[0]);
	pt[1] = m_pTransform->vp2Geo(pt[1]);
	pt[2] = m_pTransform->vp2Geo(pt[2]);
	pt[3] = m_pTransform->vp2Geo(pt[3]);
	m_pS57DocManager->createSelectPolygon(pt);
}

//************************************
// 返回值:  void 
// 简介:	渲染操作
// 日期：	2021/06/01
//************************************
void CS57Control::renderCanvas()
{
	m_pS57CanvasImage->setTask(m_pLayerTask);
	m_pS57CanvasImage->render();

	updateCanvas();
	emit canvasRefreshed();
}

//************************************
// 返回值:  void 
// 简介:	窗体大小变化事件
// 参数:	QResizeEvent * event 
// 日期：	2021/06/01
//************************************
void CS57Control::resizeEvent(QResizeEvent *event)
{
	QGraphicsView::resizeEvent(event);
	
	refreshCanvas();
	m_pS57McovrRegionLegend->move(width() - m_pS57McovrRegionLegend->getWidth(), height() - m_pS57McovrRegionLegend->getHight());
}

//************************************
// 返回值:  void 
// 简介:	刷新画布
// 日期：	2021/06/01
//************************************
void CS57Control::refreshCanvas()
{
	QSize lastSize = viewport()->size();
	m_pS57Scene->setSceneRect(QRectF(0, 0, lastSize.width(), lastSize.height()));

	updateViewport(QRectF(0, 0, lastSize.width(), lastSize.height()));
	destoryImage();
	removeRoute();
	createCanvas();
	renderCanvas();
	
}

//************************************
// 返回值:  void 
// 简介:	销毁画布背景
// 日期：	2021/06/01
//************************************
void CS57Control::destoryImage()
{
	if (m_pS57CanvasImage != nullptr)
		delete m_pS57CanvasImage;
	m_pS57CanvasImage = nullptr;
	
}

//************************************
// 返回值:  void 
// 简介:	更新视口
// 参数:	QRectF rect 视口矩形
// 日期：	2021/06/01
//************************************
void CS57Control::updateViewport(QRectF rect)
{
	setViewport(m_dDpiX, m_dDpiY, rect);
}

//************************************
// 返回值:  void 
// 简介:	偏移视口
// 参数:	int offset_x x方向偏移像素
// 参数:	int offset_y y方向偏移像素
// 日期：	2021/06/01
//************************************
void CS57Control::offsetViewport(int offset_x, int offset_y)
{
	double vpx, vpy;
	m_pTransform->m_Viewport.convertDp2Vp(m_pTransform->m_Viewport.m_nCenterPt[0] - offset_x,
		m_pTransform->m_Viewport.m_nCenterPt[1] - offset_y,&vpx, &vpy);
	m_pTransform->m_Viewport.setCenterPtXY(vpx, vpy);
}

//************************************
// 返回值:  void 
// 简介:	放大
// 参数:	double factor 
// 日期：	2021/06/01
//************************************
void CS57Control::zoomIn(double factor)
{
	double scale = m_pTransform->m_Viewport.getDisplayScale();
	scale *= factor;

	if (scale < Min_Scale)	
		scale = Min_Scale;
	if (scale > Max_Scale)	
		scale = Max_Scale;

	m_pTransform->m_Viewport.setDisplayScale(scale);
	refreshCanvas();

	scale = m_pTransform->m_Viewport.getDisplayScale();
	emit changeScale(scale);
}

//************************************
// 返回值:  void 
// 简介:	缩小
// 参数:	double factor 
// 日期：	2021/06/01
//************************************
void CS57Control::zoomOut(double factor)
{
	double scale = m_pTransform->m_Viewport.getDisplayScale();
	scale *= factor;

	if (scale < Min_Scale)	
		scale = Min_Scale;
	if (scale > Max_Scale)	
		scale = Max_Scale;

	m_pTransform->m_Viewport.setDisplayScale(scale);
	refreshCanvas();

	scale = m_pTransform->m_Viewport.getDisplayScale();
	emit changeScale(scale);
}

//************************************
// 返回值:  void 
// 简介:	滚轮事件
// 参数:	QWheelEvent * event 
// 日期：	2021/06/01
//************************************
void CS57Control::wheelEvent(QWheelEvent *event)
{
	if (event->delta() < 0)			
		zoomIn(1.4);
	else if (event->delta() > 0)	
		zoomIn(0.7);
	else							
		event->accept();
}

//************************************
// 返回值:  void 
// 简介:	鼠标移动事件
// 参数:	QMouseEvent * event 
// 日期：	2021/06/01
//************************************
void CS57Control::mouseMoveEvent(QMouseEvent *event)
{
	QPoint movePt = event->pos();
	QString strGeoText = convertDMS(movePt);
	emit moveGeo(strGeoText);

	if (m_pMapTool)
	{
		m_pMapTool->canvasMoveEvent(event);
	}
}
//************************************
// 返回值:  void 
// 简介:	鼠标按下事件
// 参数:	QMouseEvent * event 
// 日期：	2021/06/01
//************************************
void CS57Control::mousePressEvent(QMouseEvent *event)
{
	if (m_pMapTool)
	{
		m_pMapTool->canvasPressEvent(event);
	}
}

//************************************
// 返回值:  void 
// 简介:	鼠标双击事件
// 参数:	QMouseEvent * event 
// 日期：	2021/06/01
//************************************
void CS57Control::mouseDoubleClickEvent(QMouseEvent *event)
{
	if (m_pMapTool)
	{
		m_pMapTool->canvasDoubleClickEvent(event);
	}
}
//************************************
// 返回值:  void 
// 简介:	鼠标释放操作
// 参数:	QMouseEvent * event 
// 日期：	2021/06/01
//************************************
void CS57Control::mouseReleaseEvent(QMouseEvent *event)
{
	if (m_pMapTool)
	{
		m_pMapTool->canvasReleaseEvent(event);
	}
}

CS57CanvasImage* CS57Control::getCanvasImage()
{
	return m_pS57CanvasImage;
}

CS57CanvasMap* CS57Control::getCanvasMap()
{
	return m_pS57CanvasMap;
}

//************************************
// 返回值:  QString 
// 简介:	转换度分秒
// 参数:	const QPoint & pt 像素坐标
// 日期：	2021/06/01
//************************************
QString CS57Control::convertDMS(const QPoint& pt) const
{
	QPointF radPt = m_pTransform->pixel2Bl(pt);

	QString strGeoText = "";
	QString dd, mm, ss;

	float lon = radPt.x();
	while (lon > PI)
		lon -= 2 * PI;
	while (lon < -PI)
		lon += 2 * PI;

	lon = fun_radTodmmss(lon);

	int nLon = lon * 10000;
	QString strLon = QString::number(nLon);

	int len = strLon.length();
	ss = strLon.right(2);
	mm = strLon.mid(len - 4, 2);
	dd = strLon.mid(0, len - 4);

	if (ss != 0)
		strLon = QString(" %1°%2′%3″").arg(dd).arg(mm).arg(ss);
	else if (mm != 0)
		strLon = QString(" %1°%2′").arg(dd).arg(mm);
	else
		strLon = QString(" %1°").arg(dd);

	QString strEW = "";
	if (lon > 0)
		strEW = "E";
	else if (lon < 0)
		strEW = "W";

	strGeoText = strLon + strEW + ",";

	float lat = radPt.y();
	lat = fun_radTodmmss(lat);

	int nLat = lat * 10000;
	QString strLat = QString::number(nLat);

	len = strLat.length();
	ss = strLat.right(2);
	mm = strLat.mid(len - 4, 2);
	dd = strLat.mid(0, len - 4);

	if (ss != 0)		
		strLat = QString(" %1°%2′%3″").arg(dd).arg(mm).arg(ss);
	else if (mm != 0)	
		strLat = QString(" %1°%2′").arg(dd).arg(mm);
	else				
		strLat = QString(" %1°").arg(dd);

	QString strNS = "";
	if (lat > 0)		
		strNS = "N";
	else if (lat < 0)	
		strNS = "S";

	strGeoText += strLat + strNS;

	return strGeoText;
}

//************************************
// Method:    setOperateMode
// FullName:  Control::CS57Control::setOperateMode
// Access:    public 
// Returns:   void
// Brief:	  设置操作模式，浏览模式，查询模式
// Parameter: OperateMode mode	准备设置的操作模式
//************************************
void CS57Control::setOperateMode(XT::OperateMode mode)
{
	m_eCurOperateMode = mode;
	switch (mode)
	{
	case XT::NORMAL:
		setCursor(Qt::OpenHandCursor);
		break;
	case XT::QUERY:
		setCursor(Qt::WhatsThisCursor);
		break;
	case XT::SELECT:
		m_pRouteManager->releaseAllRoute(CS57RouteManager::Temp);//海图选图模式，首先清空所有临时航迹
		setCursor(Qt::CrossCursor);
		break;
	case XT::AUVQUERY:
		setCursor(Qt::ArrowCursor);
		break;
	default:
		setCursor(Qt::OpenHandCursor);
		break;
	}
}

//************************************
// 返回值:  void 
// 简介:	设置查询模式
// 参数:	XT::QueryMode mode 
// 日期：	2021/07/01
//************************************
void CS57Control::setQueryMode(XT::QueryMode mode)
{
	m_eCurQueryMode = mode;
}

//************************************
// 方法:    setSymbolMode
// 全称:  	Control::CS57Control::setSymbolMode
// 权限:    public 
// 返回值:  void
// 简介:	设置符号模式，纸图符号，S52符号，CDC符号
// 参数: 	SymbolMode mode 准备设置的符号模式
//************************************
void CS57Control::setSymbolMode(XT::SymbolMode mode)
{
	CS57AppConfig* pConfig = CS57AppConfig::instance();
	pConfig->setSymbolMode(mode);
	pConfig->updateXml();

	m_pS57DocManager->setSymbolMode(mode);
	refreshCanvas();
}

//************************************
// Method:    fastInit
// Brief:	  通过AppConfig.xml配置文件进行快速初始化
// Returns:   void
// Author:    cl
// DateTime:  2021/08/26
//************************************
void CS57Control::fastInit()
{	
	//先释放上次的内容
	releaseOldJob();

	CS57AppConfig* pConfig = CS57AppConfig::instance();

	QString strEncDir = pConfig->getEncDir();
	XT::SymbolMode symbolMode = pConfig->getSymbolMode();
	double lon = pConfig->getStartLon();//121.825;
	double lat = pConfig->getStartLat();//pConfig->getStartLat();//38.945;
	double display = pConfig->getStartDisplayScale();//45000.000000;
	double baseLine = pConfig->getBaseLine();

	//if (strEncDir.isEmpty())
	//{
	//	QMessageBox::information(this, "提示", "AppConfig.xml中EncDir设置有误");
	//	return;
	//}

	if ((lon == 0.0 && lat == 0.0) || display < 0.0)
	{
		QMessageBox::information(this, "提示", "AppConfig.xml中Location设置有误");
		return;
	}

	if (display < 500.0) display = 500.0;

	setEncDir(strEncDir);
	setSymbolMode(symbolMode);
	loadEnc();

	setProjectionName(XT::MCT);

	setBaseLine(fun_dmmssTorad(baseLine));

	setViewportCenterPt(lon, lat);
	setViewportDisplayScale(display);
	setViewport(m_dDpiX, m_dDpiY, sceneRect());
}

//************************************
// Method:    createCanvas
// Brief:	  创建画布
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57Control::createCanvas()
{
	destoryImage();

	m_eCurSelectChartType = CS57DocManager::SELECT_DISPLAY;
	double scale = m_pTransform->m_Viewport.getDisplayScale();
	m_pS57DocManager->selectChart(scale, m_eCurSelectChartType);
	m_pS57CanvasImage = new CS57CanvasImage(m_CanvasColor, viewport()->size(), m_dDpiX, m_dDpiY);

	//图幅层
	CS57CellLayer::sCellParameters cellParameters;
	cellParameters.symbolMode = m_pS57DocManager->getSymbolMode();
	CS57CellLayer::sCellDataSource cellDataSource;
	cellDataSource.pGlobalMapProvider = m_pS57DocManager->getGlobalMapProviderPtr();
	cellDataSource.pMcovrProvider = m_pS57DocManager->getMcovrProviderPtr();
	cellDataSource.pCellProvider = m_pS57DocManager->getCellProviderPtr();
	m_pCellLayer->setCellParameter(cellParameters);
	m_pCellLayer->setDataSource(cellDataSource);
	m_pCellLayer->setLayerVisible(m_bCellLayerVisible);
	m_pCellLayer->setBorderVisible(m_bCellBorderVisible);
	m_pCellLayer->setSymbolScaleMode(m_eCurSymbolScaleMode);

	m_pLayerTask->addLayer(m_pCellLayer);

	//高亮层
	CS57HighlightLayer::sHighlightParameters highlightParameters;
	highlightParameters.operateMode = m_eCurOperateMode;
	highlightParameters.queryMode = m_eCurQueryMode;
	CS57HighlightLayer::sHighlightDataSource highlightDataSource;
	highlightDataSource.pHighlightFeature = m_pS57Analysis->getFeatureQueryPtr()->getHighlightFeature();
	m_pHighlightLayer->setHighlightParameter(highlightParameters);
	m_pHighlightLayer->setDataSource(highlightDataSource);

	m_pLayerTask->addLayer(m_pHighlightLayer);

	//自定义层
	CS57CustomLayer::sAuxiliaryParameters auxiliaryParameters;
	auxiliaryParameters.enabled = false;
	auxiliaryParameters.centerPt = m_pS57CanvasImage->centerPt();
	m_pCustomLayer->setAuxiliaryParameter(auxiliaryParameters);

	m_pLayerTask->addLayer(m_pCustomLayer);

	//Mcovr分区层
	CS57McovrRegionLayer::sMcovrRegionParameters mcovrRegionParameters;
	CS57McovrRegionLayer::sMcoverRegionDataSource mcovrRegionDataSource;
	mcovrRegionDataSource.pMcovrRegionProvider = m_pS57DocManager->getMcovrRegionProviderPtr();
	mcovrRegionParameters.mcovrRegionI.lineColor = QColor(71, 71, 61);
	mcovrRegionParameters.mcovrRegionI.fillColor = QColor(0, 0, 255, 50);
	mcovrRegionParameters.mcovrRegionII.lineColor = QColor(71, 71, 61);
	mcovrRegionParameters.mcovrRegionII.fillColor = QColor(0, 255, 255, 50);
	mcovrRegionParameters.mcovrRegionIII.lineColor = QColor(71, 71, 61);
	mcovrRegionParameters.mcovrRegionIII.fillColor = QColor(0, 255, 0, 50);
	mcovrRegionParameters.mcovrRegionIV.lineColor = QColor(71, 71, 61);
	mcovrRegionParameters.mcovrRegionIV.fillColor = QColor(255, 255, 0, 50);
	mcovrRegionParameters.mcovrRegionV.lineColor = QColor(71, 71, 61);
	mcovrRegionParameters.mcovrRegionV.fillColor = QColor(255, 0, 0, 50);
	m_pMcovrRegionLayer->setMcovrRegionParameter(mcovrRegionParameters);
	m_pMcovrRegionLayer->setDataSource(mcovrRegionDataSource);

	m_pLayerTask->addLayer(m_pMcovrRegionLayer);
}

//************************************
// Method:    releaseOldJob
// Brief:	  释放旧的任务
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57Control::releaseOldJob()
{
	//释放画布
	destoryImage();
	removeRoute();
	m_pS57DocManager->releaseOldJob();
}

//************************************
// Method:    highlightItem
// Brief:	  高亮元素
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: int row
//************************************
void CS57Control::highlightItem(int row)
{
	m_pS57Analysis->getFeatureQueryPtr()->startHighlight();
	m_pS57Analysis->getFeatureQueryPtr()->setHighlightRow(row);
	refreshCanvas();
	m_pS57Analysis->getFeatureQueryPtr()->stopHighlight();
}

//************************************
// Method:    startDisplayScale
// Brief:	  获取启动时显示比例尺
// Returns:   int
// Author:    cl
// DateTime:  2021/08/26
//************************************
int CS57Control::startDisplayScale() const
{
	CS57AppConfig* pConfig = CS57AppConfig::instance();
	
	return pConfig->getStartDisplayScale();
}

//************************************
// Method:    unloadEnc
// Brief:	  卸载ENC
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57Control::unloadEnc()
{
	releaseOldJob();
}

//************************************
// Method:    viewportLeft
// Brief:	  视口左移
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: int offset
//************************************
void CS57Control::viewportLeft(int offset /*= -100*/)
{
	double x, y;
	m_pTransform->m_Viewport.convertDp2Vp(
		m_pTransform->m_Viewport.m_nCenterPt[0] + offset,
		m_pTransform->m_Viewport.m_nCenterPt[1] + 0, &x, &y);
	m_pTransform->m_Viewport.setCenterPtXY(x, y);

	refreshCanvas();
}

//************************************
// Method:    viewportRight
// Brief:	  视口右移
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: int offset
//************************************
void CS57Control::viewportRight(int offset /*= 100*/)
{
	double x, y;
	m_pTransform->m_Viewport.convertDp2Vp(
		m_pTransform->m_Viewport.m_nCenterPt[0] + offset,
		m_pTransform->m_Viewport.m_nCenterPt[1] + 0, &x, &y);
	m_pTransform->m_Viewport.setCenterPtXY(x, y);

	refreshCanvas();
}

//************************************
// Method:    viewportUp
// Brief:	  视口上移
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: int offset
//************************************
void CS57Control::viewportUp(int offset /*= -100*/)
{
	double x, y;
	m_pTransform->m_Viewport.convertDp2Vp(
		m_pTransform->m_Viewport.m_nCenterPt[0] + 0,
		m_pTransform->m_Viewport.m_nCenterPt[1] + offset, &x, &y);
	m_pTransform->m_Viewport.setCenterPtXY(x, y);

	refreshCanvas();
}

//************************************
// Method:    viewportDown
// Brief:	  视口下移
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: int offset
//************************************
void CS57Control::viewportDown(int offset /*= 100*/)
{
	double x, y;
	m_pTransform->m_Viewport.convertDp2Vp(
		m_pTransform->m_Viewport.m_nCenterPt[0] + 0, 
		m_pTransform->m_Viewport.m_nCenterPt[1] + offset, &x, &y);
	m_pTransform->m_Viewport.setCenterPtXY(x, y);

	refreshCanvas();
}

//************************************
// Method:    setDisplayCategory
// Brief:	  设置显示分类
// Returns:   void
// Author:    cl
// DateTime:  2021/08/26
// Parameter: XT::DisplayCategory displayCategory
//************************************
void CS57Control::setDisplayCategory(XT::DisplayCategory displayCategory)
{
	m_pS57DocManager->setDisplayCategory(displayCategory);

	refreshCanvas();
}

//************************************
// Method:    setGlobalMapVisible
// Brief:	  设置全球底图可见性
// Returns:   void
// Author:    cl
// DateTime:  2021/08/26
// Parameter: bool visible
//************************************
void CS57Control::setGlobalMapVisible(bool visible)
{
	CS57CellLayer::sGlobalMapParameters parameters = m_pCellLayer->getGlobalMapParameter();
	parameters.visible = visible;
	m_pCellLayer->setGlobalMapParameter(parameters);

	refreshCanvas();
}

//************************************
// Method:    setCanvasColor
// Brief:	  设置画布颜色
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QColor color
//************************************
void CS57Control::setCanvasColor(QColor color)
{
	m_CanvasColor = color;
}

//************************************
// Method:    viewportCells
// Brief:	  获取视口内图幅列表
// Returns:   void
// Author:    cl
// DateTime:  2021/08/24
//************************************
void CS57Control::viewportCells()
{
	m_vecViewportCells.clear();

	for (int i = 0; i < m_pS57DocManager->getCellProviderPtr()->m_vecCellDisplayContext.size(); i++)
	{
		CS57Cell* pCell = m_pS57DocManager->getCellProviderPtr()->m_vecCellDisplayContext[i]->pS57Cell;
		m_vecViewportCells.push_back(pCell);
	}
}
//************************************
// Method:    screenShot
// Brief:	  屏幕截图，支持高分辨率到1200dpi
// Returns:   void
// Author:    cl
// DateTime:  2021/08/16
// Parameter: const QString & imgPath
// Parameter: int dpi
//************************************
void CS57Control::screenCapture(XT::sExportParameters exportParameters)
{
	//保存旧的DPI
	double oldDpiX = m_dDpiX;
	double oldDpiY = m_dDpiY;

	int dpi = exportParameters.dpi;
	QString strExportPath = exportParameters.exportPath;

	if (dpi == -1)//本机默认
		dpi = m_dDpiX;

	if (dpi < 1 || dpi > 1200)
		return;

	updateExportCellProgress(0);

	//根据dpi重新定义纸张大小
	QSize lastSize = viewport()->size();
	int oldWidth = lastSize.width();
	int oldHeight = lastSize.height();
	int newWidth = oldWidth * dpi / m_dDpiX;
	int newHeight = oldHeight * dpi / m_dDpiY;

	//qDebug() << "CS57Control:" << newWidth << "," << newHeight << endl;

	//更新视口
	m_dDpiX = dpi;
	m_dDpiY = dpi;
	updateViewport(QRectF(0, 0, newWidth, newHeight));

	updateExportCellProgress(10);

	//重新创建画布
	m_eCurSelectChartType = CS57DocManager::SELECT_PRINT;
	double scale = m_pTransform->m_Viewport.getDisplayScale();
	m_pS57DocManager->selectChart(scale, CS57DocManager::SELECT_PRINT);
	CS57CanvasImage* pCanvasImage = new CS57CanvasImage(m_CanvasColor, QSize(newWidth, newHeight), m_dDpiX, m_dDpiY);

	//绘制
	CS57CellLayer::sCellParameters parameters;
	parameters.symbolMode = m_pS57DocManager->getSymbolMode();
	CS57CellLayer::sCellDataSource dataSource;
	dataSource.pGlobalMapProvider = m_pS57DocManager->getGlobalMapProviderPtr();
	dataSource.pMcovrProvider = m_pS57DocManager->getMcovrProviderPtr();
	dataSource.pCellProvider = m_pS57DocManager->getCellProviderPtr();
	m_pCellLayer->setCellParameter(parameters);
	m_pCellLayer->setDataSource(dataSource);
	m_pCellLayer->setSymbolScaleMode(m_eCurSymbolScaleMode);

	m_pLayerTask->addLayer(m_pCellLayer);

	updateExportCellProgress(20);
	pCanvasImage->setTask(m_pLayerTask);
	pCanvasImage->render();
	updateExportCellProgress(80);

	QImage* pImage = pCanvasImage->canvasImage();
	pImage->save(strExportPath);
	updateExportCellProgress(100);

	//释放
	if (pCanvasImage != nullptr)
		delete pCanvasImage;
	pCanvasImage = nullptr;

	//还原设置并重绘
	m_dDpiX = oldDpiX;
	m_dDpiY = oldDpiY;

	updateCanvas();
}

//************************************
// Method:    exportImage
// Brief:	  导出单幅图图像
// Returns:   void
// Author:    cl
// DateTime:  2021/08/18
// Parameter: const QString & imgPath
// Parameter: int dpi
//************************************
void CS57Control::exportImage1(XT::sExportParameters exportParameters)
{
	//保存旧的DPI
	double oldDpiX = m_dDpiX;
	double oldDpiY = m_dDpiY;

	double oldCenterVpX = m_pTransform->m_Viewport.m_dCenterPt[0];
	double oldCenterVpY = m_pTransform->m_Viewport.m_dCenterPt[1];

	int dpi = exportParameters.dpi;
	QString cellName = exportParameters.cellName;
	QString strExportPath = exportParameters.exportPath;
	double leftMargin = exportParameters.leftMargin;
	double topMargin = exportParameters.topMargin;
	double rightMargin = exportParameters.rightMargin;
	double bottomMargin = exportParameters.bottomMargin;
	bool oldBorderVisible = m_bCellBorderVisible;

	viewportCells();
	CS57Cell* pCell = nullptr;
	for (int i = 0; i < m_vecViewportCells.size(); i++)
	{
		QString strCellName = m_vecViewportCells[i]->getCellName();
		strCellName = strCellName.replace(".000", "");
		if (strCellName == cellName)
			pCell = m_vecViewportCells[i];
	}
	if (pCell == nullptr)
		return;//失败

	if (dpi == -1)//本机默认
		dpi = m_dDpiX;

	if (dpi < 1 || dpi > 1200)
		return;

	updateExportCellProgress(0);

	QSize lastSize = viewport()->size();
	int oldWidth = lastSize.width();
	int oldHeight = lastSize.height();

	//调整比例尺
	double cscl = pCell->getCellCscl();
	setViewportDisplayScale(cscl);
	updateCanvas();

	//根据dpi重新定义纸张大小
	double left = pCell->getCellExtent().xMinimum();
	double top = pCell->getCellExtent().yMaximum();
	double right = pCell->getCellExtent().xMaximum();
	double bottom = pCell->getCellExtent().yMinimum();

	//qDebug() << "left:" << left << "," << "top:" << top << "," << "right;" << right << "," << "bottom:" << bottom << endl;

	QPointF blLeftTop(left / RO, top / RO);
	QPointF blRightTop(right / RO, top / RO);
	QPointF blLeftBottom(left / RO, bottom / RO);
	QPointF blRightBottom(right / RO, bottom / RO);

	QPointF vpLeftTop = m_pTransform->bl2Vp(blLeftTop);
	QPointF vpRightTop = m_pTransform->bl2Vp(blRightTop);
	QPointF vpLeftBottom = m_pTransform->bl2Vp(blLeftBottom);
	QPointF vpRightBottom = m_pTransform->bl2Vp(blRightBottom);

	double dCellWidthCM = (vpRightTop.x() - vpLeftTop.x()) / cscl * 100.0 + leftMargin + rightMargin;//左右各扩展3厘米
	double dCellHeightCM = (vpRightTop.y() - vpRightBottom.y()) / cscl * 100.0 + topMargin + bottomMargin;//上下各扩展3厘米
	double dCellWidthMM = dCellWidthCM * 10.0;
	double dCellHeightMM = dCellHeightCM * 10.0;

	int dpCellWidth = round(dCellWidthMM * dpi / 25.40);
	int dpCellHeight = round(dCellHeightMM * dpi / 25.40);

	//计算新的纸张中心点
	QPointF newVpLeftBottom(vpLeftBottom.x() - leftMargin / 100.0, vpLeftBottom.y() - bottomMargin / 100.0);
	QPointF newVpRightTop(vpRightTop.x() + rightMargin / 100.0, vpRightTop.y() + topMargin / 100.0);
	double dCenterVpX = (newVpLeftBottom.x() + newVpRightTop.x()) / 2.0;
	double dCenterVpY = (newVpLeftBottom.y() + newVpRightTop.y()) / 2.0;
	m_pTransform->m_Viewport.setCenterPtXY(dCenterVpX, dCenterVpY);//投影坐标

	//更新视口
	m_dDpiX = dpi;
	m_dDpiY = dpi;
	updateViewport(QRectF(0, 0, dpCellWidth, dpCellHeight));//绘图机的宽高

	updateExportCellProgress(30);

	//重新创建画布
	QStringList lstCellNames;
	lstCellNames.append(pCell->getCellName());
	m_pS57DocManager->selectChart(cscl, lstCellNames);
	updateExportCellProgress(50);
	CS57CanvasImage* pCanvasImage = new CS57CanvasImage(QColor(255,255,255), QSize(dpCellWidth, dpCellHeight), m_dDpiX, m_dDpiY);

	//绘制
	CS57CellLayer::sCellParameters parameters;
	parameters.symbolMode = m_pS57DocManager->getSymbolMode();
	CS57CellLayer::sCellDataSource dataSource;
	//dataSource.pGlobalMapProvider = m_pS57DocManager->getGlobalMapProviderPtr();//当前模式可以不用开启
	//dataSource.pMcovrProvider = m_pS57DocManager->getMcovrProviderPtr();//当前模式可以不用开启
	dataSource.pCellProvider = m_pS57DocManager->getCellProviderPtr();
	m_pCellLayer->setCellParameter(parameters);
	m_pCellLayer->setDataSource(dataSource);
	m_pCellLayer->setBorderVisible(true);
	m_pCellLayer->setSymbolScaleMode(m_eCurSymbolScaleMode);

	m_pLayerTask->addLayer(m_pCellLayer);
	pCanvasImage->setTask(m_pLayerTask);
	pCanvasImage->render();
	updateExportCellProgress(80);

	QImage* pImage = pCanvasImage->canvasImage();
	pImage->save(strExportPath);
	updateExportCellProgress(100);

	//释放
	if (pCanvasImage != nullptr)
		delete pCanvasImage;
	pCanvasImage = nullptr;

	//还原设置并重绘
	//m_pTransform->m_Viewport.lockDisplayScale(false);//关闭锁定比例尺
	m_pTransform->m_Viewport.setCenterPtXY(oldCenterVpX, oldCenterVpY);//投影坐标
	m_dDpiX = oldDpiX;
	m_dDpiY = oldDpiY;

	setBorderVisible(oldBorderVisible);
}
//尝试更改之前方案
//************************************
// Method:    exportImage
// Brief:	  导出图像
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: XT::sExportParameters exportParameters
//************************************
void CS57Control::exportImage(XT::sExportParameters exportParameters)
{
	double oldBaseLineLat = m_pTransform->m_Proj.getBaseLine();
	//第一步更新基准纬线，否则下面都是错的
	int baseLineBD = exportParameters.baseLineBD;
	int baseLineBM = exportParameters.baseLineBM;
	int baseLineBS = exportParameters.baseLineBS;
	QString strBaseLine = QString("%1.%2%3")
		.arg(QString::number(baseLineBD))
		.arg(QString::number(baseLineBM))
		.arg(QString::number(baseLineBS));
	setBaseLine(fun_dmmssTorad(strBaseLine.toDouble()));

	//左下地理坐标点
	int lbLD = exportParameters.lbLD;
	int lbLM = exportParameters.lbLM;
	float lbLS = exportParameters.lbLS;
	int lbBD = exportParameters.lbBD;
	int lbBM = exportParameters.lbBM;
	float lbBS = exportParameters.lbBS;
	double lbLon = 0.0,lbLat = 0.0;
	m_pTransform->dms2Geo(lbLD, lbLM, lbLS, lbLon);
	m_pTransform->dms2Geo(lbBD, lbBM, lbBS, lbLat);
	QPointF lbPt(lbLon, lbLat);

	//右上地理坐标点
	int rtLD = exportParameters.rtLD;
	int rtLM = exportParameters.rtLM;
	float rtLS = exportParameters.rtLS;
	int rtBD = exportParameters.rtBD;
	int rtBM = exportParameters.rtBM;
	float rtBS = exportParameters.rtBS;
	double rtLon = 0.0, rtLat = 0.0;
	m_pTransform->dms2Geo(rtLD, rtLM, rtLS, rtLon);
	m_pTransform->dms2Geo(rtBD, rtBM, rtBS, rtLat);
	QPointF rtPt(rtLon, rtLat);

	QPointF lbVp = m_pTransform->geo2Vp(lbPt);
	QPointF rtVp = m_pTransform->geo2Vp(rtPt);

	double displayScale = exportParameters.displayScale;
	double dpi = exportParameters.dpi;
	QString cellName = exportParameters.cellName;
	QString strExportPath = exportParameters.exportPath;
	double leftMargin = exportParameters.leftMargin;
	double topMargin = exportParameters.topMargin;
	double rightMargin = exportParameters.rightMargin;
	double bottomMargin = exportParameters.bottomMargin;

	double userWidthCM = qAbs(rtVp.x() - lbVp.x()) / displayScale * 100.0;
	double userHeightCM = qAbs(rtVp.y() - lbVp.y()) / displayScale * 100.0;

	double userTotalWidthCM = userWidthCM + leftMargin + rightMargin;//左右各扩展3厘米
	double userTotalHeightCM = userHeightCM + topMargin + bottomMargin;//上下各扩展3厘米

	if (userTotalWidthCM >= 118.9 || userTotalHeightCM >= 84.1)//A0纸张大小
	{
		QString strW = "输入宽度 = " + QString::number(userTotalWidthCM) + "cm";
		QString strH = "输入高度 = " + QString::number(userTotalHeightCM) + "cm";
		QString msg = strW + "\n" + strH + "\n";
		//超出A0纸张宽高
		QMessageBox::information(this, "提示", msg + "输入打印范围超出A0纸张的宽度或高度");
		return;
	}

	if (dpi == -1.0)//本机默认
		dpi = m_dDpiX;

	if (dpi < 1.0 || dpi > 1200.0)
		return;

	double userTotalWidthMM = userTotalWidthCM * 10.0;
	double userTotalHeightMM = userTotalHeightCM * 10.0;

	int userTotalWidthDP = round(userTotalWidthMM * dpi / 25.40);
	int userTotalHeightDP = round(userTotalHeightMM * dpi / 25.40);

	//保存旧的DPI
	double oldDpiX = m_dDpiX;
	double oldDpiY = m_dDpiY;

	double oldCenterVpX = m_pTransform->m_Viewport.m_dCenterPt[0];
	double oldCenterVpY = m_pTransform->m_Viewport.m_dCenterPt[1];

	double oldDisplayScale = m_pTransform->m_Viewport.getDisplayScale();
	bool oldBorderVisible = m_bCellBorderVisible;

	QSize oldViewportSize = viewport()->size();

	updateExportCellProgress(0);

	double dCenterVpX = (lbVp.x() + rtVp.x()) / 2.0;
	double dCenterVpY = (lbVp.y() + rtVp.y()) / 2.0;
	m_pTransform->m_Viewport.setCenterPtXY(dCenterVpX, dCenterVpY);//投影坐标
	m_dDpiX = dpi;
	m_dDpiY = dpi;

	updateViewport(QRectF(0, 0, userTotalWidthDP, userTotalHeightDP));//用户输入宽高
	setViewportDisplayScale(displayScale);

	updateExportCellProgress(20);

	CSpatialPolygon polygon;
	QVector<QVector<QPointF>*> vpRings;
	QVector<QPointF> vpRing;
	QPointF pt[4];
	pt[0] = QPointF(lbPt.x(), lbPt.y());
	pt[1] = QPointF(lbPt.x(), rtPt.y());
	pt[2] = QPointF(rtPt.x(), rtPt.y());
	pt[3] = QPointF(rtPt.x(), lbPt.y());
	vpRing.push_back(pt[0]);
	vpRings.push_back(&vpRing);
	polygon.setPolygon(vpRings);

	//开始选图
	m_pS57DocManager->selectChart(displayScale, &polygon);
	polygon.relPolygon();//释放空间多边形
	updateExportCellProgress(40);

	//创建两张内存图像
	QImage* pImage = new QImage(QSize(userTotalWidthDP, userTotalHeightDP), QImage::Format_ARGB32);
	pImage->setDevicePixelRatio(1.0);
	pImage->setDotsPerMeterX(1.0 * 1000 * dpi / 25.4);
	pImage->setDotsPerMeterY(1.0 * 1000 * dpi / 25.4);
	pImage->fill(QColor(255, 255, 255));
	QPainter* pPainter = new QPainter(pImage);
	pPainter->setClipping(true);
	updateExportCellProgress(60);
	//此处不能通过页边距计算裁剪范围
	int left, bottom, right, top;
	m_pTransform->m_Viewport.convertVp2Dp(lbVp.x(), lbVp.y(), &left, &bottom);
	m_pTransform->m_Viewport.convertVp2Dp(rtVp.x(), rtVp.y(), &right, &top);
	QRect rect(left, top, right - left, bottom - top);
	pPainter->setClipRect(rect,Qt::ClipOperation::ReplaceClip);

	CS57CanvasImage* pCanvasImage = new CS57CanvasImage(pImage, pPainter);
	updateExportCellProgress(70);
	//绘制
	CS57CellLayer::sCellParameters parameters;
	parameters.symbolMode = m_pS57DocManager->getSymbolMode();
	CS57CellLayer::sCellDataSource dataSource;
	dataSource.pCellProvider = m_pS57DocManager->getCellProviderPtr();
	m_pCellLayer->setCellParameter(parameters);
	m_pCellLayer->setDataSource(dataSource);
	m_pCellLayer->setBorderVisible(true);
	m_pCellLayer->setRenderType(XT::RENDER_PRINT);
	m_pCellLayer->setPrintParameters(&exportParameters);
	m_pCellLayer->setSymbolScaleMode(m_eCurSymbolScaleMode);
	m_pLayerTask->addLayer(m_pCellLayer);
	pCanvasImage->setTask(m_pLayerTask);
	pCanvasImage->render();
	updateExportCellProgress(80);

	pImage->save(strExportPath);
	updateExportCellProgress(100);
	//释放
	if (pCanvasImage != nullptr)
		delete pCanvasImage;
	pCanvasImage = nullptr;

	//还原设置并重绘
	m_pTransform->m_Viewport.setCenterPtXY(oldCenterVpX, oldCenterVpY);//投影坐标
	updateViewport(QRectF(0, 0, oldViewportSize.width(), oldViewportSize.height()));//用户输入宽高
	setViewportDisplayScale(oldDisplayScale);
	m_dDpiX = oldDpiX;
	m_dDpiY = oldDpiY;

	setBorderVisible(oldBorderVisible);
	refreshCanvas();
}

//************************************
// Method:    exportPdf
// Brief:	  导出单幅图PDF文件
// Returns:   void
// Author:    cl
// DateTime:  2021/08/25
// Parameter: sExportParameters exportParameters
//************************************
void CS57Control::exportPdf1(XT::sExportParameters exportParameters)
{
	//保存旧的DPI
	double oldDpiX = m_dDpiX;
	double oldDpiY = m_dDpiY;
	double oldCenterVpX = m_pTransform->m_Viewport.m_dCenterPt[0];
	double oldCenterVpY = m_pTransform->m_Viewport.m_dCenterPt[1];
	bool oldBorderVisible = m_bCellBorderVisible;
	double oldBaseLineLat = m_pTransform->m_Proj.getBaseLine();

	int dpi = exportParameters.dpi;
	dpi = 300;
	QString strCellNo = exportParameters.cellName;
	QString strExportPath = exportParameters.exportPath;
	double leftMargin = exportParameters.leftMargin;
	double topMargin = exportParameters.topMargin;
	double rightMargin = exportParameters.rightMargin;
	double bottomMargin = exportParameters.bottomMargin;
	QString strBorderCellName = exportParameters.cellName;

	//设置基准纬线
	int baseLineBD = exportParameters.baseLineBD;
	int baseLineBM = exportParameters.baseLineBM;
	float baseLineBS = exportParameters.baseLineBS;
	double baseLineLat = fabs(baseLineBD) +
						+fabs(baseLineBM / 60.) +
						fabs(baseLineBS / 3600.);
	if (baseLineBM < 0 || baseLineBS < 0)
	{
		//参数输入有误
		return;
	}
	if (baseLineBD < 0)
	{
		baseLineLat = -abs(baseLineLat);
	}
	CS57Transform::instance()->m_Proj.setBaseLine(baseLineLat / RO);
	updateCanvas();

	viewportCells();
	CS57Cell* pCell = nullptr;
	for (int i = 0; i < m_vecViewportCells.size(); i++)
	{
		QString strCellName = m_vecViewportCells[i]->getCellName();
		strCellName = strCellName.replace(".000", "");
		if (strCellName == strCellNo)
			pCell = m_vecViewportCells[i];
	}
	if (pCell == nullptr)
		return;//失败

	if (dpi == -1)//本机默认
		dpi = m_dDpiX;

	if (dpi < 1 || dpi > 1200)
		return;

	updateExportCellProgress(0);

	QSize lastSize = viewport()->size();
	int oldWidth = lastSize.width();
	int oldHeight = lastSize.height();

	//调整比例尺
	double cscl = pCell->getCellCscl();
	setViewportDisplayScale(cscl);
	updateCanvas();

	QPrinter printer(QPrinter::HighResolution);
	printer.setPaperSize(QPrinter::Custom);
	printer.setOutputFormat(QPrinter::PdfFormat);
	printer.setOutputFileName(strExportPath);
	printer.setOrientation(QPrinter::Portrait);//横向

	//根据dpi重新定义纸张大小
	double left = pCell->getCellExtent().xMinimum();
	double top = pCell->getCellExtent().yMaximum();
	double right = pCell->getCellExtent().xMaximum();
	double bottom = pCell->getCellExtent().yMinimum();


	QPointF blLeftTop(left / RO, top / RO);
	QPointF blRightTop(right / RO, top / RO);
	QPointF blLeftBottom(left / RO, bottom / RO);
	QPointF blRightBottom(right / RO, bottom / RO);

	QPointF vpLeftTop = m_pTransform->bl2Vp(blLeftTop);
	QPointF vpRightTop = m_pTransform->bl2Vp(blRightTop);
	QPointF vpLeftBottom = m_pTransform->bl2Vp(blLeftBottom);
	QPointF vpRightBottom = m_pTransform->bl2Vp(blRightBottom);

	double dCellWidthCM = (vpRightTop.x() - vpLeftTop.x()) / cscl * 100.0 + leftMargin + rightMargin;//左右各扩展*厘米
	double dCellHeightCM = (vpRightTop.y() - vpRightBottom.y()) / cscl * 100.0 + topMargin + bottomMargin;//上下各扩展*厘米
	double dCellWidthMM = dCellWidthCM * 10.0;
	double dCellHeightMM = dCellHeightCM * 10.0;

	dpi = printer.resolution();

	int dpCellWidth = round(dCellWidthMM * dpi / 25.40);
	int dpCellHeight = round(dCellHeightMM * dpi / 25.40);

	//qDebug() << "dpCellWidth:" << dpCellWidth << "," << "dpCellHeight:" << dpCellHeight << endl;

	printer.setPaperSize(QSize(dpCellWidth, dpCellHeight), QPrinter::DevicePixel);

	//计算新的纸张中心点
	QPointF newVpLeftBottom(vpLeftBottom.x() - leftMargin / 100.0, vpLeftBottom.y() - bottomMargin / 100.0);
	QPointF newVpRightTop(vpRightTop.x() + rightMargin / 100.0, vpRightTop.y() + topMargin / 100.0);
	double dCenterVpX = (newVpLeftBottom.x() + newVpRightTop.x()) / 2.0;
	double dCenterVpY = (newVpLeftBottom.y() + newVpRightTop.y()) / 2.0;
	m_pTransform->m_Viewport.setCenterPtXY(dCenterVpX, dCenterVpY);//投影坐标

	//更新视口
	m_dDpiX = dpi;
	m_dDpiY = dpi;
	updateViewport(QRectF(0, 0, dpCellWidth, dpCellHeight));//绘图机的宽高

	updateExportCellProgress(20);

	//重新创建画布
	QStringList lstCellNames;
	lstCellNames.append(pCell->getCellName());
	m_pS57DocManager->selectChart(cscl, lstCellNames);

	updateExportCellProgress(50);

	//绘制
	CS57CellLayer::sCellParameters cellLayerParameters;
	cellLayerParameters.symbolMode = m_pS57DocManager->getSymbolMode();
	CS57CellLayer::sCellDataSource cellLayerDataSource;
	//dataSource.pGlobalMapProvider = m_pS57DocManager->getGlobalMapProviderPtr();//打印模式不绘制
	//dataSource.pMcovrProvider = m_pS57DocManager->getMcovrProviderPtr();//打印模式不绘制
	cellLayerDataSource.pCellProvider = m_pS57DocManager->getCellProviderPtr();
	m_pCellLayer->setCellParameter(cellLayerParameters);
	m_pCellLayer->setDataSource(cellLayerDataSource);
	m_pCellLayer->setBorderVisible(true);
	m_pCellLayer->setSymbolScaleMode(m_eCurSymbolScaleMode);
	QPainter painter(&printer);
	m_pCellLayer->render(&painter);

	updateExportCellProgress(100);

	//还原设置并重绘
	//m_pTransform->m_Viewport.lockDisplayScale(false);//关闭锁定比例尺
	m_pTransform->m_Viewport.setCenterPtXY(oldCenterVpX, oldCenterVpY);//投影坐标
	m_dDpiX = oldDpiX;
	m_dDpiY = oldDpiY;

	CS57Transform::instance()->m_Proj.setBaseLine(oldBaseLineLat);
	setBorderVisible(oldBorderVisible);
}

//************************************
// Method:    exportPdf
// Brief:	  导出PDF
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: XT::sExportParameters exportParameters
//************************************
void CS57Control::exportPdf(XT::sExportParameters exportParameters)
{
	double oldBaseLineLat = m_pTransform->m_Proj.getBaseLine();
		//第一步更新基准纬线，否则下面都是错的
	int baseLineBD = exportParameters.baseLineBD;
	int baseLineBM = exportParameters.baseLineBM;
	int baseLineBS = exportParameters.baseLineBS;
	QString strBaseLine = QString("%1.%2%3")
		.arg(QString::number(baseLineBD))
		.arg(QString::number(baseLineBM))
		.arg(QString::number(baseLineBS));
	setBaseLine(fun_dmmssTorad(strBaseLine.toDouble()));

	//左下地理坐标点
	int lbLD = exportParameters.lbLD;
	int lbLM = exportParameters.lbLM;
	float lbLS = exportParameters.lbLS;
	int lbBD = exportParameters.lbBD;
	int lbBM = exportParameters.lbBM;
	float lbBS = exportParameters.lbBS;
	double lbLon = 0.0,lbLat = 0.0;
	m_pTransform->dms2Geo(lbLD, lbLM, lbLS, lbLon);
	m_pTransform->dms2Geo(lbBD, lbBM, lbBS, lbLat);
	QPointF lbPt(lbLon, lbLat);

	//右上地理坐标点
	int rtLD = exportParameters.rtLD;
	int rtLM = exportParameters.rtLM;
	float rtLS = exportParameters.rtLS;
	int rtBD = exportParameters.rtBD;
	int rtBM = exportParameters.rtBM;
	float rtBS = exportParameters.rtBS;
	double rtLon = 0.0, rtLat = 0.0;
	m_pTransform->dms2Geo(rtLD, rtLM, rtLS, rtLon);
	m_pTransform->dms2Geo(rtBD, rtBM, rtBS, rtLat);
	QPointF rtPt(rtLon, rtLat);

	QPointF lbVp = m_pTransform->geo2Vp(lbPt);
	QPointF rtVp = m_pTransform->geo2Vp(rtPt);

	double displayScale = exportParameters.displayScale;
	double dpi = exportParameters.dpi;
	QString cellName = exportParameters.cellName;
	QString strExportPath = exportParameters.exportPath;
	double leftMargin = exportParameters.leftMargin;
	double topMargin = exportParameters.topMargin;
	double rightMargin = exportParameters.rightMargin;
	double bottomMargin = exportParameters.bottomMargin;

	double userWidthCM = qAbs(rtVp.x() - lbVp.x()) / displayScale * 100.0;
	double userHeightCM = qAbs(rtVp.y() - lbVp.y()) / displayScale * 100.0;

	double userTotalWidthCM = userWidthCM + leftMargin + rightMargin;//左右各扩展3厘米
	double userTotalHeightCM = userHeightCM + topMargin + bottomMargin;//上下各扩展3厘米

	if (userTotalWidthCM >= 118.9 || userTotalHeightCM >= 84.1)//A0纸张大小
	{
		QString strW = "输入宽度 = " + QString::number(userTotalWidthCM) + "cm";
		QString strH = "输入高度 = " + QString::number(userTotalHeightCM) + "cm";
		QString msg = strW + "\n" + strH + "\n";
		//超出A0纸张宽高
		QMessageBox::information(this, "提示", msg + "输入打印范围超出A0纸张的宽度或高度");
		return;
	}

	if (dpi == -1.0)//本机默认
		dpi = m_dDpiX;

	if (dpi < 1.0 || dpi > 1200.0)
		return;

	double userTotalWidthMM = userTotalWidthCM * 10.0;
	double userTotalHeightMM = userTotalHeightCM * 10.0;

	int userTotalWidthDP = round(userTotalWidthMM * dpi / 25.40);
	int userTotalHeightDP = round(userTotalHeightMM * dpi / 25.40);

	//保存旧的DPI
	double oldDpiX = m_dDpiX;
	double oldDpiY = m_dDpiY;

	double oldCenterVpX = m_pTransform->m_Viewport.m_dCenterPt[0];
	double oldCenterVpY = m_pTransform->m_Viewport.m_dCenterPt[1];

	double oldDisplayScale = m_pTransform->m_Viewport.getDisplayScale();
	bool oldBorderVisible = m_bCellBorderVisible;

	QSize oldViewportSize = viewport()->size();

	updateExportCellProgress(0);

	double dCenterVpX = (lbVp.x() + rtVp.x()) / 2.0;
	double dCenterVpY = (lbVp.y() + rtVp.y()) / 2.0;
	m_pTransform->m_Viewport.setCenterPtXY(dCenterVpX, dCenterVpY);//投影坐标
	m_dDpiX = dpi;
	m_dDpiY = dpi;

	updateViewport(QRectF(0, 0, userTotalWidthDP, userTotalHeightDP));//用户输入宽高
	setViewportDisplayScale(displayScale);

	updateExportCellProgress(20);

	CSpatialPolygon polygon;
	QVector<QVector<QPointF>*> vpRings;
	QVector<QPointF> vpRing;
	QPointF pt[4];
	pt[0] = QPointF(lbPt.x(), lbPt.y());
	pt[1] = QPointF(lbPt.x(), rtPt.y());
	pt[2] = QPointF(rtPt.x(), rtPt.y());
	pt[3] = QPointF(rtPt.x(), lbPt.y());
	vpRing.push_back(pt[0]);
	vpRings.push_back(&vpRing);
	polygon.setPolygon(vpRings);

	//开始选图
	m_pS57DocManager->selectChart(displayScale, &polygon);
	polygon.relPolygon();//释放空间多边形
	updateExportCellProgress(40);

	QPrinter printer(QPrinter::HighResolution);
	printer.setPaperSize(QPrinter::Custom);
	printer.setOutputFormat(QPrinter::PdfFormat);
	printer.setOutputFileName(strExportPath);
	printer.setOrientation(QPrinter::Portrait);//横向

	dpi = printer.resolution();

	printer.setPaperSize(QSize(userTotalWidthDP, userTotalHeightDP), QPrinter::DevicePixel);

	QPainter painter(&printer);
	painter.setClipping(true);
	updateExportCellProgress(60);
	//此处不能通过页边距计算裁剪范围
	int left, bottom, right, top;
	m_pTransform->m_Viewport.convertVp2Dp(lbVp.x(), lbVp.y(), &left, &bottom);
	m_pTransform->m_Viewport.convertVp2Dp(rtVp.x(), rtVp.y(), &right, &top);
	QRect rect(left, top, right - left, bottom - top);
	painter.setClipRect(rect, Qt::ClipOperation::ReplaceClip);

	updateExportCellProgress(70);

	//绘制
	CS57CellLayer::sCellParameters parameters;
	parameters.symbolMode = m_pS57DocManager->getSymbolMode();
	CS57CellLayer::sCellDataSource dataSource;
	dataSource.pCellProvider = m_pS57DocManager->getCellProviderPtr();
	m_pCellLayer->setCellParameter(parameters);
	m_pCellLayer->setDataSource(dataSource);
	m_pCellLayer->setBorderVisible(true);
	m_pCellLayer->setRenderType(XT::RENDER_PRINT);
	m_pCellLayer->setPrintParameters(&exportParameters);
	m_pCellLayer->setSymbolScaleMode(m_eCurSymbolScaleMode);
	m_pCellLayer->render(&painter);

	updateExportCellProgress(100);

	//还原设置并重绘
	m_pTransform->m_Viewport.setCenterPtXY(oldCenterVpX, oldCenterVpY);//投影坐标
	updateViewport(QRectF(0, 0, oldViewportSize.width(), oldViewportSize.height()));//用户输入宽高
	setViewportDisplayScale(oldDisplayScale);
	m_dDpiX = oldDpiX;
	m_dDpiY = oldDpiY;

	setBorderVisible(oldBorderVisible);
	refreshCanvas();
}

//************************************
// Method:    print
// Brief:	  用于打印纸质海图，Adobe PDF暂时无法获取打印纸宽高故无法通过此方法打印，
//			  注意事项：投影类型，基准线，编辑比例尺，显示比例尺
// Returns:   void
// Author:    cl
// DateTime:  2021/08/17
//************************************
void CS57Control::print1(XT::sExportParameters exportParameters)
{
	//保存旧的DPI
	double oldDpiX = m_dDpiX;
	double oldDpiY = m_dDpiY;

	double oldCenterVpX = m_pTransform->m_Viewport.m_dCenterPt[0];
	double oldCenterVpY = m_pTransform->m_Viewport.m_dCenterPt[1];
	double oldBaseLineLat = m_pTransform->m_Proj.getBaseLine();

	bool oldBorderVisible = m_bCellBorderVisible;


	//设置基准纬线
	int baseLineBD = exportParameters.baseLineBD;
	int baseLineBM = exportParameters.baseLineBM;
	float baseLineBS = exportParameters.baseLineBS;
	double baseLineLat = fabs(baseLineBD) +
		+fabs(baseLineBM / 60.) +
		fabs(baseLineBS / 3600.);
	if (baseLineBM < 0 || baseLineBS < 0)
	{
		//参数输入有误
		return;
	}
	if (baseLineBD < 0)
	{
		baseLineLat = -abs(baseLineLat);
	}
	CS57Transform::instance()->m_Proj.setBaseLine(baseLineLat / RO);
	updateCanvas();

	QString cellName = exportParameters.cellName;
	viewportCells();
	CS57Cell* pCell = nullptr;
	for (int i = 0; i < m_vecViewportCells.size(); i++)
	{
		QString strCellName = m_vecViewportCells[i]->getCellName();
		strCellName = strCellName.replace(".000", "");
		if (strCellName == cellName)
			pCell = m_vecViewportCells[i];
	}
	if (pCell == nullptr)
		return;//失败

	//根据dpi重新定义纸张大小
	QSize lastSize = viewport()->size();
	int oldWidth = lastSize.width();
	int oldHeight = lastSize.height();

	double cscl = pCell->getCellCscl();
	setViewportDisplayScale(cscl);
	//m_pTransform->m_Viewport.lockDisplayScale(true);//暂时锁定比例尺
	updateCanvas();

	QPrinter printer(QPrinter::HighResolution);

	QPrintDialog printDlg(&printer);
	if (printDlg.exec())
	{
		QPainter painter(&printer);
		int w = painter.device()->width();
		int h = painter.device()->height();

		////qDebug() << "绘图机宽painter:" << ww << "," << "绘图机高painter:" << hh << endl;
		//绘图机宽度
		//int w = printer.paperSize(QPrinter::Millimeter).width();
		//绘图机高度
		//int h = printer.paperSize(QPrinter::Millimeter).height();

		//qDebug() << "绘图机宽:" << w << "," << "绘图机高:" << h << endl;

		//绘图机中心点物理坐标
		QPoint center(w / 2, h / 2);

		//绘图机DPI横向
		int dpiX = painter.device()->physicalDpiX();
		//绘图机DPI纵向
		int dpiY = painter.device()->physicalDpiY();

		double left = pCell->getCellExtent().xMinimum();
		double top = pCell->getCellExtent().yMaximum();
		double right = pCell->getCellExtent().xMaximum();
		double bottom = pCell->getCellExtent().yMinimum();

		//qDebug() << "left:" << left << "," << "top:" << top << "," << "right;" << right << "," << "bottom:" << bottom << endl;

		QPointF blLeftTop(left / RO, top / RO);
		QPointF blRightTop(right / RO, top / RO);
		QPointF blLeftBottom(left / RO, bottom / RO);
		QPointF blRightBottom(right / RO, bottom / RO);

		QPointF vpLeftTop = m_pTransform->bl2Vp(blLeftTop);
		QPointF vpRightTop = m_pTransform->bl2Vp(blRightTop);
		QPointF vpLeftBottom = m_pTransform->bl2Vp(blLeftBottom);
		QPointF vpRightBottom = m_pTransform->bl2Vp(blRightBottom);

		double vpCellWidth = (vpRightTop.x() - vpLeftTop.x()) / cscl;
		double vpCellHeight = (vpRightTop.y() - vpRightBottom.y()) / cscl;

		//qDebug() << "vpCellWidth:" << vpCellWidth << "," << "vpCellHeight:" << vpCellHeight << endl;

		double dCenterVpX = (vpLeftBottom.x() + vpRightTop.x()) / 2.0;
		double dCenterVpY = (vpLeftBottom.y() + vpRightTop.y()) / 2.0;
		m_pTransform->m_Viewport.setCenterPtXY(dCenterVpX, dCenterVpY);//投影坐标

		//更新视口
		m_dDpiX = dpiX;
		m_dDpiY = dpiY;
		updateViewport(QRectF(0, 0, w, h));//绘图机的宽高

		
		//重新创建画布
		m_eCurSelectChartType = CS57DocManager::SELECT_PRINT;
		m_pS57DocManager->selectChart(cscl, CS57DocManager::SELECT_PRINT);

		//绘制
		CS57CellLayer::sCellParameters parameters;
		parameters.symbolMode = m_pS57DocManager->getSymbolMode();
		CS57CellLayer::sCellDataSource dataSource;
		//dataSource.pGlobalMapProvider = m_pS57DocManager->getGlobalMapProviderPtr();//打印模式不绘制
		//dataSource.pMcovrProvider = m_pS57DocManager->getMcovrProviderPtr();//打印模式不绘制
		dataSource.pCellProvider = m_pS57DocManager->getCellProviderPtr();
		m_pCellLayer->setCellParameter(parameters);
		m_pCellLayer->setDataSource(dataSource);
		m_pCellLayer->setBorderVisible(true);
		m_pCellLayer->setSymbolScaleMode(m_eCurSymbolScaleMode);
		m_pCellLayer->render(&painter);
	}

	m_pTransform->m_Viewport.setCenterPtXY(oldCenterVpX, oldCenterVpY);//投影坐标
	m_dDpiX = oldDpiX;
	m_dDpiY = oldDpiY;

	CS57Transform::instance()->m_Proj.setBaseLine(oldBaseLineLat);
	setBorderVisible(oldBorderVisible);

	//图号
	m_strCellNum = cellName.replace(".000", "");
	m_dCellSize = 0;//图幅是通过读取SENC，所以大小与原ENC不一样，此处直接写0，相当于该字段无效

	//打印份数
	m_nPrintNum = 1;
	//打印时间
	QDateTime curDateTime = QDateTime::currentDateTime();
	m_strPrintTime = curDateTime.toString("yyyy-MM-dd hh::mm:ss");
	//writePrintLog();
}

//************************************
// Method:    print
// Brief:	  打印
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: XT::sExportParameters exportParameters
//************************************
void CS57Control::print(XT::sExportParameters exportParameters)
{
	double oldBaseLineLat = m_pTransform->m_Proj.getBaseLine();
	//第一步更新基准纬线，否则下面都是错的
	int baseLineBD = exportParameters.baseLineBD;
	int baseLineBM = exportParameters.baseLineBM;
	int baseLineBS = exportParameters.baseLineBS;
	QString strBaseLine = QString("%1.%2%3")
		.arg(QString::number(baseLineBD))
		.arg(QString::number(baseLineBM))
		.arg(QString::number(baseLineBS));
	setBaseLine(fun_dmmssTorad(strBaseLine.toDouble()));

	//左下地理坐标点
	int lbLD = exportParameters.lbLD;
	int lbLM = exportParameters.lbLM;
	float lbLS = exportParameters.lbLS;
	int lbBD = exportParameters.lbBD;
	int lbBM = exportParameters.lbBM;
	float lbBS = exportParameters.lbBS;
	double lbLon = 0.0, lbLat = 0.0;
	m_pTransform->dms2Geo(lbLD, lbLM, lbLS, lbLon);
	m_pTransform->dms2Geo(lbBD, lbBM, lbBS, lbLat);
	QPointF lbPt(lbLon, lbLat);

	//右上地理坐标点
	int rtLD = exportParameters.rtLD;
	int rtLM = exportParameters.rtLM;
	float rtLS = exportParameters.rtLS;
	int rtBD = exportParameters.rtBD;
	int rtBM = exportParameters.rtBM;
	float rtBS = exportParameters.rtBS;
	double rtLon = 0.0, rtLat = 0.0;
	m_pTransform->dms2Geo(rtLD, rtLM, rtLS, rtLon);
	m_pTransform->dms2Geo(rtBD, rtBM, rtBS, rtLat);
	QPointF rtPt(rtLon, rtLat);

	QPointF lbVp = m_pTransform->geo2Vp(lbPt);
	QPointF rtVp = m_pTransform->geo2Vp(rtPt);

	double displayScale = exportParameters.displayScale;
	QString cellName = exportParameters.cellName;
	QString strExportPath = exportParameters.exportPath;
	double leftMargin = exportParameters.leftMargin;
	double topMargin = exportParameters.topMargin;
	double rightMargin = exportParameters.rightMargin;
	double bottomMargin = exportParameters.bottomMargin;

	double userWidthCM = qAbs(rtVp.x() - lbVp.x()) / displayScale * 100.0;
	double userHeightCM = qAbs(rtVp.y() - lbVp.y()) / displayScale * 100.0;

	double userTotalWidthCM = userWidthCM + leftMargin + rightMargin;//左右各扩展3厘米
	double userTotalHeightCM = userHeightCM + topMargin + bottomMargin;//上下各扩展3厘米

	if (userTotalWidthCM >= 118.9 || userTotalHeightCM >= 84.1)//A0纸张大小
	{
		QString strW = "输入宽度 = " + QString::number(userTotalWidthCM) + "cm";
		QString strH = "输入高度 = " + QString::number(userTotalHeightCM) + "cm";
		QString msg = strW + "\n" + strH + "\n";
		//超出A0纸张宽高
		QMessageBox::information(this, "提示", msg + "输入打印范围超出A0纸张的宽度或高度");
		return;
	}

	//保存旧的DPI
	double oldDpiX = m_dDpiX;
	double oldDpiY = m_dDpiY;

	double oldCenterVpX = m_pTransform->m_Viewport.m_dCenterPt[0];
	double oldCenterVpY = m_pTransform->m_Viewport.m_dCenterPt[1];

	double oldDisplayScale = m_pTransform->m_Viewport.getDisplayScale();
	bool oldBorderVisible = m_bCellBorderVisible;

	QSize oldViewportSize = viewport()->size();

	QPrinter printer(QPrinter::HighResolution);

	QPrintDialog printDlg(&printer);
	if (printDlg.exec())
	{
		QPainter painter(&printer);
		int w = painter.device()->width();
		int h = painter.device()->height();

		//qDebug() << "绘图机宽painter:" << ww << "," << "绘图机高painter:" << hh << endl;
		//绘图机宽度
		//int w = printer.paperSize(QPrinter::Millimeter).width();
		//绘图机高度
		//int h = printer.paperSize(QPrinter::Millimeter).height();

		//qDebug() << "绘图机宽:" << w << "," << "绘图机高:" << h << endl;

		//绘图机中心点物理坐标
		QPoint center(w / 2, h / 2);

		//绘图机DPI横向
		int dpiX = painter.device()->physicalDpiX();
		//绘图机DPI纵向
		int dpiY = painter.device()->physicalDpiY();

		double dCenterVpX = (lbVp.x() + rtVp.x()) / 2.0;
		double dCenterVpY = (lbVp.y() + rtVp.y()) / 2.0;
		m_pTransform->m_Viewport.setCenterPtXY(dCenterVpX, dCenterVpY);//投影坐标

		//更新视口
		m_dDpiX = dpiX;
		m_dDpiY = dpiY;

		double userTotalWidthMM = userTotalWidthCM * 10.0;
		double userTotalHeightMM = userTotalHeightCM * 10.0;

		int userTotalWidthDP = round(userTotalWidthMM * m_dDpiX / 25.40);
		int userTotalHeightDP = round(userTotalHeightMM * m_dDpiY / 25.40);

		updateViewport(QRectF(0, 0, userTotalWidthDP, userTotalHeightDP));//用户输入宽高
		setViewportDisplayScale(displayScale);

		CSpatialPolygon polygon;
		QVector<QVector<QPointF>*> vpRings;
		QVector<QPointF> vpRing;
		QPointF pt[4];
		pt[0] = QPointF(lbPt.x(), lbPt.y());
		pt[1] = QPointF(lbPt.x(), rtPt.y());
		pt[2] = QPointF(rtPt.x(), rtPt.y());
		pt[3] = QPointF(rtPt.x(), lbPt.y());
		vpRing.push_back(pt[0]);
		vpRings.push_back(&vpRing);
		polygon.setPolygon(vpRings);

		//开始选图
		m_pS57DocManager->selectChart(displayScale, &polygon);
		polygon.relPolygon();//释放空间多边形

		//此处不能通过页边距计算裁剪范围
		painter.setClipping(true);
		int left, bottom, right, top;
		m_pTransform->m_Viewport.convertVp2Dp(lbVp.x(), lbVp.y(), &left, &bottom);
		m_pTransform->m_Viewport.convertVp2Dp(rtVp.x(), rtVp.y(), &right, &top);
		QRect rect(left, top, right - left, bottom - top);
		painter.setClipRect(rect, Qt::ClipOperation::ReplaceClip);

		//绘制
		CS57CellLayer::sCellParameters parameters;
		parameters.symbolMode = m_pS57DocManager->getSymbolMode();
		CS57CellLayer::sCellDataSource dataSource;
		dataSource.pCellProvider = m_pS57DocManager->getCellProviderPtr();
		m_pCellLayer->setCellParameter(parameters);
		m_pCellLayer->setDataSource(dataSource);
		m_pCellLayer->setBorderVisible(true);
		m_pCellLayer->setRenderType(XT::RENDER_PRINT);
		m_pCellLayer->setPrintParameters(&exportParameters);
		m_pCellLayer->setSymbolScaleMode(m_eCurSymbolScaleMode);
		m_pCellLayer->render(&painter);
	}
	//还原设置并重绘
	m_pTransform->m_Viewport.setCenterPtXY(oldCenterVpX, oldCenterVpY);//投影坐标
	updateViewport(QRectF(0, 0, oldViewportSize.width(), oldViewportSize.height()));//用户输入宽高
	setViewportDisplayScale(oldDisplayScale);
	m_dDpiX = oldDpiX;
	m_dDpiY = oldDpiY;

	setBorderVisible(oldBorderVisible);
	refreshCanvas();

	//打印记录
	//图号
	m_strCellNum = cellName.replace(".000", "");
	m_dCellSize = 0;//图幅是通过读取SENC，所以大小与原ENC不一样，此处直接写0，相当于该字段无效
	//打印份数
	m_nPrintNum = 1;
	//打印时间
	QDateTime curDateTime = QDateTime::currentDateTime();
	m_strPrintTime = curDateTime.toString("yyyy-MM-dd hh::mm:ss");
	//writePrintLog();
}

//************************************
// Method:    isNumber
// Brief:	  是否为数字
// Returns:   bool
// Author:    cl
// DateTime:  2021/08/26
// Parameter: const QString & str
//************************************
bool CS57Control::isNumber(const QString & str)
{
	if (str.isEmpty() || str.isNull())
		return false;

	return verifyNumber(str);
}

//************************************
// Method:    getPrintableCells
// Brief:	  获取可打印图幅列表
// Returns:   QStringList
// Author:    cl
// DateTime:  2021/08/26
//************************************
QStringList CS57Control::getPrintableCells()
{
	viewportCells();

	if (m_vecViewportCells.size() == 0)
		return QStringList();

	QStringList lstCellNames;
	for (int i = 0; i < m_vecViewportCells.size(); i++)
	{
		QString strCellName = m_vecViewportCells[i]->getCellName();
		QString strNameWithoutExt = strCellName.replace(".000", "");
		lstCellNames << strNameWithoutExt;
	}

	return lstCellNames;
}

//************************************
// Method:    getCaptureSize
// Brief:	  获取截屏尺寸
// Returns:   QSize
// Author:    cl
// DateTime:  2021/08/26
// Parameter: sExportParameters exportParameters
//************************************
QSize CS57Control::getCaptureSize(XT::sExportParameters exportParameters)
{
	int dpi = exportParameters.dpi;

	if (dpi == -1)//本机默认
		dpi = m_dDpiX;

	if (dpi < 1 || dpi > 1200)
		return QSize(0, 0);

	//根据dpi重新定义纸张大小
	QSize lastSize = viewport()->size();
	int oldWidth = lastSize.width();
	int oldHeight = lastSize.height();
	int newWidth = oldWidth * dpi / m_dDpiX;
	int newHeight = oldHeight * dpi / m_dDpiY;

	//qDebug() << "CS57Control:"<< newWidth << "," << newHeight << endl;

	return QSize(newWidth,newHeight);
}

//************************************
// Method:    getImageSize
// Brief:	  获取导出图像尺寸
// Returns:   QSize
// Author:    cl
// DateTime:  2021/08/26
// Parameter: sExportParameters exportParameters
//************************************
QSize CS57Control::getImageSize(XT::sExportParameters exportParameters)
{
	QString cellName = exportParameters.cellName;
	int dpi = exportParameters.dpi;
	double leftMargin = exportParameters.leftMargin;
	double topMargin = exportParameters.topMargin;
	double rightMargin = exportParameters.rightMargin;
	double bottomMargin = exportParameters.bottomMargin;

	viewportCells();
	CS57Cell* pCell = nullptr;
	for (int i = 0; i < m_vecViewportCells.size(); i++)
	{
		QString strCellName = m_vecViewportCells[i]->getCellName();
		strCellName = strCellName.replace(".000", "");
		if (strCellName == cellName)
			pCell = m_vecViewportCells[i];
	}
	if (pCell == nullptr)
		return QSize(0, 0);

	if (dpi == -1)//本机默认
		dpi = m_dDpiX;

	if (dpi < 1 || dpi > 1200)
		return QSize(0, 0);

	//保存旧的DPI
	int oldDpiX = m_dDpiX;
	int oldDpiY = m_dDpiY;

	QSize lastSize = viewport()->size();
	int oldWidth = lastSize.width();
	int oldHeight = lastSize.height();

	//调整比例尺
	double cscl = pCell->getCellCscl();
	setViewportDisplayScale(cscl);
	//m_pTransform->m_Viewport.lockDisplayScale(true);//暂时锁定比例尺
	updateCanvas();

	//根据dpi重新定义纸张大小
	double left = pCell->getCellExtent().xMinimum();
	double top = pCell->getCellExtent().yMaximum();
	double right = pCell->getCellExtent().xMaximum();
	double bottom = pCell->getCellExtent().yMinimum();

	//qDebug() << "left:" << left << "," << "top:" << top << "," << "right;" << right << "," << "bottom:" << bottom << endl;

	QPointF blLeftTop(left / RO, top / RO);
	QPointF blRightTop(right / RO, top / RO);
	QPointF blLeftBottom(left / RO, bottom / RO);
	QPointF blRightBottom(right / RO, bottom / RO);

	QPointF vpLeftTop = m_pTransform->bl2Vp(blLeftTop);
	QPointF vpRightTop = m_pTransform->bl2Vp(blRightTop);
	QPointF vpLeftBottom = m_pTransform->bl2Vp(blLeftBottom);
	QPointF vpRightBottom = m_pTransform->bl2Vp(blRightBottom);

	double dCellWidthCM = (vpRightTop.x() - vpLeftTop.x()) / cscl * 100.0 + leftMargin + rightMargin;//左右各扩展*厘米
	double dCellHeightCM = (vpRightTop.y() - vpRightBottom.y()) / cscl * 100.0 + topMargin + bottomMargin;//上下各扩展*厘米
	double dCellWidthMM = dCellWidthCM * 10.0;
	double dCellHeightMM = dCellHeightCM * 10.0;

	int dpCellWidth = round(dCellWidthMM * dpi / 25.40);
	int dpCellHeight = round(dCellHeightMM * dpi / 25.40);

	return QSize(dpCellWidth, dpCellHeight);
}

//************************************
// Method:    getPdfSize
// Brief:	  获取导出PDF尺寸
// Returns:   QSize
// Author:    cl
// DateTime:  2021/08/26
// Parameter: sExportParameters exportParameters
//************************************
QSize CS57Control::getPdfSize(XT::sExportParameters exportParameters)
{
	QString cellName = exportParameters.cellName;
	int dpi = exportParameters.dpi;
	double leftMargin = exportParameters.leftMargin;
	double topMargin = exportParameters.topMargin;
	double rightMargin = exportParameters.rightMargin;
	double bottomMargin = exportParameters.bottomMargin;

	viewportCells();
	CS57Cell* pCell = nullptr;
	for (int i = 0; i < m_vecViewportCells.size(); i++)
	{
		QString strCellName = m_vecViewportCells[i]->getCellName();
		strCellName = strCellName.replace(".000", "");
		if (strCellName == cellName)
			pCell = m_vecViewportCells[i];
	}
	if (pCell == nullptr)
		return QSize(0, 0);

	if (dpi == -1)//本机默认
		dpi = m_dDpiX;

	if (dpi < 1 || dpi > 1200)
		return QSize(0, 0);

	//保存旧的DPI
	int oldDpiX = m_dDpiX;
	int oldDpiY = m_dDpiY;

	QSize lastSize = viewport()->size();
	int oldWidth = lastSize.width();
	int oldHeight = lastSize.height();

	//调整比例尺
	double cscl = pCell->getCellCscl();
	setViewportDisplayScale(cscl);
	//m_pTransform->m_Viewport.lockDisplayScale(true);//暂时锁定比例尺
	updateCanvas();

	QPrinter printer(QPrinter::HighResolution);
	printer.setPaperSize(QPrinter::Custom);
	printer.setOutputFormat(QPrinter::PdfFormat);
	printer.setOrientation(QPrinter::Portrait);//横向

	//根据dpi重新定义纸张大小
	double left = pCell->getCellExtent().xMinimum();
	double top = pCell->getCellExtent().yMaximum();
	double right = pCell->getCellExtent().xMaximum();
	double bottom = pCell->getCellExtent().yMinimum();

	//qDebug() << "left:" << left << "," << "top:" << top << "," << "right;" << right << "," << "bottom:" << bottom << endl;

	QPointF blLeftTop(left / RO, top / RO);
	QPointF blRightTop(right / RO, top / RO);
	QPointF blLeftBottom(left / RO, bottom / RO);
	QPointF blRightBottom(right / RO, bottom / RO);

	QPointF vpLeftTop = m_pTransform->bl2Vp(blLeftTop);
	QPointF vpRightTop = m_pTransform->bl2Vp(blRightTop);
	QPointF vpLeftBottom = m_pTransform->bl2Vp(blLeftBottom);
	QPointF vpRightBottom = m_pTransform->bl2Vp(blRightBottom);

	double dCellWidthCM = (vpRightTop.x() - vpLeftTop.x()) / cscl * 100.0 + leftMargin + rightMargin;//左右各扩展*厘米
	double dCellHeightCM = (vpRightTop.y() - vpRightBottom.y()) / cscl * 100.0 + topMargin + bottomMargin;//上下各扩展*厘米
	double dCellWidthMM = dCellWidthCM * 10.0;
	double dCellHeightMM = dCellHeightCM * 10.0;

	dpi = printer.resolution();

	int dpCellWidth = round(dCellWidthMM * dpi / 25.40);
	int dpCellHeight = round(dCellHeightMM * dpi / 25.40);

	return QSize(dpCellWidth, dpCellHeight);
}

//************************************
// Method:    getExportSize
// Brief:	  获取导出尺寸
// Returns:   QSize
// Author:    cl
// DateTime:  2021/08/26
// Parameter: sExportParameters exportParameters
//************************************
QSize CS57Control::getExportSize(XT::sExportParameters exportParameters)
{
	QSize exportSize = QSize(0, 0);
	switch (exportParameters.exportMode)
	{
	case XT::CAPTURE:
		exportSize = getCaptureSize(exportParameters);
		break;
	case XT::IMG:
		exportSize = getImageSize(exportParameters);
		break;
	case XT::PDF:
		exportSize = getPdfSize(exportParameters);
		break;
	}

	return exportSize;
}

//************************************
// Method:    setMapTool
// Brief:	  设置画布工具
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CS57MapTool * tool
//************************************
void CS57Control::setMapTool(CS57MapTool* tool)
{
	m_pMapTool = tool;
}

//************************************
// Method:    getOperateMode
// Brief:	  获取操作模式
// Returns:   XT::OperateMode
// Author:    cl
// DateTime:  2022/07/21
//************************************
XT::OperateMode CS57Control::getOperateMode() const
{
	return m_eCurOperateMode;
}

//************************************
// Method:    getQueryMode
// Brief:	  获取查询模式
// Returns:   XT::QueryMode
// Author:    cl
// DateTime:  2022/07/21
//************************************
XT::QueryMode CS57Control::getQueryMode() const
{
	return m_eCurQueryMode;
}

//************************************
// Method:    setSelectMode
// Brief:	  设置选择模式
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: XT::SelectionMode mode
//************************************
void CS57Control::setSelectMode(XT::SelectionMode mode)
{
	m_eCurSelectionMode = mode;
}

//************************************
// Method:    getSelectionMode
// Brief:	  获取选中模式
// Returns:   XT::SelectionMode
// Author:    cl
// DateTime:  2022/07/21
//************************************
XT::SelectionMode CS57Control::getSelectionMode() const
{
	return m_eCurSelectionMode;
}

//************************************
// Method:    getSceneRect
// Brief:	  获取场景矩形
// Returns:   QT_NAMESPACE::QRectF
// Author:    cl
// DateTime:  2022/07/21
//************************************
QRectF CS57Control::getSceneRect()
{
	return sceneRect();
}

//************************************
// Method:    getScene
// Brief:	  获取场景
// Returns:   QT_NAMESPACE::QGraphicsScene*
// Author:    cl
// DateTime:  2022/07/21
//************************************
QGraphicsScene* CS57Control::getScene()
{
	return m_pS57Scene;
}

//************************************
// Method:    routeManager
// Brief:	  航线管理
// Returns:   CS57RouteManager*
// Author:    cl
// DateTime:  2022/07/21
//************************************
CS57RouteManager* CS57Control::routeManager()
{
	return m_pRouteManager;
}

//************************************
// Method:    removeRoute
// Brief:	  移除航线
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57Control::removeRoute()
{
	m_pRouteManager->removeAllRoute(CS57RouteManager::Normal);
	m_pRouteManager->removeAllRoute(CS57RouteManager::Temp);
}

//************************************
// Method:    reloadEnc
// Brief:	  重新加载图幅
// Returns:   void
// Author:    cl
// DateTime:  2021/09/02
// Parameter: const QString & dir
//************************************
void CS57Control::reloadEnc(const QString& dir)
{
	setEncDir(dir);
	fastInit();
	createCanvas();
	renderCanvas();
}

//************************************
// Method:    updateDisplayScale
// Brief:	  更新显示比例尺
// Returns:   void
// Author:    cl
// DateTime:  2021/09/02
// Parameter: double scale
//************************************
void CS57Control::updateDisplayScale(double scale)
{
	if (scale < Min_Scale)
		scale = Min_Scale;
	if (scale > Max_Scale)
		scale = Max_Scale;

	m_pTransform->m_Viewport.setDisplayScale(scale);
	refreshCanvas();

	scale = m_pTransform->m_Viewport.getDisplayScale();
	emit changeScale(scale);
}

//************************************
// Method:    flyToPos
// Brief:	  飞到坐标点
// Returns:   void
// Author:    cl
// DateTime:  2021/09/02
// Parameter: double lon	目标经度
// Parameter: double lat	目标纬度
// Parameter: double scale	目标比例尺
//************************************
void CS57Control::flyToPos(double lon, double lat, double scale)
{
	QPointF vpPt = m_pTransform->geo2Vp(lon, lat);
	m_pTransform->m_Viewport.setCenterPtXY(vpPt.x(), vpPt.y());
	updateDisplayScale(scale);
}

//************************************
// Method:    flyToCell
// Brief:	  飞到图幅点
// Returns:   void
// Author:    cl
// DateTime:  2021/09/02
// Parameter: CS57CellMessage * msg
//************************************
void CS57Control::flyToCell(CS57CellMessage* msg)
{
	QPointF centerPt = msg->getMcovrCenterPos();
	double scale = msg->getCellCscl();
	flyToPos(centerPt.x(), centerPt.y(), scale);
	setLocation(centerPt.x(), centerPt.y(), scale);
}

//************************************
// Method:    flyToCell
// Brief:	  飞到图幅位置
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57Control::flyToCell()
{
	QVector<CS57CellMessage*> vecMcovr = m_pS57DocManager->getMcovrProviderPtr()->m_vecSourceMcovr;
	if (vecMcovr.size() == 0)
		return;

	flyToCell(vecMcovr[0]);
}

//************************************
// Method:    flyToCell
// Brief:	  飞到图幅位置
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: const QString & name
//************************************
void CS57Control::flyToCell(const QString& name)
{
	QVector<CS57CellMessage*> vecMcovr = m_pS57DocManager->getMcovrProviderPtr()->m_vecSourceMcovr;
	for (int i = 0; i < vecMcovr.size(); i++)
	{
		QString strCellName = vecMcovr[i]->getCellName();
		QString cellName = strCellName.replace(".000", "");
		if (cellName == name)
		{
			flyToCell(vecMcovr[i]);
			break;
		}
	}
}

//************************************
// Method:    getAllCellName
// Brief:	  获取目录下左右图幅名称
// Returns:   QStringList
// Author:    cl
// DateTime:  2022/07/21
// Parameter: const QString & dir
//************************************
QStringList CS57Control::getAllCellName(const QString& dir)
{
	QStringList lstCellNames;
	QVector<CS57CellMessage*> vecMcovr = m_pS57DocManager->getMcovrProviderPtr()->m_vecSourceMcovr;
	for (int i = 0; i < vecMcovr.size(); i++)
	{
		QString strCellName = vecMcovr[i]->getCellName();
		lstCellNames << strCellName.replace(".000", "");
	}

	return lstCellNames;
}

//************************************
// Method:    writePrintLog
// Brief:	  写打印日志
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57Control::writePrintLog()
{
	//实作项目使用
	QString strFileName = "./PrintLog.csv";
	if (strFileName.isEmpty())
		return;

	QFile file(strFileName);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Append))
	{
		//文件打开失败返回
		return;
	}

	QTextStream out(&file);
	//out.setCodec("system");
	//out << tr("图号") << tr(",") << tr("海图大小")<<","<<tr("打印份数")<<","<<tr("打印时间") << endl;
	QString strFileSize = QString::number(m_dCellSize, 'f', 2);
	out << m_strCellNum << "," << strFileSize << "," << m_nPrintNum << "," << m_strPrintTime << endl;
	file.close();
}

//************************************
// Method:    getDpi
// Brief:	  获取DPI
// Returns:   QT_NAMESPACE::qreal
// Author:    cl
// DateTime:  2022/07/21
//************************************
qreal CS57Control::getDpi()
{
	return m_dDpi;
}

//************************************
// Method:    setRegionDivide
// Brief:	  设置区域拆分开关
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: bool open
//************************************
void CS57Control::setRegionDivide(bool open)
{
	m_pS57DocManager->setRegionDivide(open);
}

//************************************
// Method:    addLayerToCanvas
// Brief:	  添加层到画布
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CS57AbstractLayer * pLayer
//************************************
void CS57Control::addLayerToCanvas(CS57AbstractLayer* pLayer)
{
	if (m_pS57CanvasImage)
	{
		m_pLayerTask->addLayer(pLayer);
	}
}

//************************************
// Method:    removeLayer
// Brief:	  移除图层
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CS57AbstractLayer * pLayer
//************************************
void CS57Control::removeLayer(CS57AbstractLayer* pLayer)
{
	if (m_pS57CanvasImage)
	{
		m_pLayerTask->removeLayer(pLayer);
	}
}

//************************************
// Method:    removeLayerFromName
// Brief:	  通过图层名称移除图层
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: const QString & strLayerName
//************************************
void CS57Control::removeLayerFromName(const QString& strLayerName)
{
	if (m_pS57CanvasImage)
	{
		m_pLayerTask->removeLayerFromName(strLayerName);
	}
}

//************************************
// Method:    transformScrnToGeo
// Brief:	  转换屏幕坐标到地理坐标
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: int scrnPtX
// Parameter: int scrnPtY
// Parameter: double * lon
// Parameter: double * lat
// Parameter: double scale
//************************************
void CS57Control::transformScrnToGeo(int scrnPtX, int scrnPtY, double* lon, double* lat, double scale /*= 0.0*/)
{
	double oldScale = getDisplayScale();
	if (scale > 0.0)
		setViewportDisplayScale(scale);

	QPointF pt = m_pTransform->pixel2Geo(scrnPtX, scrnPtY);
	*lon = pt.x();
	*lat = pt.y();

	if (scale > 0.0)
		setViewportDisplayScale(oldScale);
}

//************************************
// Method:    transformGeoToScrn
// Brief:	  转换地理坐标到屏幕坐标
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: double lon
// Parameter: double lat
// Parameter: int * scrnPtX
// Parameter: int * scrnPtY
// Parameter: double scale
//************************************
void CS57Control::transformGeoToScrn(double lon, double lat, int* scrnPtX, int* scrnPtY, double scale /*= 0.0*/)
{
	double oldScale = getDisplayScale();
	if (scale > 0.0)
		setViewportDisplayScale(scale);

	QPoint pt = m_pTransform->geo2Pixel(lon, lat);
	*scrnPtX = pt.x();

	*scrnPtY = pt.y();

	if (scale > 0.0)
		setViewportDisplayScale(oldScale);
}

//************************************
// Method:    setCellLayerVisible
// Brief:	  设置图幅层可视性
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: bool visible
//************************************
void CS57Control::setCellLayerVisible(bool visible)
{
	CS57CellProvider* provider = m_pS57DocManager->getCellProviderPtr();
	if (!provider)
		return;

	m_bCellLayerVisible = visible;
	provider->setCellLayerVisible(visible);
}

//************************************
// Method:    getDisplayScale
// Brief:	  获取显示比例尺
// Returns:   double
// Author:    cl
// DateTime:  2022/07/21
//************************************
double CS57Control::getDisplayScale() const
{
	return m_pTransform->m_Viewport.getDisplayScale();
}

//************************************
// Method:    getViewportCenterPt
// Brief:	  获取视口中心点
// Returns:   QPointF
// Author:    cl
// DateTime:  2022/07/21
//************************************
QPointF CS57Control::getViewportCenterPt() const
{
	double vpx = 0.0, vpy = 0.0;
	m_pTransform->m_Viewport.getCenterPointXY(vpx, vpy);
	QPointF ptf = m_pTransform->vp2Geo(vpx, vpy);

	return ptf;
}

//************************************
// Method:    getBaseLine
// Brief:	  获取基准纬线
// Returns:   double
// Author:    cl
// DateTime:  2022/07/21
//************************************
double CS57Control::getBaseLine() const
{
	CS57AppConfig* pConfig = CS57AppConfig::instance();

	return pConfig->getBaseLine();
}

//************************************
// Method:    setStartUpLocation
// Brief:	  设置起始跳转坐标点
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57Control::setStartUpLocation()
{
	CS57AppConfig* pConfig = CS57AppConfig::instance();

	QPointF centerPt = getViewportCenterPt();
	double scale = getDisplayScale();
	pConfig->setLocation(centerPt.x(), centerPt.y(), scale);
	pConfig->updateXml();
}

//************************************
// Method:    setMcovrRegionLegendVisible
// Brief:	  设置Mcover分区图例可视性
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: bool visible
//************************************
void CS57Control::setMcovrRegionLegendVisible(bool visible)
{
	if (visible)
	{
		m_pS57McovrRegionLegend->show();
	}
	else
	{
		m_pS57McovrRegionLegend->hide();
	}
}

//************************************
// Method:    getBorderRangeByCellNo
// Brief:	  通过图号获取图廓范围
// Returns:   XT::sExportCellBorderRange
// Author:    cl
// DateTime:  2022/07/21
// Parameter: const QString & strCellNo
//************************************
XT::sExportCellBorderRange CS57Control::getBorderRangeByCellNo(const QString& strCellNo)
{
	viewportCells();
	CS57Cell* pCell = nullptr;
	for (int i = 0; i < m_vecViewportCells.size(); i++)
	{
		QString strCellName = m_vecViewportCells[i]->getCellName();
		strCellName = strCellName.replace(".000", "");
		if (strCellName == strCellNo)
		{
			pCell = m_vecViewportCells[i];
			break;
		}
	}

	if (pCell == nullptr)
	{
		//未找到图幅
		return XT::sExportCellBorderRange();
	}
	XT::sExportCellBorderRange range;
	CRectangle<double> rectangle = pCell->getCellExtent();
	//将经纬度转度分秒
	QPointF lbPt = QPointF(rectangle.xMinimum(), rectangle.yMinimum());
	QPointF rtPt = QPointF(rectangle.xMaximum(), rectangle.yMaximum());
	QPointF lbDMS = m_pTransform->geo2DMS(lbPt);
	QPointF rtDMS = m_pTransform->geo2DMS(rtPt);

	QString strLBBD, strLBBM, strLBBS, strLBLD, strLBLM, strLBLS;
	QString strRTBD, strRTBM, strRTBS, strRTLD, strRTLM, strRTLS;
	m_pTransform->dmsSplit(lbDMS, strLBLD, strLBLM, strLBLS,strLBBD, strLBBM, strLBBS);
	m_pTransform->dmsSplit(rtDMS, strRTLD, strRTLM, strRTLS,strRTBD, strRTBM, strRTBS);

	range.lbLD = strLBLD;
	range.lbLM = strLBLM;
	range.lbLS = strLBLS;
	range.lbBD = strLBBD;
	range.lbBM = strLBBM;
	range.lbBS = strLBBS;
	range.rtLD = strRTLD;
	range.rtLM = strRTLM;
	range.rtLS = strRTLS;
	range.rtBD = strRTBD;
	range.rtBM = strRTBM;
	range.rtBS = strRTBS;

	return range;
}

//************************************
// Method:    getCsclByCellNo
// Brief:	  通过图号获取图幅编辑比例尺
// Returns:   double
// Author:    cl
// DateTime:  2022/07/21
// Parameter: const QString & strCellNo
//************************************
double CS57Control::getCsclByCellNo(const QString& strCellNo)
{
	for (int i = 0; i < m_vecViewportCells.size(); i++)
	{
		QString strCellName = m_vecViewportCells[i]->getCellNameWithoutSuffix();
		if (strCellName == strCellNo)
			return m_vecViewportCells[i]->getCellCscl();
	}
	return -1;
}

//************************************
// Method:    setBorderVisible
// Brief:	  设置图廓显隐
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: bool visible
//************************************
void CS57Control::setBorderVisible(bool visible)
{
	m_bCellBorderVisible = visible;

	refreshCanvas();
}

//************************************
// Method:    setColorMode
// Brief:	  设置颜色模式
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: XT::S52ColorMode mode
//************************************
void CS57Control::setColorMode(XT::S52ColorMode mode)
{
	m_pS57DocManager->setColorMode(mode);

	refreshCanvas();
}

//************************************
// Method:    setSymbolScaleMode
// Brief:	  设置符号缩放模式
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: XT::SymbolScaleMode mode
//************************************
void CS57Control::setSymbolScaleMode(XT::SymbolScaleMode mode /*= XT::ABS_SCALE*/)
{
	m_eCurSymbolScaleMode = mode;

	refreshCanvas();
}
