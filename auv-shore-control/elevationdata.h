#pragma once

#include <QObject>
#include <QVector>
#include <QImage>

// 高程数据结构
class ElevationData
{
public:
    ElevationData() : width(0), height(0), minElev(0), maxElev(0) {}

    int width;          // 宽度
    int height;         // 高度
    QVector<float> data;// 高程数据
    double minLon;      // 最小经度
    double maxLon;      // 最大经度
    double minLat;      // 最小纬度
    double maxLat;      // 最大纬度
    double minElev;     // 最小高程
    double maxElev;     // 最大高程

    // 获取指定经纬度的高程值
    float getElevation(double lon, double lat) const;

    // 转换为彩色图像
    QImage toImage(int width, int height) const;
};

