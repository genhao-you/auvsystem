#pragma once
#include "analysis_global.h"
#include "cs57docmanager.h"
#include "cs57projection.h"
#include "cs57viewport.h"
#include "cs57transform.h"
#include <QLine>

using namespace Doc;
namespace Analysis
{
	class CS57ChartSelection
	{
	public:
		CS57ChartSelection();
		~CS57ChartSelection();
		struct sCellRecord
		{
			QString strCellNum;//图号
			QString strCellName;//图名
			int		nCscl;//比例尺分母
			QString strDate;//出版日期
		};
		void setDoc(CS57DocManager* doc);

		//排序结果
		QVector<sCellRecord> sortResults();

		CS57DocManager*		m_pDoc;
		CS57Transform*		m_pTransform;
		QVector<CS57CellMessage*>	m_vecSelectedMcovrs;
	};
}
