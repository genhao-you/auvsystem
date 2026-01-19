#include "ixtmap.h"

IXTMap::IXTMap(QWidget *parent)
	: CS57Control(parent)
{}

IXTMap::~IXTMap()
{}

//************************************
// Method:    setEncDir
// Brief:	  设置ENC目录
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: const QString & dir
//************************************
void IXTMap::setEncDir(const QString& dir)
{
	CS57Control::setEncDir(dir);
}

//************************************
// Method:    setSymbolMode
// Brief:	  设置符号模式
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: XT::SymbolMode mode
//************************************
void IXTMap::setSymbolMode(XT::SymbolMode mode)
{
	CS57Control::setSymbolMode(mode);
}

//************************************
// Method:    setSymbolScaleMode
// Brief:	  设置符号缩放模式
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: XT::SymbolScaleMode mode
//************************************
void IXTMap::setSymbolScaleMode(XT::SymbolScaleMode mode)
{
	CS57Control::setSymbolScaleMode(mode);
}

//************************************
// Method:    loadEnc
// Brief:	  加载ENC
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void IXTMap::loadEnc()
{
	CS57Control::loadEnc();
}

//************************************
// Method:    setProjectionName
// Brief:	  设置投影名称
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: XT::ProjType type
//************************************
void IXTMap::setProjectionName(XT::ProjType type)
{
	CS57Control::setProjectionName(type);
}

//************************************
// Method:    setdBaseLine
// Brief:	  设置基准纬线
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: double baseLine
//************************************
void IXTMap::setdBaseLine(double baseLine)
{
	CS57Control::setBorderVisible(baseLine);
}

//************************************
// Method:    setViewportCenterPt
// Brief:	  设置视口中心点
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: double geo_lon
// Parameter: double geo_lat
//************************************
void IXTMap::setViewportCenterPt(double geo_lon, double geo_lat)
{
	CS57Control::setViewportCenterPt(geo_lon, geo_lat);
}

//************************************
// Method:    setViewportDisplayScale
// Brief:	  设置视口显示比例尺
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: double displayScale
//************************************
void IXTMap::setViewportDisplayScale(double displayScale)
{
	CS57Control::setViewportDisplayScale(displayScale);
}

//************************************
// Method:    setViewport
// Brief:	  设置视口
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: int dpi_x
// Parameter: int dpi_y
// Parameter: QRectF rect
//************************************
void IXTMap::setViewport(int dpi_x, int dpi_y, QRectF rect)
{
	CS57Control::setViewport(dpi_x, dpi_y, rect);
}

//************************************
// Method:    setCellBorderVisible
// Brief:	  设置图廓显隐
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: bool visible
//************************************
void IXTMap::setCellBorderVisible(bool visible)
{
	CS57Control::setBorderVisible(visible);
}

//************************************
// Method:    setGlobalMapVisible
// Brief:	  设置全球地图显隐
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: bool visible
//************************************
void IXTMap::setGlobalMapVisible(bool visible)
{
	CS57Control::setGlobalMapVisible(visible);
}

//************************************
// Method:    setCellLayerVisible
// Brief:	  设置图层显隐
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: bool visible
//************************************
void IXTMap::setCellLayerVisible(bool visible)
{
	CS57Control::setCellLayerVisible(visible);
}

//************************************
// Method:    setCanvasColor
// Brief:	  设置画布颜色
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QColor color
//************************************
void IXTMap::setCanvasColor(QColor color)
{
	CS57Control::setCanvasColor(color);
}

//************************************
// Method:    setDisplayCategory
// Brief:	  设置显示分组
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: XT::DisplayCategory value
//************************************
void IXTMap::setDisplayCategory(XT::DisplayCategory value)
{
	CS57Control::setDisplayCategory(value);
}

//************************************
// Method:    setColorMode
// Brief:	  设置颜色模式
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: XT::S52ColorMode value
//************************************
void IXTMap::setColorMode(XT::S52ColorMode value)
{
	CS57Control::setColorMode(value);
}

//************************************
// Method:    setOperateMode
// Brief:	  设置操作模式
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: XT::OperateMode mode
//************************************
void IXTMap::setOperateMode(XT::OperateMode mode)
{
	CS57Control::setOperateMode(mode);
}

//************************************
// Method:    setQueryMode
// Brief:	  设置查询模式
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: XT::QueryMode mode
//************************************
void IXTMap::setQueryMode(XT::QueryMode mode)
{
	CS57Control::setQueryMode(mode);
}

//************************************
// Method:    getSceneRect
// Brief:	  获取场景矩形
// Returns:   QT_NAMESPACE::QRectF
// Author:    cl
// DateTime:  2022/07/21
//************************************
QRectF IXTMap::getSceneRect()
{
	return CS57Control::getSceneRect();
}

