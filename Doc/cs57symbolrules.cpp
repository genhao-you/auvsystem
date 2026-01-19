#include "stdafx.h"
#include "cs57symbolrules.h"
#include "cs57recfeature.h"
#include "cs57recvector.h"
#include "cs57cell.h"
#include <QTextCodec>
#define LISTSIZE		32   // list size
#define UNKNOWN			1e6
#define UNKNOWN_DOUBLE	-9

using namespace Doc;
CS57SymbolRules::CS57SymbolRules(CS57RecFeature* pFE)
{
	m_pRecFeature = pFE;
}

CS57SymbolRules::~CS57SymbolRules()
{}

static double _MARparamVal[] =
{
	0.0,    // NONE
	1,   // SHOW_TEXT
	1,   // TWO_SHADES
	8.0,    // SAFETY_CONTOUR
			//0.0,     // SAFETY_CONTOUR  --to test DEPCNT02 selection (GL) in CA49995A.000
			//0.5,     // SAFETY_CONTOUR  --to test DEPCNT02 selection (GL) in CA49995A.000
			5.0,    // SAFETY_DEPTH
					//5.0,      // SAFETY_DEPTH
					3.0,    // SHALLOW_CONTOUR
					10.0,   // DEEP_CONTOUR
							//FALSE,    // SHALLOW_PATTERN
							1,   // SHALLOW_PATTERN
							0,  // SHIPS_OUTLINE
							0.0,    // DISTANCE_TAGS
							0.0,    // TIME_TAGS
							1,   // FULL_SECTORS
							1,   // SYMBOLIZED_BND
							1,   // SYMPLIFIED_PNT
									//    'D',      // S52_MAR_DISP_CATEGORY --DISPLAYBASE
									//    'S',      // S52_MAR_DISP_CATEGORY --STANDARD
									'O',    // S52_MAR_DISP_CATEGORY --OTHER
											//    0,        // S52_MAR_COLOR_PALETTE --DAY_BRIGHT
											1,      // S52_MAR_COLOR_PALETTE --DAY_BLACKBACK
													//    2,        // S52_MAR_COLOR_PALETTE --DAY_WHITEBACK
													//    3,        // S52_MAR_COLOR_PALETTE --DUSK
													//    4,        // S52_MAR_COLOR_PALETTE --NIGHT
													16.0    // NUM
};

double CS57SymbolRules::S52_getMarinerParam(S52_MAR_param_t param)
{
	return _MARparamVal[param];
}
int CS57SymbolRules::S52_setMarinerParam(S52_MAR_param_t param, double val)
{
	if (S52_MAR_NONE < param && param < S52_MAR_NUM)
		_MARparamVal[param] = val;
	else
		return 0;

	return 1;
}

//灯标
string CS57SymbolRules::LIGHTS06()
{
	string lights06;
	double valnmr = 9.0;
	double orient = 0.0;
	char	catlitstr[20] = { '\0' };
	char	litvisstr[20] = { '\0' };;
	char    catlit[LISTSIZE] = { '\0' };
	char    litvis[LISTSIZE] = { '\0' };
	char    col_str[20] = { '\0' };
	int 	litchr;

	bool     flare_at_45 = false;
	double   sectr1 = UNKNOWN_DOUBLE;
	double   sectr2 = UNKNOWN_DOUBLE;
	double   sweep = 0.;
	char     colist[LISTSIZE] = { '\0' };   // colour list
	bool     b_isflare = false;

	string orientstr;
	for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size();i++)
	{
		switch (m_pRecFeature->m_Attf.m_vecAttf[i].attl)
		{
		case 37:		//CATLIT	灯标类	category of light	L
			strcpy(catlitstr, (const char*)m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toLocal8Bit());
			break;
		case 75:		//COLOUR	颜色	colour	L
			strcpy(col_str, (const char*)m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toLocal8Bit());
			break;
		case 107:		// 107	LITCHR	灯质	light characteristic	E
			litchr = atoi((const char*)m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toLocal8Bit());
			break;
		case 108:		// LITVIS	灯标能见度	light visibility	L
			strcpy(litvisstr, (const char*)m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toLocal8Bit());
			break;
		case 117:		// ORIENT	方位	orientation	F
			orient = atof((const char*)m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toLocal8Bit());
			break;

		case 136:		// 136	SECTR1	扇形界线1	sector limit one	F
			sectr1 = atof((const char*)m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toLocal8Bit());
			break;
		case 137:		// SECTR2	扇形界限2	sector limit two	F

			sectr2 = atof((const char*)m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toLocal8Bit());
			break;
		case 178:		// VALNMR	标定作用距离	value of nominal range	F
			valnmr = atof((const char*)m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toLocal8Bit());
			break;
		default:
			break;
		}
	}

	if (strlen(catlitstr))
	{
		_parseList(catlitstr, catlit, sizeof(colist));

		if (strpbrk(catlit, "\010\013")) {
			lights06.append(";SY(LIGHTS82)");
			goto l06_end;
		}

		if (strpbrk(catlit, "\011")) {
			lights06.append(";SY(LIGHTS81)");
			goto l06_end;
		}
	}

	// Continuation A

	if (strlen(col_str))
		_parseList(col_str, colist, sizeof(colist));
	else
	{
		colist[0] = '\014';  // magenta (12)
		colist[1] = '\000';
	}

	if ((-9 == sectr1) || (-9 == sectr2))
	{
		// This is not a sector light
		string ssym;
		if (valnmr < 10.0) {

			//TODO create LightArray in s57chart.
			ssym = _selSYcol(colist, 0, valnmr);              // flare
			b_isflare = true;
			flare_at_45 = false;
		}
		else
		{
			ssym = _selSYcol(colist, 1, valnmr);              // all round light
			b_isflare = false;
		}

		//  Is the light a directional or moire?
		if (strpbrk(catlit, "\001\016"))
		{
			if (orientstr.length())
			{
				lights06.append(ssym);
				lights06.append(orientstr);
				lights06.append(";TE('%03.0lf deg','ORIENT',3,3,3,'15110',3,1,CHBLK,23)");
			}
			else
				lights06.append(";SY(QUESMRK1)");
		}
		else
		{
			lights06.append(ssym);

			if (b_isflare)
			{
				if (flare_at_45)
					lights06.append(",45)");
				else
					lights06.append(",135)");
			}
		}
		goto l06_end;
	}

	// Continuation B --sector light
	if (-9 == sectr1)
	{
		sectr1 = 0.0;
		sectr2 = 0.0;
	}
	else
		sweep = (sectr1 > sectr2) ? sectr2 - sectr1 + 360 : sectr2 - sectr1;


	if (sweep < 1.0 || sweep == 360.0)
	{
		// handle all round light
		string ssym = _selSYcol(colist, 1, valnmr);           // all round light
		lights06.append(ssym);

		goto l06_end;
	}
	// setup sector
	{
		double arc_radius = 20.;                // mm
		double sector_radius = 25.;

		//Another non-standard extension....
		//Sector light arc radius is scaled if the light has a reasonable VALNMR attribute
		if (valnmr > 0)
		{
			if (valnmr < 15.0)
				arc_radius = 10.;
			else if (valnmr < 30.0)
				arc_radius = 15.;
			else
				arc_radius = 20.;
		}

		char sym[80];
		strcpy(sym, ";CA(OUTLW, 4");


		// max 1 color
		if ('\0' == colist[1])
		{
			if (strpbrk(colist, "\003"))
				strcat(sym, ",LITRD, 2");
			else if (strpbrk(colist, "\004"))
				strcat(sym, ",LITGN, 2");
			else if (strpbrk(colist, "\001\006\013"))
				strcat(sym, ",LITYW, 2");
			else
				strcat(sym, ",CHMGD, 2");                 // default is magenta

		}
		else if ('\0' == colist[2])
		{
			if (strpbrk(colist, "\001") && strpbrk(colist, "\003"))
				strcat(sym, ",LITRD, 2");
			else if (strpbrk(colist, "\001") && strpbrk(colist, "\004"))
				strcat(sym, ",LITGN, 2");
			else
				strcat(sym, ",CHMGD, 2");                 // default is magenta
		}
		else
			strcat(sym, ",CHMGD, 2");                 // default is magenta


		if (strlen(litvisstr))               // Obscured/faint sector?
		{
			_parseList(litvisstr, litvis, sizeof(litvis));

			if (strpbrk(litvis, "\003\007\010"))
				strcpy(sym, ";CA(CHBLK, 4,CHBRN, 1");
		}

		if (sectr2 <= sectr1)
			sectr2 += 360;

		//Sectors are defined from seaward
		if (sectr1 > 180)
			sectr1 -= 180;
		else
			sectr1 += 180;

		if (sectr2 > 180)
			sectr2 -= 180;
		else
			sectr2 += 180;

		char arc_data[80];
		sprintf(arc_data, ",%5.1f, %5.1f, %5.1f, %5.1f", sectr1, sectr2, arc_radius, sector_radius);
		strcat(sym, arc_data);
		lights06 = sym;

		goto l06_end;
	}


l06_end:

	{
		// Only show Light in certain position once. Otherwise there will be clutter.
		//static double lastLat, lastLon;
		//if (lastLat == obj->m_lat && lastLon == obj->m_lon) isFirstSector = false;
		//lastLat = obj->m_lat;
		//lastLon = obj->m_lon;

		string lastDescription;
		bool isFirstSector = true;
		static unsigned short lastPoArray;
		if (lastPoArray == m_pRecFeature->m_Fspt.m_vecFspt[0].idx) 
			isFirstSector = false;
		lastPoArray = m_pRecFeature->m_Fspt.m_vecFspt[0].idx;

		string litdsn01 = _LITDSN01();

		if (litdsn01.length() && isFirstSector)
		{
			lastDescription = litdsn01;
			lights06.append(";TX('");
			lights06.append(litdsn01);

			if (flare_at_45)
				lights06.append("',3,3,3,'15110',2,-1,CHBLK,23)");
			else
				lights06.append("',3,2,3,'15110',2,0,CHBLK,23)");
		}

		if (!isFirstSector && lastDescription != litdsn01) {
			lastDescription = litdsn01;
			lights06.append(";TX('");
			lights06.append(litdsn01);
			lights06.append("',3,2,3,'15110',2,1,CHBLK,23)");
		}
	}

	lights06.append("\037");

	return lights06;
}
int CS57SymbolRules::_parseList(const char *str_in, char *buf, int buf_size)
{
	char *str = (char *)str_in;
	int i = 0;

	if (NULL != str && *str != '\0')
	{
		do {
			if (i >= LISTSIZE - 1) {
				printf("OVERFLOW --value in list lost!!\n");
				break;
			}

			buf[i++] = (unsigned char)atoi(str);

			while (isdigit(*str))
				str++;   // next

		} while (*str++ != '\0');      // skip ',' or exit
	}

	buf[i] = '\0';

	return i;
}
string CS57SymbolRules::_LITDSN01()
{
	char colist[20];
	string return_value;
	int litchr = -9;
	char grp_str[20] = { '\0' };
	string spost("");
	double sectrTest;
	bool hasSectors = false;
	double   sigper = UNKNOWN;
	double   height = UNKNOWN;
	double   valnmr = UNKNOWN;

	for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size();i++)
	{
		switch (m_pRecFeature->m_Attf.m_vecAttf[i].attl)
		{
		case 95:
			height = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toFloat();
			break;
		case 107:
			litchr = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toInt();
			break;
		case 136:
			hasSectors = true;
			break;
		case 141:
			strcpy(grp_str, (const char*)m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toLocal8Bit());
			break;
		case 142:
			sigper = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toDouble();
			break;
		case 178:
			valnmr = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toFloat();
			break;
		default:
			break;
		}
	}

	bool b_grp2 = false;                      // 2 GRP attributes expected
	if (-9 != litchr)
	{
		switch (litchr)
		{
		case 1: return_value.append("F");    break;                   //fixed     IP 10.1;
		case 2: return_value.append("Fl");   break;                   //flashing  IP 10.4;
		case 3: return_value.append("LFl");  break;                   //long-flashing   IP 10.5;
		case 4: return_value.append("Q");    break;                   //quick-flashing  IP 10.6;
		case 5: return_value.append("VQ");   break;                   //very quick-flashing   IP 10.7;
		case 6: return_value.append("UQ");   break;                   //ultra quick-flashing  IP 10.8;
		case 7: return_value.append("Iso");  break;                   //isophased IP 10.3;
		case 8: return_value.append("Occ");  break;                   //occulting IP 10.2;
		case 9: return_value.append("IQ");   break;                   //interrupted quick-flashing  IP 10.6;
		case 10: return_value.append("IVQ"); break;                   //interrupted very quick-flashing   IP 10.7;
		case 11: return_value.append("IUQ"); break;                   //interrupted ultra quick-flashing  IP 10.8;
		case 12: return_value.append("Mo");  break;                   //morse     IP 10.9;
		case 13: return_value.append("F + Fl");   b_grp2 = true; break;                   //fixed/flash     IP 10.10;
		case 14: return_value.append("Fl + LFl"); b_grp2 = true; break;                   //flash/long-flash
		case 15: return_value.append("Occ + Fl"); b_grp2 = true; break;                   //occulting/flash
		case 16: return_value.append("F + LFl");  b_grp2 = true;  break;                   //fixed/long-flash
		case 17: return_value.append("Al Occ");    break;                   //occulting alternating
		case 18: return_value.append("Al LFl");    break;                   //long-flash alternating
		case 19: return_value.append("Al Fl");    break;                   //flash alternating
		case 20: return_value.append("Al Grp");    break;                   //group alternating
		case 21: return_value.append("F"); spost = " (vert)";    break;                   //2 fixed (vertical)
		case 22: return_value.append("F"); spost = " (horz)";    break;                   //2 fixed (horizontal)
		case 23: return_value.append("F"); spost = " (vert)";    break;                   //3 fixed (vertical)
		case 24: return_value.append("F"); spost = " (horz)";    break;                   //3 fixed (horizontal)
		case 25: return_value.append("Q + LFl");  b_grp2 = true;    break;                   //quick-flash plus long-flash
		case 26: return_value.append("VQ + LFl"); b_grp2 = true;    break;                   //very quick-flash plus long-flash
		case 27: return_value.append("UQ + LFl"); b_grp2 = true;    break;                   //ultra quick-flash plus long-flash
		case 28: return_value.append("Alt");                        break;                   //alternating
		case 29: return_value.append("F + Alt"); b_grp2 = true;     break;                   //fixed and alternating flashing

		default: break;
		}
	}

	int nfirst_grp = -1;
	if (b_grp2)
	{
		string ret_new;
		nfirst_grp = return_value.find(" ");
		if (-1 != nfirst_grp)
		{
			ret_new = return_value.substr(0, nfirst_grp);
			ret_new.append("(?)");
			ret_new.append(return_value.substr(nfirst_grp + 1, strlen(return_value.c_str()) - (nfirst_grp + 1)));
			return_value = ret_new;
			nfirst_grp += 1;
		}
	}

	// SIGGRP, (c)(c) ...
	if (strlen(grp_str))
	{
		//string ss(grp_str, wxConvUTF8);
		string ss = grp_str;

		if (b_grp2)
		{
			vector<string> vecString;
			QString qss = QString::fromStdString(ss);
			QStringList values = qss.split("()");
			for (int i = 0; i < values.size(); i++)
			{
				 vecString.push_back(string((const char*)values[i].toLocal8Bit()));
			}
			int n_tok = 0;
			for (int i = 0; i < vecString.size() && n_tok < 2; i++)
			{
				string s = vecString[i];
				if (s.length())
				{
					if ((n_tok == 0) && (nfirst_grp > 0))
					{
						return_value[nfirst_grp] = s[0];
					}
					else
					{
						if (s != "1")
						{
							return_value.append("(");
							return_value.append(s);
							return_value.append(")");
						}
					}
					n_tok++;
				}
			}
		}
		else
		{
			if (ss != "(1)")
				return_value.append(ss);
		}
	}

	char col_str[20] = { '\0' };

	if (!hasSectors)
	{
		for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size();i++)
		{
			if (m_pRecFeature->m_Attf.m_vecAttf[i].attl == 75)
			{
				strcpy(col_str, (const char*)m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toLocal8Bit());
			}
		}

		int n_cols = 0;
		if (strlen(col_str))
			n_cols = _parseList(col_str, colist, sizeof(colist));

		if (n_cols)
			return_value.append(" ");

		for (int i = 0; i < n_cols; i++)
		{
			switch (colist[i])
			{
			case 1:  return_value.append("W"); break;
			case 3:  return_value.append("R"); break;
			case 4:  return_value.append("G"); break;
			case 6:  return_value.append("Y"); break;
			default:  break;
			}
		}
	}

	// SIGPER, xx.xx
	if (UNKNOWN != sigper)
	{
		string s;
		if (fabs(round(sigper) - sigper) > 0.01)
		{
			char cS[255];
			sprintf(cS, "%4.1fs", sigper);
			s = cS;
		}
		else
		{
			char cS[255];
			sprintf(cS, "%2.0fs", sigper);
			s = cS;
		}

		//s.Trim(false);          // remove leading spaces
		string str = " ";
		s = str.append(s);
		return_value.append(s);
	}


	// HEIGHT, xxx.x
	if (UNKNOWN != height)
	{
		string s;

		char cS[255];
		sprintf(cS, "%3.0fm", height);
		s = cS;

		//s.Trim(false);          // remove leading spaces
		string str = " ";
		s = str.append(s);
		return_value.append(s);
	}


	// VALNMR, xx.x
	if (UNKNOWN != valnmr && !hasSectors)
	{
		string s;
		char cS[255];
		sprintf(cS, "%2.0fM", valnmr);
		s = cS;
		//s.Trim(false);          // remove leading spaces
		string str = " ";
		s = str.append(s);
		return_value.append(s);
	}
	return_value.append(spost);                     // add any final modifiers

	return return_value;
}
string CS57SymbolRules::_selSYcol(char *buf, bool bsectr, double valnmr)
{
	string sym;
	if (!bsectr)
	{
		sym = ";SY(LITDEF11";                 // default

		if ('\0' == buf[1])
		{
			if (strpbrk(buf, "\003"))
				sym = ";SY(LIGHTS11";
			else if (strpbrk(buf, "\004"))
				sym = ";SY(LIGHTS12";
			else if (strpbrk(buf, "\001\006\011"))
				sym = ";SY(LIGHTS13";
		}
		else
		{
			// max 2 color
			if ('\0' == buf[2])
			{
				if (strpbrk(buf, "\001") && strpbrk(buf, "\003"))
					sym = ";SY(LIGHTS11";
				else if (strpbrk(buf, "\001") && strpbrk(buf, "\004"))
					sym = ";SY(LIGHTS12";
			}
		}
	}
	else
	{
		int radius = 3;
		if (valnmr > 0)
		{
			if (valnmr < 7.0)
				radius = 3;
			else if (valnmr < 15.0)
				radius = 10;
			else if (valnmr < 30.0)
				radius = 15;
			else
				radius = 20;
		}

		// max 1 color
		char cSym[255];
		if ('\0' == buf[1])
		{
			if (strpbrk(buf, "\003"))
			{
				sprintf(cSym, ",LITRD, 2,0,360,%d,0", radius + 1);
			}
			else if (strpbrk(buf, "\004"))
			{
				sprintf(cSym, ",LITGN, 2,0,360,%d,0", radius);
			}
			else if (strpbrk(buf, "\001\006\011"))
			{
				sprintf(cSym, ",LITYW, 2,0,360,%d,0", radius + 2);
			}
			else if (strpbrk(buf, "\014"))
			{
				sprintf(cSym, ",CHMGD, 2,0,360,%d,0", radius + 3);
			}
			else
			{
				sprintf(cSym, ",CHMGD, 2,0,360,%d,0", radius + 5);
			}
		}
		else  if ('\0' == buf[2])       // or 2 color
		{
			if (strpbrk(buf, "\001") && strpbrk(buf, "\003"))
			{
				sprintf(cSym, ",LITRD, 2,0,360,%d,0", radius + 1);
			}
			else if (strpbrk(buf, "\001") && strpbrk(buf, "\004"))
			{
				sprintf(cSym, ",LITGN, 2,0,360,%d,0", radius);
			}
			else
			{
				sprintf(cSym, ",CHMGD, 2,0,360,%d,0", radius + 5);
			}
		}
		else
			sprintf(cSym, ",CHMGD, 2,0,360,%d,0", radius + 5);

		sym = cSym;
		if (sym.length())
		{
			string str = ";CA(OUTLW, 4";
			sym = str.append(sym);
		}
	}

	return sym;
}

