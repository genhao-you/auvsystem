#include "stdafx.h"
#include "cs57recfeature.h"
#include "cs57symbolrules.h"
#include "codec.h"
#include <QDebug>
#include <QTextCodec>

using namespace Doc;
CS57FieldFRID::CS57FieldFRID()
	: numFields(0)
{}

CS57FieldFRID::~CS57FieldFRID()
{}

void CS57FieldFRID::init(DDFRecord* ddfRec)
{
	rcnm = 100;
	rcid = ddfRec->GetIntSubfield("FRID", 0, "RCID", 0);
	prim = ddfRec->GetIntSubfield("FRID", 0, "PRIM", 0);
	grup = ddfRec->GetIntSubfield("FRID", 0, "GRUP", 0);
	objl = ddfRec->GetIntSubfield("FRID", 0, "OBJL", 0);
	rver = ddfRec->GetIntSubfield("FRID", 0, "RVER", 0);
	ruin = ddfRec->GetIntSubfield("FRID", 0, "RUIN", 0);

	numFields = 1;
}

void CS57FieldFRID::ioFile(FILE *fp, bool ioFlag)
{
	if (ioFlag)
	{
		fread(&numFields, sizeof(unsigned short), 1, fp);
		if (numFields == 0) return;
		fread(&rcnm, sizeof(unsigned char), 1, fp);
		fread(&rcid, sizeof(unsigned int), 1, fp);
		fread(&prim, sizeof(unsigned char), 1, fp);
		fread(&grup, sizeof(unsigned char), 1, fp);
		fread(&objl, sizeof(unsigned short), 1, fp);
		fread(&rver, sizeof(unsigned short), 1, fp);
		fread(&ruin, sizeof(unsigned char), 1, fp);
	}
	else
	{
		fwrite(&numFields, sizeof(unsigned short), 1, fp);
		if (numFields == 0) return;
		fwrite(&rcnm, sizeof(unsigned char), 1, fp);
		fwrite(&rcid, sizeof(unsigned int), 1, fp);
		fwrite(&prim, sizeof(unsigned char), 1, fp);
		fwrite(&grup, sizeof(unsigned char), 1, fp);
		fwrite(&objl, sizeof(unsigned short), 1, fp);
		fwrite(&rver, sizeof(unsigned short), 1, fp);
		fwrite(&ruin, sizeof(unsigned char), 1, fp);
	}
}

void CS57FieldFRID::copy(CS57RecFeature *pRecFE)
{
	numFields = pRecFE->m_Frid.numFields;
	if (numFields == 0) return;

	rcnm = pRecFE->m_Frid.rcnm;
	rcid = pRecFE->m_Frid.rcid;
	prim = pRecFE->m_Frid.prim;
	grup = pRecFE->m_Frid.grup;
	objl = pRecFE->m_Frid.objl;
	rver = pRecFE->m_Frid.rver;
	ruin = pRecFE->m_Frid.ruin;
}

CS57FieldFOID::CS57FieldFOID()
	: numFields(0)
{}

CS57FieldFOID::~CS57FieldFOID()
{}

void CS57FieldFOID::init(DDFRecord* ddfRec)
{
	agen = ddfRec->GetIntSubfield("FOID", 0, "AGEN", 0);
	fidn = ddfRec->GetIntSubfield("FOID", 0, "FIDN", 0);
	fids = ddfRec->GetIntSubfield("FOID", 0, "FIDS", 0);

	numFields = 1;
}

void CS57FieldFOID::ioFile(FILE *fp, bool ioFlag)
{
	if (ioFlag)
	{
		fread(&numFields, sizeof(unsigned short), 1, fp);
		if (numFields == 0) return;
		fread(&agen, sizeof(unsigned short), 1, fp);
		fread(&fidn, sizeof(unsigned int), 1, fp);
		fread(&fids, sizeof(unsigned short), 1, fp);
	}
	else
	{
		fwrite(&numFields, sizeof(unsigned short), 1, fp);
		if (numFields == 0) return;
		fwrite(&agen, sizeof(unsigned short), 1, fp);
		fwrite(&fidn, sizeof(unsigned int), 1, fp);
		fwrite(&fids, sizeof(unsigned short), 1, fp);
	}
}

void CS57FieldFOID::copy(CS57RecFeature* pRecFE)
{
	numFields = pRecFE->m_Foid.numFields;
	if (numFields == 0) return;

	agen = pRecFE->m_Foid.agen;
	fidn = pRecFE->m_Foid.fidn;
	fids = pRecFE->m_Foid.fids;
}

CS57FieldATTF::CS57FieldATTF()
{}

CS57FieldATTF::~CS57FieldATTF()
{}

void CS57FieldATTF::init(DDFRecord* ddfRec)
{
	int numFields = ddfRec->FindField("ATTF", 0)->GetRepeatCount();
	for (int i = 0;i < numFields;i++)
	{
		sAttf oAttf;
		oAttf.attl = ddfRec->GetIntSubfield("ATTF", 0, "ATTL", i);
		oAttf.atvl = ddfRec->GetStringSubfield("ATTF", 0, "ATVL", i);
		m_vecAttf.push_back(oAttf);
	}
}

void CS57FieldATTF::ioFile(FILE *fp, bool ioFlag)
{
	int len;
	if (ioFlag)
	{
		int numFields;
		fread(&numFields, sizeof(int), 1, fp);
		if (numFields == 0) return;
		for (int i = 0;i < numFields;i++)
		{
			sAttf oAttf;
			fread(&oAttf.attl, sizeof(unsigned short), 1, fp);

			fread(&len, sizeof(int), 1, fp);
			char* pszAtvl = new char[len + 1];
			fread(pszAtvl, len, 1, fp);
			pszAtvl[len] = '\0';
			oAttf.atvl = pszAtvl;
			delete[] pszAtvl;
			pszAtvl = NULL;

			m_vecAttf.push_back(oAttf);
		}
	}
	else
	{
		int numFields = m_vecAttf.size();
		fwrite(&numFields, sizeof(int), 1, fp);
		if (numFields == 0) return;
		for (int i = 0;i < numFields;i++)
		{		
			fwrite(&m_vecAttf[i].attl, sizeof(unsigned short), 1, fp);

			len = m_vecAttf[i].atvl.length();
			fwrite(&len, sizeof(int), 1, fp);
			std::string stdStrAtvl = codec->fromUnicode(m_vecAttf[i].atvl).data();
			fwrite(stdStrAtvl.c_str(), len, 1, fp);
		}
	}
}

void CS57FieldATTF::copy(CS57RecFeature *pRecFE)
{
	int size = pRecFE->m_Attf.m_vecAttf.size();
	if (size == 0) return;

	for (int i = 0; i < size; i++)
	{
		sAttf oAttf;
		oAttf.attl = pRecFE->m_Attf.m_vecAttf[i].attl;
		oAttf.atvl = pRecFE->m_Attf.m_vecAttf[i].atvl;

		m_vecAttf.push_back(oAttf);
	}
}

CS57FieldNATF::CS57FieldNATF()
{}

CS57FieldNATF::~CS57FieldNATF()
{
	for (int i = 0; i < m_vecNatf.size(); i++)
	{
		delete[] m_vecNatf[i].atvl;
		m_vecNatf[i].atvl = nullptr;
	}
	m_vecNatf.clear();
}

