#ifndef __TSVECTOR2_H__
#define __TSVECTOR2_H__
#include <iostream>

#ifdef _MSC_VER
#	ifndef _SHOW_ALL_WARNING
#		pragma warning(disable:4244)
#	endif
#endif // _MSC_VER

#include "TSBasicIOStream.h"


TS_BEGIN_RT_NAMESPACE

static inline bool _ts_isnan(double d)
{
	unsigned char *ch = (unsigned char *)&d;
#ifdef IS_LITTLE_ENDIAN
	return (ch[7] & 0x7f) == 0x7f && ch[6] > 0xf0;
#else
	return (ch[0] & 0x7f) == 0x7f && ch[1] > 0xf0;
#endif
}

#if defined(_WRS_KERNEL)
#	define isnan _ts_isnan
#endif

#if !defined(_MSC_VER) && !defined(_isnan)
#	if  defined(__VXWORKS__) || defined(__ANDROID__)
#		define _isnan isnan
#	else
#		define _isnan __isnan
#	endif
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	2维向量类. </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////
class TSByteBuffer;

template <class T2>
class TSVector2
{
public:
	TSVector2()
	{
		x = 0;
		y = 0;
	}

	TSVector2(T2 X, T2 Y)
	{
		x = X;
		y = Y;
	}

	TSVector2(TSVector2* vec)
	{
		x = vec->x;
		y = vec->y;
	}

	TSVector2(const TSVector2 &source)
	{
		x = source.x;
		y = source.y;
	}

	TSVector2 & operator = (const TSVector2 &right){
		x = right.x;
		y = right.y;

		return *this;
	}

	FORCEINLINE TSVector2 operator + (const TSVector2 &right) const{
		return TSVector2(right.x + x, right.y + y);
	}

	FORCEINLINE TSVector2 operator - (const TSVector2 &right) const{
		return TSVector2(-right.x + x, -right.y + y);
	}


	FORCEINLINE TSVector2 operator * (const T2 scale) const{
		return TSVector2(x*scale, y*scale);
	}

	FORCEINLINE TSVector2  operator / (const T2 scale) const{
		if(scale) return TSVector2(x/scale, y/scale);
		return TSVector2(0, 0);
	}

	FORCEINLINE void operator += (const TSVector2 &right){
		x+=right.x;
		y+=right.y;
	}

	FORCEINLINE void operator -= (const TSVector2 &right){
		x-=right.x;
		y-=right.y;
	}

	FORCEINLINE void operator *= (const T2 scale){
		x*=scale;
		y*=scale;
	}

	FORCEINLINE void operator /= (const T2 scale){
		if(!scale) return;
		x/=scale;
		y/=scale;
	}

	FORCEINLINE const T2  operator[](int i) const 
	{ 
		return ((T2*)&x)[i]; 
	}

	FORCEINLINE       T2 &operator[](int i)       
	{ 
		return ((T2*)&x)[i]; 
	}

	FORCEINLINE bool operator < ( const TSVector2& rhs ) const
	{
		if( x < rhs.x && y < rhs.y)
			return true;
		return false;
	}

	FORCEINLINE bool operator > ( const TSVector2& rhs ) const
	{
		if( x > rhs.x && y > rhs.y)
			return true;
		return false;
	}

	FORCEINLINE bool operator == (const TSVector2 &right) const
	{
		return (x == right.x &&
			y == right.y);
	}

	FORCEINLINE bool operator != (const TSVector2 &right) const
	{
		return !(x == right.x &&
			y == right.y );
	}

	void set(T2 nx, T2 ny){
		x = nx;
		y = ny;
	}

	FORCEINLINE void negate(){
		x = -x;
		y = -y;
	}


	FORCEINLINE void clamp(T2 min, T2 max)
	{
		x = x > max ? max : x < min ? min  : x;
		y = y > max ? max : y < min ? min  : y;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	单一化2维向量. </summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	///
	/// <returns>	. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE  const TSVector2 normalize() const
	{
		T2 length  = getLength();

		if(!length){
			return TSVector2();
		}
		return TSVector2(x/length,y/length);
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	获取向量长度的平方. </summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	///
	/// <returns>	向量长度的平方. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE T2 getLengthSquared() const
	{ 
		return  x*x + y*y; 
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	获取向量的长度. </summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	///
	/// <returns>	向量的长度. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE T2 getLength() const       
	{ 
		return sqrtf(x*x + y*y); 
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	向量点积. 
	/// 			</summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	///
	/// <param name="t">	向量. </param>
	///
	/// <returns>	点积结果 </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE T2 getDotProduct(const TSVector2 &t) const 
	{
		return x*t.x + y*t.y;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	两向量之间的距离. </summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	///
	/// <param name="v2">	[in,out] 目标向量. </param>
	///
	/// <returns>	两向量之间的距离. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE T2 getDistance( const TSVector2& v2) const
	{
		return sqrtf((v2.x - x) * (v2.x - x) +
			(v2.y - y) * (v2.y - y));
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	2个向量之间的夹角. </summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	///
	/// <param name="v2">	[in,out] 目标向量. </param>
	///
	/// <returns>	角度. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE T2 getAngle(const TSVector2& v2) const	//两向量之间的角度
	{             

		T2 angle = acos((DOUBLE)(getDotProduct(v2) / (getLength() * v2.getLength())));
		if(_isnan(angle))
			return 0;
		return angle ;
	}

	FORCEINLINE TSString ToString() const
	{
		TSOutputStringStream oss;

		oss.precision(17);
		oss << x << TS_TEXT(",") << y;
		return oss.str();
	}
#ifndef SWIG
	friend TSBasicWriteStream & operator<<( TSBasicWriteStream & ios,const TSVector2 & data )
	{
		return ios<<data.x<<data.y;
	}

	friend TSBasicReadStream & operator>>( TSBasicReadStream & ios,TSVector2 & data )
	{
		return ios >> data.x >>data.y;
	}
#endif
	T2 x, y;
};

typedef TSVector2<int   > TSVector2i;
typedef TSVector2<FLOAT > TSVector2f;
typedef TSVector2<DOUBLE> TSVector2d;

TS_END_RT_NAMESPACE

#endif // __TSVECTOR2_H__

