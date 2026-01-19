#pragma once
#include "core_global.h"
#include "cs57symbol.h"
#include "cs57transform.h"
#include "cs57preslib.h"
#include "cs57globalmapprovider.h"
#include "cs57mcovrprovider.h"
#include "cs57cellprovider.h"
#include "cs57mcovrregionprovider.h"
#include <QPainter>

using namespace Doc;
namespace Core
{
	class CORE_EXPORT CS57AbstractRenderer
	{
	public:
		CS57AbstractRenderer();
		virtual ~CS57AbstractRenderer();		//多态注意virtual
	public:
		void setPresLib(CS57PresLib* prelib);
		virtual void doRender(QPainter* p) = 0;
	public:
		QPainter*			m_pPainter;
		CS57PresLib*		m_pPresLib;
		CS57Transform*		m_pTransform;
	};

	class CORE_EXPORT CS57GlobalMapRenderer : public CS57AbstractRenderer
	{
	public:
		CS57GlobalMapRenderer();
		~CS57GlobalMapRenderer();

	public:
		void setVisible(bool visible);

		void setLineWidth(int w);
		int getLineWidth() const;

		void setLineColor(const QColor& color);
		QColor getLineColor() const;

		void setFillColor(const QColor& color);
		QColor getFillColor() const;

		void setDataSource(CS57GlobalMapProvider* pGlobalMapProvider);

		void doRender(QPainter* p);
	private:
		int		m_nLineWidth;
		QColor	m_LineColor;
		QColor	m_FillColor;
		bool	m_bVisible;
		CS57GlobalMapProvider* m_pProvider;
	};
	class CORE_EXPORT CS57McovrRenderer : public CS57AbstractRenderer
	{
	public:
		CS57McovrRenderer();
		~CS57McovrRenderer();
	public:
		void setVisible(bool visible);

		void setLineWidth(int w);
		int getLineWidth() const;

		void setLineColor(const QColor& color);
		QColor getLineColor() const;

		void setFillColor(const QColor& color);
		QColor getFillColor() const;

		void setDataSource(CS57McovrProvider* pMcovrProvider);

		void doRender(QPainter* p);
	private:
		int		m_nLineWidth;
		QColor	m_LineColor;
		QColor	m_FillColor;
		bool	m_bVisible;
		CS57McovrProvider* m_pProvider;
	};

	class CORE_EXPORT CS57CellRenderer : public CS57AbstractRenderer
	{
	public:
		CS57CellRenderer();
		~CS57CellRenderer();
	private:
		struct sCompassInfo
		{
			QPoint centerPt;//中心点像素坐标
			double  dCscl;//图幅编辑比例尺
			QString strMagnetic;//磁差
			QString strAnnual;//年变率
			QString strYear;//年份
		};
		enum RenderType
		{
			RENDER_DISPLAY,	//显示渲染
			RENDER_PRINT	//打印渲染
		};
	public:
		void setSymbolScaleMode(XT::SymbolScaleMode mode);
		float getSymbolScaleFactor(CS57Cell* pCell, double displayScale);
		void setBorderVisible(bool visible);
		void setRenderType(XT::RenderType type);
		void setSymbolMode(QString strSymbolMode);
		void setDataSource(CS57CellProvider* pCellProvider);
		void setPrintParameters(XT::sExportParameters* pPara);
		void doRender(QPainter* p);
	private:
		void renderArea(CellDisplayContext* pContext);
		void renderLine(CellDisplayContext* pContext);
		void renderPoint(CellDisplayContext* pContext);
		void renderAnnotation(CellDisplayContext* pContext);
		void renderBorder(CellDisplayContext* pContext);

		void renderPointAnnotation(CellDisplayContext* pContext);
		void renderLineAnnotation(CellDisplayContext* pContext);
		void renderAreaAnnotation(CellDisplayContext* pContext);

		void renderCompass(sCompassInfo& compassInfo);
		void renderSounding(QPainter* painter,			// 绘图指针 
							QPoint& point,				// 水深点位 画布单位
							float scale,				// 比例
							float rotation,				// 旋转角度，单位为度
							float z,					// 水深值，单位为米，负值为干出水深
							bool shallow,				// 是否在浅水区
							bool swept);				// 是否为扫海水深
	private:
		bool				m_bBorderVisible;
		QString				m_strPSymbolMode;
		QString				m_strLSymbolMode;
		QString				m_strASymbolMode;
		CS57CellProvider*	m_pProvider;
		XT::RenderType			m_eRenderType;
		XT::SymbolScaleMode m_eCurSymbolScaleMode;
		XT::sExportParameters* m_pPrintParameters;

	};
	class CORE_EXPORT CS57McovrRegionRenderer : public CS57AbstractRenderer
	{
	public:
		CS57McovrRegionRenderer();
		~CS57McovrRegionRenderer();
		enum McovrRegion
		{
			Region_I,
			Region_II,
			Region_III,
			Region_IV,
			Region_V
		};
	public:
		void setVisible(McovrRegion r,bool visible);

		void setLineWidth(McovrRegion r, int w);
		int getLineWidth(McovrRegion r) const;

		void setLineColor(McovrRegion r, const QColor& color);
		QColor getLineColor(McovrRegion r) const;

