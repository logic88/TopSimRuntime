#ifndef __TSVECTOR4_H__
#define __TSVECTOR4_H__



#include <iostream>
#include "TSVector3.h"

TS_BEGIN_RT_NAMESPACE

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	4Œ¨œÚ¡ø¿‡. </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2015/2/2. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////

template <class type>
class TSVector4
{
public:
	TSVector4()
	{
		x = 0;
		y = 0;
		z = 0;
		w = 0;
	}

	TSVector4(type X, type Y, type Z, type W)
	{
		x = X;
		y = Y;
		z = Z;
		w = W;
	}


	TSVector4(const TSVector4 &source){
		x = source.x;
		y = source.y;
		z = source.z;
		w = source.w;
	}

	TSVector4(const TSVector3<type> &source){
		x = source.x;
		y = source.y;
		z = source.z;
		w = 1.0f;
	}

	TSVector4(const TSVector3<type> &source, const type W){
		x = source.x;
		y = source.y;
		z = source.z;
		w = W;
	}

	FORCEINLINE const type  operator[](int i) const { return ((type*)&x)[i]; }
	FORCEINLINE       type &operator[](int i)       { return ((type*)&x)[i]; }

	FORCEINLINE TSVector4 &operator = (const TSVector4 &source){
		x = source.x;
		y = source.y;
		z = source.z;
		w = source.w;
		return *this;
	}

	FORCEINLINE TSVector4 &operator = (const TSVector3<type> &source){
		x = source.x;
		y = source.y;
		z = source.z;
		w = 1.0f;
		return *this;
	}

	FORCEINLINE TSVector4 operator + (const TSVector4 &right) const{
		return TSVector4(right.x + x, right.y + y, right.z + z, right.w + w );
	}

	FORCEINLINE TSVector4 operator - (const TSVector4  &right) const{
		return TSVector4(-right.x + x, -right.y + y, -right.z + z, -right.w + w );
	}

	FORCEINLINE TSVector4 operator * (const FLOAT scale) const{
		return TSVector4(x*scale, y*scale, z*scale, w*scale);
	}

	FORCEINLINE TSVector4 operator / (const FLOAT scale) const{
		if(scale) return TSVector4(x/scale, y/scale, z/scale, w/scale);
		return TSVector4(0, 0, 0, 0);
	}

	FORCEINLINE void operator += (const TSVector4 &right){
		x +=right.x;
		y +=right.y;
		z +=right.z;
		w +=right.w;
	}

	FORCEINLINE void operator -= (const TSVector4 &right){
		x-=right.x;
		y-=right.y;
		z-=right.z;
		w-=right.w;
	}

	FORCEINLINE void negate()
	{
		x = -x;
		y = -y;
		z = -z;
		w = -w;
	}

	FORCEINLINE void clamp(const type min, const type max)
	{
		x = x < min ? min : x > max ? max : x;
		y = y < min ? min : y > max ? max : y;
		z = z < min ? min : z > max ? max : z;
		w = w < min ? min : w > max ? max : w;
	}

	FORCEINLINE void operator *= (const type scale){
		x*=scale;
		y*=scale;
		z*=scale;
		w*=scale;
	}

	FORCEINLINE void operator /= (const type scale){
		if(!scale)  return;
		x/=scale;
		y/=scale;
		z/=scale;
		w/=scale;
	}

	FORCEINLINE bool operator < ( const TSVector4& rhs ) const
	{
		if( x < rhs.x && y < rhs.y && z < rhs.z && w < rhs.w)
			return true;
		return false;
	}

	FORCEINLINE bool operator > ( const TSVector4& rhs ) const
	{
		if( x > rhs.x && y > rhs.y && z > rhs.z && w > rhs.w )
			return true;
		return false;
	}

	FORCEINLINE bool operator == (const TSVector4 &right) const{
		return (x == right.x &&
			y == right.y &&
			z == right.z &&
			w == right.w);
	}

	FORCEINLINE bool operator != (const TSVector4 &right) const{
		return !(x == right.x &&
			y == right.y &&
			z == right.z &&
			w == right.w);
	}

	FORCEINLINE void set(type xyzw){
		x = 
			y =  
			z =  
			w = xyzw;
	}

	FORCEINLINE void set(type nx, type ny, type nz, type nw){
		x = nx;
		y = ny;
		z = nz;
		w = nw;
	}

	FORCEINLINE void set(const TSVector4  & vec)
	{
		x = vec.x;
		y = vec.y;
		z = vec.z;
		w = vec.w;   
	}

	FORCEINLINE void set(const TSVector3<type>  & vec, const type W)
	{
		x = vec.x;
		y = vec.y;
		z = vec.z;
		w = W;   
	}

	FORCEINLINE void set(const TSVector3<type>  & vec){
		x = vec.x;
		y = vec.y;
		z = vec.z;
		w = type(1.0);   
	}

	FORCEINLINE TSString ToString() const
	{
		TSOutputStringStream oss;

		oss.precision(17);
		oss << x << TS_TEXT(",") << y << TS_TEXT(",") << z << TS_TEXT(",") << w;
		return oss.str();
	}

#ifndef SWIG
	friend TSBasicWriteStream & operator<<( TSBasicWriteStream & ios,const TSVector4 & data )
	{
		return ios << data.x << data.y << data.z << data.w;
	}

	friend TSBasicReadStream & operator>>( TSBasicReadStream & ios,TSVector4 & data )
	{
		return ios >> data.x >> data.y >> data.z >> data.w;
	}
#endif
	type x, y, z, w;
};

typedef TSVector4<int   > TSVector4i;
typedef TSVector4<FLOAT > TSVector4f;
typedef TSVector4<DOUBLE> TSVector4d;


TS_END_RT_NAMESPACE

#endif // __TSVECTOR4_H__

