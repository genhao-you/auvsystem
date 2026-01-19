#include "stdafx.h"
#include "cs57cellmessage.h"
#include "cs57cell.h"
#include "cs57recfeature.h"
#include "cs57recvector.h"
#include <QMessageBox>

using namespace Doc;
#pragma execution_character_set("utf-8")
CS57CellMessage::CS57CellMessage()
{}
CS57CellMessage::CS57CellMessage(CS57Cell* pCell)
{
	// 初始化DSGI，DSGR
	setDsgi(pCell->m_pRecDsgi);
	setDsgr(pCell->m_pRecDsgr);

	// 遍历特征物标，找到M_COVER
	for (int i = 0; i < pCell->m_vecS57RecFE.size(); i++)
	{
		CS57RecFeature* pRecFeature = pCell->m_vecS57RecFE[i];
		if (pRecFeature->m_Frid.objl == 302 && 
			pRecFeature->m_Attf.m_vecAttf[0].atvl == "1")
		{
			addRecFE(pCell->m_vecS57RecFE[i]);						// 增加元物标
		}
	}

	// 遍历边空间物标，找到与M_COVER相关的
	for (int i = 0; i < pCell->m_vecS57RecVE.size(); i++)
	{
		for (int j = 0; j < m_vecS57RecFE.size(); j++)
		{
			for (int k = 0;k < m_vecS57RecFE[j]->m_Fspt.m_vecFspt.size();k++)
			{
				if (i == m_vecS57RecFE[j]->m_Fspt.m_vecFspt[k].idx)
				{
					addRecVE(pCell->m_vecS57RecVE[i]);
				}
			}
		}
	}

	// 遍历链接节点，找到与边相关的
	for (int i = 0; i < pCell->m_vecS57RecVC.size(); i++)
	{
		for (int j = 0; j < m_vecS57RecVE.size(); j++)
		{
			for (int k = 0; k < m_vecS57RecVE[j]->m_Vrpt.m_vecVrpt.size(); k++)
			{
				if (i == m_vecS57RecVE[j]->m_Vrpt.m_vecVrpt[k].idx)
				{
					addRecVC(pCell->m_vecS57RecVC[i]);
				}
			}
		}
	}
	index();
}


CS57CellMessage::~CS57CellMessage()
{
	release();
}
//************************************
// Method:    setDsgi
// Brief:	  设置Dsgi
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CS57RecDsgi * pDsgi
//************************************
void CS57CellMessage::setDsgi(CS57RecDsgi* pDsgi)
{
	m_pRecDsgi = new CS57RecDsgi();
	m_pRecDsgi->copy(pDsgi);
}
//************************************
// Method:    setDsgr
// Brief:	  设置Dsgr
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CS57RecDsgr * pDsgr
//************************************
void CS57CellMessage::setDsgr(CS57RecDsgr* pDsgr)
{
	m_pRecDsgr = new CS57RecDsgr();
	m_pRecDsgr->copy(pDsgr);
}

//************************************
// Method:    addRecFE
// Brief:	  添加FE记录
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CS57RecFeature * pRecFE
//************************************
void CS57CellMessage::addRecFE(CS57RecFeature* pRecFE)
{
	CS57RecFeature* recFE = new CS57RecFeature();
	recFE->copy(pRecFE);
	m_vecS57RecFE.push_back(recFE);
}

//************************************
// Method:    addRecVE
// Brief:	  添加VE记录
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CS57RecVector * pRecVE
//************************************
void CS57CellMessage::addRecVE(CS57RecVector* pRecVE)
{
	for (int i = 0; i < m_vecS57RecVE.size(); i++)
	{
		if (pRecVE->m_Vrid.rcid == m_vecS57RecVE[i]->m_Vrid.rcid)
			return;
	}
	CS57RecVector* recVE = new CS57RecVector();
	recVE->copy(pRecVE);
	m_vecS57RecVE.push_back(recVE);
}

//************************************
// Method:    addRecVC
// Brief:	  添加VC记录
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CS57RecVector * pRecVC
//************************************
void CS57CellMessage::addRecVC(CS57RecVector* pRecVC)
{
	for (int i = 0; i < m_vecS57RecVC.size(); i++)
	{
		if (pRecVC->m_Vrid.rcid == m_vecS57RecVC[i]->m_Vrid.rcid)
			return;
	}
	CS57RecVector* recVC = new CS57RecVector();
	recVC->copy(pRecVC);
	m_vecS57RecVC.push_back(recVC);
}

