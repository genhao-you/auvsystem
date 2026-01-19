#include "stdafx.h"
#include "cs57recvector.h"
#include "codec.h"

using namespace Doc;
CS57FieldVRID::CS57FieldVRID()
	: numFields(0)
{}

CS57FieldVRID::~CS57FieldVRID()
{}

void CS57FieldVRID::init(DDFRecord* ddfRec)
{
	rcnm = ddfRec->GetIntSubfield("VRID", 0, "RCNM", 0);
	rcid = ddfRec->GetIntSubfield("VRID", 0, "RCID", 0);
	rver = ddfRec->GetIntSubfield("VRID", 0, "RVER", 0);
	ruin = ddfRec->GetIntSubfield("VRID", 0, "RUIN", 0);

	numFields = 1;
}

void CS57FieldVRID::ioFile(FILE *fp, bool ioFlag)
{
	if (ioFlag)
	{
		fread(&numFields, sizeof(unsigned char), 1, fp);
		if (numFields == 0) return;

		fread(&rcnm, sizeof(unsigned char), 1, fp);
		fread(&rcid, sizeof(unsigned int), 1, fp);
		fread(&rver, sizeof(unsigned short), 1, fp);
		fread(&ruin, sizeof(unsigned char), 1, fp);
	}
	else
	{
		fwrite(&numFields, sizeof(unsigned char), 1, fp);
		if (numFields == 0) return;

		fwrite(&rcnm, sizeof(unsigned char), 1, fp);
		fwrite(&rcid, sizeof(unsigned int), 1, fp);
		fwrite(&rver, sizeof(unsigned short), 1, fp);
		fwrite(&ruin, sizeof(unsigned char), 1, fp);
	}
}

void CS57FieldVRID::copy(CS57RecVector* pRecVec)
{
	numFields = pRecVec->m_Vrid.numFields;
	if (numFields == 0)return;

	rcnm = pRecVec->m_Vrid.rcnm;
	rcid = pRecVec->m_Vrid.rcid;
	rver = pRecVec->m_Vrid.rver;
	ruin = pRecVec->m_Vrid.ruin;
}

CS57FieldATTV::CS57FieldATTV()
{}

CS57FieldATTV::~CS57FieldATTV()
{}

void CS57FieldATTV::init(DDFRecord* ddfRec)
{
	int numFields = ddfRec->FindField("ATTV", 0)->GetRepeatCount();
	for (int i = 0;i < numFields;i++)
	{
		sAttv oAttv;
		oAttv.attl = ddfRec->GetIntSubfield("ATTV", 0, "ATTL", i);
		oAttv.atvl = ddfRec->GetStringSubfield("ATTV", 0, "ATVL", i);
		m_vecAttv.push_back(oAttv);
	}
}

void CS57FieldATTV::ioFile(FILE *fp, bool ioFlag)
{
	int len;
	if (ioFlag)
	{
		int numFields;
		fread(&numFields, sizeof(int), 1, fp);
		if (numFields == 0) return;
		for (int i = 0;i<numFields;i++)
		{
			sAttv oAttv;
			fread(&oAttv.attl, sizeof(unsigned short), 1, fp);

			fread(&len, sizeof(int), 1, fp);
			char* pszAttv = new char[len + 1];
			fread(pszAttv, len, 1, fp);
			pszAttv[len] = '\0';
			oAttv.atvl = pszAttv;
			delete[] pszAttv;
			pszAttv = NULL;

			m_vecAttv.push_back(oAttv);
		}
	}
	else
	{
		int numFields = m_vecAttv.size();
		fwrite(&numFields, sizeof(int), 1, fp);
		if (numFields == 0) return;
		for (int i = 0;i < numFields;i++)
		{
			fwrite(&m_vecAttv[i].attl, sizeof(unsigned short), 1, fp);

			len = m_vecAttv[i].atvl.length();
			fwrite(&len, sizeof(int), 1, fp);
			//QByteArray baAtvl = m_vecAttv[i].atvl.toLatin1();
			//char* pszAtvl = baAtvl.data();
			std::string stdStrAtvl = codec->fromUnicode(m_vecAttv[i].atvl).data();
			fwrite(stdStrAtvl.c_str(), len, 1, fp);
		}
	}
}

