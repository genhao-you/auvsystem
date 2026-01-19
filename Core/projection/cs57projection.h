#pragma once
#include "xt_global.h"

#define ROPP 2.062648062470958e5
#define RO 5.729577951310823e1
#define PI 3.1415926535897932384626433832795
#define HALFPI 1.5707963267948966192
#define DOUBLEPI 6.283185307179586476925286766559
#define SECONDRAD 4.8481368e-6;
#define HALFTHREEDEGZONE 0.02617993877991494
#define HALFSIXDEGZONE 0.052359877559829887
#define APPROHELFPI 1.570069106 // 89度57分30秒
#define EPS 1.0e-30
#define EPS10  1.e-10
#define EPS7 1.e-10
#define ZERO 1.0e-30

int  fun_sign(double x_in);						//取符号函数
double  fun_dmmssTorad(double dmmss_in);		//D.MMSS—RAD 弧 度 转 换 函  数
double  fun_radTodmmss(double rad_in);			//RAD — DMMSS 转换函数
double  fun_atanh(double x_in);					// 反 双 曲 正 切 函 数

enum Mode
{
	N_POLE = 0,
	S_POLE = 1,
	EQUIT = 2,
	OBLIQ = 3
};

struct pj_opaque
{
	double  sinb0;
	double  cosb0;
	enum Mode mode;
	double R;
};

enum pj_errcode
{
	RIGHT = 0,	// 正确计算
	MCTXYERR1,	// 输入的纬度值大于或等于90，按89度57分30秒计算
	MCTXYERR2,	// 输入的纬度值小于或等于-90，按-89度57分30秒计算
	MCTBLERR1,	// 输入的纵坐标值大于89度57分30秒的MCT投setBaseLine影计算值，按89度57分30秒的计算值计算
	MCTBLERR2,	// 输入的纵坐标值小于-89度57分30秒的MCT投影计算值，按-89度57分30秒的计算值计算
	GNOMXYERR1, // 切点在赤道，纬度大于等89.9度，按89.9计算
	GNOMXYERR2, // 切点在赤道，纬度小于－89.9度，按－89.9计算
	GNOMXYERR3, // 切点在赤道，经度差大于89.9度，按89.9计算
	GNOMXYERR4, // 切点在赤道，经度差小于－89.9度，按－89.9计算
	GNOMXYERR5, // 切点在南极，纬度大于小于－0.1度，按－0.1计算
	GNOMXYERR6, // 切点在北极，纬度大于大于0.1度，按0.1计算
	GNOMXYERR7	// 切点不在南北极和赤道，位置超限，采用收缩纬度范围计算
};

class CS57Projection
{
public:
	CS57Projection();
	~CS57Projection();
public:
	// 设置椭球体参数
	void setSpheroidWGS84();
	void setSpheroidKrasovsky();
	void setParameterEarth(double AxisA, double AxisB);

	// 设置与获取投影名称（枚举值）
	XT::ProjType GetProjectionName();
	void setProjectionName(XT::ProjType name);

	// 投影正反算
	pj_errcode getXY(double lat, double lon, double * x, double *y);//lat,lon,vpy,vpx
	pj_errcode getBL(double x, double y, double *latout, double *lonout);

	// 墨卡托投影参数设置，设置基准纬线,单位弧度
	void setBaseLine(double base);
	double getBaseLine();

	// 高斯投影参数设置
	void setCentralMeridianFromView(double &lon, double &scale);
	void setCentralMeridian(double mMeridian);
	void setThreeZone(bool ThreeZone) { is3Zone = ThreeZone; }
	void setThreeZoneFromScale(double scale);
	void setCrossBandCompute(bool CrossBand) { isCrossBand = CrossBand; }
	int getZoneNum(double lon);
	int getZoneNum();
	bool is3ZoneDiff();

	// 日晷投影参数设置
	void setAzimuthPoint(double lat0_in, double lon0_in);
	void getAzimuthPoint(double *lat0_in, double *lon0_in);

protected:

	// 投影函数指针
	void(CS57Projection::*_getxy)(double lat, double lon, double * x, double *y);
	void(CS57Projection::*_getbl)(double x, double y, double * lat, double *lon);

	// 墨卡托投影正反算

	void getMctXY(double lat_in, double lon_in, double* x, double* y);
	void getMctBL(double x, double y, double* lat_out, double* lon_out);


	void getGaussXY(double lat, double lon, double *x, double *y);
	void getGaussBL(double x, double y, double *latout, double *lonout);

	// 日晷投影

	void getGnomonicXY(double lat, double lon, double *x, double *y);
	void getGnomonicBL(double x, double y, double *latout, double *lonout);

	//void getAziequdisXY(double lat, double lon, double *x, double *y);
	//void getAziequdisBL(double x, double y, double *latout, double *lonout);

	//void getAlbequareXY(double lat, double lon, double *x, double *y);
	//void getAlbequareBL(double x, double y, double *latout, double *lonout);

	//void getPovecegraXY(double lat, double lon, double *x, double *y);
	//void getPovecegraBL(double x, double y, double *latout, double *lonout);

