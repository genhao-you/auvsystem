#include "polylinebuffer.h"
#include "mathtool.h"
#include "pointbuffer.h"
#include <qmath.h>
#include <QVector>
#include <QStringList>
#define PI 3.1415926

PolylineBuffer::PolylineBuffer()
{}

PolylineBuffer::~PolylineBuffer()
{}

QString PolylineBuffer::GetBufferEdgeCoords(QString strPolyLineCoords, double radius)
{
	if (strPolyLineCoords.length() < 1)
		return "";

	QVector<Coordinate> vecCoos;
	QStringList values = strPolyLineCoords.split(";");
	for (int i = 0; i < values.size(); i++)
	{
		vecCoos.push_back(Coordinate(values[i]));
	}

	//分别生成左侧和右侧的缓冲区边界点坐标串
	QString leftBufferCoords = GetLeftBufferEdgeCoords(vecCoos, radius);
	std::reverse(vecCoos.begin(), vecCoos.end());//逆序
	QString rightBufferCoords = GetLeftBufferEdgeCoords(vecCoos, radius);
	return leftBufferCoords + ";" + rightBufferCoords;
}

QString PolylineBuffer::GetLeftBufferEdgeCoords(QVector<Coordinate> coords, double radius)
{
	//参数处理
	if (coords.size() < 1)
		return "";
	else if (coords.size() < 2)
		return PointBuffer::GetBufferEdgeCoords(coords[0], radius);
	//计算时所需变量
	double alpha = 0.0;//向量绕起始点沿顺时针方向旋转到X轴正半轴所扫过的角度
	double delta = 0.0;//前后线段所形成的向量之间的夹角
	double l = 0.0;//前后线段所形成的向量的叉积
				   //辅助变量
	QString strCoords = "";
	double startRadian = 0.0;
	double endRadian = 0.0;
	double beta = 0.0;
	double x = 0.0, y = 0.0;
	//第一节点的缓冲区
	{
		alpha = MathTool::GetQuadrantAngle(coords[0], coords[1]);
		startRadian = alpha + PI;
		endRadian = alpha + (3 * PI) / 2;
		bool horizontal = (alpha == 0 && coords[0].getX() > coords[1].getX()) ? true : false;
		strCoords.append(GetBufferCoordsByRadian(coords[0], startRadian, endRadian, radius, horizontal));
	}
	//中间节点
	for (int i = 1; i < coords.size() - 1; i++)
	{
		alpha = MathTool::GetQuadrantAngle(coords[i], coords[i + 1]);
		//delta = MathTool::GetIncludedAngle(coords[i - 1], coords[i], coords[i + 1]);
		beta = MathTool::GetQuadrantAngle(coords[i - 1], coords[i]);
		delta = alpha - beta;
		l = GetVectorProduct(coords[i - 1], coords[i], coords[i + 1]);
		if (l > 0)
		{
			//startRadian = alpha + (3 * PI) / 2 - delta;
			//endRadian = alpha + (3 * PI) / 2;
			startRadian = beta - PI / 2;
			endRadian = alpha - PI / 2;
			if (strCoords.length() > 0) strCoords.append(";");
			bool horizontal = (alpha == 0 && coords[i].getX() > coords[i + 1].getX()) ? true : false;
			strCoords.append(GetBufferCoordsByRadian(coords[i], startRadian, endRadian, radius, horizontal));
		}
		else if (l < 0)
		{
			//beta = alpha - (PI - delta) / 2;
			//bool horizontal = (alpha == 0 && coords[i].GetX() > coords[i + 1].GetX()) ? true : false;
			//double ratio = delta < PI / 2 ? 1.2 : 3.2;
			//if (horizontal)
			//{
			//	x = coords[i].GetX() - (radius * cos(beta)) * ratio;
			//	y = coords[i].GetY() - (radius * sin(beta)) * ratio;
			//}
			//else
			//{
			//	x = coords[i].GetX() + (radius * cos(beta)) * ratio;
			//	y = coords[i].GetY() + (radius * sin(beta)) * ratio;
			//}
			//if (strCoords.length() > 0) strCoords.append(";");
			//strCoords.append(to_string(x) + "," + to_string(y));


			float k = (PI - delta) / 2;
			float r = alpha + k;
			float d = -radius / sin(k);

			x = coords[i].getX() + d * cos(r);
			y = coords[i].getY() + d * sin(r);

			if (strCoords.length() > 0) strCoords.append(";");
			strCoords.append(QString::number(x,'f',6) + "," + QString::number(y,'f',6));
		}
	}
	//最后一个点
	{
		alpha = MathTool::GetQuadrantAngle(coords[coords.size() - 2], coords[coords.size() - 1]);
		startRadian = alpha + (3 * PI) / 2;
		endRadian = alpha + 2 * PI;
		if (strCoords.length() > 0) strCoords.append(";");
		bool horizontal = (alpha == 0 && coords[coords.size() - 2].getX() > coords[coords.size() - 1].getX()) ? true : false;
		strCoords.append(GetBufferCoordsByRadian(coords[coords.size() - 1], startRadian, endRadian, radius, horizontal));
	}
	return strCoords;
}

QString PolylineBuffer::GetBufferCoordsByRadian(Coordinate center, double startRadian, double endRadian, double radius, bool horizontal)
{
	QString strCoords = "";
	double x = 0.0, y = 0.0;
	double cosa = (radius - 1) / radius;
	double gamma = acos(cosa) * 3 / 2;
	while (endRadian - startRadian < 0) endRadian += 2 * PI;
	for (double i = startRadian; i <= endRadian; i += gamma)
	{
		x = center.getX() + radius * cos(i);
		y = center.getY() + radius * sin(i);
		if (strCoords.length() > 0) strCoords.append(";");
		strCoords.append(QString::number(x,'f',6) + "," + QString::number(y, 'f', 6));
	}

	//double gamma = PI / 18;
	//string strCoords = "";
	//double x = 0.0, y = 0.0;
	//for (double i = startRadian; i <= endRadian + 0.000000000000001; i += gamma)
	//{
	//	if (horizontal)
	//	{
	//		x = center.GetX() - radius * cos(i);
	//		y = center.GetY() - radius * sin(i);
	//	}
	//	else
	//	{
	//		x = center.GetX() + radius * cos(i);
	//		y = center.GetY() + radius * sin(i);
	//	}
	//	if (strCoords.length() > 0) strCoords.append(";");
	//	strCoords.append(to_string(x) + "," + to_string(y));
	//}
	return strCoords;
}

double PolylineBuffer::GetVectorProduct(Coordinate preCoord, Coordinate midCoord, Coordinate nextCoord)
{
	return (midCoord.getX() - preCoord.getX()) * (nextCoord.getY() - midCoord.getY()) - (nextCoord.getX() - midCoord.getX()) * (midCoord.getY() - preCoord.getY());
}
