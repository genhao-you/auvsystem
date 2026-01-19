#pragma once
#include "doc_global.h"
#include <string>
using namespace std;
namespace Doc
{
	class CS57RecFeature;
	class DOC_EXPORT CS57SymbolRules
	{
	public:
		CS57SymbolRules(CS57RecFeature* pFE);
		~CS57SymbolRules();

		typedef enum _S52_MAR_param_t 
		{
			S52_MAR_NONE = 0,    // default
			S52_MAR_SHOW_TEXT = 1,    // view group 23
			S52_MAR_TWO_SHADES = 2,    // flag indicating selection of two depth shades (on/off) [default ON]
			S52_MAR_SAFETY_CONTOUR = 3,    // selected safety contour (meters) [IMO PS 3.6]
			S52_MAR_SAFETY_DEPTH = 4,    // selected safety depth (meters) [IMO PS 3.7]
			S52_MAR_SHALLOW_CONTOUR = 5,    // selected shallow water contour (meters) (optional)
			S52_MAR_DEEP_CONTOUR = 6,    // selected deepwatercontour (meters) (optional)
			S52_MAR_SHALLOW_PATTERN = 7,    // flag indicating selection of shallow water highlight (on/off)(optional) [default OFF]
			S52_MAR_SHIPS_OUTLINE = 8,    // flag indicating selection of ship scale symbol (on/off) [IMO PS 8.4]
			S52_MAR_DISTANCE_TAGS = 9,    // selected spacing of "distance to run" tags at a route (nm)
			S52_MAR_TIME_TAGS = 10,   // selected spacing of time tags at the pasttrack (min)
			S52_MAR_FULL_SECTORS = 11,   // show full length light sector lines
			S52_MAR_SYMBOLIZED_BND = 12,   // symbolized area boundaries

			S52_MAR_SYMPLIFIED_PNT = 13,   // simplified point

			S52_MAR_DISP_CATEGORY = 14,   // display category

			S52_MAR_COLOR_PALETTE = 15,   // color palette

			S52_MAR_NUM = 16    // number of parameters
		} S52_MAR_param_t;

		int			_parseList(const char *str_in, char *buf, int buf_size);
		string		_LITDSN01();
		string		_selSYcol(char *buf, bool bsectr, double valnmr);

		string		getShowCommand(char cs[9]);

		//灯标
		string		LIGHTS06();
		//顶标
		string		TOPMAR01();
		//水深
		string		SOUNDG03();
		string		SNDFRM02(double depth_value_in);
		//障碍物
		string		OBSTRN07();
		string		OBSTRN071();
		//沉船
		string		WRECKS02();
		string		CSQUAPNT01();
		double		_DEPVAL01(double least_depth);
		string		_UDWHAZ03(double depth_value, bool *promote_return);

		//等深线
		string		DEPCNT03();
		//未知
		string		QUALIN01();
		string		QUAPNT02();
		string		SLCONS04();
		string		DATCVR02();
		//未知物标
		string		QUAPOS01();
		string		CSQUALIN01();
		//限制区
		string		RESARE04();
		//深度区
		string		DEPARE03();

		string		RESTRN01();	
		string		RESCSP02(string strRestrn);
		void		_DEPVAL02(float * mLeast_Depth, float * mSebed_Depth);

		static double	S52_getMarinerParam(S52_MAR_param_t param);
		static int		S52_setMarinerParam(S52_MAR_param_t param, double val);

		CS57RecFeature* m_pRecFeature;

		//CDC
		string		ACHARE00();
		string		ACHBRT00();
		string		AISNAV00();
		//与文档不同
		string		BCNCAR00();
		//与文档不同
		string		BCNISD00();
		//与文档不同
		string		BCNLAT00();
		string		BCNSAW00();
		//string		BCNSPP00();
		//string		BERTHS00();
		//string		BOULNE00();
		//string		BOYCAR00();
		//string		BOYISD00();
		//string		BOYLAT00();
		//string		BOYSAW00();
		//string		BOYSPP00();
		//string		BRIDGE00();
		//string		BUAAREP00();
		//string		BUAAREA00();
		//string		BUISGLP00();
		//string		BUISGLA00();
		//string		CAUSWY00();
		//string		CBLOHD00();
		//string		CBLSUB00();
		//string		CGUSTA00();
		//string		CONVYR00();
		//string		CONZNE00();
		//string		CRANES00();
		//string		CTNARE00();
		//string		CTRPNT00();
		//string		CURENT00();
		//string		DAMCON00();
		//string		DEPARE00();
		//string		DEPCNT00();
		//string		DMPGRD00();
		//string		DRGARE00();
		//string		DRYDOC00();
		//string		DWRTPT00();
		//string		DYKCON00();
		//string		EXEZNE00();
		//string		FAIRWY00();
		//string		FERYRT00();
		//string		FLODOC00();
		//string		FOGSIG00();
		//string		FORSTC00();
		//string		FSHFAC00();
		//string		FSHZNE00();
		//string		GATCON00();
		//string		GRIDRN00();
		//string		HRBARE00();
		//string		HRBFAC00();
		//string		HULKES00();
		//string		ICEARE00();
		//string		ICNARE00();
		//string		LIGHTS00();
		//string		LITVES00();
		//string		LNDELV00();
		//string		LNDMRKL00();
		//string		LNDMRKP00();
		//string		LNDRGN00();
		//string		LOCMAG00();
		//string		LOGPON00();
		//string		MAGVARP00();
		//string		MAGVARL00();
		//string		MARCULA00();
		//string		MARCULP00();
		//string		MIPARE00();
		//string		MORFAC00();
		//string		NAVLNE00();
		//string		OBSTRNA00();
		//string		OBSTRNP00();
		//string		OFSPLF00();
		//string		OSPARE00();
		//string		PILBOP00();
		//string		PIPARE00();
		//string		PIPOHD00();
		//string		PIPSOL00();
		//string		PONTON00();
		//string		PRCARE00();
		//string		PRDARE00();
		//string		PYLONS00();
		//string		RADSTA00();
		//string		RAILWY00();
		////string		RDOCALPL00();
		////string		RDOCALP00();
		//string		RDOSTA00();
		//string		RECTRC00();
		//string		RESARE00();
		//string		RIVERS00();
		//string		ROADWYA00();
		//string		ROADWYL00();
		//string		RTPBCN00();
		//string		SBDAREA00();
		//string		SBDAREP00();
		//string		SILTNK00();
		//string		SISTAT00();
		//string		SISTAW00();
		//string		SLCONS00();
		//string		SLOGRD00();
		//string		SOUNDG00();
		//string		SPLARE00();
		//string		SWPARE00();
		//string		TESARE00();
		//string		TS_FEB00();
		//string		TS_PAD00();
		//string		TSELNE00();
		//string		TSSLPT00();
		//string		TUNNEL00();
		//string		UWTROC00();
		//string		VEGATN00();
		//string		VIEWPT00();
		//string		WATTUR00();
		//string		WRECKSA00();
		//string		WRECKSP00();
	};
}
