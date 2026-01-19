#pragma once
#include "doc_global.h"
#include "iso8211.h"
#include <QVector>

namespace Doc
{
	class CS57RecVector;
	class DOC_EXPORT CS57FieldVRID
	{
	public:
		CS57FieldVRID();
		~CS57FieldVRID();
	public:
		//初始化VRID
		void init(DDFRecord* ddfRec);
		//读写VRID
		void ioFile(FILE *fp, bool ioFlag);
		//拷贝VRID
		void copy(CS57RecVector* pRecVec);
	public:
		unsigned char	rcnm;
		unsigned int	rcid;
		unsigned short	rver;
		unsigned char	ruin;

		unsigned char numFields;	// 字段落重复个数
	};

	class DOC_EXPORT CS57FieldATTV
	{
	public:
		CS57FieldATTV();
		~CS57FieldATTV();

	public:
		//初始化ATTV
		void init(DDFRecord* ddfRec);
		//读写ATTV
		void ioFile(FILE *fp, bool ioFlag);
		//拷贝ATTV
		void copy(CS57RecVector* pRecVec);
	public:
		struct sAttv				// 向量记录属性字段结构
		{
			unsigned short attl;	// 属性标记/码
			QString atvl;			// 属性值
		};
		
		QVector<sAttv> m_vecAttv;
	};

	class DOC_EXPORT CS57FieldVRPC
	{
	public:
		CS57FieldVRPC();
		~CS57FieldVRPC();
	public:
		//初始化VRPC
		void init(DDFRecord* ddfRec);
		//拷贝VRPC
		void copy(CS57RecVector* pRecVec);
	public:
		unsigned char	vpui;
		unsigned short	vpix;
		unsigned short	nvpt;

		unsigned char	numFields;
	};

	class DOC_EXPORT CS57FieldVRPT
	{
	public:
		CS57FieldVRPT();
		~CS57FieldVRPT();
	public:
		//初始化VRPT
		void init(DDFRecord* ddfRec);
		//读取VRPT
		void ioFile(FILE *fp, bool ioFlag);
		//拷贝VRPT
		void copy(CS57RecVector* pRecVec);
	public:
		struct sVrpt
		{
			unsigned char rcnm;		// 外部指针分解为
			unsigned int rcid;		// rcnm & rcid
			unsigned char ornt;		// 方位（1：正向；2：反向；255：NULL）
			unsigned char usag;		// 用途指示（1：外部；2：内部；3：由数据界限分割的外部边界；255：NULL）
			unsigned char topi;		// 拓扑指示符（1，2：起/终节点；3，4：左/右面；5：包含面；255：NULL）			
			unsigned char mask;		// 表示（1：屏蔽；2：显示；255：NULL）

			unsigned short idx;		// 空间记录数组指针
		};
		QVector<sVrpt> m_vecVrpt;
	};

	class DOC_EXPORT CS57FieldSGCC
	{
	public:
		CS57FieldSGCC();
		~CS57FieldSGCC();

	public:
		//初始化SGCC
		void init(DDFRecord* ddfRec);
		//拷贝SGCC
		void copy(CS57RecVector* pRecVec);
	public:
		unsigned char	ccui;
		unsigned short	ccix;
		unsigned short	ccnc;

		unsigned char	numFields;
	};

	class DOC_EXPORT CS57FieldSG2D
	{
	public:
		CS57FieldSG2D();
		~CS57FieldSG2D();

	public:
		//初始化SG2D
		void	init(DDFRecord* ddfRec);
		//读取SG2D
		void	ioFile(FILE *fp, bool ioFlag);
		//拷贝SG2D
		void	copy(CS57RecVector* pRecVec);
	public:
		struct sSg2d
		{
			long ycoo;
			long xcoo;
		};

		QVector<sSg2d> m_vecSg2d;
	};

	class DOC_EXPORT CS57FieldSG3D
	{
	public:
		CS57FieldSG3D();
		~CS57FieldSG3D();

	public:
		//初始化SG3D
		void	init(DDFRecord* ddfRec);
		//读写SG3D
		void	ioFile(FILE *fp, bool ioFlag);
		//拷贝SG3D
		void	copy(CS57RecVector* pRecVec);
	public:
		struct sSg3d
		{
			long ycoo;
			long xcoo;
			long ve3d;
		};
		QVector<sSg3d> m_vecSg3d;
	};

	class DOC_EXPORT CS57RecVector
	{
	public:
		CS57RecVector();
		~CS57RecVector();

	public:
		//初始化Vector
		bool init(DDFRecord* ddfRec);
		//读写Vector
		void ioFile(FILE *fp, bool ioFlag);
		//拷贝Vector
		void copy(CS57RecVector* pRecVec);
	public:
		CS57FieldVRID m_Vrid;
		CS57FieldATTV m_Attv;
		CS57FieldVRPT m_Vrpt;
		CS57FieldVRPC m_Vrpc;
		CS57FieldSGCC m_Sgcc;
		CS57FieldSG2D m_Sg2d;
		CS57FieldSG3D m_Sg3d;

		QVector<int> vpVE;
		QVector<int> vpFE;
	};
}
