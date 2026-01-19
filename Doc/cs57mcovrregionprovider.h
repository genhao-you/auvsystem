#pragma once
#include "doc_global.h"
#include "gpcpolygon.h"
#include <QVector>

namespace Doc
{
	class CS57CellMessage;
	//信息区域划分
	class DOC_EXPORT CS57McovrRegionProvider
	{
	public:
		CS57McovrRegionProvider();
		~CS57McovrRegionProvider();

		//区域划分
		bool startDivide(QVector<CS57CellMessage*> pSourceMessage);
		//关闭区域拆分
		void closeDivide();

	private:
		//释放内存
		void release();
		//清理容器
		void clear();
		//区域融合处理
		void unionRegion();
		//融合区域I
		void unionRegionI();
		//融合区域II
		void unionRegionII();
		//融合区域III
		void unionRegionIII();
		//融合区域IV
		void unionRegionIV();
		//融合区域V
		void unionRegionV();

		//挖孔V区域中的IV区域
		void diffRegionV();
		//挖孔IV区域的III区域
		void diffRegionIV();
		//挖孔III区域的II区域
		void diffRegionIII();
		//挖孔II区域的I区域
		void diffRegionII();
		//挖孔I区域
		void diffRegionI();
		
		//转换结果多边形I为空间多边形I
		void toSpatialPolygonI();
		//转换结果多边形II为空间多边形II
		void toSpatialPolygonII();
		//转换结果多边形III为空间多边形III
		void toSpatialPolygonIII();
		//转换结果多边形IV为空间多边形IV
		void toSpatialPolygonIV();
		//转换结果多边形V为空间多边形V
		void toSpatialPolygonV();

	public:
		GpcPolygon* m_pResultGpcPolygonI;
		GpcPolygon* m_pResultGpcPolygonII;
		GpcPolygon* m_pResultGpcPolygonIII;
		GpcPolygon* m_pResultGpcPolygonIV;
		GpcPolygon* m_pResultGpcPolygonV;

		CSpatialPolygon* m_pResultSpatialPolygonI;
		CSpatialPolygon* m_pResultSpatialPolygonII;
		CSpatialPolygon* m_pResultSpatialPolygonIII;
		CSpatialPolygon* m_pResultSpatialPolygonIV;
		CSpatialPolygon* m_pResultSpatialPolygonV;

	private:
		QString m_strEncDir;
		QVector<CS57CellMessage*> m_vecCellMessageI;
		QVector<CS57CellMessage*> m_vecCellMessageII;
		QVector<CS57CellMessage*> m_vecCellMessageIII;
		QVector<CS57CellMessage*> m_vecCellMessageIV;
		QVector<CS57CellMessage*> m_vecCellMessageV;//第三位是1
	};
}

