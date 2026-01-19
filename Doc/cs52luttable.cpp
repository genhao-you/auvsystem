#include "stdafx.h"
#include "cs52luttable.h"
#include <QFile>
#include <QMessageBox>
#include <QApplication>
#include <QDebug>

using namespace Doc;
using namespace std;
#pragma execution_character_set("utf-8")
CS52LutTable::CS52LutTable()
	: m_strLutTabPath(QApplication::applicationDirPath() + "/s57Data/s52lookup.dai")
{
	readAsciiFile(m_strLutTabPath);
}

CS52LutTable::~CS52LutTable()
{
	clear();
}

LUPT* CS52LutTable::serachLupt(const QString& objlName, const QStringList& lstAttrCombin, eLookupTabType tabType)
{
	switch (tabType)
	{
	case eLookupTabType::PLAIN_BOUNDARIES:
		return serachLupt(objlName, lstAttrCombin, m_vecLutPlainBoundaries);
		break;
	case eLookupTabType::SYMBOLIZED_BOUNDARIES:
		return serachLupt(objlName, lstAttrCombin, m_vecLutSimplifiedBoundaries);
		break;
	case eLookupTabType::SIMPLIFIED:
		return serachLupt(objlName, lstAttrCombin, m_vecLutSimplified);
		break;
	case eLookupTabType::PAPER_CHART:
		return serachLupt(objlName, lstAttrCombin, m_vecLutPaperChart);
		break;
	case eLookupTabType::LINES:
		return serachLupt(objlName, lstAttrCombin, m_vecLutLines);
		break;
	}
	return nullptr;
}

LUPT* CS52LutTable::serachLupt(const QString& objlName, const QStringList& lstAttrCombin, QVector<LUPT*>& vLupt)
{
	LUPT* curLupt = nullptr;
	short loc = -1;
	unsigned short sizeAttrCombinOfLut;
	unsigned short sizeAttrCombinOfSource;
	bool bMatch;
	bool bMatchObjName = false;
	QString strAtvlLut, strAtvlSource;
	for (int i = 0; i < vLupt.size(); i++)
	{
		if (vLupt[i]->fieldLUPT.OBCL == "######")	// 当在该查找表中找不到该物标时，则为未知物标显示
		{
			curLupt = vLupt[i];
			continue;
		}
		if (vLupt[i]->fieldLUPT.OBCL != objlName)	// 该查找域物标名不匹配,如果已匹配过，则中断对检索
		{
			if (bMatchObjName)
				break;
			else
				continue;
		}
		bMatchObjName = true;

		sizeAttrCombinOfLut = vLupt[i]->fieldATTC.lstCombin.size();
		sizeAttrCombinOfSource = lstAttrCombin.size();

		if (sizeAttrCombinOfLut == 0) // 查找域中物标名相同，但属性组合为空，则该域为默认
		{
			curLupt = vLupt[i];
			continue;
		}

		if (sizeAttrCombinOfLut > sizeAttrCombinOfSource) // 查找表中属性组合数大于源属性组合数,不匹配
		{
			continue;
		}

		bMatch = true;
		int num = 0;
		int sizeCombin = vLupt[i]->fieldATTC.lstCombin.size();
		for (int j = 0; j < sizeCombin; j++) // 查找表中属遍历
		{
			QString strAttr = vLupt[i]->fieldATTC.lstCombin.at(j);
			loc = -1;
			for (int k = 0;k < lstAttrCombin.size();k++)
			{
				if (lstAttrCombin.at(k).contains(strAttr.left(6)))
				{
					loc = k;
					break;
				}
			}

			if (loc < 0)  // 源属性组合中不含该属性名,属性组合不匹配
			{
				bMatch = false;
				break;
			}

			if (vLupt[i]->fieldATTC.lstCombin[j].size() == 6) // 该属性无需属性值匹配
			{
				num++;
				continue;
			}

			strAtvlSource = lstAttrCombin[loc].mid(6);
			strAtvlLut = vLupt[i]->fieldATTC.lstCombin[j].mid(6);

			if (strAtvlSource==strAtvlLut)	// 属性值也匹配
			{
				num++;
				continue;
			}	

			if (strAtvlLut == "?"&& strAtvlSource.isEmpty())	// 查换表中属性值不是占位符号，不能匹配
			{
				num++;
				continue;
			}
			bMatch = false;
			break;
		}
 		if (bMatch&&num == sizeCombin)		// modify by wb 2021.04.29
 		{
 			curLupt = vLupt[i];
			if (num != 0)
				break;
			//break;
 		}

	}
	return curLupt;
}

