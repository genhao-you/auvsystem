#include "coordinate.h"
#include <QVector>
#include <QStringList>

Coordinate::Coordinate()
{
}

Coordinate::~Coordinate()
{
}

QString Coordinate::toString()
{
	return "(" + QString::number(m_dX, 'f', 6) + "," + QString::number(m_dY, 'f', 6) + ")";
}

Coordinate::Coordinate(double x, double y)
{
	m_dX = x;
	m_dY = y;
}

Coordinate::Coordinate(QString x, QString y)
{
	m_dX = x == "" ? 0. : x.toDouble();
	m_dY = y == "" ? 0. : y.toDouble();
}

Coordinate::Coordinate(QString coord)
{
	if (coord.length() > 0)
	{
		QStringList values = coord.split(",");
		if (values.size() == 2)
		{
			m_dX = values[0].length() > 0 ? values[0].toDouble() : 0.;
			m_dY = values[1].length() > 0 ? values[1].toDouble() : 0.;
		}
	}
}

