#include "stdafx.h"
#include "cs57cellprovider.h"
#include "cs57cellmessage.h"
#include "cs57mcovrprovider.h"
#include "cs57recfeature.h"
#include "cs57docmanager.h"
#include "cs57recvector.h"
#include "spatialpolygon.h"
#include "codec.h"
#include <QMessageBox>
#include <QDebug>

using namespace Doc;
CS57CellProvider::CS57CellProvider()
	: m_bCellLayerVisible(true)
{}

CS57CellProvider::CS57CellProvider(CS57McovrProvider* pMcovrProvider)
	: m_eDisplayCategory(XT::DISPLAY_OTHER)
	, m_bCellLayerVisible(true)
{
	m_pMcovrProvider = pMcovrProvider;
}
//************************************
// Method:    setDisplayCategory
// Brief:	  设置显示分组
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: XT::DisplayCategory displayCategory
//************************************
void CS57CellProvider::setDisplayCategory(XT::DisplayCategory displayCategory)
{
	m_eDisplayCategory = displayCategory;
}

CS57CellProvider::~CS57CellProvider()
{
	for (int i = 0; i < m_vecCellDisplayContext.size(); i++)
	{
		if (m_vecCellDisplayContext[i]->pS57Cell != nullptr)
			delete m_vecCellDisplayContext[i]->pS57Cell;
		m_vecCellDisplayContext[i]->pS57Cell = nullptr;

		if (m_vecCellDisplayContext[i] != nullptr)
			delete m_vecCellDisplayContext[i];
		m_vecCellDisplayContext[i] = nullptr;
	}
	m_vecCellDisplayContext.clear();

	for (int i = 0; i < m_vecCellOpened.size(); i++)
	{
		if (m_vecCellOpened[i] != nullptr)
			delete m_vecCellOpened[i];
		m_vecCellOpened[i] = nullptr;
	}
	m_vecCellOpened.clear();
}
//************************************
// Method:    setProvider
// Brief:	  设置数据源
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CS57McovrProvider * pMcovrProvider
//************************************
void CS57CellProvider::setProvider(CS57McovrProvider* pMcovrProvider)
{
	m_pMcovrProvider = pMcovrProvider;
}
//智能选图
//************************************
// Method:    selectChart
// Brief:	  选图
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CSpatialPolygon * pPolygon
// Parameter: double scale
//************************************
void CS57CellProvider::selectChart(CSpatialPolygon* pPolygon, double scale)
{
	if (!m_bCellLayerVisible)
		return;

	m_pMcovrProvider->m_vecRenderMcovr.clear();
	//清空显示列表
	for (int i = 0; i < m_vecCellDisplayContext.size(); i++)
	{
		if (m_vecCellDisplayContext[i] != nullptr)
			delete m_vecCellDisplayContext[i];
		m_vecCellDisplayContext[i] = nullptr;
	}
	m_vecCellDisplayContext.clear();

	QStringList vecUnloadCellNames;
	QVector<CellOpened*>::iterator iter;
	for (iter = m_vecCellOpened.begin(); iter != m_vecCellOpened.end();)
	{
		CellOpened* pCellOpened = *iter;

		double cscl = pCellOpened->pS57Cell->getCellCscl();
		float relscale = scale / cscl;
		if (relscale > 10)
		{
			//记录比例尺不合适的图幅，不用加载显示
			QString strCellName = pCellOpened->pS57Cell->getCellName();
			vecUnloadCellNames.push_back(strCellName);

			//释放图幅
			pCellOpened->pS57Cell->close();
			delete pCellOpened->pS57Cell;
			pCellOpened->pS57Cell = nullptr;

			delete pCellOpened;
			pCellOpened = nullptr;

			iter = m_vecCellOpened.erase(iter);

			if (iter == m_vecCellOpened.end())
				break;

			continue;
		}

		CS57CellMessage* pCellMsg = m_pMcovrProvider->m_vecSourceMcovr[pCellOpened->nMcovrIdx];
		if (!pCellMsg)
		{
			iter++;
			continue;
		}

		bool loadCell = false;
		for (int i = 0; i < pCellMsg->m_vecS57RecFE.size(); i++)
		{
			CS57RecFeature* pFE = pCellMsg->m_vecS57RecFE[i];
			QVector<QVector<QPointF>*> pRings;
			pCellMsg->genFeaturePolygon(pFE, &pRings);
			if(pRings.size() == 0)
				continue;

			if (pPolygon->isIntersectPolygon(pRings[0]))
			{
				loadCell = true;

				qDeleteAll(pRings);
				pRings.clear();
				break;
			}
			else
			{
				qDeleteAll(pRings);
				pRings.clear();
				continue;
			}
		}
		if (!loadCell)
		{
			//释放图幅
			pCellOpened->pS57Cell->close();
			delete pCellOpened->pS57Cell;
			pCellOpened->pS57Cell = nullptr;

			delete pCellOpened;
			pCellOpened = nullptr;

			iter = m_vecCellOpened.erase(iter);

			if (iter == m_vecCellOpened.end())
				break;
		}
		iter++;
	}
	for (int i = 0; i < m_pMcovrProvider->m_vecSourceMcovr.size(); i++)
	{
		bool opened = false;
		CS57CellMessage* pCellMsg = m_pMcovrProvider->m_vecSourceMcovr[i];
		for (int j = 0; j < m_vecCellOpened.size(); j++)			// 海图已在打开列表
		{
			QString strCellName = m_vecCellOpened[j]->pS57Cell->getCellName();
			QString strMcovrName = pCellMsg->getCellName();
			if (strCellName == strMcovrName)
			{
				opened = true;
				break;
			}
		}
		if (opened) continue;

		for (int j = 0; j < pCellMsg->m_vecS57RecFE.size(); j++)
		{
			if (isInPolygon(pPolygon, pCellMsg, pCellMsg->m_vecS57RecFE[j]))
			{
				double cscl = pCellMsg->getCellCscl();
				float relscale = scale / cscl;
				if (relscale < 10)
				{

					CS57Cell* pCell = new CS57Cell();
					QString strCellName = m_pMcovrProvider->m_vecSourceMcovr[i]->getCellName();
					QString strSencFullPath = pCell->getSencFullPath(strCellName);
					//QByteArray baSencFullPath = strSencFullPath.toLatin1();
					//FILE * fp = fopen(baSencFullPath.data(), "rb");
					std::string stdStrSencFullPath = codec->fromUnicode(strSencFullPath).data();
					FILE* fp = fopen(stdStrSencFullPath.c_str(), "rb");
					if (fp == NULL)
					{
						//释放图幅
						pCell->close();
						delete pCell;
						pCell = nullptr;
						continue;
					}

					pCell->ioSencFile(fp, true);
					CellOpened* pCellOpened = new CellOpened();
					pCellOpened->pS57Cell = pCell;
					pCellOpened->nMcovrIdx = i;
					m_vecCellOpened.push_back(pCellOpened);
				}
				else if (relscale < 500)
				{
					//只显示M_COVR
					m_pMcovrProvider->m_vecRenderMcovr.push_back(pCellMsg);
				}
				else
				{
					//全不显示
					continue;
				}
			}
		}
	}

	//更新显示要素列表
	for (int i = 0; i < m_vecCellOpened.size(); i++)
	{
		CellDisplayContext* pDisplayContext = new CellDisplayContext();
		pDisplayContext->pS57Cell = m_vecCellOpened[i]->pS57Cell;

		for (int j = 0; j < m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE.size(); j++)
		{
			//基础显示/标准显示/全部显示控制部分
			bool bContinue = false;
			for (int k = 0; k < m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE[j]->m_vecInst.size(); k++)
			{
				if (m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE[j]->m_vecInst[k]->dscnCode < 3 &&
					m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE[j]->m_vecInst[k]->dscnCode > m_eDisplayCategory)
				{
					bContinue = true;
					bContinue = true;
					break;
				}
			}
			if (bContinue)
				continue;

			if (isInPolygon(pPolygon, m_vecCellOpened[i]->pS57Cell, m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE[j]))
			{
				switch (m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE[j]->m_Frid.prim)
				{
				case 1:
					pDisplayContext->vecPFeatures.push_back(m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE[j]);
					break;
				case 2:
					pDisplayContext->vecLFeatures.push_back(m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE[j]);
					break;
				case 3:
					pDisplayContext->vecAFeatures.push_back(m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE[j]);
					break;
				default:
					break;
				}
			}
		}
		m_vecCellDisplayContext.push_back(pDisplayContext);
	}
	for (int i = 0; i < m_vecCellDisplayContext.size();i++)
	{
		qSort(m_vecCellDisplayContext[i]->vecPFeatures.begin(), m_vecCellDisplayContext[i]->vecPFeatures.end(), sortFeature);
		qSort(m_vecCellDisplayContext[i]->vecLFeatures.begin(), m_vecCellDisplayContext[i]->vecLFeatures.end(), sortFeature);
		qSort(m_vecCellDisplayContext[i]->vecAFeatures.begin(), m_vecCellDisplayContext[i]->vecAFeatures.end(), sortFeature);
	}
}

