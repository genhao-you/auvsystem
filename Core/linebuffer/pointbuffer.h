#pragma once
#include "coordinate.h"
class PointBuffer
{
public:
	PointBuffer();
	~PointBuffer();

	static QString GetBufferEdgeCoords(Coordinate center, double radius);
};