string CS57SymbolRules::getShowCommand(char cs[9])
{
	string str = "";
	//灯标
	if (strcmp(cs, "LIGHTS06") == 0)
	{
		str = LIGHTS06();
	}
	//顶标
	else if (strcmp(cs, "TOPMAR01") == 0)
	{
		str = TOPMAR01();
	}
	//沉船
	else if (strcmp(cs, "WRECKS05") == 0)
	{
		str = WRECKS02();
	}
	//障碍物
	else if (strcmp(cs, "OBSTRN07") == 0)
	{
		str = OBSTRN07();
	}
	//未知
	else if (strcmp(cs, "QUALIN01") == 0)
	{
		str = QUALIN01();
	}
	//未知
	else if (strcmp(cs, "QUAPNT02") == 0)
	{
		str = QUAPNT02();
	}
	//未知
	else if (strcmp(cs, "SLCONS04") == 0)
	{
		str = SLCONS04();
	}
	//未知
	else if (strcmp(cs, "DATCVR02") == 0)
	{
		str = DATCVR02();
	}

	//未知物标
	else if (strcmp(cs, "QUAPOS01") == 0)
	{
		str = QUAPOS01();
	}
	//限制区
	else if (strcmp(cs, "RESARE04") == 0)
	{
		str = RESARE04();
	}

	//深度区
	else if (strcmp(cs, "DEPARE03") == 0)
	{
		str = DEPARE03();
	}
	return str;
}

//顶标
string CS57SymbolRules::TOPMAR01()
{
	bool offset = false;
	int top_int = 0;
	bool battr = false;
	for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size();i++)
	{
		if (m_pRecFeature->m_Attf.m_vecAttf[i].attl == 171)
		{
			top_int = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toInt();
			battr = true;
		}
	}
	string sy;
	if (!battr)
		sy = ";SY(QUESMRK1)";
	else {
		int floating = FALSE; // not a floating platform
		int topshp = (!battr) ? 0 : top_int;


		if (m_pRecFeature->getParentCell()->getFloating(m_pRecFeature))
			floating = TRUE;
		else
			floating = FALSE;

		if (floating) {
			// floating platform
			switch (topshp) {
			case 1: sy = ";SY(TOPMAR02)"; break;
			case 2: sy = ";SY(TOPMAR04)"; break;
			case 3: sy = ";SY(TOPMAR10)"; break;
			case 4: sy = ";SY(TOPMAR12)"; break;

			case 5: sy = ";SY(TOPMAR13)"; break;
			case 6: sy = ";SY(TOPMAR14)"; break;
			case 7: sy = ";SY(TOPMAR65)"; break;
			case 8: sy = ";SY(TOPMAR17)"; break;

			case 9: sy = ";SY(TOPMAR16)"; break;
			case 10: sy = ";SY(TOPMAR08)"; break;
			case 11: sy = ";SY(TOPMAR07)"; break;
			case 12: sy = ";SY(TOPMAR14)"; break;

			case 13: sy = ";SY(TOPMAR05)"; break;
			case 14: sy = ";SY(TOPMAR06)"; break;
			case 17: sy = ";SY(TMARDEF2)"; break;
			case 18: sy = ";SY(TOPMAR10)"; break;

			case 19: sy = ";SY(TOPMAR13)"; break;
			case 20: sy = ";SY(TOPMAR14)"; break;
			case 21: sy = ";SY(TOPMAR13)"; break;
			case 22: sy = ";SY(TOPMAR14)"; break;

			case 23: sy = ";SY(TOPMAR14)"; break;
			case 24: sy = ";SY(TOPMAR02)"; break;
			case 25: sy = ";SY(TOPMAR04)"; break;
			case 26: sy = ";SY(TOPMAR10)"; break;

			case 27: sy = ";SY(TOPMAR17)"; break;
			case 28: sy = ";SY(TOPMAR18)"; break;
			case 29: sy = ";SY(TOPMAR02)"; break;
			case 30: sy = ";SY(TOPMAR17)"; break;

			case 31: sy = ";SY(TOPMAR14)"; break;
			case 32: sy = ";SY(TOPMAR10)"; break;
			case 33: sy = ";SY(TMARDEF2)"; break;
			default: sy = ";SY(TMARDEF2)"; break;
			}
		}
		else {
			// not a floating platform
			switch (topshp) {
			case 1: sy = ";SY(TOPMAR22)"; break;
			case 2: sy = ";SY(TOPMAR24)"; break;
			case 3: sy = ";SY(TOPMAR30)"; break;
			case 4: sy = ";SY(TOPMAR32)"; break;

			case 5: sy = ";SY(TOPMAR33)"; break;
			case 6: sy = ";SY(TOPMAR34)"; break;
			case 7: sy = ";SY(TOPMAR85)"; break;
			case 8: sy = ";SY(TOPMAR86)"; break;

			case 9: sy = ";SY(TOPMAR36)"; break;
			case 10: sy = ";SY(TOPMAR28)"; break;
			case 11: sy = ";SY(TOPMAR27)"; break;
			case 12: sy = ";SY(TOPMAR14)"; break;

			case 13: sy = ";SY(TOPMAR25)"; break;
			case 14: sy = ";SY(TOPMAR26)"; break;
			case 15: sy = ";SY(TOPMAR88)"; break;
			case 16: sy = ";SY(TOPMAR87)"; break;

			case 17: sy = ";SY(TMARDEF1)"; break;
			case 18: sy = ";SY(TOPMAR30)"; break;
			case 19: sy = ";SY(TOPMAR33)"; break;
			case 20: sy = ";SY(TOPMAR34)"; break;

			case 21: sy = ";SY(TOPMAR33)"; break;
			case 22: sy = ";SY(TOPMAR34)"; break;
			case 23: sy = ";SY(TOPMAR34)"; break;
			case 24: sy = ";SY(TOPMAR22)"; break;

			case 25: sy = ";SY(TOPMAR24)"; break;
			case 26: sy = ";SY(TOPMAR30)"; break;
			case 27: sy = ";SY(TOPMAR86)"; break;
			case 28: sy = ";SY(TOPMAR89)"; break;

			case 29: sy = ";SY(TOPMAR22)"; break;
			case 30: sy = ";SY(TOPMAR86)"; break;
			case 31: sy = ";SY(TOPMAR14)"; break;
			case 32: sy = ";SY(TOPMAR30)"; break;
			case 33: sy = ";SY(TMARDEF1)"; break;
			default: sy = ";SY(TMARDEF1)"; break;
			}
		}

	}

	string topmar;
	topmar.append(sy);
	if (offset)
		topmar.append("offset");
	topmar.append("\037");

	return topmar;
}