void CS52LutTable::clear()
{
	clear(m_vecLutPlainBoundaries);
	clear(m_vecLutSimplifiedBoundaries);
	clear(m_vecLutSimplified);
	clear(m_vecLutPaperChart);
	clear(m_vecLutLines);
}

void CS52LutTable::clear(QVector<LUPT*>& vLupt)
{
	for (int i = 0; i < vLupt.size(); i++)
	{
		if (vLupt[i] != nullptr)
		{
			// 析构fieldINST.vecAC
			for (int j = 0; j < vLupt[i]->fieldINST.vecAC.size(); j++)
			{
				if (vLupt[i]->fieldINST.vecAC[j] != nullptr)
				{
					delete vLupt[i]->fieldINST.vecAC[j];
					vLupt[i]->fieldINST.vecAC[j] = nullptr;
				}
			}
			vLupt[i]->fieldINST.vecAC.clear();

			// 析构fieldINST.vecAP
			for (int j = 0; j < vLupt[i]->fieldINST.vecAP.size(); j++)
			{
				if (vLupt[i]->fieldINST.vecAP[j] != nullptr)
				{
					delete vLupt[i]->fieldINST.vecAP[j];
					vLupt[i]->fieldINST.vecAP[j] = nullptr;
				}
			}
			vLupt[i]->fieldINST.vecAP.clear();

			// 析构fieldINST.vecCA
			for (int j = 0; j < vLupt[i]->fieldINST.vecCA.size(); j++)
			{
				if (vLupt[i]->fieldINST.vecCA[j] != nullptr)
				{
					delete vLupt[i]->fieldINST.vecCA[j];
					vLupt[i]->fieldINST.vecCA[j] = nullptr;
				}
			}
			vLupt[i]->fieldINST.vecCA.clear();

			// 析构fieldINST.vecCS
			for (int j = 0; j < vLupt[i]->fieldINST.vecCS.size(); j++)
			{
				if (vLupt[i]->fieldINST.vecCS[j] != nullptr)
				{
					delete vLupt[i]->fieldINST.vecCS[j];
					vLupt[i]->fieldINST.vecCS[j] = nullptr;
				}
			}
			vLupt[i]->fieldINST.vecCS.clear();

			// 析构fieldINST.vecLC
			for (int j = 0; j < vLupt[i]->fieldINST.vecLC.size(); j++)
			{
				if (vLupt[i]->fieldINST.vecLC[j] != nullptr)
				{
					delete vLupt[i]->fieldINST.vecLC[j];
					vLupt[i]->fieldINST.vecLC[j] = nullptr;
				}
			}
			vLupt[i]->fieldINST.vecLC.clear();

			// 析构fieldINST.vecLS
			for (int j = 0; j < vLupt[i]->fieldINST.vecLS.size(); j++)
			{
				if (vLupt[i]->fieldINST.vecLS[j] != nullptr)
				{
					delete vLupt[i]->fieldINST.vecLS[j];
					vLupt[i]->fieldINST.vecLS[j] = nullptr;
				}
			}
			vLupt[i]->fieldINST.vecLS.clear();

			// 析构fieldINST.vecSY
			for (int j = 0; j < vLupt[i]->fieldINST.vecSY.size(); j++)
			{
				if (vLupt[i]->fieldINST.vecSY[j] != nullptr)
				{
					delete vLupt[i]->fieldINST.vecSY[j];
					vLupt[i]->fieldINST.vecSY[j] = nullptr;
				}
			}
			vLupt[i]->fieldINST.vecSY.clear();

			// 析构fieldINST.vecTE
			for (int j = 0; j < vLupt[i]->fieldINST.vecTE.size(); j++)
			{
				if (vLupt[i]->fieldINST.vecTE[j] != nullptr)
				{
					delete vLupt[i]->fieldINST.vecTE[j];
					vLupt[i]->fieldINST.vecTE[j] = nullptr;
				}
			}
			vLupt[i]->fieldINST.vecTE.clear();

			// 析构fieldINST.vTX
			for (int j = 0; j < vLupt[i]->fieldINST.vecTX.size(); j++)
			{
				if (vLupt[i]->fieldINST.vecTX[j] != nullptr)
				{
					delete vLupt[i]->fieldINST.vecTX[j];
					vLupt[i]->fieldINST.vecTX[j] = nullptr;
				}
			}
			vLupt[i]->fieldINST.vecTX.clear();

			// 析构vLupt
			delete vLupt[i];
			vLupt[i] = nullptr;
		}
	}
	vLupt.clear();
}

