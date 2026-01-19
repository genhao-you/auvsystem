#include "mathtool.h"
#include <qmath.h>
#define PI 3.1415926

MathTool::MathTool()
{}

MathTool::~MathTool()
{}

double MathTool::GetQuadrantAngle(Coordinate preCoord, Coordinate nextCoord)
{
	return atan2(nextCoord.getY() - preCoord.getY(), nextCoord.getX() - preCoord.getX());
}

double MathTool::GetQuadrantAngle(double x, double y)
{
	double theta = atan(y / x);
	if (x > 0 && y > 0) return theta;
	if (x > 0 && y < 0) return PI * 2 + theta;
	if (x < 0 && y > 0) return theta + PI;
	if (x < 0 && y < 0) return theta + PI;
	return theta;
}

double MathTool::GetIncludedAngle(Coordinate preCoord, Coordinate midCoord, Coordinate nextCoord)
{
	double innerProduct = (midCoord.getX() - preCoord.getX()) * (nextCoord.getX() - midCoord.getX()) + (midCoord.getY() - preCoord.getY()) * (nextCoord.getY() - midCoord.getY());
	double mode1 = sqrt(pow((midCoord.getX() - preCoord.getX()), 2.0) + pow((midCoord.getY() - preCoord.getY()), 2.0));
	double mode2 = sqrt(pow((nextCoord.getX() - midCoord.getX()), 2.0) + pow((nextCoord.getY() - midCoord.getY()), 2.0));
	return acos(innerProduct / (mode1 * mode2));
}

double MathTool::GetDistance(Coordinate preCoord, Coordinate nextCoord)
{
	return sqrt(pow((nextCoord.getX() - preCoord.getX()), 2) + pow((nextCoord.getY() - preCoord.getY()), 2));
}
