#pragma once
#include "xt_global.h"
#include "control_global.h"
#include "cs57analysis.h"
#include "cs57docmanager.h"
#include "cs57canvas.h"
#include "cs57maptool.h"
#include "cs57highlightlayer.h"
#include "cs57celllayer.h"
#include "cs57customlayer.h"
#include "cs57mcovrregionlayer.h"
#include "cs57mcovrregionlegend.h"
#include "cs57canvasimage.h"
#include <QGraphicsView>
#include <QResizeEvent>


using namespace Analysis;
using namespace Doc;
using namespace Core;

class CS57MapTool;
class CS57RouteManager;
class CS57Control : public QGraphicsView
{
	Q_OBJECT
public:
	CS57Control(QWidget* parent = nullptr);
	~CS57Control();

public:	
	//通过AppConfig.xml快速初始化
	void	fastInit();	
	//创建画布
	void	createCanvas();
	//渲染画布
	void	renderCanvas();
	//设置操作模式
	void	setOperateMode(XT::OperateMode mode);
	//设置查询模式
	void	setQueryMode(XT::QueryMode mode);
	//设置抽选模式
	void	setSelectMode(XT::SelectionMode mode);
	//关闭查询
	void	closeQuery();
	//获取当前DPI
	qreal	getDpi();

	//获取操作模式
	XT::OperateMode getOperateMode() const;
	//获取查询模式
	XT::QueryMode	getQueryMode() const;
	//获取抽选模式
	XT::SelectionMode getSelectionMode() const;

	//获取场景矩形
	QRectF  getSceneRect();
	//设置地图工具
	void	setMapTool(CS57MapTool* tool);
	//设置图廓显隐
	void	setBorderVisible(bool visible);
	//设置符号缩放模式
	void	setSymbolScaleMode(XT::SymbolScaleMode mode = XT::ABS_SCALE);

	//设置显示分类
	void	setDisplayCategory(XT::DisplayCategory displayCategory);
	//设置屏幕中心点
	void	setLocation(double lon, double lat, double display);
	//设置图幅目录
	void	setEncDir(const QString& dir);
	//设置符号显示模式
	void	setSymbolMode(XT::SymbolMode mode);
	//飞到坐标点
	void	flyToPos(double lon, double lat, double scale);
	//飞到图幅
	void	flyToCell();
	void	flyToCell(const QString& name);
	//卸载ENC
	void	unloadEnc();
	//加载ENC
	void	loadEnc();
	//获取设置图幅目录中所有可用图幅名称
	QStringList getAllCellName(const QString& dir);
	//重新加载ENC
	void	reloadEnc(const QString& dir);
	//销毁画布图像
	void	destoryImage();
	//更新视口
	void	updateViewport(QRectF rect);
	//偏移视口
	void	offsetViewport(int offset_x, int offset_y);
	//设置投影名称
	void	setProjectionName(XT::ProjType type);
	//设置基准纬线
	void	setBaseLine(double baseLine);
	//获取基准纬线
	double  getBaseLine() const;
	//设置视口中心点
	void	setViewportCenterPt(double geo_lon, double geo_lat);
	//设置显示比例尺
	void	setViewportDisplayScale(double displayScale);
	//设置视口
	void	setViewport(int dpi_x, int dpi_y, int x, int y, int w, int h);
	//设置视口
	void	setViewport(int dpi_x, int dpi_y, QRectF rect);
	//
	int		startDisplayScale() const;

	void	setColorMode(XT::S52ColorMode mode);
	void	setMcovrRegionLegendVisible(bool visible);

	void    setStartUpLocation();
	double  getDisplayScale() const;
	QPointF getViewportCenterPt() const;
	//更新显示比例尺
	void	updateDisplayScale(double scale);

	//通过图号获取图廓范围
	XT::sExportCellBorderRange	getBorderRangeByCellNo(const QString& strCellNo);
	//通过图号获取图幅编辑比例尺
	double	getCsclByCellNo(const QString& strCellNo);

	//文件
	void    screenCapture(XT::sExportParameters exportParameters);
	void    exportPdf(XT::sExportParameters exportParameters);
	void	print1(XT::sExportParameters exportParameters);
	void	exportPdf1(XT::sExportParameters exportParameters);
	void	exportImage(XT::sExportParameters exportParameters);
	void	exportImage1(XT::sExportParameters exportParameters);
	void	print(XT::sExportParameters exportParameters);

