#include "stdafx.h"
#include "codec.h"
#include "cs57recdsgr.h"

using namespace Doc;
CS57FieldDSPM::CS57FieldDSPM()
{}

CS57FieldDSPM::~CS57FieldDSPM()
{}

void CS57FieldDSPM::init(DDFRecord* ddfRec)
{
	rcnm = ddfRec->GetIntSubfield("DSPM", 0, "RCNM", 0);
	rcid = ddfRec->GetIntSubfield("DSPM", 0, "RCID", 0);
	hdat = ddfRec->GetIntSubfield("DSPM", 0, "HDAT", 0);
	vdat = ddfRec->GetIntSubfield("DSPM", 0, "VDAT", 0);
	sdat = ddfRec->GetIntSubfield("DSPM", 0, "SDAT", 0);
	cscl = ddfRec->GetIntSubfield("DSPM", 0, "CSCL", 0);
	duni = ddfRec->GetIntSubfield("DSPM", 0, "DUNI", 0);
	huni = ddfRec->GetIntSubfield("DSPM", 0, "HUNI", 0);
	puni = ddfRec->GetIntSubfield("DSPM", 0, "PUNI", 0);
	coun = ddfRec->GetIntSubfield("DSPM", 0, "COUN", 0);
	comf = ddfRec->GetIntSubfield("DSPM", 0, "COMF", 0);
	somf = ddfRec->GetIntSubfield("DSPM", 0, "SOMF", 0);
	comt = ddfRec->GetStringSubfield("DSPM", 0, "COMT", 0);
	numFields = 1;
}

void CS57FieldDSPM::ioFile(FILE *fp, bool ioFlag)
{
	int len;
	if (ioFlag)
	{
		fread(&numFields, sizeof(unsigned short), 1, fp);
		if (numFields == 0) return;
		fread(&rcnm, sizeof(unsigned char), 1, fp);
		fread(&rcid, sizeof(unsigned int), 1, fp);
		fread(&hdat, sizeof(unsigned char), 1, fp);
		fread(&vdat, sizeof(unsigned char), 1, fp);
		fread(&sdat, sizeof(unsigned char), 1, fp);
		fread(&cscl, sizeof(unsigned int), 1, fp);
		fread(&duni, sizeof(unsigned char), 1, fp);
		fread(&huni, sizeof(unsigned char), 1, fp);
		fread(&puni, sizeof(unsigned char), 1, fp);
		fread(&coun, sizeof(unsigned char), 1, fp);
		fread(&comf, sizeof(unsigned int), 1, fp);
		fread(&somf, sizeof(unsigned int), 1, fp);

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
		if (numFields == 0) return;
		fwrite(&rcnm, sizeof(unsigned char), 1, fp);
		fwrite(&rcid, sizeof(unsigned int), 1, fp);
		fwrite(&hdat, sizeof(unsigned char), 1, fp);
		fwrite(&vdat, sizeof(unsigned char), 1, fp);
		fwrite(&sdat, sizeof(unsigned char), 1, fp);
		fwrite(&cscl, sizeof(unsigned int), 1, fp);
		fwrite(&duni, sizeof(unsigned char), 1, fp);
		fwrite(&huni, sizeof(unsigned char), 1, fp);
		fwrite(&puni, sizeof(unsigned char), 1, fp);
		fwrite(&coun, sizeof(unsigned char), 1, fp);
		fwrite(&comf, sizeof(unsigned int), 1, fp);
		fwrite(&somf, sizeof(unsigned int), 1, fp);

		len = comt.length();
		fwrite(&len, sizeof(int), 1, fp);
		//QByteArray baComt = comt.toLatin1();
		//char* pszComt = baComt.data();
		std::string stdStrComt = codec->fromUnicode(comt).data();
		fwrite(stdStrComt.c_str(), len, 1, fp);
	}
}

void CS57FieldDSPM::copy(CS57RecDsgr* pDsgr)
{
	numFields = pDsgr->m_Dspm.numFields;
	if (numFields == 0) return;

	rcnm = pDsgr->m_Dspm.rcnm;
	rcid = pDsgr->m_Dspm.rcid;
	hdat = pDsgr->m_Dspm.hdat;
	vdat = pDsgr->m_Dspm.vdat;
	sdat = pDsgr->m_Dspm.sdat;
	cscl = pDsgr->m_Dspm.cscl;
	duni = pDsgr->m_Dspm.duni;
	huni = pDsgr->m_Dspm.huni;
	puni = pDsgr->m_Dspm.puni;
	coun = pDsgr->m_Dspm.coun;
	comf = pDsgr->m_Dspm.comf;
	somf = pDsgr->m_Dspm.somf;
	comt = pDsgr->m_Dspm.comt;
}

