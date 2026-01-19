#include "stdafx.h"
#include "cs57docmanager.h"
#include "cs57recfeature.h"
#include "cs57symbolrules.h"
#include "spatialpolygon.h"
#include "cs57loadcellthread.h"
#include <QMessageBox>
#include <QCoreApplication>
#include <QDir>

using namespace Doc;
#pragma execution_character_set("utf-8")
CS57WorkSpaceDoc::CS57WorkSpaceDoc()
{}

CS57WorkSpaceDoc::~CS57WorkSpaceDoc()
{}

//************************************
// Method:    setEncDir
// Brief:	  设置ENC目录
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: const QString & dir
//************************************
void CS57WorkSpaceDoc::setEncDir(const QString& dir)
{
	m_strEncDir = dir;

	//遍历目录中的所有文件
	QStringList vecEnFilters;
	vecEnFilters.append("*.*");
	QStringList lstAllFiles = searchFiles(dir, vecEnFilters);
	if (lstAllFiles.size() == 0)
		return;

	//从所有文件中拆分出000和001等更新文件
	QStringList lstEnFiles;//存放000文件
	QStringList lstOtherFiles;//存放升级文件
	for (int i = 0; i < lstAllFiles.size(); i++)
	{
		QString strFile = lstAllFiles[i];
		QFileInfo fileInfo(strFile);
		QString strSuffix = fileInfo.suffix();
		if (strSuffix == "000")
			lstEnFiles.append(strFile);
		else
		{
			//判断后缀全部为数字才是升级文件
			if (verityNumber(strSuffix))
			{
				lstOtherFiles.append(strFile);
			}
		}
	}

	//过滤与000文件名称一样的文件，即为该000文件的更新文件
	for (int i = 0; i < lstEnFiles.size(); i++)
	{
		QString strEnFile = lstEnFiles[i];

		QFileInfo fileInfo(strEnFile);
		QString strSuffix ="." + fileInfo.suffix();
		QString strPathWithoutSuffix = strEnFile.replace(strSuffix, "");//类似"C:\\Hello\\World"

		sEnErFiles enerFiles;
		enerFiles.strEnPath = lstEnFiles[i];

		for (int j = 0; j < lstOtherFiles.size(); j++)
		{
			QString strOtherFile = lstOtherFiles[j];

			QFileInfo info(strOtherFile);
			QString suffix = "." + info.suffix();
			QString pathWithOutSuffix = strOtherFile.replace(suffix, "");
			if (strPathWithoutSuffix == pathWithOutSuffix)
			{
				enerFiles.lstErPaths.append(lstOtherFiles[j]);
			}
		}
		m_vecEnErFilePaths.push_back(enerFiles);
	}
}

//************************************
// Method:    getEncDir
// Brief:	  获取ENC目录
// Returns:   QString
// Author:    cl
// DateTime:  2022/07/21
//************************************
QString CS57WorkSpaceDoc::getEncDir() const
{
	return m_strEncDir;
}

//************************************
// Method:    searchFiles
// Brief:	  遍历目录下所有文件
// Returns:   QStringList
// Author:    cl
// DateTime:  2022/07/21
// Parameter: const QDir & dir
// Parameter: const QStringList & filters
//************************************
QStringList CS57WorkSpaceDoc::searchFiles(const QDir& dir, const QStringList& filters)
{
	QStringList lstFiles;
	QFileInfoList fileInfoList = dir.entryInfoList(filters, QDir::AllDirs | QDir::Files);
	foreach (QFileInfo fileInfo, fileInfoList)
	{
		if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
			continue;

		if (fileInfo.isDir())
		{
			searchFiles(fileInfo.filePath(), filters);
		}
		else
		{
			lstFiles.append(fileInfo.absoluteFilePath());
		}
	}

	return lstFiles;
}

//************************************
// Method:    getEncErFiles
// Brief:	  获取EN/ER文件
// Returns:   QVector<Doc::CS57WorkSpaceDoc::CS57WorkSpaceDoc::sEnErFiles>
// Author:    cl
// DateTime:  2022/07/21
//************************************
QVector<CS57WorkSpaceDoc::sEnErFiles> CS57WorkSpaceDoc::getEncErFiles() const
{
	return m_vecEnErFilePaths;
}

bool CS57WorkSpaceDoc::verityNumber(const QString& str)
{
	for (int i = 0; i < str.length(); i++)
	{
		if (str[i] < '0' || str[i] > '9')
			return false;
	}

	return true;
}

