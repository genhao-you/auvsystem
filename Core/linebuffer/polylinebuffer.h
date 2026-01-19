#pragma once
#include "coordinate.h"

class PolylineBuffer
{
public:
	PolylineBuffer();
	~PolylineBuffer();

	static QString GetBufferEdgeCoords(QString strPolyLineCoords, double radius);

	static QString GetLeftBufferEdgeCoords(QVector<Coordinate> coords, double radius);

	static QString GetBufferCoordsByRadian(Coordinate center, double startRadian, double endRadian, double radius, bool horizontal);

	static double GetVectorProduct(Coordinate preCoord, Coordinate midCoord, Coordinate nextCoord);
};
