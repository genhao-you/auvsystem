#include "stdafx.h"
#include "cs57projection.h"
#include <qmath.h>
#include <algorithm>
#include <QMessageBox>	
using namespace std;

CS57Projection::CS57Projection()
{
	//Krasovsky
	//this->mSemiAxisA = 6378245.;						//6378137.;
	//this->mSemiAxisB = 6356863.0188;				//6356752.31424518;;
	//WGS84
	mSemiAxisA = 6378137.;						//6378245.;
	mSemiAxisB = 6356752.31424518;				//;6356863.0188;

	mE2 = 1 - mSemiAxisB * mSemiAxisB / mSemiAxisA / mSemiAxisA;					// 第一偏心率

	mAlf = (mSemiAxisA - mSemiAxisB) / mSemiAxisA;									// 第一扁率
	mE2p = mSemiAxisA * mSemiAxisA / mSemiAxisB / mSemiAxisB - 1.;					// 第二偏心率

	mE = sqrt(mE2);
	m_n = (mSemiAxisA - mSemiAxisB) / (mSemiAxisA + mSemiAxisB);


	lr0 = mSemiAxisA;
	mBaseLine = 0;
	lat0 = 0.0;
	lon0 = 0.0;

	double mE4 = mE2 * mE2;
	double mE6 = mE4 * mE2;
	double mE8 = mE6 * mE2;
	double mE10 = mE8 * mE2;
	double mE12 = mE10 * mE2;
	// 子午线弧长反解参数

	m0 = mSemiAxisA * (1. - mE2);
	m2 = 3. * mE2*m0 / 2.;
	m4 = 5. * mE2*m2 / 4.;
	m6 = 7. * mE2*m4 / 6.;
	m8 = 9. * mE2*m6 / 8.;
	A0 = m0 + m2 / 2. + 3. * m4 / 8. + 5. * m6 / 16. + 35. * m8 / 128.;
	A1 = 3. / 4.*mE2 + 45. / 64. * mE4 + 175. / 256. * mE6 + 11025. / 16384.* mE8;
	A2 = -21. / 32. * mE4 - 277. / 192. * mE6 - 18549. / 8912. * mE8;
	A3 = 151. / 192.*mE6 + 5059. / 2048. * mE8;
	A4 = -881. / 1024. * mE8;

	// 子午线弧长正解参数（幂级数）
	A = 1. + 3. / 4.*mE2 + 45. / 64.*mE4 + 175. / 256. *mE6 + 11025. / 16384. *mE8 + 43659. / 65536.*mE10 + 693693. / 1048576.*mE12;
	B = A - 1.;// 3. / 4.*mE2 + 45. / 64.*mE4 + 175. / 256. *mE6 + 11025. / 16384. *mE8 + 43659. / 65536.*mE10 + 693693. / 1048576.*mE12
	C = 15. / 32.*mE4 + 175. / 384.*mE6 + 3675. / 8192.* mE8 + 14553. / 32768.*mE10 + 231231. / 524288.* mE12;
	D = 35. / 96. *mE6 + 735. / 2048.*mE8 + 14553. / 40960.*mE10 + 231231. / 655360.*mE12;
	E = 315. / 1024.*mE8 + 6237. / 20480.*mE10 + 99099. / 327680.*mE12;
	F = 693. / 2560.*mE10 + 11011. / 40960.*mE12;
	G = 1001. / 4096.*mE12;

	mLength90 = getMeridianArcLength(PI / 2.);

	_getxy = &CS57Projection::getMctXY;
	_getbl = &CS57Projection::getMctBL;
	m_eCurProjection = XT::MCT;
	double y;
	getXY(fun_dmmssTorad(89.5730), 0.0, &mMctMaxX, &y);
}

CS57Projection::~CS57Projection()
{}

void CS57Projection::setProjectionName(XT::ProjType name)
{
	switch (name)
	{
	case XT::MCT:											// 墨卡托投影
		_getxy = &CS57Projection::getMctXY;
		_getbl = &CS57Projection::getMctBL;
		m_eCurProjection = XT::MCT;
		break;
	case XT::GAUSS:											// 高斯克吕格投影

		_getxy = &CS57Projection::getGaussXY;
		_getbl = &CS57Projection::getGaussBL;
		m_eCurProjection = XT::GAUSS;
		break;
	case XT::GNOMONIC:										// 日晷投影（球心透视方位投影）

		_getxy = &CS57Projection::getGnomonicXY;
		_getbl = &CS57Projection::getGnomonicBL;
		m_eCurProjection = XT::GNOMONIC;
		break;
		//case AZIEQUDIS:										// 等距方位投影
		//	_getxy = &CS57Projection::GetAziequdisXY;
		//	_getbl = &CS57Projection::GetAziequdisBL;
		//	eCurProjection = AZIEQUDIS;
		//	break;
		//case ALBEQUARE:										// 等面积圆锥投影
		//	_getxy = &CS57Projection::GetAlbequareXY;
		//	_getbl = &CS57Projection::GetAlbequareBL;
		//	eCurProjection = ALBEQUARE;
		//	break;
		//case POLSTEGRA:										// 极地立体投影
		//	_getxy = &CS57Projection::GetPovecegraXY;
		//	_getbl = &CS57Projection::GetPovecegraBL;
		//	eCurProjection = POLSTEGRA;
		//	break;
		//case TRAMCT:										// 横墨卡投影

		//	_getxy = &CS57Projection::GetTramctXY;
		//	_getbl = &CS57Projection::GetTramctBL;
		//	eCurProjection = TRAMCT;
		//	break;
	default:											// 墨卡托投影

		_getxy = &CS57Projection::getMctXY;
		_getbl = &CS57Projection::getMctBL;
		m_eCurProjection = XT::MCT;
		break;
	}
}
//lat,lon,vpy,vpx
pj_errcode CS57Projection::getXY(double lat, double lon, double * x, double *y)
{
	(this->*_getxy)(lat, lon, x, y);
	return mCurErrCode;
}

pj_errcode CS57Projection::getBL(double x, double y, double *latout, double *lonout)
{
	(this->*_getbl)(x, y, latout, lonout);
	return mCurErrCode;
}

void CS57Projection::setParameterEarth(double AxisA, double AxisB)
{
	mSemiAxisA = AxisA;
	mSemiAxisB = AxisB;
	mE2 = 1 - mSemiAxisB * mSemiAxisB / mSemiAxisA / mSemiAxisA;					// 第一偏心率

	mAlf = (mSemiAxisA - mSemiAxisB) / mSemiAxisA;									// 第一扁率
	mE2p = mSemiAxisA * mSemiAxisA / mSemiAxisB / mSemiAxisB - 1.;					// 第二偏心率

	this->mE = sqrt(mE2);
	lr0 = mSemiAxisA;
	double mE4 = mE2 * mE2;
	double mE6 = mE4 * mE2;
	double mE8 = mE6 * mE2;
	double mE10 = mE8 * mE2;
	double mE12 = mE10 * mE2;
	// 子午线弧长反解参数

	m0 = mSemiAxisA * (1. - mE2);
	m2 = 3. * mE2*m0 / 2.;
	m4 = 5. * mE2*m2 / 4.;
	m6 = 7. * mE2*m4 / 6.;
	m8 = 9. * mE2*m6 / 8.;
	A0 = m0 + m2 / 2. + 3. * m4 / 8. + 5. * m6 / 16. + 35. * m8 / 128.;
	A1 = 3. / 4.*mE2 + 45. / 64. * mE4 + 175. / 256. * mE6 + 11025. / 16384.* mE8;
	A2 = -21. / 32. * mE4 - 277. / 192. * mE6 - 18549. / 8912. * mE8;
	A3 = 151. / 192.*mE6 + 5059. / 2048. * mE8;
	A4 = -881. / 1024. * mE8;

	// 子午线弧长正解参数（幂级数）
	A = 1. + 3. / 4.*mE2 + 45. / 64.*mE4 + 175. / 256. *mE6 + 11025. / 16384. *mE8 + 43659. / 65536.*mE10 + 693693. / 1048576.*mE12;
	B = A - 1.;// 3. / 4.*mE2 + 45. / 64.*mE4 + 175. / 256. *mE6 + 11025. / 16384. *mE8 + 43659. / 65536.*mE10 + 693693. / 1048576.*mE12
	C = 15. / 32.*mE4 + 175. / 384.*mE6 + 3675. / 8192.* mE8 + 14553. / 32768.*mE10 + 231231. / 524288.* mE12;
	D = 35. / 96. *mE6 + 735. / 2048.*mE8 + 14553. / 40960.*mE10 + 231231. / 655360.*mE12;
	E = 315. / 1024.*mE8 + 6237. / 20480.*mE10 + 99099. / 327680.*mE12;
	F = 693. / 2560.*mE10 + 11011. / 40960.*mE12;
	G = 1001. / 4096.*mE12;

	mLength90 = getMeridianArcLength(PI / 2.);
}

