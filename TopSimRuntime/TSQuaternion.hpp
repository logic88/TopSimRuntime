#ifndef __TSQUATERNION_H__
#define __TSQUATERNION_H__


#ifndef PI
#define PI 3.141592654f                                       // Define for PI.
#endif // PI

#define GET_RADIANS(degree) (FLOAT)((degree * PI) / 180.0f)   // Will convert degrees to radians.

#include<math.h>                                            // Math header file.
#include "TSMatrix4.h"


TS_BEGIN_RT_NAMESPACE

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	四元数定义类. </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2015/2/2. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////

template <class qType>
class  TSQuaternion
{
public:
	TSQuaternion()
	{
		// Initialize each member variables.
		x = y = z = 0.0f;
		w = 1.0f;
	}

	TSQuaternion(const TSVector3<qType> &_V, DOUBLE _a){
		u=cos(_a/2);
		TSVector3<qType> V=_V;
		V=V.normalized()*sin(_a/2);
		x=V.X();
		y=V.Y();
		z=V.Z();
	}

	TSQuaternion(qType xAxis, qType yAxis, qType zAxis, qType wAxis) 
	{
		// Initialize each member variables.
		x = xAxis; y = yAxis; z = zAxis;
		w = wAxis;
	}

	FORCEINLINE TSQuaternion &operator=(const TSMatrix4<qType> &matrix)
	{

		w = 0.5f * sqrtf(matrix[0] + matrix[5] +
			matrix[10] + matrix[15] );
		w  = w  < 0.0001f ? 0.0004f : 4.0f*w;

		x  = (matrix[6] - matrix[9])/w;
		y  = (matrix[8] - matrix[2])/w;
		z  = (matrix[1] - matrix[4])/w;
		w /= 4.0f;

		return *this;
	}

	FORCEINLINE TSQuaternion &operator=(const TSQuaternion<qType> &q)
	{
		// This will make this quaternion equal to q.
		w = q.w; x = q.x; y = q.y; z = q.z;
		return *this;
	}

	FORCEINLINE bool operator==(const TSQuaternion<qType> &q)
	{
		return ((w == q.w) && (x == q.x) && (y == q.y) && (z == q.z));
	}

	FORCEINLINE bool operator!=(const TSQuaternion<qType> &q)
	{
		return !((w == q.w) && (x == q.x) && (y == q.y) && (z == q.z));
	}

	FORCEINLINE TSQuaternion operator*(const TSQuaternion  &q)
	{
		// To multiply a quaternion you must first do the dot and cross product
		// of the 2 quaternions then add/subract them to a result.
		TSQuaternion<qType>  result;

		result.x = w * q.x + x * q.w + y * q.z - z * q.y;
		result.y = w * q.y - x * q.z + y * q.w + z * q.x;
		result.z = w * q.z + x * q.y - y * q.x + z * q.w;
		result.w = w * q.w - x * q.x - y * q.y - z * q.z;

		return result;
	}

	FORCEINLINE void slerp(TSQuaternion &q1, TSQuaternion &q2, FLOAT t)
	{
		FLOAT o, co, so, scale0, scale1;
		FLOAT qi[4];


		// Do a linear interpolation between two quaternions (0 <= t <= 1).
		co = q1.x*q2.x + q1.y*q2.y + q1.z*q2.z + q1.w*q2.w;  // dot product

		if (co < 0)
		{
			co = -co;
			qi[0] = -q2.x;
			qi[1] = -q2.y;
			qi[2] = -q2.z;
			qi[3] = -q2.w;
		}
		else
		{
			qi[0] = q2.x;
			qi[1] = q2.y;
			qi[2] = q2.z;
			qi[3] = q2.w;
		}

		// If the quaternions are really close, do a simple linear interpolation.
		if ((1 - co) <= 0.0001f)
		{
			scale0 = 1 - t;
			scale1 = t;
		}
		else
		{
			// Otherwise SLERP.
			o      = (FLOAT) acos(co);
			so     = sinf(o);
			scale0 = sinf((1 - t) * o) / so;
			scale1 = sinf(t * o) / so;
		}

		// Calculate interpolated quaternion:

		x = scale0 * q1.x + scale1 * qi[0];
		y = scale0 * q1.y + scale1 * qi[1];
		z = scale0 * q1.z + scale1 * qi[2];
		w = scale0 * q1.w + scale1 * qi[3];
	} 

	FORCEINLINE TSQuaternion conjugate() const
	{
		// The Conjugate is basically all axis negated but the w.
		return TSQuaternion<qType> (-x, -y, -z, w);
	}

	FORCEINLINE void rotatef(FLOAT amount, FLOAT xAxis, FLOAT yAxis, FLOAT zAxis)
	{
		// Pretty much what is going on here is that if there is any axis that is not
		// a 0 or a 1 (meaning its not normalized) then we want to normalize it.
		// I created a if statement because I thought this would be better than normalizing
		// it every time this function is called, which would result in a lot of expensive
		// sqrt() call.  This is just in case the user forgot to use only normalized values.
		if((xAxis != 0 && xAxis != 1) ||
			(yAxis != 0 && yAxis != 1) ||
			(zAxis != 0 && zAxis != 1))
		{
			FLOAT length = (FLOAT)sqrt(xAxis * xAxis + yAxis * yAxis + zAxis * zAxis);
			xAxis /= length; yAxis /= length; zAxis /= length;
		}

		// Convert the angle degrees into radians.
		FLOAT angle = GET_RADIANS(amount);

		// Call this once for optimization, just like using the if statement to determine if
		// we should normalize.
		FLOAT sine = (FLOAT)sin(angle / 2.0f);

		// Create the quaternion.
		x = xAxis * sine;
		y = yAxis * sine;
		z = zAxis * sine;
		w = (FLOAT)cos(angle / 2.0f);

		// Normalize the quaternion.
		FLOAT length = 1 / (FLOAT)sqrt(x * x + y * y + z * z + w * w);
		x *= length;
		y *= length;
		z *= length;
	}

	FORCEINLINE TSMatrix4<qType> convertToMatrix() const
	{
		TSMatrix4<qType> pMatrix;

		// Calculate the first row.
		pMatrix.m[0]  = 1.0f - 2.0f * (y * y + z * z); 
		pMatrix.m[1]  = 2.0f * (x * y + z * w);
		pMatrix.m[2]  = 2.0f * (x * z - y * w);
		pMatrix.m[3]  = 0.0f;  

		// Calculate the second row.
		pMatrix.m[4]  = 2.0f * (x * y - z * w);  
		pMatrix.m[5]  = 1.0f - 2.0f * (x * x + z * z); 
		pMatrix.m[6]  = 2.0f * (z * y + x * w);  
		pMatrix.m[7]  = 0.0f;  

		// Calculate the third row.
		pMatrix.m[8]  = 2.0f * (x * z + y * w);
		pMatrix.m[9]  = 2.0f * (y * z - x * w);
		pMatrix.m[10] = 1.0f - 2.0f * (x * x + y * y);  
		pMatrix.m[11] = 0.0f;  

		// Calculate the fourth row.
		pMatrix.m[12] = 0;  
		pMatrix.m[13] = 0;  
		pMatrix.m[14] = 0;  
		pMatrix.m[15] = 1.0f;
		return pMatrix;
	}


	qType x, y, z, w;                      // x, y , z, and w axis.
};

typedef TSQuaternion<FLOAT> TSQuaternionf;
typedef TSQuaternion<DOUBLE> TSQuaterniond;

TS_END_RT_NAMESPACE

#endif // __TSQUATERNION_H__