//根据图幅名称选图
//************************************
// Method:    selectChart
// Brief:	  选图
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CSpatialPolygon * pPolygon
// Parameter: double scale
// Parameter: QStringList cellNames
//************************************
void CS57CellProvider::selectChart(CSpatialPolygon* pPolygon, double scale, QStringList cellNames)
{
	if (!m_bCellLayerVisible)
		return;

	m_pMcovrProvider->m_vecRenderMcovr.clear();
	//清空显示列表
	for (int i = 0; i < m_vecCellDisplayContext.size(); i++)
	{
		if (m_vecCellDisplayContext[i] != nullptr)
			delete m_vecCellDisplayContext[i];
		m_vecCellDisplayContext[i] = nullptr;
	}
	m_vecCellDisplayContext.clear();

	QStringList vecUnloadCellNames;
	QVector<CellOpened*>::iterator iter;
	for (iter = m_vecCellOpened.begin(); iter != m_vecCellOpened.end(); )
	{
		CellOpened* pCellOpened = *iter;

		double cscl = pCellOpened->pS57Cell->getCellCscl();
		float relscale = scale / cscl;
		if (relscale > 10)
		{
			//记录比例尺不合适的图幅，不用加载显示
			QString strCellName = pCellOpened->pS57Cell->getCellName();
			vecUnloadCellNames.push_back(strCellName);

			//释放图幅
			pCellOpened->pS57Cell->close();
			delete pCellOpened->pS57Cell;
			pCellOpened->pS57Cell = nullptr;

			delete pCellOpened;
			pCellOpened = nullptr;

			iter = m_vecCellOpened.erase(iter);

			if (iter == m_vecCellOpened.end())
				break;

			continue;
		}

		CS57CellMessage* pCellMsg = m_pMcovrProvider->m_vecSourceMcovr[pCellOpened->nMcovrIdx];
		if (!pCellMsg)	continue;

		bool loadCell = false;
		for (int i = 0; i < pCellMsg->m_vecS57RecFE.size(); i++)
		{
			CS57RecFeature* pFE = pCellMsg->m_vecS57RecFE[i];
			QVector<QVector<QPointF>*> pRings;
			pCellMsg->genFeaturePolygon(pFE, &pRings);
			if (pRings.size() == 0)
				continue;

			if (pPolygon->isIntersectPolygon(pRings[0]))
			{
				loadCell = true;

				qDeleteAll(pRings);
				pRings.clear();
				break;
			}
			else
			{
				qDeleteAll(pRings);
				pRings.clear();
				continue;
			}
		}
		if (!loadCell)
		{
			//释放图幅
			pCellOpened->pS57Cell->close();
			delete pCellOpened->pS57Cell;
			pCellOpened->pS57Cell = nullptr;

			delete pCellOpened;
			pCellOpened = nullptr;

			iter = m_vecCellOpened.erase(iter);

			if (iter == m_vecCellOpened.end())
				break;
		}
		iter++;
	}
	for (int i = 0; i < m_pMcovrProvider->m_vecSourceMcovr.size(); i++)
	{
		bool opened = false;
		CS57CellMessage* pCellMsg = m_pMcovrProvider->m_vecSourceMcovr[i];
		for (int j = 0; j < m_vecCellOpened.size(); j++)			// 海图已在打开列表
		{
			QString strCellName = m_vecCellOpened[j]->pS57Cell->getCellName();
			QString strMcovrName = pCellMsg->getCellName();
			if (strCellName == strMcovrName)
			{
				opened = true;
				break;
			}
		}
		if (opened) continue;

		for (int j = 0; j < pCellMsg->m_vecS57RecFE.size(); j++)
		{
			if (isInPolygon(pPolygon, pCellMsg, pCellMsg->m_vecS57RecFE[j]))
			{
				double cscl = pCellMsg->getCellCscl();
				float relscale = scale / cscl;
				if (relscale < 10)
				{

					CS57Cell* pCell = new CS57Cell();
					QString strCellName = m_pMcovrProvider->m_vecSourceMcovr[i]->getCellName();
					QString strSencFullPath = pCell->getSencFullPath(strCellName);
					std::string stdStrSencFullPath = codec->fromUnicode(strSencFullPath).data();
					FILE* fp = fopen(stdStrSencFullPath.c_str(), "rb");
					if (fp == NULL)
					{
						//释放图幅
						pCell->close();
						delete pCell;
						pCell = nullptr;
						continue;
					}

					pCell->ioSencFile(fp, true);
					CellOpened* pCellOpened = new CellOpened();
					pCellOpened->pS57Cell = pCell;
					pCellOpened->nMcovrIdx = i;
					m_vecCellOpened.push_back(pCellOpened);
				}
				else if (relscale < 500)
				{
					//只显示M_COVR
					m_pMcovrProvider->m_vecRenderMcovr.push_back(pCellMsg);
				}
				else
				{
					//全不显示
					continue;
				}
			}
		}
	}

	QVector<CellOpened*>::iterator itr;
	for (itr = m_vecCellOpened.begin(); itr != m_vecCellOpened.end(); )
	{
		bool flag = false;
		for (int i = 0; i < cellNames.size(); i++)
		{
			if (cellNames[i] == (*itr)->pS57Cell->getCellName())
			{
				flag = true;
				break;
			}
		}
		if (!flag)
		{
			//释放图幅
			(*itr)->pS57Cell->close();
			delete (*itr)->pS57Cell;
			(*itr)->pS57Cell = nullptr;

			delete (*itr);
			(*itr) = nullptr;

			itr = m_vecCellOpened.erase(itr);

			if (itr == m_vecCellOpened.end())
				break;

			continue;
		}
		itr++;
	}
	//更新显示要素列表
	for (int i = 0; i < m_vecCellOpened.size(); i++)
	{
		CellDisplayContext* pDisplayContext = new CellDisplayContext();
		pDisplayContext->pS57Cell = m_vecCellOpened[i]->pS57Cell;

		for (int j = 0; j < m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE.size(); j++)
		{
			//基础显示/标准显示/全部显示控制部分
			bool bContinue = false;
			for (int k = 0; k < m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE[j]->m_vecInst.size(); k++)
			{
				if (m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE[j]->m_vecInst[k]->dscnCode < 3 &&
					m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE[j]->m_vecInst[k]->dscnCode > m_eDisplayCategory)
				{
					bContinue = true;
					break;
				}
			}
			if (bContinue)
				continue;

			if (isInPolygon(pPolygon, m_vecCellOpened[i]->pS57Cell, m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE[j]))
			{
				switch (m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE[j]->m_Frid.prim)
				{
				case 1:
					pDisplayContext->vecPFeatures.push_back(m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE[j]);
					break;
				case 2:
					pDisplayContext->vecLFeatures.push_back(m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE[j]);
					break;
				case 3:
					pDisplayContext->vecAFeatures.push_back(m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE[j]);
					break;
				default:
					break;
				}
			}
		}
		m_vecCellDisplayContext.push_back(pDisplayContext);
	}
	for (int i = 0; i < m_vecCellDisplayContext.size(); i++)
	{
		qSort(m_vecCellDisplayContext[i]->vecPFeatures.begin(), m_vecCellDisplayContext[i]->vecPFeatures.end(), sortFeature);
		qSort(m_vecCellDisplayContext[i]->vecLFeatures.begin(), m_vecCellDisplayContext[i]->vecLFeatures.end(), sortFeature);
		qSort(m_vecCellDisplayContext[i]->vecAFeatures.begin(), m_vecCellDisplayContext[i]->vecAFeatures.end(), sortFeature);
	}
}