void CS57Projection::setSpheroidWGS84()
{
	mSemiAxisA = 6378137.;
	mSemiAxisB = 6356752.31424518;
	mE2 = 1 - mSemiAxisB * mSemiAxisB / mSemiAxisA / mSemiAxisA;					// 第一偏心率

	mAlf = (mSemiAxisA - mSemiAxisB) / mSemiAxisA;									// 第一扁率
	mE2p = mSemiAxisA * mSemiAxisA / mSemiAxisB / mSemiAxisB - 1.;					// 第二偏心率

	mE = sqrt(mE2);
	m_n = (mSemiAxisA - mSemiAxisB) / (mSemiAxisA + mSemiAxisB);
	lr0 = mSemiAxisA;
	double mE4 = mE2 * mE2;
	double mE6 = mE4 * mE2;
	double mE8 = mE6 * mE2;
	double mE10 = mE8 * mE2;
	double mE12 = mE10 * mE2;
	// 子午线弧长反解参数

	m0 = mSemiAxisA * (1. - mE2);
	m2 = 3. * mE2*m0 / 2.;
	m4 = 5. * mE2*m2 / 4.;
	m6 = 7. * mE2*m4 / 6.;
	m8 = 9. * mE2*m6 / 8.;
	A0 = m0 + m2 / 2. + 3. * m4 / 8. + 5. * m6 / 16. + 35. * m8 / 128.;
	A1 = 3. / 4.*mE2 + 45. / 64. * mE4 + 175. / 256. * mE6 + 11025. / 16384.* mE8;
	A2 = -21. / 32. * mE4 - 277. / 192. * mE6 - 18549. / 8912. * mE8;
	A3 = 151. / 192.*mE6 + 5059. / 2048. * mE8;
	A4 = -881. / 1024. * mE8;

	// 子午线弧长正解参数（幂级数）
	A = 1. + 3. / 4.*mE2 + 45. / 64.*mE4 + 175. / 256. *mE6 + 11025. / 16384. *mE8 + 43659. / 65536.*mE10 + 693693. / 1048576.*mE12;
	B = A - 1.;// 3. / 4.*mE2 + 45. / 64.*mE4 + 175. / 256. *mE6 + 11025. / 16384. *mE8 + 43659. / 65536.*mE10 + 693693. / 1048576.*mE12
	C = 15. / 32.*mE4 + 175. / 384.*mE6 + 3675. / 8192.* mE8 + 14553. / 32768.*mE10 + 231231. / 524288.* mE12;
	D = 35. / 96. *mE6 + 735. / 2048.*mE8 + 14553. / 40960.*mE10 + 231231. / 655360.*mE12;
	E = 315. / 1024.*mE8 + 6237. / 20480.*mE10 + 99099. / 327680.*mE12;
	F = 693. / 2560.*mE10 + 11011. / 40960.*mE12;
	G = 1001. / 4096.*mE12;

	mLength90 = getMeridianArcLength(PI / 2.); // 极子午线弧长
}

void CS57Projection::setSpheroidKrasovsky()
{
	// Krasovsky
	mSemiAxisA = 6378245.;
	mSemiAxisB = 6356863.0188;
	mE2 = 1 - mSemiAxisB * mSemiAxisB / mSemiAxisA / mSemiAxisA;					// 第一偏心率

	mAlf = (mSemiAxisA - mSemiAxisB) / mSemiAxisA;									// 第一扁率
	mE2p = mSemiAxisA * mSemiAxisA / mSemiAxisB / mSemiAxisB - 1.;					// 第二偏心率

	mE = sqrt(mE2);
	m_n = (mSemiAxisA - mSemiAxisB) / (mSemiAxisA + mSemiAxisB);
	lr0 = mSemiAxisA;
	double mE4 = mE2 * mE2;
	double mE6 = mE4 * mE2;
	double mE8 = mE6 * mE2;
	double mE10 = mE8 * mE2;
	double mE12 = mE10 * mE2;
	// 子午线弧长反解参数

	m0 = mSemiAxisA * (1. - mE2);
	m2 = 3. * mE2*m0 / 2.;
	m4 = 5. * mE2*m2 / 4.;
	m6 = 7. * mE2*m4 / 6.;
	m8 = 9. * mE2*m6 / 8.;
	A0 = m0 + m2 / 2. + 3. * m4 / 8. + 5. * m6 / 16. + 35. * m8 / 128.;
	A1 = 3. / 4.*mE2 + 45. / 64. * mE4 + 175. / 256. * mE6 + 11025. / 16384.* mE8;
	A2 = -21. / 32. * mE4 - 277. / 192. * mE6 - 18549. / 8912. * mE8;
	A3 = 151. / 192.*mE6 + 5059. / 2048. * mE8;
	A4 = -881. / 1024. * mE8;

	// 子午线弧长正解参数（幂级数）
	A = 1. + 3. / 4.*mE2 + 45. / 64.*mE4 + 175. / 256. *mE6 + 11025. / 16384. *mE8 + 43659. / 65536.*mE10 + 693693. / 1048576.*mE12;
	B = A - 1.;// 3. / 4.*mE2 + 45. / 64.*mE4 + 175. / 256. *mE6 + 11025. / 16384. *mE8 + 43659. / 65536.*mE10 + 693693. / 1048576.*mE12
	C = 15. / 32.*mE4 + 175. / 384.*mE6 + 3675. / 8192.* mE8 + 14553. / 32768.*mE10 + 231231. / 524288.* mE12;
	D = 35. / 96. *mE6 + 735. / 2048.*mE8 + 14553. / 40960.*mE10 + 231231. / 655360.*mE12;
	E = 315. / 1024.*mE8 + 6237. / 20480.*mE10 + 99099. / 327680.*mE12;
	F = 693. / 2560.*mE10 + 11011. / 40960.*mE12;
	G = 1001. / 4096.*mE12;

	mLength90 = getMeridianArcLength(PI / 2.); // 极子午线弧长
}

double CS57Projection::getMerdianRadius(double lat_in)  // 计算子午圈曲率半径

{
	double sinb, s, sss;
	sinb = sin(lat_in);
	s = 1. - mE2 * sinb*sinb;
	sss = s * s*s;
	return mSemiAxisA * (1. - mE2) / sqrt(sss);
}

XT::ProjType CS57Projection::GetProjectionName()
{
	return m_eCurProjection;
}

// 等量纬度正解
/***************************************************************C
C                  等  量  纬  度  函  数
C                        王    斌
C                       1990-12-15
C       输入参数: LATITUDE  纬度数(弧度)
C       输出参数: FUN_Q 等量纬度 (弧度)
C       调    用: Q=FUN_Q(LATITUDE)
C***************************************************************/
double CS57Projection::fun_q(double lat)
{

	double sinb;
	sinb = sin(lat);
	return fun_atanh(sinb) - mE * fun_atanh(sinb*mE);
}

/***************************************************************
C            动        径 (椭球面上任一点至球心的距离)
C                        王    斌

C                       1990-12-29
C       输入参数: B  纬度数(弧度)
C       输出参数: FUN_RP          动径 (米)
C       调    用: RP=FUN_RP(B)
C***************************************************************/
double CS57Projection::fun_rp(double lat_in)
{
	double sinb, sinb2;
	sinb = sin(lat_in);
	sinb2 = sinb * sinb;
	return mSemiAxisA * sqrt((1. - mE2 * sinb2*(2. - mE2))
		/ (1. - mE2 * sinb2));
}

/***************************************************************C
C               等  量  纬  度  反  解  函  数
C                        王    斌
C                       1990-12-15
C       输入参数: FUN_Q 等量纬度 (弧度)
C       输出参数: LATITUDE  纬度数(弧度)
C       调    用: Q=FUN_invQ(LATITUDE)
C***************************************************************/

double CS57Projection::fun_invq(double q0)
{
	double q1, q10, b0, dq;
	q10 = q0;
	b0 = asin(tanh(q0));
	q1 = fun_q(b0);
	dq = q0 - q1;
	while (fabs(dq) > EPS10)
	{
		q10 = q10 + dq;
		b0 = asin(tanh(q10));// inverse
		q1 = fun_q(b0);	     // solution 
		dq = q0 - q1;
	};
	return(b0);

}

void CS57Projection::getMctXY(double lat_in, double lon_in, double *x, double *y)
{

	mCurErrCode = RIGHT;
	if (lat_in > APPROHELFPI)
	{
		lat_in = APPROHELFPI;
		mCurErrCode = MCTXYERR1;
	}
	if (lat_in < -APPROHELFPI)
	{
		lat_in = -APPROHELFPI;
		mCurErrCode = MCTXYERR2;
	}
	*x = fun_q(lat_in) * lr0;
	*y = (lon_in - mCentralMeridian) * lr0;
}

void CS57Projection::getMctBL(double x, double y, double *lat_out, double *lon_out)
{
	mCurErrCode = RIGHT;
	if (x > mMctMaxX)
	{
		x = mMctMaxX;
		mCurErrCode = MCTBLERR1;
	}
	if (x < -mMctMaxX)
	{
		x = -mMctMaxX;
		mCurErrCode = MCTBLERR2;
	}
	*lat_out = fun_invq(x / lr0);
	*lon_out = y / lr0 + mCentralMeridian;
}

void CS57Projection::setThreeZoneFromScale(double scale)
{
	if (scale < 25000)
		is3Zone = true;
	else
		is3Zone = false;
}

void CS57Projection::setAzimuthPoint(double lat0_in, double lon0_in)
{
	// TODO: 在此处添加实现代码.
	while (lon0_in > PI) lon0_in -= DOUBLEPI;
	while (lon0_in < -PI) lon0_in += DOUBLEPI; ;
	lat0 = lat0_in;
	lon0 = lon0_in;

	if (fabs(fabs(lat0) - PI / 2.) < EPS10)
	{
		azim0.mode = lat0 < 0. ? S_POLE : N_POLE;
	}
	else if (fabs(lat0) < EPS10)
	{
		azim0.mode = EQUIT;
	}
	else
		azim0.mode = OBLIQ;
	azim0.sinb0 = sin(lat0);
	azim0.cosb0 = cos(lat0);

	azim0.R = fun_rp(lat0);
}
/***************************************************************C
C                   纬    圈    半    径
C                        王    斌
C                       1990-12-29
C       输入参数: B  纬度数(弧度)
C       输出参数: FUN_LR 纬圈曲率半径 (米)
C       调    用: LR=FUN_LR(B)
C***************************************************************/

double CS57Projection::fun_lr(double lat_in)
{
	double cosb, sinb;
	cosb = cos(lat_in);
	sinb = sin(lat_in);
	return mSemiAxisA * cosb / sqrt(1. - mE2 * sinb*sinb);
}

