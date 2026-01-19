#pragma once
#include "doc_global.h"
#include "cs57attrexpectedvalue.h"
#include "cs57feaattrtab.h"
#include "cs57feaobjtab.h"
#include "cs52colortable.h"
#include "cs52symboldata.h"
#include "cs52layer.h"
#include "cs52symb.h"
#include "cs52patt.h"
#include "cs52lnst.h"
#include <QString>

namespace Doc
{
	enum eLookupTabType
	{
		PLAIN_BOUNDARIES,						// 面要素简单边线
		SYMBOLIZED_BOUNDARIES,					// 面要素符号化边
		SIMPLIFIED,								// 点要素简化符号
		PAPER_CHART,							// 点要素纸图符号
		LINES,									// 线要素
		CDC_P,									// CDC点
		CDC_L,									// CDC线
		CDC_A									// CDC面
	};
	// 检索表条目模块标识字段结构
	struct S52LuptField
	{
		QString				OBCL;				/* A(6)  Name of the addressed object class */
		char				FTYP = 'A';			/* A(1) /* Addressed Object Type -’A’ Area ’L’ Line ’P’ Point */
		unsigned short		DPRI = 0;			/* I(5) /* Display Priority */
		char				RPRI = 'O';			/*A(1) /* Radar Priority -’O’ presentation on top radar ’S’ presentation suppressed by radar */
		QString				TNAM;				/* A(1 / 15)/* Name of the addressed Look Up Table Set - ’PLAIN_BOUNDARIES’ or ’SYMBOLIZED_BOUNDARIES’ (areas) ’SIMPLIFIED’ or ’PAPER_CHART’ (points) and ’LINES’ (lines) */
	};

	// 属性组合字段结构 
	struct S52AttcField
	{
		QStringList			lstCombin;		// 字符串列表，每个元素为属性名称+属性值
	};

	// 符号化指令字段
	struct showTextTX
	{
		QString				str;				// "	"参数
		char				hjust = '3';		// HJUST "水平调整"参数:‘1’表示中心对齐‘2’表示右对齐‘3’表示左对齐（缺省值）
		char				vjust = '1';		// VJUST "垂直调整"参数:‘1’表示向下对齐（缺省值）‘2’表示中心对齐‘3’表示向上对齐
		char				space = '2';		// SPACE 字符间距参数: 1’表示撑满（FIT）间距‘2’表示标准间距，缺省值	‘3’表示带有字框的标准间距
		char				charsStyle = '1';	// 字体	"1" 简单，sans serif字体
		char				charsWeight = '5';	// 字粗	4 细体	5 中粗体，这是缺省值。	6 粗体
		char				charsItalic = '1';	// 字宽	1表示正直朝上（非斜体），ENC $CHARS属性用"2"时必须改为 "1"。
		unsigned short		charsSize = 10;		// 字号以pica points为单位给定(1点=0.351mm)，pica点规定了大写字母的高度，最小字号是pica10，并且这也默认大小，允许使用较大的字号。
		short				xoffs = 0;			// 该参数定义了相对于空间物标位置以字号(单位见CHARS参数)给出的中枢点X-偏移 (如果XOFFS未给定或定义则缺省为0);正向X偏移向右延伸(字号单位意味着，如果字号为10个pica点，则每单位的偏移是10(0.351)=3.51mm)
		short				yoffs = 0;			// 该参数定义了相对于空间物标位置以字号(单位见CHARS参数)给出的中枢点的Y-偏移 (如果YOFFS未给定或定义则缺省为0);正向Y偏移向下延伸。
		QString				color;				// 颜色标识
		unsigned short		textGroup = 0;		// 文本组合代码
	};
	struct showTextTE
	{
		QString				formatstr;			// "文本串"参数
		QString				attrb;				// 属性字段
		char				hjust;				// HJUST "水平调整"参数:‘1’表示中心对齐‘2’表示右对齐‘3’表示左对齐（缺省值）
		char				vjust;				// VJUST "垂直调整"参数:‘1’表示向下对齐（缺省值）‘2’表示中心对齐‘3’表示向上对齐
		char				space;				// SPACE 字符间距参数: 1’表示撑满（FIT）间距‘2’表示标准间距，缺省值	‘3’表示带有字框的标准间距
		char				charsStyle;			// 字体	"1" 简单，sans serif字体
		char				charsWeight;		// 字粗	4 细体	5 中粗体，这是缺省值。	6 粗体
		char				charsItalic;		// 字宽	1表示正直朝上（非斜体），ENC $CHARS属性用"2"时必须改为 "1"。
		unsigned short		charsSize;			// 字号以pica points为单位给定(1点=0.351mm)，pica点规定了大写字母的高度，最小字号是pica10，并且这也默认大小，允许使用较大的字号。
		short				xoffs;				// 该参数定义了相对于空间物标位置以字号(单位见CHARS参数)给出的中枢点X-偏移 (如果XOFFS未给定或定义则缺省为0);正向X偏移向右延伸(字号单位意味着，如果字号为10个pica点，则每单位的偏移是10(0.351)=3.51mm)
		short				yoffs;				// 该参数定义了相对于空间物标位置以字号(单位见CHARS参数)给出的中枢点的Y-偏移 (如果YOFFS未给定或定义则缺省为0);正向Y偏移向下延伸。
		QString				color;				// 颜色标识
		unsigned short		textGroup;			// 文本组合代码
	};
	struct showPointSY
	{
		QString				syname;				// 符号名是8字母编码，其中6字母类别码和两位序列号
		QString				attr;
		float				rotation = 0;		// “符号旋转”参数	0 - 360方位度（顺时针方向，从正北算起）		缺省值：0度		注释：符号应绕其中枢点旋转；S - 57属性的6字母编码可作为特殊参数进行传递。
		bool				offset = false;
	};
	struct showLineLS
	{
		QString				lstyle;				// “预定义线型”参数 	"0" SOLD’(_________)"1"‘DASH’(---- - ) dash: 3.6 mm; space: 1.8 mm "2"‘DOTT’(.........) dott : 0.6 mm; space: 1.2 mm
		char				lwidth;				// 线宽“参数:	‘1’ x 0.32 mm <= WIDTH <= ‘8’ x 0.32 mm;
		QString				lcolor;				// 线颜色
	};
	struct showLineLC
	{
		QString				Cline;				// “复合线型”命令字
	};
	struct showFillAC
	{
		QString				areacolor;
		unsigned short		transparence = 0;	// 0 opaque (= default value) 1 25 %  2 50% 3 75%
	};
	struct showFillAP
	{
		QString				patname;
		float				rotation = 0.0;
	};
	struct showProcCS
	{
		QString				procName;
	};
	struct showLineCA
	{
		char				outline_color[6];	// 边线颜色
		unsigned short		outline_width;		// 边线宽度	
		char				arc_color[6];		// 弧线颜色
		unsigned short		arc_width;			// 弧线宽度
		float				sectr1;				// 起始度数
		float				sectr2;				// 结束度数
		float				arc_radius;			// 弧线半径
		float				sector_radius;		// 扇形半径
	};
	struct S52InstField							/* Describes the instruction entry to the look-up table; */
	{
		QVector<showTextTX*>	vecTX;
		QVector<showTextTE*>	vecTE;
		QVector<showPointSY*>	vecSY;
		QVector<showLineLS*>	vecLS;
		QVector<showLineLC*>	vecLC;
		QVector<showFillAC*>	vecAC;
		QVector<showFillAP*>	vecAP;
		QVector<showProcCS*>	vecCS;
		QVector<showLineCA*>	vecCA;
	};

