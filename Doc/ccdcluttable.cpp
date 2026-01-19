#include "ccdcluttable.h"
#include <QApplication>
#include <QTextStream>
#include <QDebug>
#include <QFile>

using namespace Doc;
CCDCLutTable::CCDCLutTable()
	: m_strLutPointPath(QApplication::applicationDirPath() + "/cdcData/cdcp.txt")
	, m_strLutLinePath(QApplication::applicationDirPath() + "/cdcData/cdcl.txt")
	, m_strLutAreaPath(QApplication::applicationDirPath() + "/cdcData/cdca.txt")
{
	readLut(m_vecCDCPointLut, m_strLutPointPath,CDC_P);
	readLut(m_vecCDCLineLut, m_strLutLinePath,CDC_L);
	readLut(m_vecCDCAreaLut, m_strLutAreaPath,CDC_A);
}

CCDCLutTable::~CCDCLutTable()
{
	for (int i = 0; i < m_vecCDCPointLut.size(); i++)
	{
		if (m_vecCDCPointLut[i] != nullptr)
		{
			for (int j = 0; j < m_vecCDCPointLut[i]->fieldINST.vecAC.size(); j++)
			{
				if (m_vecCDCPointLut[i]->fieldINST.vecAC[j] != nullptr)
					delete m_vecCDCPointLut[i]->fieldINST.vecAC[j];
				m_vecCDCPointLut[i]->fieldINST.vecAC[j] = nullptr;
			}
			for (int j = 0; j < m_vecCDCPointLut[i]->fieldINST.vecAP.size(); j++)
			{
				if (m_vecCDCPointLut[i]->fieldINST.vecAP[j] != nullptr)
					delete m_vecCDCPointLut[i]->fieldINST.vecAP[j];
				m_vecCDCPointLut[i]->fieldINST.vecAP[j] = nullptr;
			}
			for (int j = 0; j < m_vecCDCPointLut[i]->fieldINST.vecCA.size(); j++)
			{
				if (m_vecCDCPointLut[i]->fieldINST.vecCA[j] != nullptr)
					delete m_vecCDCPointLut[i]->fieldINST.vecCA[j];
				m_vecCDCPointLut[i]->fieldINST.vecCA[j] = nullptr;
			}
			for (int j = 0; j < m_vecCDCPointLut[i]->fieldINST.vecCS.size(); j++)
			{
				if (m_vecCDCPointLut[i]->fieldINST.vecCS[j] != nullptr)
					delete m_vecCDCPointLut[i]->fieldINST.vecCS[j];
				m_vecCDCPointLut[i]->fieldINST.vecCS[j] = nullptr;
			}
			for (int j = 0; j < m_vecCDCPointLut[i]->fieldINST.vecLC.size(); j++)
			{
				if (m_vecCDCPointLut[i]->fieldINST.vecLC[j] != nullptr)
					delete m_vecCDCPointLut[i]->fieldINST.vecLC[j];
				m_vecCDCPointLut[i]->fieldINST.vecLC[j] = nullptr;
			}
			for (int j = 0; j < m_vecCDCPointLut[i]->fieldINST.vecLS.size(); j++)
			{
				if (m_vecCDCPointLut[i]->fieldINST.vecLS[j] != nullptr)
					delete m_vecCDCPointLut[i]->fieldINST.vecLS[j];
				m_vecCDCPointLut[i]->fieldINST.vecLS[j] = nullptr;
			}
			for (int j = 0; j < m_vecCDCPointLut[i]->fieldINST.vecSY.size(); j++)
			{
				if (m_vecCDCPointLut[i]->fieldINST.vecSY[j] != nullptr)
					delete m_vecCDCPointLut[i]->fieldINST.vecSY[j];
				m_vecCDCPointLut[i]->fieldINST.vecSY[j] = nullptr;
			}
			for (int j = 0; j < m_vecCDCPointLut[i]->fieldINST.vecTE.size(); j++)
			{
				if (m_vecCDCPointLut[i]->fieldINST.vecTE[j] != nullptr)
					delete m_vecCDCPointLut[i]->fieldINST.vecTE[j];
				m_vecCDCPointLut[i]->fieldINST.vecTE[j] = nullptr;
			}
			for (int j = 0; j < m_vecCDCPointLut[i]->fieldINST.vecTX.size(); j++)
			{
				if (m_vecCDCPointLut[i]->fieldINST.vecTX[j] != nullptr)
					delete m_vecCDCPointLut[i]->fieldINST.vecTX[j];
				m_vecCDCPointLut[i]->fieldINST.vecTX[j] = nullptr;
			}

			delete m_vecCDCPointLut[i];
			m_vecCDCPointLut[i] = nullptr;
		}

	}

 	for (int i = 0; i < m_vecCDCLineLut.size(); i++)
 	{
 		if (m_vecCDCLineLut[i] != nullptr)
 		{
 			for (int j = 0; j < m_vecCDCLineLut[i]->fieldINST.vecAC.size(); j++)
 			{
 				if (m_vecCDCLineLut[i]->fieldINST.vecAC[j] != nullptr)
 					delete m_vecCDCLineLut[i]->fieldINST.vecAC[j];
 				m_vecCDCLineLut[i]->fieldINST.vecAC[j] = nullptr;
 			}
 			for (int j = 0; j < m_vecCDCLineLut[i]->fieldINST.vecAP.size(); j++)
 			{
 				if (m_vecCDCLineLut[i]->fieldINST.vecAP[j] != nullptr)
 					delete m_vecCDCLineLut[i]->fieldINST.vecAP[j];
 				m_vecCDCLineLut[i]->fieldINST.vecAP[j] = nullptr;
 			}
 			for (int j = 0; j < m_vecCDCLineLut[i]->fieldINST.vecCA.size(); j++)
 			{
 				if (m_vecCDCLineLut[i]->fieldINST.vecCA[j] != nullptr)
 					delete m_vecCDCLineLut[i]->fieldINST.vecCA[j];
 				m_vecCDCLineLut[i]->fieldINST.vecCA[j] = nullptr;
 			}
 			for (int j = 0; j < m_vecCDCLineLut[i]->fieldINST.vecCS.size(); j++)
 			{
 				if (m_vecCDCLineLut[i]->fieldINST.vecCS[j] != nullptr)
 					delete m_vecCDCLineLut[i]->fieldINST.vecCS[j];
 				m_vecCDCLineLut[i]->fieldINST.vecCS[j] = nullptr;
 			}
 			for (int j = 0; j < m_vecCDCLineLut[i]->fieldINST.vecLC.size(); j++)
 			{
 				if (m_vecCDCLineLut[i]->fieldINST.vecLC[j] != nullptr)
 					delete m_vecCDCLineLut[i]->fieldINST.vecLC[j];
 				m_vecCDCLineLut[i]->fieldINST.vecLC[j] = nullptr;
 			}
 			for (int j = 0; j < m_vecCDCLineLut[i]->fieldINST.vecLS.size(); j++)
 			{
 				if (m_vecCDCLineLut[i]->fieldINST.vecLS[j] != nullptr)
 					delete m_vecCDCLineLut[i]->fieldINST.vecLS[j];
 				m_vecCDCLineLut[i]->fieldINST.vecLS[j] = nullptr;
 			}
 			for (int j = 0; j < m_vecCDCLineLut[i]->fieldINST.vecSY.size(); j++)
 			{
 				if (m_vecCDCLineLut[i]->fieldINST.vecSY[j] != nullptr)
 					delete m_vecCDCLineLut[i]->fieldINST.vecSY[j];
 				m_vecCDCLineLut[i]->fieldINST.vecSY[j] = nullptr;
 			}
 			for (int j = 0; j < m_vecCDCLineLut[i]->fieldINST.vecTE.size(); j++)
 			{
 				if (m_vecCDCLineLut[i]->fieldINST.vecTE[j] != nullptr)
 					delete m_vecCDCLineLut[i]->fieldINST.vecTE[j];
 				m_vecCDCLineLut[i]->fieldINST.vecTE[j] = nullptr;
 			}
 			for (int j = 0; j < m_vecCDCLineLut[i]->fieldINST.vecTX.size(); j++)
 			{
 				if (m_vecCDCLineLut[i]->fieldINST.vecTX[j] != nullptr)
 					delete m_vecCDCLineLut[i]->fieldINST.vecTX[j];
 				m_vecCDCLineLut[i]->fieldINST.vecTX[j] = nullptr;
 			}
 
 			delete m_vecCDCLineLut[i];
 			m_vecCDCLineLut[i] = nullptr;
 		}
 	}

 	for (int i = 0; i < m_vecCDCAreaLut.size(); i++)
 	{
 		if (m_vecCDCAreaLut[i] != nullptr)
 		{
 			for (int j = 0; j < m_vecCDCAreaLut[i]->fieldINST.vecAC.size(); j++)
 			{
 				if (m_vecCDCAreaLut[i]->fieldINST.vecAC[j] != nullptr)
 					delete m_vecCDCAreaLut[i]->fieldINST.vecAC[j];
 				m_vecCDCAreaLut[i]->fieldINST.vecAC[j] = nullptr;
 			}
 			for (int j = 0; j < m_vecCDCAreaLut[i]->fieldINST.vecAP.size(); j++)
 			{
 				if (m_vecCDCAreaLut[i]->fieldINST.vecAP[j] != nullptr)
 					delete m_vecCDCAreaLut[i]->fieldINST.vecAP[j];
 				m_vecCDCAreaLut[i]->fieldINST.vecAP[j] = nullptr;
 			}
 			for (int j = 0; j < m_vecCDCAreaLut[i]->fieldINST.vecCA.size(); j++)
 			{
 				if (m_vecCDCAreaLut[i]->fieldINST.vecCA[j] != nullptr)
 					delete m_vecCDCAreaLut[i]->fieldINST.vecCA[j];
 				m_vecCDCAreaLut[i]->fieldINST.vecCA[j] = nullptr;
 			}
 			for (int j = 0; j < m_vecCDCAreaLut[i]->fieldINST.vecCS.size(); j++)
 			{
 				if (m_vecCDCAreaLut[i]->fieldINST.vecCS[j] != nullptr)
 					delete m_vecCDCAreaLut[i]->fieldINST.vecCS[j];
 				m_vecCDCAreaLut[i]->fieldINST.vecCS[j] = nullptr;
 			}
 			for (int j = 0; j < m_vecCDCAreaLut[i]->fieldINST.vecLC.size(); j++)
 			{
 				if (m_vecCDCAreaLut[i]->fieldINST.vecLC[j] != nullptr)
 					delete m_vecCDCAreaLut[i]->fieldINST.vecLC[j];
 				m_vecCDCAreaLut[i]->fieldINST.vecLC[j] = nullptr;
 			}
 			for (int j = 0; j < m_vecCDCAreaLut[i]->fieldINST.vecLS.size(); j++)
 			{
 				if (m_vecCDCAreaLut[i]->fieldINST.vecLS[j] != nullptr)
 					delete m_vecCDCAreaLut[i]->fieldINST.vecLS[j];
 				m_vecCDCAreaLut[i]->fieldINST.vecLS[j] = nullptr;
 			}
 			for (int j = 0; j < m_vecCDCAreaLut[i]->fieldINST.vecSY.size(); j++)
 			{
 				if (m_vecCDCAreaLut[i]->fieldINST.vecSY[j] != nullptr)
 					delete m_vecCDCAreaLut[i]->fieldINST.vecSY[j];
 				m_vecCDCAreaLut[i]->fieldINST.vecSY[j] = nullptr;
 			}
 			for (int j = 0; j < m_vecCDCAreaLut[i]->fieldINST.vecTE.size(); j++)
 			{
 				if (m_vecCDCAreaLut[i]->fieldINST.vecTE[j] != nullptr)
 					delete m_vecCDCAreaLut[i]->fieldINST.vecTE[j];
 				m_vecCDCAreaLut[i]->fieldINST.vecTE[j] = nullptr;
 			}
 			for (int j = 0; j < m_vecCDCAreaLut[i]->fieldINST.vecTX.size(); j++)
 			{
 				if (m_vecCDCAreaLut[i]->fieldINST.vecTX[j] != nullptr)
 					delete m_vecCDCAreaLut[i]->fieldINST.vecTX[j];
 				m_vecCDCAreaLut[i]->fieldINST.vecTX[j] = nullptr;
 			}
 
 			delete m_vecCDCAreaLut[i];
 			m_vecCDCAreaLut[i] = nullptr;
 		}
 	}

}

