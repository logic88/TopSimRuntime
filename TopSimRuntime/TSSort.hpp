#ifndef __TSSORT_72E15C47_F35A_4E4F_BFE1_5B3F619B4E26_H__
#define __TSSORT_72E15C47_F35A_4E4F_BFE1_5B3F619B4E26_H__

TS_BEGIN_RT_NAMESPACE

namespace TSSortUtil
{
	/******************************************************************************\
	*
	* swap
	*
	\******************************************************************************/

	/*
	*/
	template <class Type> FORCEINLINE void swap(Type &v0,Type &v1) {
		Type temp = v0;
		v0 = v1;
		v1 = temp;
	}

	/******************************************************************************\
	*
	* quickSort
	*
	\******************************************************************************/

	/*
	*/
	template <class Type> struct quick_sort_default_compare {
		FORCEINLINE int operator()(const Type &t0,const Type &t1) const {
			return (t0 < t1);
		}
	};

	template <class Type> struct quick_sort_default_compare<Type*> {
		FORCEINLINE int operator()(const Type *t0,const Type *t1) const {
			return (t0 < t1);
		}
	};

	/*
	*/
	template <> struct quick_sort_default_compare<char> {
		FORCEINLINE int operator()(char i0,char i1) const {
			return (i0 < i1);
		}
	};

	template <> struct quick_sort_default_compare<unsigned char> {
		FORCEINLINE int operator()(unsigned char i0,unsigned char i1) const {
			return (i0 < i1);
		}
	};

	/*
	*/
	template <> struct quick_sort_default_compare<short> {
		FORCEINLINE int operator()(short i0,short i1) const {
			return (i0 < i1);
		}
	};

	template <> struct quick_sort_default_compare<unsigned short> {
		FORCEINLINE int operator()(unsigned short i0,unsigned short i1) const {
			return (i0 < i1);
		}
	};

	/*
	*/
	template <> struct quick_sort_default_compare<int> {
		FORCEINLINE int operator()(int i0,int i1) const {
			return (i0 < i1);
		}
	};

	template <> struct quick_sort_default_compare<unsigned int> {
		FORCEINLINE int operator()(unsigned int i0,unsigned int i1) const {
			return (i0 < i1);
		}
	};

	/*
	*/
	template <> struct quick_sort_default_compare<float> {
		FORCEINLINE int operator()(float f0,float f1) const {
			return (f0 < f1);
		}
	};

	/*
	*/
	template <class Type,class Func> struct quick_sort_function_compare {
		quick_sort_function_compare(Func func) : func(func) { }
		FORCEINLINE int operator()(const Type &t0,const Type &t1) const {
			return func(t0,t1);
		}
		Func func;
	};

	/*
	*/
	template <class Type,class Compare> void quick_sort(Type *array,int size,Compare compare) {
		int depth = 1;
		int left_stack[1024];
		int right_stack[1024];
		left_stack[0] = 0;
		right_stack[0] = size - 1;
		while(depth > 0) {
			int left = left_stack[--depth];
			int right = right_stack[depth];
			int l = left;
			int r = right;
			int m = (l + r) >> 1;
			do {
				const Type &c = array[m];
				while(l < right && compare(array[l],c)) l++;
				while(r > left && compare(c,array[r])) r--;
				if(l < r) {
					swap(array[l],array[r]);
					if(m == l) m = r;
					else if(m == r) m = l;
					l++;
					r--;
				} else if(l == r) {
					l++;
					r--;
				}
			} while(l <= r);
			if(left < r) {
				ASSERT(depth < 1024 && "quick_sort(): stack overflow");
				left_stack[depth] = left;
				right_stack[depth++] = r;
			}
			if(l < right) {
				ASSERT(depth < 1024 && "quick_sort(): stack overflow");
				left_stack[depth] = l;
				right_stack[depth++] = right;
			}
		}
	}

	/*
	*/
	template <class Type> void quickSort(Type *array,int size) {
		if(size < 2) return;
		quick_sort_default_compare<Type> compare;
		quick_sort(array,size,compare);
	}

	template <class Type,class Compare> void quickSort(Type *array,int size,Compare compare) {
		if(size < 2) return;
		quick_sort(array,size,compare);
	}

	template <class Type,class A0,class A1> void quickSort(Type *array,int size,int (*func)(A0,A1)) {
		if(size < 2) return;
		quick_sort_function_compare<Type,int (*)(A0,A1)> compare(func);
		quick_sort(array,size,compare);
	}

