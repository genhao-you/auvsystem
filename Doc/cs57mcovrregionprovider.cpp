#include "stdafx.h"
#include "cs57cellmessage.h"
#include "cs57mcovrregionprovider.h"
#include "cs57docmanager.h"
#include <QMessageBox>
#include <QDebug>

using namespace Doc;
#pragma execution_character_set("utf-8")
CS57McovrRegionProvider::CS57McovrRegionProvider()
	: m_pResultGpcPolygonI(nullptr)
	, m_pResultGpcPolygonII(nullptr)
	, m_pResultGpcPolygonIII(nullptr)
	, m_pResultGpcPolygonIV(nullptr)
	, m_pResultGpcPolygonV(nullptr)
	, m_pResultSpatialPolygonI(nullptr)
	, m_pResultSpatialPolygonII(nullptr)
	, m_pResultSpatialPolygonIII(nullptr)
	, m_pResultSpatialPolygonIV(nullptr)
	, m_pResultSpatialPolygonV(nullptr)
{}

CS57McovrRegionProvider::~CS57McovrRegionProvider()
{
	release();
}

//************************************
// Method:    startDivide
// Brief:	  开始区域拆分
// Returns:   bool
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QVector<CS57CellMessage * > pSourceMessage
//************************************
bool CS57McovrRegionProvider::startDivide(QVector<CS57CellMessage*> pSourceMessage)
{
	if (pSourceMessage.size() == 0)
		return false;

	//图幅路径未发生变化，防止重复分区
	if (!m_strEncDir.isEmpty() && m_strEncDir == CS57DocManager::instance()->getEncDir())
		return true;

	if (pSourceMessage.size() == 0)
	{
		QMessageBox::information(nullptr, "提示", "源信息为空");
		return false;
	}
	clear();
	for (int i = 0; i < pSourceMessage.size(); i++)
	{
		int level;
		double cscl = pSourceMessage[i]->getCellCscl();
		if (cscl < 100000)
		{
			level = 5;
		}
		else if(cscl >= 100000 && cscl < 200000)
		{
			level = 4;
		}
		else if (cscl >= 200000 && cscl < 1000000)
		{
			level = 3;
		}
		else if (cscl >= 1000000 && cscl < 3000000)
		{
			level = 2;
		}
		else if(cscl >= 3000000)
		{
			level = 1;
		}
		switch (level)
		{
		case 1://对应第V级别
			m_vecCellMessageV.push_back(pSourceMessage[i]);
			break;
		case 2://对应第IV级别
			m_vecCellMessageIV.push_back(pSourceMessage[i]);
			break;
		case 3://对应第III级别
			m_vecCellMessageIII.push_back(pSourceMessage[i]);
			break;
		case 4://对应第II级别
			m_vecCellMessageII.push_back(pSourceMessage[i]);
			break;
		case 5://对应第I级别
			m_vecCellMessageI.push_back(pSourceMessage[i]);
			break;
		}
	}

	if (m_vecCellMessageI.size() == 0 &&
		m_vecCellMessageII.size() == 0 &&
		m_vecCellMessageIII.size() == 0 &&
		m_vecCellMessageIV.size() == 0 &&
		m_vecCellMessageV.size() == 0)
	{
		QMessageBox::information(nullptr, "提示", "信息区域划分失败");
		return false;
	}

	unionRegion();
	return true;
}

//************************************
// Method:    clear
// Brief:	  清理
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57McovrRegionProvider::clear()
{
	m_vecCellMessageI.clear();
	m_vecCellMessageII.clear();
	m_vecCellMessageIII.clear();
	m_vecCellMessageIV.clear();
	m_vecCellMessageV.clear();
}

//************************************
// Method:    unionRegion
// Brief:	  对区域进行共边处理
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57McovrRegionProvider::unionRegion()
{
	unionRegionI();
	unionRegionII();
	unionRegionIII();
	unionRegionIV();
	unionRegionV();

	diffRegionV();
	diffRegionIV();
	diffRegionIII();
	diffRegionII();
	diffRegionI();

	toSpatialPolygonI();
	toSpatialPolygonII();
	toSpatialPolygonIII();
	toSpatialPolygonIV();
	toSpatialPolygonV();
}