void CS57Projection::setBaseLine(double base)
{
	mCentralMeridian = 0.0;
	mBaseLine = base;
	lr0 = fun_lr(mBaseLine);
	if (m_eCurProjection == XT::MCT)
	{
		double y;
		getXY(fun_dmmssTorad(89.5730), 0.0, &mMctMaxX, &y);
	}
}

double CS57Projection::getBaseLine()
{
	return mBaseLine;
}

/*****************************************************************
						 //取符号函数

	输入参数: 实型数

	输出参数: FUN_SIGN (X—IIN)
*****************************************************************/

int fun_sign(double x_in)   /*已校对；*/
{
	if (x_in > 0) return 1;
	else if (x_in < 0) return -1;
	else return 0;
}
/******************************************************************
			D.MMSS—RAD 弧 度 转 换 函  数

						王    斌
					   1996-07-12
	输入参数: DMMSS 以实型数形式表示的角度值

	输出参数: FUN_RAD (弧度)
******************************************************************/

double fun_dmmssTorad(double dmmss_in) /*已校对；*/

{
	int sign;
	if (dmmss_in > 0) sign = 1;
	else if (dmmss_in < 0) sign = -1;
	else return dmmss_in;

	double a, d0, mm0, ss0, ss, mm, b;
	a = fabs(dmmss_in) + 1.39e-10;
	mm = modf(a, &d0);
	ss = modf(mm*100.0, &b);

	mm0 = (b);
	ss0 = ss * 100.0;

	QString str1, str2;
	if (mm0 >= 60)
	{
		str1 = QString::fromLocal8Bit("fun_dmmssTorad错误报告：");
		str2 = QString::fromLocal8Bit("DMMSS中，小数点后两位为分，不能大于等于60，数据错！");
		QMessageBox::critical(
			NULL, str1,
			str2,
			QMessageBox::Ok | QMessageBox::Default,
			QMessageBox::Cancel | QMessageBox::Escape, 0);
	}
	if (ss0 >= 60)
	{
		str1 = QString::fromLocal8Bit("fun_dmmssTorad错误报告：");
		str2 = QString::fromLocal8Bit("DMMSS中，小数点三位后为秒，不能大于等于60，数据错！");
		QMessageBox::critical(NULL, str1, str2,
			QMessageBox::Ok | QMessageBox::Default,
			QMessageBox::Cancel | QMessageBox::Escape, 0);
	}

	//if (mm0 >= 60) MessageBox(NULL, "DMMSS中，小数点后两位为分，不能大于等于60，数据错！", "错误报告", MB_OK);
	//if (ss0 >= 60) MessageBox(NULL, , "错误报告", MB_OK);
	return	(d0*3600. + mm0 * 60. + ss0) / ROPP * sign;

}

/***************************************************************
C                RAD — DMMSS 转换函数
C                        王    斌
C                       1996-07-12
C       输入参数: RAD (弧度)
C       输出参数: FUN_DMMSS 以实型数形式表示的角度值(DDD.MMSSss)
C***************************************************************/


double fun_radTodmmss(double rad_in)  /*已校对；*/
{
	double a, d, d0, mm, dmm, ss, sign, pos;
	sign = fun_sign(rad_in);
	a = fabs(rad_in);
	d = modf(a*RO + 1.39e-6, &pos);
	d0 = pos;
	mm = modf(d*60., &pos);
	dmm = (pos) / 100.;
	ss = mm * 60. / 10000.;
	return (d0 + dmm + ss)*sign;
}

/***************************************************************
C                 反 双 曲 正 切 函 数
C                        王    斌
C                       1990-12-15
C***************************************************************/

double fun_atanh(double x_in)
{
	if ((x_in > -1.) && (x_in < 1.))
		return 0.5*log((1. + x_in) / (1. - x_in));
	else
	{
		QString str1 = QString::fromLocal8Bit("fun_atanh反双曲正切函数错误报告：");
		QString str2 = QString::fromLocal8Bit("输入参数大于等于1或小于等于－1");
		QMessageBox::critical(NULL, str1, str2,
			QMessageBox::Ok | QMessageBox::Default,
			QMessageBox::Cancel | QMessageBox::Escape, 0);
		//MessageBox(NULL, "\n error x_in of fun_atanh \n", "", MB_OK);
		return 0;
	}
}


/************高斯投影正算函数*************************
	输入 : double a ,double b，经纬度，单位为弧度
	输出 :(x,y)为高斯平面坐标,y加上了500000常量
	返回：none
	注：本算法参考自
	百度文库  https://wenku.baidu.com/view/8e5b5064590216fc700abb68a98271fe900eaf68.html?rec_flag=default&sxts=1566791043671
	高斯投影正反算公式,直接依据空间立体三角函数关系得出结果
	本算法参考自：http://www.doc88.com/p-3921624524439.html　并更改了文中多处印刷错误
*****************************************************/
void CS57Projection::getGaussXY(double lat, double lon, double *xOut, double *yOut)
{
	// TODO: 在此处添加实现代码

	double lonDeg = lon * RO;
	int curZoomNum;
	double mCurCentralMeridian;
	double dtY;
	if (!isCrossBand)  // 如果不是跨带计算，即本带计算，首先要计算当前点所在的带号
	{
		if (is3Zone)
		{
			if (lonDeg < 0)
			{
				curZoomNum = (int)(lonDeg - 1.5) / 3;
			}
			else
			{
				curZoomNum = (int)(lonDeg + 1.5) / 3;
			}

			mCurCentralMeridian = (double)curZoomNum * 3. / RO;
		}
		else
		{
			if (lonDeg < 0)
			{
				curZoomNum = (int)(lonDeg / 6) - 1;
				mCurCentralMeridian = (double)(curZoomNum * 6 + 3) / RO;
			}
			else
			{
				curZoomNum = (int)(lonDeg / 6) + 1;
				mCurCentralMeridian = ((double)curZoomNum * 6. - 3.) / RO;
			}
		}
		dtY = (mCurCentralMeridian - mCentralMeridian) * mSemiAxisA;
	}
	else
	{
		mCurCentralMeridian = mCentralMeridian;
		dtY = 0;
	}
	double X = getMeridianArcLength(lat);
	if (abs(lat) >= PI / 2.)
	{
		*xOut = X;
		*yOut = dtY;
		return;
	}
	double cosb = cos(lat);
	double cosb2 = cosb * cosb;
	double cosb3 = cosb * cosb2;
	double cosb4 = cosb2 * cosb2;
	double cosb5 = cosb4 * cosb;
	double sinb = sin(lat);
	double sinb2 = sinb * sinb;
	double tanb = tan(lat);
	double tanb2 = tanb * tanb;
	double tanb4 = tanb2 * tanb2;
	double N = mSemiAxisA / sqrt(1 - mE2 * sinb*sinb);
	double a1 = N * cosb;
	double a2 = N * sinb * cosb / 2.;
	double a3 = N * cosb3 * (1. - tanb * tanb + mE2p * cosb2) / 6.;
	double a4 = N * sinb * cosb3 * (5. - tanb2 + 9. * mE2p * cosb2 + 4. * mE2p * mE2p * cosb4) / 24.;
	double a5 = N * cosb5 * (5 - 18.*tanb2 + tanb4 + 14. * mE2p * cosb2 - 58. * mE2p * sinb2) / 120.;
	double a6 = N * sinb * cosb5*(61. - 58.* tanb2 + tanb4) / 720.;
	double l = lon - mCurCentralMeridian;
	double l2 = l * l;
	double l3 = l2 * l;
	double l4 = l3 * l;
	double l5 = l4 * l;
	double l6 = l5 * l;

	*xOut = X + a2 * l2 + a4 * l4 + a6 * l6;
	*yOut = a1 * l + a3 * l3 + a5 * l5 + dtY;
}

///**************高斯反算函数**********************
//	输入 : (x,y)为高斯平面坐标

//	输出 ：经纬度，单位为弧度
//	*返回：none
//	注：本算法参考自
//	百度文库  https ://wenku.baidu.com/view/8e5b5064590216fc700abb68a98271fe900eaf68.html?rec_flag=default&sxts=1566791043671
//	高斯投影正反算公式,直接依据空间立体三角函数关系得出结果
//************************************************/
void CS57Projection::getGaussBL(double x, double y, double *latout, double *lonout)
{

	double y0;
	double tmpy;
	double tmpL;
	double mCurCentralMeridian;
	double curZoomNum;

	y0 = mCentralMeridian * mSemiAxisA;
	tmpy = y + y0;
	tmpL = tmpy / mSemiAxisA * RO;
	if (!isCrossBand)
	{
		if (is3Zone)
		{
			if (tmpL < 0)
			{
				curZoomNum = (int)(tmpL - 1.5) / 3;
			}
			else
			{
				curZoomNum = (int)(tmpL + 1.5) / 3;
			}
			mCurCentralMeridian = (double)curZoomNum * 3. / RO;
		}
		else
		{
			if (tmpL < 0)
			{
				curZoomNum = (int)(tmpL / 6) - 1;
				mCurCentralMeridian = (double)(curZoomNum * 6 + 3) / RO;
			}
			else
			{
				curZoomNum = (int)(tmpL / 6) + 1;
				mCurCentralMeridian = (double)(curZoomNum * 6 - 3) / RO;
			}
		}

	}
	else
		mCurCentralMeridian = mCentralMeridian;

	y = tmpy - mCurCentralMeridian * mSemiAxisA;

	if (mLength90 <= abs(x))
	{
		if (x > 0)
			*latout = PI / 2.;
		else
			*latout = -PI / 2.;
		*lonout = mCurCentralMeridian;
		return;
	}

	if (abs(x) < 0.0001)
	{
		*latout = 0.0;
		*lonout = mCurCentralMeridian + y / mSemiAxisA;
		return;
	}

	double y2 = y * y;
	double y3 = y2 * y;
	double y4 = y3 * y;
	double y5 = y4 * y;
	double y6 = y5 * y;

	double Bf = getMeridianArcRad(x);
	double sinbf = sin(Bf);
	double sinbf2 = sinbf * sinbf;
	double cosbf = cos(Bf);
	double tf = tan(Bf);
	double tf2 = tf * tf;
	double tf4 = tf2 * tf2;
	double tt = 1 - mE2 * sinbf2;
	double tt3 = tt * tt * tt;
	double Nf = mSemiAxisA / sqrt(tt);
	double Nf3 = Nf * Nf * Nf;
	double Nf5 = Nf3 * Nf*Nf;
	double Mf = mSemiAxisA * (1 - mE2) / sqrt(tt3);
	double pf = sqrt(mE2p) * cosbf;
	double pf2 = pf * pf;

	*latout = Bf
		- tf * y2 / (2 * Mf*Nf)
		+ tf * (5. + 3 * tf2 + pf2 - 9.*tf2*pf2) * y4 / (24.*Mf*Nf3)
		+ tf * (61. + 90.*tf2 + 45.*tf4)*y6 / (720.*Mf*Nf5);
	double dtl = y / (Nf*cosbf)
		- (1. + 2 * tf2 + pf2) * y3 / (6.*Nf3*cosbf)
		+ (5. + 28.*tf2 + 24.*tf4 + 6.*pf2 + 8.*tf2*pf2)*y5 / (120.*Nf5*cosbf);

	if (is3Zone)
	{
		if (dtl > HALFTHREEDEGZONE)
			dtl = HALFTHREEDEGZONE;
		if (dtl < -HALFTHREEDEGZONE)
			dtl = -HALFTHREEDEGZONE;
	}
	else
	{
		if (dtl > HALFSIXDEGZONE)
			dtl = HALFSIXDEGZONE;
		if (dtl < -HALFSIXDEGZONE)
			dtl = -HALFSIXDEGZONE;
	}
	*lonout = mCurCentralMeridian + dtl;
}