void CS57FieldNATF::init(DDFRecord* ddfRec)
{
	int len, datalen;
	int numFields = ddfRec->FindField("NATF", 0)->GetRepeatCount();
	for (int i = 0;i < numFields;i++)
	{
		sNatf oNatf;
		oNatf.attl = ddfRec->GetIntSubfield("NATF", 0, "ATTL", i);										
		DDFSubfieldDefn *tmpsubfield = ddfRec->FindField("NATF", 0)->GetFieldDefn()->FindSubfieldDefn("ATVL");	// 获取子字段
		const char * subdata = ddfRec->FindField("NATF", 0)->GetSubfieldData(tmpsubfield, &len, i);				// 获取子字段数据
		datalen = tmpsubfield->GetDataLength(subdata, len, NULL);
		oNatf.len = datalen;
		oNatf.atvl = new char[datalen + 1];
		memcpy(oNatf.atvl, ddfRec->GetStringSubfield("NATF", 0, "ATVL", i), datalen);							// 当用单字节字符数组保存UNICODE时，不能用strdup
		oNatf.atvl[datalen] = '\0';

		m_vecNatf.push_back(oNatf);
	}
}

void CS57FieldNATF::ioFile(FILE *fp, bool ioFlag)
{
	int len;
	if (ioFlag)
	{
		int numFields;
		fread(&numFields, sizeof(int), 1, fp);
		if (numFields == 0) return;
		for (int i = 0;i < numFields;i++)
		{
			sNatf oNatf;
			//fread(&oNatf.attl, sizeof(unsigned short), 1, fp);
			//fread(&len, sizeof(int), 1, fp);
			//char* pszAtvl = new char[len + 1];
			//fread(pszAtvl, len, 1, fp);
			//pszAtvl[len] = '\0';
			//oNatf.atvl = pszAtvl;
			//delete[] pszAtvl;
			//pszAtvl = NULL;

			fread(&oNatf.attl, sizeof(unsigned short), 1, fp);
			fread(&oNatf.len, sizeof(unsigned short), 1, fp);
			oNatf.atvl = new char[oNatf.len + 1];
			fread(oNatf.atvl, oNatf.len, 1, fp);
			oNatf.atvl[oNatf.len] = '\0';

			m_vecNatf.push_back(oNatf);
		}
	}
	else
	{
		int numFields = m_vecNatf.size();
		fwrite(&numFields, sizeof(int), 1, fp);
		if (numFields == 0) return;
		for (int i = 0;i < numFields;i++)
		{
			fwrite(&m_vecNatf[i].attl, sizeof(unsigned short), 1, fp);
			fwrite(&m_vecNatf[i].len, sizeof(unsigned short), 1, fp);
			fwrite(m_vecNatf[i].atvl, m_vecNatf[i].len, 1, fp);
		}
	}
}

void CS57FieldNATF::copy(CS57RecFeature *pRecFE)
{
	int size = pRecFE->m_Natf.m_vecNatf.size();
	if (size == 0) return;

	for (int i = 0; i < size; i++)
	{
		sNatf oNatf;
		oNatf.attl = pRecFE->m_Natf.m_vecNatf[i].attl;
		oNatf.atvl = _strdup(pRecFE->m_Natf.m_vecNatf[i].atvl);
		oNatf.len = pRecFE->m_Natf.m_vecNatf[i].len;

		m_vecNatf.push_back(oNatf);
	}
}

CS57FieldFFPC::CS57FieldFFPC()
{}

CS57FieldFFPC::~CS57FieldFFPC()
{}

void CS57FieldFFPC::init(DDFRecord* ddfRec)
{
	sFfpc oFfpc;
	oFfpc.ffui = ddfRec->GetIntSubfield("FFPC", 0, "FFUI", 0);
	oFfpc.ffix = ddfRec->GetIntSubfield("FFPC", 0, "FFIX", 0);
	oFfpc.nfpt = ddfRec->GetIntSubfield("FFPC", 0, "NFPT", 0);
	m_vecFfpc.push_back(oFfpc);
}

CS57FieldFFPT::CS57FieldFFPT()
{}

CS57FieldFFPT::~CS57FieldFFPT()
{}

void CS57FieldFFPT::init(DDFRecord* ddfRec)
{
	int numFields = ddfRec->FindField("FFPT", 0)->GetRepeatCount();
	for (int i = 0;i < numFields;i++)
	{
		sFfpt oFfpt;
		memmove(&oFfpt.agen, ddfRec->GetStringSubfield("FFPT", 0, "LNAM", i), 2);
		memmove(&oFfpt.fidn, ddfRec->GetStringSubfield("FFPT", 0, "LNAM", i) + 2, 4);
		memmove(&oFfpt.fids, ddfRec->GetStringSubfield("FFPT", 0, "LNAM", i) + 6, 2);
		oFfpt.rind = ddfRec->GetIntSubfield("FFPT", 0, "RIND", i);
		oFfpt.comt = ddfRec->GetStringSubfield("FFPT", 0, "COMT", i);
		m_vecFfpt.push_back(oFfpt);
	}
}

void CS57FieldFFPT::ioFile(FILE *fp, bool ioFlag)
{
	int len;
	if (ioFlag)
	{
		int numFields;
		fread(&numFields, sizeof(int), 1, fp);
		if (numFields == 0) return;
		for (int i = 0;i < numFields;i++)
		{
			sFfpt oFfpt;
			fread(&oFfpt.agen, sizeof(unsigned short), 1, fp);
			fread(&oFfpt.fidn, sizeof(unsigned int), 1, fp);
			fread(&oFfpt.fids, sizeof(unsigned short), 1, fp);
			fread(&oFfpt.rind, sizeof(unsigned char), 1, fp);

			fread(&len, sizeof(int), 1, fp);
			char* pszComt = new char[len + 1];
			fread(pszComt, len, 1, fp);
			pszComt[len] = '\0';
			oFfpt.comt = pszComt;
			delete[] pszComt;
			pszComt = NULL;

			fread(&oFfpt.idx, sizeof(unsigned short), 1, fp);

			m_vecFfpt.push_back(oFfpt);
		}
	}
	else
	{
		int numFields = m_vecFfpt.size();
		fwrite(&numFields, sizeof(int), 1, fp);
		if (numFields == 0) return;
		for (int i = 0;i < numFields;i++)
		{
			fwrite(&m_vecFfpt[i].agen, sizeof(unsigned short), 1, fp);
			fwrite(&m_vecFfpt[i].fidn, sizeof(unsigned int), 1, fp);
			fwrite(&m_vecFfpt[i].fids, sizeof(unsigned short), 1, fp);
			fwrite(&m_vecFfpt[i].rind, sizeof(unsigned char), 1, fp);

			len = m_vecFfpt[i].comt.length();
			fwrite(&len, sizeof(int), 1, fp);
			//QByteArray baComt = m_vecFfpt[i].comt.toLatin1();
			//char* pszComt = baComt.data();
			std::string stdStrComt = codec->fromUnicode(m_vecFfpt[i].comt).data();
			fwrite(stdStrComt.c_str(), len, 1, fp);
			fwrite(&m_vecFfpt[i].idx, sizeof(unsigned short), 1, fp);
		}
	}
}

void CS57FieldFFPT::copy(CS57RecFeature* pRecFE)
{
	int size = pRecFE->m_Ffpt.m_vecFfpt.size();
	if (size == 0) return;

	for (int i = 0; i < size; i++)
	{
		sFfpt oFfpt;
		oFfpt.agen = pRecFE->m_Ffpt.m_vecFfpt[i].agen;
		oFfpt.fidn = pRecFE->m_Ffpt.m_vecFfpt[i].fidn;
		oFfpt.fids = pRecFE->m_Ffpt.m_vecFfpt[i].fids;
		oFfpt.rind = pRecFE->m_Ffpt.m_vecFfpt[i].rind;
		oFfpt.idx = pRecFE->m_Ffpt.m_vecFfpt[i].idx;
		oFfpt.comt = pRecFE->m_Ffpt.m_vecFfpt[i].comt;

		m_vecFfpt.push_back(oFfpt);
	}
}

CS57FieldFSPC::CS57FieldFSPC()
{}

CS57FieldFSPC::~CS57FieldFSPC()
{}