//************************************
// Method:    unionRegionI
// Brief:	  共边I区域
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57McovrRegionProvider::unionRegionI()
{
	QVector<GpcPolygon*> vecGpcPolygonI;
	for (int i = 0; i < m_vecCellMessageI.size(); i++)
	{
		CSpatialPolygon* pFilterPolygon = m_vecCellMessageI[i]->toSpatialPolygon();
		GpcPolygon* pGpcPolygon = pFilterPolygon->toGpcPolygon();
		vecGpcPolygonI.push_back(pGpcPolygon);
	}

	if (vecGpcPolygonI.size() == 0)
		return;

	if (m_pResultGpcPolygonI != nullptr)
		delete m_pResultGpcPolygonI;
	m_pResultGpcPolygonI = nullptr;

	m_pResultGpcPolygonI = new GpcPolygon(*vecGpcPolygonI[0]);
	for (int i = 1; i < vecGpcPolygonI.size(); i++)
	{
		GpcPolygon * pResultGpcPolygon = new GpcPolygon();
		m_pResultGpcPolygonI->polygon_clip(GPC_UNION, vecGpcPolygonI[i], pResultGpcPolygon);
		delete m_pResultGpcPolygonI;
		m_pResultGpcPolygonI = NULL;
		m_pResultGpcPolygonI = new GpcPolygon(*pResultGpcPolygon);
		delete pResultGpcPolygon;
		pResultGpcPolygon = NULL;
	}
	for (int i = 0; i < vecGpcPolygonI.size(); i++)
	{
		delete vecGpcPolygonI[i];
		vecGpcPolygonI[i] = NULL;
	}
	vecGpcPolygonI.clear();
}

//************************************
// Method:    unionRegionII
// Brief:	  共边II区域
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57McovrRegionProvider::unionRegionII()
{
	QVector<GpcPolygon*> vecGpcPolygonII;
	for (int i = 0; i < m_vecCellMessageII.size(); i++)
	{
		CSpatialPolygon* pFilterPolygon = m_vecCellMessageII[i]->toSpatialPolygon();
		GpcPolygon* pGpcPolygon = pFilterPolygon->toGpcPolygon();
		vecGpcPolygonII.push_back(pGpcPolygon);
	}
	if (vecGpcPolygonII.size() == 0)
		return;

	if (m_pResultGpcPolygonII != nullptr)
		delete m_pResultGpcPolygonII;
	m_pResultGpcPolygonII = nullptr;

	m_pResultGpcPolygonII = new GpcPolygon(*vecGpcPolygonII[0]);
	for (int i = 1; i < vecGpcPolygonII.size(); i++)
	{
		GpcPolygon * pResultGpcPolygon = new GpcPolygon();
		m_pResultGpcPolygonII->polygon_clip(GPC_UNION, vecGpcPolygonII[i], pResultGpcPolygon);
		delete m_pResultGpcPolygonII;
		m_pResultGpcPolygonII = NULL;
		m_pResultGpcPolygonII = new GpcPolygon(*pResultGpcPolygon);
		delete pResultGpcPolygon;
		pResultGpcPolygon = NULL;
	}
	for (int i = 0; i < vecGpcPolygonII.size(); i++)
	{
		delete vecGpcPolygonII[i];
		vecGpcPolygonII[i] = NULL;
	}
	vecGpcPolygonII.clear();
}

