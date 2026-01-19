#include "stdafx.h"
#include "cs57recdsgi.h"
#include "codec.h"

using namespace Doc;

CS57FieldDSID::CS57FieldDSID()
{}

CS57FieldDSID::~CS57FieldDSID()
{}

//************************************
// Method:    init
// Brief:	  ³õÊ¼»¯DSID
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: DDFRecord * ddfRec
//************************************
void CS57FieldDSID::init(DDFRecord* ddfRec)
{
	rcnm = ddfRec->GetIntSubfield("DSID", 0, "RCNM", 0);
	rcid = ddfRec->GetIntSubfield("DSID", 0, "RCID", 0);
	expp = ddfRec->GetIntSubfield("DSID", 0, "EXPP", 0);
	intu = ddfRec->GetIntSubfield("DSID", 0, "INTU", 0);
	dsnm = ddfRec->GetStringSubfield("DSID", 0, "DSNM", 0);
	edtn = ddfRec->GetStringSubfield("DSID", 0, "EDTN", 0);
	updn = ddfRec->GetStringSubfield("DSID", 0, "UPDN", 0);
	strcpy(uadt, ddfRec->GetStringSubfield("DSID", 0, "UADT", 0));
	uadt[8] = '\0';
	strcpy(isdt, ddfRec->GetStringSubfield("DSID", 0, "ISDT", 0));
	isdt[8] = '\0';
	sted = (float)ddfRec->GetFloatSubfield("DSID", 0, "STED", 0);
	prsp = ddfRec->GetIntSubfield("DSID", 0, "PRSP", 0);
	psdn = ddfRec->GetStringSubfield("DSID", 0, "PSDN", 0);
	pred = ddfRec->GetStringSubfield("DSID", 0, "PRED", 0);
	prof = ddfRec->GetIntSubfield("DSID", 0, "PROF", 0);
	agen = ddfRec->GetIntSubfield("DSID", 0, "AGEN", 0);
	comt = ddfRec->GetStringSubfield("DSID", 0, "COMT", 0);
	numFields = 1;
}

