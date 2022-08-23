#ifndef __TSARRAY_47935F57_49D7_4639_B095_993196B10273_H__
#define __TSARRAY_47935F57_49D7_4639_B095_993196B10273_H__

TS_BEGIN_RT_NAMESPACE

/*
 */
template <class Type,class Counter = int> class TSArray {
		
	public:
		
		// TSArray::Iterator
		class Iterator {
				
			private:
				
				friend class TSArray;
				
				Iterator(Type *ptr) : ptr(ptr) { }
				
			public:
				
				typedef Type DataType;
				
				Iterator() : ptr(0) { }
				Iterator(const Iterator &it) : ptr(it.ptr) { }
				~Iterator() { }
				
				FORCEINLINE Iterator &operator=(const Iterator &it) {
					ptr = it.ptr;
					return *this;
				}
				
				FORCEINLINE int operator==(const Iterator &it) const { return ptr == it.ptr; }
				FORCEINLINE int operator!=(const Iterator &it) const { return ptr != it.ptr; }
				
				FORCEINLINE Type &operator*() { return *ptr; }
				FORCEINLINE const Type &operator*() const { return *ptr; }
				
				FORCEINLINE Type *operator->() { return ptr; }
				FORCEINLINE const Type *operator->() const { return ptr; }
				
				FORCEINLINE Type &get() { return *ptr; }
				FORCEINLINE const Type &get() const { return *ptr; }
				
				FORCEINLINE Iterator &operator++() {
					ptr++;
					return *this;
				}
				FORCEINLINE Iterator operator++(int) {
					Iterator temp = *this;
					ptr++;
					return temp;
				}
				
				Iterator operator+(int n) const { return Iterator(ptr + n); }
				Iterator operator-(int n) const { return Iterator(ptr - n); }
				Iterator &operator+=(int n) { ptr += n; return *this; }
				Iterator &operator-=(int n) { ptr -= n; return *this; }
				
				int operator-(const Iterator &it) const { return (int)(ptr - it.ptr); }
				
			private:
				
				Type *ptr;
		};
		
		// TSArray
		TSArray() : length(0), capacity(0), data(0) {
			
		}
		TSArray(const TSArray &v) : length(0), capacity(0), data(0) {
			append(v);
		}
		TSArray(const Type &t,int size) : length(0), capacity(0), data(0) {
			resize(size);
			for(int i = 0; i < size; i++) {
				data[i] = t;
			}
		}
		TSArray(const Type *v,int size) : length(0), capacity(0), data(0) {
			append(v,size);
		}
		explicit TSArray(int size) : length(0), capacity(0), data(0) {
			resize(size);
		}
		~TSArray() {
			destroy();
		}
		
		TSArray &operator=(const TSArray &v) {
			if(this == &v) return *this;
			resize(v.length);
			for(int i = 0; i < length; i++) {
				data[i] = v.data[i];
			}
			return *this;
		}
		void swap(TSArray &v) {
			if(this == &v) return;
			if((capacity & DYNAMIC_FLAG) && (v.capacity & DYNAMIC_FLAG)) {
				Counter l = length; length = v.length; v.length = l;
				Counter c = capacity; capacity = v.capacity; v.capacity = c;
				Type *d = data; data = v.data; v.data = d;
			} else {
				TSArray<Type> temp = *this;
				*this = v;
				v = temp;
			}
		}
		
		FORCEINLINE Type &operator[](int index) {
			ASSERT((unsigned int)index < (unsigned int)length && "TSArray::operator[](): bad index");
			return data[index];
		}
		FORCEINLINE const Type &operator[](int index) const {
			ASSERT((unsigned int)index < (unsigned int)length && "TSArray::operator[](): bad index");
			return data[index];
		}
		
		FORCEINLINE void set(int index,const Type &t) {
			ASSERT((unsigned int)index < (unsigned int)length && "TSArray::set(): bad index");
			data[index] = t;
		}
		FORCEINLINE Type &get(int index) {
			ASSERT((unsigned int)index < (unsigned int)length && "TSArray::get(): bad index");
			return data[index];
		}
		FORCEINLINE const Type &get(int index) const {
			ASSERT((unsigned int)index < (unsigned int)length && "TSArray::get(): bad index");
			return data[index];
		}
		
		FORCEINLINE Type *get() { return data; }
		FORCEINLINE const Type *get() const { return data; }
		
		FORCEINLINE Iterator begin() const { return Iterator(data); }
		FORCEINLINE Iterator back() const { return Iterator(data + length - 1); }
		FORCEINLINE Iterator end() const { return Iterator(data + length); }
		
		FORCEINLINE int size() const { return length; }
		FORCEINLINE int empty() const { return (length == 0); }
		
		void resize(int size) {
			allocate(size);
			length = size;
		}
		
		void allocate(int size) {
			if(size <= (capacity & CAPACITY_MASK)) return;
			int dynamic = (capacity & DYNAMIC_FLAG);
			capacity = size | DYNAMIC_FLAG;
			Type *new_data = new Type[size];
			for(int i = 0; i < length; i++) {
				new_data[i] = data[i];
			}
			if(dynamic) delete [] data;
			data = new_data;
		}
		
		void reserve(int size) {
			if(size <= (capacity & CAPACITY_MASK)) return;
			int dynamic = (capacity & DYNAMIC_FLAG);
			capacity = (size * 2) | DYNAMIC_FLAG;
			Type *new_data = new Type[size * 2];
			for(int i = 0; i < length; i++) {
				new_data[i] = data[i];
			}
			if(dynamic) delete [] data;
			data = new_data;
		}
		
		FORCEINLINE void clear() {
			length = 0;
		}
		void destroy() {
			int dynamic = (capacity & DYNAMIC_FLAG);
			length = 0;
			capacity = 0;
			if(dynamic) delete [] data;
			data = 0;
		}
		
		template <class T> Iterator find(const T &t) const {
			for(int i = 0; i < length; i++) {
				if(data[i] == t) return Iterator(data + i);
			}
			return Iterator(data + length);
		}
		template <class T> int findIndex(const T &t) const {
			for(int i = 0; i < length; i++) {
				if(data[i] == t) return i;
			}
			return -1;
		}
		template <class T> int leftIndex(const T &t) const {
			if(length == 0 || t < data[0]) return -1;
			if(data[length - 1] < t) return length - 1;
			int left = 0;
			int right = length - 1;
			while(left <= right) {
				int middle = (left + right) >> 1;
				if(data[middle] < t) left = middle + 1;
				else if(t < data[middle]) right = middle - 1;
				else return middle - 1;
			}
			return left - 1;
		}
		template <class T> int rightIndex(const T &t) const {
			if(length == 0 || data[length - 1] < t) return -1;
			if(t < data[0]) return 0;
			int left = 0;
			int right = length - 1;
			while(left <= right) {
				int middle = (left + right) >> 1;
				if(data[middle] < t) left = middle + 1;
				else if(t < data[middle]) right = middle - 1;
				else return middle;
			}
			return right + 1;
		}
		
		FORCEINLINE Type &append() {
			if(length + 1 > (capacity & CAPACITY_MASK)) {
				reserve(length + 1);
			}
			static Type t;
			data[(int)length] = t;
			return data[(int)(length++)];
		}
		FORCEINLINE void append(const Type &t) {
			if(length + 1 > (capacity & CAPACITY_MASK)) {
				reserve(length + 1);
			}
			data[(int)(length++)] = t;
		}
		FORCEINLINE void appendFast(const Type &t) {
			data[(int)(length++)] = t;
		}
		void append(int pos,const Type &t) {
			ASSERT((unsigned int)pos <= (unsigned int)length && "TSArray::append(): bad position");
			if(length + 1 <= (capacity & CAPACITY_MASK)) {
				for(int i = length - 1; i >= pos; i--) {
					data[i + 1] = data[i];
				}
				data[pos] = t;
				length++;
				return;
			}
			int dynamic = (capacity & DYNAMIC_FLAG);
			capacity = (length * 2 + 1) | DYNAMIC_FLAG;
			Type *new_data = new Type[length * 2 + 1];
			for(int i = 0; i < pos; i++) {
				new_data[i] = data[i];
			}
			new_data[pos] = t;
			for(int i = pos; i < length; i++) {
				new_data[i + 1] = data[i];
			}
			if(dynamic) delete [] data;
			data = new_data;
			length++;
		}
		FORCEINLINE void append(const Iterator &it,const Type &t) {
			append(it - begin(),t);
		}
		
		void append(const TSArray &v) {
			if(length + v.length > (capacity & CAPACITY_MASK)) {
				reserve(length + v.length);
			}
			for(int i = 0; i < v.length; i++) {
				data[(int)(length++)] = v.data[i];
			}
		}
		void append(const Type *v,int size) {
			if(length + size > (capacity & CAPACITY_MASK)) {
				reserve(length + size);
			}
			for(int i = 0; i < size; i++) {
				data[(int)(length++)] = v[i];
			}
		}
		
		FORCEINLINE void remove() {
			ASSERT(length > 0 && "TSArray::remove(): bad length");
			length--;
		}
		void remove(int pos,int size = 1) {
			ASSERT((unsigned int)pos < (unsigned int)length && "TSArray::remove(): bad position");
			ASSERT(size >= 0 && pos + size <= length && "TSArray::remove(): bad size");
			for(int i = pos; i < length - size; i++) {
				data[i] = data[i + size];
			}
			length -= size;
		}
		FORCEINLINE void remove(const Iterator &it) {
			remove(it - begin());
		}
		
		FORCEINLINE void removeFast(int pos) {
			ASSERT((unsigned int)pos < (unsigned int)length && "TSArray::removeFast(): bad position");
			data[pos] = data[(int)(length - 1)];
			length--;
		}
		FORCEINLINE void removeFast(const Iterator &it) {
			removeFast(it - begin());
		}
		
	protected:
		
		enum {
			DYNAMIC_FLAG = (Counter)(1 << (sizeof(Counter) * 8 - 1)),
			CAPACITY_MASK = ~(Counter)DYNAMIC_FLAG,
		};
		
		Counter length;
		Counter capacity;
		Type *data;
};