//************************************
// Method:    unionRegionIII
// Brief:	  共边III区域
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57McovrRegionProvider::unionRegionIII()
{
	QVector<GpcPolygon*> vecGpcPolygonIII;
	for (int i = 0; i < m_vecCellMessageIII.size(); i++)
	{
		CSpatialPolygon* pFilterPolygon = m_vecCellMessageIII[i]->toSpatialPolygon();
		GpcPolygon* pGpcPolygon = pFilterPolygon->toGpcPolygon();
		vecGpcPolygonIII.push_back(pGpcPolygon);
	}
	if (vecGpcPolygonIII.size() == 0)
		return;

	if (m_pResultGpcPolygonIII != nullptr)
		delete m_pResultGpcPolygonIII;
	m_pResultGpcPolygonIII = nullptr;

	m_pResultGpcPolygonIII = new GpcPolygon(*vecGpcPolygonIII[0]);
	for (int i = 1; i < vecGpcPolygonIII.size(); i++)
	{
		GpcPolygon * pResultGpcPolygon = new GpcPolygon();
		m_pResultGpcPolygonIII->polygon_clip(GPC_UNION, vecGpcPolygonIII[i], pResultGpcPolygon);
		delete m_pResultGpcPolygonIII;
		m_pResultGpcPolygonIII = NULL;
		m_pResultGpcPolygonIII = new GpcPolygon(*pResultGpcPolygon);
		delete pResultGpcPolygon;
		pResultGpcPolygon = NULL;
	}
	for (int i = 0; i < vecGpcPolygonIII.size(); i++)
	{
		delete vecGpcPolygonIII[i];
		vecGpcPolygonIII[i] = NULL;
	}
	vecGpcPolygonIII.clear();
}

//************************************
// Method:    unionRegionIV
// Brief:	  共边IV区域
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57McovrRegionProvider::unionRegionIV()
{
	QVector<GpcPolygon*> vecGpcPolygonIV;
	for (int i = 0; i < m_vecCellMessageIV.size(); i++)
	{
		CSpatialPolygon* pFilterPolygon = m_vecCellMessageIV[i]->toSpatialPolygon();
		GpcPolygon* pGpcPolygon = pFilterPolygon->toGpcPolygon();
		vecGpcPolygonIV.push_back(pGpcPolygon);
	}
	if (vecGpcPolygonIV.size() == 0)
		return;

	if (m_pResultGpcPolygonIV != nullptr)
		delete m_pResultGpcPolygonIV;
	m_pResultGpcPolygonIV = nullptr;

	m_pResultGpcPolygonIV = new GpcPolygon(*vecGpcPolygonIV[0]);
	for (int i = 1; i < vecGpcPolygonIV.size(); i++)
	{
		GpcPolygon * pResultGpcPolygon = new GpcPolygon();
		m_pResultGpcPolygonIV->polygon_clip(GPC_UNION, vecGpcPolygonIV[i], pResultGpcPolygon);
		delete m_pResultGpcPolygonIV;
		m_pResultGpcPolygonIV = NULL;
		m_pResultGpcPolygonIV = new GpcPolygon(*pResultGpcPolygon);
		delete pResultGpcPolygon;
		pResultGpcPolygon = NULL;
	}
	for (int i = 0; i < vecGpcPolygonIV.size(); i++)
	{
		delete vecGpcPolygonIV[i];
		vecGpcPolygonIV[i] = NULL;
	}
	vecGpcPolygonIV.clear();
}

//************************************
// Method:    unionRegionV
// Brief:	  共边V区域
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57McovrRegionProvider::unionRegionV()
{
	QVector<GpcPolygon*> vecGpcPolygonV;
	for (int i = 0; i < m_vecCellMessageV.size(); i++)
	{
		CSpatialPolygon* pFilterPolygon = m_vecCellMessageV[i]->toSpatialPolygon();
		GpcPolygon* pGpcPolygon = pFilterPolygon->toGpcPolygon();
		vecGpcPolygonV.push_back(pGpcPolygon);
	}
	if (vecGpcPolygonV.size() == 0)
		return;

	if (m_pResultGpcPolygonV != nullptr)
		delete m_pResultGpcPolygonV;
	m_pResultGpcPolygonV = nullptr;

	m_pResultGpcPolygonV = new GpcPolygon(*vecGpcPolygonV[0]);
	for (int i = 1; i < vecGpcPolygonV.size(); i++)
	{
		GpcPolygon * pResultGpcPolygon = new GpcPolygon();
		m_pResultGpcPolygonV->polygon_clip(GPC_UNION, vecGpcPolygonV[i], pResultGpcPolygon);
		delete m_pResultGpcPolygonV;
		m_pResultGpcPolygonV = NULL;
		m_pResultGpcPolygonV = new GpcPolygon(*pResultGpcPolygon);
		delete pResultGpcPolygon;
		pResultGpcPolygon = NULL;
	}
	for (int i = 0; i < vecGpcPolygonV.size(); i++)
	{
		delete vecGpcPolygonV[i];
		vecGpcPolygonV[i] = NULL;
	}
	vecGpcPolygonV.clear();
}

