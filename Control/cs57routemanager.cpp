#include "cs57routemanager.h"
#include "cs57route.h"
#include <QDebug>


CS57RouteManager::CS57RouteManager(CS57Control* control)
	: m_pControl(control)
{}
CS57RouteManager::~CS57RouteManager()
{
	for (int i = 0; i < m_vecRoute.size(); i++)
	{
		if (m_vecRoute[i] != nullptr)
			delete m_vecRoute[i];
		m_vecRoute[i] = nullptr;
	}
	m_vecRoute.clear();
	for (int i = 0; i < m_vecTempRoute.size(); i++)
	{
		if (m_vecTempRoute[i] != nullptr)
			delete m_vecTempRoute[i];
		m_vecTempRoute[i] = nullptr;
	}
	m_vecTempRoute.clear();
}

//************************************
// Method:    addRoute
// Brief:	  添加航线
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: RouteType type
// Parameter: CS57Route * route
//************************************
void CS57RouteManager::addRoute(RouteType type, CS57Route* route)
{
	bool hasAdded = false;
	switch (type)
	{
	case Normal:
		for (int i = 0; i < m_vecRoute.size(); i++)
		{
			QString strRouteName = route->name();
			if (strRouteName == m_vecRoute[i]->name())
				hasAdded = true;
		}
		if (!hasAdded)
		{
			m_vecRoute.push_back(route);
		}
		break;
	case Temp:
		for (int i = 0; i < m_vecTempRoute.size(); i++)
		{
			QString strRouteName = route->name();
			if (strRouteName == m_vecTempRoute[i]->name())
				hasAdded = true;
		}
		if (!hasAdded)
		{
			m_vecTempRoute.push_back(route);
		}
		break;
	}
}

//************************************
// Method:    releaseRoute
// Brief:	  释放航线
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: RouteType type
// Parameter: int idx
//************************************
void CS57RouteManager::releaseRoute(RouteType type, int idx)
{
	switch (type)
	{
	case Normal:
	{
		int No = 0;
		QVector<CS57Route*>::iterator iter;
		for (iter = m_vecRoute.begin(); iter != m_vecRoute.end(); iter++)
		{
			if (No == idx)
			{
				delete *iter;
				*iter = nullptr;

				iter = m_vecRoute.erase(iter);
				if (iter == m_vecRoute.end())
					break;
				break;
			}
			No++;
		}
		break;
	}
	case Temp:
	{
		int No = 0;
		QVector<CS57Route*>::iterator iter;
		for (iter = m_vecTempRoute.begin(); iter != m_vecTempRoute.end(); iter++)
		{
			if (No == idx)
			{
				delete *iter;
				*iter = nullptr;

				iter = m_vecTempRoute.erase(iter);
				if (iter == m_vecTempRoute.end())
					break;
				break;
			}
			No++;
		}
		break;
	}
	}
}

//************************************
// Method:    releaseAllRoute
// Brief:	  释放所有航线
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: RouteType type
//************************************
void CS57RouteManager::releaseAllRoute(RouteType type)
{
	switch (type)
	{
	case Normal:
		for (int i = 0; i < m_vecRoute.size(); i++)
		{
			if (m_vecRoute[i] != nullptr)
				delete m_vecRoute[i];
			m_vecRoute[i] = nullptr;
		}
		m_vecRoute.clear();
		break;
	case Temp:
		for (int i = 0; i < m_vecTempRoute.size(); i++)
		{
			if (m_vecTempRoute[i] != nullptr)
				delete m_vecTempRoute[i];
			m_vecTempRoute[i] = nullptr;
		}
		m_vecTempRoute.clear();
		break;
	}
}

//************************************
// Method:    removeRoute
// Brief:	  移除航线
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: RouteType type
// Parameter: int idx
//************************************
void CS57RouteManager::removeRoute(RouteType type, int idx)
{
	switch (type)
	{
	case Normal:
		for (int i = 0; i < m_vecRoute.size(); i++)
		{
			if (idx == i)
			{
				m_pControl->getScene()->removeItem(m_vecRoute[i]);//只移除元素，不释放数据
				break;
			}
		}
		break;
	case Temp:
		for (int i = 0; i < m_vecTempRoute.size(); i++)
		{
			if (idx == i)
			{
				m_pControl->getScene()->removeItem(m_vecTempRoute[i]);//只移除元素，不释放数据
				break;
			}
		}
		break;
	}
}

//************************************
// Method:    removeRoute
// Brief:	  移除航线
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: RouteType type
// Parameter: const QString & name
//************************************
void CS57RouteManager::removeRoute(RouteType type, const QString& name /*= ""*/)
{
	switch (type)
	{
	case Normal:
		for (int i = 0; i < m_vecRoute.size(); i++)
		{
			if (m_vecRoute[i]->name() == name)
			{
				m_pControl->getScene()->removeItem(m_vecRoute[i]);//只移除元素，不释放数据
				break;
			}
		}
		break;
	case Temp:
		for (int i = 0; i < m_vecTempRoute.size(); i++)
		{
			if (m_vecTempRoute[i]->name() == name)
			{
				m_pControl->getScene()->removeItem(m_vecTempRoute[i]);//只移除元素，不释放数据
				break;
			}
		}
		break;
	}
}