//************************************
// Method:    getDPI
// Brief:	  获取DPI
// Returns:   double
// Author:    cl
// DateTime:  2022/07/21
//************************************
double IXTMap::getDPI()
{
	return CS57Control::getDpi();
}

//************************************
// Method:    updateDisplayScale
// Brief:	  更新显示比例尺
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: double scale
//************************************
void IXTMap::updateDisplayScale(double scale)
{
	CS57Control::updateDisplayScale(scale);
}

//************************************
// Method:    getDisplayScale
// Brief:	  获取显示比例尺
// Returns:   double
// Author:    cl
// DateTime:  2022/07/21
//************************************
double IXTMap::getDisplayScale()
{
	return CS57Control::getDisplayScale();
}

//************************************
// Method:    setMapTool
// Brief:	  设置地图工具
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CS57MapTool * tool
//************************************
void IXTMap::setMapTool(CS57MapTool* tool)
{
	CS57Control::setMapTool(tool);
}

//************************************
// Method:    zoomIn
// Brief:	  放大
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: double factor
//************************************
void IXTMap::zoomIn(double factor)
{
	CS57Control::zoomIn(factor);
}

//************************************
// Method:    zoomOut
// Brief:	  缩小
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: double factor
//************************************
void IXTMap::zoomOut(double factor)
{
	CS57Control::zoomOut(factor);
}

//************************************
// Method:    viewportLeft
// Brief:	  视口左移
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: int offset
//************************************
void IXTMap::viewportLeft(int offset)
{
	CS57Control::viewportLeft(offset);
}

//************************************
// Method:    viewportRight
// Brief:	  视口右移
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: int offset
//************************************
void IXTMap::viewportRight(int offset)
{
	CS57Control::viewportRight(offset);
}

//************************************
// Method:    viewportUp
// Brief:	  视口上移
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: int offset
//************************************
void IXTMap::viewportUp(int offset)
{
	CS57Control::viewportUp(offset);
}

//************************************
// Method:    viewportDown
// Brief:	  视口下移
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: int offset
//************************************
void IXTMap::viewportDown(int offset)
{
	CS57Control::viewportDown(offset);
}

//************************************
// Method:    transformScrnToGeo
// Brief:	  抓换屏幕坐标到地理坐标
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: int scrnPtX
// Parameter: int scrnPtY
// Parameter: double * lon
// Parameter: double * lat
// Parameter: double scale
//************************************
void IXTMap::transformScrnToGeo(int scrnPtX, int scrnPtY, double* lon, double* lat, double scale)
{
	CS57Control::transformScrnToGeo(scrnPtX, scrnPtY, lon, lat, scale);
}

//************************************
// Method:    transformGeoToScrn
// Brief:	  抓换地理坐标到屏幕坐标
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: double lon
// Parameter: double lat
// Parameter: int * scrnPtX
// Parameter: int * scrnPtY
// Parameter: double scale
//************************************
void IXTMap::transformGeoToScrn(double lon, double lat, int* scrnPtX, int* scrnPtY, double scale)
{
	CS57Control::transformGeoToScrn(lon, lat, scrnPtX, scrnPtY, scale);
}

//************************************
// Method:    addLayerToCanvas
// Brief:	  添加图层到画布
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CS57AbstractLayer * pLayer
//************************************
void IXTMap::addLayerToCanvas(CS57AbstractLayer* pLayer)
{
	CS57Control::addLayerToCanvas(pLayer);
}

//************************************
// Method:    removeLayer
// Brief:	  移除层
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CS57AbstractLayer * pLayer
//************************************
void IXTMap::removeLayer(CS57AbstractLayer* pLayer)
{
	CS57Control::removeLayer(pLayer);
}

//************************************
// Method:    removeLayerFromName
// Brief:	  通过层名称移除层
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: const QString & strLayerName
//************************************
void IXTMap::removeLayerFromName(const QString& strLayerName)
{
	CS57Control::removeLayerFromName(strLayerName);
}

//************************************
// Method:    createCanvas
// Brief:	  创建画布
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void IXTMap::createCanvas()
{
	CS57Control::createCanvas();
}

//************************************
// Method:    renderCanvas
// Brief:	  渲染画布
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void IXTMap::renderCanvas()
{
	CS57Control::renderCanvas();
}

//************************************
// Method:    refreshCanvas
// Brief:	  刷新画布
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void IXTMap::refreshCanvas()
{
	CS57Control::refreshCanvas();
}

//************************************
// Method:    updateCanvas
// Brief:	  更新画布
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void IXTMap::updateCanvas()
{
	CS57Control::updateCanvas();
}
