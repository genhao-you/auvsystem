#include "stdafx.h"
#include "cs57cell.h"
#include "cs57docmanager.h"
#include "cs57recfeature.h"
#include "cs57recvector.h"
#include "codec.h"
#include <QMessageBox>
#include <QDebug>


using namespace Doc;
#pragma  execution_character_set("utf-8")
CS57Cell::CS57Cell()
	: m_strCellFullPath("")
	, m_strCellErFullPath("")
	, m_pRecDsgiUpdate(nullptr)
	, m_pRecFeatureUpdate(nullptr)
	, m_pRecVectorUpdate(nullptr)
	, m_strXtmVersion("ver: 1.0.0")

{}

CS57Cell::~CS57Cell()
{
	close();
	releaseUpdate();
}

//************************************
// Method:    importEncFile
// Brief:	  导入Enc文件
// Returns:   bool
// Author:    cl
// DateTime:  2022/07/21
// Parameter: const QString & fileName
//************************************
bool CS57Cell::importEncFile(const QString& fileName)
{
	m_strCellFullPath = fileName;
	//QByteArray ba = strFileName.toLatin1();
	//const char* encFile = ba.data();
	//m_strCellFullPath = fileName;
	std::string strEncFile = codec->fromUnicode(fileName).data();
	if (!m_DdfModule.Open(strEncFile.c_str()))
	{
		m_DdfModule.Close();
		QMessageBox::information(nullptr, "提示", "000文件打开失败");
		return false;
	}

	DDFRecord* pDdfRec = nullptr;
	while ((pDdfRec = m_DdfModule.ReadRecord()) != nullptr)
	{
		if (pDdfRec->FindField("DSID", 0) != nullptr)
		{
			m_pRecDsgi = new CS57RecDsgi();
			m_pRecDsgi->init(pDdfRec);
			break;
		}
	}
	if (m_pRecDsgi == nullptr)
	{
		QMessageBox::information(nullptr, "提示", "无数据集描述记录");
		m_DdfModule.Close();
		return false;
	}

	int nRecFEsize = m_pRecDsgi->m_Dssi.nomr + m_pRecDsgi->m_Dssi.nocr +
					 m_pRecDsgi->m_Dssi.nogr + m_pRecDsgi->m_Dssi.nolr;
	int nRecVEsize = m_pRecDsgi->m_Dssi.noed;
	int nRecVCsize = m_pRecDsgi->m_Dssi.nocn;
	int nRecVIsize = m_pRecDsgi->m_Dssi.noin;
	//对vector容器预分配空间，提高效率
	m_vecS57RecFE.reserve(nRecFEsize + 10);
	m_vecS57RecVE.reserve(nRecVEsize + 10);
	m_vecS57RecVC.reserve(nRecVCsize + 10);
	m_vecS57RecVI.reserve(nRecVIsize + 10);

	m_DdfModule.MoveToFirstRec();
	while ((pDdfRec = m_DdfModule.ReadRecord()) != NULL)
	{
		CS57RecFeature* pRecFeature = nullptr;
		CS57RecVector* pRecVector = nullptr;
		int cRecName = pDdfRec->GetIntSubfield(pDdfRec->GetField(1)->GetFieldDefn()->GetName(), 0, "RCNM", 0);
		switch (cRecName)
		{
		case 20:	// 数据集地理参照记录
			m_pRecDsgr = new CS57RecDsgr();
			m_pRecDsgr->init(pDdfRec);
			break;
		case 100:	// 特征要素
			pRecFeature = new CS57RecFeature();
			pRecFeature->setParentCell(this);
			pRecFeature->init(pDdfRec);

			m_vecS57RecFE.push_back(pRecFeature);
			break;
		case 110:	// 孤立节点
			pRecVector = new CS57RecVector();
			pRecVector->init(pDdfRec);
			m_vecS57RecVI.push_back(pRecVector);
			break;
		case 120:	// 链接节点
			pRecVector = new CS57RecVector();
			pRecVector->init(pDdfRec);
			m_vecS57RecVC.push_back(pRecVector);
			break;
		case 130:	// 边
			pRecVector = new CS57RecVector();
			pRecVector->init(pDdfRec);
			m_vecS57RecVE.push_back(pRecVector);
			break;
		default:
			break;
		}
	}

	m_DdfModule.Close();	//关闭文件，清除数据文件记录

	indexCell();//建FE-VE-VC-VI之间关系索引
	extentCell();//计算图幅轮廓

	return true;
}

//************************************
// Method:    importErFile
// Brief:	  导入Er文件
// Returns:   bool
// Author:    cl
// DateTime:  2022/07/21
// Parameter: const QString & fileName
//************************************
bool CS57Cell::importErFile(const QString& fileName)
{
	m_strCellErFullPath = fileName;
	//QByteArray ba = strFileName.toLatin1();
	//const char* erFile = ba.data();
	std::string strErFile = codec->fromUnicode(fileName).data();
	if (!m_DdfModule.Open(strErFile.c_str()))
	{
		m_DdfModule.Close();
		QMessageBox::information(nullptr, "提示", "er文件打开失败");
		return false;
	}

	DDFRecord* pDdfRec = nullptr;
	int pnSuccess;
	CS57RecFeature* pRecFeature = nullptr;
	CS57RecVector* pRecVector = nullptr;
	releaseUpdate();
	while ((pDdfRec = m_DdfModule.ReadRecord()) != nullptr)
	{
		if (pDdfRec->FindField("DSID", 0) != nullptr)
		{
			m_pRecDsgiUpdate = new CS57RecDsgi();
			m_pRecDsgiUpdate->init(pDdfRec);
			break;
		}
	}
	if (m_pRecDsgi == nullptr)
	{
		QMessageBox::information(nullptr, "提示", "er中无数据集描述记录");
		m_DdfModule.Close();
		return false;
	}

	int nRecFEsize = m_pRecDsgiUpdate->m_Dssi.nomr + m_pRecDsgiUpdate->m_Dssi.nocr +
					 m_pRecDsgiUpdate->m_Dssi.nogr + m_pRecDsgiUpdate->m_Dssi.nolr;
	int nRecVEsize = m_pRecDsgiUpdate->m_Dssi.noed;
	int nRecVCsize = m_pRecDsgiUpdate->m_Dssi.nocn;
	int nRecVIsize = m_pRecDsgiUpdate->m_Dssi.noin;

	// 对vector容器预分配空间，提高效率
	m_vecS57RecFEUpdate.reserve(nRecFEsize + 10);
	m_vecS57RecVEUpdate.reserve(nRecVEsize + 10);
	m_vecS57RecVCUpdate.reserve(nRecVCsize + 10);
	m_vecS57RecVIUpdate.reserve(nRecVIsize + 10);

	while ((pDdfRec = m_DdfModule.ReadRecord()) != nullptr)
	{
		int cRecName = pDdfRec->GetIntSubfield(pDdfRec->GetField(1)->GetFieldDefn()->GetName(), 0, "RCNM", 0, &pnSuccess);
		switch (cRecName)
		{
		case 100:	// 特征要素
			pRecFeature = new CS57RecFeature();
			pRecFeature->setParentCell(this);
			pRecFeature->init(pDdfRec);
			m_vecS57RecFEUpdate.push_back(pRecFeature);
			break;
		case 110:	// 孤立节点
			pRecVector = new CS57RecVector();
			pRecVector->init(pDdfRec);
			m_vecS57RecVIUpdate.push_back(pRecVector);
			break;
		case 120:	// 链接节点
			pRecVector = new CS57RecVector();
			pRecVector->init(pDdfRec);
			m_vecS57RecVCUpdate.push_back(pRecVector);
			break;
		case 130:	// 边
			pRecVector = new CS57RecVector();
			pRecVector->init(pDdfRec);
			m_vecS57RecVEUpdate.push_back(pRecVector);
			break;
		default:
			break;
		}
	}
	m_DdfModule.Close();	//关闭文件，清除数据文件记录
	return true;
}

//************************************
// Method:    sortFeatureRec
// Brief:	  排序要素记录
// Returns:   bool
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CS57RecFeature * p1
// Parameter: CS57RecFeature * p2
//************************************
bool CS57Cell::sortFeatureRec(CS57RecFeature* p1, CS57RecFeature *p2)
{
	return p1->m_Foid.fidn < p2->m_Foid.fidn;
}

//************************************
// Method:    sortVectorRec
// Brief:	  排序空间记录
// Returns:   bool
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CS57RecVector * p1
// Parameter: CS57RecVector * p2
//************************************
bool CS57Cell::sortVectorRec(CS57RecVector* p1, CS57RecVector* p2)
{
	return p1->m_Vrid.rcid < p2->m_Vrid.rcid;
}