void CS57FieldFSPC::init(DDFRecord* ddfRec)
{
	sFspc oFspc;
	oFspc.fsut = ddfRec->GetIntSubfield("FSPC", 0, "FSUT", 0);
	oFspc.fsix = ddfRec->GetIntSubfield("FSPC", 0, "FSIX", 0);
	oFspc.nspt = ddfRec->GetIntSubfield("FSPC", 0, "NSPT", 0);
	m_vecFspc.push_back(oFspc);
}

CS57FieldFSPT::CS57FieldFSPT()
{}

CS57FieldFSPT::~CS57FieldFSPT()
{}

void CS57FieldFSPT::init(DDFRecord* ddfRec)
{
	int numFields = ddfRec->FindField("FSPT", 0)->GetRepeatCount();
	for (int i = 0;i < numFields;i++)
	{
		sFspt oFspt;
		memcpy(&oFspt.rcnm, ddfRec->GetStringSubfield("FSPT", 0, "NAME", i), 1);
		memcpy(&oFspt.rcid, ddfRec->GetStringSubfield("FSPT", 0, "NAME", i) + 1, 4);
		oFspt.ornt = ddfRec->GetIntSubfield("FSPT", 0, "ORNT", i);
		oFspt.usag = ddfRec->GetIntSubfield("FSPT", 0, "USAG", i);
		oFspt.mask = ddfRec->GetIntSubfield("FSPT", 0, "MASK", i);
		m_vecFspt.push_back(oFspt);
	}
}

void CS57FieldFSPT::ioFile(FILE *fp, bool ioFlag)
{
	if (ioFlag)
	{
		int numFields;
		fread(&numFields, sizeof(int), 1, fp);
		if (numFields == 0) return;

		for (int i = 0;i < numFields;i++)
		{
			sFspt oFspt;
			fread(&oFspt.rcnm, sizeof(unsigned char), 1, fp);
			fread(&oFspt.rcid, sizeof(unsigned int), 1, fp);
			fread(&oFspt.ornt, sizeof(unsigned char), 1, fp);
			fread(&oFspt.usag, sizeof(unsigned char), 1, fp);
			fread(&oFspt.mask, sizeof(unsigned char), 1, fp);
			fread(&oFspt.idx, sizeof(unsigned short), 1, fp);

			m_vecFspt.push_back(oFspt);
		}
	}
	else
	{
		int numFields = m_vecFspt.size();
		fwrite(&numFields, sizeof(int), 1, fp);
		if (numFields == 0) return;

		for (int i = 0;i < numFields;i++)
		{
			fwrite(&m_vecFspt[i].rcnm, sizeof(unsigned char), 1, fp);
			fwrite(&m_vecFspt[i].rcid, sizeof(unsigned int), 1, fp);
			fwrite(&m_vecFspt[i].ornt, sizeof(unsigned char), 1, fp);
			fwrite(&m_vecFspt[i].usag, sizeof(unsigned char), 1, fp);
			fwrite(&m_vecFspt[i].mask, sizeof(unsigned char), 1, fp);
			fwrite(&m_vecFspt[i].idx, sizeof(unsigned short), 1, fp);
		}
	}
}

void CS57FieldFSPT::copy(CS57RecFeature* pRecFE)
{
	int size = pRecFE->m_Fspt.m_vecFspt.size();
	if (size == 0) return;

	for (int i = 0; i < size; i++)
	{
		sFspt oFspt;
		oFspt.rcnm = pRecFE->m_Fspt.m_vecFspt[i].rcnm;
		oFspt.rcid = pRecFE->m_Fspt.m_vecFspt[i].rcid;
		oFspt.ornt = pRecFE->m_Fspt.m_vecFspt[i].ornt;
		oFspt.mask = pRecFE->m_Fspt.m_vecFspt[i].mask;
		oFspt.usag = pRecFE->m_Fspt.m_vecFspt[i].usag;
		oFspt.idx  = pRecFE->m_Fspt.m_vecFspt[i].idx;

		m_vecFspt.push_back(oFspt);
	}
}

CS57FieldINST::CS57FieldINST()
{}
CS57FieldINST::~CS57FieldINST()
{
	for (int i = 0; i < fieldINST.vecAC.size(); i++)
	{
		if (fieldINST.vecAC[i] != nullptr)
			delete fieldINST.vecAC[i];
		fieldINST.vecAC[i] = nullptr;
	}
	fieldINST.vecAC.clear();

	for (int i = 0; i < fieldINST.vecAP.size(); i++)
	{
		if (fieldINST.vecAP[i] != nullptr)
			delete fieldINST.vecAP[i];
		fieldINST.vecAP[i] = nullptr;
	}
	fieldINST.vecAP.clear();

	for (int i = 0; i < fieldINST.vecCA.size(); i++)
	{
		if (fieldINST.vecCA[i] != nullptr)
			delete fieldINST.vecCA[i];
		fieldINST.vecCA[i] = nullptr;
	}
	fieldINST.vecCA.clear();

	for (int i = 0; i < fieldINST.vecCS.size(); i++)
	{
		if (fieldINST.vecCS[i] != nullptr)
			delete fieldINST.vecCS[i];
		fieldINST.vecCS[i] = nullptr;
	}
	fieldINST.vecCS.clear();

	for (int i = 0; i < fieldINST.vecLC.size(); i++)
	{
		if (fieldINST.vecLC[i] != nullptr)
			delete fieldINST.vecLC[i];
		fieldINST.vecLC[i] = nullptr;
	}
	fieldINST.vecLC.clear();

	for (int i = 0; i < fieldINST.vecLS.size(); i++)
	{
		if (fieldINST.vecLS[i] != nullptr)
			delete fieldINST.vecLS[i];
		fieldINST.vecLS[i] = nullptr;
	}
	fieldINST.vecLS.clear();

	for (int i = 0; i < fieldINST.vecSY.size(); i++)
	{
		if (fieldINST.vecSY[i] != nullptr)
			delete fieldINST.vecSY[i];
		fieldINST.vecSY[i] = nullptr;
	}
	fieldINST.vecSY.clear();

	for (int i = 0; i < fieldINST.vecTE.size(); i++)
	{
		if (fieldINST.vecTE[i] != nullptr)
			delete fieldINST.vecTE[i];
		fieldINST.vecTE[i] = nullptr;
	}
	fieldINST.vecTE.clear();

	for (int i = 0; i < fieldINST.vecTX.size(); i++)
	{
		if (fieldINST.vecTX[i] != nullptr)
			delete fieldINST.vecTX[i];
		fieldINST.vecTX[i] = nullptr;
	}
	fieldINST.vecTX.clear();
}