//水深
string CS57SymbolRules::SOUNDG03()
{
	double z = (double)m_pRecFeature->getParentCell()->m_vecS57RecVI[m_pRecFeature->m_Fspt.m_vecFspt[0].idx]->m_Sg3d.m_vecSg3d[0].ve3d
		/ (double)m_pRecFeature->getParentCell()->m_pRecDsgr->m_Dspm.somf;
	string s = SNDFRM02(z);
	return s;
}
string CS57SymbolRules::SNDFRM02(double depth_value_in)
{
	string sndfrm02;
	char temp_str[LISTSIZE] = { '\0' };
	char tecsou[LISTSIZE] = { '\0' };
	char quasou[LISTSIZE] = { '\0' };
	char status[LISTSIZE] = { '\0' };
	string symbol_prefix;
	char symbol_prefix_a[200];
	string tecsoustr;
	string quasoustr;
	string statusstr;
	double leading_digit = 0.0;

	for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size();i++)
	{
		switch (m_pRecFeature->m_Attf.m_vecAttf[i].attl)
		{
		case 125:
		{
			char quasouchar[20] = { '\0' };
			strcpy(quasouchar, (const char*)m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toLocal8Bit());
			quasoustr = quasouchar;
			break;
		}
		case 149:
		{
			char statuschar[20] = { '\0' };
			strcpy(statuschar, (const char*)m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toLocal8Bit());
			statusstr = statuschar;
			break;
		}
		case 156:
		{
			char tecsouchar[20] = { '\0' };
			strcpy(tecsouchar, (const char*)m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toLocal8Bit());
			tecsoustr = tecsouchar;
			break;
		}
		default:
			break;
		}

	}
	double safety_depth = S52_getMarinerParam(S52_MAR_SAFETY_DEPTH);
	double depth_value = depth_value_in;

	if (depth_value_in > 40000.)
		depth_value = 99999.;
	if (depth_value_in < -1000.)
		depth_value = 0.;

	// FIXME: test to fix the rounding error (!?)
	depth_value += (depth_value > 0.0) ? 0.01 : -0.01;
	leading_digit = (int)fabs(depth_value);

	if (depth_value <= safety_depth)            //S52_getMarinerParam(S52_MAR_SAFETY_DEPTH)
		symbol_prefix = "SOUNDS";
	else
		symbol_prefix = "SOUNDG";

	strcpy(symbol_prefix_a, symbol_prefix.c_str());

	if (tecsoustr != "")
	{
		_parseList(tecsoustr.c_str(), tecsou, sizeof(tecsou));
		if (strpbrk(tecsou, "\006"))
		{
			snprintf(temp_str, LISTSIZE, ";SY(%sB1)", symbol_prefix_a);
			sndfrm02.append(temp_str);
		}
	}

	if (quasoustr != "") _parseList(quasoustr.c_str(), quasou, sizeof(quasou));
	if (statusstr != "") _parseList(statusstr.c_str(), status, sizeof(status));

	if (strpbrk(quasou, "\003\004\005\010\011") || strpbrk(status, "\022"))
	{
		snprintf(temp_str, LISTSIZE, ";SY(%sC2)", symbol_prefix_a);
		sndfrm02.append(temp_str);
	}
	else
	{
		int quapos = 0;
		for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size();i++)
		{
			if (m_pRecFeature->m_Attf.m_vecAttf[i].attl == 402)
			{
				quapos = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toInt();
			}
		}
		if (0 != quapos)
		{
			if (2 <= quapos && quapos < 10)
			{
				snprintf(temp_str, LISTSIZE, ";SY(%sC2)", symbol_prefix_a);
				sndfrm02.append(temp_str);
			}
		}
	}

	// Continuation A
	if (fabs(depth_value) < 10.0) {

		//      If showing as "feet", round off to one digit only
		if (depth_value > 0)
		{
			double r1 = depth_value;
			//depth_value = round(r1);
			leading_digit = (int)depth_value;
		}

		if (depth_value < 10.0) {
			// can be above water (negative)
			int fraction = (int)ABS((fabs(depth_value) - leading_digit) * 10);


			snprintf(temp_str, LISTSIZE, ";SY(%s1%1i)", symbol_prefix_a, (int)ABS(leading_digit));
			sndfrm02.append(temp_str);
			if (fraction > 0)
			{
				snprintf(temp_str, LISTSIZE, ";SY(%s5%1i)", symbol_prefix_a, fraction);
				sndfrm02.append(temp_str);
			}

			// above sea level (negative)
			if (depth_value < 0.0)
			{
				snprintf(temp_str, LISTSIZE, ";SY(%sA1)", symbol_prefix_a);
				sndfrm02.append(temp_str);
			}
			goto return_point;
		}
	}

	if (fabs(depth_value) < 31.0)
	{
		bool b_2digit = false;
		double depth_value_pos = fabs(depth_value);

		//      If showing as "feet", round off to two digits only
		if (depth_value_pos > 0)
		{
			double r1 = depth_value;
			depth_value = round(r1);
			leading_digit = (int)depth_value_pos;
			b_2digit = true;
		}


		double fraction = fabs(depth_value_pos - floor(leading_digit));

		if (fraction != 0.0)
		{
			fraction = fraction * 10;
			if (leading_digit >= 10.0)
			{
				snprintf(temp_str, LISTSIZE, ";SY(%s2%1i)", symbol_prefix_a, (int)leading_digit / 10);
				sndfrm02.append(temp_str);
			}

			double first_digit = floor(leading_digit / 10);
			int secnd_digit = (int)(floor(leading_digit - (first_digit * 10)));
			snprintf(temp_str, LISTSIZE, ";SY(%s1%1i)", symbol_prefix_a, secnd_digit/*(int)leading_digit*/);
			sndfrm02.append(temp_str);

			if (!b_2digit)
			{
				if ((int)fraction > 0)
				{
					snprintf(temp_str, LISTSIZE, ";SY(%s5%1i)", symbol_prefix_a, (int)fraction);
					sndfrm02.append(temp_str);
				}
			}

			if (depth_value < 0.0)
			{
				snprintf(temp_str, LISTSIZE, ";SY(%sA1)", symbol_prefix_a);
				sndfrm02.append(temp_str);
			}

			goto return_point;
		}
	}

	// Continuation B
	if (fabs(depth_value) < 100.0)
	{
		leading_digit = fabs(leading_digit);

		double first_digit = floor(leading_digit / 10);
		double secnd_digit = floor(leading_digit - (first_digit * 10));

		if (depth_value < 0.0)
		{
			snprintf(temp_str, LISTSIZE, ";SY(%s2%1i)",
				symbol_prefix_a, (int)first_digit);
			sndfrm02.append(temp_str);
			snprintf(temp_str, LISTSIZE, ";SY(%s1%1i)",
				symbol_prefix_a, (int)secnd_digit);
			sndfrm02.append(temp_str);
			snprintf(temp_str, LISTSIZE, ";SY(%sA1)", symbol_prefix_a);
			sndfrm02.append(temp_str);
		}
		else
		{
			snprintf(temp_str, LISTSIZE, ";SY(%s1%1i)",
				symbol_prefix_a, (int)first_digit);
			sndfrm02.append(temp_str);
			snprintf(temp_str, LISTSIZE, ";SY(%s0%1i)",
				symbol_prefix_a, (int)secnd_digit);
			sndfrm02.append(temp_str);
		}
		goto return_point;
	}

	if (depth_value < 1000.0)
	{
		double first_digit = floor(leading_digit / 100);
		double secnd_digit = floor((leading_digit - (first_digit * 100)) / 10);
		double third_digit = floor(leading_digit - (first_digit * 100) - (secnd_digit * 10));

		snprintf(temp_str, LISTSIZE, ";SY(%s2%1i)", symbol_prefix_a, (int)first_digit);
		sndfrm02.append(temp_str);
		snprintf(temp_str, LISTSIZE, ";SY(%s1%1i)", symbol_prefix_a, (int)secnd_digit);
		sndfrm02.append(temp_str);
		snprintf(temp_str, LISTSIZE, ";SY(%s0%1i)", symbol_prefix_a, (int)third_digit);
		sndfrm02.append(temp_str);

		goto return_point;
	}

	if (depth_value < 10000.0)
	{
		double first_digit = floor(leading_digit / 1000);
		double secnd_digit = floor((leading_digit - (first_digit * 1000)) / 100);
		double third_digit = floor((leading_digit - (first_digit * 1000) - (secnd_digit * 100)) / 10);
		double last_digit = floor(leading_digit - (first_digit * 1000) - (secnd_digit * 100) - (third_digit * 10));

		snprintf(temp_str, LISTSIZE, ";SY(%s2%1i)", symbol_prefix_a, (int)first_digit);
		sndfrm02.append(temp_str);
		snprintf(temp_str, LISTSIZE, ";SY(%s1%1i)",
			symbol_prefix_a, (int)secnd_digit);
		sndfrm02.append(temp_str);
		snprintf(temp_str, LISTSIZE, ";SY(%s0%1i)",
			symbol_prefix_a, (int)third_digit);
		sndfrm02.append(temp_str);
		snprintf(temp_str, LISTSIZE, ";SY(%s4%1i)",
			symbol_prefix_a, (int)last_digit);
		sndfrm02.append(temp_str);

		goto return_point;
	}

	// Continuation C
	{
		double first_digit = floor(leading_digit / 10000);
		double secnd_digit = floor((leading_digit - (first_digit * 10000)) / 1000);
		double third_digit = floor((leading_digit - (first_digit * 10000) - (secnd_digit * 1000)) / 100);
		double fourth_digit = floor((leading_digit - (first_digit * 10000) - (secnd_digit * 1000) - (third_digit * 100)) / 10);
		double last_digit = floor(leading_digit - (first_digit * 10000) - (secnd_digit * 1000) - (third_digit * 100) - (fourth_digit * 10));

		snprintf(temp_str, LISTSIZE, ";SY(%s3%1i)",
			symbol_prefix_a, (int)first_digit);
		sndfrm02.append(temp_str);
		snprintf(temp_str, LISTSIZE, ";SY(%s2%1i)",
			symbol_prefix_a, (int)secnd_digit);
		sndfrm02.append(temp_str);
		snprintf(temp_str, LISTSIZE, ";SY(%s1%1i)",
			symbol_prefix_a, (int)third_digit);
		sndfrm02.append(temp_str);
		snprintf(temp_str, LISTSIZE, ";SY(%s0%1i)",
			symbol_prefix_a, (int)fourth_digit);
		sndfrm02.append(temp_str);
		snprintf(temp_str, LISTSIZE, ";SY(%s4%1i)",
			symbol_prefix_a, (int)last_digit);
		sndfrm02.append(temp_str);

		goto return_point;
	}

return_point:
	sndfrm02.append("\037");

	return sndfrm02;
}

//障碍物
string CS57SymbolRules::OBSTRN07()
{
	string obstrn04str;
	string udwhaz03str;
	bool b_promote = false;

	double   valsou = UNKNOWN;
	double   depth_value = UNKNOWN;
	double   least_depth = UNKNOWN;

	string sndfrm02str;
	string quapnt01str;

	for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size();i++)
	{
		if (m_pRecFeature->m_Attf.m_vecAttf[i].attl == 179 &&
			m_pRecFeature->m_Attf.m_vecAttf[i].atvl != "")
		{
			valsou = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toFloat();
		}
	}

	if (valsou != UNKNOWN)
	{
		depth_value = valsou;
		sndfrm02str = SNDFRM02(valsou);
	}
	else
	{
		if (m_pRecFeature->m_Frid.prim == 3)
			least_depth = _DEPVAL01(least_depth);

		if (UNKNOWN == least_depth)
		{
			int catobs = 0;
			int watlev = 0;
			int expsou = 0;
			for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size();i++)
			{
				switch (m_pRecFeature->m_Attf.m_vecAttf[i].attl)
				{
				case 42:
					catobs = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toInt();
					break;
				case 93:
					expsou = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toInt();
					break;
				case 187:
					watlev = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toInt();
					break;
				default:
					break;
				}
			}
			if (expsou != 1) {
				if (6 == catobs)
					depth_value = 0.01;
				else if (0 == watlev) // default
					depth_value = -15.0;
				else
				{
					switch (watlev) {
					case 5: depth_value = 0.0; break;
					case 3: depth_value = 0.01; break;
					case 4:
					case 1:
					case 2:
					default: depth_value = -15.0; break;
					}
				}
			}
		}
		else
			depth_value = least_depth;
	}

	udwhaz03str = _UDWHAZ03(depth_value, &b_promote);


	if (m_pRecFeature->m_Frid.prim == 1)
	{
		// Continuation A
		int  sounding = FALSE;
		quapnt01str = CSQUAPNT01();

		if (0 != udwhaz03str.length())
		{
			obstrn04str.append(udwhaz03str);
			obstrn04str.append(quapnt01str);

			goto end;
		}

		if (UNKNOWN != valsou)
		{
			if (valsou <= 20.0)
			{
				int watlev = -9;
				for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size();i++)
				{
					if (m_pRecFeature->m_Attf.m_vecAttf[i].attl == 187)
					{
						watlev = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toInt();
					}
				}
				if (m_pRecFeature->m_Frid.objl == 153)
				{
					if (-9 == watlev)
					{  // default
						obstrn04str.append(";SY(DANGER02)");
						sounding = TRUE;
					}
					else {
						switch (watlev) {
						case 3: obstrn04str.append(";SY(DANGER02)"); sounding = TRUE; break;
						case 4:
						case 5: obstrn04str.append(";SY(UWTROC04)"); sounding = FALSE; break;
						default: obstrn04str.append(";SY(DANGER02)"); sounding = TRUE; break;
						}
					}
					if (b_promote) {
						//  Move this UWTROC object to DisplayBase category
						//rzRules->obj->m_DisplayCat = DISPLAYBASE;
					}
				}
				else
				{ // OBSTRN
					if (-9 == watlev) { // default
						obstrn04str.append(";SY(DANGER01)");
						sounding = TRUE;
					}
					else {
						switch (watlev) {
						case 1:
						case 2: obstrn04str.append(";SY(LNDARE01)"); sounding = FALSE; break;
						case 3: obstrn04str.append(";SY(DANGER02)"); sounding = TRUE;  break;
						case 4:
						case 5: obstrn04str.append(";SY(DANGER53)"); sounding = TRUE; break;
						default: obstrn04str.append(";SY(DANGER02)"); sounding = TRUE; break;
						}
					}
				}
			}
			else
			{  // valsou > 20.0
				obstrn04str.append(";SY(DANGER02)");
				sounding = TRUE;
			}
		}
		else
		{  // NO valsou
			int watlev = -9;
			for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size();i++)
			{
				if (m_pRecFeature->m_Attf.m_vecAttf[i].attl == 187)
				{
					watlev = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toInt();
				}
			}

			if (m_pRecFeature->m_Frid.objl == 153)
			{
				if (watlev == -9)  // default
					obstrn04str.append(";SY(UWTROC04)");
				else {
					switch (watlev) {
					case 2: obstrn04str.append(";SY(LNDARE01)"); break;
					case 3: obstrn04str.append(";SY(UWTROC03)"); break;
					default: obstrn04str.append(";SY(UWTROC04)"); break;
					}
				}
			}
			else
			{ // OBSTRN
				if (-9 == watlev) // default
					obstrn04str = ";SY(OBSTRN01)";
				else
				{
					switch (watlev) {
					case 1: obstrn04str.append(";SY(OBSTRN11)"); break;
					case 2: obstrn04str.append(";SY(OBSTRN11)"); break;
					case 3: obstrn04str.append(";SY(OBSTRN01)"); break;
					case 4: obstrn04str.append(";SY(OBSTRN03)"); break;
					case 5: obstrn04str.append(";SY(OBSTRN03)"); break;
					default: obstrn04str.append(";SY(OBSTRN01)"); break;
					}
				}
			}
		}

		if (sounding)
			obstrn04str.append(sndfrm02str);

		obstrn04str.append(quapnt01str);

		goto end;

	}     // if geopoint
	else
	{
		if (m_pRecFeature->m_Frid.prim == 2)
		{
			// Continuation B

			quapnt01str = CSQUAPNT01();

			if (quapnt01str.length() > 1) {
				long quapos;
				quapos = atoi(quapnt01str.c_str());
				if (2 <= quapos && quapos < 10) {
					if (udwhaz03str.length())
						obstrn04str.append(";LC(LOWACC41)");
					else
						obstrn04str.append(";LC(LOWACC31)");
				}
				goto end;
			}

			if (udwhaz03str.length())
			{
				obstrn04str.append("LS(DOTT,2,CHBLK)");
				goto end;
			}

			if (UNKNOWN != valsou) {
				if (valsou <= 20.0)
					obstrn04str.append(";LS(DOTT,2,CHBLK)");
				else
					obstrn04str.append(";LS(DASH,2,CHBLK)");
			}
			else
				obstrn04str.append(";LS(DOTT,2,CHBLK)");


			if (udwhaz03str.length()) {
				//  Show the isolated danger symbol at the midpoint of the line
			}
			else {
				if (UNKNOWN != valsou)
					if (valsou <= 20.0)
						obstrn04str.append(sndfrm02str);
			}
		}

		else                // Area feature
		{
			quapnt01str = CSQUAPNT01();

			if (0 != udwhaz03str.length())
			{
				obstrn04str.append(";AC(DEPVS);AP(FOULAR01)");
				obstrn04str.append(";LS(DOTT,2,CHBLK)");
				obstrn04str.append(udwhaz03str);
				obstrn04str.append(quapnt01str);

				goto end;
			}

			if (UNKNOWN != valsou) {
				// BUG in CA49995B.000 if we get here because there is no color
				// beside NODATA (ie there is a hole in group 1 area!)
				//g_string_append(obstrn04, ";AC(UINFR)");

				if (valsou <= 20.0)
					obstrn04str.append(";LS(DOTT,2,CHBLK)");
				else
					obstrn04str.append(";LS(DASH,2,CHBLK)");

				obstrn04str.append(sndfrm02str);

			}
			else {
				int watlev = -9;
				for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size();i++)
				{
					switch (m_pRecFeature->m_Attf.m_vecAttf[i].attl)
					{
					case 187:
						watlev = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toInt();
						break;
					default:
						break;
					}
				}
				if (watlev == -9)   // default
					obstrn04str.append(";AC(DEPVS);LS(DOTT,2,CHBLK)");
				else {
					if (3 == watlev) {
						int catobs = -9;
						for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size();i++)
						{
							if (m_pRecFeature->m_Attf.m_vecAttf[i].attl == 42)
							{
								catobs = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toInt();
							}
						}
						if (6 == catobs)
							obstrn04str.append(";AC(DEPVS);AP(FOULAR01);LS(DOTT,2,CHBLK)");
					}
					else {
						switch (watlev) {
						case 1:
						case 2: obstrn04str.append(";AC(CHBRN);LS(SOLD,2,CSTLN)"); break;
						case 4: obstrn04str.append(";AC(DEPIT);LS(DASH,2,CSTLN)"); break;
						case 5:
						case 3:
						default: obstrn04str.append(";AC(DEPVS);LS(DOTT,2,CHBLK)");  break;
						}
					}
				}
			}

			obstrn04str.append(quapnt01str);
			goto end;
		}     // area
	}

end:
	obstrn04str.append("\037");

	return obstrn04str;
}

