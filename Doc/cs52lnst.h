#pragma once
#include "doc_global.h"
#include "cs52layer.h"
#include <QString>
#include <QVector>

namespace Doc
{
	// ÏßÐÍÄ£¿é
	struct S52LindField
	{
		QString						LINM;	/* A(8) /* name of the linestyle; */
		short						LICL;	// I(5) /* pivot-point¡¯s column-number;LICL is counted from the top,left corner of the vector space to the right;-9999 (left) <= LICL <= 32767 (right) */
		short						LIRW;	// I(5) /* pivot-point¡¯s row-number;LIRW is counted from the top left corner of the vector space to the bottom; -9999 (top) <= LIRW <= 32767 (bottom) */
		unsigned short				LIHL;	// I(5) /* width of bounding box;where 1<= LIHL <=32767;Note: does not include vector line width */
		unsigned short				LIVL;	// I(5) /* height of bounding box;where 1<= LIVL <=32767;Note: does not include vector line width */
		unsigned short				LBXC;	// I(5) /* bounding box upper left column number;where 0<= LBXC <=32767; */
		unsigned short				LBXR;	// I(5) /* bounding box upper left row number;	where 0<= LBXR <=32767; */
	};
	struct S52LxpoField
	{
		QString						EXPT;			// A(1 / 15) /* free text for symbology/* describes meaning & use of symbology 	elements */
	};
	struct S52SubLcrfField
	{
		char						CIDX;			/* A(1) /* Letter (ASCII >= 64) used as colour index within PBTM.BITR field or within the PBTM.VECT field. */
		QString						CTOK;			/* A(5)colour token which is identified by the letter in CIDX. */
	};
	struct S52LcrfField
	{
		QVector<S52SubLcrfField*>		vecSubFieldLcrf;
	};
	struct S52LvctField
	{
		QString						VECD;			/* A(1 / 15) /* String of vector commands; */
	};
	struct S52InstSc
	{
		QPoint						piovtPoint;
		QString						symbName;
		unsigned short				orient;
	};

	class DOC_EXPORT LNST
	{
	public:
		S52LindField					m_fieldLind;
		S52LxpoField					m_fieldLxpo;
		S52LcrfField					m_fieldLcrf;
		QVector<S52LvctField*>		m_vecFieldLvct;
		QVector<S52Layer*>			m_vecS52Layers;
		QVector<S52InstSc*>			m_vecInstSC;
	};
}
