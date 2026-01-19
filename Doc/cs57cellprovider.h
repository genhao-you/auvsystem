#pragma once
#include "doc_global.h"
#include "xt_global.h"
#include "cs57abstractprovider.h"
#include <QVector>

namespace Doc
{
	class CS57Cell;
	class CS57CellMessage;
	class CS57RecFeature;
	class CS57McovrProvider;
	struct CellDisplayContext
	{
		CS57Cell* pS57Cell;
		QVector<CS57RecFeature*> vecPFeatures;
		QVector<CS57RecFeature*> vecLFeatures;
		QVector<CS57RecFeature*> vecAFeatures;
	};
	struct CellOpened
	{
		CS57Cell*	pS57Cell;
		int			nMcovrIdx;
	};
	class DOC_EXPORT CS57CellProvider : public CS57AbstractProvider
	{
	public:
		CS57CellProvider();
		CS57CellProvider(CS57McovrProvider* pMcovrProvider);
		~CS57CellProvider();

		void			setCellLayerVisible(bool visible);
		void			setDisplayCategory(XT::DisplayCategory displayCategory);
		void			setProvider(CS57McovrProvider* pMcovrProvider);
		void			selectChart(CSpatialPolygon* pPolygon, double scale);//智能选图
		void			selectChart(CSpatialPolygon* pPolygon, double scale, QStringList cellNames);//智能选图
		void			selectChart2(CSpatialPolygon* pPolygon, double scale);//视口范围内图幅不能关闭
		void			selectChart3(CSpatialPolygon* pPolygon, double scale);//只能开图，不允许关图

		void			releaseDisplayContext();
	private:
		bool			isInPolygon(CSpatialPolygon* pPolygon, CS57CellMessage* pMsg, CS57RecFeature* pFE);
		bool			isInPolygon(CSpatialPolygon* pPolygon, CS57Cell* pCell, CS57RecFeature* pFE);
		static bool		sortFeature(CS57RecFeature* pFE1, CS57RecFeature* pFE2);

	public:
		QVector<CellDisplayContext*>	m_vecCellDisplayContext;

	private:
		CS57McovrProvider*				m_pMcovrProvider;
		QVector<CellOpened*>			m_vecCellOpened;
		XT::DisplayCategory				m_eDisplayCategory;

		bool							m_bCellLayerVisible;
	};

}
