#include "stdafx.h"
#include "cs52symboldata.h"
#include "cs57preslib.h"
#include <QFile>
#include <QMessageBox>
#include <QApplication>

using namespace Doc;
#pragma execution_character_set("utf-8")
CS52SymbolData::CS52SymbolData()
	: m_strSymbolDataPath(QApplication::applicationDirPath() + "/s57Data/s52symbol.dai")
{
	readAsciiFile(m_strSymbolDataPath);
}

CS52SymbolData::~CS52SymbolData()
{
	clear();
}

LNST* CS52SymbolData::searchLnst(const QString& lnstName)
{
	lnstName.toUpper();
	QMap<QString, LNST*>::iterator iter = m_mapLnst.find(lnstName);
	if (iter != m_mapLnst.end())
		return iter.value();
	return nullptr;
}

SYMB* CS52SymbolData::searchSymb(const QString& symbName)
{
	symbName.toUpper();
	QMap<QString, SYMB*>::iterator iter = m_mapSymb.find(symbName);
	if (iter != m_mapSymb.end())
		return iter.value();
	return nullptr;
}

PATT* CS52SymbolData::searchPatt(const QString& pattName)
{
	pattName.toUpper();
	QMap<QString, PATT*>::iterator iter = m_mapPatt.find(pattName);
	if (iter != m_mapPatt.end())
		return iter.value();
	return nullptr;
}

void CS52SymbolData::getAllSymbName(QStringList& symbList)
{
	symbList.clear();
	QMap<QString, SYMB*>::iterator iter;
	for (iter = m_mapSymb.begin(); iter != m_mapSymb.end(); iter++)
	{
		symbList.append(iter.key());
	}
}

void CS52SymbolData::getAllPattName(QStringList& pattList)
{
	pattList.clear();
	QMap<QString, PATT*>::iterator iter;
	for (iter = m_mapPatt.begin(); iter != m_mapPatt.end(); iter++)
	{
		pattList.append(iter.key());
	}
}

void CS52SymbolData::getAllLnstName(QStringList& lnstList)
{
	lnstList.clear();
	QMap<QString, LNST*>::iterator iter;
	for (iter = m_mapLnst.begin(); iter != m_mapLnst.end(); iter++)
	{
		lnstList.append(iter.key());
	}
}

int CS52SymbolData::getSymbLibSize() const
{
	return m_mapSymb.size();
}

int CS52SymbolData::getPattLibSize() const
{
	return m_mapPatt.size();
}

int CS52SymbolData::getLnstLibSize() const
{
	return m_mapLnst.size();
}

bool CS52SymbolData::parseVECD(SYMB* symb)
{
	if (!symb)	return false;

	S52Line* pLine = nullptr;							// 线对象指针
	S52Polygon* pPolygon = nullptr;						// 多边形对象指针
	S52Circle* pCircle = nullptr;						// 圆对象指针
	QPoint* pPoint = nullptr;							// 点对象指针
	QPoint curPoint;									// 点对象
	QString strColorName = "CHBLK";
	enum eModePolygon
	{
		PM0 = 0,										// 进入多边形模式
		PM1,											// 定义新的形状
		PM2												// 退出多边形模式
	};
	eModePolygon modePolygon = eModePolygon::PM2;		// 多边形状态

	//bool modePolygon = false;							// 是否多边形模式
	bool initColor = false;								// 颜色是否初始化
	bool initWidth = false;								// 笔宽是否初始化
	bool initTrans = false;								// 透明度是否初始化
	bool penDown = false;								// 是否落笔
	QString command;										// 命令字串
	QString arguments;									// 参数字串	
	QString instStr;										// 指令字串
	QString str;										// 临时字串

	S52Layer oldLayer;
	S52Layer* pLayer = new S52Layer();					// 符号层对象指针
	symb->m_vecS52Layers.push_back(pLayer);
	oldLayer = *pLayer;

	for (int i = 0; i < symb->m_vecFieldSvct.size(); i++)
	{
		instStr = symb->m_vecFieldSvct[i]->VECD;
		QStringList lisInstStr = instStr.split(';');
		for (int j = 0; j < lisInstStr.size() - 1; j++)
		{
			command = lisInstStr[j].left(2);
			arguments = lisInstStr[j].mid(2, lisInstStr[j].size() - 2);
			if (command == "SP")
			{
				// 根据指令字符，获取颜色名
				for (int k = 0; k < symb->m_fieldScrf.vecSubFieldScrf.size(); k++)
				{
					if (arguments[0] == symb->m_fieldScrf.vecSubFieldScrf[k]->CIDX)
					{
						strColorName = symb->m_fieldScrf.vecSubFieldScrf[k]->CTOK;
					}
				}
				if (initColor)
				{
					if (pLayer->strColorName != strColorName)
					{
						pLayer = new S52Layer();
						symb->m_vecS52Layers.push_back(pLayer);
					}
				}

				pLayer->strColorName = strColorName;
				pLayer->nPenWidth = oldLayer.nPenWidth;
				pLayer->uTransparent = oldLayer.uTransparent;
				oldLayer = *pLayer;
				initColor = true;
				continue;
			}

			if ("SW" == command)								// SW		
			{
				int width = arguments.toUShort();
				if (initWidth)
				{
					if (width != pLayer->nPenWidth)
					{
						pLayer = new S52Layer;
						symb->m_vecS52Layers.push_back(pLayer);
					}
				}
				pLayer->nPenWidth = width;
				pLayer->strColorName = oldLayer.strColorName;
				pLayer->uTransparent = oldLayer.uTransparent;
				oldLayer = *pLayer;
				initWidth = true;
				continue;
			}

			if ("ST" == command)								// ST
			{
				unsigned short trans = arguments.toUShort();
				if (initTrans)
				{
					if (pLayer->uTransparent != trans)
					{
						pLayer = new S52Layer;
						symb->m_vecS52Layers.push_back(pLayer);
					}
				}
				pLayer->uTransparent = trans;
				pLayer->strColorName = oldLayer.strColorName;
				pLayer->nPenWidth = oldLayer.nPenWidth;
				oldLayer = *pLayer;
				initTrans = true;
				continue;
			}

			if ("PU" == command)
			{
				curPoint.setX(arguments.split(',')[0].toShort());
				curPoint.setY(arguments.split(',')[1].toShort());
				penDown = false;
				continue;
			}

			if ("PD" == command)
			{
				if (arguments.size() == 0)						// 落笔无坐标，在当前点绘制0.3mm的点
				{
					pCircle = new S52Circle;
					pLayer->vecCircle.push_back(pCircle);
					pCircle->cenPoint = curPoint;
					pCircle->radius = 15;
					pCircle->modeFill = 2;
					continue;
				}

				if (!penDown)									// 当前笔状态为抬笔
				{
					pPoint = new QPoint;
					pPoint->setX(curPoint.x());
					pPoint->setY(curPoint.y());
					if (modePolygon == eModePolygon::PM2)
					{
						pLine = new S52Line;
						pLine->vecLine.push_back(pPoint);
						pLayer->vecLines.push_back(pLine);
					}
					else
					{
						pPolygon = new S52Polygon;
						pPolygon->vecLine.push_back(pPoint);
						pLayer->vecPolygon.push_back(pPolygon);
					}
				}

				QStringList lisArgument = arguments.split(',');
				for (int k = 0; k < lisArgument.size(); k += 2)
				{
					pPoint = new QPoint;
					pPoint->setX(lisArgument[k].toShort());
					pPoint->setY(lisArgument[k + 1].toShort());
					if (modePolygon != eModePolygon::PM2)
						pPolygon->vecLine.push_back(pPoint);
					else
						pLine->vecLine.push_back(pPoint);
				}
				curPoint = *pPoint;
				penDown = true;
				continue;
			}

			if ("CI" == command)
			{
				pCircle = new S52Circle;
				pLayer->vecCircle.push_back(pCircle);
				pCircle->cenPoint = curPoint;
				pCircle->radius = arguments.toShort();
				continue;
			}

			// PM指令将命令解释器置于多边形定义模式。在这种模式下，您可以使用其他指令（PU，PD，CI）来构造多边形。这些说明存储在
			// 多边形缓冲区在完全定义多边形之前，它们不会执行。为了绘制多边形，必须使用FP指令填充和 / 或使用EP指令对其进行轮廓显示。界定
			// 多边形将笔移动到所需的起始位置（请参见PU或PD）。然后执行PM 0进入多边形模式，并指定适当的指令以定义多边形的形状。如果你
			// 想要定义一个子多边形，以PM 1指令结束形状并定义下一个形状；执行PM 2退出多边形模式。 PM 0之前的当前笔位置是多边形的第一个点（顶点）。
			// 可以使用笔向上或向下定义顶点（请参见PU，PD）。但是，“如果要使用EP指令勾勒多边形，请注意，EP只会绘制那些用笔向下定义的点”。
			// 另一方面，FP指令填充多边形，而与笔的上 / 下状态无关。
			if ("PM" == command)
			{
				penDown = false;
				int mode = arguments.toShort();
				switch (mode)
				{
				case 0:
					modePolygon = eModePolygon::PM0;
					break;
				case 1:
					modePolygon = eModePolygon::PM1;
					break;
				case 2:
					modePolygon = eModePolygon::PM2;
					break;
				}
			}

			// FP指令填充先前放置在多边形缓冲区中的多边形（请参阅PM）。 
			// 使用当前的笔颜色（请参见SP）和透明度（请参见ST）填充多边形。 
			// FP指令完成后，将恢复原始笔的位置和状态（上下笔）。
			if ("FP" == command)
			{
				if (pPolygon != nullptr)
					pPolygon->modeFill = 1;
				if (pCircle != nullptr)
					pCircle->modeFill = 1;
			}
			if ("EP" == command){}
		}
	}
	return true;
}

