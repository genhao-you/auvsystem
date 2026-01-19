#pragma once

#include "cs57control.h"

class IXTMap : public CS57Control
{
	Q_OBJECT

public:
	IXTMap(QWidget *parent = nullptr);
	~IXTMap();

	//设置ENC所在目录
	void setEncDir(const QString& dir);

	//设置符号模式
	void setSymbolMode(XT::SymbolMode mode);

	//设置符号缩放模式
	void setSymbolScaleMode(XT::SymbolScaleMode mode = XT::ABS_SCALE);

	//加载ENC
	void loadEnc();

	//设置投影类型-当前仅支持MCT投影
	void setProjectionName(XT::ProjType type);

	//设置基准线
	void setdBaseLine(double baseLine);

	//设置视口中心点
	void setViewportCenterPt(double geo_lon, double geo_lat);

	//设置显示比例尺
	void setViewportDisplayScale(double displayScale);

	//设置视口
	void setViewport(int dpi_x, int dpi_y, QRectF rect);

	//设置图廓显隐
	void setCellBorderVisible(bool visible);

	//设置全球底图显隐
	void setGlobalMapVisible(bool visible);

	//设置图幅图层显隐
	void setCellLayerVisible(bool visible);

	//设置画布背景色
	void setCanvasColor(QColor color);

	//设置显示分组
	void setDisplayCategory(XT::DisplayCategory value);

	//设置海图颜色模式
	void setColorMode(XT::S52ColorMode value);

	//设置当前操作模式
	void setOperateMode(XT::OperateMode mode);

	//设置查询模式
	void setQueryMode(XT::QueryMode mode);

	//获取场景矩形区域
	QRectF getSceneRect();

	//获取显示设备当前DPI
	double getDPI();

	//更新显示比例尺
	void updateDisplayScale(double scale);

	//获取显示比例尺
	double getDisplayScale();

	//设置海图工具
	void setMapTool(CS57MapTool* tool);

	//画布放大
	void zoomIn(double factor = 0.5);

	//画布缩小
	void zoomOut(double factor = 0.5);

	//视口左移
	void viewportLeft(int offset = -100);
	
	//视口右移
	void viewportRight(int offset = 100);

	//视口上移
	void viewportUp(int offset = -100);

	//视口下移
	void viewportDown(int offset = 100);
	
	//转换屏幕坐标到经纬度坐标
	//scale = 0.0时 默认以当前比例尺换算
	void transformScrnToGeo(int scrnPtX, int scrnPtY, double* lon, double* lat, double scale = 0.0);

	//转换经纬度坐标到屏幕坐标
	//scale = 0.0时 默认以当前比例尺换算
	void transformGeoToScrn(double lon, double lat, int* scrnPtX, int* scrnPtY, double scale = 0.0);

	//创建画布
	void createCanvas();

	//添加图层到画布
	void addLayerToCanvas(CS57AbstractLayer* pLayer);

	//移除图层（内部已经释放图层）
	void removeLayer(CS57AbstractLayer* pLayer);

	//通过图层名称移除图层
	void removeLayerFromName(const QString& strLayerName);

	//渲染画布
	void renderCanvas();

	//刷新画布
	void refreshCanvas();

	//更新画布
	void updateCanvas();
};