void CS57FieldATTV::copy(CS57RecVector* pRecVec)
{
	int size = pRecVec->m_Attv.m_vecAttv.size();
	if (size == 0)return;

	for (int i = 0; i < size; i++)
	{
		sAttv oAttv;
		oAttv.attl = pRecVec->m_Attv.m_vecAttv[i].attl;
		oAttv.atvl = pRecVec->m_Attv.m_vecAttv[i].atvl;

		m_vecAttv.push_back(oAttv);
	}
}

CS57FieldVRPC::CS57FieldVRPC()
	: numFields(0)
{}

CS57FieldVRPC::~CS57FieldVRPC()
{}

void CS57FieldVRPC::init(DDFRecord* ddfRec)
{
	vpui = ddfRec->GetIntSubfield("VRPC", 0, "VPUI", 0);
	vpix = ddfRec->GetIntSubfield("VRPC", 0, "VPIX", 0);
	nvpt = ddfRec->GetIntSubfield("VRPC", 0, "NVPT", 0);

	numFields = 1;
}

void CS57FieldVRPC::copy(CS57RecVector* pRecVec)
{
	numFields = pRecVec->m_Vrpc.numFields;
	if (numFields == 0)return;

	nvpt = pRecVec->m_Vrpc.nvpt;
	vpix = pRecVec->m_Vrpc.vpix;
	vpui = pRecVec->m_Vrpc.vpui;
}

CS57FieldVRPT::CS57FieldVRPT()
{}

CS57FieldVRPT::~CS57FieldVRPT()
{}

void CS57FieldVRPT::init(DDFRecord* ddfRec)
{
	int numFields = ddfRec->FindField("VRPT", 0)->GetRepeatCount();
	for (int i = 0;i < numFields;i++)
	{
		sVrpt oVrpt;
		memcpy(&oVrpt.rcnm, ddfRec->GetStringSubfield("VRPT", 0, "NAME", i), 1);
		memcpy(&oVrpt.rcid, ddfRec->GetStringSubfield("VRPT", 0, "NAME", i) + 1, 4);
		oVrpt.ornt = ddfRec->GetIntSubfield("VRPT", 0, "ORNT", i);
		oVrpt.usag = ddfRec->GetIntSubfield("VRPT", 0, "USAG", i);
		oVrpt.topi = ddfRec->GetIntSubfield("VRPT", 0, "TOPI", i);
		oVrpt.mask = ddfRec->GetIntSubfield("VRPT", 0, "MASK", i);
		m_vecVrpt.push_back(oVrpt);
	}
}

void CS57FieldVRPT::ioFile(FILE *fp, bool ioFlag)
{
	if (ioFlag)
	{
		int numFields;
		fread(&numFields, sizeof(int), 1, fp);
		if (numFields == 0) return;
		for (int i = 0;i < numFields;i++)
		{
			sVrpt oVrpt;
			fread(&oVrpt.rcnm, sizeof(unsigned char), 1, fp);
			fread(&oVrpt.rcid, sizeof(unsigned int), 1, fp);
			fread(&oVrpt.ornt, sizeof(unsigned char), 1, fp);
			fread(&oVrpt.usag, sizeof(unsigned char), 1, fp);
			fread(&oVrpt.topi, sizeof(unsigned char), 1, fp);
			fread(&oVrpt.mask, sizeof(unsigned char), 1, fp);
			fread(&oVrpt.idx,  sizeof(unsigned short), 1, fp);

			m_vecVrpt.push_back(oVrpt);
		}
	}
	else
	{
		int numFields = m_vecVrpt.size();
		fwrite(&numFields, sizeof(int), 1, fp);
		if (numFields == 0) return;
		for (int i = 0;i < numFields;i++)
		{
			fwrite(&m_vecVrpt[i].rcnm, sizeof(unsigned char), 1, fp);
			fwrite(&m_vecVrpt[i].rcid, sizeof(unsigned int), 1, fp);
			fwrite(&m_vecVrpt[i].ornt, sizeof(unsigned char), 1, fp);
			fwrite(&m_vecVrpt[i].usag, sizeof(unsigned char), 1, fp);
			fwrite(&m_vecVrpt[i].topi, sizeof(unsigned char), 1, fp);
			fwrite(&m_vecVrpt[i].mask, sizeof(unsigned char), 1, fp);
			fwrite(&m_vecVrpt[i].idx,  sizeof(unsigned short), 1, fp);
		}
	}
}