bool CS52SymbolData::parseVECD(PATT* patt)
{
	if (!patt)	return false;

	S52Layer oldLayer;
	S52Line* pLine = NULL;							// 线对象指针
	S52Polygon* pPolygon = NULL;						// 多边形对象指针
	S52Circle* pCircle = NULL;						// 圆对象指针
	QPoint* pPoint = NULL;							// 点对象指针
	QPoint curPoint;									// 点对象
	QString strColorName = "CHBLK";
	enum eModePolygon
	{
		PM0 = 0,	// 进入多边形模式
		PM1,		// 定义新的形状
		PM2			// 退出多边形模式
	};
	eModePolygon modePolygon = eModePolygon::PM2;		// 多边形状态

	//bool modePolygon = false;							// 是否多边形模式
	bool initColor = false;								// 颜色是否初始化
	bool initWidth = false;								// 笔宽是否初始化
	bool initTrans = false;								// 透明度是否初始化
	bool penDown = false;								// 是否落笔
	QString command;									// 命令字串
	QString arguments;									// 参数字串	
	QString instStr;									// 指令字串
	QString str;										// 临时字串
						
	S52Layer* pLayer = new S52Layer();						// 符号层对象指针
	patt->m_vecS52Layers.push_back(pLayer);
	oldLayer = *pLayer;


	for (int i = 0; i < patt->m_vecFieldPvct.size(); i++)
	{
		instStr = patt->m_vecFieldPvct[i]->VECD;
		QStringList lisInstStr = instStr.split(';');
		for (int j = 0; j < lisInstStr.size() - 1; j++)
		{
			command = lisInstStr[j].left(2);
			arguments = lisInstStr[j].mid(2, lisInstStr[j].size() - 2);
			if (command == "SP")
			{
				// 根据指令字符，获取颜色名
				for (int k = 0; k < patt->m_fieldPcrf.vecSubfieldPcrf.size(); k++)
				{
					if (arguments[0] == patt->m_fieldPcrf.vecSubfieldPcrf[k]->CIDX)
					{
						strColorName = patt->m_fieldPcrf.vecSubfieldPcrf[k]->CTOK;
					}
				}
				if (initColor)
				{
					if (pLayer->strColorName != strColorName)
					{
						pLayer = new S52Layer;
						patt->m_vecS52Layers.push_back(pLayer);
					}
				}

				pLayer->strColorName = strColorName;
				pLayer->nPenWidth = oldLayer.nPenWidth;
				pLayer->uTransparent = oldLayer.uTransparent;
				oldLayer = *pLayer;
				initColor = true;
				continue;
			}

			if ("SW" == command)								// SW		
			{
				int width = arguments.toUShort();
				if (initWidth)
				{
					if (width != pLayer->nPenWidth)
					{
						pLayer = new S52Layer;
						patt->m_vecS52Layers.push_back(pLayer);
					}
				}
				pLayer->nPenWidth = width;
				pLayer->strColorName = oldLayer.strColorName;
				pLayer->uTransparent = oldLayer.uTransparent;
				oldLayer = *pLayer;
				initWidth = true;
				continue;
			}

			if ("ST" == command)								// ST
			{
				unsigned short trans = arguments.toUShort();
				if (initTrans)
				{
					if (pLayer->uTransparent != trans)
					{
						pLayer = new S52Layer;
						patt->m_vecS52Layers.push_back(pLayer);
					}
				}
				pLayer->uTransparent = trans;
				pLayer->strColorName = oldLayer.strColorName;
				pLayer->nPenWidth = oldLayer.nPenWidth;
				oldLayer = *pLayer;
				initTrans = true;
				continue;
			}

			if ("PU" == command)
			{
				curPoint.setX(arguments.split(',')[0].toShort());
				curPoint.setY(arguments.split(',')[1].toShort());
				penDown = false;
				continue;
			}

			if ("PD" == command)
			{
				if (arguments.size() == 0)  // 落笔无坐标，在当前点绘制0.3mm的点
				{
					pCircle = new S52Circle;
					pLayer->vecCircle.push_back(pCircle);
					pCircle->cenPoint = curPoint;
					pCircle->radius = 15;
					pCircle->modeFill = 2;
					continue;
				}

				if (!penDown)  // 当前笔状态为抬笔
				{
					pPoint = new QPoint;
					pPoint->setX(curPoint.x());
					pPoint->setY(curPoint.y());
					if (modePolygon == eModePolygon::PM2)
					{
						pLine = new S52Line;
						pLine->vecLine.push_back(pPoint);
						pLayer->vecLines.push_back(pLine);
					}
					else
					{
						pPolygon = new S52Polygon;
						pPolygon->vecLine.push_back(pPoint);
						pLayer->vecPolygon.push_back(pPolygon);
					}
				}

				QStringList lisArgument = arguments.split(',');
				for (int k = 0; k < lisArgument.size(); k += 2)
				{
					pPoint = new QPoint;
					pPoint->setX(lisArgument[k].toShort());
					pPoint->setY(lisArgument[k + 1].toShort());
					if (modePolygon != eModePolygon::PM2)
						pPolygon->vecLine.push_back(pPoint);
					else
						pLine->vecLine.push_back(pPoint);
				}
				curPoint = *pPoint;
				penDown = true;
				continue;
			}

			if ("CI" == command)
			{
				pCircle = new S52Circle;
				pLayer->vecCircle.push_back(pCircle);
				pCircle->cenPoint = curPoint;
				pCircle->radius = arguments.toShort();
				continue;
			}

			// PM指令将命令解释器置于多边形定义模式。在这种模式下，您可以使用其他指令（PU，PD，CI）来构造多边形。这些说明存储在
			// 多边形缓冲区在完全定义多边形之前，它们不会执行。为了绘制多边形，必须使用FP指令填充和 / 或使用EP指令对其进行轮廓显示。界定
			// 多边形将笔移动到所需的起始位置（请参见PU或PD）。然后执行PM 0进入多边形模式，并指定适当的指令以定义多边形的形状。如果你
			// 想要定义一个子多边形，以PM 1指令结束形状并定义下一个形状；执行PM 2退出多边形模式。 PM 0之前的当前笔位置是多边形的第一个点（顶点）。
			// 可以使用笔向上或向下定义顶点（请参见PU，PD）。但是，“如果要使用EP指令勾勒多边形，请注意，EP只会绘制那些用笔向下定义的点”。
			// 另一方面，FP指令填充多边形，而与笔的上 / 下状态无关。
			if ("PM" == command)
			{
				penDown = false;
				int mode = arguments.toShort();
				switch (mode)
				{
				case 0:
					modePolygon = eModePolygon::PM0;
					break;
				case 1:
					modePolygon = eModePolygon::PM1;
					break;
				case 2:
					modePolygon = eModePolygon::PM2;
					break;
				}
			}

			// FP指令填充先前放置在多边形缓冲区中的多边形（请参阅PM）。 
			// 使用当前的笔颜色（请参见SP）和透明度（请参见ST）填充多边形。 
			// FP指令完成后，将恢复原始笔的位置和状态（上下笔）。
			if ("FP" == command)
			{
				if (pPolygon != nullptr)
					pPolygon->modeFill = 1;
				if (pCircle != nullptr)
					pCircle->modeFill = 1;

			}
			if ("EP" == command){}
		}
	}
	return true;
}