//************************************
// Method:    releaseEncPaths
// Brief:	  清理ENC路径
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57WorkSpaceDoc::releaseEncPaths()
{
	m_vecEnErFilePaths.clear();
}

CS57PresLibDoc::CS57PresLibDoc()
	: m_pS57PresLib(new CS57PresLib())
{}

CS57PresLibDoc::~CS57PresLibDoc()
{
	if (m_pS57PresLib != nullptr)
		delete m_pS57PresLib;
	m_pS57PresLib = nullptr;
}

CS57PresLib* CS57PresLibDoc::getPresLib() const
{
	return m_pS57PresLib;
}

CS57DocManager* CS57DocManager::m_pInstance = nullptr;
CS57DocManager::CS57DocManager()
	: m_pWorkSpaceDoc(new CS57WorkSpaceDoc())
	, m_pPresLibDoc(new CS57PresLibDoc())
	, m_pGlobalMap(new CS57GlobalMapProvider())
	, m_pMcovrProvider(new CS57McovrProvider())
	, m_pCellProvider(new CS57CellProvider(m_pMcovrProvider))
	, m_pMcovrRegionProvider(new CS57McovrRegionProvider())
	, m_pLoadCellTread(nullptr)
{
	m_pInstance = this;

	initMarinerParam();
	//默认S52符号
	setSymbolMode(XT::PAPER_CHART);
}

CS57DocManager::~CS57DocManager()
{
	if (m_pWorkSpaceDoc != nullptr)
		delete m_pWorkSpaceDoc;
	m_pWorkSpaceDoc = nullptr;

	if (m_pPresLibDoc != nullptr)
		delete m_pPresLibDoc;
	m_pPresLibDoc = nullptr;

	if (m_pGlobalMap != nullptr)
		delete m_pGlobalMap;
	m_pGlobalMap = nullptr;

	if (m_pMcovrProvider != nullptr)
		delete m_pMcovrProvider;
	m_pMcovrProvider = nullptr;

	if (m_pCellProvider != nullptr)
		delete m_pCellProvider;
	m_pCellProvider = nullptr;

	if (m_pMcovrRegionProvider != nullptr)
		delete m_pMcovrRegionProvider;
	m_pMcovrRegionProvider = nullptr;
}

//************************************
// Method:    releaseDisplayContext
// Brief:	  释放显示列表
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57DocManager::releaseDisplayContext()
{
	m_pCellProvider->releaseDisplayContext();
}

//************************************
// Method:    releaseMcovr
// Brief:	  释放Mcovr
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57DocManager::releaseMcovr()
{
	m_pMcovrProvider->releaseMcovr();
}

//************************************
// Method:    releaseGlobalMap
// Brief:	  释放全球底图
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57DocManager::releaseGlobalMap()
{
	m_pGlobalMap->releaseRenderPts();
}

//************************************
// Method:    setEncDir
// Brief:	  设置ENC目录
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: const QString & dir
//************************************
void CS57DocManager::setEncDir(const QString& dir)
{
	m_pWorkSpaceDoc->setEncDir(dir);
}

