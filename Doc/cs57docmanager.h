#pragma once
#include "xt_global.h"
#include <QObject>
#include "doc_global.h"
#include "cs57cell.h"
#include "cs57cellmessage.h"
#include "cs57preslib.h"
#include "cs57globalmapprovider.h"
#include "cs57mcovrprovider.h"
#include "cs57cellprovider.h"
#include "cs57mcovrregionprovider.h"
#include "spatialpolygon.h"

class QDir;
namespace Doc
{
	class CS57LoadCellThread;
	//工作空间文档类
	class DOC_EXPORT CS57WorkSpaceDoc
	{
	public:
		CS57WorkSpaceDoc();
		~CS57WorkSpaceDoc();

		struct sEnErFiles
		{
			QString strEnPath;
			QStringList lstErPaths;
		};

	public:
		void					setEncDir(const QString& dir);
		QString					getEncDir() const;
		QVector<sEnErFiles>	    getEncErFiles() const;

		void					releaseEncPaths();

	private:
		QStringList				searchFiles(const QDir& dir,const QStringList& filters);
		bool					verityNumber(const QString& str);
	private:
		//ENC图幅目录
		QString					m_strEncDir;
		QVector<sEnErFiles>		m_vecEnErFilePaths;
	};

	//表达库文档类
	class DOC_EXPORT CS57PresLibDoc
	{
	public:
		CS57PresLibDoc();
		~CS57PresLibDoc();

		CS57PresLib*			getPresLib() const;
	private:
		CS57PresLib*			m_pS57PresLib;
	};

	//文档管理类
	class DOC_EXPORT CS57DocManager : public QObject
	{
		Q_OBJECT
	public:
		~CS57DocManager();

		static CS57DocManager* instance()
		{
			if (m_pInstance == nullptr)
			{
				m_pInstance = new CS57DocManager();
			}
			return m_pInstance;
		}

		enum SelectChartType
		{
			SELECT_DISPLAY,		//选图用于显示
			SELECT_PRINT,		//选图用于打印
			SELECT_QUERY
		};
	public:
		//设置ENC目录
		void						setEncDir(const QString& dir);
		//获取ENC目录
		QString						getEncDir() const;
		//加载ENC
		void						loadEnc();
		//设置颜色模式
		void						setColorMode(XT::S52ColorMode mode);
		//选图
		void						selectChart(double scale, SelectChartType type);
		//选图
		void						selectChart(double scale, QStringList cellNames);
		//选图
		void						selectChart(double scale, CSpatialPolygon* pPolygon);
		//创建选图多边形
		void						createSelectPolygon(QPointF pt[4]);
		//设置符号模式
		void						setSymbolMode(XT::SymbolMode mode);
		//设置显示分组
		void						setDisplayCategory(XT::DisplayCategory displayCategory);
		//释放旧任务
		void						releaseOldJob();
		//区域拆分开关
		void						setRegionDivide(bool open);

		//获取全球底图数据源类指针
		CS57GlobalMapProvider*		getGlobalMapProviderPtr() const;
		//获取Mcovr数据源类指针
		CS57McovrProvider*			getMcovrProviderPtr() const;
		//获取图幅数据源类指针
		CS57CellProvider*			getCellProviderPtr() const;
		//获取Mcovr区域数据源类指针
		CS57McovrRegionProvider*	getMcovrRegionProviderPtr() const;
		//获取表达库类指针
		CS57PresLib*				getPresLib() const;
		//获取符号模式串
		QString						getSymbolMode() const;
	signals:
		void						updateLoadCellProgress(int cur, int total);
	private:
		//禁止外部通过new方式创建
		CS57DocManager();

		//释放全球底图
		void						releaseGlobalMap();
		//释放Mcovr
		void						releaseMcovr();
		//释放显示列表
		void						releaseDisplayContext();
		//释放ENC路径
		void						releaseEncPaths();
	private:
		void						initMarinerParam();
	private:
		static CS57DocManager*		m_pInstance;

		CS57WorkSpaceDoc*			m_pWorkSpaceDoc;
		CS57PresLibDoc*				m_pPresLibDoc;

	private:
		CS57LoadCellThread*			m_pLoadCellTread;
		CSpatialPolygon				m_pSelectPolygon;
		CS57GlobalMapProvider*		m_pGlobalMap;
		CS57McovrProvider*			m_pMcovrProvider;
		CS57CellProvider*			m_pCellProvider;
		CS57McovrRegionProvider*	m_pMcovrRegionProvider;
		QString						m_strSymbolMode;
	};
}

