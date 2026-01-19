#include "elevationcanvas.h"
#include <QGraphicsPixmapItem>
#include <QMessageBox>
#include <QDebug>
#include"tifmaptool.h"
ElevationCanvas::ElevationCanvas(QWidget* parent)
    : QGraphicsView(parent)
    , m_scene(new QGraphicsScene(this))
    , m_mapItem(new QGraphicsPixmapItem)
    , m_currentTool(nullptr)
    , m_elevData(nullptr), m_zoomLevel(1.0), m_isDragging(false)
{
    setScene(m_scene);
    m_scene->addItem(m_mapItem);

    // 初始化视图范围（全球范围）
    m_minLon = -180.0;
    m_maxLon = 180.0;
    m_minLat = -90.0;
    m_maxLat = 90.0;
    m_viewLon = 0.0;
    m_viewLat = 0.0;
}

ElevationCanvas::~ElevationCanvas()
{
    delete m_elevData;
}

bool ElevationCanvas::loadTif(const QString& filePath)
{
    // 释放旧数据
    if (m_elevData) {
        delete m_elevData;
        m_elevData = nullptr;
    }

    // 使用GDAL加载TIF文件
    GDALAllRegister();
    GDALDataset* dataset = (GDALDataset*)GDALOpen(filePath.toUtf8().constData(), GA_ReadOnly);
    if (!dataset) {
        QMessageBox::critical(this, "错误", "无法打开TIF文件");
        return false;
    }

    // 创建高程数据对象
    m_elevData = new ElevationData();
    m_elevData->width = dataset->GetRasterXSize();
    m_elevData->height = dataset->GetRasterYSize();

    // 获取地理范围
    double adfGeoTransform[6];
    if (dataset->GetGeoTransform(adfGeoTransform) == CE_None) {
        m_elevData->minLon = adfGeoTransform[0];
        m_elevData->maxLon = adfGeoTransform[0] + adfGeoTransform[1] * m_elevData->width;
        m_elevData->maxLat = adfGeoTransform[3];
        m_elevData->minLat = adfGeoTransform[3] + adfGeoTransform[5] * m_elevData->height;

        m_minLon = m_elevData->minLon;
        m_maxLon = m_elevData->maxLon;
        m_minLat = m_elevData->minLat;
        m_maxLat = m_elevData->maxLat;

        // 计算中心位置
        m_viewLon = (m_minLon + m_maxLon) / 2;
        m_viewLat = (m_minLat + m_maxLat) / 2;
    }

    // 读取高程数据
    GDALRasterBand* band = dataset->GetRasterBand(1);
    m_elevData->data.resize(m_elevData->width * m_elevData->height);
    band->RasterIO(GF_Read, 0, 0, m_elevData->width, m_elevData->height,
        m_elevData->data.data(), m_elevData->width, m_elevData->height,
        GDT_Float32, 0, 0);

    // 计算最小和最大高程
    m_elevData->minElev = m_elevData->data[0];
    m_elevData->maxElev = m_elevData->data[0];
    for (float val : m_elevData->data) {
        if (val < m_elevData->minElev) m_elevData->minElev = val;
        if (val > m_elevData->maxElev) m_elevData->maxElev = val;
    }

    GDALClose(dataset);

    // 渲染高程图
    renderElevation();
    return true;
}

void ElevationCanvas::renderElevation()
{
    if (!m_elevData) return;

    // 转换高程数据为图像
    QImage image = m_elevData->toImage(width(), height());
    m_mapItem->setPixmap(QPixmap::fromImage(image));

    // 更新视图
    setSceneRect(0, 0, image.width(), image.height());
    updateViewRect();
}

void ElevationCanvas::screenToGeo(int x, int y, double& lon, double& lat)
{
    // 计算屏幕坐标比例
    double xRatio = (double)x / width();
    double yRatio = (double)y / height();

    // 转换为地理坐标
    lon = m_minLon + (m_maxLon - m_minLon) * xRatio;
    lat = m_maxLat - (m_maxLat - m_minLat) * yRatio;
}

void ElevationCanvas::geoToScreen(double lon, double lat, int& x, int& y)
{
    // 计算地理坐标比例
    double xRatio = (lon - m_minLon) / (m_maxLon - m_minLon);
    double yRatio = (m_maxLat - lat) / (m_maxLat - m_minLat);

    // 转换为屏幕坐标
    x = (int)(xRatio * width());
    y = (int)(yRatio * height());
}

void ElevationCanvas::zoomIn(double factor)
{
    m_zoomLevel *= factor;
    scale(factor, factor);
    emit scaleChanged(m_zoomLevel);
}

void ElevationCanvas::zoomOut(double factor)
{
    m_zoomLevel *= factor;
    scale(factor, factor);
    emit scaleChanged(m_zoomLevel);
}

void ElevationCanvas::updateViewRect()
{
    // 更新当前视图范围
    QRectF viewRect = mapToScene(viewport()->rect()).boundingRect();
    screenToGeo(viewRect.left(), viewRect.top(), m_minLon, m_maxLat);
    screenToGeo(viewRect.right(), viewRect.bottom(), m_maxLon, m_minLat);
}

// 事件处理
void ElevationCanvas::mousePressEvent(QMouseEvent* event)
{
    if (m_currentTool) {
        m_currentTool->mousePressEvent(event);
    }
    m_lastMousePos = event->pos();
    m_isDragging = true;
    QGraphicsView::mousePressEvent(event);
}

void ElevationCanvas::mouseMoveEvent(QMouseEvent* event)
{
    if (m_currentTool) {
        m_currentTool->mouseMoveEvent(event);
    }

    // 计算鼠标位置的地理坐标并发送信号
    if (m_isDragging) {
        double lon, lat;
        screenToGeo(event->x(), event->y(), lon, lat);
        emit geoPositionChanged(QString("经度: %1, 纬度: %2").arg(lon, 0, 'f', 6).arg(lat, 0, 'f', 6));
    }

    m_lastMousePos = event->pos();
    QGraphicsView::mouseMoveEvent(event);
}

void ElevationCanvas::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_currentTool) {
        m_currentTool->mouseReleaseEvent(event);
    }
    m_isDragging = false;
    QGraphicsView::mouseReleaseEvent(event);
}

void ElevationCanvas::wheelEvent(QWheelEvent* event)
{
    if (m_currentTool) {
        m_currentTool->wheelEvent(event);
    }
    else {
        // 默认滚轮缩放
        if (event->angleDelta().y() > 0) {
            zoomIn();
        }
        else {
            zoomOut();
        }
    }
    QGraphicsView::wheelEvent(event);
}

void ElevationCanvas::resizeEvent(QResizeEvent* event)
{
    renderElevation();
    QGraphicsView::resizeEvent(event);
}
