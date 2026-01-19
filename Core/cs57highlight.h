#pragma once
#include "core_global.h"
#include "cs57renderer.h"
namespace Core
{
	class CORE_EXPORT CS57FeatureHighlight : public CS57AbstractRenderer
	{
	public:
		CS57FeatureHighlight();
		~CS57FeatureHighlight();
	public:
		void	setLineWidth(int w);
		int		getLineWidth() const;
		void	setLineColor(QColor color);
		QColor	getLineColor() const;
		void	setFillColor(QColor color);
		QColor	getFillColor() const;
		void	setOperateMode(XT::OperateMode mode);
		void	setQueryMode(XT::QueryMode mode);
		void	setDataSource(CS57RecFeature* pFeature);
		void	doRender(QPainter* p);
	private:
		void	pointHighlight(CS57RecFeature* pFeature);
		void	lineHighlight(CS57RecFeature* pFeature);
		void	areaHighlight(CS57RecFeature* pFeature);

	private:
		CS57RecFeature* m_pHighlightFeature;
		XT::OperateMode m_eCurOperateMode;
		XT::QueryMode   m_eCurQueryMode;
		int				m_nLineWidth;
		QColor			m_LineColor;
		QColor			m_FillColor;
	};
}

