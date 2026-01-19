#include "elevationdata.h"

#include <QColor>

float ElevationData::getElevation(double lon, double lat) const
{
    // 检查坐标是否在范围内
    if (lon < minLon || lon > maxLon || lat < minLat || lat > maxLat)
        return NAN;

    // 计算像素坐标
    double xRatio = (lon - minLon) / (maxLon - minLon);
    double yRatio = (maxLat - lat) / (maxLat - minLat);

    int x = (int)(xRatio * width);
    int y = (int)(yRatio * height);

    // 确保在范围内
    x = qBound(0, x, width - 1);
    y = qBound(0, y, height - 1);

    // 返回高程值
    return data[y * width + x];
}

QImage ElevationData::toImage(int width, int height) const
{
    QImage image(width, height, QImage::Format_ARGB32);

    // 计算缩放比例
    double xScale = (double)this->width / width;
    double yScale = (double)this->height / height;

    // 为每个像素计算颜色
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // 采样原始数据（简化版，实际应使用双线性插值）
            int srcX = (int)(x * xScale);
            int srcY = (int)(y * yScale);

            srcX = qBound(0, srcX, this->width - 1);
            srcY = qBound(0, srcY, this->height - 1);

            float elev = data[srcY * this->width + srcX];

            // 根据高程设置颜色
            QColor color;
            if (elev < 0) {
                // 水下 - 蓝色系
                double depthRatio = qMin(1.0, -elev / 5000.0); // 5000米以下都用最深色
                color.setRgb(0, 0, 150 + (int)(105 * (1 - depthRatio)));
            }
            else {
                // 陆地 - 从绿到棕到白
                double elevRatio = qMin(1.0, elev / 8000.0); // 8000米以上都用白色

                if (elevRatio < 0.2) {
                    // 低地 - 绿色
                    color.setRgb(0, 180 + (int)(75 * elevRatio / 0.2), 0);
                }
                else if (elevRatio < 0.6) {
                    // 山地 - 棕色
                    double ratio = (elevRatio - 0.2) / 0.4;
                    color.setRgb(139 + (int)(50 * ratio), 69 + (int)(30 * ratio), 19);
                }
                else {
                    // 高地/雪山 - 白色
                    double ratio = (elevRatio - 0.6) / 0.4;
                    color.setRgb(200 + (int)(55 * ratio), 200 + (int)(55 * ratio), 200 + (int)(55 * ratio));
                }
            }

            image.setPixel(x, y, color.rgba());
        }
    }

    return image;
}