//************************************
// Method:    loadEnc
// Brief:	  //加载ENC
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57DocManager::loadEnc()
{
	//读取全球底图
	m_pGlobalMap->readBinFile();

	QVector<CS57WorkSpaceDoc::sEnErFiles> encFiles = m_pWorkSpaceDoc->getEncErFiles();
	if (encFiles.size() == 0)
		return;

	QString strEncFile = encFiles[0].strEnPath;
	QFileInfo fileInfo(strEncFile);
	QString strFileName = fileInfo.fileName();
	QString strXtmFullPath = strEncFile.replace(strFileName, "XTS57MSG.XTM");
	QFile file(strXtmFullPath);
	if (file.exists())
	{
		m_pMcovrProvider->readXTM(strXtmFullPath);
	}
	else
	{
		//非多线程做法
		//for (int i = 0; i < encFiles.size(); i++)
		//{
		//	CS57Cell oCell;
		//	oCell.importEncFile(encFiles[i].strEnPath);
		//	oCell.initFeatureInst();
		//	oCell.writeSenc();
		//	//判断是否包含升级文件，进行ER自动更新操作
		//	for (int j = 0; j < encFiles[i].lstErPaths.size(); j++)
		//	{
		//		//容器一开始就是按照001,002,003顺序，所有没有特意去排序
		//		oCell.autoErUpdate(encFiles[i].lstErPaths[j]);
		//	}
		//	m_pMcovrProvider->addMsg(new CS57CellMessage(&oCell));
		//}

		//多线程做法，返回加载进度信息
		if (m_pLoadCellTread != nullptr)
		{
			delete m_pLoadCellTread;
			m_pLoadCellTread = nullptr;
		}

		m_pLoadCellTread = new CS57LoadCellThread();
		m_pLoadCellTread->setEncFiles(encFiles);
		m_pLoadCellTread->setMcovrProvider(m_pMcovrProvider);
		connect(m_pLoadCellTread, &CS57LoadCellThread::loadProgress, this, &CS57DocManager::updateLoadCellProgress);

		//线程完成后自动销毁，并触发后续操作
		connect(m_pLoadCellTread, &CS57LoadCellThread::finished, [=]() {
			m_pMcovrProvider->writeXTM(strXtmFullPath);
			m_pLoadCellTread->deleteLater();
			m_pLoadCellTread = nullptr;
			});
		m_pLoadCellTread->start();

		/*while (!m_pLoadCellTread->isFinished())
		{
			QCoreApplication::processEvents();
		}
		m_pMcovrProvider->writeXTM(strXtmFullPath);*/
	}
}

//************************************
// Method:    selectChart
// Brief:	  选图
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: double scale
// Parameter: SelectChartType type
//************************************
void CS57DocManager::selectChart(double scale, SelectChartType type)
{
	m_pGlobalMap->selectChart(&m_pSelectPolygon, scale);
	m_pMcovrProvider->selectChart(&m_pSelectPolygon, scale);

	switch (type)
	{
	case Doc::CS57DocManager::SELECT_DISPLAY:
		m_pCellProvider->selectChart(&m_pSelectPolygon, scale);//显示选图模式
		break;
	case Doc::CS57DocManager::SELECT_PRINT:
		m_pCellProvider->selectChart2(&m_pSelectPolygon, scale);//打印选图模式
		break;
	case Doc::CS57DocManager::SELECT_QUERY:
		m_pCellProvider->selectChart3(&m_pSelectPolygon, scale);//查询选图模式
		break;
	}
}

//************************************
// Method:    selectChart
// Brief:	  选图
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: double scale
// Parameter: QStringList cellNames
//************************************
void CS57DocManager::selectChart(double scale, QStringList cellNames)
{
	m_pCellProvider->selectChart(&m_pSelectPolygon, scale, cellNames);
}

//************************************
// Method:    selectChart
// Brief:	  选图
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: double scale
// Parameter: CSpatialPolygon * pPolygon
//************************************
void CS57DocManager::selectChart(double scale, CSpatialPolygon* pPolygon)
{
	m_pCellProvider->selectChart2(&m_pSelectPolygon, scale);//打印选图模式
}

//************************************
// Method:    createSelectPolygon
// Brief:	  创建选图多边形
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: QPointF pt[4]
//************************************
void CS57DocManager::createSelectPolygon(QPointF pt[4])
{
	m_pSelectPolygon.relPolygon();
	QVector<QVector<QPointF>*> vpRings;
	QVector<QPointF> vpRing;
	for (int i = 0; i < 4; i++)
	{
		vpRing.push_back(pt[i]);
	}
	vpRing.push_back(pt[0]);
	vpRings.push_back(&vpRing);
	m_pSelectPolygon.setPolygon(vpRings);
}

//************************************
// Method:    setSymbolMode
// Brief:	  设置符号模式
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: XT::SymbolMode mode
//************************************
void CS57DocManager::setSymbolMode(XT::SymbolMode mode)
{
	switch (mode)
	{
	case XT::SIMPLIFIED_CHART:	//简单符号
		m_strSymbolMode = "SIMPLIFIED,LINES,PLAIN_BOUNDARIES";
		break;
	case XT::PAPER_CHART:		//纸图符号
		m_strSymbolMode = "PAPER_CHART,LINES,SYMBOLIZED_BOUNDARIES";
		break;
	case XT::CDC_CHART:			//CDC符号
		m_strSymbolMode = "CDC_P,CDC_L,CDC_A";
		break;
	}
}