void CS57FieldVRPT::copy(CS57RecVector* pRecVec)
{
	int size = pRecVec->m_Vrpt.m_vecVrpt.size();
	if (size == 0)return;

	for (int i = 0; i < size; i++)
	{
		sVrpt oVrpt;
		oVrpt.rcnm = pRecVec->m_Vrpt.m_vecVrpt[i].rcnm;
		oVrpt.rcid = pRecVec->m_Vrpt.m_vecVrpt[i].rcid;
		oVrpt.ornt = pRecVec->m_Vrpt.m_vecVrpt[i].ornt;
		oVrpt.usag = pRecVec->m_Vrpt.m_vecVrpt[i].usag;
		oVrpt.topi = pRecVec->m_Vrpt.m_vecVrpt[i].topi;
		oVrpt.mask = pRecVec->m_Vrpt.m_vecVrpt[i].mask;
		oVrpt.idx  = pRecVec->m_Vrpt.m_vecVrpt[i].idx;

		m_vecVrpt.push_back(oVrpt);
	}
}

CS57FieldSGCC::CS57FieldSGCC()
	: numFields(0)
{}

CS57FieldSGCC::~CS57FieldSGCC()
{}

void CS57FieldSGCC::init(DDFRecord* ddfRec)
{
	ccui = ddfRec->GetIntSubfield("SGCC", 0, "CCUI", 0);
	ccnc = ddfRec->GetIntSubfield("SGCC", 0, "CCNC", 0);
	ccix = ddfRec->GetIntSubfield("SGCC", 0, "CCIX", 0);

	numFields = 1;
}

void CS57FieldSGCC::copy(CS57RecVector* pRecVec)
{
	numFields = pRecVec->m_Sgcc.numFields;
	if (numFields == 0)return;

	ccui = pRecVec->m_Sgcc.ccui;
	ccnc = pRecVec->m_Sgcc.ccnc;
	ccix = pRecVec->m_Sgcc.ccix;
}

CS57FieldSG2D::CS57FieldSG2D()
{}

CS57FieldSG2D::~CS57FieldSG2D()
{}

void CS57FieldSG2D::init(DDFRecord* ddfRec)
{
	int numFields = ddfRec->FindField("SG2D", 0)->GetRepeatCount();
	for (int i = 0; i < numFields;i++)
	{
		sSg2d oSg2d;
		oSg2d.ycoo = ddfRec->GetIntSubfield("SG2D", 0, "YCOO", i);
		oSg2d.xcoo = ddfRec->GetIntSubfield("SG2D", 0, "XCOO", i);

		m_vecSg2d.push_back(oSg2d);
	}
}

void CS57FieldSG2D::ioFile(FILE *fp, bool ioFlag)
{
	if (ioFlag)
	{
		int numFields;
		fread(&numFields, sizeof(int), 1, fp);
		if (numFields == 0) return;
		for (int i = 0;i < numFields;i++)
		{
			sSg2d oSg2d;
			fread(&oSg2d.ycoo, sizeof(long), 1, fp);
			fread(&oSg2d.xcoo, sizeof(long), 1, fp);

			m_vecSg2d.push_back(oSg2d);
		}
	}
	else
	{
		int numFields = m_vecSg2d.size();
		fwrite(&numFields, sizeof(int), 1, fp);
		if (numFields == 0) return;
		for (int i = 0;i < numFields;i++)
		{
			fwrite(&m_vecSg2d[i].ycoo, sizeof(long), 1, fp);
			fwrite(&m_vecSg2d[i].xcoo, sizeof(long), 1, fp);
		}
	}
}

void CS57FieldSG2D::copy(CS57RecVector* pRecVec)
{
	int size = pRecVec->m_Sg2d.m_vecSg2d.size();
	if (size == 0)return;

	for (int i = 0; i < size; i++)
	{
		sSg2d oSg2d;
		oSg2d.xcoo = pRecVec->m_Sg2d.m_vecSg2d[i].xcoo;
		oSg2d.ycoo = pRecVec->m_Sg2d.m_vecSg2d[i].ycoo;

		m_vecSg2d.push_back(oSg2d);
	}
}

CS57FieldSG3D::CS57FieldSG3D()
{}