//视口范围内图幅不关闭
//************************************
// Method:    selectChart2
// Brief:	  选图
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CSpatialPolygon * pPolygon
// Parameter: double scale
//************************************
void CS57CellProvider::selectChart2(CSpatialPolygon* pPolygon, double scale)
{
	if (!m_bCellLayerVisible)
		return;

	m_pMcovrProvider->m_vecRenderMcovr.clear();
	//清空显示列表
	for (int i = 0; i < m_vecCellDisplayContext.size(); i++)
	{
		if (m_vecCellDisplayContext[i] != nullptr)
			delete m_vecCellDisplayContext[i];
		m_vecCellDisplayContext[i] = nullptr;
	}
	m_vecCellDisplayContext.clear();

	QStringList vecUnloadCellNames;
	QVector<CellOpened*>::iterator iter;
	for (iter = m_vecCellOpened.begin(); iter != m_vecCellOpened.end();)
	{
		CellOpened* pCellOpened = *iter;

		double cscl = pCellOpened->pS57Cell->getCellCscl();
		float relscale = scale / cscl;
		if (relscale > 10)
		{
			//记录比例尺不合适的图幅，不用加载显示
			QString strCellName = pCellOpened->pS57Cell->getCellName();
			vecUnloadCellNames.push_back(strCellName);

			//释放图幅
			pCellOpened->pS57Cell->close();
			delete pCellOpened->pS57Cell;
			pCellOpened->pS57Cell = nullptr;

			delete pCellOpened;
			pCellOpened = nullptr;

			iter = m_vecCellOpened.erase(iter);

			if (iter == m_vecCellOpened.end())
				break;

			continue;
		}

		CS57CellMessage* pCellMsg = m_pMcovrProvider->m_vecSourceMcovr[pCellOpened->nMcovrIdx];
		if (!pCellMsg)
		{
			iter++;
			continue;
		}

		bool loadCell = false;
		for (int i = 0; i < pCellMsg->m_vecS57RecFE.size(); i++)
		{
			CS57RecFeature* pFE = pCellMsg->m_vecS57RecFE[i];
			QVector<QVector<QPointF>*> pRings;
			pCellMsg->genFeaturePolygon(pFE, &pRings);
			if (pRings.size() == 0)
				continue;

			if (pPolygon->isIntersectPolygon(pRings[0]))
			{
				loadCell = true;

				qDeleteAll(pRings);
				pRings.clear();
				break;
			}
			else
			{
				qDeleteAll(pRings);
				pRings.clear();
				continue;
			}
		}
		if (!loadCell)
		{
			//释放图幅
			pCellOpened->pS57Cell->close();
			delete pCellOpened->pS57Cell;
			pCellOpened->pS57Cell = nullptr;

			delete pCellOpened;
			pCellOpened = nullptr;

			iter = m_vecCellOpened.erase(iter);

			if (iter == m_vecCellOpened.end())
				break;
		}
		iter++;
	}
	for (int i = 0; i < m_pMcovrProvider->m_vecSourceMcovr.size(); i++)
	{
		bool opened = false;
		CS57CellMessage* pCellMsg = m_pMcovrProvider->m_vecSourceMcovr[i];
		for (int j = 0; j < m_vecCellOpened.size(); j++)			// 海图已在打开列表
		{
			QString strCellName = m_vecCellOpened[j]->pS57Cell->getCellName();
			QString strMcovrName = pCellMsg->getCellName();
			if (strCellName == strMcovrName)
			{
				opened = true;
				break;
			}
		}
		if (opened) continue;

		for (int j = 0; j < pCellMsg->m_vecS57RecFE.size(); j++)
		{
			if (isInPolygon(pPolygon, pCellMsg, pCellMsg->m_vecS57RecFE[j]))
			{
				CS57Cell* pCell = new CS57Cell();
				QString strCellName = m_pMcovrProvider->m_vecSourceMcovr[i]->getCellName();
				QString strSencFullPath = pCell->getSencFullPath(strCellName);
				std::string stdStrSencFullPath = codec->fromUnicode(strSencFullPath).data();
				FILE* fp = fopen(stdStrSencFullPath.c_str(), "rb");
				if (fp == NULL)
				{
					//释放图幅
					pCell->close();
					delete pCell;
					pCell = nullptr;
					continue;
				}

				pCell->ioSencFile(fp, true);
				CellOpened* pCellOpened = new CellOpened();
				pCellOpened->pS57Cell = pCell;
				pCellOpened->nMcovrIdx = i;
				m_vecCellOpened.push_back(pCellOpened);
			}
		}
	}

	//更新显示要素列表
	for (int i = 0; i < m_vecCellOpened.size(); i++)
	{
		CellDisplayContext* pDisplayContext = new CellDisplayContext();
		pDisplayContext->pS57Cell = m_vecCellOpened[i]->pS57Cell;

		for (int j = 0; j < m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE.size(); j++)
		{
			//基础显示/标准显示/全部显示控制部分
			bool bContinue = false;
			for (int k = 0; k < m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE[j]->m_vecInst.size(); k++)
			{
				if (m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE[j]->m_vecInst[k]->dscnCode < 3 &&
					m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE[j]->m_vecInst[k]->dscnCode > m_eDisplayCategory)
				{
					bContinue = true;
					break;
				}
			}
			if (bContinue)
				continue;

			if (isInPolygon(pPolygon, m_vecCellOpened[i]->pS57Cell, m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE[j]))
			{
				switch (m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE[j]->m_Frid.prim)
				{
				case 1:
					pDisplayContext->vecPFeatures.push_back(m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE[j]);
					break;
				case 2:
					pDisplayContext->vecLFeatures.push_back(m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE[j]);
					break;
				case 3:
					pDisplayContext->vecAFeatures.push_back(m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE[j]);
					break;
				default:
					break;
				}
			}
		}
		m_vecCellDisplayContext.push_back(pDisplayContext);
	}
	for (int i = 0; i < m_vecCellDisplayContext.size(); i++)
	{
		qSort(m_vecCellDisplayContext[i]->vecPFeatures.begin(), m_vecCellDisplayContext[i]->vecPFeatures.end(), sortFeature);
		qSort(m_vecCellDisplayContext[i]->vecLFeatures.begin(), m_vecCellDisplayContext[i]->vecLFeatures.end(), sortFeature);
		qSort(m_vecCellDisplayContext[i]->vecAFeatures.begin(), m_vecCellDisplayContext[i]->vecAFeatures.end(), sortFeature);
	}
}

