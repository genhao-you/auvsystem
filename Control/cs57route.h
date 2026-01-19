#pragma once

#include "cs57polyline.h"
#include "gpcpolygon.h"
extern "C" {
#include "gpc.h"
}

class CS57Route : public CS57Polyline
{
	Q_OBJECT
public:
	CS57Route(CS57Control* control,const QString& name = "");
	~CS57Route();
	int	 ptCount() const;
	void setBufferDis(double dis);//µ•≤‡£¨µ•Œªm
	void setBufferEnabled(bool enable);
	void copy(CS57Route* route);
	void calcBoundingBox();
	void buffer2Polygon(QVector<QVector<QPointF>*>* vecPtfs);
private:
	void		updateBuffer();
	void		makeBuffer();
	void		makeGpcPolygon();
	CPolylineF	toPointFs(QString coords);
	CPolyline	toPoints(QString coords);
	void		renderBufferPolygon(QPainter* painter, GpcPolygon* polygon);

protected:
	void paint(QPainter *p) override;
private:
	double	m_dBufferDisM;//ª∫¥Êæ‡¿Îm
	int		m_nBufferDisPix;//ª∫≥Âæ‡¿ÎœÒÀÿ
	bool		m_bEnableBuffer;
	CPolylineF	m_vecBuffer;
	CPolyline	m_vecBufferPix;
	GpcPolygon* m_pResultGpcPolygon;
	QVector<GpcPolygon*> m_vecGpcPolygon;
};