	/*
	*/
	template <class Iterator> void quickSort(Iterator begin,Iterator end) {
		int size = end - begin;
		if(size < 2) return;
		quick_sort_default_compare<typename Iterator::DataType> compare;
		quick_sort(&(*begin),size,compare);
	}

	template <class Iterator,class Compare> void quickSort(Iterator begin,Iterator end,Compare compare) {
		int size = end - begin;
		if(size < 2) return;
		quick_sort(&(*begin),size,compare);
	}

	template <class Iterator,class A0,class A1> void quickSort(Iterator begin,Iterator end,int (*func)(A0,A1)) {
		int size = end - begin;
		if(size < 2) return;
		quick_sort_function_compare<typename Iterator::DataType,int (*)(A0,A1)> compare(func);
		quick_sort(&(*begin),size,compare);
	}

	/******************************************************************************\
	*
	* radixSort
	*
	\******************************************************************************/

	/*
	*/
	template <class Type> struct radix_sort_default_compare;

	/*
	*/
	template <class Type> struct radix_sort_default_compare<Type*> {
		typedef size_t Hash;
		FORCEINLINE Hash operator()(Type *p) const {
			return (size_t)p;
		}
	};

	/*
	*/
	template <> struct radix_sort_default_compare<char> {
		typedef unsigned char Hash;
		FORCEINLINE Hash operator()(char i) const {
			return i ^ 0x80;
		}
	};

	template <> struct radix_sort_default_compare<unsigned char> {
		typedef unsigned char Hash;
		FORCEINLINE Hash operator()(unsigned char i) const {
			return i;
		}
	};

	/*
	*/
	template <> struct radix_sort_default_compare<short> {
		typedef unsigned short Hash;
		FORCEINLINE Hash operator()(short i) const {
			return i ^ 0x8000;
		}
	};

	template <> struct radix_sort_default_compare<unsigned short> {
		typedef unsigned short Hash;
		FORCEINLINE Hash operator()(unsigned short i) const {
			return i;
		}
	};

	/*
	*/
	template <> struct radix_sort_default_compare<int> {
		typedef unsigned int Hash;
		FORCEINLINE Hash operator()(int i) const {
			return i ^ 0x80000000;
		}
	};

	template <> struct radix_sort_default_compare<unsigned int> {
		typedef unsigned int Hash;
		FORCEINLINE Hash operator()(unsigned int i) const {
			return i;
		}
	};

	/*
	*/
	template <> struct radix_sort_default_compare<long long> {
		typedef unsigned long long Hash;
		FORCEINLINE Hash operator()(long long i) const {
			return i ^ 0x8000000000000000LL;
		}
	};

	template <> struct radix_sort_default_compare<unsigned long long> {
		typedef unsigned long long Hash;
		FORCEINLINE Hash operator()(unsigned long long i) const {
			return i;
		}
	};

	/*
	*/
	template <> struct radix_sort_default_compare<float> {
		typedef unsigned int Hash;
		FORCEINLINE Hash operator()(float f) const {
			union IntFloat {
				unsigned int i;
				float f;
			};
			IntFloat hash;
			hash.f = f;
			if(hash.i & 0x80000000) return hash.i ^ 0xffffffff;
			return hash.i ^ 0x80000000;
		}
	};

	/*
	*/
	template <class Type,class Ret,class Func> struct radix_sort_function_compare {
		radix_sort_function_compare(Func func) : func(func) { }
		typedef Ret Hash;
		FORCEINLINE Hash operator()(const Type &t) const {
			return func(t);
		}
		Func func;
	};