string CS57SymbolRules::OBSTRN071()
{
	string obstrn04str;
	string udwhaz03str;
	bool b_promote = false;

	double   valsou = UNKNOWN;
	double   depth_value = UNKNOWN;
	float   least_depth = UNKNOWN;
	float   seabed_depth = UNKNOWN;

	int catobs = 0;
	int watlev = 0;
	int expsou = 0;

	string sndfrm02str;
	string quapnt01str;

	for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size(); i++)
	{
		switch (m_pRecFeature->m_Attf.m_vecAttf[i].attl)
		{
		case 179:
			if (m_pRecFeature->m_Attf.m_vecAttf[i].atvl !=  "")
				valsou = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toFloat();
			break;
		case 42:
			catobs = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toInt();
			break;
		case 93:
			expsou = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toInt();
			break;
		case 187:
			watlev = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toInt();
			break;
		default:
			break;
		}
	}

	if (valsou != UNKNOWN)
	{
		depth_value = valsou;
		//sndfrm02str = SNDFRM02(valsou);
	}
	else
	{
		_DEPVAL02(&least_depth, &seabed_depth);
		if (least_depth != UNKNOWN)
		{
			depth_value = least_depth;
		}
		else if (catobs == 6 || watlev == 3)
		{
			depth_value = 0.01;
		}
		else if (watlev == 5)
		{
			depth_value = 0;
		}
		else
		{
			depth_value = -15;
		}

		if (m_pRecFeature->m_Frid.prim == 3)
			least_depth = _DEPVAL01(least_depth);

		if (UNKNOWN == least_depth)
		{
			int catobs = 0;
			int watlev = 0;
			int expsou = 0;
			for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size(); i++)
			{
				switch (m_pRecFeature->m_Attf.m_vecAttf[i].attl)
				{
				case 42:
					catobs = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toInt();
					break;
				case 93:
					expsou = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toInt();
					break;
				case 187:
					watlev = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toInt();
					break;
				default:
					break;
				}
			}
			if (expsou != 1) {
				if (6 == catobs)
					depth_value = 0.01;
				else if (0 == watlev) // default
					depth_value = -15.0;
				else
				{
					switch (watlev) {
					case 5: depth_value = 0.0; break;
					case 3: depth_value = 0.01; break;
					case 4:
					case 1:
					case 2:
					default: depth_value = -15.0; break;
					}
				}
			}
		}
		else
			depth_value = least_depth;
	}

	udwhaz03str = _UDWHAZ03(depth_value, &b_promote);


	if (m_pRecFeature->m_Frid.prim == 1)
	{
		// Continuation A
		int  sounding = FALSE;
		quapnt01str = CSQUAPNT01();

		if (0 != udwhaz03str.length())
		{
			obstrn04str.append(udwhaz03str);
			obstrn04str.append(quapnt01str);

			goto end;
		}

		if (UNKNOWN != valsou)
		{
			if (valsou <= 20.0)
			{
				int watlev = -9;
				for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size(); i++)
				{
					if (m_pRecFeature->m_Attf.m_vecAttf[i].attl == 187)
					{
						watlev = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toInt();
					}
				}
				if (m_pRecFeature->m_Frid.objl == 153)
				{
					if (-9 == watlev)
					{  // default
						obstrn04str.append(";SY(DANGER02)");
						sounding = TRUE;
					}
					else {
						switch (watlev) {
						case 3: obstrn04str.append(";SY(DANGER02)"); sounding = TRUE; break;
						case 4:
						case 5: obstrn04str.append(";SY(UWTROC04)"); sounding = FALSE; break;
						default: obstrn04str.append(";SY(DANGER02)"); sounding = TRUE; break;
						}
					}
					if (b_promote) {
						//  Move this UWTROC object to DisplayBase category
						//rzRules->obj->m_DisplayCat = DISPLAYBASE;
					}
				}
				else
				{ // OBSTRN
					if (-9 == watlev) { // default
						obstrn04str.append(";SY(DANGER01)");
						sounding = TRUE;
					}
					else {
						switch (watlev) {
						case 1:
						case 2: obstrn04str.append(";SY(LNDARE01)"); sounding = FALSE; break;
						case 3: obstrn04str.append(";SY(DANGER02)"); sounding = TRUE;  break;
						case 4:
						case 5: obstrn04str.append(";SY(DANGER53)"); sounding = TRUE; break;
						default: obstrn04str.append(";SY(DANGER02)"); sounding = TRUE; break;
						}
					}
				}
			}
			else
			{  // valsou > 20.0
				obstrn04str.append(";SY(DANGER02)");
				sounding = TRUE;
			}
		}
		else
		{  // NO valsou
			int watlev = -9;
			for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size(); i++)
			{
				if (m_pRecFeature->m_Attf.m_vecAttf[i].attl == 187)
				{
					watlev = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toInt();
				}
			}

			if (m_pRecFeature->m_Frid.objl == 153)
			{
				if (watlev == -9)  // default
					obstrn04str.append(";SY(UWTROC04)");
				else {
					switch (watlev) {
					case 2: obstrn04str.append(";SY(LNDARE01)"); break;
					case 3: obstrn04str.append(";SY(UWTROC03)"); break;
					default: obstrn04str.append(";SY(UWTROC04)"); break;
					}
				}
			}
			else
			{ // OBSTRN
				if (-9 == watlev) // default
					obstrn04str = ";SY(OBSTRN01)";
				else
				{
					switch (watlev) {
					case 1: obstrn04str.append(";SY(OBSTRN11)"); break;
					case 2: obstrn04str.append(";SY(OBSTRN11)"); break;
					case 3: obstrn04str.append(";SY(OBSTRN01)"); break;
					case 4: obstrn04str.append(";SY(OBSTRN03)"); break;
					case 5: obstrn04str.append(";SY(OBSTRN03)"); break;
					default: obstrn04str.append(";SY(OBSTRN01)"); break;
					}
				}
			}
		}

		if (sounding)
			obstrn04str.append(sndfrm02str);

		obstrn04str.append(quapnt01str);

		goto end;

	}     // if geopoint
	else
	{
		if (m_pRecFeature->m_Frid.prim == 2)
		{
			// Continuation B

			quapnt01str = CSQUAPNT01();

			if (quapnt01str.length() > 1) {
				long quapos;
				quapos = atoi(quapnt01str.c_str());
				if (2 <= quapos && quapos < 10) {
					if (udwhaz03str.length())
						obstrn04str.append(";LC(LOWACC41)");
					else
						obstrn04str.append(";LC(LOWACC31)");
				}
				goto end;
			}

			if (udwhaz03str.length())
			{
				obstrn04str.append("LS(DOTT,2,CHBLK)");
				goto end;
			}

			if (UNKNOWN != valsou) {
				if (valsou <= 20.0)
					obstrn04str.append(";LS(DOTT,2,CHBLK)");
				else
					obstrn04str.append(";LS(DASH,2,CHBLK)");
			}
			else
				obstrn04str.append(";LS(DOTT,2,CHBLK)");


			if (udwhaz03str.length()) {
				//  Show the isolated danger symbol at the midpoint of the line
			}
			else {
				if (UNKNOWN != valsou)
					if (valsou <= 20.0)
						obstrn04str.append(sndfrm02str);
			}
		}

		else                // Area feature
		{
			quapnt01str = CSQUAPNT01();

			if (0 != udwhaz03str.length())
			{
				obstrn04str.append(";AC(DEPVS);AP(FOULAR01)");
				obstrn04str.append(";LS(DOTT,2,CHBLK)");
				obstrn04str.append(udwhaz03str);
				obstrn04str.append(quapnt01str);

				goto end;
			}

			if (UNKNOWN != valsou) {
				// BUG in CA49995B.000 if we get here because there is no color
				// beside NODATA (ie there is a hole in group 1 area!)
				//g_string_append(obstrn04, ";AC(UINFR)");

				if (valsou <= 20.0)
					obstrn04str.append(";LS(DOTT,2,CHBLK)");
				else
					obstrn04str.append(";LS(DASH,2,CHBLK)");

				obstrn04str.append(sndfrm02str);

			}
			else {
				int watlev = -9;
				for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size(); i++)
				{
					switch (m_pRecFeature->m_Attf.m_vecAttf[i].attl)
					{
					case 187:
						watlev = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toInt();
						break;
					default:
						break;
					}
				}
				if (watlev == -9)   // default
					obstrn04str.append(";AC(DEPVS);LS(DOTT,2,CHBLK)");
				else {
					if (3 == watlev) {
						int catobs = -9;
						for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size(); i++)
						{
							if (m_pRecFeature->m_Attf.m_vecAttf[i].attl == 42)
							{
								catobs = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toInt();
							}
						}
						//                                    GString *catobsstr = S57_getAttVal(geo, "CATOBS");
						if (6 == catobs)
							obstrn04str.append(";AC(DEPVS);AP(FOULAR01);LS(DOTT,2,CHBLK)");
					}
					else {
						switch (watlev) {
						case 1:
						case 2: obstrn04str.append(";AC(CHBRN);LS(SOLD,2,CSTLN)"); break;
						case 4: obstrn04str.append(";AC(DEPIT);LS(DASH,2,CSTLN)"); break;
						case 5:
						case 3:
						default: obstrn04str.append(";AC(DEPVS);LS(DOTT,2,CHBLK)");  break;
						}
					}
				}
			}

			obstrn04str.append(quapnt01str);
			goto end;
		}     // area
	}

end:
	obstrn04str.append("\037");

	return obstrn04str;
}

string CS57SymbolRules::_UDWHAZ03(double depth_value, bool *promote_return)
{
	string udwhaz03str;
	int      danger = FALSE;
	int	     expsou = 0;
	double   safety_contour = S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR);

	if (depth_value == UNKNOWN)
	{
		for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size();i++)
		{
			if (m_pRecFeature->m_Attf.m_vecAttf[i].attl == 93)
			{
				expsou = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toFloat();
			}
		}
		if (expsou != 1)
			danger = TRUE;
	}

	if (danger == FALSE && (expsou == 1 || depth_value <= safety_contour))
	{
		QVector<CS57RecFeature*> vecFeature = m_pRecFeature->getParentCell()->GetAssociatedFeatures(m_pRecFeature);

		for (int i = 0; i < vecFeature.size(); i++)
		{
			CS57RecFeature* tmpFE = vecFeature[i];
			if (tmpFE->m_Frid.prim == 2)
			{
				double drval2 = 0.0;
				for (int j = 0; j < tmpFE->m_Attf.m_vecAttf.size();j++)
				{
					if (tmpFE->m_Attf.m_vecAttf[j].attl == 88)
					{
						drval2 = tmpFE->m_Attf.m_vecAttf[j].atvl.toFloat();
					}
				}
				if (drval2 < safety_contour)
				{
					danger = TRUE;
					break;
				}
			}
			else
			{
				double drval1 = 0.0;
				for (int j = 0; j < tmpFE->m_Attf.m_vecAttf.size();j++)
				{
					if (tmpFE->m_Attf.m_vecAttf[j].attl == 87)
					{
						drval1 = tmpFE->m_Attf.m_vecAttf[j].atvl.toFloat();
					}
				}

				if (drval1 >= safety_contour && expsou != 1)
				{
					danger = TRUE;
					break;
				}
			}
		}
	}


	if (TRUE == danger)
	{
		int watlev = 0; // Enum 0 invalid
		for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size();i++)
		{
			switch (m_pRecFeature->m_Attf.m_vecAttf[i].attl)
			{
			case 187:
				watlev = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toFloat();
				break;
			default:
				break;
			}
		}

		if (watlev != 1 && watlev != 2)
		{
			//udwhaz03str = (";SY(ISODGR51)");
			udwhaz03str = ";SY(ISODGR01)";
		}
	}
	return udwhaz03str;
}

//沉船
string CS57SymbolRules::WRECKS02()
{
	string wrecks02str;
	string sndfrm02str;
	string udwhaz03str;
	string quapnt01str;
	double least_depth = UNKNOWN;
	double depth_value = UNKNOWN;
	double valsou = UNKNOWN;
	bool b_promote = false;
	int catwrk = -9;
	int watlev = -9;
	string quasoustr;
	int quasou = -9;
	char quasouchar[LISTSIZE] = { '\0' };

	for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size();i++)
	{
		switch (m_pRecFeature->m_Attf.m_vecAttf[i].attl)
		{
		case 71:
			catwrk = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toFloat();
			break;
		case 125:
		{
			char quasouchar[20] = { '\0' };
			strcpy(quasouchar, (const char*)m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toLocal8Bit());
			quasoustr = quasouchar;
			break;
		}
		case 179:
			valsou = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toFloat();
			break;
		case 187:
			watlev = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toInt();
			break;
		default:
			break;
		}
	}
	double safety_contour = S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR);

	if (valsou != UNKNOWN)
	{
		depth_value = valsou;
		sndfrm02str = SNDFRM02(depth_value);
	}
	else
	{
		if (m_pRecFeature->m_Frid.prim == 3)
			least_depth = _DEPVAL01(least_depth);

		if (least_depth == UNKNOWN)
		{
			if (catwrk != -9)
			{
				switch (catwrk)
				{
				case 1: depth_value = 20.0; break;       // safe
				case 2: depth_value = 0.0; break;       // dangerous
				case 4:
				case 5: depth_value = -15.0; break;
				}
			}
			else
			{
				if (watlev == -9) // default
					depth_value = -15.0;
				else
				{
					switch (watlev)
					{
					case 1:
					case 2: depth_value = -15.0; break;
					case 3: depth_value = 0.01; break;
					case 4: depth_value = -15.0; break;
					case 5: depth_value = 0.0; break;
					case 6: depth_value = -15.0; break;
					}
				}
			}
		}
		else
			depth_value = least_depth;
	}
	if (quasoustr != "")
		_parseList(quasoustr.c_str(), quasouchar, sizeof(quasouchar));

	if (quasouchar[0] == 0 || NULL == strpbrk(quasouchar, "\07"))
	{
		udwhaz03str = _UDWHAZ03(depth_value, &b_promote);
	}
	else
	{
		quasou = 7;
		udwhaz03str = "";
	}
	quapnt01str = CSQUAPNT01();

	if (m_pRecFeature->m_Frid.prim == 1)
	{
		if (0 != udwhaz03str.length())
		{
			wrecks02str = udwhaz03str;
			wrecks02str.append(quapnt01str);
		}
		else
		{
			// Continuation A (POINT_T)
			if (valsou != UNKNOWN)
			{
				if ((valsou < safety_contour)/* || (2 == catwrk)*/)    // maybe redundant, seems like wrecks with valsou < 20
																	   // are always coded as "dangerous wrecks"
																	   // Excluding (2 == catwrk) matches Caris logic
					wrecks02str = string((";SY(DANGER02)"));
				wrecks02str.append((";TX('Wk',2,1,2,'15110',1,0,CHBLK,21)"));
				if (7 == quasou) //Fixes FS 165
					wrecks02str.append((";SY(WRECKS07)"));

				wrecks02str.append(sndfrm02str);       // always show valsou depth
													   ///////////////////////////////////////////

				wrecks02str.append(udwhaz03str);
				wrecks02str.append(quapnt01str);
			}
			else
			{
				string sym;
				if (catwrk != -9 && watlev != -9)
				{
					if (catwrk == 1 && watlev == 3)
						sym = (";SY(WRECKS04)");
					else
					{
						if (catwrk == 2 && watlev == 3)
							sym = (";SY(WRECKS05)");
						else
						{
							if (catwrk == 4 || catwrk == 5)
								sym = (";SY(WRECKS01)");
							else
							{
								if (watlev == 1 ||
									watlev == 2 ||
									watlev == 5 ||
									watlev == 4)
								{
									sym = (";SY(WRECKS01)");
								}
								else
									sym = (";SY(WRECKS05)"); // default
							}
						}
					}
				}
				wrecks02str = sym;
				if (quapnt01str != "")
					wrecks02str.append(quapnt01str);
			}
		}
	}
	else
	{
		// Continuation B (AREAS_T)
		int quapos = 0;
		string line;

		if (quapos >= 2 && quapos < 10)
			line = (";LC(LOWACC41)");
		else
		{
			if (udwhaz03str.length() != 0)
				line = (";LS(DOTT,2,CHBLK)");
			else
			{
				if (UNKNOWN != valsou)
				{
					if (valsou <= 20)
						line = (";LS(DOTT,2,CHBLK)");
					else
						line = (";LS(DASH,2,CHBLK)");
				}
				else
				{
					if (watlev == -9)
						line = (";LS(DOTT,2,CSTLN)");
					else
					{
						switch (watlev)
						{
						case 1:
						case 2: line = (";LS(SOLD,2,CSTLN)"); break;
						case 4: line = (";LS(DASH,2,CSTLN)"); break;
						case 3:
						case 5:

						default: line = (";LS(DOTT,2,CSTLN)"); break;
						}
					}
				}
			}
		}
		wrecks02str = string(line);

		if (UNKNOWN != valsou)
		{
			if (valsou <= 20)
			{
				wrecks02str.append(udwhaz03str);
				wrecks02str.append(quapnt01str);
				wrecks02str.append(sndfrm02str);
			}
			else
			{
				// NOTE: ??? same as above ???
				wrecks02str.append(udwhaz03str);
				wrecks02str.append(quapnt01str);
			}
		}
		else
		{
			string ac;

			if (watlev == -9)
				ac = (";AC(DEPVS)");
			else
				switch (watlev) {
				case 1:
				case 2: ac = (";AC(CHBRN)"); break;
				case 4: ac = (";AC(DEPIT)"); break;
				case 5:
				case 3:
				default: ac = (";AC(DEPVS)"); break;
				}

			wrecks02str.append(ac);

			wrecks02str.append(udwhaz03str);
			wrecks02str.append(quapnt01str);
		}
	}

	wrecks02str.append("\037");

	return wrecks02str;
}
string CS57SymbolRules::CSQUAPNT01()
{
	string quapnt01;
	int accurate = TRUE;
	int qualty = 10;
	bool bquapos = false;
	//bool bquapos = GetIntAttr(obj, "QUAPOS", qualty);

	if (bquapos)
	{
		if (2 <= qualty && qualty < 10)
			accurate = FALSE;
	}

	if (!accurate)
	{
		switch (qualty)
		{
		case 4:
			quapnt01.append((";SY(QUAPOS01)")); break;      // "PA"
		case 5:
			quapnt01.append((";SY(QUAPOS02)")); break;      // "PD"
		case 7:
		case 8:
			quapnt01.append((";SY(QUAPOS03)")); break;      // "REP"
		default:
			quapnt01.append((";SY(LOWACC03)")); break;      // "?"
		}
	}

	quapnt01.append("\037");

	string r;

	r = quapnt01;
	return r;
}
double CS57SymbolRules::_DEPVAL01(double least_depth)
{
	least_depth = UNKNOWN;
	return least_depth;
}

