#pragma once
#include "core_global.h"
#include "geometry.h"
#include "cs57preslib.h"
#include <QPainter>

using namespace Doc;
namespace Core
{
	class CORE_EXPORT CS57AbstractSymbol
	{
	public:
		CS57AbstractSymbol();
		~CS57AbstractSymbol();

	public:
		CS57PresLib* m_pPresLib;
	};

	class CORE_EXPORT CS57PointSymbol : public CS57AbstractSymbol
	{
	public:
		CS57PointSymbol(CS57PresLib* preslib);
		~CS57PointSymbol();

		void renderSY(QPainter* painter, QPoint& pt, const QString& symbName, float scale, float rotation);
		void renderCA(QPainter* painter, 
						QPoint& pt,
						float sectr1,
						float sectr2,
						float arc_radius,
						float sector_radius,
						unsigned short outline_width,
						unsigned short arc_width,
						char arc_color[6],
						char outline_color[6]);
		void renderSounding(QPainter* painter, QPoint& pt,const QString& symbName, float scale, float rotation);

	};
	class CORE_EXPORT CS57LineSymbol : public CS57AbstractSymbol
	{
	public:
		CS57LineSymbol(CS57PresLib* preslib);
		~CS57LineSymbol();

		void renderLS(QPainter* pPainter, CPolyline& pts, float scale, int lineWidth, QString lineStyle, QString lineColor);
		void renderLC(QPainter* pPainter, CPolyline& pts, float scale, const QString& lnstName);

	private:
		void renderDotLine(QPainter* pPainter, CPolyline& pts, float scale, QString lineColor);
		void renderLnst(QPainter* pPainter, QPoint& posPoint, LNST* pLnst, float scale, float rotation);
		void renderSymbol(QPainter* pPainter, SYMB* pSymbol, float rotation);
		//判断点在线的左边还是右边 返回值 >0 左侧  < 0 右侧  = 0 线段上
		int  LocOfPointAndLine(QPointF p1, QPointF p2, QPointF p0);
		int  LocOfPointAndLine(QPoint p1, QPoint p2, QPoint p0);
		void setClipPath(float preCross,		// 当前线段与上一条线段叉积
						 float nextCross,	// 当前线段与下一条线段叉积
						 float preAng,		// 上一条线段方向角
						 float curAng,		// 当前线段方向角		
						 float nextAng,		// 下一条线段方向角
						 float radius,		// 宽度
						 QPointF points[2],	// 当前线段
						 QPainterPath& path);
	private:
		float			m_fPreAng;		// 上一条线段方向角
		float			m_fCurAng;		// 当前线段方向角		
		float			m_fNextAng;		// 下一条线段方向角
		QPointF			m_StartPt;
		QPointF			m_EndPt;
		QPainterPath	m_ClipPath;
	};
	class CORE_EXPORT CS57AreaSymbol : public CS57AbstractSymbol
	{
	public:
		CS57AreaSymbol(CS57PresLib* preslib);
		~CS57AreaSymbol();

		void renderAC(QPainter* pPainter, QVector<QPolygon>& polygons, unsigned short transparent, QString areaColor);
		void renderAP(QPainter* pPainter, QVector<QPolygon>& polygons, QString patName, float scale, float rotation);
		void renderLS(QPainter* pPainter, CPolyline& pts, float scale, int lineWidth, QString lineStyle, QString lineColor);
		void renderLC(QPainter* pPainter, CPolyline& pts, float scale, const QString& lnstName);

	private:
		void initPatt(QPainter* pPainter, QString pattName, QPoint& posPoint, float scale, float rotation);
		void initPatt(float dpiX,float dpiY,QString pattName, QPoint& posPoint, float scale, float rotation);
		void renderPatt(QPainter* pPainter,float scale, float rotation);
		QPixmap* getPattPixmap(QPainter* pPainter, QString pattName, QPoint& piovtPt, float scale, float rotation);

	private:
		CS57LineSymbol* m_pLineSymbol;
		QPoint m_CurPoint;
		float  m_fCurPatScale;
		float  m_fPainterScaleX;
		float  m_fPainterScaleY;
		QPoint m_patPiovt;	// 符号的定位点
		PATT*  m_pCurPatt;
	};
}
