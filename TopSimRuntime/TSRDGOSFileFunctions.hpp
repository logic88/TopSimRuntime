#if defined(_MSC_VER)&&(_MSC_VER>=1200)
#pragma once
#endif


#ifndef TSRDGOSFILEFUNCTIONS_HPP
#define TSRDGOSFILEFUNCTIONS_HPP

#ifndef  _WRS_KERNEL

#include <boost/interprocess/detail/os_file_functions.hpp>

namespace boost{
	namespace interprocess{
		namespace ipcdetail{


#if (defined BOOST_INTERPROCESS_WINDOWS)

			inline bool try_acquire_file_lock_ex(file_handle_t hnd, bool &acquired, size_t pos, size_t len)
			{
				
				unsigned long posL = 0, posH = 0, lenL = 0, lenH = 0;
				if(sizeof(size_t) == 8){

#pragma warning( push ) // 32 bit compilation trick
#pragma warning( disable: 4293 )
					posL = static_cast<unsigned long>(pos);
					posH = static_cast<unsigned long>(pos >> 32);
					lenL = static_cast<unsigned long>(len);
					lenH = static_cast<unsigned long>(len >> 32);
#pragma warning( pop )

				}else{
					posL = static_cast<unsigned long>(pos);
					lenL = static_cast<unsigned long>(len);
				}
				winapi::interprocess_overlapped overlapped;
				std::memset(&overlapped, 0, sizeof(overlapped));
				overlapped.dummy.offset = posL;
				overlapped.dummy.offset_high = posH;
				if(!winapi::lock_file_ex
					(hnd, winapi::lockfile_exclusive_lock | winapi::lockfile_fail_immediately,
					0, lenL, lenH, &overlapped)){
						return winapi::get_last_error() == winapi::error_lock_violation ?
							acquired = false, true : false;

				}
				return (acquired = true);
			}

			inline bool release_file_lock_ex(file_handle_t hnd, size_t pos, size_t len)
			{
				unsigned long posL = 0, posH = 0, lenL = 0, lenH = 0;
				if(sizeof(size_t) == 8){

#pragma warning( push ) // 32 bit compilation trick
#pragma warning( disable: 4293 )

					posL = static_cast<unsigned long>(pos);
					posH = static_cast<unsigned long>(pos >> 32);
					lenL = static_cast<unsigned long>(len);
					lenH = static_cast<unsigned long>(len >> 32);
#pragma warning( pop )

				}else{
					posL = static_cast<unsigned long>(pos);
					lenL = static_cast<unsigned long>(len);
				}
				winapi::interprocess_overlapped overlapped;
				std::memset(&overlapped, 0, sizeof(overlapped));
				overlapped.dummy.offset = posL;
				overlapped.dummy.offset_high = posH;
				return winapi::unlock_file_ex(hnd, 0, lenL, lenH, &overlapped);
			}

			inline bool read_file(file_handle_t hnd, void *data, std::size_t numdata)
			{
				unsigned long readen;
				return 0 != winapi::read_file(hnd, data, (unsigned long)numdata, &readen, 0);
			}

#else
			inline bool try_acquire_file_lock(file_handle_t hnd, bool &acquired, size_t pos, size_t len)
			{
				struct ::flock lock;
				lock.l_type    = F_WRLCK;
				lock.l_whence  = SEEK_SET;
				lock.l_start   = pos;
				lock.l_len     = len;
				int ret = ::fcntl(hnd, F_SETLK, &lock);
				if(ret == -1){
					return (errno == EAGAIN || errno == EACCES) ?
						acquired = false, true : false;
				}
				return (acquired = true);
			}

			inline bool release_file_lock(file_handle_t hnd, size_t pos, size_t len)
			{
				struct ::flock lock;
				lock.l_type    = F_UNLCK;
				lock.l_whence  = SEEK_SET;
				lock.l_start   = pos;
				lock.l_len     = len;
				return -1 != ::fcntl(hnd, F_SETLK, &lock);
			}

			inline bool read_file(file_handle_t hnd, void *data, std::size_t numdata)
			{  return (ssize_t(numdata)) == ::read(hnd, data, numdata);  }

#endif
}}} //boost::interproccess::ipcdetail
#endif

#endif //_WRS_KERNEL
