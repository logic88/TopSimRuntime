#ifndef __TSMATRIX4_H__
#define __TSMATRIX4_H__
#include "TSVector3.h"
#include "TSVector4.h"


TS_BEGIN_RT_NAMESPACE

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	矩阵操作类. </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2011/8/4. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////
/// 矩阵结构为：
/// 
///	      |  m[0]  m[4]  m[8]   m[12] |
///   M = |  m[1]  m[5]  m[9]   m[13] |
///       |  m[2]  m[6]  m[10]  m[14] |
///       |  m[3]  m[7]  m[11]  m[15] |
////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
class  TSMatrix4
{
public:

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>默认构造矩阵为单位矩阵，即不做变换.</summary>
	///
	/// <remarks>Huaru Infrastructure Team, 2011-8-8.</remarks>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	TSMatrix4()
	{
		setIdentity();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>Constructor.</summary>
	///
	/// <remarks>Huaru Infrastructure Team, 2011-8-8.</remarks>
	///
	///	      |  m0  m4  m8   m12 |
	///   M = |  m1  m5  m9   m13 |
	///       |  m2  m6  m10  m14 |
	///       |  m3  m7  m11  m15 |
	////////////////////////////////////////////////////////////////////////////////////////////////////

	TSMatrix4(T  m0, T  m1, T  m2, T  m3,
		T  m4, T  m5, T  m6, T  m7,
		T  m8, T  m9, T m10, T m11,
		T m12, T m13, T m14, T m15)
	{
		m[ 0] =  m0; m[ 1] =  m1; m[ 2] =  m2; m[ 3] =  m3;
		m[ 4] =  m4; m[ 5] =  m5; m[ 6] =  m6; m[ 7] =  m7;
		m[ 8] =  m8; m[ 9] =  m9; m[10] = m10; m[11] = m11;
		m[12] = m12; m[13] = m13; m[14] = m14; m[15] = m15;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	memcpy模板类型指针构造. </summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	///
	/// <param name="elements">	模板类型元素指针，取前16个元素. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	TSMatrix4(const T* elements)
	{
		memcpy(m, elements, sizeof(T)*16);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Copy 构造. </summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	///
	/// <param name="mat">	矩阵. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	TSMatrix4(const TSMatrix4 &mat)
	{
		memcpy(m, mat.m, sizeof(T)*16);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	+操作符重载. </summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	///
	/// <param name="mat">	矩阵. </param>
	///
	/// <returns>	结果矩阵. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE TSMatrix4 operator + (const TSMatrix4 &mat) const
	{
		return TSMatrix4(m[ 0] + mat.m[ 0], m[ 1] + mat.m[ 1], m[ 2] + mat.m[ 2], m[ 3] + mat.m[ 3],
			m[ 4] + mat.m[ 4], m[ 5] + mat.m[ 5], m[ 6] + mat.m[ 6], m[ 7] + mat.m[ 7],
			m[ 8] + mat.m[ 8], m[ 9] + mat.m[ 9], m[10] + mat.m[10], m[11] + mat.m[11],
			m[12] + mat.m[12], m[13] + mat.m[13], m[14] + mat.m[14], m[15] + mat.m[15]);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	-操作符重载. </summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	///
	/// <param name="mat">	矩阵. </param>
	///
	/// <returns>	结果矩阵. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE TSMatrix4 operator - (const TSMatrix4 &mat) const
	{
		return TSMatrix4(m[ 0] - mat.m[ 0], m[ 1] - mat.m[ 1], m[ 2] - mat.m[ 2], m[ 3] - mat.m[ 3],
			m[ 4] - mat.m[ 4], m[ 5] - mat.m[ 5], m[ 6] - mat.m[ 6], m[ 7] - mat.m[ 7],
			m[ 8] - mat.m[ 8], m[ 9] - mat.m[ 9], m[10] - mat.m[10], m[11] - mat.m[11],
			m[12] - mat.m[12], m[13] - mat.m[13], m[14] - mat.m[14], m[15] - mat.m[15]);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	*操作符重载. </summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	///
	/// <param name="mat">	矩阵. </param>
	///
	/// <returns>	结果矩阵. </returns>
	/// 
	/// M2 * M1 ： 表示先做M1所表示的变换，然后做M2所表示的变换
	/// 
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE TSMatrix4 operator * (const TSMatrix4 &mat) const
	{
		return TSMatrix4(m[0]*mat.m[ 0]+m[4]*mat.m[ 1]+m[ 8]*mat.m[ 2]+ m[12]*mat.m[ 3],
			m[1]*mat.m[ 0]+m[5]*mat.m[ 1]+m[ 9]*mat.m[ 2]+ m[13]*mat.m[ 3],
			m[2]*mat.m[ 0]+m[6]*mat.m[ 1]+m[10]*mat.m[ 2]+ m[14]*mat.m[ 3],
			m[3]*mat.m[ 0]+m[7]*mat.m[ 1]+m[11]*mat.m[ 2]+ m[15]*mat.m[ 3],
			m[0]*mat.m[ 4]+m[4]*mat.m[ 5]+m[ 8]*mat.m[ 6]+ m[12]*mat.m[ 7],
			m[1]*mat.m[ 4]+m[5]*mat.m[ 5]+m[ 9]*mat.m[ 6]+ m[13]*mat.m[ 7],
			m[2]*mat.m[ 4]+m[6]*mat.m[ 5]+m[10]*mat.m[ 6]+ m[14]*mat.m[ 7],
			m[3]*mat.m[ 4]+m[7]*mat.m[ 5]+m[11]*mat.m[ 6]+ m[15]*mat.m[ 7],
			m[0]*mat.m[ 8]+m[4]*mat.m[ 9]+m[ 8]*mat.m[10]+ m[12]*mat.m[11],
			m[1]*mat.m[ 8]+m[5]*mat.m[ 9]+m[ 9]*mat.m[10]+ m[13]*mat.m[11],
			m[2]*mat.m[ 8]+m[6]*mat.m[ 9]+m[10]*mat.m[10]+ m[14]*mat.m[11],
			m[3]*mat.m[ 8]+m[7]*mat.m[ 9]+m[11]*mat.m[10]+ m[15]*mat.m[11],
			m[0]*mat.m[12]+m[4]*mat.m[13]+m[ 8]*mat.m[14]+ m[12]*mat.m[15],
			m[1]*mat.m[12]+m[5]*mat.m[13]+m[ 9]*mat.m[14]+ m[13]*mat.m[15],
			m[2]*mat.m[12]+m[6]*mat.m[13]+m[10]*mat.m[14]+ m[14]*mat.m[15],
			m[3]*mat.m[12]+m[7]*mat.m[13]+m[11]*mat.m[14]+ m[15]*mat.m[15]);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	看指定位置时的矩阵调整. </summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	///
	/// <param name="viewPosition">	[in,out] 视点位置矢量. </param>
	/// <param name="viewFocus">   	[in,out] 焦点位置矢量. </param>
	/// <param name="upVector">	   	[in,out] 向上方向矢量. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE void lookAt(TSVector3<T> &viewPosition,
		TSVector3<T> &viewFocus, 
		TSVector3<T> &upVector)
	{
		TSVector3<T> *xaxis = (TSVector3<T> *)m ,
			*up    = (TSVector3<T> *)(m + 4),
			*at    = (TSVector3<T> *)(m + 8);

		*at  = viewFocus;
		*at -= viewPosition;
		at->normalize();

		upVector = xaxis->getCrossProduct(*at);
		xaxis->normalize();

		*at = up->getCrossProduct(*xaxis);

		*at *= -1.f;

		m[3]  = 0.f;
		m[7]  = 0.f;
		m[11] = 0.f;

		m[12] = viewPosition.x;
		m[13] = viewPosition.y; 
		m[14] = viewPosition.z;
		m[15] = 1.f;
		setInverse();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	+= 操作符重载 . </summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	///
	/// <param name="mat">	矩阵. </param>
	///
	/// <returns>	结果矩阵. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE TSMatrix4 &operator +=(const TSMatrix4 &mat)
	{
		for(int i = 0; i < 16; i++)
			m[i] += mat.m[i];
		return *this;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	-= 操作符重载 . </summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	///
	/// <param name="mat">	矩阵. </param>
	///
	/// <returns>	结果矩阵. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE TSMatrix4 &operator -=(const TSMatrix4 &mat)
	{
		for(int i = 0; i < 16; i++)
			m[i] -= mat.m[i];
		return *this;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	*= 操作符重载 . </summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	///
	/// <param name="mat">	矩阵. </param>
	///
	/// <returns>	结果矩阵. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	FORCEINLINE TSMatrix4 &operator *=(const T f)
	{
		for(int i = 0; i < 16; i++)
			m[i] *= f;
		return *this;
	}

	FORCEINLINE void setInfinitePerspective(T fovy, T aspect, T zNear)
	{
		FLOAT tangent	= tanf(1.57f*((fovy/2.0f)/180.0f));
		T		y		= tangent * zNear;
		T		x		= aspect  * y;

		setInfiniteFrustum(-x, x, -y, y, zNear);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	设置投影. </summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	///
	/// <param name="left">  	The left. </param>
	/// <param name="right"> 	The right. </param>
	/// <param name="bottom">	The bottom. </param>
	/// <param name="top">   	The top. </param>
	/// <param name="zNear"> 	The z coordinate near. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE void setInfiniteFrustum(T left, T  right, T  bottom, T  top,T  zNear)
	{
		setIdentity();
		m[ 0] = (2*zNear) / (right - left);
		m[ 2] = (right + left) / (right - left);

		m[ 5] = (2*zNear) / (top - bottom);
		m[ 6] = (top + bottom) / (top - bottom);

		FLOAT nudge = 1 - 1.0 / (1<<23);

		m[10] = -1 * nudge;
		m[11] = -2* zNear * nudge;

		m[14] = -1;
		m[15] =  0;
	}

	FORCEINLINE TSMatrix4 & operator *= (const TSMatrix4 &mat)
	{
		setElements(m[0]*mat.m[ 0]+m[4]*mat.m[ 1]+m[ 8]*mat.m[ 2]+ m[12]*mat.m[ 3],
			m[1]*mat.m[ 0]+m[5]*mat.m[ 1]+m[ 9]*mat.m[ 2]+ m[13]*mat.m[ 3],
			m[2]*mat.m[ 0]+m[6]*mat.m[ 1]+m[10]*mat.m[ 2]+ m[14]*mat.m[ 3],
			m[3]*mat.m[ 0]+m[7]*mat.m[ 1]+m[11]*mat.m[ 2]+ m[15]*mat.m[ 3],
			m[0]*mat.m[ 4]+m[4]*mat.m[ 5]+m[ 8]*mat.m[ 6]+ m[12]*mat.m[ 7],
			m[1]*mat.m[ 4]+m[5]*mat.m[ 5]+m[ 9]*mat.m[ 6]+ m[13]*mat.m[ 7],
			m[2]*mat.m[ 4]+m[6]*mat.m[ 5]+m[10]*mat.m[ 6]+ m[14]*mat.m[ 7],
			m[3]*mat.m[ 4]+m[7]*mat.m[ 5]+m[11]*mat.m[ 6]+ m[15]*mat.m[ 7],
			m[0]*mat.m[ 8]+m[4]*mat.m[ 9]+m[ 8]*mat.m[10]+ m[12]*mat.m[11],
			m[1]*mat.m[ 8]+m[5]*mat.m[ 9]+m[ 9]*mat.m[10]+ m[13]*mat.m[11],
			m[2]*mat.m[ 8]+m[6]*mat.m[ 9]+m[10]*mat.m[10]+ m[14]*mat.m[11],
			m[3]*mat.m[ 8]+m[7]*mat.m[ 9]+m[11]*mat.m[10]+ m[15]*mat.m[11],
			m[0]*mat.m[12]+m[4]*mat.m[13]+m[ 8]*mat.m[14]+ m[12]*mat.m[15],
			m[1]*mat.m[12]+m[5]*mat.m[13]+m[ 9]*mat.m[14]+ m[13]*mat.m[15],
			m[2]*mat.m[12]+m[6]*mat.m[13]+m[10]*mat.m[14]+ m[14]*mat.m[15],
			m[3]*mat.m[12]+m[7]*mat.m[13]+m[11]*mat.m[14]+ m[15]*mat.m[15]);
		return *this;
	} 

	FORCEINLINE bool operator == (const TSMatrix4 &mat) const
	{
		return memcmp(m, mat.m, sizeof(T)*16) == 0;
	}

	FORCEINLINE bool operator != (const TSMatrix4 &mat) const
	{
		return memcmp(m, mat.m, sizeof(T)*16) != 0;
	}

	FORCEINLINE TSMatrix4 &operator  = (const TSMatrix4 &mat)
	{
		memcpy(m,mat.m, sizeof(T)*16);
		return *this;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>三维向量 * 四阶矩阵.</summary>
	///
	/// <remarks>Huaru Infrastructure Team, 2011-8-8.</remarks>
	///
	/// <param name="t">  原三维向量t.</param>
	/// <param name="mat">变换矩阵.</param>
	///
	/// <returns>变换后的向量.</returns>
	/// 
	/// 注： 不区分左乘向量和右乘向量， t*Mat 和 Mat*t 结果相同
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE friend TSVector3<T> operator * (const TSVector3<T> &t,const TSMatrix4 &mat){
		return TSVector3<T>(mat.m[ 0]*t.x + mat.m[ 4]*t.y + mat.m[ 8]*t.z + mat.m[12],
			mat.m[ 1]*t.x + mat.m[ 5]*t.y + mat.m[ 9]*t.z + mat.m[13],
			mat.m[ 2]*t.x + mat.m[ 6]*t.y + mat.m[10]*t.z + mat.m[14]);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>四阶矩阵 * 三维向量 .</summary>
	///
	/// <remarks>Huaru Infrastructure Team, 2011-8-8.</remarks>
	///
	/// <param name="mat">变换矩阵.</param>
	/// <param name="t">  原三维向量t.</param>
	///
	/// <returns>变换后的向量.</returns>
	/// 
	/// 注： 不区分左乘向量和右乘向量， t*Mat 和 Mat*t 结果相同
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE friend TSVector3<T> operator * (const TSMatrix4 &mat,const TSVector3<T> &t){
		return TSVector3<T>(mat.m[ 0]*t.x + mat.m[ 4]*t.y + mat.m[ 8]*t.z + mat.m[12],
			mat.m[ 1]*t.x + mat.m[ 5]*t.y + mat.m[ 9]*t.z + mat.m[13],
			mat.m[ 2]*t.x + mat.m[ 6]*t.y + mat.m[10]*t.z + mat.m[14]);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>四阶矩阵 * 四维向量 .</summary>
	///
	/// <remarks>Huaru Infrastructure Team, 2011-8-8.</remarks>
	///
	/// <param name="mat">变换矩阵.</param>
	/// <param name="t">  原四维向量t.</param>
	///
	/// <returns>变换后的向量.</returns>
	/// 
	/// 注： 不区分左乘向量和右乘向量， t*Mat 和 Mat*t 结果相同
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE friend TSVector4<T> operator * (const TSMatrix4 &mat, const TSVector4<T> &t){
		return TSVector4<T>(mat.m[ 0]*t.x + mat.m[ 4]*t.y + mat.m[ 8]*t.z + mat.m[12]*t.w,
			mat.m[ 1]*t.x + mat.m[ 5]*t.y + mat.m[ 9]*t.z + mat.m[13]*t.w,
			mat.m[ 2]*t.x + mat.m[ 6]*t.y + mat.m[10]*t.z + mat.m[14]*t.w,
			mat.m[ 3]*t.x + mat.m[ 7]*t.y + mat.m[11]*t.z + mat.m[15]*t.w);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>四阶矩阵 * 四维向量 .</summary>
	///
	/// <remarks>Huaru Infrastructure Team, 2011-8-8.</remarks>
	///
	/// <param name="t">  原四维向量t.</param>
	/// <param name="mat">变换矩阵.</param>
	///
	/// <returns>变换后的向量.</returns>
	/// 
	/// 注： 不区分左乘向量和右乘向量， t*Mat 和 Mat*t 结果相同
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE friend TSVector4<T> operator * (const TSVector4<T> &t,const TSMatrix4 &mat){
		return TSVector4<T>(mat.m[ 0]*t.x + mat.m[ 4]*t.y + mat.m[ 8]*t.z + mat.m[12]*t.w,
			mat.m[ 1]*t.x + mat.m[ 5]*t.y + mat.m[ 9]*t.z + mat.m[13]*t.w,
			mat.m[ 2]*t.x + mat.m[ 6]*t.y + mat.m[10]*t.z + mat.m[14]*t.w,
			mat.m[ 3]*t.x + mat.m[ 7]*t.y + mat.m[11]*t.z + mat.m[15]*t.w);
	}

	FORCEINLINE friend  void operator *=(TSVector3<T> &t,const TSMatrix4 &mat){
		t.set(mat.m[ 0]*t.x + mat.m[ 4]*t.y + mat.m[ 8]*t.z + mat.m[12],
			mat.m[ 1]*t.x + mat.m[ 5]*t.y + mat.m[ 9]*t.z + mat.m[13],
			mat.m[ 2]*t.x + mat.m[ 6]*t.y + mat.m[10]*t.z + mat.m[14]);
	}

	FORCEINLINE friend void operator *=(TSVector4<T> &t,const TSMatrix4 &mat){
		t.set(mat.m[ 0]*t.x + mat.m[ 4]*t.y + mat.m[ 8]*t.z + mat.m[12]*t.w,
			mat.m[ 1]*t.x + mat.m[ 5]*t.y + mat.m[ 9]*t.z + mat.m[13]*t.w,
			mat.m[ 2]*t.x + mat.m[ 6]*t.y + mat.m[10]*t.z + mat.m[14]*t.w,
			mat.m[ 3]*t.x + mat.m[ 7]*t.y + mat.m[11]*t.z + mat.m[15]*t.w);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>Gets the determinant.</summary>
	///
	/// <remarks>Huaru Infrastructure Team, 2011-8-8.</remarks>
	///
	/// <returns>The determinant.</returns>
	/// 	
	/// 	\返回矩阵的行列式.
	/// 	  该函数只计算矩阵前三行三列的行列式
	/// 	
	/// 	  行列式等于变换后的基向量为三边的平行六面体的有符号体积 
	/// 	  行列式的值和矩阵变换导致的尺寸变换有关。 
	/// 	  其中行列式的绝对值和体积改变有关。
	/// 	  行列式的符号说明了变换矩阵是否包含镜像或投影。 
	/// 	  矩阵的行列式还能对矩阵所代表的变换进行分类：
	/// 	  	如果矩阵行列式为零，那么该矩阵包含投影。 
	/// 	  	如果矩阵行列式为负，那么该矩阵包含镜像。.</date>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE T getDeterminant() const
	{	
		T det;
		det = m[0] * m[5] * m[10];
		det += m[4] * m[9] * m[2];
		det += m[8] * m[1] * m[6];
		det -= m[8] * m[5] * m[2];
		det -= m[4] * m[1] * m[10];
		det -= m[0] * m[9] * m[6];
		return det;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>Sets the inverse.</summary>
	///
	/// <remarks>Huaru Infrastructure Team, 2011-8-8.</remarks>
	///
	/// <returns>true if it succeeds, false if it fails.
	/// 	
	/// 	矩阵的逆使得我们可以计算变换的“反向”或者“相反”变换——能“撤销”原变换的变换。 
	/// 	所以，如果向量V使用矩阵M来进行变换，接着使用M的逆M（^-1）进行变换，将会得到原向量。
	/// 		M（^-1）（VM） = V.</returns>
	///
	/// ### <date>2011-8-8 将矩阵的逆赋值给该矩阵.</date>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE bool setInverse()
	{
		int i, j, k, swap;
		T temp[16], t;

		memcpy(temp, m, 16*sizeof(T));
		setIdentity();

		for (i = 0; i < 4; i++) {
			swap = i;
			for (j = i + 1; j < 4; j++)
			{
				if (fabs(temp[j*4 + i]) >  fabs(temp[i*4 + i]))
				{
					swap = j;
				}
			}

			if (swap != i) {
				for (k = 0; k < 4; k++) {
					t                = temp[i*4 + k];
					temp[i*4 + k]    = temp[swap*4 + k];
					temp[swap*4 + k] = t;

					t             = m[i*4 + k];
					m[i*4 + k]    = m[swap*4 + k];
					m[swap*4 + k] = t;
				}
			}

			if(!temp[i*4 + i])
				return false;

			t = temp[i*4 + i];

			for (k = 0; k < 4; k++) {
				temp[i*4 + k] /= t;
				m[i*4 + k]     = m[i*4 + k] / t;
			}

			for (j = 0; j < 4; j++) {
				if (j != i) {
					t = temp[j*4 + i];
					for (k = 0; k < 4; k++) {
						temp[j*4 + k] -= temp[i*4 + k] * t;
						m[j*4 + k]     = m[j*4 + k] - m[i*4 + k] * t;
					}
				}
			}
		}
		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	缩放. </summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	///
	/// <param name="x">	x轴的值. </param>
	/// <param name="y">	y轴的值. </param>
	/// <param name="z">	z轴的值. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE void setScale(T x, T y, T z)
	{
		setIdentity();
		m[ 0] = x;
		m[ 5] = y;
		m[10] = z;     
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	将矩阵绕指定矢量旋转指定角度. </summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	///
	/// <param name="angle">	旋转角度. </param>
	/// <param name="axis"> 	旋转矢量. </param>
	/// 
	/// 记 axis.x为x，axis.y为y，axis.z为z，angle为A
	///     | x*x + cosA*(1-x*x)      x*y*(1-cosA) - sinA*z   x*z*(1-cosA) + sinA*y  0 |
	/// M = | x*y*(1-cosA) + sinA*z   y*y + cosA*(1-y*y)      y*z*(1-cosA) - sinA*x  0 |
	///     | x*z*(1-cosA) - sinA*y   y*z*(1-cosA) + sinA*x   z*z + cosA*(1-z*z)     0 |
	///     |         0                         0                      0             1 |
	/// 
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE void rotateAxis(FLOAT angle, const TSVector3<T> &axis) 
	{
		FLOAT sinAngle = sin(angle),
			cosAngle = cos(angle),
			oneMinusCosAngle = 1.0f - cosAngle;

		setIdentity();

		m[ 0] = (axis.x)*(axis.x) + cosAngle*(1-(axis.x)*(axis.x));
		m[ 4] = (axis.x)*(axis.y)*(oneMinusCosAngle) - sinAngle*axis.z;
		m[ 8] = (axis.x)*(axis.z)*(oneMinusCosAngle) + sinAngle*axis.y;

		m[ 1] = (axis.x)*(axis.y)*(oneMinusCosAngle) + sinAngle*axis.z;
		m[ 5] = (axis.y)*(axis.y) + cosAngle*(1-(axis.y)*(axis.y));
		m[ 9] = (axis.y)*(axis.z)*(oneMinusCosAngle) - sinAngle*axis.x;

		m[ 2] = (axis.x)*(axis.z)*(oneMinusCosAngle) - sinAngle*axis.y;
		m[ 6] = (axis.y)*(axis.z)*(oneMinusCosAngle) + sinAngle*axis.x;
		m[10] = (axis.z)*(axis.z) + cosAngle*(1-(axis.z)*(axis.z));
	}

	FORCEINLINE void setElements(const T* elements)
	{
		memcpy(m, elements, sizeof(T)*16);
	}

	void setElements(T  m0, T  m1, T  m2, T  m3,
		T  m4, T  m5, T  m6, T  m7,
		T  m8, T  m9, T m10, T m11,
		T m12, T m13, T m14, T m15)
	{
		m[ 0] =  m0; m[ 1] =  m1; m[ 2] =  m2; m[ 3] =  m3;
		m[ 4] =  m4; m[ 5] =  m5; m[ 6] =  m6; m[ 7] =  m7;
		m[ 8] =  m8; m[ 9] =  m9; m[10] = m10; m[11] = m11;
		m[12] = m12; m[13] = m13; m[14] = m14; m[15] = m15;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>矩阵转置.</summary>
	///
	/// <remarks>Huaru Infrastructure Team, 2011-8-8.</remarks>
	///
	/// 如果矩阵是正交矩阵，矩阵转置 = 矩阵的逆
	/// 旋转变换矩阵和镜像变换矩阵是正交矩阵。
	/// 如果知道矩阵是正交的，就可以避免计算逆矩阵。
	/// 
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE void setTranspose()
	{
		T    temp = 0;
		temp  = m[4];  m[4]  = m[1];
		m[1]  = temp;  temp  = m[8];
		m[8]  = m[2];  m[2]  = temp;
		temp  = m[12]; m[12] = m[3];
		m[3]  = temp;  temp  = m[9];
		m[9]  = m[6];  m[6]  = temp;
		temp  = m[13]; m[13] = m[7];
		m[7]  = temp;  temp  = m[14];
		m[14] = m[11]; m[11] = temp;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	矩阵归一化. </summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	/// 
	/// 单位矩阵表示不做任何变换
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE void setIdentity()
	{
		m[ 0] = 1; m[ 1] = 0; m[ 2] = 0; m[ 3] = 0;
		m[ 4] = 0; m[ 5] = 1; m[ 6] = 0; m[ 7] = 0;
		m[ 8] = 0; m[ 9] = 0; m[10] = 1; m[11] = 0;
		m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	矩阵转置. </summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE void setInverseTranspose()
	{
		if(!setInverse())
			return;
		setTranspose(); 
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>设置为零矩阵.</summary>
	/// 
	///	将任何向量转换到（0，0，0，0）
	///	
	/// <remarks>Huaru Infrastructure Team, 2011-8-8.</remarks>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void setZero(){
		for(int i = 0; i < 16; i++)
			m[i] = 0;
	}

	void  set(const TSMatrix4  &matrix){
		setElements(matrix.m);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	TSMatrix4d x = -y. </summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void negate(){
		for(int i = 0; i < 16; i++)
			m[i] =-m[i];
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	沿X轴旋转指定角度. </summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	///
	/// <param name="angle">	旋转角度. </param>
	/// 
	///     | 1       0          0        0  |
	/// M = | 0   cos(angle) -sin(angle)  0  |
	///     | 0   sin(angle)  cos(angle)  0  |
	///     | 0       0          0        1  |
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE void rotateX(const FLOAT angle){
		T s = sin(angle);
		T c = cos(angle);

		m00 = 1.0f; m01 = 0.0f; m02 = 0.0f; m03 = 0.0f;
		m10 = 0.0f; m11 = c;    m12 = -s;   m13 = 0.0f;
		m20 = 0.0f; m21 = s;    m22 = c;    m23 = 0.0f;
		m30 = 0.0f; m31 = 0.0f; m32 = 0.0f; m33 = 1.0f;
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	沿Y轴旋转指定角度. </summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	///
	/// <param name="angle">	旋转角度. </param>
	/// 
	///     | cos(angle)  0   sin(angle)   0 |
	/// M = |      0      1        0        0 |
	///     | -sin(angle)  0    cos(angle)   0 |
	///     |      0      0        0        1 |
	/// 
	///         
	/// 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	FORCEINLINE void rotateY(const FLOAT angle){
		T s = sin(angle);
		T c = cos(angle);

		m00 = c;    m01 = 0.0f; m02 = s;    m03 = 0.0f;
		m10 = 0.0f; m11 = 1.0f; m12 = 0.0f; m13 = 0.0f;
		m20 = -s;   m21 = 0.0f; m22 = c;    m23 = 0.0f;
		m30 = 0.0f; m31 = 0.0f; m32 = 0.0f; m33 = 1.0f;
	}  
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	沿Z轴旋转指定角度. </summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	///
	/// <param name="angle">	旋转角度. </param>
	/// 
	///     | cos(angle)  -sin(angle)  0  0 |
	/// M = | sin(angle)   cos(angle)  0  0 |
	///     |     0            0       1  0 |
	///     |     0            0       0  1 | 
	/// 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	FORCEINLINE void rotateZ(const FLOAT angle){

		T s = sin(angle);
		T c = cos(angle);

		m00 = c;    m01 = -s;   m02 = 0.0f; m03 = 0.0f;
		m10 = s;    m11 = c;    m12 = 0.0f; m13 = 0.0f;
		m20 = 0.0f; m21 = 0.0f; m22 = 1.0f; m23 = 0.0f;
		m30 = 0.0f; m31 = 0.0f; m32 = 0.0f; m33 = 1.0f;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	沿XYZ轴旋转指定角度. </summary>
	///
	/// <remarks>	Huaru Infrastructure Team, 2011/8/5. </remarks>
	///
	/// <param name="angle">	旋转角度. </param>
	/// 
	///     | cosY * cosZ + sinX * sinY * sinZ     cosY * sinZ - sinX * sinY * cosZ    cosX * sinY   0  |
	/// M = |       -cosX * sinZ                          cosX * cosZ                      sinX      0  |
	///     | sinX * cosY * sinZ - sinY * cosZ    -sinY * sinZ - sinX * cosY * cosZ    cosX * cosY   0  |
	///     |             0                                    0                            0        1  |
	///       
	////////////////////////////////////////////////////////////////////////////////////////////////////
	FORCEINLINE void rotateXYZ(FLOAT x, FLOAT y, FLOAT z)
	{
		FLOAT cosX = cos(x), sinX = sin(x),
			cosY = cos(y), sinY = sin(y),
			cosZ = cos(z), sinZ = sin(z);

		setElements(cosY * cosZ + sinX * sinY * sinZ,   -cosX * sinZ,    sinX * cosY * sinZ - sinY * cosZ,  0,
			cosY * sinZ - sinX * sinY * cosZ,    cosX * cosZ,   -sinY * sinZ - sinX * cosY * cosZ,  0,
			cosX * sinY,                         sinX,           cosX * cosY,                       0,
			0,                                   0,              0,                                 1);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>平移变换矩阵.</summary>
	///
	/// <remarks>Huaru Infrastructure Team, 2011-8-8.</remarks>
	///
	/// <param name="t">平移向量.</param>
	///		沿X轴平移t.x
	///		沿Y轴平移t.y
	///		沿Z轴平移t.z
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE void setTranslation(const TSVector3<T> &t)
	{
		setIdentity();
		m[12] =  t.x;
		m[13] =  t.y;
		m[14] =  t.z;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>平移变换矩阵.</summary>
	///
	/// <remarks>Huaru Infrastructure Team, 2011-8-8.</remarks>
	///
	/// <param name="x">沿X轴平移x.</param>
	/// <param name="y">沿Y轴平移y.</param>
	/// <param name="z">沿Z轴平移z.</param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE void  setTranslation(const FLOAT x,const FLOAT y,const FLOAT z)
	{
		setIdentity();
		m[12] =  x;
		m[13] =  y;
		m[14] =  z;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>平移变换矩阵.</summary>
	///
	/// <remarks>Huaru Infrastructure Team, 2011-8-8.</remarks>
	///
	/// <param name="x">沿X轴平移x.</param>
	/// <param name="y">沿Y轴平移y.</param>
	/// <param name="z">沿Z轴平移z.</param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE void  setTranslation(const DOUBLE x,const DOUBLE y,const DOUBLE z)
	{
		setIdentity();
		m[12] =  x;
		m[13] =  y;
		m[14] =  z;
	}


	FORCEINLINE TSVector3<T> getOrientBX() const
	{
		TSVector3<T> v;
		v.x = m[0];
		v.y = m[4];
		v.z = m[8];

		return v;
	}


	FORCEINLINE TSVector3<T> getOrientNY() const
	{
		TSVector3<T> v;
		v.x = m[1];
		v.y = m[5];
		v.z = m[9];
		return v;
	}
	FORCEINLINE TSVector3<T> getOrientTZ() const
	{
		TSVector3<T> v;
		v.x = m[2];
		v.y = m[6];
		v.z = m[10];
		return v;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>矩阵输出.</summary>
	///
	/// <remarks>Huaru Infrastructure Team, 2011-8-8.</remarks>
	///
	/// <param name="out">  [in,out] The out.</param>
	/// <param name="right">待输出矩阵.</param>
	///
	/// <returns>The shifted result.</returns>
	/// 输出格式为 
	/// TSMatrix4[ m[0]， m[1]， m[2]， m[3]),
	///          [ m[4]， m[5]， m[6]， m[7]),
	///          [ m[8]， m[9]， m[10]，m[11]),
	///          [ m[12]，m[13]，m[14]，m[15])]
	/// 
	////////////////////////////////////////////////////////////////////////////////////////////////////

	friend std::ostream & operator << ( std::ostream & out, const TSMatrix4 & right){
		return STD_COUT <<
			TS_TEXT("TSMatrix4[") << right.m[ 0]	<< TS_TEXT(", ") << right.m[ 1] << TS_TEXT(", ") << right.m[ 2] << TS_TEXT(", ") << right.m[ 3] << TS_TEXT("),\n")
			TS_TEXT("       [") << right.m[ 4]		<< TS_TEXT(", ") << right.m[ 5] << TS_TEXT(", ") << right.m[ 6] << TS_TEXT(", ") << right.m[ 7] << TS_TEXT("),\n")
			TS_TEXT("       [") << right.m[ 8]		<< TS_TEXT(", ") << right.m[ 9] << TS_TEXT(", ") << right.m[10] << TS_TEXT(", ") << right.m[11] << TS_TEXT("),\n")
			TS_TEXT("       [") << right.m[12]		<< TS_TEXT(", ") << right.m[13] << TS_TEXT(", ") << right.m[14] << TS_TEXT(", ") << right.m[15] << TS_TEXT(")]\n");

	}


	//get xyz order rotate value from rotate matrix

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>从旋转矩阵中提取绕x、y、z轴旋转角度.</summary>
	///
	/// <remarks>Huaru Infrastructure Team, 2011-8-8.</remarks>
	///
	/// <param name="rotate">绕x、y、z旋转角度组成的三维向量.</param>
	/// 一次绕x、y、z轴旋转的旋转矩阵是：
	/// 
	///                               
	/// M = rotateZ * rotateY * rotateX
	/// 
	///     | cosZ   sinZ  0  0 |   | cosY  0    sinY  0 |   | 1    0     0    0  |
	///   = | -sinZ  cosZ  0  0 | * |   0   1     0    0 | * | 0   cosX -sinX  0  |
	///     |   0     0    1  0 |   | -sinY  0   cosY  0 |   | 0   sinX  cosX  0  |
	///     |   0     0    0  1 |   |   0   0     0    1 |   | 0    0      0   1  |
	/// 
	///     | cosY*cosZ  -cosX*sinZ + sinX*sinY*cosZ   sinY*sinZ + cosX*sinY*cosZ   ** |
	///   = | cosY*sinZ   cosX*cosZ + sinX*sinY*sinZ  -sinY*cosZ + cosX*sinY*sinZ   ** |
	///     |   -sinY              sinX*cosY                   cosX*cosY            ** |
	///     |     **                  **                           **               ** |
	/// 
	/// 
	////////////////////////////////////////////////////////////////////////////////////////////////////

	FORCEINLINE void getXYZ(TSVector3<FLOAT> &rotate) const {
		rotate.y = asinf(-m[2]);

		if(rotate.y < 3.1415926F/2)
		{
			if(rotate.y > -3.1415926/2)
			{
				rotate.z = atan2f(m[1], m[0]);
				rotate.x = atan2f(m[6], m[10]);
			}
			else
			{
				//not a unique solution (r->x - r->z constant)
				rotate.z = atan2f(-m[4], -m[8]);
				rotate.x = 0;
			}
		}
		else
		{
			//not a unique solution(r->x + r->z constant)
			rotate.z = -atan2f(m[4], m[8]);
			rotate.x = 0;
		}
	}
	union
	{
		struct 
		{
			T m00,m10,m20,m30;
			T m01,m11,m21,m31;
			T m02,m12,m22,m32;
			T m03,m13,m23,m33;
		};
		T  m[16];
	};
};

typedef TSMatrix4<int>    TSMatrix4i;
typedef TSMatrix4<FLOAT>  TSMatrix4f;
typedef TSMatrix4<DOUBLE> TSMatrix4d;

static TSMatrix4f IDENTITY;

TS_END_RT_NAMESPACE

TS_DECLARE_METATYPE(TSMatrix4f);
TS_DECLARE_METATYPE(TSMatrix4d);

#endif // __TSMATRIX4_H__

