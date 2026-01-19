#pragma once
#include "doc_global.h"
#include "cs57recdsgi.h"
#include "cs57recdsgr.h"
#include <QList>

namespace Doc
{
	class CS57RecFeature;
	class CS57RecVector;
	class DOC_EXPORT CS57CellBase
	{
	public:
		CS57CellBase();
		~CS57CellBase();

	public:
		//获取图幅名称
		QString getCellName() const;
		//获取图幅更新信息
		QString getCellEdtn() const;
		QString getCellUpdn() const;
		//获取图幅编辑比例尺
		double getCellCscl() const;
		//获取图幅乘数因子
		double getCellComf() const;

	public:
		CS57RecDsgi* m_pRecDsgi;
		CS57RecDsgr* m_pRecDsgr;

		// S57特征记录指针集合
		QList<CS57RecFeature*> m_lstS57RecFE;
		// S57矢量记录(链接节点)指针集合
		QList<CS57RecVector*> m_lstS57RecVC;
		// S57矢量记录(边)指针集合
		QList<CS57RecVector*> m_lstS57RecVE;
	};
}