//************************************
// Method:    ioFile
// Brief:	  ¶ÁÐ´DSID
// Returns:   bool
// Author:    cl
// DateTime:  2022/07/21
// Parameter: FILE * fp
// Parameter: bool ioFlag
//************************************
bool CS57FieldDSID::ioFile(FILE *fp, bool ioFlag)
{
	int len;
	if (ioFlag)
	{
		fread(&numFields, sizeof(unsigned short), 1, fp);
		if (numFields == 0) return true;

		fread(&rcnm, sizeof(unsigned char), 1, fp);
		fread(&rcid, sizeof(unsigned int), 1, fp);
		fread(&expp, sizeof(unsigned char), 1, fp);
		fread(&intu, sizeof(unsigned char), 1, fp);

		fread(&len, sizeof(int), 1, fp);
		char* pszDsnm = new char[len + 1];
		fread(pszDsnm, len, 1, fp);
		pszDsnm[len] = '\0';
		dsnm = pszDsnm;
		delete[] pszDsnm;
		pszDsnm = NULL;

		if (dsnm.isEmpty())
			return false;

		fread(&len, sizeof(int), 1, fp);
		char* pszEdtn = new char[len + 1];
		fread(pszEdtn, len, 1, fp);
		pszEdtn[len] = '\0';
		edtn = pszEdtn;
		delete[] pszEdtn;
		pszEdtn = NULL;

		fread(&len, sizeof(int), 1, fp);
		char* pszUpdn = new char[len + 1];
		fread(pszUpdn, len, 1, fp);
		pszUpdn[len] = '\0';
		updn = pszUpdn;
		delete[] pszUpdn;
		pszUpdn = NULL;

		fread(uadt, 8, 1, fp);uadt[8] = '\0';
		fread(isdt, 8, 1, fp);isdt[8] = '\0';
		fread(&sted, sizeof(float), 1, fp);
		fread(&prsp, sizeof(unsigned char), 1, fp);

		fread(&len, sizeof(int), 1, fp);
		char* pszPsdn = new char[len + 1];
		fread(pszPsdn, len, 1, fp);
		pszPsdn[len] = '\0';
		psdn = pszPsdn;
		delete[] pszPsdn;
		pszPsdn = NULL;

		fread(&len, sizeof(int), 1, fp);
		char* pszPred = new char[len + 1];
		fread(pszPred, len, 1, fp);
		pszPred[len] = '\0';
		pred = pszPred;
		delete[] pszPred;
		pszPred = NULL;

		fread(&prof, sizeof(unsigned char), 1, fp);
		fread(&agen, sizeof(unsigned short), 1, fp);

		fread(&len, sizeof(int), 1, fp);
		char* pszComt = new char[len + 1];
		fread(pszComt, len, 1, fp);
		pszComt[len] = '\0';
		comt = pszComt;
		delete[] pszComt;
		pszComt = NULL;
	}
	else
	{
		fwrite(&numFields, sizeof(unsigned short), 1, fp);
		if (numFields == 0) return true;

		fwrite(&rcnm, sizeof(unsigned char), 1, fp);
		fwrite(&rcid, sizeof(unsigned int), 1, fp);
		fwrite(&expp, sizeof(unsigned char), 1, fp);
		fwrite(&intu, sizeof(unsigned char), 1, fp);

		len = dsnm.length();
		fwrite(&len, sizeof(int), 1, fp);
		//QByteArray baDsnm = dsnm.toLatin1();
		//char* pszDsnm = baDsnm.data();
		std::string stdStrDsnm = codec->fromUnicode(dsnm).data();
		fwrite(stdStrDsnm.c_str(), len, 1, fp);

		len = edtn.length();
		fwrite(&len, sizeof(int), 1, fp);
		//QByteArray baEdtn = edtn.toLatin1();
		//char* pszEdtn = baEdtn.data();
		std::string stdStrEdtn = codec->fromUnicode(edtn).data();
		fwrite(stdStrEdtn.c_str(), len, 1, fp);

		len = updn.length();
		fwrite(&len, sizeof(int), 1, fp);
		//QByteArray baUpdn = updn.toLatin1();
		//char* pszUpdn = baUpdn.data();
		std::string stdStrUpdn = codec->fromUnicode(updn).data();
		fwrite(stdStrUpdn.c_str(), len, 1, fp);

		fwrite(uadt, sizeof(char), 8, fp);
		fwrite(isdt, sizeof(char), 8, fp);
		fwrite(&sted, sizeof(float), 1, fp);
		fwrite(&prsp, sizeof(unsigned char), 1, fp);

		len = psdn.length();
		fwrite(&len, sizeof(int), 1, fp);
		//QByteArray baPsdn = psdn.toLatin1();
		//char* pszPsdn = baPsdn.data();
		std::string stdStrPsdn = codec->fromUnicode(psdn).data();
		fwrite(stdStrPsdn.c_str(), len, 1, fp);

		len = pred.length();
		fwrite(&len, sizeof(int), 1, fp);
		//QByteArray baPred = pred.toLatin1();
		//char* pszPred = baPred.data();
		std::string stdStrPred = codec->fromUnicode(pred).data();
		fwrite(stdStrPred.c_str(), len, 1, fp);

		fwrite(&prof, sizeof(unsigned char), 1, fp);
		fwrite(&agen, sizeof(unsigned short), 1, fp);

		len = comt.length();
		fwrite(&len, sizeof(int), 1, fp);
		//QByteArray baComt = comt.toLatin1();
		//char* pszComt = baComt.data();
		std::string stdStrComt = codec->fromUnicode(comt).data();
		fwrite(stdStrComt.c_str(), len, 1, fp);
	}
	return true;
}

//************************************
// Method:    copy
// Brief:	  ¿½±´DSID
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CS57RecDsgi * pDsgi
//************************************
void CS57FieldDSID::copy(CS57RecDsgi* pDsgi)
{
	numFields = pDsgi->m_Dsid.numFields;
	if (numFields == 0)
		return;

	rcnm = pDsgi->m_Dsid.rcnm;
	rcid = pDsgi->m_Dsid.rcid;
	expp = pDsgi->m_Dsid.expp;
	intu = pDsgi->m_Dsid.intu;

	dsnm = pDsgi->m_Dsid.dsnm;
	edtn = pDsgi->m_Dsid.edtn;
	updn = pDsgi->m_Dsid.updn;

	strcpy(uadt, pDsgi->m_Dsid.uadt);
	strcpy(isdt, pDsgi->m_Dsid.isdt);

	sted = pDsgi->m_Dsid.sted;
	prsp = pDsgi->m_Dsid.prsp;

	psdn = pDsgi->m_Dsid.psdn;
	pred = pDsgi->m_Dsid.pred;

	prof = pDsgi->m_Dsid.prof;
	agen = pDsgi->m_Dsid.agen;

	comt = pDsgi->m_Dsid.comt;
}

CS57FieldDSSI::CS57FieldDSSI()
{}

CS57FieldDSSI::~CS57FieldDSSI()
{}