//************************************
// Method:    ioXTMFile
// Brief:	  读写XTM文件
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: FILE * fp
// Parameter: bool ioFlag
//************************************
void CS57CellMessage::ioXTMFile(FILE* fp, bool ioFlag)
{
	int size;
	if (ioFlag)//读
	{
		m_pRecDsgi = new CS57RecDsgi();
		m_pRecDsgi->ioFile(fp, ioFlag);

		m_pRecDsgr = new CS57RecDsgr();
		m_pRecDsgr->ioFile(fp, ioFlag);

		size = m_vecS57RecFE.size();
		fread(&size, sizeof(int), 1, fp);
		for (int i = 0; i < size; i++)
		{
			CS57RecFeature* pRecFeature = new CS57RecFeature();
			pRecFeature->ioFile(fp, ioFlag);
			m_vecS57RecFE.push_back(pRecFeature);
		}
		size = m_vecS57RecVC.size();
		fread(&size, sizeof(int), 1, fp);
		for (int i = 0; i < size; i++)
		{
			CS57RecVector* pRecVector = new CS57RecVector();
			pRecVector->ioFile(fp, ioFlag);
			m_vecS57RecVC.push_back(pRecVector);
		}
		size = m_vecS57RecVE.size();
		fread(&size, sizeof(int), 1, fp);
		for (int i = 0; i < size; i++)
		{
			CS57RecVector* pRecVector = new CS57RecVector();
			pRecVector->ioFile(fp, ioFlag);
			m_vecS57RecVE.push_back(pRecVector);
		}
	}
	else//写
	{
		m_pRecDsgi->ioFile(fp, ioFlag);
		m_pRecDsgr->ioFile(fp, ioFlag);

		size = m_vecS57RecFE.size();
		fwrite(&size, sizeof(int), 1, fp);
		for (int i = 0; i < size; i++)
		{
			m_vecS57RecFE[i]->ioFile(fp, ioFlag);
		}

		size = m_vecS57RecVC.size();
		fwrite(&size, sizeof(int), 1, fp);
		for (int i = 0; i < size; i++)
		{
			m_vecS57RecVC[i]->ioFile(fp, ioFlag);
		}

		size = m_vecS57RecVE.size();
		fwrite(&size, sizeof(int), 1, fp);
		for (int i = 0; i < size; i++)
		{
			m_vecS57RecVE[i]->ioFile(fp, ioFlag);
		}
	}
}

//************************************
// Method:    index
// Brief:	  拓扑索引
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57CellMessage::index()
{
	// 建立边与链接节点的关系
	for (int i = 0; i < m_vecS57RecVE.size(); i++)
	{
		for (int j = 0; j < m_vecS57RecVE[i]->m_Vrpt.m_vecVrpt.size(); j++)
		{
			bool flag = false;
			for (int k = 0; k < m_vecS57RecVC.size(); k++)
			{
				if (m_vecS57RecVE[i]->m_Vrpt.m_vecVrpt[j].rcid == m_vecS57RecVC[k]->m_Vrid.rcid)
				{
					m_vecS57RecVE[i]->m_Vrpt.m_vecVrpt[j].idx = k;
					flag = true;
				}
			}
			if (!flag)
			{
				QMessageBox::information(nullptr, "提示", "边与链接节点拓扑关系错误!");
				return;
			}
		}
	}

	// 建立特征物标与边的关系
	for (int i = 0; i < m_vecS57RecFE.size(); i++)
	{
		for (int j = 0; j < m_vecS57RecFE[i]->m_Fspt.m_vecFspt.size(); j++)
		{
			bool flag = false;
			for (int k = 0; k < m_vecS57RecVE.size(); k++)
			{
				if (m_vecS57RecFE[i]->m_Fspt.m_vecFspt[j].rcid == m_vecS57RecVE[k]->m_Vrid.rcid)
				{
					m_vecS57RecFE[i]->m_Fspt.m_vecFspt[j].idx = k;
					flag = true;
				}
			}
			if (!flag)
			{
				QMessageBox::information(nullptr, "提示", "要素与边拓扑关系错!");
				return;
			}
		}
	}
}

