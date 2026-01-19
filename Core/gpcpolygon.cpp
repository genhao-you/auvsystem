#include "gpcpolygon.h"
#include "cs57transform.h"

GpcPolygon::GpcPolygon()
{
	m_gpcPolygon.num_contours = 0;	// modify by wb 20211021
	m_gpcPolygon.hole = NULL;
	m_gpcPolygon.contour = NULL;
}

GpcPolygon::GpcPolygon(GpcPolygon& gpcPolygon)
{
	m_gpcPolygon.num_contours = 0;
	m_gpcPolygon.hole = NULL;
	m_gpcPolygon.contour = NULL;
	for (int i = 0; i < gpcPolygon.m_gpcPolygon.num_contours; i++)
	{
		gpc_add_contour(&m_gpcPolygon, &gpcPolygon.m_gpcPolygon.contour[i], gpcPolygon.m_gpcPolygon.hole[i]);
	}
}

GpcPolygon::~GpcPolygon()
{
	freePolygon();
}

void GpcPolygon::createPolygon(CPolylineF &vecPtFs)
{
	gpc_free_polygon(&m_gpcPolygon);
	add_contour(vecPtFs, 0);
}

void GpcPolygon::createPolygon(CPolyline &vecPts)
{
	gpc_free_polygon(&m_gpcPolygon);
	add_contour(vecPts, 0);
}

void GpcPolygon::add_contour(CPolylineF &vecPtFs, int hole)
{
	gpc_vertex_list gpclist;
	gpclist.num_vertices = vecPtFs.size();
	gpclist.vertex = new gpc_vertex[vecPtFs.size()];
	for (int i = 0; i < vecPtFs.size(); i++)
	{
		gpclist.vertex[i].x = vecPtFs[i].x();
		gpclist.vertex[i].y = vecPtFs[i].y();
	}
	gpc_add_contour(&m_gpcPolygon, &gpclist, hole);
	delete[]gpclist.vertex;
	gpclist.vertex = NULL;
}

void GpcPolygon::add_contour(CPolyline &vecPts, int hole)
{
	gpc_vertex_list gpclist;
	gpclist.num_vertices = vecPts.size();
	gpclist.vertex = new gpc_vertex[vecPts.size()];
	for (int i = 0; i < vecPts.size(); i++)
	{
		gpclist.vertex[i].x = vecPts[i].x();
		gpclist.vertex[i].y = vecPts[i].y();
	}
	gpc_add_contour(&m_gpcPolygon, &gpclist, hole);
	delete[]gpclist.vertex;
	gpclist.vertex = NULL;
}

void GpcPolygon::polygon_clip(gpc_op op, GpcPolygon *clip, GpcPolygon *result)
{
	gpc_polygon_clip(op, &m_gpcPolygon, &clip->m_gpcPolygon, &result->m_gpcPolygon);
}

void GpcPolygon::freePolygon()
{
	gpc_free_polygon(&m_gpcPolygon);
}

//************************************
// Method:    toPolygonF
// Brief:	  将gpc多边形转变为经纬度坐标容器（可以处理多种坐标系，但此处只处理经纬度坐标防止混淆）
// Returns:   void
// Author:    cl
// DateTime:  2021/10/26
// Parameter: QVector<QVector<QPointF>*> * pRings
//************************************
void GpcPolygon::toPolygonF(QVector<QVector<QPointF>*>* pRings)
{
	CPolylineF* ring = nullptr;
	for (int i = 0; i < m_gpcPolygon.num_contours; i++)
	{
		ring = new CPolylineF();
		for (int j = 0; j < m_gpcPolygon.contour[i].num_vertices; j++)
		{
			QPointF ptf;
			ptf.setX(m_gpcPolygon.contour[i].vertex[j].x);
			ptf.setY(m_gpcPolygon.contour[i].vertex[j].y);
			ring->push_back(ptf);
		}
		pRings->push_back(ring);
	}
}