bool CS52SymbolData::parseVECD(LNST* lnst)
{
	if (!lnst)	return false;

	S52Layer oldLayer;
	S52Line* pLine = nullptr;							// 线对象指针
	S52Polygon* pPolygon = nullptr;						// 多边形对象指针
	S52Circle* pCircle = nullptr;						// 圆对象指针
	QPoint* pPoint = nullptr;							// 点对象指针
	QPoint curPoint;									// 点对象
	QString strColorName = "CHBLK";
	enum eModePolygon
	{
		PM0 = 0,	// 进入多边形模式
		PM1,		// 定义新的形状
		PM2			// 退出多边形模式
	};
	eModePolygon modePolygon = eModePolygon::PM2;						// 多边形状态

																		//	bool modePolygon = false;							// 是否多边形模式
	bool initColor = false;								// 颜色是否初始化
	bool initWidth = false;								// 笔宽是否初始化
	bool initTrans = false;								// 透明度是否初始化
	bool penDown = false;								// 是否落笔
	QString command;										// 命令字串
	QString arguments;									// 参数字串	
	QString instStr;										// 指令字串
	QString str;										// 临时字串
						
	S52Layer* pLayer = new S52Layer();					// 符号层对象指针
	lnst->m_vecS52Layers.push_back(pLayer);
	oldLayer = *pLayer;


	//if (lnst->fieldLIND.LINM == "CHRVDEL2")
	//	int ff = 0;

	for (int i = 0; i < lnst->m_vecFieldLvct.size(); i++)
	{
		instStr = lnst->m_vecFieldLvct[i]->VECD;
		QStringList lisInstStr = instStr.split(';');
		for (int j = 0; j < lisInstStr.size(); j++)
		{
			command = lisInstStr[j].left(2);
			arguments = lisInstStr[j].mid(2, lisInstStr[j].size() - 2);
			if (command == "SP")
			{
				// 根据指令字符，获取颜色名
				for (int k = 0; k < lnst->m_fieldLcrf.vecSubFieldLcrf.size(); k++)
				{
					if (arguments[0] == lnst->m_fieldLcrf.vecSubFieldLcrf[k]->CIDX)
					{
						strColorName = lnst->m_fieldLcrf.vecSubFieldLcrf[k]->CTOK;
					}
				}
				if (initColor)
				{
					if (pLayer->strColorName != strColorName)
					{
						pLayer = new S52Layer;
						lnst->m_vecS52Layers.push_back(pLayer);
					}
				}

				pLayer->strColorName = strColorName;
				pLayer->nPenWidth = oldLayer.nPenWidth;
				pLayer->uTransparent = oldLayer.uTransparent;
				oldLayer = *pLayer;
				initColor = true;
				continue;
			}

			if ("SW" == command)								// SW		
			{
				int width = arguments.toUShort();
				if (initWidth)
				{
					if (width != pLayer->nPenWidth)
					{
						pLayer = new S52Layer;
						lnst->m_vecS52Layers.push_back(pLayer);
					}
				}
				pLayer->nPenWidth = width;
				pLayer->strColorName = oldLayer.strColorName;
				pLayer->uTransparent = oldLayer.uTransparent;
				oldLayer = *pLayer;
				initWidth = true;
				continue;
			}

			if ("ST" == command)								// ST
			{
				unsigned short trans = arguments.toUShort();
				if (initTrans)
				{
					if (pLayer->uTransparent != trans)
					{
						pLayer = new S52Layer;
						lnst->m_vecS52Layers.push_back(pLayer);
					}
				}
				pLayer->uTransparent = trans;
				pLayer->strColorName = oldLayer.strColorName;
				pLayer->nPenWidth = oldLayer.nPenWidth;
				oldLayer = *pLayer;
				initTrans = true;
				continue;
			}

			if ("PU" == command)
			{
				curPoint.setX(arguments.split(',')[0].toShort());
				curPoint.setY(arguments.split(',')[1].toShort());
				penDown = false;
				continue;
			}

			if ("PD" == command)
			{
				if (arguments.size() == 0)  // 落笔无坐标，在当前点绘制0.3mm的点
				{
					pCircle = new S52Circle;
					pLayer->vecCircle.push_back(pCircle);
					pCircle->cenPoint = curPoint;
					pCircle->radius = 15;
					pCircle->modeFill = 2;
					continue;
				}

				if (!penDown)  // 当前笔状态为抬笔
				{
					pPoint = new QPoint;
					pPoint->setX(curPoint.x());
					pPoint->setY(curPoint.y());
					if (modePolygon == eModePolygon::PM2)
					{
						pLine = new S52Line;
						pLine->vecLine.push_back(pPoint);
						pLayer->vecLines.push_back(pLine);
					}
					else
					{
						pPolygon = new S52Polygon;
						pPolygon->vecLine.push_back(pPoint);
						pLayer->vecPolygon.push_back(pPolygon);
					}
				}

				QStringList lisArgument = arguments.split(',');
				for (int k = 0; k < lisArgument.size(); k += 2)
				{
					pPoint = new QPoint;
					pPoint->setX(lisArgument[k].toShort());
					pPoint->setY(lisArgument[k + 1].toShort());
					if (modePolygon != eModePolygon::PM2)
						pPolygon->vecLine.push_back(pPoint);
					else
						pLine->vecLine.push_back(pPoint);
				}
				curPoint = *pPoint;
				penDown = true;
				continue;
			}

			if ("CI" == command)
			{
				pCircle = new S52Circle;
				pLayer->vecCircle.push_back(pCircle);
				pCircle->cenPoint = curPoint;
				pCircle->radius = arguments.toShort();
				continue;
			}

			// PM指令将命令解释器置于多边形定义模式。在这种模式下，您可以使用其他指令（PU，PD，CI）来构造多边形。这些说明存储在
			// 多边形缓冲区在完全定义多边形之前，它们不会执行。为了绘制多边形，必须使用FP指令填充和 / 或使用EP指令对其进行轮廓显示。界定
			// 多边形将笔移动到所需的起始位置（请参见PU或PD）。然后执行PM 0进入多边形模式，并指定适当的指令以定义多边形的形状。如果你
			// 想要定义一个子多边形，以PM 1指令结束形状并定义下一个形状；执行PM 2退出多边形模式。 PM 0之前的当前笔位置是多边形的第一个点（顶点）。
			// 可以使用笔向上或向下定义顶点（请参见PU，PD）。但是，“如果要使用EP指令勾勒多边形，请注意，EP只会绘制那些用笔向下定义的点”。
			// 另一方面，FP指令填充多边形，而与笔的上 / 下状态无关。
			if ("PM" == command)
			{
				penDown = false;
				int mode = arguments.toShort();
				switch (mode)
				{
				case 0:
					modePolygon = eModePolygon::PM0;
					break;
				case 1:
					modePolygon = eModePolygon::PM1;
					break;
				case 2:
					modePolygon = eModePolygon::PM2;
					break;
				}
			}

			if ("SC" == command)
			{
				S52InstSc* pInstSC = new S52InstSc;
				lnst->m_vecInstSC.push_back(pInstSC);
				pInstSC->piovtPoint = curPoint;
				pInstSC->symbName = arguments.left(8);
				pInstSC->orient = arguments.mid(9).toUShort();
				// SC指令调用另一个符号定义。 方向指定被调用的符号是
				// 垂直绘制（orientation = 0）还是旋转到最后一个笔移动
				// 指令的方向（orientation = 1），还是以90度旋转到符号
				// 位置的符号边缘的切线（ 方向 = 2）。 符号的枢轴点将
				// 放置在当前笔位置上。 SC指令完成后，将恢复原始的笔
				// 位置和状态（上下笔）。
				switch (pInstSC->orient)
				{
				case 1:
					pInstSC->orient = 90;
					break;
				case 2:
					pInstSC->orient = 0;
					break;
				default:
					break;
				}
			}

			// FP指令填充先前放置在多边形缓冲区中的多边形（请参阅PM）。 
			// 使用当前的笔颜色（请参见SP）和透明度（请参见ST）填充多边形。 
			// FP指令完成后，将恢复原始笔的位置和状态（上下笔）。
			if ("FP" == command)
			{
				if (pPolygon != nullptr)
					pPolygon->modeFill = 1;
				if (pCircle != nullptr)
					pCircle->modeFill = 1;

			}
			if ("EP" == command){}
		}
	}
	return true;
}