void CS57FieldDSSI::init(DDFRecord* ddfRec)
{
	dstr = ddfRec->GetIntSubfield("DSSI", 0, "DSTR", 0);
	aall = ddfRec->GetIntSubfield("DSSI", 0, "AALL", 0);
	nall = ddfRec->GetIntSubfield("DSSI", 0, "NALL", 0);
	nomr = ddfRec->GetIntSubfield("DSSI", 0, "NOMR", 0);
	nocr = ddfRec->GetIntSubfield("DSSI", 0, "NOCR", 0);
	nogr = ddfRec->GetIntSubfield("DSSI", 0, "NOGR", 0);
	nolr = ddfRec->GetIntSubfield("DSSI", 0, "NOLR", 0);
	noin = ddfRec->GetIntSubfield("DSSI", 0, "NOIN", 0);
	nocn = ddfRec->GetIntSubfield("DSSI", 0, "NOCN", 0);
	noed = ddfRec->GetIntSubfield("DSSI", 0, "NOED", 0);
	nofa = ddfRec->GetIntSubfield("DSSI", 0, "NOFA", 0);
	numFields = 1;
}

void CS57FieldDSSI::ioFile(FILE *fp, bool ioFlag)
{
	if (ioFlag)
	{
		fread(&numFields, sizeof(unsigned short), 1, fp);
		if (numFields == 0) return;
		fread(&dstr, sizeof(unsigned char), 1, fp);
		fread(&aall, sizeof(unsigned char), 1, fp);
		fread(&nall, sizeof(unsigned char), 1, fp);
		fread(&nomr, sizeof(unsigned int), 1, fp);
		fread(&nocr, sizeof(unsigned int), 1, fp);
		fread(&nogr, sizeof(unsigned int), 1, fp);
		fread(&nolr, sizeof(unsigned int), 1, fp);
		fread(&noin, sizeof(unsigned int), 1, fp);
		fread(&nocn, sizeof(unsigned int), 1, fp);
		fread(&noed, sizeof(unsigned int), 1, fp);
		fread(&nofa, sizeof(unsigned int), 1, fp);
	}
	else
	{
		fwrite(&numFields, sizeof(unsigned short), 1, fp);
		if (numFields == 0) return;
		fwrite(&dstr, sizeof(unsigned char), 1, fp);
		fwrite(&aall, sizeof(unsigned char), 1, fp);
		fwrite(&nall, sizeof(unsigned char), 1, fp);
		fwrite(&nomr, sizeof(unsigned int), 1, fp);
		fwrite(&nocr, sizeof(unsigned int), 1, fp);
		fwrite(&nogr, sizeof(unsigned int), 1, fp);
		fwrite(&nolr, sizeof(unsigned int), 1, fp);
		fwrite(&noin, sizeof(unsigned int), 1, fp);
		fwrite(&nocn, sizeof(unsigned int), 1, fp);
		fwrite(&noed, sizeof(unsigned int), 1, fp);
		fwrite(&nofa, sizeof(unsigned int), 1, fp);
	}
}

void CS57FieldDSSI::copy(CS57RecDsgi* pDsgi)
{
	numFields = pDsgi->m_Dssi.numFields;
	if (numFields == 0)
		return;

	dstr = pDsgi->m_Dssi.dstr;
	aall = pDsgi->m_Dssi.aall;
	nall = pDsgi->m_Dssi.nall;
	nomr = pDsgi->m_Dssi.nomr;
	nocr = pDsgi->m_Dssi.nocr;
	nogr = pDsgi->m_Dssi.nogr;
	nolr = pDsgi->m_Dssi.nolr;
	noin = pDsgi->m_Dssi.noin;
	nocn = pDsgi->m_Dssi.nocn;
	noed = pDsgi->m_Dssi.noed;
	nofa = pDsgi->m_Dssi.nofa;
}

CS57RecDsgi::CS57RecDsgi()
{}

CS57RecDsgi::~CS57RecDsgi()
{}

bool CS57RecDsgi::init(DDFRecord* ddfRec)
{
	Q_ASSERT(ddfRec);
	if (!ddfRec)
		return false;

	if (ddfRec->FindField("DSID", 0) != NULL) m_Dsid.init(ddfRec);
	if (ddfRec->FindField("DSSI", 0) != NULL) m_Dssi.init(ddfRec);
	return true;
}

void CS57RecDsgi::ioFile(FILE *fp, bool ioFlag)
{
	m_Dsid.ioFile(fp, ioFlag);
	m_Dssi.ioFile(fp, ioFlag);
}

void CS57RecDsgi::copy(CS57RecDsgi* pDsgi)
{
	m_Dsid.copy(pDsgi);
	m_Dssi.copy(pDsgi);
}