void CCDCLutTable::readLut(QVector<LUPT*>& vecLut, const QString& strLutPath, SymbolType type)
{
	QFile file(strLutPath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	QTextStream in(&file);
	QString line = in.readLine();
// 	if (line.isNull())
// 	{
// 		qDebug() << strLutPath << " file is null." << endl;
// 		return;
// 	}
// 
// 	QStringList vecValues = line.split('\t');
// 	if (vecValues.size() == 0 ||
// 		vecValues.size() != 7)
// 	{
// 		qDebug() << strLutPath << " must be 7 columns." << endl;
// 		return;
// 	}

	while (!line.isNull())
	{
		line = in.readLine();
		if (line.isNull())
			break;

		if(line.at(0).toLatin1() == '#')
			continue;

		QStringList vecValues = line.split('\t');
		if (vecValues.size() == 0 ||
			vecValues.size() != 7)
		{
			qDebug() << m_strLutPointPath << " must be 7 columns." << endl;
			return;
		}
		//先检查数据，如果存在问题直接返回
		//防止new后才发现数据有问题出现内存泄漏
		for (int i = 0; i < 7; i++)
		{
			QString strValue = vecValues[i];
			if (i == 3 && strValue == "")
				return;
			else if (i == 4 && strValue.length() != 1)
				return;
		}

		LUPT* pLupt = new LUPT();

		switch (type)
		{
		case Doc::CCDCLutTable::CDC_P:
			pLupt->fieldLUPT.TNAM = "CDC_P";
			break;
		case Doc::CCDCLutTable::CDC_L:
			pLupt->fieldLUPT.TNAM = "CDC_L";
			break;
		case Doc::CCDCLutTable::CDC_A:
			pLupt->fieldLUPT.TNAM = "CDC_A";
			break;
		}
		for (int i = 0; i < 7; i++)
		{
			QString strValue = vecValues[i];
			switch (i)
			{
			case 0:
				pLupt->fieldLUPT.OBCL = strValue;
				break;
			case 1:
			{
				QStringList lstValues = strValue.split("|");
				pLupt->fieldATTC.lstCombin = lstValues;
				break;
			}
			case 2:
				parseInst(pLupt->fieldINST, strValue);
				break;
			case 3:
				pLupt->fieldLUPT.DPRI = strValue.toInt();
				break;
			case 4:
				pLupt->fieldLUPT.RPRI = strValue.at(0).toLatin1();
				break;
			case 5:
				pLupt->fieldDISC.DSCN = strValue;
				break;
			case 6:
				pLupt->fieldLUCM.LUED = strValue;
				break;
			default:
				break;
			}
		}
		vecLut.push_back(pLupt);
	}
	file.close();
}

void CCDCLutTable::parseInst(S52InstField& instField, QString instString)
{
	//showSY
	int loc = 0;
	int ptr = 0;
	loc = instString.indexOf("SY(", ptr);
	if(instString.indexOf("SY(", ptr) != -1)
	{
		int end = instString.indexOf(")", ptr);
		ptr = loc + 3;
		showPointSY* pSY = new showPointSY();
		pSY->syname = instString.mid(ptr, 8);
		ptr = loc + 12;
		if (end > ptr)
		{
			if ((end - ptr) == 6)
			{
				pSY->attr = instString.mid(ptr, 6);
			}
			else
			{
				QString strRotation = instString.mid(ptr, end - ptr);
				if (strRotation == "")
					strRotation = "0";
				pSY->rotation = strRotation.toInt();
			}
		}
		instField.vecSY.push_back(pSY);
	}

	//showLC
	loc = 0;
	ptr = 0;
	loc = instString.indexOf("LC(", ptr);
	if(instString.indexOf("LC(", ptr) != -1)
	{
		ptr = loc + 3;
		showLineLC* pLC = new showLineLC();
		pLC->Cline = instString.mid(ptr, 8);
		instField.vecLC.push_back(pLC);
	}

	//showAP
	loc = 0;
	ptr = 0;
	loc = instString.indexOf("AP(", ptr);
	if(instString.indexOf("AP(", ptr) != -1)
	{
		ptr = loc + 3;
		showFillAP* pAP = new showFillAP();
		pAP->patname = instString.mid(ptr, 8);
		instField.vecAP.push_back(pAP);
	}

	//showCS
	loc = 0;
	ptr = 0;
	loc = instString.indexOf("CS(", ptr);
	if (instString.indexOf("CS(", ptr) != -1)
	{
		ptr = loc + 3;
		showProcCS* pCS = new showProcCS();
		pCS->procName = instString.mid(ptr, 8);
		instField.vecCS.push_back(pCS);
	}
}

LUPT* CCDCLutTable::serachLupt(const QString& objlName, const QStringList& lstAttrCombin, eLookupTabType tabType)
{
	switch (tabType)
	{
	case eLookupTabType::CDC_P:
		return serachLupt(objlName, lstAttrCombin, m_vecCDCPointLut);
		break;
	case eLookupTabType::CDC_L:
		return serachLupt(objlName, lstAttrCombin, m_vecCDCLineLut);
		break;
	case eLookupTabType::CDC_A:
		return serachLupt(objlName, lstAttrCombin, m_vecCDCAreaLut);
		break;
	}
	return nullptr;
}

LUPT* CCDCLutTable::serachLupt(const QString& objlName, const QStringList& lstAttrCombin, QVector<LUPT*>& vLupt)
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
			for (int k = 0; k < lstAttrCombin.size(); k++)
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

			if (strAtvlSource == strAtvlLut)	// 属性值也匹配
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