//************************************
// Method:    diffRegionV
// Brief:	  对V区域中IV区域挖孔
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57McovrRegionProvider::diffRegionV()
{
	if (!m_pResultGpcPolygonV)
		return;

	if (m_pResultGpcPolygonIV)
	{
		GpcPolygon * pResultGpcPolygon = new GpcPolygon();
		m_pResultGpcPolygonV->polygon_clip(GPC_DIFF, m_pResultGpcPolygonIV, pResultGpcPolygon);
		delete m_pResultGpcPolygonV;
		m_pResultGpcPolygonV = NULL;
		m_pResultGpcPolygonV = new GpcPolygon(*pResultGpcPolygon);
		delete pResultGpcPolygon;
		pResultGpcPolygon = NULL;
	}
	if (m_pResultGpcPolygonIII)
	{
		GpcPolygon * pResultGpcPolygon = new GpcPolygon();
		m_pResultGpcPolygonV->polygon_clip(GPC_DIFF, m_pResultGpcPolygonIII, pResultGpcPolygon);
		delete m_pResultGpcPolygonV;
		m_pResultGpcPolygonV = NULL;
		m_pResultGpcPolygonV = new GpcPolygon(*pResultGpcPolygon);
		delete pResultGpcPolygon;
		pResultGpcPolygon = NULL;
	}
	if (m_pResultGpcPolygonII)
	{
		GpcPolygon * pResultGpcPolygon = new GpcPolygon();
		m_pResultGpcPolygonV->polygon_clip(GPC_DIFF, m_pResultGpcPolygonII, pResultGpcPolygon);
		delete m_pResultGpcPolygonV;
		m_pResultGpcPolygonV = NULL;
		m_pResultGpcPolygonV = new GpcPolygon(*pResultGpcPolygon);
		delete pResultGpcPolygon;
		pResultGpcPolygon = NULL;
	}
	if (m_pResultGpcPolygonI)
	{
		GpcPolygon * pResultGpcPolygon = new GpcPolygon();
		m_pResultGpcPolygonV->polygon_clip(GPC_DIFF, m_pResultGpcPolygonI, pResultGpcPolygon);
		delete m_pResultGpcPolygonV;
		m_pResultGpcPolygonV = NULL;
		m_pResultGpcPolygonV = new GpcPolygon(*pResultGpcPolygon);
		delete pResultGpcPolygon;
		pResultGpcPolygon = NULL;
	}
}

//************************************
// Method:    diffRegionIV
// Brief:	  对IV区域的III区域挖孔
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57McovrRegionProvider::diffRegionIV()
{
	if (!m_pResultGpcPolygonIV)
		return;

	if (m_pResultGpcPolygonIII)
	{
		GpcPolygon * pResultGpcPolygon = new GpcPolygon();
		m_pResultGpcPolygonIV->polygon_clip(GPC_DIFF, m_pResultGpcPolygonIII, pResultGpcPolygon);
		delete m_pResultGpcPolygonIV;
		m_pResultGpcPolygonIV = NULL;
		m_pResultGpcPolygonIV = new GpcPolygon(*pResultGpcPolygon);
		delete pResultGpcPolygon;
		pResultGpcPolygon = NULL;
	}
	if (m_pResultGpcPolygonII)
	{
		GpcPolygon * pResultGpcPolygon = new GpcPolygon();
		m_pResultGpcPolygonIV->polygon_clip(GPC_DIFF, m_pResultGpcPolygonII, pResultGpcPolygon);
		delete m_pResultGpcPolygonIV;
		m_pResultGpcPolygonIV = NULL;
		m_pResultGpcPolygonIV = new GpcPolygon(*pResultGpcPolygon);
		delete pResultGpcPolygon;
		pResultGpcPolygon = NULL;
	}
	if (m_pResultGpcPolygonI)
	{
		GpcPolygon * pResultGpcPolygon = new GpcPolygon();
		m_pResultGpcPolygonIV->polygon_clip(GPC_DIFF, m_pResultGpcPolygonI, pResultGpcPolygon);
		delete m_pResultGpcPolygonIV;
		m_pResultGpcPolygonIV = NULL;
		m_pResultGpcPolygonIV = new GpcPolygon(*pResultGpcPolygon);
		delete pResultGpcPolygon;
		pResultGpcPolygon = NULL;
	}
}

