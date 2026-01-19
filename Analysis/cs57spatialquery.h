#pragma once
#include "xt_global.h"
#include "analysis_global.h"
#include "cs57recfeature.h"
#include "cs57projection.h"
#include "cs57viewport.h"
#include "cs57cootranshelper.h"
#include <QPoint>

using namespace Doc;
namespace Analysis
{
	class CS57FeatureQuery;
	class ANALYSIS_EXPORT CS57SpatialQuery
	{
	public:
		CS57SpatialQuery();
		~CS57SpatialQuery();

		void setProj(CS57Projection* proj);
		void setViewport(CS57Viewport* viewport);
		void setPresLib(CS57PresLib* prelib);
		void setProvider(CS57CellProvider* provider);
		void startHighlight();
		void stopHighlight();
		void closeFeatureQuery();
		void setHighlightRow(int row);
		void queryFeatureInfo(XT::QueryMode mode, QPoint pt);
		void releaseHighlightFeature();
		QMultiMap<QString, QVector<QString>>* getObjAttributesMapPtr();
		CS57RecFeature* getHighlightFeature() const;
	private:
		CS57FeatureQuery* m_pFeatureQuery;
	};
	class ANALYSIS_EXPORT CS57FeatureQuery
	{
	public:
		CS57FeatureQuery();
		~CS57FeatureQuery();

	public:
		void setProj(CS57Projection* proj);
		void setViewport(CS57Viewport* viewport);
		void setPresLib(CS57PresLib* prelib);
		void setProvider(CS57CellProvider* provider);
		void startHighlight();
		void stopHighlight();
		void closeFeatureQuery();
		void setHighlightRow(int row);
		void queryFeatureInfo(XT::QueryMode mode, QPoint pt);
		void releaseHighlightFeature();
		QMultiMap<QString, QVector<QString>>* getObjAttributesMapPtr();
		CS57RecFeature* getHighlightFeature() const;
	private:
		void queryPointInfo(QPoint pt);
		void queryLineInfo(QPoint pt);
		void queryAreaInfo(QPoint pt);

		void genDisplayContext();

	private:
		bool						m_bHighlight;
		CS57RecFeature*				m_pHighlightFeature;
		CS57Projection*				m_pProjection;
		CS57Viewport*				m_pViewport;
		CS57CooTransHelper			m_CooTransHelper;
		CS57PresLib*				m_pPresLib;
		CS57CellProvider*			m_pProvider;		
		QVector<CS57RecFeature*>	m_vecQueryResult;
		QMultiMap<QString, QVector<QString>> m_mulmapObjAttributes;
	};
}