//日晷投影
void CS57Projection::getGnomonicXY(double lat, double lon, double *y, double *x)
{
	mCurErrCode = RIGHT;
	double  sinl = 0.0, cosl = 0.0, sinb = 0.0, cosb = 0.0, dtl = 0.0;
	double tmpx = 0.0, tmpy = 0.0;

	dtl = lon - lon0;
	if (lat > HALFPI) lat = HALFPI;
	if (lat < -HALFPI) lat = -HALFPI;
	if (lat0 > 0.0&&lat - lat0 < -HALFPI)lat = lat0 - HALFPI + EPS10;
	if (lat0 < 0.0&&lat - lat0 > HALFPI) lat = lat0 + HALFPI - EPS10;

	sinb = sin(lat);
	cosb = cos(lat);
	cosl = cos(dtl);

	int i = 0;
	// 检核输入经纬度的合法性

	double boundar = 89.9 / RO;
	double boundar1 = 0.1 / RO;
	switch (azim0.mode)
	{
	case EQUIT:		// 切点在赤道
		if (lat > boundar)
		{
			lat = boundar;
			mCurErrCode = GNOMXYERR1; // 切点在赤道，纬度大于等89.9度，按89.9计算
		}
		else if (lat < -boundar)
		{
			lat = -boundar;
			mCurErrCode = GNOMXYERR2; // 切点在赤道，纬度小于－89.9度，按－89.9计算
		}
		if (dtl > boundar)
		{
			lon = lon0 + boundar;
			mCurErrCode = GNOMXYERR3; // 切点在赤道，经度差大于89.9度，按89.9计算
		}
		else if (dtl < -boundar)
		{
			lon = lon0 - boundar;
			mCurErrCode = GNOMXYERR4; // 切点在赤道，经度差小于－89.9度，按－89.9计算
		}
		break;
	case S_POLE:
		if (lat > -boundar1)
		{
			lat = -boundar1;
			mCurErrCode = GNOMXYERR5; // 切点在南极，纬度大于小于－0.1度，按－0.1计算
		}
		break;
	case N_POLE:
		if (lat < boundar1)
		{
			lat = boundar1;
			mCurErrCode = GNOMXYERR6; // 切点在北极，纬度大于大于0.1度，按0.1计算
		}
		break;
	case OBLIQ:
		tmpy = azim0.sinb0 * sinb + azim0.cosb0 * cosb * cosl;
		while (tmpy <= EPS10)
		{
			if (lat0 > 0)lat = lat + /*SECONDRAD*/0.1 / RO;
			if (lat0 < 0)lat = lat - /*SECONDRAD*/0.1 / RO;
			sinb = sin(lat);
			cosb = cos(lat);
			tmpy = azim0.sinb0 * sinb + azim0.cosb0 * cosb * cosl;
			mCurErrCode = GNOMXYERR7; // 切点不在南北极和赤道，位置超限，采用收缩纬度范围计算
			//i++;
		}
		break;
	}

	dtl = lon - lon0;
	sinb = sin(lat);
	cosb = cos(lat);
	sinl = sin(dtl);
	cosl = cos(dtl);

	switch (azim0.mode)
	{
	case EQUIT:
		tmpy = cosb * cosl;
		break;
	case OBLIQ:
		tmpy = azim0.sinb0 * sinb + azim0.cosb0 * cosb * cosl;
		break;
	case S_POLE:
		tmpy = -sinb;
		break;
	case N_POLE:
		tmpy = sinb;
		break;
	}

	// 
	if (abs(tmpy) <= EPS10)
	{
		QString str1 = QString::fromLocal8Bit("GetGnomonicXY日晷投影函数错误报告：");
		QString str2 = QString::fromLocal8Bit("计算错误");
		QMessageBox::critical(NULL, str1, str2,
			QMessageBox::Ok | QMessageBox::Default,
			QMessageBox::Cancel | QMessageBox::Escape, 0);
		//MessageBox(NULL, "GNOM error", "", MB_OK);
	}
	tmpy = 1. / tmpy;
	tmpx = tmpy * cosb*sinl;

	switch (azim0.mode)
	{
	case EQUIT:		// 切点在赤道

		tmpy *= sinb;
		break;
	case OBLIQ:
		tmpy *= azim0.cosb0 * sinb - azim0.sinb0 * cosb * cosl;
		break;
	case N_POLE:		// 切点在北极

		cosl = -cosl;
		/*-fallthrough*/
	case S_POLE:		// 切点在南极

		tmpy *= cosb * cosl;
		break;
	}
	*x = azim0.R * tmpx;
	*y = azim0.R * tmpy;
}

void CS57Projection::getGnomonicBL(double y, double x, double *latout, double *lonout)
{
	double  rh, cosz, sinz;
	double tmplat, tmplon;
	x = x / azim0.R;
	y = y / azim0.R;

	rh = hypot(x, y);
	tmplat = atan(rh);
	sinz = sin(tmplat);
	cosz = sqrt(1. - sinz * sinz);

	if (fabs(rh) <= EPS10)
	{
		tmplat = lat0;
		tmplon = 0.0;
	}
	else
	{
		switch (azim0.mode)
		{
		case OBLIQ:
			tmplat = cosz * azim0.sinb0 + y * sinz*azim0.cosb0 / rh;
			if (fabs(tmplat) >= 1.)
				tmplat = tmplat > 0. ? HALFPI : -HALFPI;
			else
				tmplat = asin(tmplat);
			y = (cosz - azim0.sinb0 * sin(tmplat)) * rh;
			x *= sinz * azim0.cosb0;
			break;
		case EQUIT:
			tmplat = y * sinz / rh;
			if (fabs(tmplat) >= 1.)
				tmplat = tmplat > 0. ? HALFPI : -HALFPI;
			else
				tmplat = asin(tmplat);
			y = cosz * rh;
			x *= sinz;
			break;
		case S_POLE:
			tmplat -= HALFPI;
			break;
		case N_POLE:
			tmplat = HALFPI - tmplat;
			y = -y;
			break;
		}
		tmplon = atan2(x, y);
	}
	tmplon = tmplon + lon0;

	*latout = tmplat;
	*lonout = tmplon;
}


void CS57Projection::setCentralMeridianFromView(double &lon, double &scale)
{
	setThreeZoneFromScale(scale);
	double lonDeg = lon * RO;

	if (is3Zone)
	{
		if (lon < 0)
		{
			mZoomNum = (int)(lonDeg - 1.5) / 3;
		}
		else
		{
			mZoomNum = (int)(lonDeg + 1.5) / 3;
		}

		mCentralMeridian = (double)mZoomNum * 3. / RO;
	}
	else
	{
		if (lon < 0)
		{
			mZoomNum = (int)(lonDeg / 6) - 1;
			mCentralMeridian = ((double)mZoomNum * 6. + 3.) / RO;
		}
		else
		{
			mZoomNum = (int)(lonDeg / 6) + 1;
			mCentralMeridian = ((double)mZoomNum * 6. - 3.) / RO;
		}

	}
}

void CS57Projection::setCentralMeridian(double mMeridian)
{
	// TODO: 在此处添加实现代码.
	double lonDeg = mMeridian * RO;

	if (is3Zone)
	{
		if (lonDeg < 0)
		{
			mZoomNum = (int)(lonDeg - 1.5) / 3;
		}
		else
		{
			mZoomNum = (int)(lonDeg + 1.5) / 3;
		}

		mCentralMeridian = (double)mZoomNum * 3. / RO;
	}
	else
	{
		if (lonDeg < 0)
		{
			mZoomNum = (int)(lonDeg / 6) - 1;
			mCentralMeridian = ((double)mZoomNum * 6. + 3.) / RO;
		}
		else
		{
			mZoomNum = (int)(lonDeg / 6) + 1;
			mCentralMeridian = ((double)mZoomNum * 6. - 3.) / RO;
		}
	}
}


