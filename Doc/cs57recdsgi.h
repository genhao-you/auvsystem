#pragma once
#include "doc_global.h"
#include "iso8211.h"
#include <QString>
#include <QVector>

namespace Doc
{
	class CS57RecDsgi;
	class DOC_EXPORT CS57FieldDSID
	{
	public:
		CS57FieldDSID();
		~CS57FieldDSID();
	public:
		//初始化DSID
		void init(DDFRecord* ddfRec);
		//读写DSID
		bool ioFile(FILE *fp, bool ioFlag);
		//拷贝DSID
		void copy(CS57RecDsgi* pDsgi);
	public:
		unsigned char	rcnm;			// 记录名
		unsigned int	rcid;			// 记录标识号
		unsigned char	expp;			// 交换用途
		unsigned char	intu;			// 预期用途
		QString			dsnm;			// 数据集名称
		QString			edtn;			// 版本号
		QString			updn;			// 更新号
		char			uadt[9];		// 更新应用日期
		char			isdt[9];		// 出版日期
		float			sted;			// S57的版本号
		unsigned char	prsp;			// 产品规范
		QString			psdn;			// 产品规范描述
		QString			pred;			// 产品规范版本号
		unsigned char	prof;			// 应用文件标识
		unsigned short	agen;			// 生产机构
		QString			comt;			// 注释

		unsigned short	numFields;
	};
	class DOC_EXPORT CS57FieldDSSI
	{
	public:
		CS57FieldDSSI();
		~CS57FieldDSSI();
	public:
		//初始化DSSI
		void init(DDFRecord* ddfRec);
		//读写DSSI
		void ioFile(FILE *fp, bool ioFlag);
		//拷贝DSSI
		void copy(CS57RecDsgi* pDsgi);
	public:
		unsigned char	dstr;			// 数据结构
		unsigned char	aall;			// ATTF词汇级
		unsigned char	nall;			// NATE词汇级
		unsigned int	nomr;			// 元记录数
		unsigned int	nocr;			// 制图记录数
		unsigned int	nogr;			// 地理记录数
		unsigned int	nolr;			// 集合记录数
		unsigned int	noin;			// 孤立节点数
		unsigned int	nocn;			// 连接节点数
		unsigned int	noed;			// 边数
		unsigned int	nofa;			// 面数

		unsigned short	numFields;
	};

	class DOC_EXPORT CS57RecDsgi
	{
	public:
		CS57RecDsgi();
		~CS57RecDsgi();

	public:
		//初始化DSGI
		bool init(DDFRecord* ddfRec);
		//读写DSGI
		void ioFile(FILE *fp, bool ioFlag);
		//拷贝DSGI
		void copy(CS57RecDsgi* pDsgi);
	public:
		CS57FieldDSID m_Dsid;
		CS57FieldDSSI m_Dssi;
	};
}