//************************************
// Method:    releaseRoute
// Brief:	  释放航线
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: RouteType type
// Parameter: const QString & name
//************************************
void CS57RouteManager::releaseRoute(RouteType type, const QString& name)
{
	switch (type)
	{
	case Normal:
	{
		QVector<CS57Route*>::iterator iter;
		for (iter = m_vecRoute.begin(); iter != m_vecRoute.end(); iter++)
		{
			if ((*iter)->name() == name)
			{
				delete *iter;
				*iter = nullptr;

				iter = m_vecRoute.erase(iter);
				if (iter == m_vecRoute.end())
					break;
				break;
			}
		}
		break;
	}
	case Temp:
	{
		QVector<CS57Route*>::iterator iter;
		for (iter = m_vecTempRoute.begin(); iter != m_vecTempRoute.end(); iter++)
		{
			if ((*iter)->name() == name)
			{
				delete *iter;
				*iter = nullptr;

				iter = m_vecTempRoute.erase(iter);
				if (iter == m_vecTempRoute.end())
					break;
				break;
			}
		}
		break;
	}
	}
}

//************************************
// Method:    removeAllRoute
// Brief:	  移除所有航线
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: RouteType type
//************************************
void CS57RouteManager::removeAllRoute(RouteType type)
{
	switch (type)
	{
	case Normal:
		for (int i = 0; i < m_vecRoute.size(); i++)
		{
			m_pControl->getScene()->removeItem(m_vecRoute[i]);
		}
		break;
	case Temp:
		for (int i = 0; i < m_vecTempRoute.size(); i++)
		{
			m_pControl->getScene()->removeItem(m_vecTempRoute[i]);
		}
		break;
	}
}

//************************************
// Method:    renderAllRoute
// Brief:	  渲染所有航线
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: RouteType type
//************************************
void CS57RouteManager::renderAllRoute(RouteType type)
{
	switch (type)
	{
	case Normal:
		for (int i = 0; i < m_vecRoute.size(); i++)
		{
			CS57Route* route = new CS57Route(m_pControl);
			route->copy(m_vecRoute[i]);
			route->calcBoundingBox();
			route->show();

			delete m_vecRoute[i];
			m_vecRoute[i] = nullptr;

			m_vecRoute[i] = route;
		}
		//qDebug() << m_vecRoute.size() << endl;
		break;
	case Temp:
		for (int i = 0; i < m_vecTempRoute.size(); i++)
		{
			CS57Route* route = new CS57Route(m_pControl, m_vecTempRoute[i]->name());
			route->copy(m_vecTempRoute[i]);
			route->calcBoundingBox();
			route->show();

			delete m_vecTempRoute[i];
			m_vecTempRoute[i] = nullptr;

			m_vecTempRoute[i] = route;
		}
		//qDebug() << m_vecTempRoute.size() << endl;
		break;
	}
}

//************************************
// Method:    getRoute
// Brief:	  获取航线
// Returns:   CS57Route*
// Author:    cl
// DateTime:  2022/07/21
// Parameter: RouteType type
// Parameter: const QString & name
//************************************
CS57Route* CS57RouteManager::getRoute(RouteType type, const QString & name)
{
	switch (type)
	{
	case Normal:
		for (int i = 0; i < m_vecRoute.size(); i++)
		{
			QString strName = m_vecRoute[i]->name();
			if (strName == name)
			{
				return m_vecRoute[i];
			}
		}
		break;
	case Temp:
		for (int i = 0; i < m_vecTempRoute.size(); i++)
		{
			QString strName = m_vecTempRoute[i]->name();
			if (strName == name)
			{
				return m_vecTempRoute[i];
			}
		}
		break;
	}
	return nullptr;
}

//************************************
// Method:    routeExists
// Brief:	  航线是否存在
// Returns:   bool
// Author:    cl
// DateTime:  2022/07/21
// Parameter: RouteType type
// Parameter: const QString & name
//************************************
bool CS57RouteManager::routeExists(RouteType type, const QString& name)
{
	bool hasExists = false;
	switch (type)
	{
	case Normal:
		for (int i = 0; i < m_vecRoute.size(); i++)
		{
			if (m_vecRoute[i]->name() == name)
				hasExists = true;
		}
		break;
	case Temp:
		for (int i = 0; i < m_vecTempRoute.size(); i++)
		{
			if (m_vecTempRoute[i]->name() == name)
				hasExists = true;
		}
		break;
	}
	return hasExists;
}
