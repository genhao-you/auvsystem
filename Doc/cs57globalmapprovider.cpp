#include "stdafx.h"
#include "cs57globalmapprovider.h"
#include "spatialpolygon.h"
#include "codec.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QApplication>

using namespace Doc;
#pragma execution_character_set("utf-8")
CS57GlobalMapProvider::CS57GlobalMapProvider()
	: m_strGlobalPath(QApplication::applicationDirPath() + "/s57Base/base")
{

}

CS57GlobalMapProvider::~CS57GlobalMapProvider()
{
	qDeleteAll(m_vecSourcePts);
	m_vecSourcePts.clear();
	m_vecRenderPts.clear();
}

//************************************
// Method:    readFile
// Brief:	  读取文件
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57GlobalMapProvider::readFile()
{
	if (m_vecSourcePts.size() > 0)
		return;

	QFile file(m_strGlobalPath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	QTextStream in(&file);
	QString line = in.readLine();
	if (line.isNull())
	{
		QMessageBox::information(nullptr, "提示", "base.txt文件读取失败，即将返回。");
		return;
	}

	QStringList vecValues = line.split('=');
	Q_ASSERT(vecValues.size() == 2);
	if (vecValues.size() != 2)
	{
		QMessageBox::information(nullptr, "提示", "base.txt文件读取失败，即将返回。");
		return;
	}

	int nRingNum = vecValues[1].toInt();
	for (int i = 0; i < nRingNum; i++)
	{
		QString line = in.readLine();
		if (line.isNull())
		{
			QMessageBox::information(nullptr, "提示", "base.txt文件读取失败，即将返回。");
			return;
		}
		QStringList vecValues = line.split('=');
		Q_ASSERT(vecValues.size() == 2);
		if (vecValues.size() != 2)
		{
			QMessageBox::information(nullptr, "提示", "base.txt文件读取失败，即将返回。");
			return;
		}
		int nEveryRingPtNum = vecValues[1].toInt();
		CPolylineF* pvecRing = new CPolylineF();
		pvecRing->reserve(nEveryRingPtNum);
		for (int j = 0; j < nEveryRingPtNum; j++)
		{
			QString line = in.readLine();
			if (line.isNull())
			{
				QMessageBox::information(nullptr, "提示", "base.txt文件读取失败，即将返回。");
				return;			
			}
			QStringList vecValues = line.split(',');
			Q_ASSERT(vecValues.size() == 2);
			if (vecValues.size() != 2)
			{
				QMessageBox::information(nullptr, "提示", "base.txt文件读取失败，即将返回。");
				return;
			}
			double dLat = vecValues[0].toDouble();
			double dLon = vecValues[1].toDouble();
			QPointF pt(dLon, dLat);
			pvecRing->push_back(pt);
		}
		m_vecSourcePts.push_back(pvecRing);
	}
	file.close();
}

//************************************
// Method:    selectChart
// Brief:	  全球底图选图
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CSpatialPolygon * pPolygon
// Parameter: double scale
//************************************
void CS57GlobalMapProvider::selectChart(CSpatialPolygon* pPolygon, double scale)
{
	Q_UNUSED(scale);
	m_vecRenderPts.clear();
	for (int i = 0; i < m_vecSourcePts.size(); i++)
	{
		if (pPolygon->isIntersectPolygon(m_vecSourcePts[i]))
		{
			m_vecRenderPts.push_back(m_vecSourcePts[i]);
		}
	}
}

void CS57GlobalMapProvider::releaseRenderPts()
{
	m_vecRenderPts.clear();
}

//************************************
// Method:    readBinFile
// Brief:	  读取二进制全球底图文件
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57GlobalMapProvider::readBinFile()
{
	if (m_vecSourcePts.size() > 0)
		return;

	//QByteArray ba = m_strGlobalPath.toLatin1();
	//char* szGlobalPath = ba.data();
	//FILE* fp = fopen(szGlobalPath, "rb");
	std::string strGlobalPath = codec->fromUnicode(m_strGlobalPath).data();
	FILE* fp = fopen(strGlobalPath.c_str(), "rb");
	if (fp == NULL)
		return;

	long ringNum = 0;
	fread(&ringNum, sizeof(long), 1, fp);
	if (ringNum == 0)
		return;

	int nRingNum = ringNum;
	for (int i = 0; i < nRingNum; i++)
	{
		long everyRingNum = 0;
		fread(&everyRingNum, sizeof(long), 1, fp);
		int nEveryRingPtNum = everyRingNum;
		CPolylineF* pvecRing = new CPolylineF();
		for (int j = 0; j < nEveryRingPtNum; j++)
		{
			long xcoo = 0;
			long ycoo = 0;
			fread(&xcoo, sizeof(long), 1, fp);
			fread(&ycoo, sizeof(long), 1, fp);
			double dLon = (double)xcoo / 10000000;//经度
			double dLat = (double)ycoo / 10000000;//纬度	
			QPointF pt(dLon, dLat);
			pvecRing->push_back(pt);
		}
		m_vecSourcePts.push_back(pvecRing);
	}
	fclose(fp);
}

