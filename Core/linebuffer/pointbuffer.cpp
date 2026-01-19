#include "pointbuffer.h"
#include <qmath.h>
#define PI 3.1415926
#define N 12
PointBuffer::PointBuffer()
{}

PointBuffer::~PointBuffer()
{}
QString PointBuffer::GetBufferEdgeCoords(Coordinate center, double radius)
{
	double alpha = 0.0;//Math.PI / 6;
	double gamma = (2 * PI) / N;
	QString strCoords = "";
	double x = 0.0, y = 0.0;
	for (double i = 0; i < (N - 1) * gamma; i += gamma)
	{
		x = center.getX() + radius * cos(alpha + i);
		y = center.getY() + radius * sin(alpha + i);
		if (strCoords.length() > 0) strCoords.append(";");
		strCoords.append(QString::number(x, 'f', 6) + "," + QString::number(y, 'f', 6));
	}
	return strCoords;
}