void CS57FieldINST::ioFile(FILE *fp, bool ioFlag)
{
	unsigned short size;
	unsigned short len;
	if (ioFlag)
	{
		fread(&dpri, sizeof(unsigned short), 1, fp);					// 显示优先级
		fread(&rpri, sizeof(char), 1, fp);								// 雷达优先

		fread(&len, sizeof(unsigned short), 1, fp);
		char* pszDscn = new char[len + 1];
		fread(pszDscn, len, 1, fp);
		pszDscn[len] = '\0';
		dscn = pszDscn;
		delete[] pszDscn;
		pszDscn = NULL;													// 显示分类名

		fread(&dscnCode, sizeof(unsigned short), 1, fp);				// 显示分类代码

		fread(&len, sizeof(unsigned short), 1, fp);
		char* pszTnam = new char[len + 1];
		fread(pszTnam, len, 1, fp);
		pszTnam[len] = '\0';
		tnam = pszTnam;
		delete[] pszTnam;
		pszTnam = NULL;													// 查找表名

		fread(&len, sizeof(unsigned short), 1, fp);
		char* pszGroup = new char[len + 1];
		fread(pszGroup, len, 1, fp);
		pszGroup[len] = '\0';
		groupDisp = pszGroup;
		delete[] pszGroup;
		pszGroup = NULL;												// 显示分组

		fread(&size, sizeof(unsigned short), 1, fp);					// 符号化指令
		fieldINST.vecAC.reserve(size);
		for (int i = 0; i < size; i++)
		{
			showFillAC* pAC = new showFillAC();
			fread(&len, sizeof(unsigned short), 1, fp);
			char* pszAreaColor = new char[len + 1];
			fread(pszAreaColor, len, 1, fp);
			pszAreaColor[len] = '\0';
			pAC->areacolor = pszAreaColor;
			delete[] pszAreaColor;
			pszAreaColor = NULL;				

			fread(&pAC->transparence, sizeof(unsigned short), 1, fp);
			fieldINST.vecAC.push_back(pAC);
		}

		fread(&size, sizeof(unsigned short), 1, fp);
		fieldINST.vecAP.reserve(size);
		for (int i = 0; i < size; i++)
		{
			showFillAP* pAP = new showFillAP();
			fread(&len, sizeof(unsigned short), 1, fp);
			char* pszPatName = new char[len + 1];
			fread(pszPatName, len, 1, fp);
			pszPatName[len] = '\0';
			pAP->patname = pszPatName;
			delete[] pszPatName;
			pszPatName = NULL;

			fread(&pAP->rotation, sizeof(float), 1, fp);
			fieldINST.vecAP.push_back(pAP);
		}

		fread(&size, sizeof(unsigned short), 1, fp);
		fieldINST.vecCS.reserve(size);
		for (int i = 0; i < size; i++)
		{
			showProcCS* pCS = new showProcCS();
			fread(&len, sizeof(unsigned short), 1, fp);
			char* pszProcName = new char[len + 1];
			fread(pszProcName, len, 1, fp);
			pszProcName[len] = '\0';
			pCS->procName = pszProcName;
			delete[] pszProcName;
			pszProcName = NULL;

			fieldINST.vecCS.push_back(pCS);
		}

		fread(&size, sizeof(unsigned short), 1, fp);
		fieldINST.vecLC.reserve(size);
		for (int i = 0; i < size; i++)
		{
			showLineLC* pLC = new showLineLC();
			fread(&len, sizeof(unsigned short), 1, fp);
			char* pszCline = new char[len + 1];
			fread(pszCline, len, 1, fp);
			pszCline[len] = '\0';
			pLC->Cline = pszCline;
			delete[] pszCline;
			pszCline = NULL;

			fieldINST.vecLC.push_back(pLC);
		}

		fread(&size, sizeof(unsigned short), 1, fp);
		fieldINST.vecLS.reserve(size);
		for (int i = 0; i < size; i++)
		{
			showLineLS* pLS = new showLineLS();		
			fread(&len, sizeof(unsigned short), 1, fp);
			char* pszLColor = new char[len + 1];
			fread(pszLColor, len, 1, fp);
			pszLColor[len] = '\0';
			pLS->lcolor = pszLColor;
			delete[] pszLColor;
			pszLColor = NULL;

			fread(&len, sizeof(unsigned short), 1, fp);
			char* pszLStyle = new char[len + 1];
			fread(pszLStyle, len, 1, fp);
			pszLStyle[len] = '\0';
			pLS->lstyle = pszLStyle;
			delete[] pszLStyle;
			pszLStyle = NULL;

			fread(&pLS->lwidth, sizeof(char), 1, fp);
			fieldINST.vecLS.push_back(pLS);
		}

		fread(&size, sizeof(unsigned short), 1, fp);
		fieldINST.vecSY.reserve(size);
		for (int i = 0; i < size; i++)
		{
			showPointSY* pSY = new showPointSY();
			fread(&len, sizeof(unsigned short), 1, fp);
			char* pszSyName = new char[len + 1];
			fread(pszSyName, len, 1, fp);
			pszSyName[len] = '\0';
			pSY->syname = pszSyName;
			delete[] pszSyName;
			pszSyName = NULL;

			fread(&len, sizeof(unsigned short), 1, fp);
			char* pszAttr = new char[len + 1];
			fread(pszAttr, len, 1, fp);
			pszAttr[len] = '\0';
			pSY->attr = pszAttr;
			delete[] pszAttr;
			pszAttr = NULL;
						
			fread(&pSY->rotation, sizeof(float), 1, fp);
			fread(&pSY->offset, sizeof(bool), 1, fp);
			fieldINST.vecSY.push_back(pSY);
		}

		fread(&size, sizeof(unsigned short), 1, fp);
		fieldINST.vecTE.reserve(size);
		for (int i = 0; i < size; i++)
		{
			showTextTE* pTE = new showTextTE();
			fread(&len, sizeof(unsigned short), 1, fp);
			char* pszAttrB = new char[len + 1];
			fread(pszAttrB, len, 1, fp);
			pszAttrB[len] = '\0';
			pTE->attrb = pszAttrB;
			delete[] pszAttrB;
			pszAttrB = NULL;

			fread(&len, sizeof(unsigned short), 1, fp);
			char* pszColor = new char[len + 1];
			fread(pszColor, len, 1, fp);
			pszColor[len] = '\0';
			pTE->color = pszColor;
			delete[] pszColor;
			pszColor = NULL;

			fread(&len, sizeof(unsigned short), 1, fp);
			char* pszFormat = new char[len + 1];
			fread(pszFormat, len, 1, fp);
			pszFormat[len] = '\0';
			pTE->formatstr = pszFormat;
			delete[] pszFormat;
			pszFormat = NULL;
			
			fread(&pTE->charsSize, sizeof(unsigned short), 1, fp);
			fread(&pTE->charsStyle, sizeof(char), 1, fp);
			fread(&pTE->charsWeight, sizeof(char), 1, fp);
			fread(&pTE->charsItalic, sizeof(char), 1, fp);
			fread(&pTE->hjust, sizeof(char), 1, fp);
			fread(&pTE->space, sizeof(char), 1, fp);
			fread(&pTE->vjust, sizeof(char), 1, fp);
			fread(&pTE->xoffs, sizeof(short), 1, fp);
			fread(&pTE->yoffs, sizeof(short), 1, fp);
			fread(&pTE->textGroup, sizeof(unsigned short), 1, fp);
			fieldINST.vecTE.push_back(pTE);
		}

		fread(&size, sizeof(unsigned short), 1, fp);
		fieldINST.vecTX.reserve(size);
		for (int i = 0; i < size; i++)
		{
			showTextTX* pTX = new showTextTX();
			fread(&len, sizeof(unsigned short), 1, fp);
			char* pszStr = new char[len + 1];
			fread(pszStr, len, 1, fp);
			pszStr[len] = '\0';
			pTX->str = pszStr;
			delete[] pszStr;
			pszStr = NULL;

			fread(&len, sizeof(unsigned short), 1, fp);
			char* pszColor = new char[len + 1];
			fread(pszColor, len, 1, fp);
			pszColor[len] = '\0';
			pTX->color = pszColor;
			delete[] pszColor;
			pszColor = NULL;

			fread(&pTX->charsSize, sizeof(unsigned short), 1, fp);
			fread(&pTX->charsStyle, sizeof(char), 1, fp);
			fread(&pTX->charsWeight, sizeof(char), 1, fp);
			fread(&pTX->charsItalic, sizeof(char), 1, fp);
			fread(&pTX->hjust, sizeof(char), 1, fp);
			fread(&pTX->space, sizeof(char), 1, fp);
			fread(&pTX->vjust, sizeof(char), 1, fp);
			fread(&pTX->xoffs, sizeof(short), 1, fp);
			fread(&pTX->yoffs, sizeof(short), 1, fp);
			fread(&pTX->textGroup, sizeof(unsigned short), 1, fp);
			fieldINST.vecTX.push_back(pTX);
		}

		fread(&size, sizeof(unsigned short), 1, fp);
		fieldINST.vecCA.reserve(size);
		for (int i = 0; i < size; i++)
		{
			showLineCA* pCA = new showLineCA;
			fread(pCA->arc_color, 5, 1, fp);
			pCA->arc_color[5] = '\0';

			fread(&pCA->arc_width, sizeof(unsigned short), 1, fp);
			fread(&pCA->arc_radius, sizeof(float), 1, fp);

			fread(pCA->outline_color, 5, 1, fp);
			pCA->outline_color[5] = '\0';

			fread(&pCA->outline_width, sizeof(unsigned short), 1, fp);
			fread(&pCA->sector_radius, sizeof(float), 1, fp);
			fread(&pCA->sectr1, sizeof(float), 1, fp);
			fread(&pCA->sectr2, sizeof(float), 1, fp);
			fieldINST.vecCA.push_back(pCA);
		}
	}
	else
	{
		fwrite(&dpri, sizeof(unsigned short), 1, fp);				// 显示优先级
		fwrite(&rpri, sizeof(char), 1, fp);							// 雷达优先

		len = dscn.length();
		fwrite(&len, sizeof(unsigned short), 1, fp);
		//QByteArray baDscn = dscn.toLatin1();
		//char* pszDscn = baDscn.data();
		std::string stdStrDscn = codec->fromUnicode(dscn).data();
		fwrite(stdStrDscn.c_str(), len, 1, fp);								// 显示分类名

		fwrite(&dscnCode, sizeof(unsigned short), 1, fp);           // 显示分类代码

		len = tnam.length();
		fwrite(&len, sizeof(unsigned short), 1, fp);
		//QByteArray baTnam = tnam.toLatin1();
		//char* pszTnam = baTnam.data();
		std::string stdStrTnam = codec->fromUnicode(tnam).data();
		fwrite(stdStrTnam.c_str(), len, 1, fp);

		len = groupDisp.length();
		fwrite(&len, sizeof(unsigned short), 1, fp);
		//QByteArray baGroup = groupDisp.toLatin1();
		//char* pszGroup = baGroup.data();
		std::string stdStrGroup = codec->fromUnicode(groupDisp).data();
		fwrite(stdStrGroup.c_str(), len, 1, fp);			// 显示分组

		size = fieldINST.vecAC.size();
		fwrite(&size, sizeof(unsigned short), 1, fp);
		for (int i = 0; i < size; i++)
		{
			len = fieldINST.vecAC[i]->areacolor.length();
			fwrite(&len, sizeof(unsigned short), 1, fp);
			//QByteArray baAreaColor = fieldINST.vecAC[i]->areacolor.toLatin1();
			//char* pszAreaColor = baAreaColor.data();
			std::string stdStrAreaColor = codec->fromUnicode(fieldINST.vecAC[i]->areacolor).data();
			fwrite(stdStrAreaColor.c_str(), len, 1, fp);

			fwrite(&fieldINST.vecAC[i]->transparence, sizeof(unsigned short), 1, fp);
		}

		size = fieldINST.vecAP.size();
		fwrite(&size, sizeof(unsigned short), 1, fp);
		for (int i = 0; i < size; i++)
		{
			len = fieldINST.vecAP[i]->patname.length();
			fwrite(&len, sizeof(unsigned short), 1, fp);
			//QByteArray baPatName = fieldINST.vecAP[i]->patname.toLatin1();
			//char* pszPatName = baPatName.data();
			std::string stdStrPatName = codec->fromUnicode(fieldINST.vecAP[i]->patname).data();
			fwrite(stdStrPatName.c_str(), len, 1, fp);

			fwrite(&fieldINST.vecAP[i]->rotation, sizeof(float), 1, fp);
		}

		size = fieldINST.vecCS.size();
		fwrite(&size, sizeof(unsigned short), 1, fp);
		for (int i = 0; i < size; i++)
		{
			len = fieldINST.vecCS[i]->procName.length();
			fwrite(&len, sizeof(unsigned short), 1, fp);
			//QByteArray baProcName = fieldINST.vecCS[i]->procName.toLatin1();
			//char* pszProcName = baProcName.data();
			std::string stdStrProcName = codec->fromUnicode(fieldINST.vecCS[i]->procName).data();
			fwrite(stdStrProcName.c_str(), len, 1, fp);
		}

		size = fieldINST.vecLC.size();
		fwrite(&size, sizeof(unsigned short), 1, fp);
		for (int i = 0; i < size; i++)
		{
			len = fieldINST.vecLC[i]->Cline.length();
			fwrite(&len, sizeof(unsigned short), 1, fp);
			//QByteArray baCline = fieldINST.vecLC[i]->Cline.toLatin1();
			//char* pszCline = baCline.data();
			std::string stdStrCline = codec->fromUnicode(fieldINST.vecLC[i]->Cline).data();
			fwrite(stdStrCline.c_str(), len, 1, fp);
		}

		size = fieldINST.vecLS.size();
		fwrite(&size, sizeof(unsigned short), 1, fp);
		for (int i = 0; i < size; i++)
		{
			len = fieldINST.vecLS[i]->lcolor.length();
			fwrite(&len, sizeof(unsigned short), 1, fp);
			//QByteArray baLColor = fieldINST.vecLS[i]->lcolor.toLatin1();
			//char* pszLColor = baLColor.data();
			std::string stdStrColor = codec->fromUnicode(fieldINST.vecLS[i]->lcolor).data();
			fwrite(stdStrColor.c_str(), len, 1, fp);

			len = fieldINST.vecLS[i]->lstyle.length();
			fwrite(&len, sizeof(unsigned short), 1, fp);
			//QByteArray baLStyle = fieldINST.vecLS[i]->lstyle.toLatin1();
			//char* pszLStyle = baLStyle.data();
			std::string stdLStyle = codec->fromUnicode(fieldINST.vecLS[i]->lstyle).data();
			fwrite(stdLStyle.c_str(), len, 1, fp);

			fwrite(&fieldINST.vecLS[i]->lwidth, sizeof(char), 1, fp);
		}

		size = fieldINST.vecSY.size();
		fwrite(&size, sizeof(unsigned short), 1, fp);
		for (int i = 0; i < size; i++)
		{
			len = fieldINST.vecSY[i]->syname.length();
			fwrite(&len, sizeof(unsigned short), 1, fp);
			//QByteArray baSyName = fieldINST.vecSY[i]->syname.toLatin1();
			//char* pszSyName = baSyName.data();
			std::string stdStrSyName = codec->fromUnicode(fieldINST.vecSY[i]->syname).data();
			fwrite(stdStrSyName.c_str(), len, 1, fp);

			len = fieldINST.vecSY[i]->attr.length();
			fwrite(&len, sizeof(unsigned short), 1, fp);
			//QByteArray baAttr = fieldINST.vecSY[i]->attr.toLatin1();
			//char* pszAttr = baAttr.data();
			std::string stdStrAttr = codec->fromUnicode(fieldINST.vecSY[i]->attr).data();
			fwrite(stdStrAttr.c_str(), len, 1, fp);

			fwrite(&fieldINST.vecSY[i]->rotation, sizeof(float), 1, fp);
			fwrite(&fieldINST.vecSY[i]->offset, sizeof(bool), 1, fp);
		}

		size = fieldINST.vecTE.size();
		fwrite(&size, sizeof(unsigned short), 1, fp);
		for (int i = 0; i < size; i++)
		{
			len = fieldINST.vecTE[i]->attrb.length();
			fwrite(&len, sizeof(unsigned short), 1, fp);
			//QByteArray baAttrB = fieldINST.vecTE[i]->attrb.toLatin1();
			//char* pszAttrB = baAttrB.data();
			std::string stdStrAttrB = codec->fromUnicode(fieldINST.vecTE[i]->attrb).data();
			fwrite(stdStrAttrB.c_str(), len, 1, fp);

			len = fieldINST.vecTE[i]->color.length();
			fwrite(&len, sizeof(unsigned short), 1, fp);
			//QByteArray baColor= fieldINST.vecTE[i]->color.toLatin1();
			//char* pszColor = baColor.data();
			std::string stdStrColor = codec->fromUnicode(fieldINST.vecTE[i]->color).data();
			fwrite(stdStrColor.c_str(), len, 1, fp);

			len = fieldINST.vecTE[i]->formatstr.length();
			fwrite(&len, sizeof(unsigned short), 1, fp);
			//QByteArray baFormat = fieldINST.vecTE[i]->formatstr.toLatin1();
			//char* pszFormat = baFormat.data();
			std::string stdStrFormat = codec->fromUnicode(fieldINST.vecTE[i]->formatstr).data();
			fwrite(stdStrFormat.c_str(), len, 1, fp);

			fwrite(&fieldINST.vecTE[i]->charsSize, sizeof(unsigned short), 1, fp);
			fwrite(&fieldINST.vecTE[i]->charsStyle, sizeof(char), 1, fp);
			fwrite(&fieldINST.vecTE[i]->charsWeight, sizeof(char), 1, fp);
			fwrite(&fieldINST.vecTE[i]->charsItalic, sizeof(char), 1, fp);
			fwrite(&fieldINST.vecTE[i]->hjust, sizeof(char), 1, fp);
			fwrite(&fieldINST.vecTE[i]->space, sizeof(char), 1, fp);
			fwrite(&fieldINST.vecTE[i]->vjust, sizeof(char), 1, fp);
			fwrite(&fieldINST.vecTE[i]->xoffs, sizeof(short), 1, fp);
			fwrite(&fieldINST.vecTE[i]->yoffs, sizeof(short), 1, fp);
			fwrite(&fieldINST.vecTE[i]->textGroup, sizeof(unsigned short), 1, fp);
		}

		size = fieldINST.vecTX.size();
		fwrite(&size, sizeof(unsigned short), 1, fp);
		for (int i = 0; i < size; i++)
		{
			len = fieldINST.vecTX[i]->str.length();
			fwrite(&len, sizeof(unsigned short), 1, fp);
			//QByteArray baStr = fieldINST.vecTX[i]->str.toLatin1();
			//char* pszStr = baStr.data();
			std::string stdStrStr = codec->fromUnicode(fieldINST.vecTX[i]->str).data();
			fwrite(stdStrStr.c_str(), len, 1, fp);

			len = fieldINST.vecTX[i]->color.length();
			fwrite(&len, sizeof(unsigned short), 1, fp);
			//QByteArray baColor = fieldINST.vecTX[i]->color.toLatin1();
			//char* pszColor = baColor.data();
			std::string stdStrColor = codec->fromUnicode(fieldINST.vecTX[i]->color).data();
			fwrite(stdStrColor.c_str(), len, 1, fp);

			fwrite(&fieldINST.vecTX[i]->charsSize, sizeof(unsigned short), 1, fp);
			fwrite(&fieldINST.vecTX[i]->charsStyle, sizeof(char), 1, fp);
			fwrite(&fieldINST.vecTX[i]->charsWeight, sizeof(char), 1, fp);
			fwrite(&fieldINST.vecTX[i]->charsItalic, sizeof(char), 1, fp);
			fwrite(&fieldINST.vecTX[i]->hjust, sizeof(char), 1, fp);
			fwrite(&fieldINST.vecTX[i]->space, sizeof(char), 1, fp);
			fwrite(&fieldINST.vecTX[i]->vjust, sizeof(char), 1, fp);
			fwrite(&fieldINST.vecTX[i]->xoffs, sizeof(short), 1, fp);
			fwrite(&fieldINST.vecTX[i]->yoffs, sizeof(short), 1, fp);
			fwrite(&fieldINST.vecTX[i]->textGroup, sizeof(unsigned short), 1, fp);
		}
		size = fieldINST.vecCA.size();
		fwrite(&size, sizeof(unsigned short), 1, fp);
		for (int i = 0; i < size; i++)
		{
			fwrite(fieldINST.vecCA[i]->arc_color, 5, 1, fp);

			fwrite(&fieldINST.vecCA[i]->arc_width, sizeof(unsigned short), 1, fp);
			fwrite(&fieldINST.vecCA[i]->arc_radius, sizeof(float), 1, fp);
			fwrite(fieldINST.vecCA[i]->outline_color, 5, 1, fp);

			fwrite(&fieldINST.vecCA[i]->outline_width, sizeof(unsigned short), 1, fp);
			fwrite(&fieldINST.vecCA[i]->sector_radius, sizeof(float), 1, fp);
			fwrite(&fieldINST.vecCA[i]->sectr1, sizeof(float), 1, fp);
			fwrite(&fieldINST.vecCA[i]->sectr2, sizeof(float), 1, fp);
		}
	}
}

