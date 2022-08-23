#ifndef __TSFIXEDSTRING_H__
#define __TSFIXEDSTRING_H__

TS_BEGIN_RT_NAMESPACE

template< int MAX_STR_LEN >
class TSFixedString
{
public:
	TSFixedString()
	{
		Empty();
	}
	TSFixedString(TSChar ch)
	{
		_Buffer[0] = ch;
        _Buffer[1] = TS_TEXT('\0');
	}
	TSFixedString(const TSChar * lpsz)
	{      
		ASSERT(_tcslen(lpsz) <= MAX_STR_LEN);

		if( _tcslen(lpsz)<MAX_STR_LEN ) 
		{
			_tcscpy(_Buffer,lpsz); 
		}
		else 
		{
			Empty();
		}
	}
	TSFixedString(const TSString & Str)
	{      
		if( Str.length()<=MAX_STR_LEN ) 
		{
			_tcscpy(_Buffer,Str.c_str());
		}
		else 
		{
			Empty();
		}
	}
	template<int OTHER_MAX_STR_LEN>
	TSFixedString(const TSFixedString<OTHER_MAX_STR_LEN> & Str)
	{      
		if( Str.GetLength()<=MAX_STR_LEN ) 
		{
			_tcscpy(_Buffer,(const TSChar *)Str);
		}
		else 
		{
			Empty();
		}
	}

	int GetLength() const 
	{ 
		return _tcslen(_Buffer); 
	}

	bool IsEmpty() const 
	{ 
        return _Buffer[0]==TS_TEXT('\0');
	}

	void Empty() 
	{ 
        _Buffer[0]=TS_TEXT('\0');
	}

	TSChar GetAt(int nIndex) const
	{
		ASSERT( nIndex>=0 && nIndex<=MAX_STR_LEN );
		return _Buffer[nIndex];
	}
	TSChar operator[] (int nIndex) const 
	{ 
		ASSERT( nIndex>=0 && nIndex<=MAX_STR_LEN );
		return _Buffer[nIndex];
	}

	void SetAt(int nIndex, TSChar ch)
	{
		ASSERT( nIndex>=0 && nIndex<=MAX_STR_LEN );
		_Buffer[nIndex] = ch;
	}

	operator const TSChar *() const 
	{ 
		return _Buffer; 
	}

	operator TSString() const 
	{ 
		return TSString(_Buffer); 
	}

	template<int OTHER_MAX_STR_LEN>
	operator TSFixedString<OTHER_MAX_STR_LEN>() const
	{
		return TSFixedString<OTHER_MAX_STR_LEN>(_Buffer);
	}

	const TSFixedString & operator=(TSChar ch)
	{
		_Buffer[0] = ch;
		_Buffer[1] = TEXT('\0');
		return *this;
	}

	TSFixedString & operator=(const TSChar * lpsz)
	{      
		if( _tcslen(lpsz)<=MAX_STR_LEN ) 
		{
			_tcscpy(_Buffer,lpsz);
		}
		else 
		{
			Empty();
		}
		return *this;
	}

	TSFixedString & operator=(const TSString & Str)
	{      
		if( Str.length()<=MAX_STR_LEN ) 
		{
			_tcscpy(_Buffer,Str.c_str());
		}
		else 
		{
			Empty();
		}
		return *this;
	}

	template<int OTHER_MAX_STR_LEN>
	TSFixedString & operator=(const TSFixedString<OTHER_MAX_STR_LEN> & Str)
	{      
		if( Str.GetLength()<=MAX_STR_LEN ) 
		{
			_tcscpy(_Buffer,(const TSChar *)Str);
		}
		else 
		{
			Empty();
		}

		return *this;
	}

	TSFixedString & operator+=(TSChar ch)
	{      
		if( GetLength() <= MAX_STR_LEN-1 ) 
		{
			TSChar szTmp[2] = {ch,0};
			_tcscat(_Buffer,szTmp);
		}
		return *this;
	}

