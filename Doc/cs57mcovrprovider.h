#pragma once
#include "doc_global.h"
#include "cs57abstractprovider.h"
#include <QMap>

namespace Doc
{
	class CS57CellMessage;
	class DOC_EXPORT CS57McovrProvider : public CS57AbstractProvider
	{
	public:
		CS57McovrProvider();
		~CS57McovrProvider();

		//添加Msg
		void addMsg(CS57CellMessage* pCellMsg);
		//写XTM文件
		void writeXTM(const QString& strXtmPath);
		//读取XTM文件
		void readXTM(const QString& strXtmPath);
		//选图
		void selectChart(CSpatialPolygon* pPolygon, double scale);
		//释放Mcovr
		void releaseMcovr();
		//获取Mcovr多边形
		QMap<CS57CellMessage*, CSpatialPolygon*> getMcovrPolygons();
	public:
		QVector<CS57CellMessage*> m_vecSourceMcovr;
		QVector<CS57CellMessage*> m_vecRenderMcovr;
	};
}