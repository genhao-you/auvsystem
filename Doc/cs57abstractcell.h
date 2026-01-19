#pragma once
#include "doc_global.h"
#include "geometry.h"
#include "cs57recdsgi.h"
#include "cs57recdsgr.h"
#include <QVector>

namespace Doc
{
	class CS57RecFeature;
	class CS57RecVector;
	class DOC_EXPORT CS57AbstractCell
	{
	public:
		CS57AbstractCell();
		virtual ~CS57AbstractCell();

	public:
		//获取图幅名称
		QString getCellName() const;
		//获取图幅名称不带扩展名
		QString getCellNameWithoutSuffix() const;
		//获取图幅更新信息
		QString getCellEdtn() const;
		//获取图幅更新号
		QString getCellUpdn() const;
		//获取图幅编辑比例尺
		double getCellCscl() const;
		//获取图幅乘数因子
		double getCellComf() const;
		//获取测深放大系数
		double getCellSomf() const;
		//获取要素数
		int getFeatureSize() const;
		//生成线要素集合
		void genFeaturePolyline(CS57RecFeature* pFE, QVector<QVector<QPointF>*>* pPolylines);
		//生成面要素集合
		void genFeaturePolygon(CS57RecFeature* pFE, QVector<QVector<QPointF>*>* pRings);
	public:
		CS57RecDsgi* m_pRecDsgi;
		CS57RecDsgr* m_pRecDsgr;

		// S57特征记录指针集合
		QVector<CS57RecFeature*> m_vecS57RecFE;
		// S57矢量记录(链接节点)指针集合
		QVector<CS57RecVector*> m_vecS57RecVC;
		// S57矢量记录(边)指针集合
		QVector<CS57RecVector*> m_vecS57RecVE;

		double m_dLeft, m_dRight, m_dTop, m_dBottom;
	};
}