void CS57FieldINST::copy(CS57FieldINST * pFieldInst)
{
	unsigned short size;
	dpri = pFieldInst->dpri;			// 显示优先级
	rpri = pFieldInst->rpri;			// 雷达优先
	dscn = pFieldInst->dscn;			// 显示分类名
	dscnCode = pFieldInst->dscnCode;	// 显示分类码
	tnam = pFieldInst->tnam;			// 查找表名
	groupDisp = pFieldInst->groupDisp;	// 显示分组
	size = pFieldInst->fieldINST.vecAC.size();
	for (int i = 0; i < size; i++)
	{
		showFillAC* pAC = new showFillAC();
		pAC->areacolor = pFieldInst->fieldINST.vecAC[i]->areacolor;
		pAC->transparence = pFieldInst->fieldINST.vecAC[i]->transparence;
		fieldINST.vecAC.push_back(pAC);
	}

	size = pFieldInst->fieldINST.vecAP.size();
	for (int i = 0; i < size; i++)
	{
		showFillAP* pAP = new showFillAP();
		pAP->patname = pFieldInst->fieldINST.vecAP[i]->patname;
		pAP->rotation = pFieldInst->fieldINST.vecAP[i]->rotation;
		fieldINST.vecAP.push_back(pAP);
	}

	size = pFieldInst->fieldINST.vecCS.size();
	for (int i = 0; i < size; i++)
	{
		showProcCS* pCS = new showProcCS();
		pCS->procName = pFieldInst->fieldINST.vecCS[i]->procName;
		fieldINST.vecCS.push_back(pCS);
	}

	size = pFieldInst->fieldINST.vecLC.size();
	for (int i = 0; i < size; i++)
	{
		showLineLC* pLC = new showLineLC();
		pLC->Cline = pFieldInst->fieldINST.vecLC[i]->Cline;
		fieldINST.vecLC.push_back(pLC);
	}

	size = pFieldInst->fieldINST.vecLS.size();
	for (int i = 0; i < size; i++)
	{
		showLineLS* pLS = new showLineLS();
		pLS->lcolor = pFieldInst->fieldINST.vecLS[i]->lcolor;
		pLS->lstyle = pFieldInst->fieldINST.vecLS[i]->lstyle;
		pLS->lwidth = pFieldInst->fieldINST.vecLS[i]->lwidth;
		fieldINST.vecLS.push_back(pLS);
	}

	size = pFieldInst->fieldINST.vecSY.size();
	for (int i = 0; i < size; i++)
	{
		showPointSY* pSY = new showPointSY();
		pSY->syname = pFieldInst->fieldINST.vecSY[i]->syname;
		pSY->attr = pFieldInst->fieldINST.vecSY[i]->attr;
		pSY->rotation = pFieldInst->fieldINST.vecSY[i]->rotation;
		pSY->offset = pFieldInst->fieldINST.vecSY[i]->offset;
		fieldINST.vecSY.push_back(pSY);
	}

	size = pFieldInst->fieldINST.vecTE.size();
	for (int i = 0; i < size; i++)
	{
		showTextTE* pTE = new showTextTE();
		pTE->attrb = pFieldInst->fieldINST.vecTE[i]->attrb;
		pTE->color = pFieldInst->fieldINST.vecTE[i]->color;
		pTE->formatstr = pFieldInst->fieldINST.vecTE[i]->formatstr;
		pTE->charsSize = pFieldInst->fieldINST.vecTE[i]->charsSize;
		pTE->charsStyle = pFieldInst->fieldINST.vecTE[i]->charsStyle;
		pTE->charsWeight = pFieldInst->fieldINST.vecTE[i]->charsWeight;
		pTE->charsItalic = pFieldInst->fieldINST.vecTE[i]->charsItalic;
		pTE->hjust = pFieldInst->fieldINST.vecTE[i]->hjust;
		pTE->space = pFieldInst->fieldINST.vecTE[i]->space;
		pTE->vjust = pFieldInst->fieldINST.vecTE[i]->vjust;
		pTE->xoffs = pFieldInst->fieldINST.vecTE[i]->xoffs;
		pTE->yoffs = pFieldInst->fieldINST.vecTE[i]->yoffs;
		pTE->textGroup = pFieldInst->fieldINST.vecTE[i]->textGroup;
		fieldINST.vecTE.push_back(pTE);
	}

	size = pFieldInst->fieldINST.vecTX.size();
	for (int i = 0; i < size; i++)
	{
		showTextTX* pTX = new showTextTX();
		pTX->str = pFieldInst->fieldINST.vecTX[i]->str;
		pTX->color = pFieldInst->fieldINST.vecTX[i]->color;
		pTX->charsSize = pFieldInst->fieldINST.vecTX[i]->charsSize;
		pTX->charsStyle = pFieldInst->fieldINST.vecTX[i]->charsStyle;
		pTX->charsWeight = pFieldInst->fieldINST.vecTX[i]->charsWeight;
		pTX->charsItalic = pFieldInst->fieldINST.vecTX[i]->charsItalic;
		pTX->hjust = pFieldInst->fieldINST.vecTX[i]->hjust;
		pTX->space = pFieldInst->fieldINST.vecTX[i]->space;
		pTX->vjust = pFieldInst->fieldINST.vecTX[i]->vjust;
		pTX->xoffs = pFieldInst->fieldINST.vecTX[i]->xoffs;
		pTX->yoffs = pFieldInst->fieldINST.vecTX[i]->yoffs;
		pTX->textGroup = pFieldInst->fieldINST.vecTX[i]->textGroup;
		fieldINST.vecTX.push_back(pTX);
	}
}