//未知
string CS57SymbolRules::QUALIN01()
{
	string q = CSQUALIN01();
	return q;
}
string CS57SymbolRules::QUAPNT02()
{
	string q = CSQUAPNT01();
	return q;
}
string CS57SymbolRules::SLCONS04()
{
	string slcons03;
	bool bvalstr = false;
	int ival;
	const char *cmdw = NULL;   // command word
	int quapos;
	bool bquapos = false;

	for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size();i++)
	{
		if (m_pRecFeature->m_Attf.m_vecAttf[i].attl == 402 &&
			m_pRecFeature->m_Attf.m_vecAttf[i].atvl != "")
		{
			quapos = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toInt();
			bquapos = true;
		}
	}
	if (m_pRecFeature->m_Frid.prim == 1)
	{
		if (bquapos)
		{
			if (2 <= quapos && quapos < 10)
				cmdw = "SY(LOWACC01)";
		}
	}
	else
	{

		// This instruction not found in PLIB 3.4, but seems to appear in later PLIB implementations
		// by commercial ECDIS providers, so.....
		if (m_pRecFeature->m_Frid.prim == 3)
		{
			slcons03 = ("AP(CROSSX01);");
		}

		// GEO_LINE and GEO_AREA are the same
		if (bquapos) {
			if (2 <= quapos && quapos < 10)
				cmdw = "LC(LOWACC01)";
		}
		else
		{
			for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size();i++)
			{
				if (m_pRecFeature->m_Attf.m_vecAttf[i].attl == 81 &&
					m_pRecFeature->m_Attf.m_vecAttf[i].atvl != "")
				{
					ival = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toInt();
					bvalstr = true;
				}
			}
			if (bvalstr && (1 == ival || 2 == ival))
				cmdw = "LS(DASH,1,CSTLN)";
			else
			{
				ival = 0;

				for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size();i++)
				{
					if (m_pRecFeature->m_Attf.m_vecAttf[i].attl == 60 &&
						m_pRecFeature->m_Attf.m_vecAttf[i].atvl != "")
					{
						ival = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toInt();
						bvalstr = true;
					}
				}

				if (bvalstr && (6 == ival || 15 == ival || 16 == ival))      // Some sort of wharf
					cmdw = "LS(SOLD,4,CSTLN)";
				else {
					for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size();i++)
					{
						if (m_pRecFeature->m_Attf.m_vecAttf[i].attl == 187 &&
							m_pRecFeature->m_Attf.m_vecAttf[i].atvl != "")
						{
							ival = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toInt();
							bvalstr = true;
						}
					}

					if (bvalstr && 2 == ival)
						cmdw = "LS(SOLD,2,CSTLN)";
					else
						if (bvalstr && (3 == ival || 4 == ival))
							cmdw = "LS(DASH,2,CSTLN)";
						else
							cmdw = "LS(SOLD,2,CSTLN)";  // default
				}
			}
		}
	}

	if (NULL != cmdw)
		slcons03.append(cmdw);

	slcons03.append("\037");

	return slcons03;
}
string CS57SymbolRules::DATCVR02()
{
	string rule_str;
	rule_str.append("LC(HODATA01)");
	string datcvr01;
	datcvr01.append(rule_str);
	datcvr01.append("\037");

	return datcvr01;
}

//未知物标
string CS57SymbolRules::QUAPOS01()
{
	string q = "";
	if (m_pRecFeature->m_Frid.prim == 2)
		q = CSQUALIN01();

	else
		q = CSQUAPNT01();

	return q;
}
string CS57SymbolRules::CSQUALIN01()
{
	string qualino1;
	int quapos = 0;
	bool bquapos = false;
	for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size();i++)
	{
		if (m_pRecFeature->m_Attf.m_vecAttf[i].attl == 402 &&
			m_pRecFeature->m_Attf.m_vecAttf[i].atvl != "")
		{
			quapos = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toInt();
			bquapos = true;
		}
	}
	const char *line = NULL;

	if (bquapos) {
		if (2 <= quapos && quapos < 10)
			line = "LC(LOWACC21)";
	}
	else
	{
		if (m_pRecFeature->m_Frid.objl == 30)
		{
			int conrad;
			bool bconrad = false;
			for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size();i++)
			{
				if (m_pRecFeature->m_Attf.m_vecAttf[i].attl == 82 &&
					m_pRecFeature->m_Attf.m_vecAttf[i].atvl != "")
				{
					conrad = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toInt();
					bconrad = true;
				}
			}

			if (bconrad)
			{
				if (1 == conrad)
					line = "LS(SOLD,3,CHMGF);LS(SOLD,1,CSTLN)";
				else
					line = "LS(SOLD,1,CSTLN)";
			}
			else
				line = "LS(SOLD,1,CSTLN)";

		}
		else  //LNDARE
			line = "LS(SOLD,1,CSTLN)";
	}

	if (NULL != line)
		qualino1.append(line);

	qualino1.append("\037");
	return qualino1;
}

//等深线
string CS57SymbolRules::DEPCNT03()
{
	double depth_value;
	double drval1 = 0.0, drval2 =0.0;
	bool safe = FALSE;
	string rule_str;
	double safety_contour = S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR);

	if (m_pRecFeature->m_Frid.objl == 42 && m_pRecFeature->m_Frid.prim == 2)
	{
		drval1 = 0.0;                                          // default values
		for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size();i++)
		{
			if (m_pRecFeature->m_Attf.m_vecAttf[i].attl == 87 &&
				m_pRecFeature->m_Attf.m_vecAttf[i].atvl != "")
			{
				drval1 = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toFloat();
			}
		}
		drval2 = drval1;
		for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size();i++)
		{
			if (m_pRecFeature->m_Attf.m_vecAttf[i].attl == 88 &&
				m_pRecFeature->m_Attf.m_vecAttf[i].atvl != "")
			{
				drval2 = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toFloat();
			}
		}

		if (drval1 <= safety_contour)
		{
			if (drval2 >= safety_contour)
				safe = TRUE;
		}
		else
		{
			double next_safe_contour = 1e6;
			if (m_pRecFeature)
			{
				next_safe_contour = 10.;
				if (drval1 == next_safe_contour)
					safe = TRUE;
			}
			else
			{
				next_safe_contour = 10.;

				if (fabs(drval1 - next_safe_contour) < 1e-4)
					safe = true;
			}
		}

		depth_value = drval1;

	}
	else
	{
		// continuation A (DEPCNT)
		double valdco = 0.0;
		for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size();i++)
		{
			if (m_pRecFeature->m_Attf.m_vecAttf[i].attl == 174 &&
				m_pRecFeature->m_Attf.m_vecAttf[i].atvl != "")
			{
				valdco = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toFloat();
			}
		}

		depth_value = valdco;

		if (valdco == safety_contour)
			safe = TRUE;   // this is useless !?!?
		else
		{
			//double next_safe_contour = 1e6;
			//if (mCell) 
			//{
			//	next_safe_contour = 10.;
			//	if (valdco == next_safe_contour)
			//		safe = TRUE;
			//}
			//else 
			//{
			//	next_safe_contour = 10.;

			//	if (fabs(valdco - next_safe_contour) < 1e-4)
			//		safe = true;
			//}
			safe = false;
		}
	}

	// Continuation B
	int quapos = 0;
	for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size();i++)
	{
		if (m_pRecFeature->m_Attf.m_vecAttf[i].attl == 402 &&
			m_pRecFeature->m_Attf.m_vecAttf[i].atvl != "")
		{
			quapos = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toFloat();
		}
	}

	if (0 != quapos)
	{
		if (2 <= quapos && quapos < 10)
		{
			if (safe)
			{
				string safeCntr = "LS(DASH,2,DEPSC)";
				//S57Obj tempObj;
				//LUPrec* safelup = ps52plib->S52_LUPLookup(PLAIN_BOUNDARIES, "SAFECD", &tempObj, false);
				//if (safelup)
				//	safeCntr = *safelup->INST;
				rule_str = ";" + safeCntr;
			}
			else
				rule_str = ";LS(DASH,1,DEPCN)";
		}
	}
	else
	{
		if (safe)
		{
			string safeCntr = "LS(SOLD,2,DEPSC)";
			//S57Obj tempObj;
			//LUPrec* safelup = ps52plib->S52_LUPLookup(PLAIN_BOUNDARIES, "SAFECN", &tempObj, false);
			//if (safelup)
			//	safeCntr = *safelup->INST;
			rule_str = ";" + safeCntr;
		}
		else
			rule_str = ";LS(SOLD,1,DEPCN)";
	}

	rule_str.append("\037");

	return rule_str;
}


