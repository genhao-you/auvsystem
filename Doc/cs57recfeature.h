#pragma once
#include "doc_global.h"
#include "iso8211.h"
#include "cs57docmanager.h"
#include <QString>
#include <QVector>

namespace Doc
{
	class CS57RecFeature;
	class DOC_EXPORT CS57FieldFRID
	{
	public:
		CS57FieldFRID();
		~CS57FieldFRID();

	public:
		//初始化FRID
		void init(DDFRecord* ddfRec);
		//读写FRID
		void ioFile(FILE *fp, bool ioFlag);
		//拷贝FRID
		void copy(CS57RecFeature* pRecFE);
	public:
		unsigned char	rcnm;				// 记录名称
		unsigned int	rcid;				// 记录标识号
		unsigned char	prim;				// 物标几何图示（1/2/3/255：点、线、面、
											// 物标不直接参照任何空间对象）
		unsigned char	grup;				// 范围1到254,255无分组
		unsigned short	objl;				// 有效的物标标识/编码
		unsigned short	rver;				// 包含记录版本的系列数
		unsigned char	ruin;				// 记录改正指令（1/2/3：插入、删除、修改）

		unsigned short numFields;
	};

	class DOC_EXPORT CS57FieldFOID
	{
	public:
		CS57FieldFOID();
		~CS57FieldFOID();
	public:
		//初始化FOID
		void init(DDFRecord* ddfRec);
		//读写FOID
		void ioFile(FILE *fp, bool ioFlag);
		//拷贝FOID
		void copy(CS57RecFeature* pRecFE);
	public:
		unsigned short	agen;		// 机构代码
		unsigned int	fidn;		// 要素标识号F
		unsigned short	fids;		// 要素标识区分

		unsigned short numFields;
	};

	class DOC_EXPORT CS57FieldATTF
	{
	public:
		CS57FieldATTF();
		~CS57FieldATTF();
	public:
		//初始化ATTF
		void init(DDFRecord* ddfRec);
		//读写ATTF
		void ioFile(FILE *fp, bool ioFlag);
		//拷贝ATTF
		void copy(CS57RecFeature *pRecFE);
	public:
		struct sAttf			
		{
			unsigned short attl;		// 属性编码
			QString atvl;				// 属性值
		};
		QVector<sAttf> m_vecAttf;
	};

	class DOC_EXPORT CS57FieldNATF
	{
	public:
		CS57FieldNATF();
		~CS57FieldNATF();
	public:
		//初始化NATF
		void init(DDFRecord* ddfRec);
		//读写NATF
		void ioFile(FILE *fp, bool ioFlag);
		//拷贝NATF
		void copy(CS57RecFeature* pRecFE);
	public:
		struct sNatf
		{
			unsigned short attl;
			//QString	atvl;
			char*		atvl;
			unsigned short len = 0;
		};
		QVector<sNatf> m_vecNatf;
	};

	class DOC_EXPORT CS57FieldFFPC
	{
	public:
		CS57FieldFFPC();
		~CS57FieldFFPC();
	public:
		//初始化FFPC
		void init(DDFRecord* ddfRec);
	public:
		struct sFfpc
		{
			unsigned char	ffui;			// 特征物标更新指令
			unsigned short	ffix;			// 特征物标指针变址
			unsigned short	nfpt;			// 特征物标指针数
		};
		QVector<sFfpc> m_vecFfpc;
	};

	class DOC_EXPORT CS57FieldFFPT
	{
	public:
		CS57FieldFFPT();
		~CS57FieldFFPT();
	public:
		//初始化FFPT
		void init(DDFRecord* ddfRec);
		//读写FFPT
		void ioFile(FILE *fp, bool ioFlag);
		//拷贝FFPT
		void copy(CS57RecFeature* pRecFE);
	public:
		struct sFfpt
		{
			unsigned short	agen;		// 机构代码
			unsigned int	fidn;		// 要素标识号
			unsigned short	fids;		// 要素标识区分

			unsigned char	rind;
			QString			comt;
			unsigned short  idx;		// 特征记录数组指针
		};
		QVector<sFfpt> m_vecFfpt;
	};

	class DOC_EXPORT CS57FieldFSPC
	{
	public:
		CS57FieldFSPC();
		~CS57FieldFSPC();
	public:
		// 初始化
		void init(DDFRecord* ddfRec);
	public:
		struct sFspc
		{
			unsigned char	fsut;		// 特征向空间记录指针更新指令
			unsigned short	fsix;		// 特征的空间指针
			unsigned short	nspt;		// 特征的空间指针数
		};
		QVector<sFspc> m_vecFspc;
	};

	class DOC_EXPORT CS57FieldFSPT
	{
	public:
		CS57FieldFSPT();
		~CS57FieldFSPT();
	public:
		//初始化FSPT
		void init(DDFRecord* ddfRec);
		//读写FSPT
		void ioFile(FILE *fp, bool ioFlag);
		//拷贝FSPT
		void copy(CS57RecFeature* pRecFE);
	public:
		struct sFspt
		{
			unsigned char	rcnm;
			unsigned int	rcid;
			unsigned char	ornt;
			unsigned char	usag;
			unsigned char	mask;
			unsigned short	idx;		// 空间记录数组指针
		};
		QVector<sFspt> m_vecFspt;
	};

	class DOC_EXPORT CS57FieldINST
	{
	public:
		CS57FieldINST();
		~CS57FieldINST();
		//读写INST
		void ioFile(FILE *fp, bool ioFlag);
		//拷贝INST
		void copy(CS57FieldINST * pFieldInst);
	public:
		unsigned short	dpri;			// Display Priority 显示优先级
		char			rpri;			// Radar Priority -	’O’ 在雷达图像之上显示	’S’ 被雷达遮蔽显示 
		QString			tnam;			// Name of the addressed Look Up Table Set -’PLAIN_BOUNDARIES’ or ’SYMBOLIZED_BOUNDARIES’(areas)’SIMPLIFIED’ or ’PAPER_CHART’(points)		and ’LINES’(lines)
		QString			dscn;			// Defines membership within a display category 定义显示分类成员 /* Name of the display category */
		QString			groupDisp;
		unsigned short	dscnCode = 3;

		S52InstField fieldINST;
	};

	class DOC_EXPORT CS57RecFeature
	{
	public:
		CS57RecFeature();
		~CS57RecFeature();

	public:
		//初始化Feature
		bool		init(DDFRecord* ddfRec);
		//读写Feature
		void		ioFile(FILE *fp, bool ioFlag);
		//拷贝Feature
		void		copy(CS57RecFeature* pRecFE);
		QString		getAttrValue(unsigned short& attrCode) const;
		void		setParentCell(CS57Cell* cell);
		CS57Cell*	getParentCell() const;
		void		initFieldInst();
		void		genDisplayCommand(S52InstField* instField, CS57FieldINST* fieldInst);
	public:
		CS57FieldFRID	m_Frid;
		CS57FieldFOID	m_Foid;
		CS57FieldATTF	m_Attf;
		CS57FieldNATF	m_Natf;
		CS57FieldFFPC	m_Ffpc;
		CS57FieldFFPT	m_Ffpt;
		CS57FieldFSPC	m_Fspc;
		CS57FieldFSPT	m_Fspt;
		QVector<CS57FieldINST *> m_vecInst;
		int m_nMaster;						// 主物标数组指针
		CS57Cell*		m_pCell;
	};
}
