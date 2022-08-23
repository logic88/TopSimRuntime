#ifndef __TSVECTOR3_H__
#define __TSVECTOR3_H__

#include <cmath>
#include <float.h>
#include <iostream>
#include "TSVector2.h"


TS_BEGIN_RT_NAMESPACE

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	3ά������. </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
class  TSVector3
{
public:
	TSVector3()	
	{
		x = 0;
		y = 0;
		z = 0;
	}

	TSVector3(T nx, T ny, T nz)
	{
		x = nx;
		y = ny;
		z = nz;
	}
#ifndef SWIG
	TSVector3(TSVector3* vec)
	{
		x = vec->x;
		y = vec->y;
		z = vec->z;
	}
#endif
	TSVector3(const TSVector3 &source)
	{
		x = source.x;
		y = source.y;
		z = source.z;
	}

	TSVector3(T* source)
	{
		x = source[0];
		y = source[1];
		z = source[2];
	}
#ifndef SWIG
	TSVector3(const TSVector2<T> &source)
	{
		x = source.x;
		y = source.y;
		z = 1.0;
	}

	TSVector3(const TSVector2<T> &source, const T Z)
	{
		x = source.x;
		y = source.y;
		z = Z;
	}
#endif
	FORCEINLINE TSVector3 &operator = (const TSVector3 &right)
	{
		x = right.x;
		y = right.y;
		z = right.z;
		return *this;
	}


	FORCEINLINE TSVector3 operator + (const TSVector3 &right) const
	{
		return TSVector3(right.x + x, right.y + y, right.z + z);
	}

	FORCEINLINE TSVector3 operator - (const TSVector3 &right) const
	{
		return TSVector3(-right.x + x, -right.y + y, -right.z + z);
	}
	FORCEINLINE TSVector3 operator-() const
	{
		return TSVector3(-x,-y,-z);
	}


	FORCEINLINE TSVector3 operator * (const T scale) const
	{
		return TSVector3(x*scale, y*scale, z*scale);
	}

	FORCEINLINE TSVector3  operator / (const T scale) const
	{
		if(scale) return TSVector3(x/scale, y/scale, z/scale);
		return TSVector3(0, 0, 0);
	}

	FORCEINLINE void operator += (const TSVector3 &right)
	{
		x+=right.x;
		y+=right.y;
		z+=right.z;
	}


	FORCEINLINE void operator -= (const TSVector3 &right)
	{
		x-=right.x;
		y-=right.y;
		z-=right.z;
	}

	FORCEINLINE void operator *= (T scale) 
	{
		x*=scale;
		y*=scale;
		z*=scale;
	}

	FORCEINLINE void operator /= (T scale)
	{
		if(!scale) 
			return;
		x/=scale;
		y/=scale;
		z/=scale;
	}

	bool operator == (const TSVector3 &right) const
	{
		return (x == right.x &&
			y == right.y &&
			z == right.z);
	}

	bool operator != (const TSVector3 &right) const
	{
		return !(x == right.x &&
			y == right.y &&
			z == right.z);
	}

	FORCEINLINE const T  operator[](int i) const 
	{ 
		return ((T*)&x)[i]; 
	}
	FORCEINLINE       T &operator[](int i)       
	{ 
		return ((T*)&x)[i]; 
	}

	FORCEINLINE void set(T nx, T ny, T nz)
	{
		x = nx;
		y = ny;
		z = nz;
	}
#ifndef SWIG
	FORCEINLINE void set(const TSVector2<T> & vec, const T Z)
	{
		x = vec.x;
		y = vec.y;
		z = Z;   
	}
#endif
	FORCEINLINE void set(T xyz)
	{
		x =   
			y =  
			z = xyz;
	}