//限制区
string CS57SymbolRules::RESARE04()
{
	string resare02;
	char  restrn[LISTSIZE] = { '\0' };
	string restrnstr;
	string catreastr;
	char  catrea[LISTSIZE] = { '\0' };
	string symb;
	string line;
	string prio;
	for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size(); i++)
	{
		if (m_pRecFeature->m_Attf.m_vecAttf[i].attl == 131)
		{
			char restrnchar[20] = { '\0' };
			strcpy(restrnchar, (const char*)m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toLocal8Bit());
			restrnstr = restrnchar;
		}
		if (m_pRecFeature->m_Attf.m_vecAttf[i].attl == 56)
		{
			char catreachar[20] = { '\0' };
			strcpy(catreachar, (const char*)m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toLocal8Bit());
			catreastr = catreachar;
		}
	}

	if (catreastr != "")
		_parseList(catreastr.c_str(), catrea, sizeof(catrea));

	if (restrnstr != "")
	{
		_parseList(restrnstr.c_str(), restrn, sizeof(restrn));

		// Does 'RESTRN'include 7 || 8 || 14 ? // Continuation A
		if (strpbrk(restrn, "\007\010\016"))
		{
			// Continuation A
			// Does 'RESTRN' include 1 || 2 || 3 || 4 || 5 || 6 ||13 || 16 || 17 || 23 || 24 || 25 || 26 || 27 ?
			if (strpbrk(restrn, "\001\002\003\004\005\006\015\20\21\27\30\31\32\33"))         // anchoring, fishing, trawling
				symb = ";SY(ENTRES61)";

			// Has value('CATREA') &&Does 'CATREA' include 1||8||9||12||14||18 || 19 ||21 || 24 || 25 || 26 ?
			else if (catreastr != "" && strpbrk(catrea, "\001\010\011\014\016\022\023\025\030\031\32"))
				symb = ";SY(ENTRES61)";

			// Does 'RESTRN' include 9 || 10 || 11|| 12 || 15 || 18 || 19 || 20 || 21 || 22 ?
			else if (strpbrk(restrn, "\011\012\013\014\017\022\023\024\025\026"))
				symb = ";SY(ENTRES71)";
			// Has value('CATREA') &&Does 'CATREA' include 4 || 5|| 6 || 7 || 10 || 20 || 22 || 23 ?

			else if (catreastr != "" && strpbrk(catrea, "\004\005\006\007\012\024\026\027"))
				symb = ";SY(ENTRES71)";

			else
				symb = ";SY(ENTRES51)";

			if (TRUE == S52_getMarinerParam(S52_MAR_SYMBOLIZED_BND))
				line = ";LC(ENTRES51)";
			else
				line = ";LS(DASH,2,CHMGD)";

			prio = ";OP(6---)";  // display prio set to 6

		}

		// Does 'RESTRN'include 1 || 2 ? // Continuation B
		else if (strpbrk(restrn, "\001\002"))
		{
			// Continuation B
			// Does 'RESTRN' include 3 || 4 || 5 || 6 ||13 || 16 || 17 || 23 || 24 || 25 || 26 || 27 ?
			if (strpbrk(restrn, "\003\004\005\006\015\020\021\027\030\031\032\033"))
				symb = ";SY(ACHRES61)";
			// Has value ('CATREA') &&Does 'CATREA' include 1 ||8 || 9 || 12 || 14 || 18 || 19 ||21 || 24 || 25 || 26 ?
			else if (catreastr != "" && strpbrk(catrea, "\001\010\011\014\016\022\023\025\030\031\032"))
				symb = ";SY(ACHRES61)";
			// Does 'RESTRN' include 9 || 10 || 11 ||12 || 15 || 18 || 19 || 20 || 21 || 22 ?
			else if (strpbrk(restrn, "\011\012\013\014\017\022\023\024\025\026"))
				symb = ";SY(ACHRES71)";
			// Has value('CATREA') && Does 'CATREA' include 4 || 5|| 6 || 7 || 10 || 20 || 22 || 23 ?
			else if (catreastr != "" && strpbrk(catrea, "\004\005\006\007\012\\024\026\027"))
				symb = ";SY(ACHRES71)";
			else
				symb = ";SY(RESTRN51)";

			if (TRUE == S52_getMarinerParam(S52_MAR_SYMBOLIZED_BND))
				line = ";LC(ACHRES51)";                // could be ACHRES51 when _drawLC is implemented fully
			else
				line = ";LS(DASH,2,CHMGD)";

			prio = ";OP(6---)";  // display prio set to 6
		}

		// Does 'RESTRN' include 3 || 4 || 5 || 6 || 24 ? // Continuation C
		else if (strpbrk(restrn, "\003\004\005\006\030"))
		{
			// Continuation C
			// Does 'RESTRN' include 13 || 16|| 17 || 23 || 24 || 25 || 26 || 27 ?
			if (strpbrk(restrn, "\015\020\021\024\030\031\032\033"))
				symb = ";SY(FSHRES71)";
			// Has value('CATREA') &&Does 'CATREA' include 1 || 8|| 9 || 12 || 14 || 18 || 19 || 21	|| 24 || 25 || 26 ?
			else if (catreastr != ""&& strpbrk(catrea, "\001\010\011\014\016\022\023\025\030\031\032"))
				symb = ";SY(FSHRES61)";
			// Does 'RESTRN' include 9 || 10 || 11|| 12 || 15 || 18 || 19 || 20 || 21 || 22 ?
			else if (strpbrk(restrn, "\011\012\013\014\022\023\024\025\026"))
				symb = ";SY(FSHRES71)";
			// Has value('CATREA') &&Does 'CATREA' include 4 || 5|| 6 || 7 || 10 || 20 || 22 || 23 ?
			else if (catreastr != "" && strpbrk(catrea, "\004\005\006\007\012\024\026\027"))
				symb = ";SY(FSHRES71)";
			else
				symb = ";SY(FSHRES51)";

			if (TRUE == S52_getMarinerParam(S52_MAR_SYMBOLIZED_BND))
				line = ";LC(FSHRES51)";
			else
				line = ";LS(DASH,2,CHMGD)";

			prio = ";OP(6---)";  // display prio set to 6

		}

		// Does 'RESTRN' include 13 || 16 || 17 || 23 || 25 ||26 || 27 ? // Continuation D
		else if (strpbrk(restrn, "\015\020\021\027\031\032\033"))
		{
			// Does 'RESTRN' include 9 || 10 || 11 ||12 || 15 || 18 || 19 || 20 || 21 || 22 ?
			if (strpbrk(restrn, "\011\012\013\014\017\022\023\024\025\026"))
				symb = ";SY(CTYARE71)";
			// Has value('CATREA') &&Does 'CATREA' include 4 || 5|| 6 || 7 || 10 || 20 || 22 || 23 ?
			else if (catreastr != ""&& strpbrk(catrea, "\004\005\006\007\012\024\026\027"))
				symb = ";SY(CTYARE71)";
			else
				symb = ";SY(CTYARE51)";

			if (TRUE == S52_getMarinerParam(S52_MAR_SYMBOLIZED_BND))
				line = ";LC(CTYARE51)";
			else
				line = ";LS(DASH,2,CHMGD)";
			prio = ";OP(6---)";  // display prio set to 6			
		}
		else
		{
			// Does 'RESTRN' include 9 || 10 || 11 || 12 || 15 ||18 || 19 || 20 || 21 || 22 ?
			if (strpbrk(restrn, "\011\012\013\014\017\022\023\024\025\026"))
				symb = ";SY(INFARE51)";
			else
				symb = ";SY(RSRDEF51)";

			if (TRUE == S52_getMarinerParam(S52_MAR_SYMBOLIZED_BND))
				line = ";LC(CTYARE51)";
			else
				line = ";LS(DASH,2,CHMGD)";

		}
	}
	else
	{
		// Continuation E
		// Has value('CATREA') ?
		if (catreastr != "")
		{
			// Does 'CATREA' include 1 ||8 || 9 || 12 || 14 || 18 || 19 ||21 || 24 || 25 || 26 ?
			if (strpbrk(catrea, "\001\010\011\014\016\024\025\030\031\032"))
			{
				//Does 'CATREA' include 4 || 5|| 6 || 7 || 10 || 20 || 22 || 23 ?
				if (strpbrk(catrea, "\004\005\006\007\012\024\026\027"))
					symb = ";SY(CTYARE71)";
				else
					symb = ";SY(CTYARE51)";
			}
			else
			{
				// Does 'CATREA' include 4 || 5	|| 6 || 7 || 10 || 20 || 22 || 23 ?
				if (strpbrk(catrea, "\004\005\006\007\012\024\026\027"))
					symb = ";SY(INFARE51)";
				else
					symb = ";SY(RSRDEF51)";
			}
		}
		else
			symb = ";SY(RSRDEF51)";

		if (TRUE == S52_getMarinerParam(S52_MAR_SYMBOLIZED_BND))
			line = ";LC(CTYARE51)";
		else
			line = ";LS(DASH,2,CHMGD)";
	}

	// create command word
	if (prio.length())
		resare02.append(prio);
	resare02.append(line);
	resare02.append(symb);

	resare02.append("\037");

	return resare02;
}



//深度区
string CS57SymbolRules::DEPARE03()
{
	double drval1 = 0.0, drval2 = 0.0;
	bool drval1_found = false;

	drval1 = -1.0;
	for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size();i++)
	{
		if (m_pRecFeature->m_Attf.m_vecAttf[i].attl == 87 &&
			m_pRecFeature->m_Attf.m_vecAttf[i].atvl != "")
		{
			drval1 = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toFloat();
			drval1_found = true;
		}
	}

	// default values
	drval2 = drval1 + 0.01;
	for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size();i++)
	{
		if (m_pRecFeature->m_Attf.m_vecAttf[i].attl == 88 &&
			m_pRecFeature->m_Attf.m_vecAttf[i].atvl != "")
		{
			drval2 = m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toFloat();
		}
	}

	//   Create a string of the proper color reference

	bool shallow = TRUE;
	string rule_str = "AC(DEPIT)";			// 黄绿色


	if (drval1 >= 0.0 && drval2 > 0.0)			// 深度值大于０，浅兰色
		rule_str = "AC(DEPVS)";

	if (TRUE == S52_getMarinerParam(S52_MAR_TWO_SHADES))	// 大于安全水深，背景色
	{
		if (drval1 >= S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR) &&
			drval2 > S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR))
		{
			rule_str = "AC(DEPDW)";
			shallow = FALSE;
		}
	}
	else
	{
		if (drval1 >= S52_getMarinerParam(S52_MAR_SHALLOW_CONTOUR) &&	// 大于浅水深等深线，浅兰色
			drval2 > S52_getMarinerParam(S52_MAR_SHALLOW_CONTOUR))
			rule_str = "AC(DEPMS)";

		if (drval1 >= S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR) &&	// 大于安全水深，淡兰色
			drval2 > S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR))
		{
			rule_str = "AC(DEPMD)";
			shallow = FALSE;
		}

		if (drval1 >= S52_getMarinerParam(S52_MAR_DEEP_CONTOUR) &&		// 大于安全水深，背景色
			drval2 > S52_getMarinerParam(S52_MAR_DEEP_CONTOUR))
		{
			rule_str = "AC(DEPDW)";
			shallow = FALSE;
		}
	}
	// 	else if (drval1 >= S52_getMarinerParam(S52_MAR_SHALLOW_CONTOUR) &&	// 大于浅水深，浅兰色
	// 			drval2 > S52_getMarinerParam(S52_MAR_SHALLOW_CONTOUR))
	// 	{
	// 		rule_str = "AC(DEPMS)";
	// 	}
	// 	else if (drval1 >= S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR) &&	// 大于安全水深，淡兰色
	// 			drval2 > S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR))
	// 	{
	// 		rule_str = "AC(DEPMD)";
	// 		shallow = FALSE;
	// 	}
	// 	else if (drval1 >= S52_getMarinerParam(S52_MAR_DEEP_CONTOUR) &&		// 大于深水区，背景色
	// 			drval2 > S52_getMarinerParam(S52_MAR_DEEP_CONTOUR))
	// 	{
	// 		rule_str = "AC(DEPDW)";
	// 		shallow = FALSE;
	// 	}
	// // 	else if(drval2 <= S52_getMarinerParam(S52_MAR_SHALLOW_CONTOUR))
	// // 		rule_str = "AC(DEPVS)";
	// 	else
	// 		rule_str = "AC(DEPDW)";

	//  If object is DRGARE....
	if (m_pRecFeature->m_Frid.objl == 46)
	{
		if (!drval1_found) //If DRVAL1 was not defined...
		{
			rule_str = "AC(DEPMD)";
			shallow = FALSE;
		}
		rule_str.append(";AP(DRGARE01)");
		rule_str.append(";LS(DASH,1,CHGRF)");

	}

	rule_str.append("\037");
	return rule_str;
}