// 等距方位投影
//void CS57Projection::GetAziequdisXY(double lat, double lon, double *x, double *y)
//{
//	Geodesic geod(mSemiAxisA, mAlf);
//	AzimuthalEquidistant obProj(geod);
//	obProj.Forward(lat0 * 180. / PI, lon0 * 180. / PI, lat * 180. / PI, lon * 180. / PI, *x, *y);
//}


//void CS57Projection::GetAziequdisBL(double x, double y, double *latout, double *lonout)
//{
//	Geodesic geod(mSemiAxisA, mAlf);
//	AzimuthalEquidistant obProj(geod);
//	obProj.Reverse(lat0 * 180. / PI, lon0 * 180. / PI, x, y, *latout, *lonout);
//	*latout = *latout * PI / 180.;
//	*lonout = *lonout * PI / 180.;
//}

// 等积圆锥投影
void CS57Projection::setConicStandLats(double lat1, double lat2)
{
	this->stdlat1 = lat1;
	this->stdlat2 = lat2;
}

//void CS57Projection::GetAlbequareXY(double lat, double lon, double *x, double *y)
//{
//	AlbersEqualArea obProj(mSemiAxisA, mAlf, stdlat1 * 180. / PI, stdlat2 * 180. / PI, 1.0);
//	obProj.Forward(mCentralMeridian * 180. / PI, lat * 180. / PI, lon * 180. / PI, *x, *y);
//}


//void CS57Projection::GetAlbequareBL(double x, double y, double *latout, double *lonout)
//{
//	AlbersEqualArea obProj(mSemiAxisA, mAlf, stdlat1, stdlat2, 1.0);
//	obProj.Reverse(mCentralMeridian * 180. / PI, x, y, *latout, *lonout);
//	*latout = *latout * PI / 180.;
//	*lonout = *lonout * PI / 180.;
//}
//
//// 极地立体投影
//void CS57Projection::setNorthPole(bool northpole)
//{
//	this->northp = northpole;
//}

int CS57Projection::getZoneNum(double lon)
{
	double lonDeg = lon * RO;
	if (is3Zone)
	{
		if (lonDeg < 0)
		{
			return (int)(lonDeg - 1.5) / 3;
		}
		else
		{
			return (int)(lonDeg + 1.5) / 3;
		}
	}
	else
	{
		if (lonDeg < 0)
		{
			return (int)(lonDeg / 6) - 1;
		}
		else
		{
			return (int)(lonDeg / 6) + 1;
		}
	}
}

int CS57Projection::getZoneNum()
{
	double lonDeg = mCentralMeridian * RO;
	while (lonDeg > 180)
	{
		lonDeg -= 360;
	}
	while (lonDeg < -180)
	{
		lonDeg += 360;
	}
	if (is3Zone)
	{
		if (lonDeg < 0)
		{
			return (int)(lonDeg - 1.5) / 3;
		}
		else
		{
			return (int)(lonDeg + 1.5) / 3;
		}
	}
	else
	{
		if (lonDeg < 0)
		{
			return (int)(lonDeg / 6) - 1;
		}
		else
		{
			return (int)(lonDeg / 6) + 1;
		}
	}
}

bool CS57Projection::is3ZoneDiff()
{
	return is3Zone;
}

void CS57Projection::getAzimuthPoint(double *lat0_out, double *lon0_out)
{
	*lat0_out = lat0;
	*lon0_out = lon0;
}

// 子午线弧长正解

double CS57Projection::getMeridianArcLength(double lat)
{
	double sb = sin(lat);
	double cb = cos(lat);
	double sb2 = sb * sb;
	double sb3 = sb2 * sb;
	double sb5 = sb3 * sb2;
	double sb7 = sb5 * sb2;
	double sb9 = sb7 * sb2;
	double sb11 = sb9 * sb2;
	return mSemiAxisA * (1. - mE2)*(A*lat - (B*sb + C * sb3 + D * sb5 + E * sb7 + F * sb9 + G * sb11)*cb);

}

// 子午线弧长反解

double CS57Projection::getMeridianArcRad(double s)
{
	double b0 = s / A0;
	double sb = sin(b0);
	double sb2 = sb * sb;
	double sb4 = sb2 * sb2;
	double sb6 = sb4 * sb2;
	double scb = sb * cos(b0);
	return b0 + (A1 + A2 * sb2 + A3 * sb4 + A4 * sb6)*scb;

}

// 恒向线大地距离正算，已知经纬度（弧度）坐标，方向角，距离，求点

void CS57Projection::getDistanceRhumbForward(double lat1, double lon1, double ang, double S, double * lat2, double *lon2)
{
	if (fabs(lat1 - PI / 2.) < EPS10)
	{
		ang = PI;
	}
	if (fabs(lat1 + PI / 2.) < EPS10)
	{
		ang = 0.0;
	}
	double cosa = cos(ang);
	double sina = sin(ang);
	if (fabs(cosa) < EPS10)
	{
		*lat2 = lat1;
		*lon2 = lon1 + S * sina / fun_lr(lat1);
	}
	else
	{
		double x1, y1, x2, y2;
		double sb1 = getMeridianArcLength(lat1);
		double sb2 = sb1 + S * cosa;
		*lat2 = getMeridianArcRad(sb2);
		double q1 = fun_q(lat1);
		double q2 = fun_q(*lat2);
		*lon2 = lon1 + (q2 - q1)*tan(ang);
	}
	return;
}

//void CS57Projection::GetDistanceRhumbForwardMct(double lat1, double lon1, double ang, double S, double * lat2, double *lon2)
//{
//
//	if (fabs(lat1 - PI / 2.) < EPS10)
//	{
//		ang = PI;
//	}
//	if (fabs(lat1 + PI / 2.) < EPS10)
//	{
//		ang = 0.0;
//	}
//	double cosa = cos(ang);
//	double sina = sin(ang);
//	if (fabs(cosa) < EPS10)
//	{
//		*lat2 = lat1;
//		*lon2 = lon1 + S * sina / fun_lr(lat1);
//	}
//	else
//	{
//		CS57Projection mProj;
//		double x, y, x2, y2;
//		double sb1 = getMeridianArcLength(lat1);
//		double sb2 = sb1 + S * cosa;
//		*lat2 = getMeridianArcRad(sb2);
//		mProj.GetXY(*lat2, 0.0, &x2, &y2);
//		y = x2 * sina / cosa;
//		*lon2 = lon1 + y / mSemiAxisA;
//		double q1 = fun_q(lat1);
//		double q2 = fun_q(*lat2);
//		*lon2 = lon1 + (q2 - q1)*tan(ang);
//	}
//	// 
//	// 
//	// 	double x1, y1, x2, y2;
//	// 	mProj.GetXY(lat1, lon1, &x1, &y1);
//	// 	x2 = x1 + S * cos(ang);
//	// 	y2 = y1 + S * sin(ang);
//	// 	mProj.GetBL(x2, y2, lat2, lon2);
//}

void CS57Projection::getDistanceRhumbReverse(double lat1, double lon1, double lat2, double lon2, double * ang, double *S)
{
	double dtb = lat2 - lat1;
	double dtl = lon2 - lon1;

	if (fabs(dtb) < EPS10)
	{
		*S = fabs(fun_lr(lat1)*dtl);
		if (dtl > 0)
			*ang = PI * 0.5;
		else
			*ang = PI * 1.5;
		return;
	}

	double q1 = fun_q(lat1);
	double q2 = fun_q(lat2);
	double dtx = mSemiAxisA * dtl;
	double dty = mSemiAxisA * (q2 - q1);
	*ang = atan2(dtx, dty);
	while (*ang < 0)*ang += PI * 2.;
	while (*ang > PI*2.)*ang -= PI * 2.;
	double sb1 = getMeridianArcLength(lat1);
	double sb2 = getMeridianArcLength(lat2);
	*S = fabs((sb2 - sb1) / cos(*ang));
}

//void CS57Projection::GetDistanceRhumbReverseMct(double lat1, double lon1, double lat2, double lon2, double * ang, double *S)
//{
//	double dtb = lat2 - lat1;
//	//double dtl = lon2 - lon1;
//	double x1, y1, x2, y2;
//	if (fabs(dtb) < EPS10)
//	{
//		*S = fabs(fun_lr(lat1)*dtl);
//		if (dtl > 0)
//			*ang = PI * 0.5;
//		else
//			*ang = PI * 1.5;
//		return;
//	}
//
//	CS57Projection mproj;
//	mproj.GetXY(lat1, lon1, &x1, &y1);
//	mproj.GetXY(lat2, lon2, &x2, &y2);
//	*ang = atan2(y2 - y1, x2 - x1);
//	while (*ang < 0)*ang += PI * 2.;
//	while (*ang > PI*2.)*ang -= PI * 2.;
//	double sb1 = getMeridianArcLength(lat1);
//	double sb2 = getMeridianArcLength(lat2);
//	*S = fabs((sb2 - sb1) / cos(*ang));
//}

