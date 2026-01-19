#pragma once

#include <QObject>
#include "cs57control.h"
class CS57Route;
class CS57RouteManager : public QObject
{
	Q_OBJECT

public:
	CS57RouteManager(CS57Control* control);
	~CS57RouteManager();
	enum RouteType
	{
		Normal,	//长期的
		Temp	//临时的
	};
	CS57Route* getRoute(RouteType type, const QString& name);
	bool routeExists(RouteType type, const QString& name);
	void addRoute(RouteType type,CS57Route* route);
	void removeRoute(RouteType type, int idx);
	void removeRoute(RouteType type, const QString& name = "");
	void removeAllRoute(RouteType type);
	void releaseRoute(RouteType type, int idx);
	void releaseRoute(RouteType type, const QString& name = "");
	void releaseAllRoute(RouteType type);

	void renderAllRoute(RouteType type);

private:
	QVector<CS57Route*> m_vecRoute;
	QVector<CS57Route*> m_vecTempRoute;
	CS57Control* m_pControl;
};