//************************************
// Method:    diffRegionIII
// Brief:	  对III区域的II区域挖孔
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57McovrRegionProvider::diffRegionIII()
{
	if (!m_pResultGpcPolygonIII)
		return;

	if (m_pResultGpcPolygonII)
	{
		GpcPolygon * pResultGpcPolygon = new GpcPolygon();
		m_pResultGpcPolygonIII->polygon_clip(GPC_DIFF, m_pResultGpcPolygonII, pResultGpcPolygon);
		delete m_pResultGpcPolygonIII;
		m_pResultGpcPolygonIII = NULL;
		m_pResultGpcPolygonIII = new GpcPolygon(*pResultGpcPolygon);
		delete pResultGpcPolygon;
		pResultGpcPolygon = NULL;
	}
	if (m_pResultGpcPolygonI)
	{
		GpcPolygon * pResultGpcPolygon = new GpcPolygon();
		m_pResultGpcPolygonIII->polygon_clip(GPC_DIFF, m_pResultGpcPolygonI, pResultGpcPolygon);
		delete m_pResultGpcPolygonIII;
		m_pResultGpcPolygonIII = NULL;
		m_pResultGpcPolygonIII = new GpcPolygon(*pResultGpcPolygon);
		delete pResultGpcPolygon;
		pResultGpcPolygon = NULL;
	}
}

//************************************
// Method:    diffRegionII
// Brief:	  对II区域的I区域挖孔
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57McovrRegionProvider::diffRegionII()
{
	if (!m_pResultGpcPolygonII)
		return;

	if (m_pResultGpcPolygonI)
	{
		GpcPolygon * pResultGpcPolygon = new GpcPolygon();
		m_pResultGpcPolygonII->polygon_clip(GPC_DIFF, m_pResultGpcPolygonI, pResultGpcPolygon);
		delete m_pResultGpcPolygonII;
		m_pResultGpcPolygonII = NULL;
		m_pResultGpcPolygonII = new GpcPolygon(*pResultGpcPolygon);
		delete pResultGpcPolygon;
		pResultGpcPolygon = NULL;
	}
}

void CS57McovrRegionProvider::diffRegionI()
{}

//************************************
// Method:    toSpatialPolygonI
// Brief:	  将I结果多边形转换为空间多边形I
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57McovrRegionProvider::toSpatialPolygonI()
{
	try
	{
		if (!m_pResultGpcPolygonI)
			return;

		if(m_pResultSpatialPolygonI != nullptr)
			m_pResultSpatialPolygonI->relPolygon();
		QVector<QVector<QPointF>*> pRings;
		m_pResultGpcPolygonI->toPolygonF(&pRings);
		m_pResultSpatialPolygonI = new CSpatialPolygon();
		m_pResultSpatialPolygonI->setPolygon(pRings);
		qDeleteAll(pRings);
		pRings.clear();
	}
	catch (std::bad_exception& e)
	{
		//qDebug() << e.what() << endl;
	}
}

//************************************
// Method:    toSpatialPolygonII
// Brief:	  将结果多边形II转换为空间多边形II
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57McovrRegionProvider::toSpatialPolygonII()
{
	try
	{
		if (!m_pResultGpcPolygonII)
			return;
		if (m_pResultSpatialPolygonII != nullptr)
			m_pResultSpatialPolygonII->relPolygon();
		QVector<QVector<QPointF>*> pRings;
		m_pResultGpcPolygonII->toPolygonF(&pRings);
		m_pResultSpatialPolygonII = new CSpatialPolygon();
		m_pResultSpatialPolygonII->setPolygon(pRings);
		qDeleteAll(pRings);
		pRings.clear();
	}
	catch (std::bad_exception& e)
	{
		//qDebug() << e.what() << endl;
	}
}