//************************************
// Method:    indexCell
// Brief:	  创建图幅索引
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57Cell::indexCell()
{
	qSort(m_vecS57RecVE.begin(), m_vecS57RecVE.end(), sortVectorRec);
	qSort(m_vecS57RecVC.begin(), m_vecS57RecVC.end(), sortVectorRec);
	qSort(m_vecS57RecVI.begin(), m_vecS57RecVI.end(), sortVectorRec);
	qSort(m_vecS57RecFE.begin(), m_vecS57RecFE.end(), sortFeatureRec);

	// 取边的范围　并建立边与链接节点的索引
	for (int i = 0; i < m_vecS57RecVE.size(); i++)  // 边记录
	{
		for (int j = 0; j < m_vecS57RecVE[i]->m_Vrpt.m_vecVrpt.size(); j++) // 边指针字段
		{
			int m = 0;
			int startNum = 0;
			int endNum = m_vecS57RecVC.size();// mArrayRecVCsize;
			int mid = 0;
			while (startNum <= endNum)
			{
				mid = (startNum + endNum) / 2;
				if (m_vecS57RecVE[i]->m_Vrpt.m_vecVrpt[j].rcid < m_vecS57RecVC[mid]->m_Vrid.rcid)
					endNum = mid - 1;
				else if (m_vecS57RecVE[i]->m_Vrpt.m_vecVrpt[j].rcid == m_vecS57RecVC[mid]->m_Vrid.rcid)
				{
					m_vecS57RecVE[i]->m_Vrpt.m_vecVrpt[j].idx = mid;
					m = 1;
					// 保存链接节点指向边的信息
					m_vecS57RecVC[mid]->vpVE.push_back(i);
					break;
				}
				else startNum = mid + 1;
			}

			if (m == 0)
			{
				QMessageBox::information(nullptr, "提示", "空间拓扑错!");
				return;
			}
		}
	}

	for (int i = 0; i < m_vecS57RecFE.size(); i++)
	{
		for (int j = 0; j < m_vecS57RecFE[i]->m_Fspt.m_vecFspt.size(); j++)
		{
			int m = 0, startNum = 0, endNum = 0, mid = 0;
			unsigned int tmpRecId = m_vecS57RecFE[i]->m_Fspt.m_vecFspt[j].rcid;
			switch (m_vecS57RecFE[i]->m_Fspt.m_vecFspt[j].rcnm)
			{
			case 110:
				endNum = m_vecS57RecVI.size();
				while (startNum <= endNum)
				{
					mid = (startNum + endNum) / 2;
					if (tmpRecId < m_vecS57RecVI[mid]->m_Vrid.rcid)
						endNum = mid - 1;
					else if (tmpRecId == m_vecS57RecVI[mid]->m_Vrid.rcid)
					{
						if (mid > m_vecS57RecVI.size())
							return;

						m_vecS57RecFE[i]->m_Fspt.m_vecFspt[j].idx = mid;
						m = 1;

						// 保存弧立节点指向要素的信息
						m_vecS57RecVI[mid]->vpFE.push_back(i);
						break;
					}
					else startNum = mid + 1;
				}
				if (m == 0)
				{
					QMessageBox::information(nullptr, "提示", "特征－孤立节点空间拓扑错!");
					return;
				}
				break;
			case 120:
				endNum = m_vecS57RecVC.size();
				while (startNum <= endNum)
				{
					mid = (startNum + endNum) / 2;
					if (tmpRecId < m_vecS57RecVC[mid]->m_Vrid.rcid)
						endNum = mid - 1;
					else if (tmpRecId == m_vecS57RecVC[mid]->m_Vrid.rcid)
					{
						m_vecS57RecFE[i]->m_Fspt.m_vecFspt[j].idx = mid;
						m = 1;
						// 保存链接节点指向要素的信息
						m_vecS57RecVC[mid]->vpFE.push_back(i);
						break;
					}
					else startNum = mid + 1;
				}
				if (m == 0)
				{
					QMessageBox::information(nullptr, "提示", "特征-链接节点空间拓扑错!");
					return;
				}
				break;
			case 130:
				endNum = m_vecS57RecVE.size();
				while (startNum <= endNum)
				{
					mid = (startNum + endNum) / 2;
					if (tmpRecId < m_vecS57RecVE[mid]->m_Vrid.rcid)
						endNum = mid - 1;
					else if (tmpRecId == m_vecS57RecVE[mid]->m_Vrid.rcid)
					{
						m_vecS57RecFE[i]->m_Fspt.m_vecFspt[j].idx = mid;
						m = 1;
						// 保存边指向要素的信息
						m_vecS57RecVE[mid]->vpFE.push_back(i);
						break;
					}
					else startNum = mid + 1;
				}
				if (m == 0)
				{
					QMessageBox::information(nullptr, "提示", "特征-链接节点空间拓扑错!");
					return;
				}
				break;
			}
		}

		// 建立特征到特征对象索引
		for (int j = 0; j < m_vecS57RecFE[i]->m_Ffpt.m_vecFfpt.size(); j++)
		{
			int m = 0;
			int startNum = 0;
			int endNum = m_vecS57RecFE.size();
			int mid = 0;
			while (startNum <= endNum)
			{
				mid = (startNum + endNum) / 2;
				if (m_vecS57RecFE[i]->m_Ffpt.m_vecFfpt[j].fidn < m_vecS57RecFE[mid]->m_Foid.fidn)
					endNum = mid - 1;
				else if (m_vecS57RecFE[i]->m_Ffpt.m_vecFfpt[j].fidn == m_vecS57RecFE[mid]->m_Foid.fidn)
				{
					if (m_vecS57RecFE[i]->m_Ffpt.m_vecFfpt[j].rind == 2)
						m_vecS57RecFE[mid]->m_nMaster = i;
					m_vecS57RecFE[i]->m_Ffpt.m_vecFfpt[j].idx = mid;
					m = 1;
					break;
				}
				else startNum = mid + 1;
			}

			if (m == 0)
			{
				QMessageBox::information(nullptr, "提示", "特征-特征拓扑错!");
				return;
			}
		}
	}
}

//************************************
// Method:    ioSencFile
// Brief:	  读写SENC文件
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: FILE * fp
// Parameter: bool ioFlag
//************************************
void CS57Cell::ioSencFile(FILE* fp, bool ioFlag)
{
	int size;
	if (ioFlag)
	{
		QString strXtmVersion = "";
		int len = 0;
		fread(&len, sizeof(int), 1, fp);
		char* pszXtmVersion = new char[len + 1];
		fread(pszXtmVersion, len, 1, fp);
		pszXtmVersion[len] = '\0';
		strXtmVersion = pszXtmVersion;
		delete[] pszXtmVersion;
		pszXtmVersion = NULL;

		if (m_strXtmVersion != strXtmVersion)
		{
			//qDebug() << "当前SENC版本与系统支持SENC版本不一致,请重新生成SENC文件并加载！" << endl;
			return;
		}

		m_pRecDsgi = new CS57RecDsgi();
		m_pRecDsgi->ioFile(fp, ioFlag);
		m_pRecDsgr = new CS57RecDsgr();
		m_pRecDsgr->ioFile(fp, ioFlag);

		fread(&size, sizeof(int), 1, fp);
		for (int i = 0; i < size; i++)
		{
			CS57RecFeature *pRecFeature = new CS57RecFeature();
			pRecFeature->setParentCell(this);
			pRecFeature->ioFile(fp, ioFlag);
			m_vecS57RecFE.push_back(pRecFeature);
		}
		fread(&size, 4, 1, fp);
		for (int i = 0; i < size; i++)
		{
			CS57RecVector* pRecVector = new CS57RecVector();
			pRecVector->ioFile(fp, ioFlag);
			m_vecS57RecVI.push_back(pRecVector);
		}

		fread(&size, 4, 1, fp);
		for (int i = 0; i < size; i++)
		{
			CS57RecVector* pRecVector = new CS57RecVector();
			pRecVector->ioFile(fp, ioFlag);
			m_vecS57RecVC.push_back(pRecVector);
		}

		fread(&size, 4, 1, fp);
		for (int i = 0; i < size; i++)
		{
			CS57RecVector* pRecVector = new CS57RecVector();
			pRecVector->ioFile(fp, ioFlag);
			m_vecS57RecVE.push_back(pRecVector);
		}

		double xMin = std::numeric_limits<double>::max();
		double yMin = std::numeric_limits<double>::max();
		double xMax = -std::numeric_limits<double>::max();
		double yMax = -std::numeric_limits<double>::max();
		fread(&xMin, sizeof(double), 1, fp);
		fread(&yMin, sizeof(double), 1, fp);
		fread(&xMax, sizeof(double), 1, fp);
		fread(&yMax, sizeof(double), 1, fp);
		m_CellRectangle.set(xMin, yMin, xMax, yMax);
	}
	else
	{
		//写XTM文件版本
		int len = m_strXtmVersion.length();
		fwrite(&len, sizeof(int), 1, fp);
		std::string stdStrXtmVersion = codec->fromUnicode(m_strXtmVersion).data();
		fwrite(stdStrXtmVersion.c_str(), len, 1, fp);

		m_pRecDsgi->ioFile(fp, ioFlag);
		m_pRecDsgr->ioFile(fp, ioFlag);
		size = m_vecS57RecFE.size();
		fwrite(&size, 4, 1, fp);
		for (int i = 0; i < size; i++)
			m_vecS57RecFE[i]->ioFile(fp, ioFlag);

		size = m_vecS57RecVI.size();
		fwrite(&size, 4, 1, fp);
		for (int i = 0; i < size; i++)
			m_vecS57RecVI[i]->ioFile(fp, ioFlag);

		size = m_vecS57RecVC.size();
		fwrite(&size, 4, 1, fp);
		for (int i = 0; i < size; i++)
			m_vecS57RecVC[i]->ioFile(fp, ioFlag);

		size = m_vecS57RecVE.size();
		fwrite(&size, 4, 1, fp);
		for (int i = 0; i < size; i++)
			m_vecS57RecVE[i]->ioFile(fp, ioFlag);

		double xMin = m_CellRectangle.xMinimum();
		double yMin = m_CellRectangle.yMinimum();
		double xMax = m_CellRectangle.xMaximum();
		double yMax = m_CellRectangle.yMaximum();

		fwrite(&xMin, sizeof(double), 1, fp);
		fwrite(&yMin, sizeof(double), 1, fp);
		fwrite(&xMax, sizeof(double), 1, fp);
		fwrite(&yMax, sizeof(double), 1, fp);
	}
}