	TSFixedString & operator+=(const TSChar * str)
	{
		if( _tcslen(_Buffer) + _tcslen(str) <= MAX_STR_LEN )
		{
			_tcscat( _Buffer, str );
		}

		return *this;
	}
	TSFixedString & operator+=(const TSString & Str)
	{
		if( _tcslen(_Buffer) + Str.length() <= MAX_STR_LEN )
		{
			_tcscat( _Buffer, Str.c_str() );
		}
		return *this;
	}

	template<int OTHER_MAX_STR_LEN>
	TSFixedString & operator+=(const TSFixedString<OTHER_MAX_STR_LEN> & Str)
	{
		if( _tcslen(_Buffer) + Str.GetLength() <= MAX_STR_LEN )
		{
			_tcscat( _Buffer, (const TSChar *)Str );
		}

		return *this;
	}

	TSFixedString  operator+(TSChar ch) const
	{     
		TSFixedString str;
		if( GetLength() <= MAX_STR_LEN-1 ) 
		{
			TSChar szTmp[2] = {ch,0};
			_tcscat(str._Buffer,szTmp);
		}
		return str;
	}

	TSFixedString operator+(const TSChar * lpsz) const
	{    
		TSFixedString str;

		if( GetLength() + _tcslen(lpsz) <= MAX_STR_LEN )
		{
			str = *this;

			_tcscat(str._Buffer,lpsz);
		}

		return str;
	}

	TSFixedString operator+(const TSString & Str) const
	{    
		TSFixedString str;

		if( GetLength() + Str.length() <= MAX_STR_LEN )
		{
			_tcscat(str._Buffer,Str.c_str());
		}
		return str;
	}

	template<int OTHER_MAX_STR_LEN>
	TSFixedString operator+(const TSFixedString<OTHER_MAX_STR_LEN> & Str) const
	{    
		TSFixedString str;

		if( GetLength() + Str.GetLength() <= MAX_STR_LEN )
		{
			_tcscat(str._Buffer,(const TSChar *)Str);
		}
		return str;
	}

	bool operator == (const TSChar * str) const 
	{ 
		return (Compare(str) == 0); 
	}

	bool operator == (const TSString & str) const 
	{ 
		return (Compare(str) == 0); 
	}

	template<int OTHER_MAX_STR_LEN>
	bool operator ==  (const TSFixedString<OTHER_MAX_STR_LEN> & str) const 
	{ 
		return (Compare(str) ==  0); 
	}

	bool operator != (const TSChar * str) const 
	{ 
		return (Compare(str) != 0); 
	}

	bool operator != (const TSString & str) const 
	{ 
		return (Compare(str) != 0); 
	}

	template<int OTHER_MAX_STR_LEN>
	bool operator !=  (const TSFixedString<OTHER_MAX_STR_LEN> & str) const 
	{ 
		return (Compare(str) !=  0); 
	}

	bool operator <= (const TSChar * str) const 
	{ 
		return (Compare(str) <= 0); 
	}

	bool operator <= (const TSString & str) const 
	{ 
		return (Compare(str) <= 0); 
	}

	template<int OTHER_MAX_STR_LEN>
	bool operator <=  (const TSFixedString<OTHER_MAX_STR_LEN> & str) const 
	{ 
		return (Compare(str) <= 0); 
	}

	bool operator <  (const TSChar * str) const 
	{ 
		return (Compare(str) <  0); 
	}

	bool operator <  (const TSString & str) const 
	{ 
		return (Compare(str) <  0); 
	}

	template<int OTHER_MAX_STR_LEN>
	bool operator <  (const TSFixedString<OTHER_MAX_STR_LEN> & str) const 
	{ 
		return (Compare(str) <  0); 
	}

	bool operator >= (const TSChar * str) const 
	{ 
		return (Compare(str) >= 0); 
	}

	bool operator >= (const TSString & str) const 
	{ 
		return (Compare(str) >= 0); 
	}

	template<int OTHER_MAX_STR_LEN>
	bool operator >=  (const TSFixedString<OTHER_MAX_STR_LEN> & str) const 
	{ 
		return (Compare(str) >=  0); 
	}

	bool operator >  (const TSChar * str) const 
	{ 
		return (Compare(str) >  0); 
	}