CS57RecFeature::CS57RecFeature()
	: m_nMaster(-1)
	, m_pCell(nullptr)
{}

CS57RecFeature::~CS57RecFeature()
{
	for(int i = 0; i < m_vecInst.size(); i++)
	{
		if (m_vecInst[i] != nullptr)
			delete m_vecInst[i];
		m_vecInst[i] = nullptr;
	}
	m_vecInst.clear();
}

bool CS57RecFeature::init(DDFRecord* ddfRec)
{
	Q_ASSERT(ddfRec);
	if (!ddfRec)
		return false;

	if (ddfRec->FindField("FRID", 0) != NULL) m_Frid.init(ddfRec);
	if (ddfRec->FindField("FOID", 0) != NULL) m_Foid.init(ddfRec);
	if (ddfRec->FindField("ATTF", 0) != NULL) m_Attf.init(ddfRec);
	if (ddfRec->FindField("NATF", 0) != NULL) m_Natf.init(ddfRec);
	if (ddfRec->FindField("FFPC", 0) != NULL) m_Ffpc.init(ddfRec);
	if (ddfRec->FindField("FFPT", 0) != NULL) m_Ffpt.init(ddfRec);
	if (ddfRec->FindField("FSPC", 0) != NULL) m_Fspc.init(ddfRec);
	if (ddfRec->FindField("FSPT", 0) != NULL) m_Fspt.init(ddfRec);

	return true;
}

