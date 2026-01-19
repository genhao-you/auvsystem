#pragma once
#include "doc_global.h"
#include "crectangle.h"
#include "cs57abstractcell.h"

namespace Doc
{
	class DOC_EXPORT CS57Cell : public CS57AbstractCell
	{
	public:
		CS57Cell();
		~CS57Cell();

	public:
		//导入ENC文件
		bool	importEncFile(const QString& fileName);
		//导入ER文件
		bool	importErFile(const QString& fileName);
		//获取图幅全路径
		QString getCellFullPath(const QString& name_with_ext) const;
		//获取SENC全路径
		QString getSencFullPath(const QString& name_with_ext) const;
		bool	getFloating(CS57RecFeature * pFeature);
		QVector<CS57RecFeature*> GetAssociatedFeatures(CS57RecFeature* pFeature);
		//更新图幅
		int		updateCell();
		//写Senc
		void	writeSenc();
		//自动ER更新
		void	autoErUpdate(const QString& strErPath);
		//初始化要素指令
		void	initFeatureInst();
		//获取图幅中心点
		QPointF				getCellCenterPos() const;
		//获取图幅范围
		CRectangle<double>	getCellExtent();//里面存的经纬度坐标

	public:
		//读写SENC
		void ioSencFile(FILE* fp, bool ioFlag);
		//关图
		void close();

	private:
		//添加要素
		void	addFeature(CS57RecFeature* pFeature);
		//删除要素
		void	delFeature(CS57RecFeature* pFeature);
		//添加空间
		void	addVector(CS57RecVector* pRecVector);
		//删除空间
		void	delVector(CS57RecVector* pRecVector);
		//应用记录更新
		void	applyRecordUpdate(CS57RecFeature* pFeature, CS57RecFeature *pFeatureUpdate);
		//应用记录更新
		void	applyRecordUpdate(CS57RecVector * pVector, CS57RecVector *pVectorUpdate);
		//释放更新
		void	releaseUpdate();
		//图幅索引构建
		void	indexCell();
		//计算图幅范围
		void	extentCell();
		//排序要素记录
		static bool sortFeatureRec(CS57RecFeature* p1, CS57RecFeature* p2);
		//排序空间记录
		static bool sortVectorRec(CS57RecVector* p1, CS57RecVector* p2);

	public:
		//S57矢量记录(孤立节点)指针集合
		QVector<CS57RecVector*> m_vecS57RecVI;

		QVector<CS57RecFeature*>	m_vecS57RecFEUpdate;			// S57数据要素更新记录
		QVector<CS57RecVector*>		m_vecS57RecVIUpdate;			// S57更新文件矢量记录(孤立节点)指针数组
		QVector<CS57RecVector*>		m_vecS57RecVCUpdate;			// S57更新文件矢量记录(链接节点)指针数组
		QVector<CS57RecVector*>		m_vecS57RecVEUpdate;			// S57更新文件矢量记录(边)指针数组

		CS57RecDsgi*	m_pRecDsgiUpdate;							// S57数据集更新一般信息记录
		CS57RecFeature* m_pRecFeatureUpdate;						// S57数据更新要素记录
		CS57RecVector*	m_pRecVectorUpdate;							// S57数据更新矢量记录

		QString m_strCellFullPath;
		QString m_strCellErFullPath;

	private:
		DDFModule m_DdfModule;
		CRectangle<double> m_CellRectangle; // 把它写入SENC中//内部存储经纬度

		QString m_strXtmVersion;
	};
}

