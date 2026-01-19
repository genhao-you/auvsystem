#pragma once
#include "doc_global.h"
#include "iso8211.h"
#include <QString>

namespace Doc
{
	class CS57RecDsgr;
	class DOC_EXPORT CS57FieldDSPM
	{
	public:
		CS57FieldDSPM();
		~CS57FieldDSPM();
	public:
		//初始化DSPM
		void init(DDFRecord* ddfRec);
		//读写DSPM
		void ioFile(FILE *fp, bool ioFlag);
		//拷贝DSPM
		void copy(CS57RecDsgr* pDsgr);
	public:
		unsigned char	rcnm;	// 记录名
		unsigned int	rcid;		// 记录标识号
		unsigned char	hdat;		// 水平测量基准面
		unsigned char	vdat;		// 垂直基准面
		unsigned char	sdat;		// 测深基准面
		unsigned int	cscl;		// 数据编辑比例尺
		unsigned char	duni;		// 深度计量单位
		unsigned char	huni;		// 高度计量单位
		unsigned char	puni;		// 位置精度单位  S57 3.1版增补
		unsigned char	coun;		// 坐标单位
		unsigned int	comf;		// 坐标放大系数
		unsigned int	somf;		// 测深放大系数
		QString			comt;		// 注释

		unsigned short numFields;
	};

	class DOC_EXPORT CS57FieldDSPR
	{
	public:
		CS57FieldDSPR();
		~CS57FieldDSPR();

	public:
		//初始化DSPR
		void init(DDFRecord* ddfRec);
		//读写DSPR
		void ioFile(FILE *fp, bool ioFlag);
		//拷贝DSPR
		void copy(CS57RecDsgr* pDsgr);
	public:
		unsigned char	proj;
		long			prp1;
		long			prp2;
		long			prp3;
		long			prp4;
		long			feas;
		long			fnor;
		unsigned int	fpmf;
		QString			comt;

		unsigned short	numFields;
	};

	class DOC_EXPORT CS57RecDsgr
	{
	public:
		CS57RecDsgr();
		~CS57RecDsgr();

	public:
		//初始化DSGR
		bool init(DDFRecord* ddfRec);
		//读写DSGR
		void ioFile(FILE *fp, bool ioFlag);
		//拷贝DSGR
		void copy(CS57RecDsgr* pDsgr);
	public:
		CS57FieldDSPM m_Dspm;
		CS57FieldDSPR m_Dspr;
	};
}
