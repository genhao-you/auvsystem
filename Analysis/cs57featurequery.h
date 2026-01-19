#pragma once
#include "xt_global.h"
#include "analysis_global.h"
#include "cs57recfeature.h"
#include "cs57transform.h"
#include <QPoint>

using namespace Doc;
namespace Analysis
{
	class CS57FeatureQuery;
	class ANALYSIS_EXPORT CS57FeatureQuery
	{
	public:
		CS57FeatureQuery();
		~CS57FeatureQuery();

	public:
		//设置Doc类指针
		void setDoc(CS57DocManager* doc);
		//设置表达库指针
		void setPresLib(CS57PresLib* prelib);
		//开启高亮
		void startHighlight();
		//停止高亮
		void stopHighlight();
		//关闭要素查询
		void closeFeatureQuery();
		//设置高亮行数
		void setHighlightRow(int row);
		void queryMouse(XT::QueryMode mode, QPoint pt);
		//释放高亮要素
		void releaseHighlightFeature();
		QMultiMap<QString, QVector<QString>>* getObjAttributesMapPtr();
		CS57RecFeature* getHighlightFeature() const;
	private:
		//查询点物标信息
		void queryPointInfo(QPoint pt);
		//查询线物标信息
		void queryLineInfo(QPoint pt);
		//查询面物标信息
		void queryAreaInfo(QPoint pt);

		//生成属性显示列表
		void genDisplayContext();

	private:
		bool						m_bHighlight;
		CS57DocManager*				m_pDoc;
		CS57RecFeature*				m_pHighlightFeature;
		CS57Transform*				m_pTransform;
		CS57PresLib*				m_pPresLib;	
		QMap<int, CS57RecFeature*>   m_mapQueryResult;
		QMultiMap<QString, QVector<QString>> m_mulmapObjAttributes;
	};
}