		void setFillColor(McovrRegion r, const QColor& color);
		QColor getFillColor(McovrRegion r) const;

		void setDataSource(CS57McovrRegionProvider* pMcovrRegionProvider);

		void doRender(QPainter* p);
	private:
		void renderMcovrRegion(McovrRegion r,GpcPolygon* polygon);
	private:
		int		m_nLineWidthI;
		QColor	m_LineColorI;
		QColor	m_FillColorI;
		bool	m_bVisibleI;

		int		m_nLineWidthII;
		QColor	m_LineColorII;
		QColor	m_FillColorII;
		bool	m_bVisibleII;

		int		m_nLineWidthIII;
		QColor	m_LineColorIII;
		QColor	m_FillColorIII;
		bool	m_bVisibleIII;

		int		m_nLineWidthIV;
		QColor	m_LineColorIV;
		QColor	m_FillColorIV;
		bool	m_bVisibleIV;

		int		m_nLineWidthV;
		QColor	m_LineColorV;
		QColor	m_FillColorV;
		bool	m_bVisibleV;
		CS57McovrRegionProvider* m_pProvider;
	};

	class CORE_EXPORT CS57CellBorderRenderer : public CS57AbstractRenderer
	{
	public:
		CS57CellBorderRenderer(CS57Cell* pCell);
		CS57CellBorderRenderer(XT::sExportParameters* pPara);
		~CS57CellBorderRenderer();
		struct sChartInfo
		{
			QString	chartName;				// 图名
			QString chartNum;				// 图号
			double minLat;					// 南图廓纬度　 （单位：弧度）
			double maxLat;					// 北图廓纬度   （单位：弧度）
			double minLon;					// 西图廓经度   （单位：弧度）
			double maxLon;					// 东图廓经度   （单位：弧度）
			double csclScale;				// 编辑比例尺   
			QString projName;				// 投影名称
			double baseLat;					// 基准纬线　	（单位：弧度）
			double midLon;					// 中央经线　	（单位：弧度）
		};
		struct sBorderInfo
		{
			double minProjCoorX;			// 矩形内图廓投影水平坐标最小值
			double minProjCoorY;			// 矩形内图廓投影垂直坐标最小值		
			double maxProjCoorX;			// 矩形内图廓投影水平坐标最大值
			double maxProjCoorY;			// 矩形内图廓投影垂直坐标最大值
			// 图廓细分参数
			int maxSubInterval;			// 最大细分间隔 (单位：弧度)
			int largerSubInterval;		// 较大细分间隔 (单位：弧度)
			int smallSubInterval;		// 较小细分间隔 (单位：弧度)
			int minSubInterval;			// 最小细分间隔 (单位：弧度)
			int blackWhiteSectionInterval;	// 黑白节长度   (单位：弧度)
			int lableIntercal;			// 标注间隔		(单位：弧度)
			int lineInterval;			// 经纬线间隔   (单位：弧度)

			double lenin2Outline;			// 外图廓(中线)与内图廓距离
			double lenin2labe;				// 
			double lenMaxSub;				// 最大细分长度 (单位：mm)
			double lenLargerSub;			// 较大细分长度 (单位：mm)
			double lenSmallSub;				// 较小细分长度 (单位：mm)
			double lenMinSub;				// 最小细分长度 (单位：mm)
			double widthBlackWhiteSection;	// 黑白节宽度   (单位：mm)
			double widthOutLine = 0.8;		// 外图廓宽度 (单位：mm)

			int KmMaxInterval;
			int KmMinInterval;
			int KmMarkInterval;

			double wKmMax;
			double wKmMid;
			double wKmMin;
		};
	public:
		void setVisible(bool visible);

		void doRender(QPainter* p);
	private:
		void init();
		void getBorderInnerSize(double * mmleve, double *vertical);
		void splitDMMSS(double DMMSS, QString &Degree, QString &Minute, QString &second);
		//void drawLatLine(QPainter* painter, const double &lat, double &lonStart, double &lonEnd);
		//void drawLonLine(QPainter* painter, const double &lon, double &latStart, double &latEnd);
		////************************************************************		////  src(QPointF pts[3], int &idStatus, double &distance)                    		////               直代曲计算-- - 子程序                        		////  转入参数 : pts 表示三个坐标点，点坐标单位mm       		////  转出参数 : idStatus 状态码,0 三点为直线，１合适直距，2 不合适的直距 3,第一点和第三点重合		////			 : distance 代曲直距　正确的直距                          		////                     王    斌                               		////                    1990 - 12 - 15                          		////
		////************************************************************
		//bool src(const QPointF pts[3], int &idStatus, double &distance);
		//bool src(const QPointF pts[3]);
		//bool calcuInnerProfiler();

		void renderBorder(QPainter* painter);

	private:
		void preCalcBorder();
		void renderAnno(QPainter * &painter, QFont &font, double x, double y, float ang, QString str);
		bool calcInnerCoors();;
	private:
		sChartInfo	m_ChartInfo;
		sBorderInfo	m_BorderInfo;
		CS57Cell*	m_pCell;
		bool		m_bVisible;

		QPointF PointLB, PointLT, pointRT, PointRB;  // 内图廓四角点投影坐标
	};
}