//************************************
// Method:    readAsciiFile
// Brief:	  读取Ascii查找表文件
// Returns:   bool
// Author:    cl
// DateTime:  2022/07/21
// Parameter: const QString & fileName
//************************************
bool CS52LutTable::readAsciiFile(const QString& fileName)
{
	char nDelimite = '\x1F';	// 分隔符
	QString info;
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		info = fileName + "文件打开失败，退出！";
		//qDebug() << info << endl;
		return false;
	}

	QString line;
	unsigned short num = 0;
	unsigned short nNumChar;
	QString str;

	S52LuptField filedLUPT;			bool bLupt = false;
	S52AttcField fieldATTC;			bool bAttc = false;
	S52InstField fieldINST;			bool bInst = false;
	S52DiscField fieldDISC;			bool bDisc = false;
	S52LucmField fieldLUCM;			bool bLucm = false;

	showTextTX*		pShowTextTX;
	showTextTE*		pShowTextTE;
	showPointSY*	pShowPointSY;
	showLineLS*		pShowLineLS;
	showLineLC*		pShowLineLC;
	showFillAC*		pShowFillAC;
	showFillAP*		pShowFillAP;
	showProcCS*		pShowProcCS;
	showLineCA*		pShowLineCA;

	QStringList lstValues;
	QString indexStr;

	while (!file.atEnd())
	{
		line = file.readLine();
		num++;
		if (line.isNull())
		{
			info = fileName + "文件中第" + QString("%1").arg(num) + "行是空行，跳过！";
			//qDebug() << info << endl;
			continue;
		}

		str = line.left(4);

		if (str == "0001")
			continue;

		// 查找表域结束标志
		if (str == "****" &&
			bLupt == true &&
			bAttc == true &&
			bInst == true &&
			bDisc == true &&
			bLucm == true)
		{
			LUPT* pLupt = new LUPT();
			pLupt->fieldLUPT = filedLUPT;
			pLupt->fieldATTC = fieldATTC;
			pLupt->fieldDISC = fieldDISC;
			pLupt->fieldINST = fieldINST;
			pLupt->fieldLUCM = fieldLUCM;
			if (filedLUPT.TNAM == "PLAIN_BOUNDARIES")
				m_vecLutPlainBoundaries.push_back(pLupt);
			else if (filedLUPT.TNAM == "SYMBOLIZED_BOUNDARIES")
				m_vecLutSimplifiedBoundaries.push_back(pLupt);
			else if (filedLUPT.TNAM == "SIMPLIFIED")
				m_vecLutSimplified.push_back(pLupt);
			else if (filedLUPT.TNAM == "PAPER_CHART")
				m_vecLutPaperChart.push_back(pLupt);
			else if (filedLUPT.TNAM == "LINES")
				m_vecLutLines.push_back(pLupt);

			fieldATTC.lstCombin.clear();
			fieldINST.vecAC.clear();
			fieldINST.vecAP.clear();
			fieldINST.vecCA.clear();
			fieldINST.vecCS.clear();
			fieldINST.vecLC.clear();
			fieldINST.vecLS.clear();
			fieldINST.vecSY.clear();
			fieldINST.vecTE.clear();
			fieldINST.vecTX.clear();
			bLupt = false;
			bAttc = false;
			bInst = false;
			bDisc = false;
			bLucm = false;
			continue;
		}
		nNumChar = line.mid(4, 5).toUShort();

		// 查找表LUPT字段
		if (str == "LUPT")
		{
			filedLUPT.OBCL = line.mid(19, 6).toUpper();
			filedLUPT.FTYP = line.at(25).toLatin1();
			filedLUPT.DPRI = line.mid(26, 5).toUShort();
			filedLUPT.RPRI = line.at(31).toLatin1();
			filedLUPT.TNAM = line.mid(32, nNumChar - 32 + 8);
			bLupt = true;
			continue;
		}

		// 查找表ATTC字段
		if (str == "ATTC")
		{
			bAttc = true;
			if (nNumChar == 0)
				continue;
			lstValues = line.mid(9).split(nDelimite);
			for (int i = 0; i < lstValues.size() - 1; i++)
			{
				if (lstValues[i].size() > 5)
					fieldATTC.lstCombin.append(lstValues[i]);
			}
			continue;
		}

		// 查换表INST字段
		if (str == "INST")
		{
			bInst = true;
			lstValues = line.mid(9).split(';');
			for (int i = 0; i < lstValues.size(); i++)
			{
				// showpointSY
				int loc1 = lstValues[i].indexOf("SY(", 0, Qt::CaseInsensitive);
				int loc2 = lstValues[i].indexOf(")", 0);
				if (loc1 >= 0)
				{
					pShowPointSY = new showPointSY();
					pShowPointSY->syname = lstValues[i].mid(3, 8).toUpper();
					if (loc2 - loc1 - 3 > 8)
					{
						pShowPointSY->attr = lstValues[i].mid(loc1 + 12, loc2 - loc1 - 12);
					}
					fieldINST.vecSY.push_back(pShowPointSY);
					continue;
				}

				// showtextTX
				loc1 = lstValues[i].indexOf("TX(", 0, Qt::CaseInsensitive);
				if (loc1 >= 0)
				{
					pShowTextTX = new showTextTX();
					QStringList lstValuesTmp = lstValues[i].split(',');
					pShowTextTX->str = lstValuesTmp[0].mid(3);
					pShowTextTX->hjust = lstValuesTmp[1].at(0).toLatin1();
					pShowTextTX->vjust = lstValuesTmp[2].at(0).toLatin1();
					pShowTextTX->space = lstValuesTmp[3].at(0).toLatin1();
					pShowTextTX->charsStyle = lstValuesTmp[4].at(1).toLatin1();
					pShowTextTX->charsWeight = lstValuesTmp[4].at(2).toLatin1();
					pShowTextTX->charsItalic = lstValuesTmp[4].at(3).toLatin1();
					pShowTextTX->charsSize = lstValuesTmp[4].mid(4, 2).toUShort();
					pShowTextTX->xoffs = lstValuesTmp[5].toShort();
					pShowTextTX->yoffs = lstValuesTmp[6].toShort();
					pShowTextTX->color = lstValuesTmp[7];
					pShowTextTX->textGroup = lstValuesTmp[8].left(lstValuesTmp[8].size() - 1).toShort();
					fieldINST.vecTX.push_back(pShowTextTX);
					continue;
				}

				// showtextTE
				loc1 = lstValues[i].indexOf("TE(", 0, Qt::CaseInsensitive);
				if (loc1 >= 0)
				{
					pShowTextTE = new showTextTE();
					QStringList lstValuesTmp = lstValues[i].split(',');
					pShowTextTE->formatstr = lstValuesTmp[0].mid(4, lstValuesTmp[0].size() - 5);
					pShowTextTE->attrb = lstValuesTmp[1].mid(1, 6).toUpper();
					pShowTextTE->hjust = lstValuesTmp[2].at(0).toLatin1();
					pShowTextTE->vjust = lstValuesTmp[3].at(0).toLatin1();
					pShowTextTE->space = lstValuesTmp[4].at(0).toLatin1();
					pShowTextTE->charsStyle = lstValuesTmp[5].at(1).toLatin1();
					pShowTextTE->charsWeight = lstValuesTmp[5].at(2).toLatin1();
					pShowTextTE->charsItalic = lstValuesTmp[5].at(3).toLatin1();
					pShowTextTE->charsSize = lstValuesTmp[5].mid(4, 2).toUShort();
					pShowTextTE->xoffs = lstValuesTmp[6].toShort();
					pShowTextTE->yoffs = lstValuesTmp[7].toShort();
					pShowTextTE->color = lstValuesTmp[8];
					pShowTextTE->textGroup = lstValuesTmp[9].left(lstValuesTmp[9].size() - 1).toShort();

					fieldINST.vecTE.push_back(pShowTextTE);
				}

				// showlineLS
				loc1 = lstValues[i].indexOf("LS(", 0, Qt::CaseInsensitive);
				if (loc1 >= 0)
				{
					pShowLineLS = new showLineLS();
					QStringList lstValuesTmp = lstValues[i].split(',');

					pShowLineLS->lstyle = lstValuesTmp[0].mid(3).toUpper();
					pShowLineLS->lwidth = lstValuesTmp[1].toUShort();
					pShowLineLS->lcolor = lstValuesTmp[2].mid(0, 5).toUpper();

					fieldINST.vecLS.push_back(pShowLineLS);
					continue;
				}

				// showlineLC
				loc1 = lstValues[i].indexOf("LC(", 0, Qt::CaseInsensitive);
				if (loc1 >= 0)
				{
					pShowLineLC = new showLineLC;

					pShowLineLC->Cline = lstValues[i].mid(3, 8);

					fieldINST.vecLC.push_back(pShowLineLC);
				}

				// showlineAC
				loc1 = lstValues[i].indexOf("AC(", 0, Qt::CaseInsensitive);
				if (loc1 >= 0)
				{
					pShowFillAC = new showFillAC();
					pShowFillAC->areacolor = lstValues[i].mid(3, 5);
					if (loc2 - loc1 > 9)
						pShowFillAC->transparence = lstValues[i].mid(9, 1).toUShort();
					fieldINST.vecAC.push_back(pShowFillAC);
				}

				// showlineAP
				loc1 = lstValues[i].indexOf("AP(", 0, Qt::CaseInsensitive);
				if (loc1 >= 0)
				{
					pShowFillAP = new showFillAP;
					pShowFillAP->patname = lstValues[i].mid(3, 8);
					pShowFillAP->rotation = 0.0;

					fieldINST.vecAP.push_back(pShowFillAP);
				}

				// showprocCS
				loc1 = lstValues[i].indexOf("CS(", 0, Qt::CaseInsensitive);
				if (loc1 >= 0)
				{
					pShowProcCS = new showProcCS;
					pShowProcCS->procName = lstValues[i].mid(3, 8);
					fieldINST.vecCS.push_back(pShowProcCS);
				}

			}
			continue;
		}

		if (str == "DISC")
		{
			bDisc = true;
			fieldDISC.DSCN = line.mid(9, nNumChar - 1);
			continue;
		}

		if (str == "LUCM")
		{
			bLucm = true;
			fieldLUCM.LUED = line.mid(9, nNumChar - 1);
			continue;
		}
	}
	file.close();
	return true;
}

