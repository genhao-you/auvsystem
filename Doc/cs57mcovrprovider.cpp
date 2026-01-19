#include "stdafx.h"
#include "cs57mcovrprovider.h"
#include "cs57cellmessage.h"
#include "spatialpolygon.h"
#include "codec.h"
#include <QMessageBox>

using namespace Doc;
CS57McovrProvider::CS57McovrProvider()
{}

CS57McovrProvider::~CS57McovrProvider()
{
	for (int i = 0; i < m_vecSourceMcovr.size(); i++)
	{
		if (m_vecSourceMcovr[i] != nullptr)
			delete m_vecSourceMcovr[i];
		m_vecSourceMcovr[i] = nullptr;
	}
	m_vecSourceMcovr.clear();
}

//************************************
// Method:    addMsg
// Brief:	  添加信息
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CS57CellMessage * pCellMsg
//************************************
void CS57McovrProvider::addMsg(CS57CellMessage* pCellMsg)
{
	m_vecSourceMcovr.push_back(pCellMsg);
}

//************************************
// Method:    readXTM
// Brief:	  读取XTM文件
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: const QString & strXtmPath
//************************************
void CS57McovrProvider::readXTM(const QString& strXtmPath)
{
	//QByteArray baXtm = strXtmPath.toLatin1();
	//char* pszXtm = baXtm.data();
	//FILE *fp = fopen(pszXtm, "rb");
	std::string strXtm = codec->fromUnicode(strXtmPath).data();
	FILE* fp = fopen(strXtm.c_str(), "rb");
	if (fp == NULL)
	{
		//打开文件失败
		return;
	}
	quint16 size;
	fread(&size, sizeof(quint16), 1, fp);
	for (int i = 0; i < size; i++)
	{
		CS57CellMessage* pCellMsg = new CS57CellMessage();
		pCellMsg->ioXTMFile(fp, true);
		if (!pCellMsg->getCellName().isEmpty())//部分读取失败的图幅过滤掉
		{
			m_vecSourceMcovr.push_back(pCellMsg);
		}
		else
		{
			if (pCellMsg != nullptr)
			{
				delete pCellMsg;
				pCellMsg = nullptr;
			}
		}
	}
	fclose(fp);
}

//************************************
// Method:    writeXTM
// Brief:	  写XTM文件
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: const QString & strXtmPath
//************************************
void CS57McovrProvider::writeXTM(const QString& strXtmPath)
{
	//QByteArray baXtm = strXtmPath.toLatin1();
	//char* pszXtm = baXtm.data();
	//FILE *fp = fopen(pszXtm, "wb");
	std::string strXtm = codec->fromUnicode(strXtmPath).data();
	FILE* fp = fopen(strXtm.c_str(), "wb");
	if (fp == NULL)
	{
		//文件打开失败
		return;
	}
	quint16 size = m_vecSourceMcovr.size();
	fwrite(&size, sizeof(quint16), 1, fp);
	for (int i = 0; i < size; i++)
	{
		m_vecSourceMcovr[i]->ioXTMFile(fp, false);
	}
	fclose(fp);
}

//************************************
// Method:    selectChart
// Brief:	  选图
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CSpatialPolygon * pPolygon
// Parameter: double scale
//************************************
void CS57McovrProvider::selectChart(CSpatialPolygon* pPolygon, double scale)
{
	Q_UNUSED(pPolygon);
	Q_UNUSED(scale);
}

//************************************
// Method:    releaseMcovr
// Brief:	  释放Mcovr
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57McovrProvider::releaseMcovr()
{
	for (int i = 0; i < m_vecSourceMcovr.size(); i++)
	{
		if (m_vecSourceMcovr[i] != nullptr)
			delete m_vecSourceMcovr[i];
		m_vecSourceMcovr[i] = nullptr;
	}
	m_vecSourceMcovr.clear();
}

//************************************
// Method:    createFilterPolygon
// Brief:	  将所有M_Covr构建筛选多边形
// Returns:   void
// Author:    cl
// DateTime:  2021/08/23
//************************************
QMap<CS57CellMessage*, CSpatialPolygon*> CS57McovrProvider::getMcovrPolygons()
{
	QMap<CS57CellMessage*,CSpatialPolygon*> mapSpatialPolygons;
	for (int i = 0; i < m_vecSourceMcovr.size(); i++)
	{
		CSpatialPolygon* pPolygon = m_vecSourceMcovr[i]->toSpatialPolygon();
		mapSpatialPolygons.insert(m_vecSourceMcovr[i], pPolygon);
	}

	return mapSpatialPolygons;
}
