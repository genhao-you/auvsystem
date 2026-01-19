#pragma once
#include <QVector>
#include <QPoint>

typedef QVector<QPoint> CPolyline;
typedef QVector<QPointF> CPolylineF;
typedef QVector<CPolyline> CPolylines;
typedef QVector<CPolyline*> _CPolylines;
typedef QVector<CPolylineF> CPolylineFs;
typedef QVector<CPolylineF*> _CPolylineFs;
typedef QVector<QVector<QPoint>> CPolylines;
typedef QVector<QVector<QPointF>> CPolylineFs;
typedef QVector<QVector<QPoint>*> _CPolylines;
typedef QVector<QVector<QPointF>*> _CPolylineFs;