	bool operator >  (const TSString & str) const 
	{ 
		return (Compare(str) >  0); 
	}

	template<int OTHER_MAX_STR_LEN>
	bool operator >  (const TSFixedString<OTHER_MAX_STR_LEN> & str) const 
	{ 
		return (Compare(str) >  0); 
	}

	int Compare(const TSChar * lpsz) const 
	{ 
		return _tcscmp(_Buffer,lpsz); 
	}

	int Compare(const TSString & str) const 
	{ 
		return _tcscmp(_Buffer,str.c_str()); 
	}

	template<int OTHER_MAX_STR_LEN>
	int Compare(const TSFixedString<OTHER_MAX_STR_LEN> & str) const 
	{ 
		return _tcscmp(_Buffer,(const TSChar *)str); 
	}

	int CompareNoCase(const TSChar * lpsz) const 
	{ 
		return _tcsicmp(_Buffer,lpsz); 
	}

	int CompareNoCase(const TSString & str) const 
	{ 
		return _tcsicmp(_Buffer,str.c_str()); 
	}

	template<int OTHER_MAX_STR_LEN>
	int CompareNoCase(const TSFixedString<OTHER_MAX_STR_LEN> & str) const 
	{ 
		return _tcsicmp(_Buffer,(const TSChar *)str); 
	}

	void MakeUpper() 
	{
		_tcsupr(_Buffer); 
	};

	void MakeLower() 
	{ 
		_tcslwr(_Buffer); 
	};

	TSFixedString Left(int len) 
	{
		TSFixedString<MAX_STR_LEN> dest;
		if( len>GetLength() )
		{
			len = GetLength();
		}
		_tcsncpy( dest._Buffer, _Buffer, len );
        dest._Buffer[len] = TS_TEXT('\0');

		return dest;
	}
	TSFixedString Mid(int pos,int len=-1) 
	{
		TSFixedString<MAX_STR_LEN> dest;

		if( len<0 ) 
		{
			len = GetLength()-pos;
		}
		if( pos+len>GetLength() ) 
		{
			len = GetLength() - pos;
		}
		if( len<=0 )
		{
			return dest;
		}
		_tcsncpy( dest._Buffer, &_Buffer[pos], len );

        dest._Buffer[len] = TS_TEXT('\0');

		return dest;
	}
	TSFixedString Right(int len) 
	{
		TSFixedString<MAX_STR_LEN> dest;
		int pos = GetLength() - len;
		if( pos<0 ) 
		{
			pos = 0;
			len = GetLength();
		}
		_tcsncpy( dest._Buffer, &_Buffer[pos], len );

        dest._Buffer[len] = TS_TEXT('\0');

		return dest;
	}

	int Find(TSChar ch) const
	{
		const TSChar * p = _tcschr(_Buffer, ch);
		if(p==NULL) 
		{
			return -1;
		}
		return p - _Buffer;
	}

	int ReverseFind(TSChar ch) const
	{
		const TSChar * p = _tcsrchr(_Buffer, ch);

		if(p==NULL) 
		{
			return -1;
		}
		return p - _Buffer;
	}

	int Find(const TSChar * lpszSub) const
	{
		const TSChar * p = _tcsstr(_Buffer, lpszSub);
		if(p==NULL) 
		{
			return -1;
		}
		return p - _Buffer;
	}
private:
	TSChar _Buffer[MAX_STR_LEN+1];
};

template<int MAX_STR_LEN>
bool operator==(const TSString & l,const TSFixedString<MAX_STR_LEN> & r)
{
	return r==l;
}

template<int MAX_STR_LEN>
bool operator<(const TSString & l,const TSFixedString<MAX_STR_LEN> & r)
{
	return r>l;
}

template<int MAX_STR_LEN>
bool operator<=(const TSString & l,const TSFixedString<MAX_STR_LEN> & r)
{
	return r>=l;
}

template<int MAX_STR_LEN>
bool operator!=(const TSString & l,const TSFixedString<MAX_STR_LEN> & r)
{
	return r!=l;
}

TS_END_RT_NAMESPACE

#endif // __TSFIXEDSTRING_H__