//************************************
// Method:    toSpatialPolygonIII
// Brief:	  将结果多边形III转为空间多边形III
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57McovrRegionProvider::toSpatialPolygonIII()
{
	try
	{
		if (!m_pResultGpcPolygonIII)
			return;
		if (m_pResultSpatialPolygonIII != nullptr)
			m_pResultSpatialPolygonIII->relPolygon();
		QVector<QVector<QPointF>*> pRings;
		m_pResultGpcPolygonIII->toPolygonF(&pRings);
		m_pResultSpatialPolygonIII = new CSpatialPolygon();
		m_pResultSpatialPolygonIII->setPolygon(pRings);
		qDeleteAll(pRings);
		pRings.clear();
	}
	catch (std::bad_exception& e)
	{
		//qDebug() << e.what() << endl;
	}
}

//************************************
// Method:    toSpatialPolygonIV
// Brief:	  将结果多边形IV转换为空间多边形IV
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57McovrRegionProvider::toSpatialPolygonIV()
{
	try
	{
		if (!m_pResultGpcPolygonIV)
			return;

		if (m_pResultSpatialPolygonIV != nullptr)
			m_pResultSpatialPolygonIV->relPolygon();
		QVector<QVector<QPointF>*> pRings;
		m_pResultGpcPolygonIV->toPolygonF(&pRings);
		m_pResultSpatialPolygonIV = new CSpatialPolygon();
		m_pResultSpatialPolygonIV->setPolygon(pRings);
		qDeleteAll(pRings);
		pRings.clear();
	}
	catch (std::bad_exception& e)
	{
		//qDebug() << e.what() << endl;
	}
}

//************************************
// Method:    toSpatialPolygonV
// Brief:	  将结果多边形V转换为空间多边形V
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57McovrRegionProvider::toSpatialPolygonV()
{
	try
	{
		if (!m_pResultGpcPolygonV)
			return;

		if (m_pResultSpatialPolygonV != nullptr)
			m_pResultSpatialPolygonV->relPolygon();
		QVector<QVector<QPointF>*> pRings;
		m_pResultGpcPolygonV->toPolygonF(&pRings);
		m_pResultSpatialPolygonV = new CSpatialPolygon();
		m_pResultSpatialPolygonV->setPolygon(pRings);
		qDeleteAll(pRings);
		pRings.clear();
	}
	catch (std::bad_exception& e)
	{
		//qDebug() << e.what() << endl;
	}
}

//************************************
// Method:    closeDivide
// Brief:	  //关闭拆分
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57McovrRegionProvider::closeDivide()
{
	release();
	clear();
}

//************************************
// Method:    release
// Brief:	  释放
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57McovrRegionProvider::release()
{
	if (m_pResultGpcPolygonI != nullptr)
		delete m_pResultGpcPolygonI;
	m_pResultGpcPolygonI = nullptr;

	if (m_pResultGpcPolygonII != nullptr)
		delete m_pResultGpcPolygonII;
	m_pResultGpcPolygonII = nullptr;

	if (m_pResultGpcPolygonIII != nullptr)
		delete m_pResultGpcPolygonIII;
	m_pResultGpcPolygonIII = nullptr;

	if (m_pResultGpcPolygonIV != nullptr)
		delete m_pResultGpcPolygonIV;
	m_pResultGpcPolygonIV = nullptr;

	if (m_pResultGpcPolygonV != nullptr)
		delete m_pResultGpcPolygonV;
	m_pResultGpcPolygonV = nullptr;

	if (m_pResultSpatialPolygonI != nullptr)
		delete m_pResultSpatialPolygonI;
	m_pResultSpatialPolygonI = nullptr;

	if (m_pResultSpatialPolygonII != nullptr)
		delete m_pResultSpatialPolygonII;
	m_pResultSpatialPolygonII = nullptr;

	if (m_pResultSpatialPolygonIII != nullptr)
		delete m_pResultSpatialPolygonIII;
	m_pResultSpatialPolygonIII = nullptr;

	if (m_pResultSpatialPolygonIV != nullptr)
		delete m_pResultSpatialPolygonIV;
	m_pResultSpatialPolygonIV = nullptr;

	if (m_pResultSpatialPolygonV != nullptr)
		delete m_pResultSpatialPolygonV;
	m_pResultSpatialPolygonV = nullptr;
}