void CS57Projection::getDistanceForward(double lat1, double lon1, double ang1, double S, double * lat2, double *lon2, double *ang2)
{
	// 首点方向角化为0-360；

	while (ang1 > 2. * PI) ang1 -= 2. * PI;
	while (ang1 < 0.0)ang1 += 2 * PI;

	// 大地线正向子午线
	double S2;
	if (ang1 < EPS10)  // 正向子午线

	{
		S2 = fabs(S) + getMeridianArcLength(lat1);
		if (S2 > mLength90)
		{
			S2 = mLength90 * 2. - S2;
			*lat2 = getMeridianArcRad(S2);
			*lon2 = lon1 + PI;
		}
		else
		{
			*lat2 = getMeridianArcRad(S2);
			*lon2 = lon1;
			*ang2 = PI;
		}
		return;
	}

	// 大地线是反向子午线

	if (fabs(ang1 - PI) < EPS10)
	{
		S2 = getMeridianArcLength(lat1) - S;
		if (S2 < -mLength90)
		{
			S2 = -(S2 + mLength90 * 2);
			*lat2 = getMeridianArcRad(S2);
			*lon2 = lon1 + PI;
			*ang2 = PI;
		}
		else
		{
			*lat2 = getMeridianArcRad(S2);
			*ang2 = 0.0;
			*lon2 = lon1;
		}
		return;
	}

	// 大地线是赤道，正向

	if (fabs(ang1 - PI / 2.) < EPS10&&fabs(lat1) < EPS10)
	{
		*lat2 = 0.0;
		*ang2 = PI * 1.5;
		*lon2 = lon1 + S / mSemiAxisA;
		return;
	}

	// 大地线是赤道，反向

	if (fabs(ang1 - PI * 3. / 2.) < EPS10&&fabs(lat1) < EPS10)
	{
		*lat2 = 0.0;
		*ang2 = PI * 0.5;
		*lon2 = lon1 - S / mSemiAxisA;
		return;
	}

	// 计算起点归化纬度 u1
	double sinb1 = sin(lat1);
	double cosb1 = cos(lat1);
	double u1 = atan2(sinb1*(1 - mAlf), cosb1);

	// 大地线在赤道处的方位角

	double sinA1 = sin(ang1); //始点方位角

	double cosA1 = cos(ang1);
	double sinu1 = sin(u1);
	double cosu1 = cos(u1);
	double sinA0 = cosu1 * sinA1;
	double Alf0 = atan2(sinA0, sqrt(cosA1*cosA1 + sinA1 * sinA1 * sinu1 *sinu1)); // 大地线在赤道处的方位角

	double cosA0 = cos(Alf0);


	// 计算大地线起点至大地线与赤道交点的（弧度）角

	double t1 = atan2(sinu1, cosA1 * cosu1); // 大地线起点至大地线与赤道交点的（弧度）角

	double sint1 = sin(t1);
	double cost1 = cos(t1);
	double sin2t1 = 2.*sint1*cost1;
	double cos2t1 = cost1 * cost1 - sint1 * sint1;
	double sin4t1 = 2.*sin2t1*cos2t1;
	double sin6t1 = 3. * sin2t1 - 4. * sin2t1*sin2t1*sin2t1;
	double sin8t1 = -4.*cos2t1*sin2t1*(2.*sin2t1*sin2t1 - 1.);
	double sin10t1 = 16.*pow(sin2t1, 5) - 20.*pow(sin2t1, 3) + 5 * sin2t1;
	double sin12t1 = 2.*sin2t1*cos2t1*(2.*sin2t1 + 1)*(2.*sin2t1 + 1)*(4.*sin2t1*sin2t1 - 3.);

	// 计算始点-大地线与赤道交点的经差


	double omg1 = atan2(sinA0*sint1, cost1);
	// 计算系数
	double k2 = (1 - sinA0 * sinA0)*mE2p;
	double E = (sqrt(1. + k2) - 1.) / (sqrt(1. + k2) + 1.);
	double E2 = E * E;
	double E3 = E2 * E;
	double E4 = E3 * E;
	double E5 = E4 * E;
	double E6 = E5 * E;

	double A1 = (1. + E2 / 4. + E4 / 64. + E6 / 256.) / (1. - E);
	double C11 = -E / 2. + E3 / 16. - E5 / 32.;
	double C12 = -E2 / 16. + E4 / 32. + E6 * 9. / 2048.;
	double C13 = -E3 / 48. + E5 * 3. / 256.;
	double C14 = -E4 * 5. / 512. + E6 * 3. / 512.;
	double C15 = -E5 * 7. / 1280.;
	double C16 = -E6 * 7. / 2048.;

	double Cp11 = E / 2. - E3 * 9. / 32. + E5 * 205. / 1536.;
	double Cp12 = E2 * 5. / 16. - E4 * 37. / 96. + E6 * 1335. / 4096.;
	double Cp13 = E3 * 29. / 96. - E5 * 75. / 128.;
	double Cp14 = E4 * 539. / 1536. - E6 * 2391. / 2560.;
	double Cp15 = E5 * 3467. / 7680.;
	double Cp16 = E6 * 38081. / 61440.;

	double n2 = m_n * m_n;
	double A3 = 1. - (1. - m_n)*E*0.5
		- (0.25 + 0.125*m_n - 0.375*n2)*E2
		- (0.0625 + 0.1875*m_n + 0.0625*n2)*E3
		- (0.046875 + 0.03125*m_n)*E4
		- 0.0234375*E5;
	double C31 = (1 - m_n)*E*0.25
		+ (1 - n2)*E2*0.125
		+ (3. + 3.*m_n - n2)*E3*0.015625
		+ (2.5 + m_n)*E4*0.015625
		+ 0.0234375*E5;
	double C32 = (1. / 16. - 3. / 32.*m_n + 1. / 32.*n2)*E2
		+ (3. / 64. - 1. / 32.*m_n - 3. / 64.*n2)*E3
		+ (3. / 128. + 1. / 128.*m_n)*E4
		+ 5. / 2556.*E5;
	double C33 = (5. / 192. - 3. / 64.*m_n + 5. / 192.*n2)*E3
		+ (3. / 128. - 5. / 192.*m_n)*E4
		+ 7. / 512.*E5;
	double C34 = (7. / 512. - 7. / 256.*m_n)*E4
		+ 7. / 512.*E5;
	double C35 = 21. / 2560 * E5;

	double I1 = A1 * (t1
		+ C11 * sin2t1
		+ C12 * sin4t1
		+ C13 * sin6t1
		+ C14 * sin8t1
		+ C15 * sin10t1
		+ C16 * sin12t1);

	double s1 = mSemiAxisB * I1;
	double s2 = s1 + S;
	double t2 = s2 / (mSemiAxisB*A1);

	double sint2 = sin(t2);
	double cost2 = cos(t2);
	double sin2t2 = 2.*sint2*cost2;
	double cos2t2 = cost2 * cost2 - sint2 * sint2;
	double sin4t2 = 2.*sin2t2*cos2t2;
	double sin6t2 = 3. * sin2t2 - 4. * sin2t2*sin2t2*sin2t2;
	double sin8t2 = -4.*cos2t2*sin2t2*(2.*sin2t2*sin2t2 - 1.);
	double sin10t2 = 16.*pow(sin2t2, 5) - 20.*pow(sin2t2, 3) + 5 * sin2t2;
	double sin12t2 = 2.*sin2t2*cos2t2*(2.*sin2t2 + 1)*(2.*sin2t2 + 1)*(4.*sin2t2*sin2t2 - 3.);
	t2 = t2
		+ Cp11 * sin2t2
		+ Cp12 * sin4t2
		+ Cp13 * sin6t2
		+ Cp14 * sin8t2
		+ Cp15 * sin10t2
		+ Cp16 * sin12t2;

	sint2 = sin(t2);
	cost2 = cos(t2);

	double u2 = atan2(cosA0*sint2, sqrt(cosA0*cost2*cosA0*cost2 + sinA0 * sinA0));
	double omg2 = atan2(sinA0*sint2, cost2);
	double I31 = A3 * (t1 + C31 * sin2t1 + C32 * sin4t1 + C33 * sin6t1 + C34 * sin8t1 + C35 * sin10t1);
	double I32 = A3 * (t2 + C31 * sin2t2 + C32 * sin4t2 + C33 * sin6t2 + C34 * sin8t2 + C35 * sin10t2);

	double lmd1 = omg1 - mAlf * sinA0*I31;
	double lmd2 = omg2 - mAlf * sinA0*I32;
	double lmd12 = lmd2 - lmd1;

	*lat2 = atan2(tan(u2), 1. - mAlf);
	*lon2 = lon1 + lmd12;
	*ang2 = atan2(sinA0, cosA0*cost2) + PI;
	while (*ang2 > 2 * PI) *ang2 -= PI * 2.;
	while (*ang2 < 0.0) *ang2 += PI * 2.;
	while (*lon2 > PI) *lon2 -= PI * 2;
	while (*lon2 < -PI) *lon2 += PI * 2.;
	double H = cosu1 * sinA1*sinA1 - sin(u2) * sin(*ang2);
}