void CS57RecFeature::ioFile(FILE *fp, bool ioFlag)
{
	m_Frid.ioFile(fp, ioFlag);
	m_Foid.ioFile(fp, ioFlag);
	m_Attf.ioFile(fp, ioFlag);
	m_Natf.ioFile(fp, ioFlag);
	m_Ffpt.ioFile(fp, ioFlag);
	m_Fspt.ioFile(fp, ioFlag);

	unsigned short size;
	if (ioFlag)
	{
		fread(&size, sizeof(unsigned short), 1, fp);
		for (int i = 0; i < size; i++)
		{
			CS57FieldINST* pFieldInst = new CS57FieldINST();
			pFieldInst->ioFile(fp, ioFlag);
			m_vecInst.push_back(pFieldInst);
		}
	}
	else
	{
		size = m_vecInst.size();
		fwrite(&size, sizeof(unsigned short), 1, fp);
		for (int i = 0; i < size; i++)
		{
			m_vecInst[i]->ioFile(fp, ioFlag);
		}
	}
}	

void CS57RecFeature::copy(CS57RecFeature* pRecFE)
{
	m_Frid.copy(pRecFE);
	m_Foid.copy(pRecFE);
	m_Attf.copy(pRecFE);
	m_Natf.copy(pRecFE);
	m_Ffpt.copy(pRecFE);
	m_Fspt.copy(pRecFE);

	unsigned short size = pRecFE->m_vecInst.size();
	for (int i = 0; i < size; i++)
	{
		CS57FieldINST* pFieldInst = new CS57FieldINST();
		pFieldInst->copy(pRecFE->m_vecInst[i]);
		m_vecInst.push_back(pFieldInst);
	}
	m_pCell = pRecFE->m_pCell;
}

void CS57RecFeature::initFieldInst()
{
	for (int i = 0; i < m_vecInst.size(); i++)
	{
		if (m_vecInst[i] != nullptr)
			delete m_vecInst[i];
		m_vecInst[i] = nullptr;
	}
	m_vecInst.clear();

	CS57PresLib* pPresLib = CS57DocManager::instance()->getPresLib();

	QString strFeaObjectAcronym = pPresLib->getFeatureInfo(m_Frid.objl)->strFeaObjectAcronym;
	QStringList lstFeaAttrCombin;
	for (int i = 0;i < m_Attf.m_vecAttf.size();i++)
	{
		QString strCombin =
			pPresLib->getAttributeInfo(m_Attf.m_vecAttf[i].attl)->strFeaAttrAcronym + 
			m_Attf.m_vecAttf[i].atvl;
		lstFeaAttrCombin.append(strCombin);
	}

	for (int i = 0;i < m_Natf.m_vecNatf.size();i++)
	{
		QString strCombin =
			pPresLib->getAttributeInfo(m_Natf.m_vecNatf[i].attl)->strFeaAttrAcronym + 
			m_Natf.m_vecNatf[i].atvl;
		lstFeaAttrCombin.append(strCombin);
	}

	LUPT* pLuptSimple = nullptr;
	LUPT* pLuptPaper = nullptr;
	LUPT* pLuptCdcP = nullptr;
	LUPT* pLuptCdcL = nullptr;
	LUPT* pLuptCdcA = nullptr;

	switch (m_Frid.prim)
	{
	case 1:
		pLuptSimple = pPresLib->serachLupt(strFeaObjectAcronym, lstFeaAttrCombin, Doc::SIMPLIFIED);
		pLuptPaper = pPresLib->serachLupt(strFeaObjectAcronym, lstFeaAttrCombin, Doc::PAPER_CHART);
		pLuptCdcP = pPresLib->serachLupt(strFeaObjectAcronym, lstFeaAttrCombin, Doc::CDC_P);
		break;
	case 2:
		pLuptSimple = pPresLib->serachLupt(strFeaObjectAcronym, lstFeaAttrCombin, Doc::LINES);
		pLuptCdcL = pPresLib->serachLupt(strFeaObjectAcronym, lstFeaAttrCombin, Doc::CDC_L);
		break;
	case 3:
		pLuptSimple = pPresLib->serachLupt(strFeaObjectAcronym, lstFeaAttrCombin, Doc::PLAIN_BOUNDARIES);
		pLuptPaper = pPresLib->serachLupt(strFeaObjectAcronym, lstFeaAttrCombin, Doc::SYMBOLIZED_BOUNDARIES);
		pLuptCdcA = pPresLib->serachLupt(strFeaObjectAcronym, lstFeaAttrCombin, Doc::CDC_A);
		break;
	}
	QVector<LUPT*> vecLupts;
	if(pLuptSimple != nullptr)
		vecLupts.push_back(pLuptSimple);
	if(pLuptPaper != nullptr) 
		vecLupts.push_back(pLuptPaper);
	if(pLuptCdcP != nullptr)
		vecLupts.push_back(pLuptCdcP);
	if (pLuptCdcL != nullptr)
		vecLupts.push_back(pLuptCdcL);
	if (pLuptCdcA != nullptr)
		vecLupts.push_back(pLuptCdcA);

	for (int i = 0; i < vecLupts.size(); i++)
	{
		CS57FieldINST* pFieldInst = new CS57FieldINST();
		genDisplayCommand(&vecLupts[i]->fieldINST, pFieldInst);
		pFieldInst->dpri = vecLupts[i]->fieldLUPT.DPRI;
		pFieldInst->rpri = vecLupts[i]->fieldLUPT.RPRI;
		pFieldInst->dscn = vecLupts[i]->fieldDISC.DSCN;
		if (pFieldInst->dscn == "DISPLAYBASE")
		{
			pFieldInst->dscnCode = 0;
		}
		else if (pFieldInst->dscn == "STANDARD")
		{
			pFieldInst->dscnCode = 1;
		}
		else if (pFieldInst->dscn == "OTHER")
		{
			pFieldInst->dscnCode = 2;
		}
		pFieldInst->tnam = vecLupts[i]->fieldLUPT.TNAM;
		pFieldInst->groupDisp = vecLupts[i]->fieldLUCM.LUED;
		m_vecInst.push_back(pFieldInst);
	}
}

