#pragma once

#include "cs57abstractcell.h"
#include "crectangle.h"
#include "spatialpolygon.h"

namespace Doc
{
	class CS57Cell;
	class DOC_EXPORT CS57CellMessage : public CS57AbstractCell
	{
	public:
		CS57CellMessage();
		CS57CellMessage(CS57Cell* pCell);
		~CS57CellMessage();

	public:
		//转换位空间多边形
		CSpatialPolygon*	toSpatialPolygon();
		//获取MCOVR中心点
		QPointF				getMcovrCenterPos();	
		//读写XTM文件
		void ioXTMFile(FILE* fp, bool ioFlag);
		//释放
		void release();
	private:
		//设置DSGI
		void setDsgi(CS57RecDsgi* pDsgi);
		//设置DSGR
		void setDsgr(CS57RecDsgr* pDsgr);
		//添加FE记录
		void addRecFE(CS57RecFeature* pRecFE);
		//添加VE记录
		void addRecVE(CS57RecVector* pRecVE);
		//添加VC记录
		void addRecVC(CS57RecVector* pRecVC);
		//拓扑索引
		void index();
		//MCOVR范围
		void extentMcovr();
	private:
		CSpatialPolygon m_SpatialPolygon;
		CRectangle<double> m_McovrRectangle; // 把它写入SENC中//内部存储经纬度
	};
}