//************************************
// Method:    getEncDir
// Brief:	  获取ENC目录
// Returns:   QString
// Author:    cl
// DateTime:  2022/07/21
//************************************
QString CS57DocManager::getEncDir() const
{
	return m_pWorkSpaceDoc->getEncDir();
}

//************************************
// Method:    getGlobalMapProviderPtr
// Brief:	  获取全球底图数据源类指针
// Returns:   Doc::CS57GlobalMapProvider*
// Author:    cl
// DateTime:  2022/07/21
//************************************
CS57GlobalMapProvider* CS57DocManager::getGlobalMapProviderPtr() const
{
	return m_pGlobalMap;
}

//************************************
// Method:    getMcovrProviderPtr
// Brief:	  获取Mcovr数据源类指针
// Returns:   Doc::CS57McovrProvider*
// Author:    cl
// DateTime:  2022/07/21
//************************************
CS57McovrProvider* CS57DocManager::getMcovrProviderPtr() const
{
	return m_pMcovrProvider;
}

//************************************
// Method:    getCellProviderPtr
// Brief:	  获取图幅数据源类指针
// Returns:   Doc::CS57CellProvider*
// Author:    cl
// DateTime:  2022/07/21
//************************************
CS57CellProvider* CS57DocManager::getCellProviderPtr() const
{
	return m_pCellProvider;
}

//************************************
// Method:    getPresLib
// Brief:	  获取表达库类指针
// Returns:   Doc::CS57PresLib*
// Author:    cl
// DateTime:  2022/07/21
//************************************
CS57PresLib* CS57DocManager::getPresLib() const
{
	return m_pPresLibDoc->getPresLib();
}

//************************************
// Method:    getSymbolMode
// Brief:	  获取符号模式
// Returns:   QString
// Author:    cl
// DateTime:  2022/07/21
//************************************
QString CS57DocManager::getSymbolMode() const
{
	return m_strSymbolMode;
}

//************************************
// Method:    releaseEncPaths
// Brief:	  释放ENC路径
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57DocManager::releaseEncPaths()
{
	m_pWorkSpaceDoc->releaseEncPaths();
}
//************************************
// Method:    setDisplayCategory
// Brief:	  设置显示分组
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: XT::DisplayCategory displayCategory
//************************************
void CS57DocManager::setDisplayCategory(XT::DisplayCategory displayCategory)
{
	m_pCellProvider->setDisplayCategory(displayCategory);
}
//************************************
// Method:    releaseOldJob
// Brief:	  释放旧的任务
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57DocManager::releaseOldJob()
{
	releaseEncPaths();
	releaseGlobalMap();
	releaseMcovr();
	releaseDisplayContext();
}

//************************************
// Method:    setRegionDivide
// Brief:	  设置区域拆分
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: bool open
//************************************
void CS57DocManager::setRegionDivide(bool open)
{
	if (open)
		m_pMcovrRegionProvider->startDivide(m_pMcovrProvider->m_vecSourceMcovr);
	else
		m_pMcovrRegionProvider->closeDivide();
}

//************************************
// Method:    getMcovrRegionProviderPtr
// Brief:	  获取Mcovr区域数据源类指针
// Returns:   Doc::CS57McovrRegionProvider*
// Author:    cl
// DateTime:  2022/07/21
//************************************
CS57McovrRegionProvider* CS57DocManager::getMcovrRegionProviderPtr() const
{
	return m_pMcovrRegionProvider;
}

//************************************
// Method:    setColorMode
// Brief:	  设置颜色模式
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: XT::S52ColorMode mode
//************************************
void CS57DocManager::setColorMode(XT::S52ColorMode mode)
{
	m_pPresLibDoc->getPresLib()->setColorMode(mode);
}

void CS57DocManager::initMarinerParam()
{
	CS57SymbolRules::S52_setMarinerParam(CS57SymbolRules::S52_MAR_SAFETY_CONTOUR, 10.);
	CS57SymbolRules::S52_setMarinerParam(CS57SymbolRules::S52_MAR_DEEP_CONTOUR, 20.);
	CS57SymbolRules::S52_setMarinerParam(CS57SymbolRules::S52_MAR_SHALLOW_CONTOUR, 5.);
	CS57SymbolRules::S52_setMarinerParam(CS57SymbolRules::S52_MAR_TWO_SHADES, 0.);
	CS57SymbolRules::S52_setMarinerParam(CS57SymbolRules::S52_MAR_SAFETY_DEPTH, 5.);
}