void CS57RecFeature::genDisplayCommand(S52InstField* instField, CS57FieldINST* fieldInst)
{
	for (int i = 0; i < instField->vecTX.size(); i++)
	{
		showTextTX* pTX = new showTextTX();
		pTX->charsSize = instField->vecTX[i]->charsSize;
		pTX->charsStyle = instField->vecTX[i]->charsStyle;
		pTX->charsWeight = instField->vecTX[i]->charsWeight;
		pTX->charsItalic = instField->vecTX[i]->charsItalic;
		pTX->color = instField->vecTX[i]->color;
		pTX->hjust = instField->vecTX[i]->hjust;
		pTX->space = instField->vecTX[i]->space;
		pTX->str = instField->vecTX[i]->str;
		pTX->textGroup = instField->vecTX[i]->textGroup;
		pTX->vjust = instField->vecTX[i]->vjust;
		pTX->xoffs = instField->vecTX[i]->xoffs;
		pTX->yoffs = instField->vecTX[i]->yoffs;
		fieldInst->fieldINST.vecTX.push_back(pTX);
	}

	for (int i = 0; i < instField->vecTE.size(); i++)
	{
		showTextTE* pTE = new showTextTE();
		pTE->charsSize = instField->vecTE[i]->charsSize;
		pTE->charsStyle = instField->vecTE[i]->charsStyle;
		pTE->charsWeight = instField->vecTE[i]->charsWeight;
		pTE->charsItalic = instField->vecTE[i]->charsItalic;
		pTE->color = instField->vecTE[i]->color;
		pTE->hjust = instField->vecTE[i]->hjust;
		pTE->space = instField->vecTE[i]->space;
		pTE->formatstr = instField->vecTE[i]->formatstr;
		pTE->textGroup = instField->vecTE[i]->textGroup;
		pTE->vjust = instField->vecTE[i]->vjust;
		pTE->xoffs = instField->vecTE[i]->xoffs;
		pTE->yoffs = instField->vecTE[i]->yoffs;
		pTE->attrb = instField->vecTE[i]->attrb;
		fieldInst->fieldINST.vecTE.push_back(pTE);
	}

	for (int i = 0; i < instField->vecAC.size(); i++)
	{
		showFillAC* pAC = new showFillAC();
		pAC->areacolor = instField->vecAC[i]->areacolor;
		pAC->transparence = instField->vecAC[i]->transparence;
		fieldInst->fieldINST.vecAC.push_back(pAC);
	}

	for (int i = 0; i < instField->vecAP.size(); i++)
	{
		showFillAP* pAP = new showFillAP();
		pAP->patname =  instField->vecAP[i]->patname;
		pAP->rotation = instField->vecAP[i]->rotation;
		fieldInst->fieldINST.vecAP.push_back(pAP);
	}

	for (int i = 0; i < instField->vecCS.size(); i++)
	{
		showProcCS* pCS = new showProcCS();
		pCS->procName = instField->vecCS[i]->procName;

		CS57SymbolRules symbolRules(this);
		QString strProcName = instField->vecCS[i]->procName;
		std::string stdStrProcName = codec->fromUnicode(strProcName).data();
		char szProcName[9] = { "\0" };
		strcpy(szProcName, stdStrProcName.c_str());
		string ruleString = symbolRules.getShowCommand(szProcName);
		CS57PresLib* pPresLib = CS57DocManager::instance()->getPresLib();
		pPresLib->parseCS(fieldInst->fieldINST, ruleString);

		fieldInst->fieldINST.vecCS.push_back(pCS);
	}

	for (int i = 0; i < instField->vecLC.size(); i++)
	{
		showLineLC* pLC = new showLineLC();
		pLC->Cline = instField->vecLC[i]->Cline;
		fieldInst->fieldINST.vecLC.push_back(pLC);
	}

	for (int i = 0; i < instField->vecLS.size(); i++)
	{
		showLineLS * pLS = new showLineLS();
		pLS->lcolor = instField->vecLS[i]->lcolor;
		pLS->lstyle = instField->vecLS[i]->lstyle;
		pLS->lwidth = instField->vecLS[i]->lwidth;
		fieldInst->fieldINST.vecLS.push_back(pLS);
	}

	for (int i = 0; i < instField->vecSY.size(); i++)
	{
		showPointSY* pSY = new showPointSY();
		pSY->rotation = instField->vecSY[i]->rotation;
		pSY->offset = instField->vecSY[i]->offset;
		pSY->attr = instField->vecSY[i]->attr;
		if (pSY->attr == "ORIENT")
		{
			unsigned short attrCode = 117;
			pSY->rotation = getAttrValue(attrCode).toFloat();
		}
		pSY->syname = instField->vecSY[i]->syname;
		fieldInst->fieldINST.vecSY.push_back(pSY);
	}
}

QString CS57RecFeature::getAttrValue(unsigned short& attrCode) const
{
	QString str = "";
	for (int i = 0; i < m_Attf.m_vecAttf.size(); i++)
	{
		if (m_Attf.m_vecAttf[i].attl == attrCode)
		{
			if (m_Attf.m_vecAttf[i].atvl.length() > 0)
				str = m_Attf.m_vecAttf[i].atvl;
			return str;
		}
	}
	for (int i = 0; i < m_Natf.m_vecNatf.size(); i++)
	{
		if (m_Natf.m_vecNatf[i].attl == attrCode)
		{
			if (m_Natf.m_vecNatf[i].len > 0)
			{
				QTextCodec* codec_16 = QTextCodec::codecForName("UTF-16");
				QString value = codec_16->toUnicode(m_Natf.m_vecNatf[i].atvl);

				str = value;
			}
			return str;
		}
	}
	return str;
}

void CS57RecFeature::setParentCell(CS57Cell* cell)
{
	m_pCell = cell;
}

CS57Cell* CS57RecFeature::getParentCell() const
{
	return m_pCell;
}


