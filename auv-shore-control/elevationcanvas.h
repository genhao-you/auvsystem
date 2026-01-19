#pragma once

#include <QObject>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QWheelEvent>
#include <gdal_priv.h>
#include"elevationdata.h"
class TifMapTool;
class ElevationCanvas  : public QGraphicsView
{
	Q_OBJECT

public:
	ElevationCanvas(QWidget * parent = nullptr);
	~ElevationCanvas();
    // 加载TIF文件
    bool loadTif(const QString& filePath);

    // 缩放功能
    void zoomIn(double factor = 0.8);
    void zoomOut(double factor = 1.25);

    // 设置当前工具
    void setTool(class TifMapTool* tool) { m_currentTool = tool; }

    // 坐标转换
    void screenToGeo(int x, int y, double& lon, double& lat);
    void geoToScreen(double lon, double lat, int& x, int& y);

    // 获取当前高程数据
    ElevationData* getElevationData() { return m_elevData; }

signals:
    // 坐标更新信号
    void geoPositionChanged(const QString& pos);
    // 缩放级别更新
    void scaleChanged(double scale);

protected:
    // 重写事件处理
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    // 渲染高程数据
    void renderElevation();

    // 计算视图范围
    void updateViewRect();

    QGraphicsScene* m_scene;
    QGraphicsPixmapItem* m_mapItem;
    class TifMapTool* m_currentTool;

    // 高程数据
    ElevationData* m_elevData;

    // 地理范围
    double m_minLon, m_maxLon;
    double m_minLat, m_maxLat;

    // 当前视图
    double m_viewLon, m_viewLat;
    double m_zoomLevel;

    // 鼠标状态
    bool m_isDragging;
    QPoint m_lastMousePos;
};
