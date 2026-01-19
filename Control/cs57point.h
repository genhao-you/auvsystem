#pragma once
#include "control_global.h"
#include "cs57geometry.h"

class CS57Point : public CS57Geometry
{
	Q_OBJECT
public:
	CS57Point(CS57Control* control, const QString& name);
	~CS57Point();
};
