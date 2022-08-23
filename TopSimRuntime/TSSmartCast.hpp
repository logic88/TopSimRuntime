#ifndef __TSSMARTCAST_H__
#define __TSSMARTCAST_H__

#include <exception>
#include <typeinfo>
#include <cstddef> // NULL
#ifndef Q_MOC_RUN
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_base_and_derived.hpp>
#include <boost/type_traits/is_polymorphic.hpp>
#include <boost/type_traits/is_pointer.hpp>
#include <boost/type_traits/is_reference.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/identity.hpp>
#endif

TS_BEGIN_RT_NAMESPACE


template<class T>
struct TSCastReference 
{
	struct polymorphic 
	{

		struct linear 
		{
			template<class U>
			static T Cast(U & u)
			{
				return static_cast<T>(u);
			}
		};

		struct cross 
		{
			template<class U>
			static T Cast(U & u)
			{
				return dynamic_cast<T>(u);
			}
		};

		template<class U>
		static T Cast(U & u){
			// if we're in debug mode
#if ! defined(NDEBUG)                               \
	|| defined(__BORLANDC__) && (__BORLANDC__ <= 0x560) \
	|| defined(__MWERKS__)
			return cross::Cast(u);
#else
			typedef BOOST_DEDUCED_TYPENAME boost::mpl::eval_if<
				BOOST_DEDUCED_TYPENAME boost::mpl::and_<
				boost::mpl::not_<boost::is_base_and_derived<
				BOOST_DEDUCED_TYPENAME boost::remove_reference<T>::type,
				U
				> >,
				boost::mpl::not_<boost::is_base_and_derived<
				U,
				BOOST_DEDUCED_TYPENAME boost::remove_reference<T>::type
				> >
				>,
				boost::mpl::identity<cross>,
				boost::mpl::identity<linear>
			>::type typex;
			return typex::Cast(u);
#endif
		}
	};

	struct non_polymorphic 
	{
		template<class U>
		static T Cast(U & u)
		{
			return static_cast<T>(u);
		}
	};
	template<class U>
	static T Cast(U & u)
	{
		typedef BOOST_DEDUCED_TYPENAME boost::mpl::eval_if<
			boost::is_polymorphic<U>,
			boost::mpl::identity<polymorphic>,
			boost::mpl::identity<non_polymorphic>
		>::type typex;
		return typex::Cast(u);
	}
};

template<class T>
struct TSCastPointer 
{
	struct polymorphic 
	{
		template<class U>
		static T Cast(U * u)
		{
			T tmp = dynamic_cast<T>(u);
#ifndef NDEBUG
			if ( tmp == 0 ) throw std::bad_cast();
#endif
			return tmp;
		}
	};

	struct non_polymorphic 
	{
		template<class U>
		static T Cast(U * u){
			return static_cast<T>(u);
		}
	};

	template<class U>
	static T Cast(U * u)
	{
		typedef BOOST_DEDUCED_TYPENAME boost::mpl::eval_if<
			boost::is_polymorphic<U>,
			boost::mpl::identity<polymorphic>,
			boost::mpl::identity<non_polymorphic>
		>::type typex;
		return typex::Cast(u);
	}

};

template<class TPtr>
struct TSCastVoidPointer 
{
	template<class UPtr>
	static TPtr Cast(UPtr uptr)
	{
		return static_cast<TPtr>(uptr);
	}
};

template<class T>
struct TSCastError 
{
	template<class U>
	static T Cast(U u){
		BOOST_STATIC_ASSERT(sizeof(T)==0);
		return * static_cast<T *>(NULL);
	}
};

template<class T, class U>
T TSSmartCast(U u) 
{
	typedef
		BOOST_DEDUCED_TYPENAME boost::mpl::eval_if<
		BOOST_DEDUCED_TYPENAME boost::mpl::or_<
		boost::is_same<void *, U>,
		boost::is_same<void *, T>,
		boost::is_same<const void *, U>,
		boost::is_same<const void *, T>
		>,
		boost::mpl::identity<TSCastVoidPointer<T> >,
		// else
		BOOST_DEDUCED_TYPENAME boost::mpl::eval_if<boost::is_pointer<U>,
		boost::mpl::identity<TSCastPointer<T> >,
		// else
		BOOST_DEDUCED_TYPENAME boost::mpl::eval_if<boost::is_reference<U>,
		boost::mpl::identity<TSCastReference<T> >,
		// else
		boost::mpl::identity<TSCastError<T>
		>
		>
		>
		>::type typex;
	return typex::Cast(u);
}

TS_END_RT_NAMESPACE

#endif // __TSSMARTCAST_H__

