#pragma once
#include "doc_global.h"
#include "geometry.h"
#include "cs57abstractprovider.h"
#include <QString>
#include <QVector>
#include <QPoint>

namespace Doc
{
	class DOC_EXPORT CS57GlobalMapProvider : public CS57AbstractProvider
	{
	public:
		CS57GlobalMapProvider();
		~CS57GlobalMapProvider();
	public:
		//读取二进制文件
		void readBinFile();
		//读取文件
		void readFile();
		//选图
		void selectChart(CSpatialPolygon* pPolygon, double scale);

		void releaseRenderPts();
	private:
		QString						m_strGlobalPath;
		QVector<QVector<QPointF>*>	m_vecSourcePts;
	public:
		//第一层：环数
		//第二层：环点数
		//第三层：环点坐标
		QVector<QVector<QPointF>*>	m_vecRenderPts;
	};
}