/*
 */
template <class Type,int Capacity = 128,class Counter = int> class TSArrayStack : public TSArray<Type,Counter> {
		
	public:
		
		TSArrayStack() {
			TSArray<Type,Counter>::capacity = Capacity;
			TSArray<Type,Counter>::data = stack_data;
		}
		explicit TSArrayStack(int size) {
			TSArray<Type,Counter>::capacity = Capacity;
			TSArray<Type,Counter>::data = stack_data;
			TSArray<Type,Counter>::resize(size);
		}
		TSArrayStack(const TSArray<Type,Counter> &v) {
			TSArray<Type,Counter>::capacity = Capacity;
			TSArray<Type,Counter>::data = stack_data;
			TSArray<Type,Counter>::append(v);
		}
		TSArrayStack(const Type &t,int size) {
			TSArray<Type,Counter>::capacity = Capacity;
			TSArray<Type,Counter>::data = stack_data;
			TSArray<Type,Counter>::resize(size);
			for(int i = 0; i < size; i++) {
				TSArray<Type,Counter>::data[i] = t;
			}
		}
		TSArrayStack(const Type *v,int size) {
			TSArray<Type,Counter>::capacity = Capacity;
			TSArray<Type,Counter>::data = stack_data;
			TSArray<Type,Counter>::append(v,size);
		}
		~TSArrayStack() { }
		
		TSArrayStack &operator=(const TSArrayStack &v) {
			if(this == &v) return *this;
			TSArray<Type,Counter>::resize(v.length);
			for(int i = 0; i < TSArray<Type,Counter>::length; i++) {
				TSArray<Type,Counter>::data[i] = v.data[i];
			}
			return *this;
		}
		TSArrayStack &operator=(const TSArray<Type,Counter> &v) {
			if(this == &v) return *this;
			TSArray<Type,Counter>::resize(v.size());
			for(int i = 0; i < TSArray<Type,Counter>::length; i++) {
				TSArray<Type,Counter>::data[i] = v[i];
			}
			return *this;
		}
		
		void destroy() {
			TSArray<Type,Counter>::destroy();
			TSArray<Type,Counter>::capacity = Capacity;
			TSArray<Type,Counter>::data = stack_data;
		}
		
	private:
		
		Type stack_data[Capacity];
};

template<class Type,std::size_t Count>
struct TSFixedArray
{
	typedef const Type & const_reference;
	typedef std::size_t size_type;
	typedef Type & reference;

	FORCEINLINE const_reference operator[](size_type _Pos) const
	{
		return Data[_Pos];
	}

	FORCEINLINE reference operator[](size_type _Pos)
	{
		return Data[_Pos];
	}

	Type Data[Count];
};

template<class Type,std::size_t Count1D,std::size_t Count2D>
struct TSFixed2DArray
{
	typedef TSFixedArray<Type,Count2D> TSFixed1DArray;
	typedef const TSFixed1DArray & const_reference;
	typedef std::size_t size_type;
	typedef TSFixed1DArray & reference;
	typedef TSFixed1DArray element_type;

	FORCEINLINE const_reference operator[](size_type _Pos) const
	{
		return Data[_Pos];
	}

	FORCEINLINE reference operator[](size_type _Pos)
	{
		return Data[_Pos];
	}

	element_type Data[Count1D];
};

TS_END_RT_NAMESPACE

#endif // __TSARRAY_47935F57_49D7_4639_B095_993196B10273_H__

