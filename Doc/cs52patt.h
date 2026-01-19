#pragma once

#include "doc_global.h"
#include <QVector>
#include <QString>

namespace Doc
{
	// 填充模板模块
	struct S52PatdField			// “填充模板定义”
	{
		QString						PANM;	/* A(8) /* name of the fill pattern; */
		char						PADF;	/* A(1) /* type of pattern definition:V Vector definition R Raster definition */
		QString						PATP;	/* A(3) /* type of the fill pattern:STG staggered pattern LIN linear pattern */
		QString						PASP;	/* A(3) /* pattern-symbol spacing:CON constant space SCL scale dependent spacing */
		unsigned short				PAMI;	/* I(5) /* minimum distance (units of 0.01 mm)between pattern symbols covers (bounding box + pivot point);where 0 <= PAMI <= 32767 */
		unsigned short				PAMA;	/* I(5) /* maximum distance (units of 0.01 mm)between pattern symbols covers (bounding box + pivot point);where 0 <= PAMA <= 32767;		PAMA is meaningless if PASP = ’CON’ */
		short						PACL;	/*I(5) /* pivot-point’s column number;	PACL is counted from the top, left corner of the vector/raster space to the right;-9999 (left) <= PACL <= 32767 (right) */
		short						PARW;	/* I(5) /* pivot-point’s row number;PARW is counted from the top, left corner of the vector/raster space to the bottom;-9999 (top) <= PARW <= 32767 (bottom) */
		unsigned short				PAHL;	/* I(5) /* width of bounding box;where 1<= PAHL <=122 for raster and where 1<= PAHL <=32767 for vector Note: does not include vector line width */
		unsigned short				PAVL;	/* I(5) /* height of bounding box;where 1<= PAVL <=122 for raster and where 1<= PAGL <=32767 for vector Note: does not include vector line width */
		unsigned short				PBXC;	/* I(5) /* bounding box upper left column number;where 0<= PBXC <=122 for raster and where 0<= PBXC <=32767 for vector */
		unsigned short				PBXR;	/* I(5) /* bounding box upper left row number;where 0<= PBXR <=122 for raster and where 0<= PBXR <=32767 for vector */
	};
	struct S52PxpoField						// P X P O Pattern Exposition Field
	{
		QString						EXPT;	// A(1 / 15) /* free text for symbology/* describes meaning & use of symbology 	elements */
	};
	struct S52SubPcrfField						// Pattern Colour Reference
	{
		char						CIDX;	/* A(1) /* Letter (ASCII >= 64) used as colour index within PBTM.BITR field or within the PBTM.VECT field. */
		QString						CTOK;	/* A(5)colour token which is identified by the letter in CIDX. */
	};
	struct S52PcrfField
	{
		QVector<S52SubPcrfField*>	vecSubfieldPcrf;
	};
	struct S52PvctField
	{
		QString VECD = NULL;		/* A(1 / 15) /* String of vector commands; */
	};

	class DOC_EXPORT PATT			//P A T T Pattern Identifier
	{
	public:
		S52PatdField				m_fieldPatd;
		S52PxpoField				m_fieldPxpo;
		S52PcrfField				m_fieldPcrf;
		QVector<S52PvctField*>		m_vecFieldPvct;
		QVector<S52Layer*>			m_vecS52Layers;
	};
}