CS57FieldSG3D::~CS57FieldSG3D()
{}

void CS57FieldSG3D::init(DDFRecord* ddfRec)
{
	int numFields = ddfRec->FindField("SG3D", 0)->GetRepeatCount();
	for (unsigned int i = 0; i < numFields;i++)
	{
		sSg3d oSg3d;
		oSg3d.ycoo = ddfRec->GetIntSubfield("SG3D", 0, "YCOO", i);
		oSg3d.xcoo = ddfRec->GetIntSubfield("SG3D", 0, "XCOO", i);
		oSg3d.ve3d = ddfRec->GetIntSubfield("SG3D", 0, "VE3D", i);

		m_vecSg3d.push_back(oSg3d);
	}
}

void CS57FieldSG3D::ioFile(FILE *fp, bool ioFlag)
{
	if (ioFlag)
	{
		int numFields;
		fread(&numFields, sizeof(int), 1, fp);
		if (numFields == 0) return;
		for (int i = 0;i < numFields;i++)
		{
			sSg3d oSg3d;
			fread(&oSg3d.ycoo, sizeof(long), 1, fp);
			fread(&oSg3d.xcoo, sizeof(long), 1, fp);
			fread(&oSg3d.ve3d, sizeof(long), 1, fp);

			m_vecSg3d.push_back(oSg3d);
		}
	}
	else
	{
		int numFields = m_vecSg3d.size();
		fwrite(&numFields, sizeof(int), 1, fp);
		if (numFields == 0) return;
		for (int i = 0;i < numFields;i++)
		{
			fwrite(&m_vecSg3d[i].ycoo, sizeof(long), 1, fp);
			fwrite(&m_vecSg3d[i].xcoo, sizeof(long), 1, fp);
			fwrite(&m_vecSg3d[i].ve3d, sizeof(long), 1, fp);
		}
	}
}

void CS57FieldSG3D::copy(CS57RecVector* pRecVec)
{
	int size = pRecVec->m_Sg3d.m_vecSg3d.size();
	if (size == 0)return;

	for (int i = 0; i < size; i++)
	{
		sSg3d oSg3d;
		oSg3d.xcoo = pRecVec->m_Sg3d.m_vecSg3d[i].xcoo;
		oSg3d.ycoo = pRecVec->m_Sg3d.m_vecSg3d[i].ycoo;
		oSg3d.ve3d = pRecVec->m_Sg3d.m_vecSg3d[i].ve3d;

		m_vecSg3d.push_back(oSg3d);
	}
}

CS57RecVector::CS57RecVector()
{}

CS57RecVector::~CS57RecVector()
{}

bool CS57RecVector::init(DDFRecord* ddfRec)
{
	Q_ASSERT(ddfRec);
	if (!ddfRec) return false;

	if (ddfRec->FindField("VRID", 0) != NULL) m_Vrid.init(ddfRec);
	if (ddfRec->FindField("ATTV", 0) != NULL) m_Attv.init(ddfRec);
	if (ddfRec->FindField("VRPC", 0) != NULL) m_Vrpc.init(ddfRec);
	if (ddfRec->FindField("VRPT", 0) != NULL) m_Vrpt.init(ddfRec);
	if (ddfRec->FindField("SGCC", 0) != NULL) m_Sgcc.init(ddfRec);
	if (ddfRec->FindField("SG2D", 0) != NULL) m_Sg2d.init(ddfRec);
	if (ddfRec->FindField("SG3D", 0) != NULL) m_Sg3d.init(ddfRec);

	return true;
}

void CS57RecVector::ioFile(FILE *fp, bool ioFlag)
{
	m_Vrid.ioFile(fp, ioFlag);
	m_Attv.ioFile(fp, ioFlag);
	m_Vrpt.ioFile(fp, ioFlag);
	m_Sg2d.ioFile(fp, ioFlag);
	m_Sg3d.ioFile(fp, ioFlag);
}

void CS57RecVector::copy(CS57RecVector* pRecVec)
{
	m_Vrid.copy(pRecVec);
	m_Attv.copy(pRecVec);
	m_Vrpc.copy(pRecVec);
	m_Vrpt.copy(pRecVec);
	m_Sgcc.copy(pRecVec);
	m_Sg2d.copy(pRecVec);
	m_Sg3d.copy(pRecVec);
}