//************************************
// Method:    toSpatialPolygon
// Brief:	  转空间多边形
// Returns:   CSpatialPolygon*
// Author:    cl
// DateTime:  2022/07/21
//************************************
CSpatialPolygon* CS57CellMessage::toSpatialPolygon()
{
	if (m_vecS57RecFE.size() == 0)
		return nullptr;

	//正常情况Msg中只有一个要素
	m_SpatialPolygon.relPolygon();
	CS57RecFeature* pFE = m_vecS57RecFE[0];
	QVector<QVector<QPointF>*> pRings;
	genFeaturePolygon(pFE, &pRings);
	m_SpatialPolygon.setPolygon(pRings);//内部经纬度坐标
	qDeleteAll(pRings);
	pRings.clear();

	return &m_SpatialPolygon;
}

//************************************
// Method:    extentMcovr
// Brief:	  Mcovr范围
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57CellMessage::extentMcovr()
{
	//分别查找VI/VC/VE中坐标范围
	double comf = getCellComf();
	long xMin = std::numeric_limits<long>::max();
	long yMin = std::numeric_limits<long>::max();
	long xMax = -std::numeric_limits<long>::max();
	long yMax = -std::numeric_limits<long>::max();
	//MCOVR没有VI孤立点
	for (int i = 0; i < m_vecS57RecVC.size(); i++)
	{
		if (m_vecS57RecVC[i]->m_Sg2d.m_vecSg2d.size() > 0)
		{
			long lat = m_vecS57RecVC[i]->m_Sg2d.m_vecSg2d[0].ycoo;
			long lon = m_vecS57RecVC[i]->m_Sg2d.m_vecSg2d[0].xcoo;
			xMin = std::min(xMin, lon);
			yMin = std::min(yMin, lat);
			xMax = std::max(xMax, lon);
			yMax = std::max(yMax, lat);

		}
	}

	for (int i = 0; i < m_vecS57RecVE.size(); i++)
	{
		for (int j = 0; j < m_vecS57RecVE[i]->m_Sg2d.m_vecSg2d.size(); j++)
		{
			long lat = m_vecS57RecVE[i]->m_Sg2d.m_vecSg2d[j].ycoo;
			long lon = m_vecS57RecVE[i]->m_Sg2d.m_vecSg2d[j].xcoo;
			xMin = std::min(xMin, lon);
			yMin = std::min(yMin, lat);
			xMax = std::max(xMax, lon);
			yMax = std::max(yMax, lat);
		}
	}

	m_McovrRectangle = CRectangle<double>(xMin / comf, yMin / comf, xMax / comf, yMax / comf);
}

//************************************
// Method:    getMcovrCenterPos
// Brief:	  获取Mcovr中心点
// Returns:   QPointF
// Author:    cl
// DateTime:  2022/07/21
//************************************
QPointF CS57CellMessage::getMcovrCenterPos()
{
	extentMcovr();
	double lon = (m_McovrRectangle.xMinimum() + m_McovrRectangle.xMaximum()) / 2.0;
	double lat = (m_McovrRectangle.yMinimum() + m_McovrRectangle.yMaximum()) / 2.0;

	return QPointF(lon, lat);
}

//************************************
// Method:    release
// Brief:	  释放
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57CellMessage::release()
{
	if (m_pRecDsgi != nullptr)
		delete m_pRecDsgi;
	m_pRecDsgi = nullptr;

	if (m_pRecDsgr != nullptr)
		delete m_pRecDsgr;
	m_pRecDsgr = nullptr;

	for (int i = 0; i < m_vecS57RecFE.size(); i++)
	{
		if (m_vecS57RecFE[i] != nullptr)
		{
			delete m_vecS57RecFE[i];
			m_vecS57RecFE[i] = nullptr;
		}
	}
	m_vecS57RecFE.clear();

	for (int i = 0; i < m_vecS57RecVC.size(); i++)
	{
		if (m_vecS57RecVC[i] != nullptr)
		{
			delete m_vecS57RecVC[i];
			m_vecS57RecVC[i] = nullptr;
		}
	}
	m_vecS57RecVC.clear();

	for (int i = 0; i < m_vecS57RecVE.size(); i++)
	{
		if (m_vecS57RecVE[i] != nullptr)
		{
			delete m_vecS57RecVE[i];
			m_vecS57RecVE[i] = nullptr;
		}
	}
	m_vecS57RecVE.clear();
}

