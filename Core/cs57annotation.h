#pragma once
#include "core_global.h"
#include "cs57docmanager.h"
#include <QPainter>

using namespace Doc;
namespace Core
{
	class CORE_EXPORT CS57AbstractAnnotation
	{
	public:
		CS57AbstractAnnotation();
		~CS57AbstractAnnotation();

		virtual void setTE(showTextTE* pTE) = 0;
		virtual void setTX(showTextTX* pTX) = 0;

	public:
		CS57PresLib* m_pPresLib;
	};
	class CORE_EXPORT CS57PointAnnotation : public CS57AbstractAnnotation
	{
	public:
		CS57PointAnnotation(CS57PresLib* preslib);
		~CS57PointAnnotation();

		void setAnnoFeature(CS57RecFeature* pFE);
		void setTE(showTextTE* pTE);
		void setTX(showTextTX* pTX);
		void renderTE(QPainter* painter, QPoint& pt, float scale);
		void renderTX(QPainter* painter, QPoint& pt, float scale);

	private:
		CS57RecFeature* m_pFeature;
		showTextTE* m_pTE;
		showTextTX* m_pTX;
	};
	class CORE_EXPORT CS57LineAnnotation : public CS57AbstractAnnotation
	{
	public:
		CS57LineAnnotation(CS57PresLib* preslib);
		~CS57LineAnnotation();

		void setAnnoFeature(CS57RecFeature* pFE);
		void setTE(showTextTE* pTE);
		void setTX(showTextTX* pTX);
		void renderTE(QPainter* painter, CPolylines& polylines, QString strAnno, float angle, float scale);
		void renderTX(QPainter* painter, CPolylines& polylines, QString strAnno, float angle, float scale);
	private:
		void calcLinePts(CPolylines& polylines,QPoint& pt1,QPoint&pt2);
	private:
		CS57RecFeature* m_pFeature;
		showTextTE* m_pTE;
		showTextTX* m_pTX;
	};
	class CORE_EXPORT CS57AreaAnnotation : public CS57AbstractAnnotation
	{
	public:
		CS57AreaAnnotation(CS57PresLib* preslib);
		~CS57AreaAnnotation();

		void setTE(showTextTE* pTE);
		void setTX(showTextTX* pTX);
		void renderTE(QPainter* painter, CPolylines& polygons, unsigned short objl, QString strAnno, float angle, float scale);
		void renderTX(QPainter* painter, CPolylines& polygons, unsigned short objl, QString strAnno, float angle, float scale);
	private:
		QPoint getCentriodOfPolygon(CPolylines& polygons);

		showTextTE* m_pTE;
		showTextTX* m_pTX;
	};
}