CS57FieldDSPR::CS57FieldDSPR()
{}

CS57FieldDSPR::~CS57FieldDSPR()
{}

void CS57FieldDSPR::init(DDFRecord* ddfRec)
{
	proj = ddfRec->GetIntSubfield("DSPR", 0, "PROJ", 0);
	prp1 = ddfRec->GetIntSubfield("DSPR", 0, "PRP1", 0);
	prp2 = ddfRec->GetIntSubfield("DSPR", 0, "PRP2", 0);
	prp3 = ddfRec->GetIntSubfield("DSPR", 0, "PRP3", 0);
	prp4 = ddfRec->GetIntSubfield("DSPR", 0, "PRP4", 0);
	feas = ddfRec->GetIntSubfield("DSPR", 0, "FEAS", 0);
	fnor = ddfRec->GetIntSubfield("DSPR", 0, "FNOR", 0);
	fpmf = ddfRec->GetIntSubfield("DSPR", 0, "FPMF", 0);
	comt = ddfRec->GetStringSubfield("DSPR", 0, "COMT", 0);
	numFields = 1;
}

void CS57FieldDSPR::ioFile(FILE *fp, bool ioFlag)
{
	int len;
	if (ioFlag)
	{
		fread(&numFields, sizeof(unsigned short), 1, fp);
		if (numFields == 0) return;
		fread(&proj, sizeof(unsigned char), 1, fp);
		fread(&prp1, sizeof(long), 1, fp);
		fread(&prp2, sizeof(long), 1, fp);
		fread(&prp3, sizeof(long), 1, fp);
		fread(&prp4, sizeof(long), 1, fp);
		fread(&feas, sizeof(long), 1, fp);
		fread(&fnor, sizeof(long), 1, fp);
		fread(&fpmf, sizeof(unsigned int), 1, fp);

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
		if (numFields == 0) return;
		fwrite(&proj, sizeof(unsigned char), 1, fp);
		fwrite(&prp1, sizeof(long), 1, fp);
		fwrite(&prp2, sizeof(long), 1, fp);
		fwrite(&prp3, sizeof(long), 1, fp);
		fwrite(&prp4, sizeof(long), 1, fp);
		fwrite(&feas, sizeof(long), 1, fp);
		fwrite(&fnor, sizeof(long), 1, fp);
		fwrite(&fpmf, sizeof(unsigned int), 1, fp);

		len = comt.length();
		fwrite(&len, sizeof(int), 1, fp);
		//QByteArray baComt = comt.toLatin1();
		//char* pszComt = baComt.data();
		std::string stdStrComt = codec->fromUnicode(comt).data();
		fwrite(stdStrComt.c_str(), len, 1, fp);
	}
}

void CS57FieldDSPR::copy(CS57RecDsgr* pDsgr)
{
	numFields = pDsgr->m_Dspr.numFields;
	if (numFields == 0)return;

	proj = pDsgr->m_Dspr.proj;
	prp1 = pDsgr->m_Dspr.prp1;
	prp2 = pDsgr->m_Dspr.prp2;
	prp3 = pDsgr->m_Dspr.prp3;
	prp4 = pDsgr->m_Dspr.prp4;
	feas = pDsgr->m_Dspr.feas;
	fnor = pDsgr->m_Dspr.fnor;
	fpmf = pDsgr->m_Dspr.fpmf;
	comt = pDsgr->m_Dspr.comt;
}

CS57RecDsgr::CS57RecDsgr()
{}

CS57RecDsgr::~CS57RecDsgr()
{}

bool CS57RecDsgr::init(DDFRecord* ddfRec)
{
	Q_ASSERT(ddfRec);
	if (!ddfRec)
		return false;

	if (ddfRec->FindField("DSPM", 0) != NULL) m_Dspm.init(ddfRec);
	if (ddfRec->FindField("DSPR", 0) != NULL) m_Dspr.init(ddfRec);

	return true;
}

void CS57RecDsgr::ioFile(FILE *fp, bool ioFlag)
{
	m_Dspm.ioFile(fp, ioFlag);
	m_Dspr.ioFile(fp, ioFlag);
}

void CS57RecDsgr::copy(CS57RecDsgr* pDsgr)
{
	m_Dspm.copy(pDsgr);
	m_Dspr.copy(pDsgr);
}
