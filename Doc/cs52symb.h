#pragma once

#include "doc_global.h"
#include <QString>
#include <QVector>

namespace Doc
{
	// 符号模块
	struct S52SymdField			/* Defines several symbol-parameters. */
	{
		QString			SYNM;	/* A(8) /* name of the symbol; */
		char			SYDF;	/* A(1) /* type of symbol definition:V Vector definition R Raster definition 符号定义类型：V矢量定义R光栅定义*/
		short			SYCL;	/* I(5) /* pivot-point’s column-number;SYCL is counted from the top,left corner of the vector/raster space to the right;-9999 (left) <= SYCL <= 32767 (right) */
								/* pivot-point的列号; SYCL从向量/栅格空间的左上角向右计数; -9999（左）<= SYCL <= 32767（右）*/
		short			SYRW;	/* I(5) /* pivot-point’s row-number;PROW is counted from the top, left	corner of the vector/raster space to the bottom ;-9999 (top) <= SYRW <= 32767 (bottom) */
								/* I（5）/ * pivot-point的行号; PROW从矢量/栅格空间的左上角到底部计算; -9999（上）<= SYRW <= 32767（下）*/
		unsigned short	SYHL;	/* I(5) /* width of bounding box;where 1<= PAHL <=128 for raster and where 1<= PAHL <=32767 for vector Note: does not include vector line width */
								/* 边界框的宽度;其中1 <= PAHL <= 128表示栅格，其中1 <= PAHL <= 32767表示矢量注意：不包括矢量线宽*/
		unsigned short	SYVL;	/* I(5) /* height of bounding box;where 1<= PAVL <=128 for raster and where 1<= PAGL <=32767 for vector	Note: does not include vector line width */
		unsigned short	SBXC;	/* I(5) /* bounding box upper left column number;where 1<= SBXC <=128 for raster and where 1<= SBXC <=32767 for vector */
		unsigned short	SBXR;	/* I(5) /* bounding box upper left row number;	where 1<= SBXR <=128 for raster and	where 1<= SBXR <=32767 for vector */
	};
	struct S52SxpoField			// The exposition field for the symbol module is identical to the exposition field for the pattern module
	{
		QString EXPT;			// A(1 / 15) /* free text for symbology/* describes meaning & use of symbology 	elements 符号系统的自由文本/ *描述符号系统元素的含义和用法*/
	};
	struct S52SubScrfField
	{
		char	CIDX;			/* A(1) /* Letter (ASCII >= 64) used as colour index within PBTM.BITR field or within the PBTM.VECT field.字母（ASCII> = 64）用作PBTM.BITR字段内或PBTM.VECT字段内的颜色索引。 */
		QString CTOK;			/* A(5)colour token which is identified by the letter in CIDX. */
	};
	struct S52ScrfField
	{
		QVector<S52SubScrfField*> vecSubFieldScrf;
	};
	struct S52SvctField
	{
		QString VECD;			/* A(1 / 15) /* String of vector commands; */
	};

	class DOC_EXPORT SYMB
	{
	public:
		S52SymdField				m_fieldSymd;
		S52SxpoField				m_fieldSxpo;
		S52ScrfField				m_fieldScrf;
		QVector<S52SvctField*>		m_vecFieldSvct;
		QVector<S52Layer*>			m_vecS52Layers;
	};
}