//查询模式使用，只开图，不允许关图
//************************************
// Method:    selectChart3
// Brief:	  选图
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CSpatialPolygon * pPolygon
// Parameter: double scale
//************************************
void CS57CellProvider::selectChart3(CSpatialPolygon * pPolygon, double scale)
{
	m_pMcovrProvider->m_vecRenderMcovr.clear();
	//清空显示列表
	for (int i = 0; i < m_vecCellDisplayContext.size(); i++)
	{
		if (m_vecCellDisplayContext[i] != nullptr)
			delete m_vecCellDisplayContext[i];
		m_vecCellDisplayContext[i] = nullptr;
	}
	m_vecCellDisplayContext.clear();

	for (int i = 0; i < m_pMcovrProvider->m_vecSourceMcovr.size(); i++)
	{
		bool opened = false;
		CS57CellMessage* pCellMsg = m_pMcovrProvider->m_vecSourceMcovr[i];
		for (int j = 0; j < m_vecCellOpened.size(); j++)			// 海图已在打开列表
		{
			QString strCellName = m_vecCellOpened[j]->pS57Cell->getCellName();
			QString strMcovrName = pCellMsg->getCellName();
			if (strCellName == strMcovrName)
			{
				opened = true;
				break;
			}
		}
		if (opened) continue;

		for (int j = 0; j < pCellMsg->m_vecS57RecFE.size(); j++)
		{
			if (isInPolygon(pPolygon, pCellMsg, pCellMsg->m_vecS57RecFE[j]))
			{
				CellOpened* pCellOpened = new CellOpened();
				CS57Cell* pCell = new CS57Cell();
				QString strCellName = m_pMcovrProvider->m_vecSourceMcovr[i]->getCellName();
				QString strSencFullPath = pCell->getSencFullPath(strCellName);
				std::string stdStrSencFullPath = codec->fromUnicode(strSencFullPath).data();
				FILE* fp = fopen(stdStrSencFullPath.c_str(), "rb");
				pCell->ioSencFile(fp, true);
				pCellOpened->pS57Cell = pCell;
				pCellOpened->nMcovrIdx = i;
				m_vecCellOpened.push_back(pCellOpened);
			}
		}
	}

	//更新显示要素列表
	for (int i = 0; i < m_vecCellOpened.size(); i++)
	{
		CellDisplayContext* pDisplayContext = new CellDisplayContext();
		pDisplayContext->pS57Cell = m_vecCellOpened[i]->pS57Cell;

		for (int j = 0; j < m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE.size(); j++)
		{
			if (isInPolygon(pPolygon, m_vecCellOpened[i]->pS57Cell, m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE[j]))
			{
				switch (m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE[j]->m_Frid.prim)
				{
				case 1:
					pDisplayContext->vecPFeatures.push_back(m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE[j]);
					break;
				case 2:
					pDisplayContext->vecLFeatures.push_back(m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE[j]);
					break;
				case 3:
					pDisplayContext->vecAFeatures.push_back(m_vecCellOpened[i]->pS57Cell->m_vecS57RecFE[j]);
					break;
				default:
					break;
				}
			}
		}
		m_vecCellDisplayContext.push_back(pDisplayContext);
	}
	for (int i = 0; i < m_vecCellDisplayContext.size(); i++)
	{
		qSort(m_vecCellDisplayContext[i]->vecPFeatures.begin(), m_vecCellDisplayContext[i]->vecPFeatures.end(), sortFeature);
		qSort(m_vecCellDisplayContext[i]->vecLFeatures.begin(), m_vecCellDisplayContext[i]->vecLFeatures.end(), sortFeature);
		qSort(m_vecCellDisplayContext[i]->vecAFeatures.begin(), m_vecCellDisplayContext[i]->vecAFeatures.end(), sortFeature);
	}
}