void CS52SymbolData::clear()
{
	QMap<QString, LNST*>::iterator lniter;
	S52SubLcrfField* pSubLcrf;
	S52LvctField* pLvctField;
	for (lniter = m_mapLnst.begin(); lniter != m_mapLnst.end(); lniter++)
	{
		LNST* pLnst = lniter.value();
		if (pLnst != nullptr)
		{
			for (int i = 0; i < pLnst->m_fieldLcrf.vecSubFieldLcrf.size(); i++)
			{
				if (pLnst->m_fieldLcrf.vecSubFieldLcrf[i] != nullptr)
				{
					pSubLcrf = pLnst->m_fieldLcrf.vecSubFieldLcrf[i];
					delete pSubLcrf;
					pSubLcrf = nullptr;
				}
			}
			pLnst->m_fieldLcrf.vecSubFieldLcrf.clear();

			for (int j = 0; j < pLnst->m_vecFieldLvct.size(); j++)
			{
				if (pLnst->m_vecFieldLvct[j] != nullptr)
				{
					pLvctField = pLnst->m_vecFieldLvct[j];
					delete pLvctField;
					pLvctField = nullptr;
				}
			}
			pLnst->m_vecFieldLvct.clear();

			for (int i = 0; i < pLnst->m_vecS52Layers.size(); i++)
			{
				if (pLnst->m_vecS52Layers[i] != nullptr)
				{
					for (int j = 0; j < pLnst->m_vecS52Layers[i]->vecCircle.size(); j++)
					{
						if (pLnst->m_vecS52Layers[i]->vecCircle[j] != nullptr)
							delete pLnst->m_vecS52Layers[i]->vecCircle[j];
						pLnst->m_vecS52Layers[i]->vecCircle[j] = nullptr;
					}
					pLnst->m_vecS52Layers[i]->vecCircle.clear();

					for (int j = 0; j < pLnst->m_vecS52Layers[i]->vecLines.size(); j++)
					{
						if (pLnst->m_vecS52Layers[i]->vecLines[j] != nullptr)
						{
							for (int k = 0; k < pLnst->m_vecS52Layers[i]->vecLines[j]->vecLine.size(); k++)
							{
								if (pLnst->m_vecS52Layers[i]->vecLines[j]->vecLine[k] != nullptr)
									delete pLnst->m_vecS52Layers[i]->vecLines[j]->vecLine[k];
								pLnst->m_vecS52Layers[i]->vecLines[j]->vecLine[k] = nullptr;
							}
							pLnst->m_vecS52Layers[i]->vecLines[j]->vecLine.clear();
							delete pLnst->m_vecS52Layers[i]->vecLines[j];
							pLnst->m_vecS52Layers[i]->vecLines[j] = nullptr;
						}
					}
					pLnst->m_vecS52Layers[i]->vecLines.clear();

					for (int j = 0; j < pLnst->m_vecS52Layers[i]->vecPolygon.size(); j++)
					{
						if (pLnst->m_vecS52Layers[i]->vecPolygon[j] != nullptr)
						{
							for (int k = 0; k < pLnst->m_vecS52Layers[i]->vecPolygon[j]->vecLine.size(); k++)
							{
								if (pLnst->m_vecS52Layers[i]->vecPolygon[j]->vecLine[k] != nullptr)
									delete pLnst->m_vecS52Layers[i]->vecPolygon[j]->vecLine[k];
								pLnst->m_vecS52Layers[i]->vecPolygon[j]->vecLine[k] = nullptr;
							}
							pLnst->m_vecS52Layers[i]->vecPolygon[j]->vecLine.clear();
							delete pLnst->m_vecS52Layers[i]->vecPolygon[j];
							pLnst->m_vecS52Layers[i]->vecPolygon[j] = nullptr;
						}
					}
					pLnst->m_vecS52Layers[i]->vecPolygon.clear();

					delete pLnst->m_vecS52Layers[i];
					pLnst->m_vecS52Layers[i] = nullptr;
				}
			}
			pLnst->m_vecS52Layers.clear();

			for (int i = 0; i < pLnst->m_vecInstSC.size(); i++)
			{
				if (pLnst->m_vecInstSC[i] != nullptr)
				{
					delete pLnst->m_vecInstSC[i];
					pLnst->m_vecInstSC[i] = nullptr;
				}
			}
			pLnst->m_vecInstSC.clear();
			delete pLnst;
			pLnst = nullptr;
		}
	}
	m_mapLnst.clear();

	QMap<QString, PATT*>::iterator paiter;
	for (paiter = m_mapPatt.begin(); paiter != m_mapPatt.end(); paiter++)
	{
		PATT* pPatt = paiter.value();
		if (pPatt != nullptr)
		{
			for (int i = 0; i < pPatt->m_fieldPcrf.vecSubfieldPcrf.size(); i++)
			{
				if (pPatt->m_fieldPcrf.vecSubfieldPcrf[i] != nullptr)
				{
					delete pPatt->m_fieldPcrf.vecSubfieldPcrf[i];
					pPatt->m_fieldPcrf.vecSubfieldPcrf[i] = nullptr;
				}
			}
			pPatt->m_fieldPcrf.vecSubfieldPcrf.clear();

			for (int j = 0; j < pPatt->m_vecFieldPvct.size(); j++)
			{
				if (pPatt->m_vecFieldPvct[j] != nullptr)
				{
					delete pPatt->m_vecFieldPvct[j];
					pPatt->m_vecFieldPvct[j] = nullptr;
				}
			}
			pPatt->m_vecFieldPvct.clear();

			for (int i = 0; i < pPatt->m_vecS52Layers.size(); i++)
			{
				if (pPatt->m_vecS52Layers[i] != nullptr)
				{
					for (int j = 0; j < pPatt->m_vecS52Layers[i]->vecCircle.size(); j++)
					{
						if (pPatt->m_vecS52Layers[i]->vecCircle[j] != nullptr)
							delete pPatt->m_vecS52Layers[i]->vecCircle[j];
						pPatt->m_vecS52Layers[i]->vecCircle[j] = nullptr;
					}
					pPatt->m_vecS52Layers[i]->vecCircle.clear();

					for (int j = 0; j < pPatt->m_vecS52Layers[i]->vecLines.size(); j++)
					{
						if (pPatt->m_vecS52Layers[i]->vecLines[j] != nullptr)
						{
							for (int k = 0; k < pPatt->m_vecS52Layers[i]->vecLines[j]->vecLine.size(); k++)
							{
								if (pPatt->m_vecS52Layers[i]->vecLines[j]->vecLine[k] != nullptr)
									delete pPatt->m_vecS52Layers[i]->vecLines[j]->vecLine[k];
								pPatt->m_vecS52Layers[i]->vecLines[j]->vecLine[k] = nullptr;
							}
							pPatt->m_vecS52Layers[i]->vecLines[j]->vecLine.clear();
							delete pPatt->m_vecS52Layers[i]->vecLines[j];
							pPatt->m_vecS52Layers[i]->vecLines[j] = nullptr;
						}
					}
					pPatt->m_vecS52Layers[i]->vecLines.clear();

					for (int j = 0; j < pPatt->m_vecS52Layers[i]->vecPolygon.size(); j++)
					{
						if (pPatt->m_vecS52Layers[i]->vecPolygon[j] != nullptr)
						{
							for (int k = 0; k < pPatt->m_vecS52Layers[i]->vecPolygon[j]->vecLine.size(); k++)
							{
								if (pPatt->m_vecS52Layers[i]->vecPolygon[j]->vecLine[k] != nullptr)
									delete pPatt->m_vecS52Layers[i]->vecPolygon[j]->vecLine[k];
								pPatt->m_vecS52Layers[i]->vecPolygon[j]->vecLine[k] = nullptr;
							}
							pPatt->m_vecS52Layers[i]->vecPolygon[j]->vecLine.clear();
							delete pPatt->m_vecS52Layers[i]->vecPolygon[j];
							pPatt->m_vecS52Layers[i]->vecPolygon[j] = nullptr;
						}
					}
					pPatt->m_vecS52Layers[i]->vecPolygon.clear();

					delete pPatt->m_vecS52Layers[i];
					pPatt->m_vecS52Layers[i] = nullptr;
				}
			}
			pPatt->m_vecS52Layers.clear();
			delete pPatt;
			pPatt = nullptr;
		}
	}
	m_mapPatt.clear();

	QMap<QString, SYMB*>::iterator syiter;
	for (syiter = m_mapSymb.begin(); syiter != m_mapSymb.end(); syiter++)
	{
		SYMB* pSymb = syiter.value();
		if (pSymb != nullptr)
		{
			for (int i = 0; i < pSymb->m_fieldScrf.vecSubFieldScrf.size(); i++)
			{
				if (pSymb->m_fieldScrf.vecSubFieldScrf[i] != nullptr)
				{
					delete pSymb->m_fieldScrf.vecSubFieldScrf[i];
					pSymb->m_fieldScrf.vecSubFieldScrf[i] = nullptr;
				}
			}
			pSymb->m_fieldScrf.vecSubFieldScrf.clear();

			for (int j = 0; j < pSymb->m_vecFieldSvct.size(); j++)
			{
				if (pSymb->m_vecFieldSvct[j] != nullptr)
				{
					delete pSymb->m_vecFieldSvct[j];
					pSymb->m_vecFieldSvct[j] = nullptr;
				}
			}
			pSymb->m_vecFieldSvct.clear();

			for (int i = 0; i < pSymb->m_vecS52Layers.size(); i++)
			{
				if (pSymb->m_vecS52Layers[i] != nullptr)
				{
					for (int j = 0; j < pSymb->m_vecS52Layers[i]->vecCircle.size(); j++)
					{
						if (pSymb->m_vecS52Layers[i]->vecCircle[j] != nullptr)
							delete pSymb->m_vecS52Layers[i]->vecCircle[j];
						pSymb->m_vecS52Layers[i]->vecCircle[j] = nullptr;
					}
					pSymb->m_vecS52Layers[i]->vecCircle.clear();

					for (int j = 0; j < pSymb->m_vecS52Layers[i]->vecLines.size(); j++)
					{
						if (pSymb->m_vecS52Layers[i]->vecLines[j] != nullptr)
						{
							for (int k = 0; k < pSymb->m_vecS52Layers[i]->vecLines[j]->vecLine.size(); k++)
							{
								if (pSymb->m_vecS52Layers[i]->vecLines[j]->vecLine[k] != nullptr)
									delete pSymb->m_vecS52Layers[i]->vecLines[j]->vecLine[k];
								pSymb->m_vecS52Layers[i]->vecLines[j]->vecLine[k] = nullptr;
							}
							pSymb->m_vecS52Layers[i]->vecLines[j]->vecLine.clear();
							delete pSymb->m_vecS52Layers[i]->vecLines[j];
							pSymb->m_vecS52Layers[i]->vecLines[j] = nullptr;
						}
					}
					pSymb->m_vecS52Layers[i]->vecLines.clear();

					for (int j = 0; j < pSymb->m_vecS52Layers[i]->vecPolygon.size(); j++)
					{
						if (pSymb->m_vecS52Layers[i]->vecPolygon[j] != nullptr)
						{
							for (int k = 0; k < pSymb->m_vecS52Layers[i]->vecPolygon[j]->vecLine.size(); k++)
							{
								if (pSymb->m_vecS52Layers[i]->vecPolygon[j]->vecLine[k] != nullptr)
									delete pSymb->m_vecS52Layers[i]->vecPolygon[j]->vecLine[k];
								pSymb->m_vecS52Layers[i]->vecPolygon[j]->vecLine[k] = nullptr;
							}
							pSymb->m_vecS52Layers[i]->vecPolygon[j]->vecLine.clear();
							delete pSymb->m_vecS52Layers[i]->vecPolygon[j];
							pSymb->m_vecS52Layers[i]->vecPolygon[j] = nullptr;
						}
					}
					pSymb->m_vecS52Layers[i]->vecPolygon.clear();

					delete pSymb->m_vecS52Layers[i];
					pSymb->m_vecS52Layers[i] = nullptr;
				}
			}
			pSymb->m_vecS52Layers.clear();
			delete pSymb;
		}
		pSymb = nullptr;
	}
	m_mapSymb.clear();
}

