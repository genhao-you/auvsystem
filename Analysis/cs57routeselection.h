#pragma once

#include "cs57route.h"
#include "cs57chartselectionl.h"


namespace Analysis
{
	//航线区域分段
	class CS57RouteSelection : public CS57ChartSelectionL
	{
	public:
		CS57RouteSelection();
		~CS57RouteSelection();
		//
		void routeRegion(CS57Route* route);
		//开始抽选
		void startSelect();

		//获取区域I航线
		CPolylineFs* getRoutePtsI();
		//获取区域II航线
		CPolylineFs* getRoutePtsII();
		//获取区域III航线
		CPolylineFs* getRoutePtsIII();
		//获取区域IV航线
		CPolylineFs* getRoutePtsIV();
		//获取区域V航线
		CPolylineFs* getRoutePtsV();
	private:
		//区域I抽选
		void startSelectI();
		//区域II抽选
		void startSelectII();
		//区域III抽选
		void startSelectIII();
		//区域IV抽选
		void startSelectIV();
		//区域V抽选
		void startSelectV();

		void routeRegion(CS57Route* route, GpcPolygon* gpc, CPolylineFs& result);
		void holeClip(CPolylineFs& vpInPolygons, CPolylineFs& result);//内环裁剪

		void translate2GeoResult(CPolylineFs& vecRoutes);

	private:
		CPolylineFs m_vecRoutesI;
		CPolylineFs m_vecRoutesII;
		CPolylineFs m_vecRoutesIII;
		CPolylineFs m_vecRoutesIV;
		CPolylineFs m_vecRoutesV;

		QVector<CS57CellMessage*>	m_vecSelectedMcovrsI;
		QVector<CS57CellMessage*>	m_vecSelectedMcovrsII;
		QVector<CS57CellMessage*>	m_vecSelectedMcovrsIII;
		QVector<CS57CellMessage*>	m_vecSelectedMcovrsIV;
		QVector<CS57CellMessage*>	m_vecSelectedMcovrsV;
	};

}