	//选项
	void    setGlobalMapVisible(bool visible);
	void    setCanvasColor(QColor color);

	CS57RouteManager* routeManager();
	QGraphicsScene* getScene();
	QSize	getExportSize(XT::sExportParameters exportParameters);
	void	updateCanvas();
	bool	isNumber(const QString& str);
	void	refreshCanvas();
	QStringList getPrintableCells();

	void	setCellLayerVisible(bool visible);
	//设置区域划分 海图抽选中使用
	void    setRegionDivide(bool open);

	void	addLayerToCanvas(CS57AbstractLayer* pLayer);
	void	removeLayer(CS57AbstractLayer* pLayer);
	void    removeLayerFromName(const QString& strLayerName);

	void	transformScrnToGeo(int scrnPtX, int scrnPtY, double* lon, double* lat, double scale = 0.0);
	void	transformGeoToScrn(double lon, double lat, int* scrnPtX, int* scrnPtY, double scale = 0.0);
public:
	void	zoomIn(double factor = 0.5);
	void	zoomOut(double factor = 1.5);

	void	viewportLeft(int offset = -100);
	void	viewportRight(int offset = 100);
	void	viewportUp(int offset = -100);
	void	viewportDown(int offset = 100);
	CS57CanvasImage* getCanvasImage();
private:
	void	init();
	void	initCanvas();
	void	initConnects();

	void	removeRoute();
	void	flyToCell(CS57CellMessage* msg);
	QString	convertDMS(const QPoint& pt) const;
	//释放旧的任务
	void	releaseOldJob();
	//获取当前视口内图幅列表
	void	viewportCells();

	QSize	getCaptureSize(XT::sExportParameters exportParameters);
	QSize	getImageSize(XT::sExportParameters exportParameters);
	QSize	getPdfSize(XT::sExportParameters exportParameters);

	void	writePrintLog();

signals:
	void	mousePressPt(QPoint pt);
	void	updateExportCellProgress(int value);
	void	changeScale(int scale);
	void	updateImgSize(int w,int h);
	void	moveGeo(const QString& text);
	void	updateLoadCellProgress(int cur, int total);
	void    canvasRefreshed();
public slots:
	void	highlightItem(int row);

protected:
	void	wheelEvent(QWheelEvent *event) override;
	void	resizeEvent(QResizeEvent *event) override;
	void	mouseMoveEvent(QMouseEvent *event) override;
	void	mouseDoubleClickEvent(QMouseEvent *event) override;
	void	mousePressEvent(QMouseEvent *event) override;
	void	mouseReleaseEvent(QMouseEvent *event) override;
	
	CS57CanvasMap* getCanvasMap();
private:
	CS57DocManager*			m_pS57DocManager;
	CS57Analysis*			m_pS57Analysis;
	QGraphicsScene*			m_pS57Scene;
	CS57CanvasMap*			m_pS57CanvasMap;
	CS57CanvasImage*		m_pS57CanvasImage;
	CS57McovrRegionLegend*  m_pS57McovrRegionLegend;

	CS57Transform*			m_pTransform;
	CS57RouteManager*		m_pRouteManager;
	QVector<CS57Cell*>		m_vecViewportCells;

	bool					m_bMousePress;
	QPoint					m_MouseEndPt;
	QPoint					m_MouseStartPt;
	XT::OperateMode			m_eCurOperateMode;
	XT::QueryMode			m_eCurQueryMode;
	XT::SelectionMode		m_eCurSelectionMode;
	XT::SymbolScaleMode		m_eCurSymbolScaleMode;
	CS57DocManager::SelectChartType m_eCurSelectChartType;

	CS57MapTool*			m_pMapTool;

	//鼠标移动实时地理坐标
	QPointF					m_GeoMouseMove;					
	qreal					m_dDpi;
	qreal					m_dDpiX;
	qreal					m_dDpiY;
	QColor					m_CanvasColor;

	CS57LayerTask*			m_pLayerTask;

	//逻辑图层
	CS57CellLayer*			m_pCellLayer;
	CS57HighlightLayer*		m_pHighlightLayer;
	CS57CustomLayer*		m_pCustomLayer;
	CS57McovrRegionLayer*	m_pMcovrRegionLayer;
	bool					m_bCellLayerVisible;
	bool					m_bCellBorderVisible;

	//打印日志
	QString					m_strCellNum;//图号
	double					m_dCellSize;//大小
	int						m_nPrintNum;//打印份数
	QString					m_strPrintTime;//打印时间	
};