	//void getTramctXY(double lat, double lon, double *x, double *y);
	//void getTramctBL(double x, double y, double *latout, double *lonout);

protected:
	double mSemiAxisA, mSemiAxisB;	// 地球椭球体长轴、短轴半径
	double mE2;						// 第一偏心率
	double mAlf;					// 第一扁率
	double mE;						// 第一偏心率的开方
	double mE2p;					// 第二偏心率
	double m_n;						// 大地计算辅助量之一

	double mBaseLine;				// 基准纬线	
	double mMctMaxX;				// 墨卡托投影纵轴最大值（89.59） 
	double lr0;						// 基准纬线半径
	double mCentralMeridian = 0;	// 中央经线
	int mZoomNum;					// 带号
	double mLength90;				// 极点子午线弧长

	bool is3Zone = true;			// 是否ThreeDeg带
	bool isCrossBand = false;		// 是否跨带计算,默认为不跨带，只计算本带坐标，然后再归化到原坐标

	double lat0;					//	方位投影切点
	double lon0;					//  方位投影切点
	pj_opaque azim0;

	double stdlat1;					// 圆锥投影标准纬线1
	double stdlat2;					// 圆锥投影标准纬线2

	bool northp;					// the pole which is the center of projection(true means north, false means south).

	// 子午线弧长反解参数
	double m0, m2, m4, m6, m8, A0, A1, A2, A3, A4;
	// 子午线弧长正解参数（幂级数）
	double A, B, C, D, E, F, G;

	char m_iProjectionID;
	XT::ProjType m_eCurProjection;
	pj_errcode mCurErrCode = RIGHT;
protected:
	double fun_lr(double lat_in);		// 纬圈半径
	double fun_invq(double q0);			// 等量纬度反解
	double fun_q(double lat);			// 等理纬度正解
	double fun_rp(double lat);			// 动径，(椭球面上任一点至球心的距离)
public:
	void setConicStandLats(double lat1, double lat2);
	//void setNorthPole(bool northp);

	// 计算子午圈曲率半径
	double getMerdianRadius(double lat_in);
	// 子午线弧长正解
	double getMeridianArcLength(double lat);
	double getMeridianArcRad(double s);
	//恒向线大地距离正反算
	// 恒向线大地距离正算，已知经纬度（弧度）坐标，方向角，距离，求点
	void getDistanceRhumbForward(double lat1, double lon1, double ang, double S, double * lat2, double *lon2);
	//void GetDistanceRhumbForwardMct(double lat1, double lon1, double ang, double S, double * lat2, double *lon2);
	// 恒向线大地距离反算，已知两点经纬度（弧度）坐标，求方向角，距离
	void getDistanceRhumbReverse(double lat1, double lon1, double lat2, double lon2, double * ang, double *S);
	//void GetDistanceRhumbReverseMct(double lat1, double lon1, double lat2, double lon2, double * ang, double *S);
	// 大地距离正反算
	// 大地距离正算，已知起点经纬度（弧度）坐标，起方向角，求点和点的方向角
	void getDistanceForward(double lat1, double lon1, double ang1, double S, double * lat2, double *lon2, double *ang2);
	// 大地距离反算，已知两点经纬度（弧度）坐标，求两点方向角，和距离
	void getDistanceReverse(double lat1, double lon1, double lat2, double lon2, double * ang1, double *S, double *ang2);
private:
	/*************************************************
	Function: solve_quadratic_equation
	Description: 求一元二次方程(a*x^2 + b*x + c = 0)的所有实数根
	Input: 方程的系数 p = {c, b, a}
	Output: 方程的所有实数根x
	Return: 实数根的个数
	Author: 枫箫
	Version: 1.0
	Date: 2017.7.8
	Others: 如有疑问、意见或建议, 请多多交流, 多多指教!邮箱:gxb31415926@163.com
	*************************************************/
	int solve_quadratic_equation(double p[], double x[]);

	/*************************************************
	Function: solve_cubic_equation
	Description: 盛金公式求一元三次方程(a*x^3 + b*x^2 + c*x + d = 0)的所有实数根
					A = b * b - 3.0 * a * c;
					B = b * c - 9.0 * a * d;
					C = c * c - 3.0 * b * d;
					(1)当A = B = 0时，方程有一个三重实根
					(2)当Δ = B^2－4AC>0时，方程有一个实根和一对共轭虚根
					(3)当Δ = B^2－4AC = 0时，方程有三个实根，其中有一个两重根
					(4)当Δ = B^2－4AC<0时，方程有三个不相等的实根
	Input: 方程的系数 p = {d, c, b, a}
	Output: 方程的所有实数根x
	Return: 实数根的个数
	Author: 枫箫
	Version: 1.0
	Date: 2017.7.8
	Others: 如有疑问、意见或建议, 请多多交流, 多多指教!邮箱:gxb31415926@163.com
	*************************************************/
	int solve_cubic_equation(double p[], double x[]);

	/*************************************************
	Function: solve_quartic_equation
	Description: 费拉里法求一元四次方程(a*x^4 + b*x^3 + c*x^2 + d*x + e = 0)的所有实数根
	Input: 方程的系数 p = {e, d, c, b, a}
	Output: 方程的所有实数根x
	Return: 实数根的个数
	Author: 枫箫
	Version: 1.0
	Date: 2017.7.8
	Others: 如有疑问、意见或建议, 请多多交流, 多多指教!邮箱:gxb31415926@163.com
	*************************************************/
	int solve_quartic_equation(double p[], double x[]);
};