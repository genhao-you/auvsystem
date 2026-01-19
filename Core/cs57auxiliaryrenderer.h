#pragma once
#include "core_global.h"
#include "cs57renderer.h"

namespace Core
{
	/********************************************************************
		created:	2021/07/13
		author:		Chen.Luan
		class:		CS57AuxiliaryRenderer
		purpose:	辅助渲染类
	*********************************************************************/
	class CORE_EXPORT CS57AuxiliaryRenderer : public CS57AbstractRenderer
	{
	public:
		CS57AuxiliaryRenderer();
		~CS57AuxiliaryRenderer();
	public:
		//渲染任意坐标点的十字线
		void renderCrossLine(QPainter* painter, QPoint pt);
		void renderCrossLine(QPainter* painter, QPoint pt,QColor color,int penWidth);
	public:
		//启用中心十字线
		void setEnabled(bool enable = false);
		void setCenterPt(QPoint pt);
		void doRender(QPainter* p);
	private:
		//是否开启中心十字线
		bool	m_bEnabled;
		//中心坐标点
		QPoint	m_CenterPt;
	};
}