	FORCEINLINE void set(const TSVector3& t)
	{
		x = t.x;
		y = t.y;
		z = t.z;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	������һ��. </summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	///
	/// <returns>	��λ����. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE  const TSVector3 normalize() const
	{
		T length  = getLength();

		if(!length){
			return TSVector3();
		}
		return TSVector3(x/length,y/length,z/length);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	��ȡ�����ĳ��ȵ�ƽ��. </summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	///
	/// <returns>	���ȵ�ƽ��. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE const T getLengthSquared() const
	{ 
		return  x*x + y*y + z*z; 
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	��ȡ��������. </summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	///
	/// <returns>	��������. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE const T getLength() const      
	{ 
		return sqrt((DOUBLE)(x*x + y*y + z*z)); 
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	���. 
	/// 			</summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	///
	/// <param name="t">	Ŀ������. </param>
	///
	/// <returns>	��˽��. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE const T getDotProduct(const TSVector3 &t) const
	{
		return x*t.x + y*t.y + z*t.z;
	}

	FORCEINLINE TSVector3 operator ^(const TSVector3 &t)  const
	{
		return TSVector3(y   * t.z - z   * t.y,
			t.x * z   - t.z * x,
			x   * t.y - y   * t.x);
	}

	FORCEINLINE TSVector3 &operator ^=(const TSVector3 &t)
	{
		set(y   * t.z - z   * t.y,
			t.x * z   - t.z * x,
			x   * t.y - y   * t.x);
		return *this;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	���. </summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	///
	/// <param name="t2">	Ŀ������. </param>
	///
	/// <returns>	���. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE TSVector3 getCrossProduct(const TSVector3 &t2) const
	{
		TSVector3 v(y * t2.z - z * t2.y,
			t2.x * z - t2.z * x,
			x * t2.y - y * t2.x);
		return v;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	��ȡ��Ŀ������֮��ľ���. </summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	///
	/// <param name="v2">	Ŀ������. </param>
	///
	/// <returns>	����. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE const T getDistance( const TSVector3& v2) const
	{
		return sqrt((DOUBLE)((v2.x - x) * (v2.x - x) +
			(v2.y - y) * (v2.y - y) +
			(v2.z - z) * (v2.z - z)));
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	��ȡ��Ŀ������֮��ľ���ƽ��. </summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	///
	/// <param name="v2">	Ŀ������. </param>
	///
	/// <returns>	����ƽ��. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	FORCEINLINE const T getDistanceSquared( const TSVector3& v2) const
	{
		return (DOUBLE)((v2.x - x) * (v2.x - x) +
			(v2.y - y) * (v2.y - y) +
			(v2.z - z) * (v2.z - z));
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	��ȡ��Ŀ������֮��ļн�. </summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	///
	/// <param name="v2">	[in,out] Ŀ������. </param>
	///
	/// <returns>	�Ƕ�. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE const T getAngle(TSVector3& v2) const
	{             

		T angle = acos((DOUBLE)(getDotProduct(v2) / (getLength() * v2.getLength())));
		if(_isnan(angle))
			return 0;
		return angle ;
	}

	FORCEINLINE void clamp(T min, T max) 
	{
		x = x > max ? max : x < min ? min  : x;
		y = y > max ? max : y < min ? min  : y;
		z = z > max ? max : z < min ? min  : z;
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>���������</summary>
	///
	/// <remarks>	jimi, 2011/5/23. </remarks>
	///
	/// <param name="a">			���Ƚ���1. </param>
	/// <param name="b">			���Ƚ���1. </param>
	/// <param name="tolerance">	�Ƚϵľ���. </param>
	///
	/// <returns>	��ȷ�����. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	FORCEINLINE bool RealEqual( T a, T b, FLOAT tolerance ) const
	{
		if (fabs((DOUBLE)(b-a)) <= tolerance)
			return true;
		else
			return false;
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>�ж�λ���Ƿ��غ� </summary>
	///
	/// <remarks>	jimi, 2011/5/23. </remarks>
	/// <returns>	�غϷ�����. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	FORCEINLINE bool positionEquals(const TSVector3& rhs, FLOAT tolerance = 1e-03) const
	{
		return RealEqual(x, rhs.x, tolerance) &&
			RealEqual(y, rhs.y, tolerance) &&
			RealEqual(z, rhs.z, tolerance);

	}

	FORCEINLINE bool operator < ( const TSVector3& rhs ) const
	{
		if( x < rhs.x && y < rhs.y && z < rhs.z )
			return true;
		return false;
	}

	FORCEINLINE bool operator > ( const TSVector3& rhs ) const
	{
		if( x > rhs.x && y > rhs.y && z > rhs.z )
			return true;
		return false;
	}

	FORCEINLINE TSString ToString() const
	{
		TSOutputStringStream oss;

		oss.precision(17);
		oss << x << TS_TEXT(",") << y << TS_TEXT(",") << z;
		return oss.str();
	}
#ifndef SWIG
	friend TSBasicWriteStream & operator<<( TSBasicWriteStream & ios,const TSVector3 & data )
	{
		return ios << data.x << data.y << data.z;
	}

	friend TSBasicReadStream & operator>>( TSBasicReadStream & ios,TSVector3 & data )
	{
		return ios >> data.x >> data.y >> data.z;
	}
#endif
	T x, y, z;
};

typedef TSVector3<int   > TSVector3i;
typedef TSVector3<FLOAT > TSVector3f;
typedef TSVector3<DOUBLE> TSVector3d;

TS_END_RT_NAMESPACE

#endif // __TSVECTOR3_H__

