#pragma once
#include "doc_global.h"
#include <QVector>
#include <QPoint>

namespace Doc
{
	// 符号公用定义
	struct S52Circle
	{
		QPoint					cenPoint;
		unsigned short			radius;
		unsigned short			modeFill = 0;	// 0,画线，１，填充，２画线并填充
	};
	struct S52Polygon
	{
		QVector<QPoint*>		vecLine;
		unsigned short			modeFill = 0;	// 0,画线，１，填充，２画线并填充
	};
	struct S52Line
	{
		QVector<QPoint*>		vecLine;
	};
	struct S52Layer
	{
		int						nPenWidth = 1;
		QString					strColorName = "CHBLK";
		unsigned short			uTransparent = 0;
		QVector<S52Line*>		vecLines;
		QVector<S52Polygon*>	vecPolygon;
		QVector<S52Circle*>		vecCircle;
	};
}