	/*
	*/
	template <class Type,class Compare> class RadixSort {

	public:

		RadixSort(Type *array,int size,Compare compare) {
			reserve(size);
			for(int i = 0; i < size; i++) {
				src[i].hash = compare(array[i]);
				src[i].index = i;
				data[i] = array[i];
			}
			int index[256];
			int distribution[256];
#ifdef IS_BIG_ENDIAN
			for(int j = (int)sizeof(Hash) - 1; j >= 0; j--) {
#else
			for(int j = 0; j < (int)sizeof(Hash); j++) {
#endif
				for(int i = 0; i < 256; i += 4) {
					distribution[i + 0] = 0;
					distribution[i + 1] = 0;
					distribution[i + 2] = 0;
					distribution[i + 3] = 0;
				}
				for(int i = 0; i < size; i++) {
					distribution[get_byte(src[i].hash,j)]++;
				}
				index[0] = 0;
				index[1] = distribution[0];
				index[2] = index[1] + distribution[1];
				index[3] = index[2] + distribution[2];
				for(int i = 4; i < 256; i += 4) {
					index[i + 0] = index[i - 1] + distribution[i - 1];
					index[i + 1] = index[i + 0] + distribution[i + 0];
					index[i + 2] = index[i + 1] + distribution[i + 1];
					index[i + 3] = index[i + 2] + distribution[i + 2];
				}
				for(int i = 0; i < size; i++) {
					dest[index[get_byte(src[i].hash,j)]++] = src[i];
				}
				swap(src,dest);
			}
			for(int i = 0; i < size; i++) {
				array[i] = data[src[i].index];
			}
		}

		static void clear() {
			capacity = 0;
			delete [] src;
			delete [] dest;
			delete [] data;
			src = 0;
			dest = 0;
			data = 0;
		}

		static void allocate(int size) {
			if(size <= capacity) return;
			capacity = size;
			delete [] src;
			delete [] dest;
			delete [] data;
			src = new Data[capacity];
			dest = new Data[capacity];
			data = new Type[capacity];
		}

		static void reserve(int size) {
			if(size <= capacity) return;
			capacity = size * 2;
			delete [] src;
			delete [] dest;
			delete [] data;
			src = new Data[capacity];
			dest = new Data[capacity];
			data = new Type[capacity];
		}

	private:

		typedef typename Compare::Hash Hash;

		FORCEINLINE int get_byte(const Hash &hash,int i) const {
			return *((const unsigned char*)&hash + i);
		}

		struct Data {
			Hash hash;
			int index;
		};

		static int capacity;
		static Data *src;
		static Data *dest;
		static Type *data;
	};

	/*
	*/
	template <class Type,class Compare> int RadixSort<Type,Compare>::capacity = 0;
	template <class Type,class Compare> typename RadixSort<Type,Compare>::Data *RadixSort<Type,Compare>::src = 0;
	template <class Type,class Compare> typename RadixSort<Type,Compare>::Data *RadixSort<Type,Compare>::dest = 0;
	template <class Type,class Compare> Type *RadixSort<Type,Compare>::data = 0;

	/*
	*/
	template <class Type> void radixSort(Type *array,int size) {
		if(size < 2) return;
		radix_sort_default_compare<Type> compare;
		RadixSort<Type,radix_sort_default_compare<Type> > radix_sort(array,size,compare);
	}

	template <class Type,class Compare> void radixSort(Type *array,int size,Compare compare) {
		if(size < 2) return;
		RadixSort<Type,Compare> radix_sort(array,size,compare);
	}

	template <class Type,class Ret,class A0,class A1> void radixSort(Type *array,int size,Ret (*func)(A0,A1)) {
		if(size < 2) return;
		radix_sort_function_compare<Type,Ret,Ret (*)(A0,A1)> compare(func);
		RadixSort<Type,radix_sort_function_compare<Type,Ret,Ret (*)(A0,A1)> > radix_sort(array,size,compare);
	}

	/*
	*/
	template <class Iterator> void radixSort(Iterator begin,Iterator end) {
		int size = end - begin;
		if(size < 2) return;
		radix_sort_default_compare<typename Iterator::DataType> compare;
		RadixSort<typename Iterator::DataType,radix_sort_default_compare<typename Iterator::DataType> > radix_sort(&(*begin),size,compare);
	}

	template <class Iterator,class Compare> void radixSort(Iterator begin,Iterator end,Compare compare) {
		int size = end - begin;
		if(size < 2) return;
		RadixSort<typename Iterator::DataType,Compare> radix_sort(&(*begin),size,compare);
	}

	template <class Iterator,class Ret,class A0,class A1> void radixSort(Iterator begin,Iterator end,Ret (*func)(A0,A1)) {
		int size = end - begin;
		if(size < 2) return;
		radix_sort_function_compare<typename Iterator::DataType,Ret,Ret (*)(A0,A1)> compare(func);
		RadixSort<typename Iterator::DataType,radix_sort_function_compare<typename Iterator::DataType,Ret,Ret (*)(A0,A1)> > radix_sort(&(*begin),size,compare);
	}

}

TS_END_RT_NAMESPACE

#endif // __TSSORT_72E15C47_F35A_4E4F_BFE1_5B3F619B4E26_H__