//************************************
// Method:    close
// Brief:	  关闭图幅
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57Cell::close()
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
			delete m_vecS57RecFE[i];
		m_vecS57RecFE[i] = nullptr;
	}
	m_vecS57RecFE.clear();

	for (int i = 0; i < m_vecS57RecVE.size(); i++)
	{
		if (m_vecS57RecVE[i] != nullptr)
			delete m_vecS57RecVE[i];
		m_vecS57RecVE[i] = nullptr;
	}
	m_vecS57RecVE.clear();

	for (int i = 0; i < m_vecS57RecVC.size(); i++)
	{
		if (m_vecS57RecVC[i] != nullptr)
			delete m_vecS57RecVC[i];
		m_vecS57RecVC[i] = nullptr;
	}
	m_vecS57RecVC.clear();

	for (int i = 0; i < m_vecS57RecVI.size(); i++)
	{
		if (m_vecS57RecVI[i] != nullptr)
			delete m_vecS57RecVI[i];
		m_vecS57RecVI[i] = nullptr;
	}
	m_vecS57RecVI.clear();
}
//************************************
// Method:    getCellFullPath
// Brief:	  获取图幅全路径
// Returns:   QString
// Author:    cl
// DateTime:  2022/07/21
// Parameter: const QString & name_with_ext
//************************************
QString CS57Cell::getCellFullPath(const QString& name_with_ext) const
{
	if (m_strCellFullPath == "")
	{
		QString strEncDir = CS57DocManager::instance()->getEncDir();
		QString strCellName = name_with_ext;
		//QString strCellFullPath = strEncDir + "\\" + strCellName;
		QString strCellFullPath = strEncDir + "/" + strCellName;

		return strCellFullPath;
	}
	return m_strCellFullPath;
}

//************************************
// Method:    getSencFullPath
// Brief:	  获取SENC全路径
// Returns:   QString
// Author:    cl
// DateTime:  2022/07/21
// Parameter: const QString & name
//************************************
QString CS57Cell::getSencFullPath(const QString& name) const
{
	QString strCellFullPath = getCellFullPath(name);
	QString strSencFullPath = strCellFullPath.replace(".000", ".s57");

	return strSencFullPath;
}

bool CS57Cell::getFloating(CS57RecFeature * pFeature)
{
	for (int i = 0; i < m_vecS57RecFE.size(); i++)
	{
		if (m_vecS57RecFE[i]->m_Frid.prim == 1)
		{
			if (m_vecS57RecFE[i]->m_Frid.objl == 76 ||		// LITFLT
				m_vecS57RecFE[i]->m_Frid.objl == 77 ||		// LITVES 	
				m_vecS57RecFE[i]->m_Frid.objl == 14 ||		// BOYCAR
				m_vecS57RecFE[i]->m_Frid.objl == 15 ||		// BOYINB
				m_vecS57RecFE[i]->m_Frid.objl == 16 ||		// BOYISD
				m_vecS57RecFE[i]->m_Frid.objl == 17 ||		// BOYLAT
				m_vecS57RecFE[i]->m_Frid.objl == 18 ||		// BOYSAW
				m_vecS57RecFE[i]->m_Frid.objl == 19 ||		// BOYSPP
				m_vecS57RecFE[i]->m_Frid.objl == 84)		// MORFAC 
			{
				for (int j = 0; j < m_vecS57RecFE[i]->m_Ffpt.m_vecFfpt.size(); j++)
				{
					if (pFeature->m_Foid.agen == m_vecS57RecFE[i]->m_Ffpt.m_vecFfpt[j].agen&&
						pFeature->m_Foid.fidn == m_vecS57RecFE[i]->m_Ffpt.m_vecFfpt[j].fidn&&
						pFeature->m_Foid.fids == m_vecS57RecFE[i]->m_Ffpt.m_vecFfpt[j].fids)
					{
						if (m_vecS57RecFE[i]->m_Ffpt.m_vecFfpt[j].rind != 1)
						{
							if (m_vecS57RecFE[i]->m_Frid.objl == 14 ||		// BOYCAR
								m_vecS57RecFE[i]->m_Frid.objl == 16 ||		// BOYISD
								m_vecS57RecFE[i]->m_Frid.objl == 17 ||		// BOYLAT
								m_vecS57RecFE[i]->m_Frid.objl == 18 ||		// BOYSAW
								m_vecS57RecFE[i]->m_Frid.objl == 19 ||		// BOYSPP
								m_vecS57RecFE[i]->m_Frid.objl == 84)		// MORFAC 
								return true;
						}
					}
				}
			}
		}
	}
	return false;
}