void CS57Projection::getDistanceReverse(double lat1, double lon1, double lat2, double lon2, double * ang1, double *S, double *ang2)
{

	while (lon1 > PI) lon1 -= PI * 2.;
	while (lon1 < -PI)lon1 += PI * 2.;
	while (lon2 > PI) lon2 -= PI * 2.;
	while (lon2 < -PI)lon2 += PI * 2.;
	// 大地线是子午线

	if (fabs(lon2 - lon1) < EPS10)
	{
		*S = getMeridianArcLength(lat2) - getMeridianArcLength(lat1);
		if (lat2 > lat1)
		{
			*ang1 = 0.0;
			*ang2 = PI;
		}
		else
		{
			*ang1 = PI;
			*ang2 = 0.0;
			*S = -*S;
		}
		return;
	}

	// 大地线是赤道
	if (fabs(lat2) < EPS10&&fabs(lat1) < EPS10)
	{
		*S = (lon2 - lon1)*mSemiAxisA;
		if (lon2 > lon1)
		{
			*ang1 = PI * 0.5;
			*ang2 = PI * 1.5;
		}
		else
		{
			*ang1 = PI * 1.5;
			*ang2 = PI * 0.5;
			*S = -*S;
		}
		return;
	}

	double sinb1, cosb1;		// 首点纬度，lat1
	double sinb2, cosb2;		// 终点纬度，lat2
	double Alf0, sinA0, cosA0;  // 大地线方位角（大地线与赤道交点处方位角）
	double Alf1, sinA1, cosA1;	// 首点方位角

	double Alf2, sinA2, cosA2;	// 终点方位角

	double u1, sinu1, cosu1;	// 首点归化纬度
	double u2, sinu2, cosu2;	// 终点归化纬度
	double lmd1, lmd2, lmd12, lmd12In, dtlmd;	// 首点大地线经差，终点大地线经差，起讫点经差

	double omg1, omg2; // 过渡球上首末点与起点经差
	double t1, sint1, cost1;		// 过渡球上首点的弧度数
	double t2, sint2, cost2;		// 过渡球上末点的弧度数
	double sin2t1, cos2t1, sin4t1, sin6t1, sin8t1, sin10t1, sin12t1;
	double sin2t2, cos2t2, sin4t2, sin6t2, sin8t2, sin10t2, sin12t2;
	double k2;
	double x, y;
	double E, E2, E3, E4, E5, E6;
	double A1, C11, C12, C13, C14, C15, C16;
	double Cp11, Cp12, Cp13, Cp14, Cp15, Cp16;
	double A2, C21, C22, C23, C24, C25, C26;
	double A3, C31, C32, C33, C34, C35;

	lmd12In = lon2 - lon1;
	lmd12 = lmd12In;
	sinb1 = sin(lat1);
	cosb1 = cos(lat1);
	sinb2 = sin(lat2);
	cosb2 = cos(lat2);
	u1 = atan2(sinb1*(1 - mAlf), cosb1);
	u2 = atan2(sinb2*(1 - mAlf), cosb2);
	cosu1 = cos(u1);
	cosu2 = cos(u2);
	sinu1 = sin(u1);
	sinu2 = sin(u2);

	// 求解首点方位角 公式56
	x = (lmd12 - PI) / (mAlf*PI*cosu1);		// 公式53
	y = (u1 + u2) / (mAlf*PI*cosu1*cosu1);	// 公式53

	// 公式55，求解一元四次方程

	if (fabs(y) < EPS10)
	{
		Alf1 = atan2(-x, sqrt(max(0.0, (1. - x * x))));
		if (lmd12 < 0.0) Alf1 = -Alf1;
	}
	else
	{
		// 求解一元四次方程

		double xishu[5];
		double mu[4], mul;
		xishu[0] = -y * y;
		xishu[1] = -2.*y*y;
		xishu[2] = (1. - x * x - y * y);
		xishu[3] = 2.0;
		xishu[4] = 1.0;
		int num = solve_quartic_equation(xishu, mu);
		mul = 1.;
		for (int i = 0; i < num; i++)
		{
			if (mu[i] > 0)
			{
				mul = mu[i];
				break;
			}
		}
		Alf1 = atan2(-x / (1. + mul), y / mul);
	}


ITERATION:  // 迭代起点
	sinA1 = sin(Alf1);
	cosA1 = cos(Alf1);

	//	double degAlf1;
	//	degAlf1 = Alf1 * RO;

	// 求解大地线方位角,公式10
	Alf0 = atan2(sinA1*cosu1, sqrt(cosA1*cosA1 + sinA1 * sinA1*sinu1*sinu1));
	sinA0 = sin(Alf0);
	cosA0 = cos(Alf0);

	//	double degAlf0 = Alf0 * RO;
	t1 = atan2(sinu1, cosA1*cosu1);				//公式11
	sint1 = sin(t1);
	cost1 = cos(t1);
	//	double degt1 = t1 * RO;

	omg1 = atan2(sinA0*sint1, cost1);		//公式12
	//	double degomg1 = omg1 * RO;

	sinA2 = sinA0 / cosu2;						// 公式5
	cosA2 = sqrt(cosA1*cosA1*cosu1*cosu1 + cosu2 * cosu2 - cosu1 * cosu1) / cosu2;//公式45
	Alf2 = atan2(sinA2, cosA2);
	//	double degAlf2 = Alf2 * RO;

	t2 = atan2(sinu2, cosA2*cosu2);
	sint2 = sin(t2);
	cost2 = cos(t2);
	//	double degt2 = t2 * RO;

	omg2 = atan2(sinA0*sint2, cost2);
	//	double degomg2 = omg2 * RO;

	k2 = mE2p * cosA0*cosA0;
	E = (sqrt(1. + k2) - 1.) / (sqrt(1 + k2) + 1.);
	E2 = E * E;
	E3 = E2 * E;
	E4 = E3 * E;
	E5 = E4 * E;
	E6 = E5 * E;

	A1 = (1. + E2 / 4. + E4 / 64. + E6 / 256.) / (1. - E);
	C11 = -E / 2. + E3 / 16. - E5 / 32.;
	C12 = -E2 / 16. + E4 / 32. + E6 * 9. / 2048.;
	C13 = -E3 / 48. + E5 * 3. / 256.;
	C14 = -E4 * 5. / 512. + E6 * 3. / 512.;
	C15 = -E5 * 7. / 1280.;
	C16 = -E6 * 7. / 2048.;

	A2 = (1. - E)*(1. + E2 / 4. + E4 * 9. / 64. + E6 * 25. / 256.);
	C21 = E / 2. + E3 / 16. + E5 / 32.;
	C22 = E2 * 3. / 16. + E4 / 32. + E6 * 35. / 2048.;
	C23 = E3 * 5. / 48. + E5 * 5. / 256.;
	C24 = E4 * 35. / 512. + E6 * 7. / 512.;
	C25 = E5 * 63. / 1280.;
	C26 = E6 * 77. / 2048.;

	Cp11 = E / 2. - E3 * 9. / 32. + E5 * 205. / 1536.;
	Cp12 = E2 * 5. / 16. - E4 * 37. / 96. + E6 * 1335. / 4096.;
	Cp13 = E3 * 29. / 96. - E5 * 75. / 128.;
	Cp14 = E4 * 539. / 1536. - E6 * 2391. / 2560.;
	Cp15 = E5 * 3467. / 7680.;
	Cp16 = E6 * 38081. / 61440.;

	double n2 = m_n * m_n;
	A3 = 1. - (1. - m_n)*E*0.5
		- (0.25 + 0.125*m_n - 0.375*n2)*E2
		- (0.0625 + 0.1875*m_n + 0.0625*n2)*E3
		- (0.046875 + 0.03125*m_n)*E4
		- 0.0234375*E5;
	C31 = (1 - m_n)*E*0.25
		+ (1 - n2)*E2*0.125
		+ (3. + 3.*m_n - n2)*E3*0.015625
		+ (2.5 + m_n)*E4*0.015625
		+ 0.0234375*E5;
	C32 = (1. / 16. - 3. / 32.*m_n + 1. / 32.*n2)*E2
		+ (3. / 64. - 1. / 32.*m_n - 3. / 64.*n2)*E3
		+ (3. / 128. + 1. / 128.*m_n)*E4
		+ 5. / 2556.*E5;
	C33 = (5. / 192. - 3. / 64.*m_n + 5. / 192.*n2)*E3
		+ (3. / 128. - 5. / 192.*m_n)*E4
		+ 7. / 512.*E5;
	C34 = (7. / 512. - 7. / 256.*m_n)*E4
		+ 7. / 512.*E5;
	C35 = 21. / 2560 * E5;

	sin2t1 = 2.*sint1*cost1;
	cos2t1 = cost1 * cost1 - sint1 * sint1;
	sin4t1 = 2.*sin2t1*cos2t1;
	sin6t1 = 3. * sin2t1 - 4. * sin2t1*sin2t1*sin2t1;
	sin8t1 = -4.*cos2t1*sin2t1*(2.*sin2t1*sin2t1 - 1.);
	sin10t1 = 16.*pow(sin2t1, 5) - 20.*pow(sin2t1, 3) + 5 * sin2t1;
	sin12t1 = 2.*sin2t1*cos2t1*(2.*sin2t1 + 1)*(2.*sin2t1 + 1)*(4.*sin2t1*sin2t1 - 3.);

	sin2t2 = 2.*sint2*cost2;
	cos2t2 = cost2 * cost2 - sint2 * sint2;
	sin4t2 = 2.*sin2t2*cos2t2;
	sin6t2 = 3. * sin2t2 - 4. * sin2t2*sin2t2*sin2t2;
	sin8t2 = -4.*cos2t2*sin2t2*(2.*sin2t2*sin2t2 - 1.);
	sin10t2 = 16.*pow(sin2t2, 5) - 20.*pow(sin2t2, 3) + 5 * sin2t2;
	sin12t2 = 2.*sin2t2*cos2t2*(2.*sin2t2 + 1)*(2.*sin2t2 + 1)*(4.*sin2t2*sin2t2 - 3.);

	double I31 = A3 * (t1 + C31 * sin2t1 + C32 * sin4t1 + C33 * sin6t1 + C34 * sin8t1 + C35 * sin10t1);
	double I32 = A3 * (t2 + C31 * sin2t2 + C32 * sin4t2 + C33 * sin6t2 + C34 * sin8t2 + C35 * sin10t2);

	double I21 = A2 * (t1 + C21 * sin2t1 + C22 * sin4t1 + C23 * sin6t1 + C24 * sin8t1 + C25 * sin10t1 + C26 * sin12t1);
	double I22 = A2 * (t2 + C21 * sin2t2 + C22 * sin4t2 + C23 * sin6t2 + C24 * sin8t2 + C25 * sin10t2 + C26 * sin12t2);

	double I11 = A1 * (t1 + C11 * sin2t1 + C12 * sin4t1 + C13 * sin6t1 + C14 * sin8t1 + C15 * sin10t1 + C16 * sin12t1);
	double I12 = A1 * (t2 + C11 * sin2t2 + C12 * sin4t2 + C13 * sin6t2 + C14 * sin8t2 + C15 * sin10t2 + C16 * sin12t2);
	double J1 = I11 - I21;
	double J2 = I12 - I22;

	lmd1 = omg1 - mAlf * sinA0*I31;
	lmd2 = omg2 - mAlf * sinA0*I32;
	lmd12 = lmd2 - lmd1;

	double deglmd1, deglmd2, deglmd12;
	deglmd1 = lmd1 * RO;
	deglmd2 = lmd2 * RO;
	deglmd12 = lmd12 * RO;

	dtlmd = lmd12 - lmd12In;

	//	double degdtlmd = dtlmd * RO;

	double m12 = mSemiAxisB * (sqrt(1. + k2 * sint2*sint2)*cost1*sint2
		- sqrt(1. + k2 * sint1*sint1)*sint1*cost2
		- cost1 * cost2*(J2 - J1));
	double tmp = m12 / mSemiAxisA / cosA2 / cosu2;
	double dtAlf1 = -dtlmd / tmp;
	double degdtAlf1 = dtAlf1 * RO;
	Alf1 = Alf1 + dtAlf1;
	double newAlf1 = Alf1 * RO;

	//  判断迭代条件
	if (fabs(dtlmd) > EPS10)
		goto ITERATION;

	*S = (I12 - I11)*mSemiAxisB;
	*ang1 = Alf1;
	*ang2 = Alf2 + PI;
	if (*S < 0)
	{
		*S = -*S;
		*ang1 -= PI;
		*ang2 += PI;
	}
	while (*ang1 > 2 * PI) *ang1 -= 2 * PI;
	while (*ang1 < 0)*ang1 += 2 * PI;
	while (*ang2 > 2 * PI) *ang2 -= 2 * PI;
	while (*ang2 < 0)*ang2 += 2 * PI;
}