bool CS52SymbolData::readAsciiFile(const QString& fileName)
{
	char nDelimite = '\x1F';	// 分隔符
	QString info;
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		info = fileName + "文件打开失败，退出！";
		//QMessageBox::information(nullptr, "提示", info);
		return false;
	}

	QString line;
	quint16 num = 0;
	unsigned short nNumChar;
	QString str;

	// Symbol Module
	S52SymdField fieldSYMD;					bool bSymd = false;
	S52SxpoField fieldSXPO;					bool bSxpo = false;
	S52ScrfField fieldSCRF;					bool bScrf = false;
	QVector<S52SvctField*> vecFieldSVCT;		bool bSvct = false;

	// Pattern Module
	S52PatdField fieldPATD;					bool bPatd = false;
	S52PxpoField fieldPXPO;					bool bPxpo = false;
	S52PcrfField fieldPCRF;					bool bPcrf = false;
	QVector<S52PvctField*> vecFieldPVCT;	bool bPvct = false;

	// Linestyle Module
	S52LindField fieldLIND;					bool bLind = false;
	S52LxpoField fieldLXPO;					bool bLxpo = false;
	S52LcrfField fieldLCRF;					bool bLcrf = false;
	QVector<S52LvctField*> vecLVCT;			bool bLvct = false;

	//int kkkk = 0;

	while (!file.atEnd())
	{
		line = file.readLine();
		num++;
		if (line.isNull())
		{
			info = fileName + "文件中第" + QString("%1").arg(num) + "行是空行，跳过！";
			//QMessageBox::information(nullptr, "提示", info);
			continue;
		}

		str = line.left(4);
		if (str == "0001" ||
			str == "SYMB" ||
			str == "PATT" ||
			str == "LNST")
			continue;

		if (str == "****" &&
			bSymd == true &&
			bSxpo == true &&
			bScrf == true &&
			bSvct == true)
		{
			bSymd = false;
			bSxpo = false;
			bScrf = false;
			bSvct = false;
			SYMB* pSymb = new SYMB();
			pSymb->m_fieldSymd = fieldSYMD;
			pSymb->m_fieldScrf = fieldSCRF;
			pSymb->m_fieldSxpo = fieldSXPO;
			pSymb->m_vecFieldSvct = vecFieldSVCT;
			m_mapSymb.insert(fieldSYMD.SYNM, pSymb);

			fieldSCRF.vecSubFieldScrf.clear();
			vecFieldSVCT.clear();
			continue;
		}

		if (str == "****" &&
			bPatd == true &&
			bPxpo == true &&
			bPcrf == true &&
			bPvct == true)
		{
			bPatd = false;
			bPxpo = false;
			bPcrf = false;
			bPvct = false;
			PATT* pPatt = new PATT();
			pPatt->m_fieldPatd = fieldPATD;
			pPatt->m_fieldPcrf = fieldPCRF;
			pPatt->m_fieldPxpo = fieldPXPO;
			pPatt->m_vecFieldPvct = vecFieldPVCT;
			m_mapPatt.insert(fieldPATD.PANM, pPatt);

			fieldPCRF.vecSubfieldPcrf.clear();
			vecFieldPVCT.clear();
			continue;
		}

		if (str == "****" &&
			bLind == true &&
			bLcrf == true &&
			bLxpo == true &&
			bLvct == true)
		{
			bLind = false;
			bLcrf = false;
			bLxpo = false;
			bLvct = false;
			LNST* pLnst = new LNST;
			pLnst->m_fieldLind = fieldLIND;
			pLnst->m_fieldLcrf = fieldLCRF;
			pLnst->m_fieldLxpo = fieldLXPO;
			pLnst->m_vecFieldLvct = vecLVCT;
			m_mapLnst.insert(fieldLIND.LINM, pLnst);


			fieldLCRF.vecSubFieldLcrf.clear();
			vecLVCT.clear();
			continue;
		}

		nNumChar = line.mid(4, 5).toUShort();
		line = line.mid(9);

		// Symbol Module BEGIN
		// Symbol Module SYMD字段
		if (str == "SYMD")
		{

			if (bSymd)
			{
				info = fileName + "文件中第" + QString("%1").arg(num) + "格式错，跳过！";
				//QMessageBox::information(nullptr, "提示", info);
				continue;
			}
			//kkkk++;
			bSymd = true;
			fieldSYMD.SYNM = line.left(8);
			fieldSYMD.SYDF = line.at(8).toLatin1();
			fieldSYMD.SYCL = line.mid(9, 5).toShort();
			fieldSYMD.SYRW = line.mid(14, 5).toShort();
			fieldSYMD.SYHL = line.mid(19, 5).toUShort();
			fieldSYMD.SYVL = line.mid(24, 5).toUShort();
			fieldSYMD.SBXC = line.mid(29, 5).toUShort();
			fieldSYMD.SBXR = line.mid(34, 5).toUShort();
			continue;
		}

		// Symbol Module SXPO字段
		if (str == "SXPO")
		{
			if (bSxpo)
			{
				info = fileName + "文件中第" + QString("%1").arg(num) + "格式错，跳过！";
				//QMessageBox::information(nullptr, "提示", info);
				continue;
			}
			bSxpo = true;
			fieldSXPO.EXPT = line.left(nNumChar - 1);
			continue;
		}

		// Symbol Module SCRF字段
		if (str == "SCRF")
		{
			if (bScrf)
			{
				info = fileName + "文件中第" + QString("%1").arg(num) + "格式错，跳过！";
				//QMessageBox::information(nullptr, "提示", info);
				continue;
			}
			bScrf = true;
			for (int i = 0; i < nNumChar / 6; i++)
			{
				S52SubScrfField* pSubScrf = new S52SubScrfField();
				pSubScrf->CIDX = line.at(i * 6).toLatin1();
				pSubScrf->CTOK = line.mid(i * 6 + 1, 5);
				fieldSCRF.vecSubFieldScrf.push_back(pSubScrf);
			}
			continue;
		}

		// Symbol Module SVCT字段
		if (str == "SVCT")
		{
			bSvct = true;
			S52SvctField* pSvctField = new S52SvctField;
			pSvctField->VECD = line.left(nNumChar - 1);
			vecFieldSVCT.push_back(pSvctField);
			continue;
		}
		//  Symbol Module END

		// Pattern Module BEGIN
		// Pattern Module PATD字段
		if (str == "PATD")
		{

			if (bPatd)
			{
				info = fileName + "文件中第" + QString("%1").arg(num) + "格式错，跳过！";
				//QMessageBox::information(nullptr, "提示", info);
				continue;
			}
			bPatd = true;
			fieldPATD.PANM = line.left(8);
			fieldPATD.PADF = line.at(8).toLatin1();
			fieldPATD.PATP = line.mid(9, 3);
			fieldPATD.PASP = line.mid(12, 3);
			fieldPATD.PAMI = line.mid(15, 5).toUShort();
			fieldPATD.PAMA = line.mid(20, 5).toUShort();
			fieldPATD.PACL = line.mid(25, 5).toShort();	// pivot x;
			fieldPATD.PARW = line.mid(30, 5).toShort();	// pivot y;
			fieldPATD.PAHL = line.mid(35, 5).toUShort();	// box width
			fieldPATD.PAVL = line.mid(40, 5).toUShort();	// box height
			fieldPATD.PBXC = line.mid(45, 5).toUShort();	// upper left x
			fieldPATD.PBXR = line.mid(50, 5).toUShort();	// upper left y;

			int leftX = fieldPATD.PACL - fieldPATD.PBXC;
			int rightX = fieldPATD.PACL - (fieldPATD.PBXC + fieldPATD.PAHL);
			int uperY = fieldPATD.PARW - fieldPATD.PBXR;
			int downY = fieldPATD.PARW - (fieldPATD.PBXR + fieldPATD.PAVL);
			int minDistX, maxDistX, minDistY, maxIistY;
			continue;
		}

		// Pattern Module PXPO字段
		if (str == "PXPO")
		{
			if (bPxpo)
			{
				info = fileName + "文件中第" + QString("%1").arg(num) + "格式错，跳过！";
				//QMessageBox::information(nullptr, "提示", info);
				continue;
			}
			bPxpo = true;
			fieldPXPO.EXPT = line.left(nNumChar - 1);
			continue;
		}

		// Pattern Module PCRF字段
		if (str == "PCRF")
		{
			if (bPcrf)
			{
				info = fileName + "文件中第" + QString("%1").arg(num) + "格式错，跳过！";
				//QMessageBox::information(nullptr, "提示", info);
				continue;
			}
			bPcrf = true;
			for (int i = 0; i < nNumChar / 6; i++)
			{
				S52SubPcrfField* pSubPcrf = new S52SubPcrfField();
				pSubPcrf->CIDX = line.at(i * 6).toLatin1();
				pSubPcrf->CTOK = line.mid(i * 6 + 1, 5);
				fieldPCRF.vecSubfieldPcrf.push_back(pSubPcrf);
			}
			continue;
		}

		// Pattern Module PVCT字段
		if (str == "PVCT")
		{
			bPvct = true;
			S52PvctField* pPvctField = new S52PvctField();
			pPvctField->VECD = line.left(nNumChar - 1);
			vecFieldPVCT.push_back(pPvctField);
			continue;
		}
		// Pattern Module END
		// Linestyle Module BEGIN
		// Linestyle Module LIND字段
		if (str == "LIND")
		{

			if (bLind)
			{
				info = fileName + "文件中第" + QString("%1").arg(num) + "格式错，跳过！";
				//QMessageBox::information(nullptr, "提示", info);
				continue;
			}
			bLind = true;
			fieldLIND.LINM = line.left(8);
			fieldLIND.LICL = line.mid(8, 5).toShort();
			fieldLIND.LIRW = line.mid(13, 5).toShort();
			fieldLIND.LIHL = line.mid(18, 5).toUShort();
			fieldLIND.LIVL = line.mid(23, 5).toUShort();
			fieldLIND.LBXC = line.mid(28, 5).toUShort();
			fieldLIND.LBXR = line.mid(33, 5).toUShort();
			continue;
		}

		// Linestyle Module LXPO字段
		if (str == "LXPO")
		{
			if (bLxpo)
			{
				info = fileName + "文件中第" + QString("%1").arg(num) + "格式错，跳过！";
				//QMessageBox::information(nullptr, "提示", info);
				continue;
			}
			bLxpo = true;
			fieldLXPO.EXPT = line.left(nNumChar - 1);
			continue;
		}

		// Linestyle Module LCRF字段
		if (str == "LCRF")
		{
			if (bLcrf)
			{
				info = fileName + "文件中第" + QString("%1").arg(num) + "格式错，跳过！";
				//QMessageBox::information(nullptr, "提示", info);
				continue;
			}
			bLcrf = true;
			for (int i = 0; i < nNumChar / 6; i++)
			{
				S52SubLcrfField* pSubLcrf = new S52SubLcrfField;
				pSubLcrf->CIDX = line.at(i * 6).toLatin1();
				pSubLcrf->CTOK = line.mid(i * 6 + 1, 5);
				fieldLCRF.vecSubFieldLcrf.push_back(pSubLcrf);
			}
			continue;
		}

		// Linestyle Module LVCT字段
		if (str == "LVCT")
		{
			bLvct = true;
			S52LvctField* pPvctField = new S52LvctField();
			pPvctField->VECD = line.left(nNumChar - 1);
			vecLVCT.push_back(pPvctField);
			continue;
		}
		// Linestyle Module END
	}
	file.close();

	parseMap();

	return true;
}

bool CS52SymbolData::parseMap()
{
	QMap<QString, LNST*>::iterator iterLNST;
	for (iterLNST = m_mapLnst.begin(); iterLNST != m_mapLnst.end(); iterLNST++)
	{
		LNST* pLNST = iterLNST.value();
		parseVECD(pLNST);
	}

	QMap<QString, PATT*>::iterator iterPATT;
	for (iterPATT = m_mapPatt.begin(); iterPATT != m_mapPatt.end(); iterPATT++)
	{
		PATT* pPATT = iterPATT.value();
		parseVECD(pPATT);
	}

	QMap<QString, SYMB*>::iterator iterSYMB;
	for (iterSYMB = m_mapSymb.begin(); iterSYMB != m_mapSymb.end(); iterSYMB++)
	{
		SYMB* pSYMB = iterSYMB.value();
		parseVECD(pSYMB);
	}
	return true;
}