//************************************
// 方法:    GetAssociatedFeatures
// 全称:  	Doc::CS57Cell::GetAssociatedFeatures
// 权限:    public 
// 返回值:  QVector<CS57RecFeature*> 与要素关联的所有要素集合
// 简介:	获取要素关联
// 参数:	CS57RecFeature * pFeature 要素
//************************************
QVector<CS57RecFeature*> CS57Cell::GetAssociatedFeatures(CS57RecFeature* pFeature)
{
	double comf = getCellComf();
	QVector<CS57RecFeature*> vecFeatures;
	double lat = 0.0, lon = 0.0;
	if (pFeature->m_Frid.objl == 129)
		return QVector<CS57RecFeature*>();
	if (pFeature->m_Fspt.m_vecFspt[0].rcnm == 110)
	{
		lon = m_vecS57RecVI[pFeature->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d[0].xcoo / comf;
		lat = m_vecS57RecVI[pFeature->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d[0].ycoo / comf;
	}
	else if (pFeature->m_Fspt.m_vecFspt[0].rcnm == 120)
	{
		lon = m_vecS57RecVC[pFeature->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d[0].xcoo / comf;
		lat = m_vecS57RecVC[pFeature->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d[0].ycoo / comf;
	}
	else if (pFeature->m_Fspt.m_vecFspt[0].rcnm == 130)
	{
		CS57RecVector *pVE = m_vecS57RecVE[pFeature->m_Fspt.m_vecFspt[0].idx];
		CS57RecVector *pVC = m_vecS57RecVC[pVE->m_Vrpt.m_vecVrpt[0].idx];
		lon = pVC->m_Sg2d.m_vecSg2d[0].xcoo / comf;
		lat = pVC->m_Sg2d.m_vecSg2d[0].ycoo / comf;
	}
	else
	{
		return QVector<CS57RecFeature*>();
	}
	for (int i = 0;i < m_vecS57RecFE.size();i++)
	{
		CS57RecFeature * pFE = m_vecS57RecFE[i];
		if (pFE->m_Frid.objl == 42 || pFE->m_Frid.objl == 46)
		{
			if (pFE->m_Frid.prim == 3)
			{
				QVector<QVector<QPointF>*> vecRings;
				genFeaturePolygon(pFE, &vecRings);
				CSpatialPolygon featurePolygon(vecRings);
				if (featurePolygon.ptInPolygon(lon, lat))
				{
					vecFeatures.push_back(m_vecS57RecFE[i]);
				}
				qDeleteAll(vecRings);
				vecRings.clear();
			}
		}
	}

	return vecFeatures;
}


//************************************
// Method:    releaseUpdate
// Brief:	  释放更新
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57Cell::releaseUpdate()
{
	if (m_pRecDsgiUpdate != nullptr)
		delete m_pRecDsgiUpdate;
	m_pRecDsgiUpdate = nullptr;

	for (int i = 0; i < m_vecS57RecFEUpdate.size(); i++)
	{
		if (m_vecS57RecFEUpdate[i] != nullptr)
			delete m_vecS57RecFEUpdate[i];
		m_vecS57RecFEUpdate[i] = nullptr;
	}
	m_vecS57RecFEUpdate.clear();

	for (int i = 0; i < m_vecS57RecVEUpdate.size(); i++)
	{
		if (m_vecS57RecVEUpdate[i] != nullptr)
			delete m_vecS57RecVEUpdate[i];
		m_vecS57RecVEUpdate[i] = nullptr;
	}
	m_vecS57RecVEUpdate.clear();

	for (int i = 0; i < m_vecS57RecVCUpdate.size(); i++)
	{
		if (m_vecS57RecVCUpdate[i] != nullptr)
			delete m_vecS57RecVCUpdate[i];
		m_vecS57RecVCUpdate[i] = nullptr;
	}
	m_vecS57RecVCUpdate.clear();

	for (int i = 0; i < m_vecS57RecVIUpdate.size(); i++)
	{
		if (m_vecS57RecVIUpdate[i] != nullptr)
			delete m_vecS57RecVIUpdate[i];
		m_vecS57RecVIUpdate[i] = nullptr;
	}
	m_vecS57RecVIUpdate.clear();
}

//************************************
// Method:    updateCell
// Brief:	  更新图幅
// Returns:   int
// Author:    cl
// DateTime:  2022/07/21
//************************************
int CS57Cell::updateCell()
{
	CS57RecFeature* pFeature = nullptr;					// S57数据集要素记录
	CS57RecVector* pVector = nullptr;					// S57数据集矢量记录
	CS57RecFeature* pFeatureER = nullptr;				// S57更新数据集要素记录
	CS57RecVector* pVectorER = nullptr;					// S57更新数据集矢量记录

	int nUpdnEr = 0, nUpdnEn = 0;
	nUpdnEr = m_pRecDsgiUpdate->m_Dsid.updn.toInt();
	nUpdnEn = m_pRecDsgi->m_Dsid.updn.toInt();

	if (nUpdnEr <= nUpdnEn)
	{
		QMessageBox::information(nullptr, "提示", "数据文件已升级");
		return 2;			// 返回值为错误代码,2 原始数据文件已升级;
	}
	if (nUpdnEr > nUpdnEn + 1)
	{
		QMessageBox::information(nullptr, "提示", "数据文件文件版本不能越级升级");
		return 3;			// 返回值为错误代码,3 不能越级升级;
	}

	m_pRecDsgi->m_Dsid.updn = m_pRecDsgiUpdate->m_Dsid.updn;
	strcpy(m_pRecDsgi->m_Dsid.uadt, m_pRecDsgiUpdate->m_Dsid.isdt);// 将升级文件的出版时间付给数据集文件的更新日期

	//  更新地理要素
	for (int i = 0; i < m_vecS57RecFEUpdate.size(); i++)
	{
		pFeatureER = m_vecS57RecFEUpdate[i];

		switch (pFeatureER->m_Frid.ruin)
		{
		case 1:		// 插入特征要素记录
			addFeature(pFeatureER);
			break;
		case 2:		// 删除特征要素记录
			delFeature(pFeatureER);
			break;
		case 3:		// 修改特征记录
			for (int j = 0; j < m_vecS57RecFE.size(); j++)																				// 目标数据要素对象循环
			{
				pFeature = m_vecS57RecFE[j];								// 目标数据要素对获取
				if (pFeature->m_Frid.rcid == pFeatureER->m_Frid.rcid)		// 更新数据与目标数据要素比对
				{
					applyRecordUpdate(pFeature, pFeatureER);
				}
			}
			break;
		default:
			QMessageBox::information(nullptr, "提示", "更新数据记录的记录修改指令非法");
			break;
		}
	}

	//	更新空间要素-弧立节点
	for (int i = 0; i < m_vecS57RecVIUpdate.size(); i++)
	{
		pVectorER = m_vecS57RecVIUpdate[i];
		switch (pVectorER->m_Vrid.ruin)
		{
		case 1:		// 插入矢量记录－孤立节点
			addVector(pVectorER);
			break;
		case 2:		// 删除矢量记录－孤立节点
			delVector(pVectorER);
			break;
		case 3:		// 修改记录
			for (int j = 0; j < m_vecS57RecVI.size(); j++)
			{
				pVector = m_vecS57RecVI[j];								// 目标数据要素对获取
				if (pVector->m_Vrid.rcid == pVectorER->m_Vrid.rcid)								// 更新数据与目标数据要素比对
				{
					applyRecordUpdate(pVector, pVectorER);
				}
			}
			break;
		}
	}
	//	更新空间要素-链接节点
	for (int i = 0; i < m_vecS57RecVCUpdate.size(); i++)
	{
		pVectorER = m_vecS57RecVCUpdate[i];
		switch (pVectorER->m_Vrid.ruin)
		{
		case 1:		// 插入矢量记录－链接节点
			addVector(pVectorER);
			break;
		case 2:		// 删除矢量记录－链接节点
			delVector(pVectorER);
			break;
		case 3:		// 修改矢量记录－链接节点
			for (int j = 0; j < m_vecS57RecVC.size(); j++)
			{
				pVector = m_vecS57RecVC[j];								// 目标数据要素对获取
				if (pVector->m_Vrid.rcid == pVectorER->m_Vrid.rcid)								// 更新数据与目标数据要素比对
				{
					applyRecordUpdate(pVector, pVectorER);
				}
			}
			break;
		}
	}
	//	更新空间要素-边
	for (int i = 0; i < m_vecS57RecVEUpdate.size(); i++)
	{
		pVectorER = m_vecS57RecVEUpdate[i];
		switch (pVectorER->m_Vrid.ruin)
		{
		case 1:		// 插入矢量记录－边
			addVector(pVectorER);
			break;
		case 2:		// 删除矢量记录－边
			delVector(pVectorER);
			break;
		case 3:		// 修改矢量记录－边
			for (int j = 0; j < m_vecS57RecVC.size(); j++)
			{
				pVector = m_vecS57RecVE[j];								// 目标数据要素对获取
				if (pVector->m_Vrid.rcid == pVectorER->m_Vrid.rcid)								// 更新数据与目标数据要素比对
				{
					applyRecordUpdate(pVector, pVectorER);
				}
			}
			break;
		}
	}
	indexCell();
	return 0;
}

//************************************
// Method:    addFeature
// Brief:	  添加要素到图幅
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CS57RecFeature * pFeature
//************************************
void CS57Cell::addFeature(CS57RecFeature* pFeature)
{
	CS57RecFeature *pFE = new CS57RecFeature();
	pFE->copy(pFeature);
	m_vecS57RecFE.push_back(pFE);
}

//************************************
// Method:    delFeature
// Brief:	  从图幅删除要素
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CS57RecFeature * pFeature
//************************************
void CS57Cell::delFeature(CS57RecFeature* pFeature)
{
	for (int i = 0; i < m_vecS57RecFE.size(); i++)
	{
		if (pFeature->m_Frid.rcid == m_vecS57RecFE[i]->m_Frid.rcid)
		{
			delete m_vecS57RecFE[i];
			m_vecS57RecFE.erase(m_vecS57RecFE.begin() + i);
			return;
		}
	}
}
//************************************
// Method:    applyRecordUpdate
// Brief:	  应用记录更新
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CS57RecFeature * pFeature
// Parameter: CS57RecFeature * pFeatureUpdate
//************************************
void CS57Cell::applyRecordUpdate(CS57RecFeature* pFeature, CS57RecFeature *pFeatureUpdate)
{
	if (pFeature->m_Frid.rver + 1 != pFeatureUpdate->m_Frid.rver)
	{
		QMessageBox::information(nullptr, "提示", "特征记录版本错误");
		return;
	}
	if (pFeature->m_Frid.prim != pFeatureUpdate->m_Frid.prim)
	{
		QMessageBox::information(nullptr, "提示", "特征记录几何错误");
		return;
	}
	// 更新版本
	pFeature->m_Frid.rver++;

	/* -------------------------------------------------------------------- */
	/*      Check for, and apply record record to spatial record pointer    */
	/*      updates.                                                        */
	/* -------------------------------------------------------------------- */
	if (pFeatureUpdate->m_Fspc.m_vecFspc.size() == 1)
	{
		int nFSUI = pFeatureUpdate->m_Fspc.m_vecFspc[0].fsut;
		int nFSIX = pFeatureUpdate->m_Fspc.m_vecFspc[0].fsix;
		int nNSPT = pFeatureUpdate->m_Fspc.m_vecFspc[0].nspt;
		CS57FieldFSPT *pSrcFSPT = &pFeatureUpdate->m_Fspt;
		CS57FieldFSPT *pDstFSPT = &pFeature->m_Fspt;

		if (pSrcFSPT->m_vecFspt.size() == 0 && nFSUI != 2)
			return;

		bool idDelete = false;
		int num = 0;
		//CS57FieldFSPT::sFspt* pFspt = nullptr;
		switch (nFSUI)
		{
		case 1:/* INSERT */// 插入FSPT子字段
		{
			if (nNSPT > pFeatureUpdate->m_Fspt.m_vecFspt.size())
			{
				QMessageBox::information(nullptr, "提示", "更新记录特征记录指针数错误");
				nNSPT = pFeatureUpdate->m_Fspt.m_vecFspt.size();
			}	
			QVector<CS57FieldFSPT::sFspt> vecFspt;
			for (int i = 0; i < pDstFSPT->m_vecFspt.size(); i++)
			{
				CS57FieldFSPT::sFspt oFspt;
				oFspt.mask = pDstFSPT->m_vecFspt[i].mask;
				oFspt.ornt = pDstFSPT->m_vecFspt[i].ornt;
				oFspt.rcid = pDstFSPT->m_vecFspt[i].rcid;
				oFspt.rcnm = pDstFSPT->m_vecFspt[i].rcnm;
				oFspt.usag = pDstFSPT->m_vecFspt[i].usag;
				vecFspt.push_back(oFspt);

			}
			for (int i = 0; i < pSrcFSPT->m_vecFspt.size(); i++)
			{
				CS57FieldFSPT::sFspt oFspt;
				oFspt.mask = pSrcFSPT->m_vecFspt[i].mask;
				oFspt.ornt = pSrcFSPT->m_vecFspt[i].ornt;
				oFspt.rcid = pSrcFSPT->m_vecFspt[i].rcid;
				oFspt.rcnm = pSrcFSPT->m_vecFspt[i].rcnm;
				oFspt.usag = pSrcFSPT->m_vecFspt[i].usag;
				vecFspt.push_back(oFspt);
			}

			pFeature->m_Fspt.m_vecFspt.clear();

			for (int i = 0; i < vecFspt.size(); i++)
			{
				pFeature->m_Fspt.m_vecFspt.push_back(vecFspt[i]);
			}
			break;
		}
		case 2: /* DELETE */
		{
			QVector<CS57FieldFSPT::sFspt> vecFspt;
			num = 0;
			for (int i = 0; i < pDstFSPT->m_vecFspt.size(); i++)
			{
				for (int j = nNSPT - 1; j >= 0; j--)
				{
					if (i == j + nFSIX - 1)
						continue;
				}
				if (num >= pDstFSPT->m_vecFspt.size() - nNSPT)
				{
					QMessageBox::information(nullptr, "提示", "特征记录删除FSPT指针错");
					continue;
				}
				CS57FieldFSPT::sFspt oFspt;
				oFspt.mask = pDstFSPT->m_vecFspt[i].mask;
				oFspt.ornt = pDstFSPT->m_vecFspt[i].ornt;
				oFspt.rcid = pDstFSPT->m_vecFspt[i].rcid;
				oFspt.rcnm = pDstFSPT->m_vecFspt[i].rcnm;
				oFspt.usag = pDstFSPT->m_vecFspt[i].usag;
				vecFspt.push_back(oFspt);
				num++;
			}
			pFeature->m_Fspt.m_vecFspt.clear();
			for (int i = 0; i < vecFspt.size(); i++)
			{
				pFeature->m_Fspt.m_vecFspt.push_back(vecFspt[i]);
			}
			break;
		}

		case 3:// 修改FSPT字段
			for (int i = 0; i < pDstFSPT->m_vecFspt.size(); i++)
			{
				for (int j = nNSPT - 1; j >= 0; j--)
				{
					if (i == j + nFSIX - 1)
					{
						pDstFSPT->m_vecFspt[i].mask = pSrcFSPT->m_vecFspt[j].mask;
						pDstFSPT->m_vecFspt[i].ornt = pSrcFSPT->m_vecFspt[j].ornt;
						pDstFSPT->m_vecFspt[i].rcid = pSrcFSPT->m_vecFspt[j].rcid;
						pDstFSPT->m_vecFspt[i].rcnm = pSrcFSPT->m_vecFspt[j].rcnm;
						pDstFSPT->m_vecFspt[i].usag = pSrcFSPT->m_vecFspt[j].usag;
					}
				}
			}
			break;
		}
	}

	if (pFeatureUpdate->m_Ffpc.m_vecFfpc.size() == 1)
	{
		int nFFUI = pFeatureUpdate->m_Ffpc.m_vecFfpc[0].ffui;
		int nFFIX = pFeatureUpdate->m_Ffpc.m_vecFfpc[0].ffix;
		int nFFPT = pFeatureUpdate->m_Ffpc.m_vecFfpc[0].nfpt;
		CS57FieldFFPT *pSrcFFPT = &pFeatureUpdate->m_Ffpt;
		CS57FieldFFPT *pDstFFPT = &pFeature->m_Ffpt;

		if (pSrcFFPT->m_vecFfpt.size() == 0 && nFFUI != 2)
		{
			return;
		}

		int num = 0;
		//CS57FieldFFPT::sFfpt* pFfpt = nullptr;
		switch (nFFUI)
		{
		case 1:/* INSERT */// 插入FSPT子字段
		{
			if (nFFPT > pFeatureUpdate->m_Ffpt.m_vecFfpt.size())
			{
				QMessageBox::information(nullptr, "提示", "更新记录特征记录指针数错误");
				nFFPT = pFeatureUpdate->m_Ffpt.m_vecFfpt.size();
			}
			QVector<CS57FieldFFPT::sFfpt> vecFfpt;
			for (int i = 0; i < pDstFFPT->m_vecFfpt.size(); i++)
			{
				CS57FieldFFPT::sFfpt oFfpt;
				oFfpt.agen = pDstFFPT->m_vecFfpt[i].agen;
				oFfpt.fidn = pDstFFPT->m_vecFfpt[i].fidn;
				oFfpt.fids = pDstFFPT->m_vecFfpt[i].fids;
				oFfpt.rind = pDstFFPT->m_vecFfpt[i].rind;
				oFfpt.comt = pDstFFPT->m_vecFfpt[i].comt;
				vecFfpt.push_back(oFfpt);

			}
			for (int i = 0; i < nFFPT; i++)
			{
				CS57FieldFFPT::sFfpt oFfpt;
				oFfpt.agen = pSrcFFPT->m_vecFfpt[i].agen;
				oFfpt.fidn = pSrcFFPT->m_vecFfpt[i].fidn;
				oFfpt.fids = pSrcFFPT->m_vecFfpt[i].fids;
				oFfpt.rind = pSrcFFPT->m_vecFfpt[i].rind;
				oFfpt.comt = pSrcFFPT->m_vecFfpt[i].comt;
				vecFfpt.push_back(oFfpt);
			}
			pFeature->m_Ffpt.m_vecFfpt.clear();
			for (int i = 0; i < vecFfpt.size(); i++)
			{
				pFeature->m_Ffpt.m_vecFfpt.push_back(vecFfpt[i]);
			}
			break;
		}
		case 2: /* DELETE */
		{
			QVector<CS57FieldFFPT::sFfpt> vecFfpt;
			for (int i = 0; i < pDstFFPT->m_vecFfpt.size(); i++)
			{
				for (int j = nFFPT - 1; j >= 0; j--)
				{
					if (i == j + nFFIX - 1)
					{
						continue;
					}
				}

				if (num >= pDstFFPT->m_vecFfpt.size() - nFFPT)
				{
					QMessageBox::information(nullptr, "提示", "特征记录删除FSPT指针错");
					continue;
				}
				CS57FieldFFPT::sFfpt oFfpt;
				oFfpt.agen = pDstFFPT->m_vecFfpt[i].agen;
				oFfpt.fidn = pDstFFPT->m_vecFfpt[i].fidn;
				oFfpt.fids = pDstFFPT->m_vecFfpt[i].fids;
				oFfpt.rind = pDstFFPT->m_vecFfpt[i].rind;
				oFfpt.comt = pDstFFPT->m_vecFfpt[i].comt;
				vecFfpt.push_back(oFfpt);
				num++;
			}
			pFeature->m_Ffpt.m_vecFfpt.clear();
			for (int i = 0; i < vecFfpt.size(); i++)
			{
				pFeature->m_Ffpt.m_vecFfpt.push_back(vecFfpt[i]);
			}
			break;
		}
		case 3:// 修改FSPT字段
			for (int i = 0; i < pDstFFPT->m_vecFfpt.size(); i++)
			{
				for (int j = nFFPT - 1; j >= 0; j--)
				{
					if (i == j + nFFIX - 1)
					{
						pDstFFPT->m_vecFfpt[i].agen = pSrcFFPT->m_vecFfpt[j].agen;
						pDstFFPT->m_vecFfpt[i].fidn = pSrcFFPT->m_vecFfpt[j].fidn;
						pDstFFPT->m_vecFfpt[i].fids = pSrcFFPT->m_vecFfpt[j].fids;
						pDstFFPT->m_vecFfpt[i].rind = pSrcFFPT->m_vecFfpt[j].rind;
						pDstFFPT->m_vecFfpt[i].comt = pSrcFFPT->m_vecFfpt[j].comt;
					}
				}
			}
			break;
		}
	}
	/* -------------------------------------------------------------------- */
	/*      Check for and apply changes to attribute lists.                 */
	/* -------------------------------------------------------------------- */

	if (pFeatureUpdate->m_Attf.m_vecAttf.size() > 0)
	{
		bool b_newField = false;
		CS57FieldATTF *pSrcATTF = &pFeatureUpdate->m_Attf;
		CS57FieldATTF *pDstATTF = &pFeature->m_Attf;

		if (pDstATTF->m_vecAttf.size() == 0)
		{
			//  This probably means that the update applies to an attribute that doesn't (yet) exist
			//  To fix, we need to add an attribute, then update it.
			for (int i = 0; i < pSrcATTF->m_vecAttf.size(); i++)
			{
				CS57FieldATTF::sAttf oAttf;
				oAttf.attl = pSrcATTF->m_vecAttf[i].attl;
				oAttf.atvl = pSrcATTF->m_vecAttf[i].atvl;
				pDstATTF->m_vecAttf.push_back(oAttf);
			}
		}
		else
		{
			QVector<CS57FieldATTF::sAttf> vecAttf;
			for (int i = 0; i < pSrcATTF->m_vecAttf.size(); i++)
			{
				b_newField = true;
				for (int j = 0; j < pDstATTF->m_vecAttf.size(); j++)
				{
					if (pSrcATTF->m_vecAttf[i].attl == pDstATTF->m_vecAttf[j].attl)
					{
						pDstATTF->m_vecAttf[j].atvl = pSrcATTF->m_vecAttf[i].atvl;
						b_newField = false;
						break;
					}
				}
				if (b_newField)
					vecAttf.push_back(pSrcATTF->m_vecAttf[i]);
			}
			if (vecAttf.size() > 0)
			{
				int num = 0;
				QVector<CS57FieldATTF::sAttf> vecTaAttf;
				for (int i = 0; i < vecAttf.size() + pDstATTF->m_vecAttf.size(); i++)
				{
					if (i < pDstATTF->m_vecAttf.size())
					{
						CS57FieldATTF::sAttf oAttf;
						oAttf.attl = pDstATTF->m_vecAttf[i].attl;
						oAttf.atvl = pDstATTF->m_vecAttf[i].atvl;
						vecTaAttf.push_back(oAttf);
					}
					else
					{
						CS57FieldATTF::sAttf oAttf;
						oAttf.attl = vecAttf[num].attl;
						oAttf.atvl = vecAttf[num].atvl;
						vecTaAttf.push_back(oAttf);
						num++;
					}
				}
				pDstATTF->m_vecAttf.clear();
				for (int i = 0; i < vecTaAttf.size(); i++)
				{
					pDstATTF->m_vecAttf.push_back(vecTaAttf[i]);
				}
			}
		}
	}

	/* -------------------------------------------------------------------- */
	/*      Check for and apply changes to attribute lists.                 */
	/* -------------------------------------------------------------------- */

	if (pFeatureUpdate->m_Natf.m_vecNatf.size() > 0)
	{
		bool b_newField = false;
		CS57FieldNATF *pSrcNATF = &pFeatureUpdate->m_Natf;
		CS57FieldNATF *pDstNATF = &pFeature->m_Natf;

		if (pDstNATF->m_vecNatf.size() == 0)
		{
			//  This probably means that the update applies to an attribute that doesn't (yet) exist
			//  To fix, we need to add an attribute, then update it.
			for (int i = 0; i < pSrcNATF->m_vecNatf.size(); i++)
			{
				CS57FieldNATF::sNatf oNatf;
				pDstNATF->m_vecNatf[i].attl = pSrcNATF->m_vecNatf[i].attl;
				pDstNATF->m_vecNatf[i].atvl = _strdup(pSrcNATF->m_vecNatf[i].atvl);
				pDstNATF->m_vecNatf[i].len = strlen(pDstNATF->m_vecNatf[i].atvl);
				pDstNATF->m_vecNatf.push_back(oNatf);
			}
		}
		else
		{
			QVector<CS57FieldNATF::sNatf> tmpNATF;
			for (int i = 0; i < pSrcNATF->m_vecNatf.size(); i++)
			{
				b_newField = true;
				for (int j = 0; j < pDstNATF->m_vecNatf.size(); j++)
				{
					if (pSrcNATF->m_vecNatf[i].attl == pDstNATF->m_vecNatf[j].attl)
					{
						delete[]pDstNATF->m_vecNatf[j].atvl;
						pDstNATF->m_vecNatf[j].atvl = _strdup(pSrcNATF->m_vecNatf[i].atvl);
						pDstNATF->m_vecNatf[j].len = strlen(pDstNATF->m_vecNatf[j].atvl);
						b_newField = false;
						break;
					}
				}
				if (b_newField)
					tmpNATF.push_back(pSrcNATF->m_vecNatf[i]);
			}
			if (tmpNATF.size() > 0)
			{
				int num = 0;
				QVector<CS57FieldNATF::sNatf> vecTaNatf;
				for (int i = 0; i < tmpNATF.size() + pDstNATF->m_vecNatf.size(); i++)
				{
					if (i < pDstNATF->m_vecNatf.size())
					{
						CS57FieldNATF::sNatf oNatf;
						oNatf.attl = pDstNATF->m_vecNatf[i].attl;
						oNatf.atvl = _strdup(pDstNATF->m_vecNatf[i].atvl);
						oNatf.len = strlen(oNatf.atvl);
						vecTaNatf.push_back(oNatf);
					}
					else
					{
						CS57FieldNATF::sNatf oNatf;
						oNatf.attl = tmpNATF[num].attl;
						oNatf.atvl = _strdup(tmpNATF[num].atvl);
						oNatf.len = strlen(oNatf.atvl);
						vecTaNatf.push_back(oNatf);
						num++;
					}
				}
				for (int i = 0; i < pDstNATF->m_vecNatf.size(); i++)
				{
					delete[] pDstNATF->m_vecNatf[i].atvl;
					pDstNATF->m_vecNatf[i].atvl = nullptr;
				}
				pDstNATF->m_vecNatf.clear();
				for (int i = 0; i < vecTaNatf.size(); i++)
				{
					pDstNATF->m_vecNatf.push_back(vecTaNatf[i]);
				}
			}
		}
	}
}

//************************************
// Method:    addVector
// Brief:	  添加空间
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CS57RecVector * pRecVector
//************************************
void CS57Cell::addVector(CS57RecVector* pRecVector)
{
	CS57RecVector * tmpVector = new CS57RecVector;
	tmpVector->copy(pRecVector);
	switch (pRecVector->m_Vrid.rcnm)
	{
	case 110:
		m_vecS57RecVI.push_back(tmpVector);
		break;
	case 120:
		m_vecS57RecVC.push_back(tmpVector);
		break;
	case 130:
		m_vecS57RecVE.push_back(tmpVector);
		break;
	default:
		break;
	}
}

//************************************
// Method:    delVector
// Brief:	  删除空间
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CS57RecVector * pRecVector
//************************************
void CS57Cell::delVector(CS57RecVector* pRecVector)
{
	switch (pRecVector->m_Vrid.rcnm)
	{
	case 110:
		for (int i = 0; i < m_vecS57RecVI.size(); i++)
		{

			if (m_vecS57RecVI[i]->m_Vrid.rcid == pRecVector->m_Vrid.rcid)
			{
				delete m_vecS57RecVI[i];
				m_vecS57RecVI.erase(m_vecS57RecVI.begin() + i);
			}
		}
		break;
	case 120:
		for (int i = 0; i < m_vecS57RecVC.size(); i++)
		{
			if (m_vecS57RecVC[i]->m_Vrid.rcid == pRecVector->m_Vrid.rcid)
			{
				delete m_vecS57RecVC[i];
				m_vecS57RecVC.erase(m_vecS57RecVC.begin() + i);
			}
		}
		break;
	case 130:
		for (int i = 0; i < m_vecS57RecVE.size(); i++)
		{
			if (m_vecS57RecVE[i]->m_Vrid.rcid == pRecVector->m_Vrid.rcid)
			{
				delete m_vecS57RecVE[i];
				m_vecS57RecVE.erase(m_vecS57RecVE.begin() + i);
			}
		}
		break;
	default:
		break;
	}
}
//************************************
// Method:    applyRecordUpdate
// Brief:	  应用记录更新
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CS57RecVector * pVector
// Parameter: CS57RecVector * pVectorUpdate
//************************************
void CS57Cell::applyRecordUpdate(CS57RecVector * pVector, CS57RecVector *pVectorUpdate)
{
	/* -------------------------------------------------------------------- */
	/*      Check for, and apply vector record to vector record pointer     */
	/*      updates.                                                        */
	/* -------------------------------------------------------------------- */
	if (pVectorUpdate->m_Vrpt.m_vecVrpt.size() == 1)
	{
		int     nVPUI = pVectorUpdate->m_Vrpc.vpui;
		int     nVPIX = pVectorUpdate->m_Vrpc.vpix;
		int     nNVPT = pVectorUpdate->m_Vrpc.nvpt;
		CS57FieldVRPT *pSrcVRPT = &pVectorUpdate->m_Vrpt;
		CS57FieldVRPT *pDstVRPT = &pVector->m_Vrpt;

		if ((pSrcVRPT->m_vecVrpt.size() == 0 && nVPUI != 2) || 
			pDstVRPT->m_vecVrpt.size() == 0)
		{
			return;
		}

		int num = 0;
		switch (nVPUI)
		{
		case 1:
		{
			QVector<CS57FieldVRPT::sVrpt> vecVrpt;
			if (nNVPT > pVectorUpdate->m_Vrpt.m_vecVrpt.size())
			{
				QMessageBox::information(nullptr, "提示", "更新记录特征记录指针数错误");
				nNVPT = pVectorUpdate->m_Vrpt.m_vecVrpt.size();
			}
			num = 0;
			for (int i = 0; i < pDstVRPT->m_vecVrpt.size(); i++)
			{
				CS57FieldVRPT::sVrpt oVrpt;
				oVrpt.mask = pDstVRPT->m_vecVrpt[i].mask;
				oVrpt.ornt = pDstVRPT->m_vecVrpt[i].ornt;
				oVrpt.rcid = pDstVRPT->m_vecVrpt[i].rcid;
				oVrpt.rcnm = pDstVRPT->m_vecVrpt[i].rcnm;
				oVrpt.usag = pDstVRPT->m_vecVrpt[i].usag;
				oVrpt.topi = pDstVRPT->m_vecVrpt[i].topi;
				vecVrpt.push_back(oVrpt);

			}
			for (int i = 0; i < pSrcVRPT->m_vecVrpt.size(); i++)
			{
				CS57FieldVRPT::sVrpt oVrpt;
				oVrpt.mask = pSrcVRPT->m_vecVrpt[i].mask;
				oVrpt.ornt = pSrcVRPT->m_vecVrpt[i].ornt;
				oVrpt.rcid = pSrcVRPT->m_vecVrpt[i].rcid;
				oVrpt.rcnm = pSrcVRPT->m_vecVrpt[i].rcnm;
				oVrpt.usag = pSrcVRPT->m_vecVrpt[i].usag;
				oVrpt.topi = pSrcVRPT->m_vecVrpt[i].topi;
				vecVrpt.push_back(oVrpt);
			}
			pVector->m_Vrpt.m_vecVrpt.clear();
			for (int i = 0; i < vecVrpt.size(); i++)
			{
				pVector->m_Vrpt.m_vecVrpt.push_back(vecVrpt[i]);
			}
			break;
		}
		case 2:
		{
			QVector<CS57FieldVRPT::sVrpt> vecVrpt;
			num = 0;
			for (int i = 0; i < pDstVRPT->m_vecVrpt.size(); i++)
			{
				for (int j = nNVPT - 1; j >= 0; j--)
				{
					if (i == j + nVPIX - 1)
						continue;
				}

				if (num >= pDstVRPT->m_vecVrpt.size() - nNVPT)
				{
					QMessageBox::information(nullptr, "提示", "空间记录删除VRPT指针错！");
					continue;
				}
				CS57FieldVRPT::sVrpt oVrpt;
				oVrpt.mask = pDstVRPT->m_vecVrpt[i].mask;
				oVrpt.ornt = pDstVRPT->m_vecVrpt[i].ornt;
				oVrpt.rcid = pDstVRPT->m_vecVrpt[i].rcid;
				oVrpt.rcnm = pDstVRPT->m_vecVrpt[i].rcnm;
				oVrpt.usag = pDstVRPT->m_vecVrpt[i].usag;
				oVrpt.topi = pDstVRPT->m_vecVrpt[i].topi;
				vecVrpt.push_back(oVrpt);
				num++;
			}
			pVector->m_Vrpt.m_vecVrpt.clear();
			for (int i = 0; i < vecVrpt.size(); i++)
			{
				pVector->m_Vrpt.m_vecVrpt.push_back(vecVrpt[i]);
			}
			break;
		}
		case 3:
			for (int i = 0; i < pDstVRPT->m_vecVrpt.size(); i++)
			{
				for (int j = nNVPT - 1; j >= 0; j--)
				{
					if (i == j + nVPIX - 1)
					{
						pDstVRPT->m_vecVrpt[i].mask = pSrcVRPT->m_vecVrpt[j].mask;
						pDstVRPT->m_vecVrpt[i].ornt = pSrcVRPT->m_vecVrpt[j].ornt;
						pDstVRPT->m_vecVrpt[i].rcid = pSrcVRPT->m_vecVrpt[j].rcid;
						pDstVRPT->m_vecVrpt[i].rcnm = pSrcVRPT->m_vecVrpt[j].rcnm;
						pDstVRPT->m_vecVrpt[i].usag = pSrcVRPT->m_vecVrpt[j].usag;
						pDstVRPT->m_vecVrpt[i].topi = pSrcVRPT->m_vecVrpt[j].topi;
					}
				}
			}
			break;
		}
	}
	/* -------------------------------------------------------------------- */
	/*      Check for, and apply record update to coordinates.              */
	/* -------------------------------------------------------------------- */
	if (pVectorUpdate->m_Sgcc.numFields == 1)
	{
		int nCCUI = pVectorUpdate->m_Sgcc.ccui;
		int nCCIX = pVectorUpdate->m_Sgcc.ccix;
		int nCCNC = pVectorUpdate->m_Sgcc.ccnc;
		CS57FieldSG2D* pSrcSG2D = &pVectorUpdate->m_Sg2d;
		CS57FieldSG2D* pDstSG2D = &pVector->m_Sg2d;
		CS57FieldSG3D* pSrcSG3D = &pVectorUpdate->m_Sg3d;
		CS57FieldSG3D* pDstSG3D = &pVector->m_Sg3d;
		switch (nCCUI)
		{
		case 1:
			if (pSrcSG2D->m_vecSg2d.size() > 0)
			{
				QVector<CS57FieldSG2D::sSg2d> vecSg2d;
				if (nCCNC > pVectorUpdate->m_Sg2d.m_vecSg2d.size())
				{
					QMessageBox::information(nullptr, "提示", "更新记录SG3D指针数错误！");
					nCCNC = pVectorUpdate->m_Sg2d.m_vecSg2d.size();
				}
				for (int i = 0; i < pDstSG2D->m_vecSg2d.size(); i++)
				{
					CS57FieldSG2D::sSg2d oSg2d;
					oSg2d.xcoo = pDstSG2D->m_vecSg2d[i].xcoo;
					oSg2d.ycoo = pDstSG2D->m_vecSg2d[i].ycoo;
					vecSg2d.push_back(oSg2d);
				}
				for (int i = 0; i < pSrcSG2D->m_vecSg2d.size(); i++)
				{
					CS57FieldSG2D::sSg2d oSg2d;
					oSg2d.xcoo = pSrcSG2D->m_vecSg2d[i].xcoo;
					oSg2d.ycoo = pSrcSG2D->m_vecSg2d[i].ycoo;
					vecSg2d.push_back(oSg2d);
				}
				pVector->m_Sg2d.m_vecSg2d.clear();
				for (int i = 0; i < vecSg2d.size(); i++)
				{
					pVector->m_Sg2d.m_vecSg2d.push_back(vecSg2d[i]);
				}
			}
			else if (pSrcSG3D->m_vecSg3d.size() > 0)
			{
				QVector<CS57FieldSG3D::sSg3d> vecSg3d;
				if (nCCNC > pVectorUpdate->m_Sg3d.m_vecSg3d.size())
				{
					QMessageBox::information(nullptr, "提示", "更新记录SG3D指针数错误！");
					nCCNC = pVectorUpdate->m_Sg3d.m_vecSg3d.size();
				}
				for (int i = 0; i < pDstSG3D->m_vecSg3d.size(); i++)
				{
					CS57FieldSG3D::sSg3d oSg3d;
					oSg3d.xcoo = pDstSG3D->m_vecSg3d[i].xcoo;
					oSg3d.ycoo = pDstSG3D->m_vecSg3d[i].ycoo;
					oSg3d.ve3d = pDstSG3D->m_vecSg3d[i].ve3d;
					vecSg3d.push_back(oSg3d);
				}
				for (int i = 0; i < pSrcSG3D->m_vecSg3d.size(); i++)
				{
					CS57FieldSG3D::sSg3d oSg3d;
					oSg3d.xcoo = pSrcSG3D->m_vecSg3d[i].xcoo;
					oSg3d.ycoo = pSrcSG3D->m_vecSg3d[i].ycoo;
					oSg3d.ve3d = pSrcSG3D->m_vecSg3d[i].ve3d;
					vecSg3d.push_back(oSg3d);
				}
				pVector->m_Sg3d.m_vecSg3d.clear();
				for (int i = 0; i < vecSg3d.size(); i++)
				{
					pVector->m_Sg3d.m_vecSg3d.push_back(vecSg3d[i]);
				}
			}
			break;
		case 2:
			if (pVector->m_Sg2d.m_vecSg2d.size() > 0)
			{
				QVector<CS57FieldSG2D::sSg2d> vecSg2d;
				int num = 0;
				bool flag = false;
				for (int i = 0; i < pDstSG2D->m_vecSg2d.size(); i++)
				{
					flag = false;
					for (int j = nCCNC - 1; j >= 0; j--)
					{
						if (i == (j + nCCIX - 1))
						{
							flag = true;
							break;
						}
					}
					if (flag)
						continue;
					if (num >= pDstSG2D->m_vecSg2d.size() - nCCNC)
					{
						QMessageBox::information(nullptr, "提示", "空间记录删除SG2D指针错！");
						continue;
					}
					CS57FieldSG2D::sSg2d oSg2d;
					oSg2d.xcoo = pDstSG2D->m_vecSg2d[i].xcoo;
					oSg2d.ycoo = pDstSG2D->m_vecSg2d[i].ycoo;
					vecSg2d.push_back(oSg2d);
					num++;
				}
				pVector->m_Sg2d.m_vecSg2d.clear();
				for (int i = 0; i < vecSg2d.size(); i++)
				{
					pVector->m_Sg2d.m_vecSg2d.push_back(vecSg2d[i]);
				}
			}
			else if (pVector->m_Sg3d.m_vecSg3d.size() > 0)
			{
				QVector<CS57FieldSG3D::sSg3d> vecSg3d;
				int num = 0;
				bool flag = false;
				for (int i = 0; i < pDstSG3D->m_vecSg3d.size(); i++)
				{
					for (int j = nCCNC - 1; j >= 0; j--)
					{
						if (i == (j + nCCIX - 1))
						{
							flag = true;
							break;
						}
					}
					if (flag)
						continue;

					if (num >= pDstSG3D->m_vecSg3d.size() - nCCNC)
					{
						QMessageBox::information(nullptr, "提示", "空间记录删除SG3D指针错！");
						continue;
					}
					CS57FieldSG3D::sSg3d oSg3d;
					oSg3d.xcoo = pDstSG3D->m_vecSg3d[i].xcoo;
					oSg3d.ycoo = pDstSG3D->m_vecSg3d[i].ycoo;
					oSg3d.ve3d = pDstSG3D->m_vecSg3d[i].ve3d;
					vecSg3d.push_back(oSg3d);
					num++;
				}
				pVector->m_Sg3d.m_vecSg3d.clear();
				for (int i = 0; i < vecSg3d.size(); i++)
				{
					pVector->m_Sg3d.m_vecSg3d.push_back(vecSg3d[i]);
				}
			}
			break;
		case 3:
			for (int i = 0; i < pDstSG2D->m_vecSg2d.size(); i++)
			{
				for (int j = nCCNC - 1; j >= 0; j--)
				{
					if (i == j + nCCIX - 1)
					{
						pDstSG2D->m_vecSg2d[i].xcoo = pSrcSG2D->m_vecSg2d[j].xcoo;
						pDstSG2D->m_vecSg2d[i].ycoo = pSrcSG2D->m_vecSg2d[j].ycoo;
					}
				}
			}

			for (int i = 0; i < pDstSG3D->m_vecSg3d.size(); i++)
			{
				for (int j = nCCNC - 1; j >= 0; j--)
				{
					if (i == j + nCCIX - 1)
					{
						pDstSG3D->m_vecSg3d[i].xcoo = pSrcSG3D->m_vecSg3d[j].xcoo;
						pDstSG3D->m_vecSg3d[i].ycoo = pSrcSG3D->m_vecSg3d[j].ycoo;
						pDstSG3D->m_vecSg3d[i].ve3d = pSrcSG3D->m_vecSg3d[j].ve3d;
					}
				}
			}
			break;
		}
	}
}

//************************************
// Method:    writeSenc
// Brief:	  写SENC
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57Cell::writeSenc()
{
	QString strCellName = getCellName();
	QString strSencPath = getSencFullPath(strCellName);
	//QByteArray baSencFullPath = strSencPath.toLatin1();
	//FILE *fp = fopen(baSencFullPath.data(), "wb");
	std::string stdStrSencFullPath = codec->fromUnicode(strSencPath).data();
	FILE *fp = fopen(stdStrSencFullPath.c_str(), "wb");
	if (fp == NULL)
		return;

	ioSencFile(fp, false);
	fclose(fp);
}

//************************************
// Method:    autoErUpdate
// Brief:	  自动ER更新
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: const QString & strErPath
//************************************
void CS57Cell::autoErUpdate(const QString& strErPath)
{
	importErFile(strErPath);
	updateCell();
	initFeatureInst();
	// 写入SENC
	writeSenc();
}

//************************************
// Method:    initFeatureInst
// Brief:	  初始化要素指令
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57Cell::initFeatureInst()
{
	for (int i = 0; i < m_vecS57RecFE.size(); i++)
	{
		m_vecS57RecFE[i]->initFieldInst();
	}
}
//************************************
// Method:    getCellExtent
// Brief:	  获取图幅范围
// Returns:   CRectangle<double>
// Author:    cl
// DateTime:  2022/07/21
//************************************
CRectangle<double> CS57Cell::getCellExtent()
{
	return m_CellRectangle;
}

//************************************
// Method:    getCellCenterPos
// Brief:	  获取图幅中心坐标点
// Returns:   QPointF
// Author:    cl
// DateTime:  2022/07/21
//************************************
QPointF CS57Cell::getCellCenterPos() const
{
	double lon = (m_CellRectangle.xMinimum() + m_CellRectangle.xMaximum()) / 2.0;
	double lat = (m_CellRectangle.yMinimum() + m_CellRectangle.yMaximum()) / 2.0;

	return QPointF(lon, lat);
}

//************************************
// Method:    extentCell
// Brief:	  图幅范围
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57Cell::extentCell()
{
	//分别查找VI/VC/VE中坐标范围
	double comf = getCellComf();
	long xMin = std::numeric_limits<long>::max();
	long yMin = std::numeric_limits<long>::max();
	long xMax = -std::numeric_limits<long>::max();
	long yMax = -std::numeric_limits<long>::max();
	for (int i = 0; i < m_vecS57RecVI.size(); i++)
	{
		if (m_vecS57RecVI[i]->m_Sg2d.m_vecSg2d.size() > 0)
		{
			long lat = m_vecS57RecVI[i]->m_Sg2d.m_vecSg2d[0].ycoo;
			long lon = m_vecS57RecVI[i]->m_Sg2d.m_vecSg2d[0].xcoo;
			xMin = std::min(xMin, lon);
			yMin = std::min(yMin, lat);
			xMax = std::max(xMax, lon);
			yMax = std::max(yMax, lat);
		}
		else
		{
			for (int j = 0; j < m_vecS57RecVI[i]->m_Sg3d.m_vecSg3d.size(); j++)
			{
				long lat = m_vecS57RecVI[i]->m_Sg3d.m_vecSg3d[j].ycoo;
				long lon = m_vecS57RecVI[i]->m_Sg3d.m_vecSg3d[j].xcoo;
				xMin = std::min(xMin, lon);
				yMin = std::min(yMin, lat);
				xMax = std::max(xMax, lon);
				yMax = std::max(yMax, lat);
			}
		}		
	}

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

	m_CellRectangle = CRectangle<double>(xMin/ comf, yMin / comf, xMax / comf, yMax / comf);
}