//************************************
// Method:    isInPolygon
// Brief:	  Mcovr要素是否在视口多边形内
// Returns:   bool
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CSpatialPolygon * pPolygon
// Parameter: CS57CellMessage * pCellMsg
// Parameter: CS57RecFeature * pFE
//************************************
bool CS57CellProvider::isInPolygon(CSpatialPolygon* pPolygon, CS57CellMessage* pCellMsg, CS57RecFeature* pFE)
{
	if (pFE->m_Frid.prim != 3) return false;

	QVector<QVector<QPointF>*> pRings;
	pCellMsg->genFeaturePolygon(pFE, &pRings);
	bool intersect = pPolygon->isIntersect(&pRings);
	qDeleteAll(pRings);
	pRings.clear();
	return intersect;
}

//************************************
// Method:    isInPolygon
// Brief:	  图幅要素是否在视口多边形内
// Returns:   bool
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CSpatialPolygon * pPolygon
// Parameter: CS57Cell * pCell
// Parameter: CS57RecFeature * pFE
//************************************
bool CS57CellProvider::isInPolygon(CSpatialPolygon* pPolygon, CS57Cell* pCell, CS57RecFeature* pFE)
{
	double lat, lon;
	double comf = pCell->getCellComf();
	if (pFE->m_Frid.prim == 1 && pFE->m_Fspt.m_vecFspt[0].rcnm == 110)
	{
		for (int i = 0;i < pCell->m_vecS57RecVI[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg3d.m_vecSg3d.size();i++)
		{
			lon = pCell->m_vecS57RecVI[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg3d.m_vecSg3d[i].xcoo / comf;
			lat = pCell->m_vecS57RecVI[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg3d.m_vecSg3d[i].ycoo / comf;
			if (pPolygon->ptInPolygon(lon, lat))
				return true;
		}

		if (pCell->m_vecS57RecVI[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d.size() != 1)
			return false;

		lon = pCell->m_vecS57RecVI[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d[0].xcoo / comf;
		lat = pCell->m_vecS57RecVI[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d[0].ycoo / comf;

		return pPolygon->ptInPolygon(lon, lat);
	}
	if (pFE->m_Frid.prim == 1 && pFE->m_Fspt.m_vecFspt[0].rcnm == 120)
	{
		if (pCell->m_vecS57RecVC[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d.size() != 1)
			return false;

		lat = pCell->m_vecS57RecVC[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d[0].ycoo / comf;
		lon = pCell->m_vecS57RecVC[pFE->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d[0].xcoo / comf;

		return pPolygon->ptInPolygon(lon, lat);
	}
	if (pFE->m_Frid.prim == 2)
	{
		QVector<QVector<QPointF>*> pPolylines;
		pCell->genFeaturePolyline(pFE, &pPolylines);
		bool intersect = pPolygon->isIntersectPolyline(&pPolylines);
		qDeleteAll(pPolylines);
		pPolylines.clear();
		return intersect;
	}
	if (pFE->m_Frid.prim == 3)
	{
		QVector<QVector<QPointF>*> vecRings;
		pCell->genFeaturePolygon(pFE, &vecRings);
		bool intersect = pPolygon->isIntersect(&vecRings);
		qDeleteAll(vecRings);
		vecRings.clear();
		return intersect;
	}
	return false;
}

//************************************
// Method:    sortFeature
// Brief:	  要素排序
// Returns:   bool
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CS57RecFeature * pFE1
// Parameter: CS57RecFeature * pFE2
//************************************
bool CS57CellProvider::sortFeature(CS57RecFeature* pFE1, CS57RecFeature* pFE2)
{
	if (pFE1 != nullptr && pFE2 != nullptr &&pFE1->m_vecInst.size() != 0 && pFE2->m_vecInst.size() != 0)
		return pFE1->m_vecInst[0]->dpri < pFE2->m_vecInst[0]->dpri;
	else
		return true;
}

//************************************
// Method:    releaseDisplayContext
// Brief:	  释放显示列表
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57CellProvider::releaseDisplayContext()
{
	for (int i = 0; i < m_vecCellDisplayContext.size(); i++)
	{
		if (m_vecCellDisplayContext[i]->pS57Cell != nullptr)
			delete m_vecCellDisplayContext[i]->pS57Cell;
		m_vecCellDisplayContext[i]->pS57Cell = nullptr;

		if (m_vecCellDisplayContext[i] != nullptr)
			delete m_vecCellDisplayContext[i];
		m_vecCellDisplayContext[i] = nullptr;
	}
	m_vecCellDisplayContext.clear();

	for (int i = 0; i < m_vecCellOpened.size(); i++)
	{
		if (m_vecCellOpened[i] != nullptr)
			delete m_vecCellOpened[i];
		m_vecCellOpened[i] = nullptr;
	}
	m_vecCellOpened.clear();
}

//************************************
// Method:    setCellLayerVisible
// Brief:	  设置图层显隐
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: bool visible
//************************************
void CS57CellProvider::setCellLayerVisible(bool visible)
{
	m_bCellLayerVisible = visible;
}

