#ifndef __TSBYTEORDER_H__
#define __TSBYTEORDER_H__
#include "TopSimCommTypes.h"

TS_BEGIN_RT_NAMESPACE

#define SWAP_BYTE_USING_TEMP(a,b,t) t=*(a); *(a)=*(b); *(b)=t
FORCEINLINE void SwapInt16Force (INT16 *PtrValue)
{
	unsigned char temp;
	unsigned char *ptr = (unsigned char *) PtrValue;

	SWAP_BYTE_USING_TEMP(ptr, ptr+1, temp);
}
FORCEINLINE void SwapInt32Force (INT32 *PtrValue)
{
	unsigned char temp;
	unsigned char *ptr = (unsigned char *) PtrValue;

	SWAP_BYTE_USING_TEMP(ptr, ptr+3, temp);
	SWAP_BYTE_USING_TEMP(ptr+1, ptr+2, temp);
}
FORCEINLINE void SwapInt64Force (INT64 *PtrValue)
{
	unsigned char temp; 
	unsigned char *ptr = (unsigned char *) PtrValue;

	SWAP_BYTE_USING_TEMP(ptr, ptr+7, temp);
	SWAP_BYTE_USING_TEMP(ptr+1, ptr+6, temp);
	SWAP_BYTE_USING_TEMP(ptr+2, ptr+5, temp);
	SWAP_BYTE_USING_TEMP(ptr+3, ptr+4, temp);
}

FORCEINLINE void SwapUInt16Force (UINT16 *PtrValue)
{
	unsigned char temp;
	unsigned char *ptr = (unsigned char *) PtrValue;

	SWAP_BYTE_USING_TEMP(ptr, ptr+1, temp);
}
FORCEINLINE void SwapUInt32Force (UINT32 *PtrValue)
{
	unsigned char temp;
	unsigned char *ptr = (unsigned char *) PtrValue;

	SWAP_BYTE_USING_TEMP(ptr, ptr+3, temp);
	SWAP_BYTE_USING_TEMP(ptr+1, ptr+2, temp);
}
FORCEINLINE void SwapUInt64Force (UINT64 *PtrValue)
{
	unsigned char temp; 
	unsigned char *ptr = (unsigned char *) PtrValue;

	SWAP_BYTE_USING_TEMP(ptr, ptr+7, temp);
	SWAP_BYTE_USING_TEMP(ptr+1, ptr+6, temp);
	SWAP_BYTE_USING_TEMP(ptr+2, ptr+5, temp);
	SWAP_BYTE_USING_TEMP(ptr+3, ptr+4, temp);
}

FORCEINLINE void SwapFloatForce (FLOAT *PtrValue)
{
	unsigned char temp;
	unsigned char *ptr = (unsigned char *) PtrValue;

	SWAP_BYTE_USING_TEMP(ptr, ptr+3, temp);
	SWAP_BYTE_USING_TEMP(ptr+1, ptr+2, temp);
}
FORCEINLINE void SwapDoubleForce (DOUBLE *PtrValue)
{
	unsigned char temp; 
	unsigned char *ptr = (unsigned char *) PtrValue;

	SWAP_BYTE_USING_TEMP(ptr, ptr+7, temp);
	SWAP_BYTE_USING_TEMP(ptr+1, ptr+6, temp);
	SWAP_BYTE_USING_TEMP(ptr+2, ptr+5, temp);
	SWAP_BYTE_USING_TEMP(ptr+3, ptr+4, temp);
}

#ifdef IS_BIG_ENDIAN
#define SwapInt16(a)		SwapInt16Force(&a) 
#define SwapInt32(a) 		SwapInt32Force(&a) 
#define SwapInt64(a) 		SwapInt64Force(&a) 
#define SwapUInt16(a)		SwapUInt16Force(&a)
#define SwapUInt32(a)		SwapUInt32Force(&a)
#define SwapUInt64(a)		SwapUInt64Force(&a)
#define SwapFloat(a) 		SwapFloatForce(&a) 
#define SwapDouble(a)		SwapDoubleForce(&a)
#else
#define SwapInt16(a)	a
#define SwapInt32(a) 	a
#define SwapInt64(a) 	a
#define SwapUInt16(a)	a
#define SwapUInt32(a)	a
#define SwapUInt64(a)	a
#define SwapFloat(a) 	a
#define SwapDouble(a)	a
#endif

#define AssignSwapInt16(a,val)		\
	a = val;\
	SwapInt16(a); 
#define AssignSwapInt32(a,val)		\
	a = val;\
	SwapInt32(a);  
#define AssignSwapInt64(a,val)		\
	a = val;\
	SwapInt64(a);  
#define AssignSwapUInt16(a,val)		\
	a = val;\
	SwapUInt16(a); 
#define AssignSwapUInt32(a,val)		\
	a = val;\
	SwapUInt32(a); 
#define AssignSwapUInt64(a,val)		\
	a = val;\
	SwapUInt64(a); 
#define AssignSwapFloat(a,val)		\
	a = val;\
	SwapFloat(a);  
#define AssignSwapDouble(a,val)		\
	a = val;\
	SwapDouble(a); 

TS_END_RT_NAMESPACE

#endif // __TSBYTEORDER_H__

