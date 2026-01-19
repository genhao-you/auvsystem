#pragma once
#include <QString>
class Coordinate
{
public:
	Coordinate();
	Coordinate(double x, double y);
	Coordinate(QString x, QString y);
	Coordinate(QString coord);
	~Coordinate();

	QString toString();

	double getX(){	return m_dX;	}
	double getY(){	return m_dY;	}

private:
	double m_dX;
	double m_dY;
};
