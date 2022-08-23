#ifndef __TSCONSTVAL_H__
#define __TSCONSTVAL_H__

 
#define  INVALIDDOUBLE DBL_MAX +1

enum TSTblOrderType
{
	NonOrder = 0,
	RiseOrder = 1,
	FallOrder = 2,


};

enum TSTblInterpolateType
{
	NonInp = 0,
	LinerInp = 1,

};

#define HALFPI 1.5707963267948966
#define PI_2 1.57079632679489661923
#define PI_4 0.78539816339744830961
#define PI 3.14159265358979323846
#define TWOPI 6.283185307
#define FOURPI 12.5663706144
#define FOURPI3 1984.4017075391882

#define DEFAULT_OFFTIME_SHORT   9999
#define DEFAULT_OFFTIME_INT     99999
#define DEFAULT_OFFTIME_LONG    999999
#define DEFAULT_OFFTIME_FLOAT   99999.0
#define DEFAULT_OFFTIME_DOUBLE  999999.0

#define MAX_MISS_FLIGHT_TIME 5400.0 

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	无穷小的数. </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2011/8/4. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////

#define EPSILON 1.0e-25

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	宇宙重力值相对于地球质量的倍数. </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2011/8/4. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////

#define GM 3.9805235e14

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	宇宙重力值. </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2011/8/4. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////

#define UNIVGRAVCONST 398601620553728.0 

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	1个重力加速度（米/秒）. </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2011/8/4. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////

#define ONEG 9.80665 
#define INVONEG (1/ONEG)

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	压力系数(Coefficient of Pressure). </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2011/8/4. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////

#define CP 1.4 

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	空气分子的重量. </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2011/8/4. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////

#define AIRWEIGHT 28.9644

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	宇宙大气常数. </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2011/8/4. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////

#define GASCONSTANT 8314.32

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	AIRWEIGHT/GASCONSTANT. </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2011/8/4. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////

#define AIR_OVER_GAS 3.48367635597e-3

#define SIGN(a) ((a) >= 0 ? (1) : (-1))
#define SINC(x) ((x) == 0.0 ? (1.0) : sin (PI*x)/(PI * x))
#define ABS(a) ((a) >= 0 ? (a) : -(a))


#define FT2METER(f) ((f) * 0.3048)
#define METER2FT(f) ((f) * 3.28084)

#define FTSQ2METERSQ(f) ((f) * 0.3048*0.3048)
#define METERSQ2FTSQ(f) ((f) * 3.28084*3.28084)

#define KM2METER(f) ((f) * 1000.0)
#define METER2KM(f) ((f) * 0.001)

#define METER2DATAMILE(m) ((m) * 5.46807e-04)
#define MPSTODATAMPH(m) ((m) * 1.968503937)
#define DATAMPHTOMPS(d) ((d) * 0.508)
#define DATAMILE2METER(m) ((m) / 5.46807e-04)
#define KNOTSTOMPS(k)  ((k) * 0.514444)
#define MPSTOKNOTS(k)  ((k) * 1.943846)
#define MPSTOMPH(k)  ((k) * 2.236936364)
#define MPHTOMPS(m)  ((m) * 0.447039985)
#define LBSTONEWTONS(n) ((n) * 4.4482)
#define NEWTONSTOLBS(n) ((n) * 0.22481)
#define WATTSTOSHP(w) ((w) * 1.3410220896e-3)
#define SHPTOWATTS(s) ((s) * 745.699871582)
#define KGSECWTOLBHRHP(k) ((k) * 5918352.50161)
#define LBHRHPTOKGSECW(l) ((l) * 1.68965941067e-7)
#define RADTODEG(r) ((r) * 57.29577951308232)
#define DEGTORAD(d)  ((d) * 0.017453292519943295)
#define NM2METER(f) ((f) * 1852.0)
#define METERS2NM(f) ((f) / 1852.0)
#ifndef MIN
#define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif // MIN
#ifndef MAX
#define MAX(a,b)  ((a) > (b) ? (a) : (b))
#endif // _DEBUG
#define FEETTOMILES(r) ((r) / 5280.0)
#define MILESTOFEET(r) ((r) * 5280.0)
#define SQUARE(x) ((x)*(x))
#define ATAN2(x,y) (((x) == 0.0) && ((y) == 0.0) ? 0.0 : atan2(x,y))
#define ACOS(x) acos((x > 1.0) ? 1.0 : ((x < -1.0) ? -1.0: x)) 
#define ASIN(x) asin((x > 1.0) ? 1.0 : ((x < -1.0) ? -1.0: x))
#define COS(x) (x == 0.0 ? 1.0 : cos(x))
#define SIN(x) (x == PI_2 ? 1.0 : sin(x))
#define TAN(x) (x == PI_4 ? 1.0 : tan(x))
#define NTRACKQ 7  
#define EMINUS6  0.002478752176666
#define MINRANGEDOT 100.0*0.5144444
#define HALFMILE 804.672  /* meters */
#define LIGHTSPEED 3.0e8
#define COS45DEGREES 0.7071067
#define BOLTZMANN  1.38e-23 /* Boltzmann's constant (joules/deg Kelvin) */ 
#define STD_TEMP      290.0 /* Standard temperature (Kelvin) */
#define DB_TO_POWER(a)  pow (10.0, (a)/10.0)
#define POWER_TO_DB(a) (10.0 * log10 ((a < 1.0e-38 ? 1.0e-38 : a)))
#define DBM_TO_POWER(a)  pow (10.0, (a-30.0)/10.0)
#define POWER_TO_DBM(a) (10.0 * log10 ((a < 1.0e-38 ? 1.0e-38 : a)) + 30.0)
#define TWO31 2147483648.0  /* Two raised to the 31 power. */


#define SAFETY_FACTOR 10.0
#define VERY_SMALL_NO 1e-10
#define GRConst1 1.0016827521667031
#define GRConst2 -0.0016792197312156636


#define ONE_MINUS_ESQUARED 0.99330561555795738
#define SQRT_ONE_MINUS_ESQUARED 0.99664718710181355
#define FLATTENING_FACTOR 0.0033528128981864525
//******************************************************

#define EARTHRADIUS 6371220.0 /* Meters */
#define EARTH_RADIUS_ADJUSTMENT (6371220.0/3.0)
#define EARTHROTATIONRATE 0.0   /* Rate of earth rotation (rads/sec) */
#define EARTHROTATIONRATESAT 7.2921158553e-5  /* Rate of earth rotation  (radians/sec) */

#define EQUATORIAL_RADIUS 6378137.0/* WGS-84 earth radius, meters */
#define POLAR_RADIUS 6356752.3142
#define WGS84_ECCENTRICITY_SQUARED  0.0066943799014

#define	RADAREARTHRADADJ	1.3333333
#define	SIGINTEARTHRADADJ	1.0
#define	HUMINTEARTHRADADJ	1.0
#define	IMINTEARTHRADADJ	1.0
#define	IRLEARTHRADADJ		1.0
#define	IREARTHRADADJ		1.0
#endif // __TSCONSTVAL_H__