QVector<LUPT*> CS52LutTable::getLuptLst(int prim) const
{
	return	prim == 1 ? m_vecLutSimplified : 
			(prim == 2 ? m_vecLutLines : m_vecLutPlainBoundaries);
}

bool CS52LutTable::parseCS(S52InstField& instField, string ruleString)
{
	size_t loc = 0;
	size_t ptr = 0;
	char szSection[256];
	char* szString = new char[strlen(ruleString.c_str()) + 1];
	strcpy(szString, ruleString.c_str());
	szString[strlen(ruleString.c_str())] = '\0';

	// showTX
	while ((loc = ruleString.find("TX(", ptr)) != std::string::npos)
	{
		ptr = loc + 3;
		showTextTX* pTX = new showTextTX;
		loc = ruleString.find(",", ptr);								// TX第一个参数结束位
		strncpy(szSection, szString + ptr, loc - ptr);
		szSection[loc - ptr] = '\0';
		pTX->str.append(szSection);
		memset(szSection, 0, 256);
		ptr = loc + 1;

		pTX->hjust = szString[ptr] - '0';
		pTX->vjust = szString[ptr + 2] - '0';
		pTX->space = szString[ptr + 4] - '0';
		pTX->charsStyle = szString[ptr + 7] - '0';
		pTX->charsWeight = szString[ptr + 8] - '0';
		pTX->charsItalic = szString[ptr + 9] - '0';
		ptr = loc + 11;

		loc = ruleString.find(",", ptr);
		strncpy(szSection, szString + ptr, loc - ptr - 1);
		szSection[loc - ptr - 1] = '\0';
		pTX->charsSize = atoi(szSection);
		ptr = loc + 1;

		loc = ruleString.find(",", ptr);
		strncpy(szSection, szString + ptr, loc - ptr);
		szSection[loc - ptr] = '\0';
		pTX->xoffs = atoi(szSection);
		ptr = loc + 1;

		loc = ruleString.find(",", ptr);
		strncpy(szSection, szString + ptr, loc - ptr);
		szSection[loc - ptr] = '\0';
		pTX->yoffs = atoi(szSection);
		ptr = loc + 1;

		strncpy(szSection, szString + ptr, 5);
		szSection[5] = '\0';
		pTX->color.append(szSection);
		memset(szSection, 0, 256);
		ptr += 6;

		loc = ruleString.find(")", ptr);
		strncpy(szSection, szString + ptr, loc - ptr);
		szSection[loc - ptr] = '\0';
		pTX->textGroup = atoi(szSection);
		ptr = loc + 1;
		instField.vecTX.push_back(pTX);
	}

	// showTE
	ptr = 0;
	while ((loc = ruleString.find("TE(", ptr)) != std::string::npos)
	{
		ptr = loc + 3;
		showTextTE* pTE = new showTextTE();
		loc = ruleString.find(",", ptr);								// TE第一个参数结束位
		strncpy(szSection, szString + ptr, loc - ptr);
		szSection[loc - ptr] = '\0';
		pTE->formatstr.append(szSection);
		memset(szSection, 0, 256);
		ptr = loc + 2;

		strncpy(szSection, szString + ptr, 6);
		szSection[6] = '\0';
		pTE->attrb.append(szSection);
		memset(szSection, 0, 256);
		pTE->hjust = szString[ptr + 8] - '0';
		pTE->vjust = szString[ptr + 10] - '0';
		pTE->space = szString[ptr + 12] - '0';
		pTE->charsStyle = szString[ptr + 15] - '0';
		pTE->charsWeight = szString[ptr + 16] - '0';
		pTE->charsItalic = szString[ptr + 17] - '0';
		ptr = loc + 20;

		loc = ruleString.find(",", ptr);
		strncpy(szSection, szString + ptr, loc - ptr - 1);
		szSection[loc - ptr - 1] = '\0';
		pTE->charsSize = atoi(szSection);
		ptr = loc + 1;

		loc = ruleString.find(",", ptr);
		strncpy(szSection, szString + ptr, loc - ptr);
		szSection[loc - ptr] = '\0';
		pTE->xoffs = atoi(szSection);
		ptr = loc + 1;

		loc = ruleString.find(",", ptr);
		strncpy(szSection, szString + ptr, loc - ptr);
		szSection[loc - ptr] = '\0';
		pTE->yoffs = atoi(szSection);
		ptr = loc + 1;

		strncpy(szSection, szString + ptr, 5);
		szSection[5] = '\0';
		pTE->color.append(szSection);
		memset(szSection, 0, 256);
		ptr += 6;

		loc = ruleString.find(")", ptr);
		strncpy(szSection, szString + ptr, loc - ptr);
		szSection[loc - ptr] = '\0';
		pTE->textGroup = atoi(szSection);
		ptr = loc + 1;
		instField.vecTE.push_back(pTE);
	}

	//showSY
	ptr = 0;
	while ((loc = ruleString.find("SY(", ptr)) != std::string::npos)
	{
		int end = ruleString.find(")", ptr);
		ptr = loc + 3;
		showPointSY* pSY = new showPointSY;
		strncpy(szSection, szString + ptr, 8);
		szSection[8] = '\0';
		pSY->syname.append(szSection);
		memset(szSection, 0, 256);
		ptr = loc + 12;
		if (end > ptr)
		{

			if ((end - ptr) == 6)
			{
				strncpy(szSection, szString + ptr, 6);
				szSection[6] = '\0';
				pSY->attr.append(szSection);
				memset(szSection, 0, 256);
			}
			else
			{
				strncpy(szSection, szString + ptr, end - ptr);
				szSection[end - ptr] = '\0';
				pSY->rotation = atof(szSection);
			}
		}
		if (ruleString.find("offset") != string::npos)
			pSY->offset = true;

		instField.vecSY.push_back(pSY);
	}

	ptr = 0;
	while ((loc = ruleString.find("LS(", ptr)) != std::string::npos)
	{
		ptr = loc + 3;
		showLineLS* pLS = new showLineLS;
		strncpy(szSection, szString + ptr, 4);
		szSection[4] = '\0';
		pLS->lstyle.append(szSection);
		memset(szSection, 0, 256);
		pLS->lwidth = szString[ptr + 5] - '0';
		strncpy(szSection, szString + ptr + 7, 5);
		szSection[5] = '\0';
		pLS->lcolor.append(szSection);
		memset(szSection, 0, 256);
		ptr = loc + 12;
		instField.vecLS.push_back(pLS);
	}

	ptr = 0;
	while ((loc = ruleString.find("LC(", ptr)) != std::string::npos)
	{
		ptr = loc + 3;
		showLineLC* pLC = new showLineLC;
		strncpy(szSection, szString + ptr, 8);
		szSection[8] = '\0';
		pLC->Cline.append(szSection);
		memset(szSection, 0, 256);
		ptr = loc + 11;
		instField.vecLC.push_back(pLC);
	}

	ptr = 0;
	while ((loc = ruleString.find("AC(", ptr)) != std::string::npos)
	{
		ptr = loc + 3;
		showFillAC* pAC = new showFillAC;
		strncpy(szSection, szString + ptr, 5);
		szSection[5] = '\0';  // add by wb
		pAC->areacolor.append(szSection);
		memset(szSection, 0, 256);
		ptr = loc + 9;
		loc = ruleString.find(")", ptr);
		if ((loc - ptr) == 1)
		{
			pAC->transparence = szString[ptr] - '0';
		}
		instField.vecAC.push_back(pAC);
	}

	ptr = 0;
	while ((loc = ruleString.find("AP(", ptr)) != std::string::npos)
	{
		ptr = loc + 3;
		showFillAP* pAP = new showFillAP;
		strncpy(szSection, szString + ptr, 8);
		szSection[8] = '\0';
		pAP->patname.append(szSection);
		memset(szSection, 0, 256);
		ptr = loc + 11;
		instField.vecAP.push_back(pAP);
	}

	ptr = 0;
	int loc1;
	while ((loc = ruleString.find("CA(", ptr)) != std::string::npos)
	{
		ptr = loc + 3;
		if ((loc1 = ruleString.find(',', ptr)) == string::npos)
		{
			continue;
		}
		string outlinecolor = ruleString.substr(ptr, loc1 - ptr);

		ptr = loc1 + 1;
		if ((loc1 = ruleString.find(',', ptr)) == string::npos)
		{
			continue;
		}
		unsigned short outlinewidth = atoi(ruleString.substr(ptr, loc1).c_str());

		ptr = loc1 + 1;
		if ((loc1 = ruleString.find(',', ptr)) == string::npos)
		{
			continue;
		}
		string arccolor = ruleString.substr(ptr, loc1 - ptr);

		ptr = loc1 + 1;
		if ((loc1 = ruleString.find(',', ptr)) == string::npos)
		{
			continue;
		}
		unsigned short arcwidth = atoi(ruleString.substr(ptr, loc1).c_str());

		ptr = loc1 + 1;
		if ((loc1 = ruleString.find(',', ptr)) == string::npos)
		{
			continue;
		}
		float sectr1 = atof(ruleString.substr(ptr, loc1).c_str());

		ptr = loc1 + 1;
		if ((loc1 = ruleString.find(',', ptr)) == string::npos)
		{
			continue;
		}
		float sectr2 = atof(ruleString.substr(ptr, loc1).c_str());

		ptr = loc1 + 1;
		if ((loc1 = ruleString.find(',', ptr)) == string::npos)
		{
			continue;
		}
		float arcR = atof(ruleString.substr(ptr, loc1).c_str());

		ptr = loc1 + 1;
		float sectorR = atof(ruleString.substr(ptr, ruleString.length()).c_str());

		showLineCA* pCA = new showLineCA;

		strncpy(pCA->outline_color, outlinecolor.c_str(), 5);
		pCA->outline_color[5] = '\0';
		strncpy(pCA->arc_color, arccolor.c_str(), 5);
		pCA->arc_color[5] = '\0';
		pCA->arc_radius = arcR;
		pCA->sector_radius = sectorR;
		pCA->arc_width = arcwidth;
		pCA->outline_width = outlinewidth;
		pCA->sectr1 = sectr1;
		pCA->sectr2 = sectr2;

		instField.vecCA.push_back(pCA);
	}
	delete[] szString;
	szString = NULL;

	return true;
}
//bool CS52LutTable::parseCS(S52InstField& tmpInstField, string ruleString)
//{
//	QString strInst;
//	strInst = ruleString.c_str();
//
//	showPointSY * tmpShowPointSY = nullptr;
//	showTextTX * tmpShowTextTX = nullptr;
//	showTextTE * tmpShowTextTE = nullptr;
//	showLineLS * tmpShowLineLS = nullptr;
//	showLineLC * tmpShowLineLC = nullptr;
//	showFillAC * tmpShowFillAC = nullptr;
//	showFillAP * tmpShowFillAP = nullptr;
//	showLineCA * tmpCA = nullptr;
//
//	QStringList lstInst = strInst.split(';');
//
//	for (int i = 0; i < lstInst.size(); i++)
//	{
//		QString subInst = lstInst[i];
//
//		// showpointSY
//		if (subInst.contains("SY("))
//		{
//			subInst = subInst.remove("SY(");
//			subInst = subInst.remove(")");
//			QStringList lstSym = subInst.split(",");
//
//			if (lstSym.size() < 1)
//				continue;
//			tmpShowPointSY = new showPointSY;
//			tmpShowPointSY->syname = lstSym.at(0);
//			if (lstSym.size() > 1)
//				tmpShowPointSY->attr = lstSym.at(1);
//			tmpInstField.vecSY.push_back(tmpShowPointSY);
//			continue;
//		}
//
//		// showtextTX
//		if (subInst.contains("TX("))
//		{
//			subInst = subInst.remove("TX(");
//			subInst = subInst.remove(")");
//			QStringList lstValuesTmp = subInst.split(',');
//
//			tmpShowTextTX = new showTextTX;
//			tmpShowTextTX->str = lstValuesTmp[0].mid(0);
//			tmpShowTextTX->hjust = lstValuesTmp[1].at(0).toLatin1();
//			tmpShowTextTX->vjust = lstValuesTmp[2].at(0).toLatin1();
//			tmpShowTextTX->space = lstValuesTmp[3].at(0).toLatin1();
//			tmpShowTextTX->charsStyle = lstValuesTmp[4].at(1).toLatin1();	// '15110'-1
//			tmpShowTextTX->charsWeight = lstValuesTmp[4].at(2).toLatin1();	// '15110'-5
//			tmpShowTextTX->charsItalic = lstValuesTmp[4].at(3).toLatin1();	// '15110'-1
//			tmpShowTextTX->charsSize = lstValuesTmp[4].mid(4, 2).toUShort();// '15110'-10
//			tmpShowTextTX->xoffs = lstValuesTmp[5].toShort();
//			tmpShowTextTX->yoffs = lstValuesTmp[6].toShort();
//			tmpShowTextTX->color = lstValuesTmp[7];
//			tmpShowTextTX->textGroup = lstValuesTmp[8].toShort();
//
//			tmpInstField.vecTX.push_back(tmpShowTextTX);
//			continue;
//		}
//
//		// showtextTE
//		if (subInst.contains("TE("))
//		{
//			subInst = subInst.remove("TE(");
//			subInst = subInst.remove(")");
//			QStringList lstValuesTmp = subInst.split(',');
//
//			tmpShowTextTE = new showTextTE;
//			tmpShowTextTE->formatstr = lstValuesTmp[0].mid(1, lstValuesTmp[0].size() - 2);
//			tmpShowTextTE->attrb = lstValuesTmp[1].mid(1, lstValuesTmp[1].size() - 2).toUpper();
//			tmpShowTextTE->hjust = lstValuesTmp[2].at(0).toLatin1();
//			tmpShowTextTE->vjust = lstValuesTmp[3].at(0).toLatin1();
//			tmpShowTextTE->space = lstValuesTmp[4].at(0).toLatin1();
//			tmpShowTextTE->charsStyle = lstValuesTmp[5].at(1).toLatin1();
//			tmpShowTextTE->charsWeight = lstValuesTmp[5].at(2).toLatin1();
//			tmpShowTextTE->charsItalic = lstValuesTmp[5].at(3).toLatin1();
//			tmpShowTextTE->charsSize = lstValuesTmp[5].mid(4, 2).toUShort();
//			tmpShowTextTE->xoffs = lstValuesTmp[6].toShort();
//			tmpShowTextTE->yoffs = lstValuesTmp[7].toShort();
//			tmpShowTextTE->color = lstValuesTmp[8];
//			tmpShowTextTE->textGroup = lstValuesTmp[9].left(lstValuesTmp[9].size() - 1).toShort();
//
//			tmpInstField.vecTE.push_back(tmpShowTextTE);
//			continue;
//		}
//
//		// showlineLS
//		if (subInst.contains("LS("))
//		{
//			subInst = subInst.remove("LS(");
//			subInst = subInst.remove(")");
//			QStringList lstValuesTmp = subInst.split(',');
//
//			tmpShowLineLS = new showLineLS;
//			tmpShowLineLS->lstyle = lstValuesTmp[0].toUpper();
//			tmpShowLineLS->lwidth = lstValuesTmp[1].toUShort();
//			tmpShowLineLS->lcolor = lstValuesTmp[2].toUpper();
//
//			tmpInstField.vecLS.push_back(tmpShowLineLS);
//			continue;
//		}
//
//		// showlineLC
//		if (subInst.contains("LC("))
//		{
//			subInst = subInst.remove("LC(");
//			subInst = subInst.remove(")");
//
//			tmpShowLineLC = new showLineLC;
//			tmpShowLineLC->Cline = subInst;
//
//			tmpInstField.vecLC.push_back(tmpShowLineLC);
//			continue;
//		}
//
//		// showlineAC
//		if (subInst.contains("AC("))
//		{
//			subInst = subInst.remove("AC(");
//			subInst = subInst.remove(")");
//			QStringList lstValuesTmp = subInst.split(',');
//
//			tmpShowFillAC = new showFillAC;
//			tmpShowFillAC->areacolor = lstValuesTmp[0];
//			if (lstValuesTmp.size() > 1)
//				tmpShowFillAC->transparence = lstValuesTmp[1].toUShort();
//			tmpInstField.vecAC.push_back(tmpShowFillAC);
//			continue;
//		}
//
//		// showlineAP
//		if (subInst.contains("AP("))
//		{
//			subInst = subInst.remove("AP(");
//			subInst = subInst.remove(")");
//
//			tmpShowFillAP = new showFillAP;
//			tmpShowFillAP->patname = subInst;
//			tmpInstField.vecAP.push_back(tmpShowFillAP);
//			continue;
//		}
//
//		//showLineCA
//		if (subInst.contains("CA("))
//		{
//			subInst = subInst.remove("CA(");
//			subInst = subInst.remove(")");
//			QStringList lstElement = subInst.split(',');
//			if (lstElement.size() >= 7)
//			{
//				tmpCA = new showLineCA;
//				tmpCA->outline_color = lstElement.at(0);
//				tmpCA->outline_width = lstElement.at(1).toUShort();
//				tmpCA->arc_color = lstElement.at(2);
//				tmpCA->arc_width = lstElement.at(3).toUShort();
//				tmpCA->sectr1 = lstElement.at(4).toFloat();
//				tmpCA->sectr2 = lstElement.at(5).toFloat();
//				tmpCA->sector_radius = lstElement.at(6).toFloat();
//				tmpInstField.vecCA.push_back(tmpCA);
//			}
//			else
//			{ /*CA非法！*/
//			}
//			continue;
//		}
//
//	}
//	return true;
//}