void CS57SymbolRules::_DEPVAL02(float * mLeast_Depth, float * mSebed_Depth)
{
	*mLeast_Depth = UNKNOWN;
	*mSebed_Depth = UNKNOWN;
	if (m_pRecFeature->m_Frid.objl != 86 &&// OBSTRN
		m_pRecFeature->m_Frid.objl != 153 && // UWTROC
		m_pRecFeature->m_Frid.objl != 159) // WRECKS
		return;

	double lat = 0.0, lon = 0.0;
	CS57Cell *pCell = m_pRecFeature->getParentCell();
	double comf = pCell->getCellComf();

	switch (m_pRecFeature->m_Fspt.m_vecFspt[0].rcnm)
	{
	case 110:
		lon = pCell->m_vecS57RecVI[m_pRecFeature->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d[0].xcoo/comf;
		lat = pCell->m_vecS57RecVI[m_pRecFeature->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d[0].ycoo/comf;
		break;
	case 120:
		lon = pCell->m_vecS57RecVC[m_pRecFeature->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d[0].xcoo/comf;
		lat = pCell->m_vecS57RecVC[m_pRecFeature->m_Fspt.m_vecFspt[0].idx]->m_Sg2d.m_vecSg2d[0].ycoo/comf;
		break;
	case 130:
		lon = pCell->m_vecS57RecVC[pCell->m_vecS57RecVE[m_pRecFeature->m_Fspt.m_vecFspt[0].idx]->m_Vrpt.m_vecVrpt[0].idx]->m_Sg2d.m_vecSg2d[0].xcoo/comf;
		lat = pCell->m_vecS57RecVC[pCell->m_vecS57RecVE[m_pRecFeature->m_Fspt.m_vecFspt[0].idx]->m_Vrpt.m_vecVrpt[0].idx]->m_Sg2d.m_vecSg2d[0].ycoo/comf;
		break;
	default:
		return;
	}
	CS57RecFeature * selFE = NULL;
	for (int i = 0; i < pCell->m_vecS57RecFE.size(); i++)
	{
		CS57RecFeature* pFE = pCell->m_vecS57RecFE[i];

		if (pFE->m_Frid.objl != 42 &&// DEPARE
			pFE->m_Frid.objl != 46 &&
			pFE->m_Frid.prim != 3) // DRGARE
			continue;

		QVector<QVector<QPointF>*> vecRings;
		pCell->genFeaturePolygon(pFE, &vecRings);
		CSpatialPolygon featurePolygon(vecRings);
		if (featurePolygon.ptInPolygon(lon, lat))
		{
			for (int j = 0; j < pFE->m_Attf.m_vecAttf.size(); j++)
			{
				if (pFE->m_Attf.m_vecAttf[j].attl == 87) // DRVAL1
				{
					selFE = pFE;
					*mLeast_Depth = pFE->m_Attf.m_vecAttf[j].atvl.toFloat();
					break;
				}
			}
		}
		qDeleteAll(vecRings);
		vecRings.clear();

		if (*mLeast_Depth != UNKNOWN)
			break;
	}
	if (*mLeast_Depth == UNKNOWN || selFE == NULL)
		return;

	int mWatlev = 0;
	int mExpsou = 0;
	for (int i = 0; i < selFE->m_Attf.m_vecAttf.size(); i++)
	{
		switch (selFE->m_Attf.m_vecAttf[i].attl)
		{
		case 93:
			mExpsou = selFE->m_Attf.m_vecAttf[i].atvl.toInt();
			break;
		case 187:
			mWatlev = selFE->m_Attf.m_vecAttf[i].atvl.toInt();
			break;
		default:
			break;
		}
	}
	if (mWatlev == 3 && mExpsou == 1 || mExpsou == 3)
	{
		*mSebed_Depth = *mLeast_Depth;
	}
	else
	{
		*mSebed_Depth = *mLeast_Depth;
		*mLeast_Depth = UNKNOWN;
	}
}
string Doc::CS57SymbolRules::RESTRN01()
{
	char  restrn[LISTSIZE] = { '\0' };
	string restrnstr;

	for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size(); i++)
	{
		if (m_pRecFeature->m_Attf.m_vecAttf[i].attl == 131)
		{
			char restrnchar[20] = { '\0' };
			strcpy(restrnchar, (const char*)m_pRecFeature->m_Attf.m_vecAttf[i].atvl.toLocal8Bit());
			_parseList(restrnchar, restrn, sizeof(restrn));
			restrnstr = restrn;
			return RESCSP02(restrnstr);
		}
	}
	return "";
}

string Doc::CS57SymbolRules::RESCSP02(string strRestrn)
{
	string symb;
	string line;
	string prio;
	// Does 'RESTRN'include 7 || 8 || 14 ?
	if (strpbrk(strRestrn.c_str(), "\007\010\016"))
	{
		// Does 'RESTRN' include 1 || 2 ||3 || 4 || 5 || 6 || 13 || 16 || 17 ||	23 || 24 || 25 || 26 || 27 ?
		if (strpbrk(strRestrn.c_str(), "001\002\003\004\005\006\015\020\021\027\030\031\032\033"))
		{
			symb = ";SY((ENTRES61)";
		}
		// Does 'RESTRN' include 9 ||10 || 11 || 12 || 15 || 18 || 19|| 20 || 21 || 22 ?
		else if (strpbrk(strRestrn.c_str(), "\011\012\013\014\017\022\023\024\025\026"))
		{
			symb = ";SY((ENTRES71)";
		}
		else
		{
			symb = ";SY((ENTRES51)";
		}
	}
	// Does 'RESTRN'include 1 || 2 ?
	else if (strpbrk(strRestrn.c_str(), "\001\002"))
	{
		// Does 'RESTRN' include 3 || 4 || 	5 || 6 || 13 || 16 || 17 || 23 || 24 || 25 || 26 || 27 ?
		if (strpbrk(strRestrn.c_str(), "\003\004\005\006\015\020\021\027\030\031\032\033"))
		{
			symb = ";SY(ACHRES61)";
		}
		// Does 'RESTRN' include 9 ||10 || 11 || 12 || 15 || 18 || 19|| 20 || 21 || 22 ?
		else if(strpbrk(strRestrn.c_str(), "\011\012\013\014\017\022\023\024\025\026"))
		{
			symb = ";SY(ACHRES71)";
		}
		else
		{
			symb = ";SY(ACHRES51)";
		}
	}
	// Does 'RESTRN' include 3 || 4 || 5 || 6 || 24 ?
	else if (strpbrk(strRestrn.c_str(), "\003\004\005\006\030"))
	{
		// Does 'RESTRN' include 13 ||16 || 17 || 23 || 25 || 26 || 27 ?
		if (strpbrk(strRestrn.c_str(), "\015\020\021\027\031\032\033"))
		{
			symb = ";SY(FSHRES61)";
		}
		// Does 'RESTRN' include 9 ||10 || 11 || 12 || 15 || 18 || 19 || 20 || 21 || 22 ?
		else if (strpbrk(strRestrn.c_str(), "\011\012\013\014\017\022\023\024\025\026"))
		{
			symb = ";SY(FSHRES71)";
		}
		else
		{
			symb = ";SY(FSHRES51)";
		}

	}
	// Does 'RESTRN' include 13 ||	16 || 17 || 23 || 25 || 26 || 27 ?
	else if (strpbrk(strRestrn.c_str(), "\015\020\021\027\031\032\033"))
	{
		// Does 'RESTRN' include 9 || 10 || 11 || 12 || 15 || 18 || 19 || 20 || 21 || 22 ?
		if (strpbrk(strRestrn.c_str(), "\011\012\013\014\017\022\023\024\025\026"))
		{
			symb = ";SY(CTYARE51)";
		}
		else
		{
			symb = ";SY(CTYARE71)";
		}
	}
	// Does 'RESTRN' include 9 || 10 || 11 ||12 || 15 || 18 || 19 || 20 || 21 || 22 ?
	else if (strpbrk(strRestrn.c_str(), "\011\012\013\014\017\022\023\024\025\026"))
	{
		symb = ";SY(RSRDEF51)";
	}
	else
	{
		symb = ";SY(INFARE51)";
	}
	return symb;
}

string CS57SymbolRules::ACHARE00()
{
	bool chineseFlag = false;
	string resultString = "";
	QString inform = "";
	QString objnam = "";
	QString ninfom = "";
	QString nobjnm = "";
	QString catach = "";
	QString status = "";
	if (chineseFlag)
	{
		for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size(); i++)
		{
			switch (m_pRecFeature->m_Attf.m_vecAttf[i].attl)
			{
			case 102://INFORM
				inform = m_pRecFeature->m_Attf.m_vecAttf[i].atvl;
				break;
			case 116://OBJNAM
				objnam = m_pRecFeature->m_Attf.m_vecAttf[i].atvl;
				break;
			}
		}
		for (int i = 0; i < m_pRecFeature->m_Natf.m_vecNatf.size(); i++)
		{
			switch (m_pRecFeature->m_Natf.m_vecNatf[i].attl)
			{
			case 300://NINFOM
			{
				QTextCodec* codec_16 = QTextCodec::codecForName("UTF-16");
				ninfom = codec_16->toUnicode(m_pRecFeature->m_Natf.m_vecNatf[i].atvl);
				break;
			}
			case 301://NOBJNM
			{
				QTextCodec* codec_16 = QTextCodec::codecForName("UTF-16");
				nobjnm = codec_16->toUnicode(m_pRecFeature->m_Natf.m_vecNatf[i].atvl);
				break;
			}
			default:
				break;
			}
		}
		if (ninfom != "" && nobjnm != "")
		{
			//QString str = "TE('%s(%s)','NOBJNM,NINFOM',2,2,2,'64112',-1,0,CHBLK,26)";
			QString str = QString("TE('%s(%s)','%1,%2',2,2,2,'64112',-1,0,CHBLK,26)").arg(nobjnm).arg(ninfom);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (ninfom == "" && nobjnm != "")
		{
			//QString str = "TX('N0BJNM',2,2,2,'64112',-1,0,CHBLK,26)";
			QString str = QString("TX('%1',2,2,2,'64112',-1,0,CHBLK,26)").arg(nobjnm);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (ninfom != "" && nobjnm == "")
		{
			//QString str = "TX('NINF0M',2,2,2,'64112',-1,0,CHBLK,26)";
			QString str = QString("TX('%1',2,2,2,'64112',-1,0,CHBLK,26)").arg(ninfom);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (inform != "" && objnam != "")
		{
			//QString str = "TE('%s(%s)'，'OBJNAM,INFORM',2,2,2,'64112',-1,0,CHBLK,26)";
			QString str = QString("TE('%s(%s)'，'%1,%2',2,2,2,'64112',-1,0,CHBLK,26)").arg(objnam).arg(inform);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (inform == "" && objnam != "")
		{
			//QString str = "TX('OBJNAM',2,2,2,'64112',-1,0,CHBLK,26)";
			QString str = QString("TX('%1',2,2,2,'64112',-1,0,CHBLK,26)").arg(objnam);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (inform != "" && objnam == "")
		{
			//QString str = "TX('INF0RM',2,2,2,'64112',-1,0,CHBLK,26)";
			QString str = QString("TX('%1',2,2,2,'64112',-1,0,CHBLK,26)").arg(inform);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
	}
	else
	{
		for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size(); i++)
		{
			switch (m_pRecFeature->m_Attf.m_vecAttf[i].attl)
			{
			case 102://INFORM
				inform = m_pRecFeature->m_Attf.m_vecAttf[i].atvl;
				break;
			case 116://OBJNAM
				objnam = m_pRecFeature->m_Attf.m_vecAttf[i].atvl;
				break;
			}
		}
		if (inform != "" && objnam != "")
		{
			//QString str = "TE('%s(%s)'，'OBJNAM,INFORM',2,2,2,'64112',-1,0,CHBLK,26)";
			QString str = QString("TE('%s(%s)'，'%1,%2',2,2,2,'64112',-1,0,CHBLK,26)").arg(objnam).arg(inform);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (inform == "" && objnam != "")
		{
			//QString str = "TX('OBJNAM',2,2,2,'64112',-1,0,CHBLK,26)";
			QString str = QString("TX('%1',2,2,2,'64112',-1,0,CHBLK,26)").arg(objnam);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (inform != "" && objnam == "")
		{
			//QString str = "TX('INF0RM',2,2,2,'64112',-1,0,CHBLK,26)";
			QString str = QString("TX('%1',2,2,2,'64112',-1,0,CHBLK,26)").arg(inform);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
	}
	for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size(); i++)
	{
		switch (m_pRecFeature->m_Attf.m_vecAttf[i].attl)
		{
		case 8://CATACH
			catach = m_pRecFeature->m_Attf.m_vecAttf[i].atvl;
			break;
		case 149:
			status = m_pRecFeature->m_Attf.m_vecAttf[i].atvl;
			break;
		}
	}
	for (int i = 0; i < m_pRecFeature->m_Natf.m_vecNatf.size(); i++)
	{
		switch (m_pRecFeature->m_Natf.m_vecNatf[i].attl)
		{
		case 300://NINFOM
		{
			QTextCodec* codec_16 = QTextCodec::codecForName("UTF-16");
			ninfom = codec_16->toUnicode(m_pRecFeature->m_Natf.m_vecNatf[i].atvl);
			break;
		}
		}
	}
	if (ninfom != "" && catach != "")
	{
		//QString str = "TX('NINF0M'2,2,2,'64112',-1,0,CHBLK,26)";
		QString str = QString("TX('%1',2,2,2,'64112',-1,0,CHBLK,26)").arg(ninfom);
		resultString.append(string((const char*)str.toLocal8Bit()));
	}
	else if (ninfom == "" && catach != "")
	{
		if (catach == "2")
		{
			//QString str = "TX('深水',2,2,2,'64112',-1,0,CHBLK,26)";
			QString str = QString("TX('深水',2,2,2,'64112',-1,0,CHBLK,26)");
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (catach == "3")
		{
			//QString str = "TX('游轮',2,2,2,'64112',-1,0,CHBLK,26)";
			QString str = QString("TX('游轮',2,2,2,'64112',-1,0,CHBLK,26)");
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (catach == "7")
		{
			//QString str = "TX('小船',2,2,2,'64112',-1,0,CHBLK,26)";
			QString str = QString("TX('小船',2,2,2,'64112',-1,0,CHBLK,26)");
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (catach == "1001")
		{
			//QString str = "TX('防台',2,2,2,'64112',-1,0,CHBLK,26)";
			QString str = QString("TX('防台',2,2,2,'64112',-1,0,CHBLK,26)");
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (catach == "1002")
		{
			//QString str = "TX('驳载',2,2,2,'64112',-1,0,CHBLK,26)";
			QString str = QString("TX('驳载',2,2,2,'64112',-1,0,CHBLK,26)");
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (status == "6")
		{
			//QString str = "TX('备用',2,2,2,'64112',-1,0,CHBLK,26)";
			QString str = QString("TX('备用',2,2,2,'64112',-1,0,CHBLK,26)");
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
	}

	return resultString;
}

string CS57SymbolRules::ACHBRT00()
{
	bool chineseFlag = false;
	string resultString = "";
	QString inform = "";
	QString objnam = "";
	QString ninfom = "";
	QString nobjnm = "";
	QString catach = "";
	QString status = "";
	if (chineseFlag)
	{
		for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size(); i++)
		{
			switch (m_pRecFeature->m_Attf.m_vecAttf[i].attl)
			{
			case 102://INFORM
				inform = m_pRecFeature->m_Attf.m_vecAttf[i].atvl;
				break;
			case 116://OBJNAM
				objnam = m_pRecFeature->m_Attf.m_vecAttf[i].atvl;
				break;
			}
		}
		for (int i = 0; i < m_pRecFeature->m_Natf.m_vecNatf.size(); i++)
		{
			switch (m_pRecFeature->m_Natf.m_vecNatf[i].attl)
			{
			case 300://NINFOM
			{
				QTextCodec* codec_16 = QTextCodec::codecForName("UTF-16");
				ninfom = codec_16->toUnicode(m_pRecFeature->m_Natf.m_vecNatf[i].atvl);
				break;
			}
			case 301://NOBJNM
			{
				QTextCodec* codec_16 = QTextCodec::codecForName("UTF-16");
				nobjnm = codec_16->toUnicode(m_pRecFeature->m_Natf.m_vecNatf[i].atvl);
				break;
			}
			default:
				break;
			}
		}
		if (ninfom != "" && nobjnm != "")
		{
			//QString str = "TE('%s(%s)'，'NOBJNM,NINFOM',1,2,2,'65110',0,0,CHBLK,29)";
			QString str = QString("TE('%s(%s)','%1,%2',1,2,2,'65110',0,0,CHBLK,29)").arg(nobjnm).arg(ninfom);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (ninfom == "" && nobjnm != "")
		{
			//QString str = "TX('N0BJNM',1,2,2,'65110',0,0,CHBLK,29)";
			QString str = QString("TX('%1',1,2,2,'65110',0,0,CHBLK,29)").arg(nobjnm);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (ninfom != "" && nobjnm == "")
		{
			//QString str = "TX('NINF0M',1,2,2,'65110',0,0,CHBLK,29)";
			QString str = QString("TX('%1',1,2,2,'65110',0,0,CHBLK,29)").arg(ninfom);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (inform != "" && objnam != "")
		{
			//QString str = "TE('%s(%s)'，'OBJNAM,INFORM',1,2,2,'65110',0,0,CHBLK,29)";
			QString str = QString("TE('%s(%s)','%1,%2',1,2,2,'65110',0,0,CHBLK,29)").arg(objnam).arg(inform);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (inform == "" && objnam != "")
		{
			//QString str = "TX('OBJNAM',1,2,2,'65110',0,0,CHBLK,29)";
			QString str = QString("TX('1%',1,2,2,'65110',0,0,CHBLK,29)").arg(objnam);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (inform != "" && objnam == "")
		{
			//QString str = "TX('INF0RM',1,2,2,'65110',0,0,CHBLK,29)";
			QString str = QString("TX('1%',1,2,2,'65110',0,0,CHBLK,29)").arg(inform);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
	}
	else
	{
		for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size(); i++)
		{
			switch (m_pRecFeature->m_Attf.m_vecAttf[i].attl)
			{
			case 102://INFORM
				inform = m_pRecFeature->m_Attf.m_vecAttf[i].atvl;
				break;
			case 116://OBJNAM
				objnam = m_pRecFeature->m_Attf.m_vecAttf[i].atvl;
				break;
			}
		}
		if (inform != "" && objnam != "")
		{
			//QString str = "TE('%s(%s)'，'OBJNAM,INFORM',1,2,2,'65110',0,0,CHBLK,29)";
			QString str = QString("TE('%s(%s)'，'1%,2%',1,2,2,'65110',0,0,CHBLK,29)").arg(objnam).arg(inform);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (inform == "" && objnam != "")
		{
			//QString str = "TX('OBJNAM',1,2,2,'65110',0,0,CHBLK,29)";
			QString str = QString("TX('1%',1,2,2,'65110',0,0,CHBLK,29)").arg(objnam);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (inform != "" && objnam == "")
		{
			//QString str = "TX('INF0RM'1,2,2,'65110',0,0,CHBLK,29)";
			QString str = QString("TX('2%'1,2,2,'65110',0,0,CHBLK,29)").arg(inform);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
	}

	return resultString;
}

string CS57SymbolRules::AISNAV00()
{
	bool chineseFlag = false;
	string resultString = "";
	QString inform = "";
	QString objnam = "";
	QString ninfom = "";
	QString nobjnm = "";
	QString catach = "";
	QString status = "";
	if (chineseFlag)
	{
		for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size(); i++)
		{
			switch (m_pRecFeature->m_Attf.m_vecAttf[i].attl)
			{
			case 102://INFORM
				inform = m_pRecFeature->m_Attf.m_vecAttf[i].atvl;
				break;
			case 116://OBJNAM
				objnam = m_pRecFeature->m_Attf.m_vecAttf[i].atvl;
				break;
			}
		}
		for (int i = 0; i < m_pRecFeature->m_Natf.m_vecNatf.size(); i++)
		{
			switch (m_pRecFeature->m_Natf.m_vecNatf[i].attl)
			{
			case 300://NINFOM
			{
				QTextCodec* codec_16 = QTextCodec::codecForName("UTF-16");
				ninfom = codec_16->toUnicode(m_pRecFeature->m_Natf.m_vecNatf[i].atvl);
				break;
			}
			case 301://NOBJNM
			{
				QTextCodec* codec_16 = QTextCodec::codecForName("UTF-16");
				nobjnm = codec_16->toUnicode(m_pRecFeature->m_Natf.m_vecNatf[i].atvl);
				break;
			}
			default:
				break;
			}
		}
		if (ninfom != "" && nobjnm != "")
		{
			//QString str = "TE('%s(%s)','NOBJNM,NINFOM',2,2,2,'75112',-1,0,CHBLK,21)";
			QString str = QString("TE('%s(%s)','1%,2%',2,2,2,'75112',-1,0,CHBLK,21)").arg(nobjnm).arg(ninfom);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (ninfom == "" && nobjnm != "")
		{
			//QString str = "TX('N0BJNM',2,2,2,'75112',-1,0,CHBLK,21)";
			QString str = QString("TX('1%',2,2,2,'75112',-1,0,CHBLK,21)").arg(nobjnm);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (ninfom != "" && nobjnm == "")
		{
			//QString str = "TX('NINF0M',2,2,2,'75110',-1,0,CHBLK,21)";
			QString str = QString("TX('1%',2,2,2,'75110',-1,0,CHBLK,21)").arg(ninfom);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (inform != "" && objnam != "")
		{
			//QString str = "TE('%s(%s)','OBJNAM,INFORM',2,2,2,'75112',-1,0,CHBLK,21)";
			QString str = QString("TE('%s(%s)','1%,2%',2,2,2,'75112',-1,0,CHBLK,21)").arg(objnam).arg(inform);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (inform == "" && objnam != "")
		{
			//QString str = "TX('OBJNAM',2,2,2,'75112',-1,0,CHBLK,21)";
			QString str = QString("TX('1%',2,2,2,'75112',-1,0,CHBLK,21)").arg(objnam);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (inform != "" && objnam == "")
		{
			//QString str = "TX('INF0RM',2,2,2,'75110',-1,0,CHBLK,21)";
			QString str = QString("TX('1%',2,2,2,'75110',-1,0,CHBLK,21)").arg(inform);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
	}
	else
	{
		for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size(); i++)
		{
			switch (m_pRecFeature->m_Attf.m_vecAttf[i].attl)
			{
			case 102://INFORM
				inform = m_pRecFeature->m_Attf.m_vecAttf[i].atvl;
				break;
			case 116://OBJNAM
				objnam = m_pRecFeature->m_Attf.m_vecAttf[i].atvl;
				break;
			}
		}
		if (inform != "" && objnam != "")
		{
			//QString str = "TE('%s(%s)','OBJNAM,INFORM',2,2,2,'75112',-1,0,CHBLK,21)";
			QString str = QString("TE('%s(%s)','1%,2%',2,2,2,'75112',-1,0,CHBLK,21)").arg(objnam).arg(inform);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (inform == "" && objnam != "")
		{
			//QString str = "TX('OBJNAM',2,2,2,'75112',-1,0,CHBLK,21)";
			QString str = QString("TX('1%',2,2,2,'75112',-1,0,CHBLK,21)").arg(objnam);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (inform != "" && objnam == "")
		{
			//QString str = "TX('INF0RM'2,2,2,'75110',-1,0,CHBLK,21)";
			QString str = QString("TX('1%'2,2,2,'75110',-1,0,CHBLK,21)").arg(inform);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
	}
	QString str = "TX('AIS'3,2,2,'74109',1,0,CHMGD,21)";
	resultString.append(string((const char*)str.toLocal8Bit()));

	return resultString;
}

string CS57SymbolRules::BCNCAR00()
{
	bool chineseFlag = false;
	string resultString = "";
	QString inform = "";
	QString objnam = "";
	QString ninfom = "";
	QString nobjnm = "";
	QString catach = "";
	QString status = "";
	if (chineseFlag)
	{
		for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size(); i++)
		{
			switch (m_pRecFeature->m_Attf.m_vecAttf[i].attl)
			{
			case 102://INFORM
				inform = m_pRecFeature->m_Attf.m_vecAttf[i].atvl;
				break;
			case 116://OBJNAM
				objnam = m_pRecFeature->m_Attf.m_vecAttf[i].atvl;
				break;
			}
		}
		for (int i = 0; i < m_pRecFeature->m_Natf.m_vecNatf.size(); i++)
		{
			switch (m_pRecFeature->m_Natf.m_vecNatf[i].attl)
			{
			case 300://NINFOM
			{
				QTextCodec* codec_16 = QTextCodec::codecForName("UTF-16");
				ninfom = codec_16->toUnicode(m_pRecFeature->m_Natf.m_vecNatf[i].atvl);
				break;
			}
			case 301://NOBJNM
			{
				QTextCodec* codec_16 = QTextCodec::codecForName("UTF-16");
				nobjnm = codec_16->toUnicode(m_pRecFeature->m_Natf.m_vecNatf[i].atvl);
				break;
			}
			default:
				break;
			}
		}
		if (ninfom != "" && nobjnm != "")
		{
			//QString str = "TE('%s(%s)'，'NOBJNM,NINFOM',2,2,2,'74110',-1,0,CHBLK,21)";
			QString str = QString("TE('%s(%s)','1%,2%',2,2,2,'74110',-1,0,CHBLK,21)").arg(nobjnm).arg(ninfom);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (ninfom == "" && nobjnm != "")
		{
			//QString str = "TX('N0BJNM',2,2,2,'74110',-1,0,CHBLK,21)";
			QString str = QString("TX('1%',2,2,2,'74110',-1,0,CHBLK,21)").arg(nobjnm);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (ninfom != "" && nobjnm == "")
		{
			//QString str = "TX('NINF0M',2,2,2,'74110',-1,0,CHBLK,21)";
			QString str = QString("TX('1%',2,2,2,'74110',-1,0,CHBLK,21)").arg(ninfom);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (inform != "" && objnam != "")
		{
			//QString str = "TE('%s(%s)','OBJNAM,INFORM',2,2,2,'74110',-1,0,CHBLK,21)";
			QString str = QString("TE('%s(%s)','1%,2%',2,2,2,'74110',-1,0,CHBLK,21)").arg(objnam).arg(inform);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (inform == "" && objnam != "")
		{
			//QString str = "TX('OBJNAM',2,2,2,'74110',-1,0,CHBLK,21)";
			QString str = QString("TX('1%',2,2,2,'74110',-1,0,CHBLK,21)").arg(objnam);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (inform != "" && objnam == "")
		{
			//QString str = "TX('INF0RM',2,2,2,'74110',-1,0,CHBLK,21)";
			QString str = QString("TX('1%',2,2,2,'74110',-1,0,CHBLK,21)").arg(inform);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
	}
	else
	{
		for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size(); i++)
		{
			switch (m_pRecFeature->m_Attf.m_vecAttf[i].attl)
			{
			case 102://INFORM
				inform = m_pRecFeature->m_Attf.m_vecAttf[i].atvl;
				break;
			case 116://OBJNAM
				objnam = m_pRecFeature->m_Attf.m_vecAttf[i].atvl;
				break;
			}
		}
		if (inform != "" && objnam != "")
		{
			//QString str = "TE('%s(%s)','OBJNAM,INFORM',2,2,2,'75112',-1,0,CHBLK,21)";
			QString str = QString("TE('%s(%s)','1%,2%',2,2,2,'75112',-1,0,CHBLK,21)").arg(objnam).arg(inform);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (inform == "" && objnam != "")
		{
			//QString str = "TX('OBJNAM',2,2,2,'75112',-1,0,CHBLK,21)";
			QString str = QString("TX('1%',2,2,2,'75112',-1,0,CHBLK,21)").arg(objnam);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (inform != "" && objnam == "")
		{
			//QString str = "TX('INF0RM'2,2,2,'75110',-1,0,CHBLK,21)";
			QString str = QString("TX('1%'2,2,2,'75110',-1,0,CHBLK,21)").arg(inform);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
	}

	return resultString;
}

string CS57SymbolRules::BCNISD00()
{
	bool chineseFlag = false;
	string resultString = "";
	QString inform = "";
	QString objnam = "";
	QString ninfom = "";
	QString nobjnm = "";
	QString catach = "";
	QString status = "";
	if (chineseFlag)
	{
		for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size(); i++)
		{
			switch (m_pRecFeature->m_Attf.m_vecAttf[i].attl)
			{
			case 102://INFORM
				inform = m_pRecFeature->m_Attf.m_vecAttf[i].atvl;
				break;
			case 116://OBJNAM
				objnam = m_pRecFeature->m_Attf.m_vecAttf[i].atvl;
				break;
			}
		}
		for (int i = 0; i < m_pRecFeature->m_Natf.m_vecNatf.size(); i++)
		{
			switch (m_pRecFeature->m_Natf.m_vecNatf[i].attl)
			{
			case 300://NINFOM
			{
				QTextCodec* codec_16 = QTextCodec::codecForName("UTF-16");
				ninfom = codec_16->toUnicode(m_pRecFeature->m_Natf.m_vecNatf[i].atvl);
				break;
			}
			case 301://NOBJNM
			{
				QTextCodec* codec_16 = QTextCodec::codecForName("UTF-16");
				nobjnm = codec_16->toUnicode(m_pRecFeature->m_Natf.m_vecNatf[i].atvl);
				break;
			}
			default:
				break;
			}
		}
		if (ninfom != "" && nobjnm != "")
		{
			//QString str = "TE('%s(%s)'，'NOBJNM,NINFOM',2,2,2,'74110',-1,0,CHBLK,21)";
			QString str = QString("TE('%s(%s)','1%,2%',2,2,2,'74110',-1,0,CHBLK,21)").arg(nobjnm).arg(ninfom);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (ninfom == "" && nobjnm != "")
		{
			//QString str = "TX('N0BJNM',2,2,2,'74110',-1,0,CHBLK,21)";
			QString str = QString("TX('1%',2,2,2,'74110',-1,0,CHBLK,21)").arg(nobjnm);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (ninfom != "" && nobjnm == "")
		{
			//QString str = "TX('NINF0M',2,2,2,'74110',-1,0,CHBLK,21)";
			QString str = QString("TX('1%',2,2,2,'74110',-1,0,CHBLK,21)").arg(ninfom);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (inform != "" && objnam != "")
		{
			//QString str = "TE('%s(%s)'，'OBJNAM,INFORM',2,2,2,'74110',-1,0,CHBLK,21)";
			QString str = QString("TE('%s(%s)','1%,2%',2,2,2,'74110',-1,0,CHBLK,21)").arg(objnam).arg(inform);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (inform == "" && objnam != "")
		{
			//QString str = "TX('OBJNAM',2,2,2,'74110',-1,0,CHBLK,21)";
			QString str = QString("TX('1%',2,2,2,'74110',-1,0,CHBLK,21)").arg(objnam);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (inform != "" && objnam == "")
		{
			//QString str = "TX('INF0RM',2,2,2,'74110',-1,0,CHBLK,21)";
			QString str = QString("TX('1%',2,2,2,'74110',-1,0,CHBLK,21)").arg(inform);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
	}
	else
	{
		for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size(); i++)
		{
			switch (m_pRecFeature->m_Attf.m_vecAttf[i].attl)
			{
			case 102://INFORM
				inform = m_pRecFeature->m_Attf.m_vecAttf[i].atvl;
				break;
			case 116://OBJNAM
				objnam = m_pRecFeature->m_Attf.m_vecAttf[i].atvl;
				break;
			}
		}
		if (inform != "" && objnam != "")
		{
			//QString str = "TE('%s(%s)','OBJNAM,INFORM',2,2,2,'74110',-1,0,CHBLK,21)";
			QString str = QString("TE('%s(%s)','1%,2%',2,2,2,'74110',-1,0,CHBLK,21)").arg(objnam).arg(inform);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (inform == "" && objnam != "")
		{
			//QString str = "TX('OBJNAM',2,2,2,'74110',-1,0,CHBLK,21)";
			QString str = QString("TX('1%',2,2,2,'74110',-1,0,CHBLK,21)").arg(objnam);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (inform != "" && objnam == "")
		{
			//QString str = "TX('INF0RM'2,2,2,'74110',-1,0,CHBLK,21)";
			QString str = QString("TX('1%'2,2,2,'74110',-1,0,CHBLK,21)").arg(inform);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
	}

	return resultString;
}

string CS57SymbolRules::BCNLAT00()
{
	bool chineseFlag = false;
	string resultString = "";
	QString inform = "";
	QString objnam = "";
	QString ninfom = "";
	QString nobjnm = "";
	QString catach = "";
	QString status = "";
	if (chineseFlag)
	{
		for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size(); i++)
		{
			switch (m_pRecFeature->m_Attf.m_vecAttf[i].attl)
			{
			case 102://INFORM
				inform = m_pRecFeature->m_Attf.m_vecAttf[i].atvl;
				break;
			case 116://OBJNAM
				objnam = m_pRecFeature->m_Attf.m_vecAttf[i].atvl;
				break;
			}
		}
		for (int i = 0; i < m_pRecFeature->m_Natf.m_vecNatf.size(); i++)
		{
			switch (m_pRecFeature->m_Natf.m_vecNatf[i].attl)
			{
			case 300://NINFOM
			{
				QTextCodec* codec_16 = QTextCodec::codecForName("UTF-16");
				ninfom = codec_16->toUnicode(m_pRecFeature->m_Natf.m_vecNatf[i].atvl);
				break;
			}
			case 301://NOBJNM
			{
				QTextCodec* codec_16 = QTextCodec::codecForName("UTF-16");
				nobjnm = codec_16->toUnicode(m_pRecFeature->m_Natf.m_vecNatf[i].atvl);
				break;
			}
			default:
				break;
			}
		}
		if (ninfom != "" && nobjnm != "")
		{
			//QString str = "TE('%s(%s)'，'NOBJNM,NINFOM',2,2,2,'74110',-1,0,CHBLK,21)";
			QString str = QString("TE('%s(%s)','1%,2%',2,2,2,'74110',-1,0,CHBLK,21)").arg(nobjnm).arg(ninfom);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (ninfom == "" && nobjnm != "")
		{
			//QString str = "TX('N0BJNM',2,2,2,'74110',-1,0,CHBLK,21)";
			QString str = QString("TX('1%',2,2,2,'74110',-1,0,CHBLK,21)").arg(nobjnm);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (ninfom != "" && nobjnm == "")
		{
			//QString str = "TX('NINF0M',2,2,2,'74110',-1,0,CHBLK,21)";
			QString str = QString("TX('1%',2,2,2,'74110',-1,0,CHBLK,21)").arg(ninfom);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (inform != "" && objnam != "")
		{
			//QString str = "TE('%s(%s)'，'OBJNAM,INFORM',2,2,2,'74110',-1,0,CHBLK,21)";
			QString str = QString("TE('%s(%s)','1%,2%',2,2,2,'74110',-1,0,CHBLK,21)").arg(objnam).arg(inform);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (inform == "" && objnam != "")
		{
			//QString str = "TX('OBJNAM',2,2,2,'74110',-1,0,CHBLK,21)";
			QString str = QString("TX('1%',2,2,2,'74110',-1,0,CHBLK,21)").arg(objnam);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (inform != "" && objnam == "")
		{
			//QString str = "TX('INF0RM',2,2,2,'74110',-1,0,CHBLK,21)";
			QString str = QString("TX('1%',2,2,2,'74110',-1,0,CHBLK,21)").arg(inform);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
	}
	else
	{
		for (int i = 0; i < m_pRecFeature->m_Attf.m_vecAttf.size(); i++)
		{
			switch (m_pRecFeature->m_Attf.m_vecAttf[i].attl)
			{
			case 102://INFORM
				inform = m_pRecFeature->m_Attf.m_vecAttf[i].atvl;
				break;
			case 116://OBJNAM
				objnam = m_pRecFeature->m_Attf.m_vecAttf[i].atvl;
				break;
			}
		}
		if (inform != "" && objnam != "")
		{
			//QString str = "TE('%s(%s)','OBJNAM,INFORM',2,2,2,'74110',-1,0,CHBLK,21)";
			QString str = QString("TE('%s(%s)','1%,2%',2,2,2,'74110',-1,0,CHBLK,21)").arg(objnam).arg(inform);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (inform == "" && objnam != "")
		{
			//QString str = "TX('OBJNAM',2,2,2,'74110',-1,0,CHBLK,21)";
			QString str = QString("TX('1%',2,2,2,'74110',-1,0,CHBLK,21)").arg(objnam);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
		else if (inform != "" && objnam == "")
		{
			//QString str = "TX('INF0RM'2,2,2,'74110',-1,0,CHBLK,21)";
			QString str = QString("TX('1%'2,2,2,'74110',-1,0,CHBLK,21)").arg(inform);
			resultString.append(string((const char*)str.toLocal8Bit()));
		}
	}

	return resultString;
}

string CS57SymbolRules::BCNSAW00()
{
	return "";
}