int CS57Projection::solve_quadratic_equation(double p[], double x[])
{

	double a, b, c, delta, sqrtDelta;

	a = p[2];
	b = p[1];
	c = p[0];
	if (fabs(a) < EPS)
	{
		if (fabs(b) < EPS)
		{
			return 0;
		}
		else
		{
			x[0] = -c / b;
			return 1;
		}
	}
	else
	{
		delta = b * b - 4.0 * a * c;
		if (delta > ZERO)
		{
			if (fabs(c) < EPS)    //若c = 0,由于计算误差,sqrt(b*b - 4*a*c）不等于b
			{
				x[0] = 0.0;
				x[1] = -b / a;
			}
			else
			{
				sqrtDelta = sqrt(delta);
				if (b > 0.0)
				{
					x[0] = (-2.0 * c) / (b + sqrtDelta);    //避免两个很接近的数相减,导致精度丢失
					x[1] = (-b - sqrtDelta) / (2.0 * a);
				}
				else
				{
					x[0] = (-b + sqrtDelta) / (2.0 * a);
					x[1] = (-2.0 * c) / (b - sqrtDelta);    //避免两个很接近的数相减,导致精度丢失
				}
			}
			return 2;
		}
		else if (fabs(delta) < EPS)
		{
			x[0] = x[1] = -b / (2.0 * a);
			return 2;
		}
		else
		{
			return 0;
		}
	}
}

int CS57Projection::solve_cubic_equation(double p[], double x[])
{
	double a, b, c, d, A, B, C, delta;
	double Y1, Y2, Z1, Z2, K, parm[3], roots[2], theta, T;

	a = p[3];
	b = p[2];
	c = p[1];
	d = p[0];

	if (fabs(a) < EPS)
	{
		parm[2] = b;
		parm[1] = c;
		parm[0] = d;

		return solve_quadratic_equation(parm, x);
	}
	else
	{
		A = b * b - 3.0 * a * c;
		B = b * c - 9.0 * a * d;
		C = c * c - 3.0 * b * d;

		delta = B * B - 4.0 * A * C;

		if (fabs(A) < EPS && fabs(B) < EPS)
		{
			x[0] = x[1] = x[2] = -b / (3.0 * a);
			return 3;
		}

		if (delta > ZERO)
		{
			parm[2] = 1.0;
			parm[1] = B;
			parm[0] = A * C;

			solve_quadratic_equation(parm, roots);
			Z1 = roots[0];
			Z2 = roots[1];

			Y1 = A * b + 3.0 * a * Z1;
			Y2 = A * b + 3.0 * a * Z2;

			if (Y1 < 0.0 && Y2 < 0.0)    //pow函数的底数必须为非负数,必须分类讨论
			{
				x[0] = (-b + pow(-Y1, 1.0 / 3.0) + pow(-Y2, 1.0 / 3.0)) / (3.0*a);
			}
			else if (Y1 < 0.0 && Y2 > 0.0)
			{
				x[0] = (-b + pow(-Y1, 1.0 / 3.0) - pow(Y2, 1.0 / 3.0)) / (3.0*a);
			}
			else if (Y1 > 0.0 && Y2 < 0.0)
			{
				x[0] = (-b - pow(Y1, 1.0 / 3.0) + pow(-Y2, 1.0 / 3.0)) / (3.0*a);
			}
			else
			{
				x[0] = (-b - pow(Y1, 1.0 / 3.0) - pow(Y2, 1.0 / 3.0)) / (3.0*a);
			}
			return 1;
		}
		else if (fabs(delta - 0.0) < EPS)
		{
			if (fabs(A - 0.0) > EPS)
			{
				K = B / A;
				x[0] = -b / a + K;
				x[1] = x[2] = -0.5 * K;
				return 3;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			if (A > 0.0)
			{
				T = (2.0 * A * b - 3.0 * a * B) / (2.0 * pow(A, 3.0 / 2.0));
				if (T > 1.0)    //由于计算误差,T的值可能略大于1(如1.0000001)
				{
					T = 1.0;
				}
				if (T < -1.0)
				{
					T = -1.0;
				}
				theta = acos(T);
				x[0] = (-b - 2.0 * sqrt(A) * cos(theta / 3.0)) / (3.0 * a);
				x[1] = (-b + sqrt(A) * (cos(theta / 3.0) + sqrt(3.0) * sin(theta / 3.0))) / (3.0 * a);
				x[2] = (-b + sqrt(A) * (cos(theta / 3.0) - sqrt(3.0) * sin(theta / 3.0))) / (3.0 * a);
				return 3;
			}
			else
			{
				return 0;
			}
		}
	}
}

int CS57Projection::solve_quartic_equation(double p[], double x[])
{
	//#define EPS 1.0e-30

	double a, b, c, d, e;
	double parm[4], roots[3];
	double y, M, N;
	double x1[2], x2[2];
	int rootCount1, rootCount2, rootCount, i;
	double MSquareTemp, MSquare, yTemp;

	a = p[4];
	b = p[3];
	c = p[2];
	d = p[1];
	e = p[0];

	if (fabs(a - 0.0) < EPS)
	{
		if (fabs(b - 0.0) < EPS)
		{
			parm[2] = c;
			parm[1] = d;
			parm[0] = e;
			return solve_quadratic_equation(parm, x);
		}
		else
		{
			parm[3] = b;
			parm[2] = c;
			parm[1] = d;
			parm[0] = e;
			return solve_cubic_equation(parm, x);
		}
	}
	else
	{
		b = b / a;
		c = c / a;
		d = d / a;
		e = e / a;

		parm[3] = 8.0;
		parm[2] = -4.0 * c;
		parm[1] = 2.0 * (b * d - 4.0 * e);
		parm[0] = -e * (b * b - 4.0 * c) - d * d;

		if (rootCount = solve_cubic_equation(parm, roots))
		{
			y = roots[0];
			MSquare = 8.0 * y + b * b - 4.0 * c;
			for (i = 1; i < rootCount; i++)
			{
				yTemp = roots[i];
				MSquareTemp = 8.0 * yTemp + b * b - 4.0 * c;
				if (MSquareTemp > MSquare)
				{
					MSquare = MSquareTemp;
					y = yTemp;
				}
			}

			if (MSquare > 0.0)
			{
				M = sqrt(MSquare);
				N = b * y - d;
				parm[2] = 2.0;
				parm[1] = b + M;
				parm[0] = 2.0 * (y + N / M);
				rootCount1 = solve_quadratic_equation(parm, x1);

				parm[2] = 2.0;
				parm[1] = b - M;
				parm[0] = 2.0 * (y - N / M);
				rootCount2 = solve_quadratic_equation(parm, x2);

				if (rootCount1 == 2)
				{
					x[0] = x1[0];
					x[1] = x1[1];
					x[2] = x2[0];
					x[3] = x2[1];
				}
				else
				{
					x[0] = x2[0];
					x[1] = x2[1];
					x[2] = x1[0];
					x[3] = x1[1];
				}
				return rootCount1 + rootCount2;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}
	}
}

//void CS57Projection::GetPovecegraXY(double lat, double lon, double *x, double *y)
//{
//	PolarStereographic obProj(mSemiAxisA, mAlf, 1.0);
//	obProj.Forward(northp, lat*180. / PI, lon*180. / PI, *x, *y);
//}
//
//void CS57Projection::GetPovecegraBL(double x, double y, double *latout, double *lonout)
//{
//	PolarStereographic obProj(mSemiAxisA, mAlf, 1.0);
//	obProj.Reverse(northp, x, y, *latout, *lonout);
//	*latout = *latout*PI / 180.;
//	*lonout = *lonout*PI / 180.;
//}
//
//// 横墨卡托投影
//void CS57Projection::GetTramctXY(double lat, double lon, double *x, double *y)
//{
//	TransverseMercator obProj(mSemiAxisA, mAlf, 1.0);
//	obProj.Forward(mCentralMeridian * 180 / PI, lat*180. / PI, lon*180. / PI, *x, *y);
//}
//
//void CS57Projection::GetTramctBL(double x, double y, double *latout, double *lonout)
//{
//	TransverseMercator obProj(mSemiAxisA, mAlf, 1.0);
//	obProj.Reverse(mCentralMeridian, x, y, *latout, *lonout);
//	*latout = *latout*PI / 180.;
//	*lonout = *lonout*PI / 180.;
//}