#ifndef __TSKEYPAIR_H__
#define __TSKEYPAIR_H__

TS_BEGIN_RT_NAMESPACE

template<typename KeyType,typename ValueType>
class TSKeyPair
{
public:
	TSKeyPair()
		:_Key(),_Value()
	{
	
	}
	TSKeyPair(const KeyType & Key,const ValueType & Value)
		:_Key(Key),_Value(Value)
	{
		
	}

	TSKeyPair(const TSKeyPair & r)
		:_Key(r._Key),_Value(r._Value)
	{

	}

	FORCEINLINE TSKeyPair & operator=(const TSKeyPair & r)
	{
		_Key = r._Key;
		_Value = r._Value;

		return *this;
	}

	FORCEINLINE bool operator==(const TSKeyPair & r) const
	{
		return (_Key == r._Key && _Value == r._Value);
	}

	FORCEINLINE bool operator<(const TSKeyPair & r) const
	{
		if (_Key == r._Key)
		{
			return _Value < r._Value;
		}

		return (_Key < r._Key);
	}

	FORCEINLINE const ValueType & GetValue() const 
	{ 
		return _Value; 
	}

	FORCEINLINE void SetValue(const ValueType & Value) 
	{ 
		_Value = Value; 
	}
	FORCEINLINE const KeyType & GetKey() const 
	{ 
		return _Key; 
	}

	FORCEINLINE void SetKey(const KeyType & Key) 
	{ 
		_Key = Key; 
	}
private:
	KeyType _Key;
	ValueType _Value;
};

template<typename KeyType,typename ValueType>
class TSKeyPairHash
{
public:
	enum 
	{
		bucket_size = 4,
		min_buckets = 8
	};

    FORCEINLINE std::size_t operator()( const TSKeyPair<KeyType,ValueType> & key ) const
	{
		return boost::hash_value(key.GetKey()) ^ boost::hash_value(key.GetValue());

	};
	FORCEINLINE bool operator()( const TSKeyPair<KeyType,ValueType> & key1,const TSKeyPair<KeyType,ValueType> & key2 ) const
	{
		return key1 < key2;
	}
};

template<class T>
struct TSObjIndex : public TSKeyPair< UINT32,boost::shared_ptr<T> >
{
	typedef TSKeyPair< UINT32,boost::shared_ptr<T> > PairType;

	TSObjIndex(){};
	TSObjIndex(UINT32 Key,boost::shared_ptr<T> Value) 
		: TSKeyPair< UINT32,boost::shared_ptr<T> >(Key,Value){}

	bool operator! () const { return !PairType::GetValue(); }
	operator bool() const { return (bool)PairType::GetValue(); }
	boost::shared_ptr<T> operator->(){ return PairType::GetValue(); }
	boost::shared_ptr<T> operator->() const { return PairType::GetValue(); }
};

TS_END_RT_NAMESPACE

#endif // __TSKEYPAIR_H__