	struct S52DiscField							/* Defines membership within a display category */
	{
		QString DSCN;							/* DSCN A(1 / 15) /* Name of the display category */
	};

	struct S52LucmField							/**L U C M Look - Up Comment*//* Describes look-up table entry */
	{
		QString LUED;							/* A(1 / 15) /* free text to describe look-up entry */
	};

	struct LUPT									// 检索表条目模块
	{
		S52LuptField	fieldLUPT;
		S52AttcField	fieldATTC;
		S52InstField	fieldINST;
		S52DiscField	fieldDISC;
		S52LucmField	fieldLUCM;
	};
	class CS52LutTable;
	class CCDCLutTable;
	class DOC_EXPORT CS57PresLib
	{
	public:
		CS57PresLib();
		~CS57PresLib();

		SYMB*					getSymb(const QString& symbName) const;
		PATT*					getPatt(const QString& pattName)const;
		LNST*					getLnst(const QString& lnstName)const;
		void					getAllLnstName(QStringList& lstLnst);
		void					getAllSymbName(QStringList& lstSymb);
		void					getAllPattName(QStringList& lstPatt);
		void					setColorMode(XT::S52ColorMode mode);
		void					getS52Color(const QString& colorName, unsigned short& r, unsigned short& g, unsigned short& b) const;
		S52Color*				getS52Color(const QString& colorName) const;
		QVector<LUPT*>			getLuptLst(int prim) const;
		bool					parseCS(S52InstField& instField, std::string ruleString);

		LUPT*					serachLupt(const QString& objlName, const QStringList& lstAttrCombin, eLookupTabType tabType) const;

		//如果找到返回结构体指针，否则返回nullptr
		S57Object*				getFeatureInfo(const QString& featureAcronym) const;

		//如果找到返回结构体指针，否则返回nullptr
		S57Object*				getFeatureInfo(unsigned short featureCode) const;

		//通过物标缩写返回物标码
		//如果找到返回物标码，否则返回-1
		unsigned short			getFeatureCode(const QString& featureAcronym) const;

		//通过物标属性缩写获取物标属性编码
		//成功返回物标属性编码，失败返回-1
		unsigned short			getAttrCode(const QString& feaAttrAcronym) const;

		//通过物标属性缩写获取物标属性结构体指针
		//成功返回有效指针，失败返回nullptr
		S57AttributeValue*		getAttributeInfo(const QString& feaAttrAcronym) const;

		//通过物标属性编码获取物标属性结构体指针
		//成功返回有效指针，失败返回nullptr
		S57AttributeValue*		getAttributeInfo(unsigned short attrCode) const;

		//通过物标属性码和期望值获取期望值结构体指针
		//成功返回有效指针，失败返回nullptr
		S57ExpectedValue*		getS57AttrExpValueInfo(unsigned short& code, unsigned short value) const;

		//通过物标属性码和期望值返回物标中英文名称
		//成功返回true，失败返回false
		bool					getAttrExpInfo(unsigned short attrCode, unsigned short expValue, QString& cName, QString& eName)const;

	private:
		CS52ColorTable*			m_pS52ColorTable;
		CS52LutTable*			m_pS52LutTable;
		CCDCLutTable*			m_pCDCLutTable;
		CS52SymbolData*			m_pS52SymbData;

		CS57AttrExpectedValue*	m_pS57AttrExpectedValue;
		CS57FeaAttrTab*			m_pS57FeaAttrTab;
		CS57FeaObjTab*			m_pS57FeaObjTab;
	};
}
