#if defined(_MSC_VER)&&(_MSC_VER>=1200)
#pragma once
#endif

#ifndef TSRDGROBUSTMUTEX_HPP
#define TSRDGROBUSTMUTEX_HPP

#ifndef  _WRS_KERNEL

#include <boost/interprocess/detail/config_begin.hpp>
#include <boost/interprocess/detail/workaround.hpp>
#include <boost/interprocess/detail/os_thread_functions.hpp>
#include <boost/interprocess/detail/posix_time_types_wrk.hpp>
#include <boost/interprocess/detail/atomic.hpp>
#include <boost/interprocess/sync/spin/wait.hpp>
#include <boost/cstdint.hpp>

#ifdef _WIN32
#include "TSRDGOSFileFunctions.hpp"

//2^22 of possible processes
#define MAX_PIDS_NUMBER 4194304 

#define MAX_PATH 260

#if defined(_DEBUG) && defined(_ROBUST_MUTEX_TRACE) 
#define IPC_DBG_TRACE(msg) { std::stringstream __str; __str << bipc::ipcdetail::get_current_system_highres_count() << TS_TEXT(" " << msg; TSRDGDBGTrace::trace(__str); }
#else
#define IPC_DBG_TRACE(msg)
#endif

// there are 3 mutex states, correct, fixing and broken, if mutex is broken it can be
// automatically put into fixing state if TSRDG_IPC_STRICT_ROBUST_MUTEX_BROKEN_POLICY is not defined
#if !defined(TSRDG_IPC_STRICT_ROBUST_MUTEX_BROKEN_POLICY)
#define TSRDG_IPC_HANDLE_BROKEN_STATE() \
	boost::uint32_t oldState = bipc::ipcdetail::atomic_cas32(&this->_state, robust_mutex_state::fixing_state, robust_mutex_state::broken_state);\
	if(oldState == robust_mutex_state::broken_state){\
	IPC_DBG_TRACE("Recovered from broken ... (" << _name << ")");\
	}
#else
#define TSRDG_IPC_HANDLE_BROKEN_STATE() \
	if(bipc::ipcdetail::atomic_read32(&this->_state) == robust_mutex_state::broken_state){\
	throw TSException("Robust mutex is broken!");\
	}
#endif


namespace bipc = boost::interprocess;

#ifdef _DEBUG 
struct TSRDGDBGTrace{
	static FILE* f ;
	static void trace(const std::stringstream& str){
		char tmp[1024];
		sprintf(tmp, "%s/ipc_robust_mutex_log_trace.log", bipc::ipcdetail::get_temporary_path().c_str());
		if ( NULL == f ) {
			f = fopen(tmp, "a+b");
		}
		if(f){
			fprintf(f, "%6d %s\r\n", _getpid(), str.str().c_str());
			fflush(f) ;
			// fclose(f);
		}
	}
};
#ifdef _ROBUST_MUTEX_TRACE_FILE
FILE* dbg_tracer::f = (FILE *)NULL ;
#endif
#endif

namespace helper {

	inline void get_pids_locking_file_base_name(std::string &s)
	{
		char tmp[1024];
		std::string path = bipc::ipcdetail::get_temporary_path();
		sprintf(tmp, "%s/rdg_ipc_robust_mutex_pids", path.c_str());
		s.assign(tmp);
	}

	// This class will be a intermodule_singleton. 
	// The constructor will check existence of robust_mutexes central pids
	// lock file and it will lock current pid byte in.
	// destructor will release that locked byte
	// if we have some pid byte locked we know that process is alive

	class process_life_guard
	{
	public:
		static process_life_guard* instance(){
			static process_life_guard _instance;
			return &_instance;
		}
	private:
		struct scoped_file_lock{
			bipc::file_handle_t fd;
			bool _locked;
			scoped_file_lock(const bipc::file_handle_t& fd) : fd(fd), _locked(false){
				bipc::ipcdetail::try_acquire_file_lock(fd, _locked);
				if(!_locked){
					throw TSException("Robust mutex can't lock file!");
				}
			}
			~scoped_file_lock(){ 
				if(_locked){ 
					bipc::ipcdetail::release_file_lock(fd); 
				}
			}
		};

		struct _deleter{
			char* _ptr;

			_deleter(char* p) : _ptr(p){}
			~_deleter(){
				if(_ptr) delete[] _ptr;
			}
		};

		bipc::file_handle_t pids_file_lock_fd; // while we are creating "main pids file" we have to sync!!
		bipc::file_handle_t pids_lock_fd;		 // 2^22 big file with reserved 1byte for each eventually alive process 
		int	self_pid;
		bool im_a_locker;

	public:

		bool is_procces_dead(bipc::ipcdetail::OS_process_id_t pid){
			bool _locked = false;

			if ( pid == bipc::ipcdetail::OS_process_id_t(0) ) {
				return false ; // If process is unknown, than the process is not dead.
			}

#ifdef _WIN32
			bipc::ipcdetail::try_acquire_file_lock_ex(pids_lock_fd, _locked, pid, 1);
#else
			bipc::ipcdetail::try_acquire_file_lock(pids_lock_fd, _locked, pid, 1);
#endif

			if(_locked){
				IPC_DBG_TRACE("check process " << pid << " was dead");
#ifdef _WIN32
				bipc::ipcdetail::release_file_lock_ex(pids_lock_fd, pid, 1);
#else
				bipc::ipcdetail::release_file_lock(pids_lock_fd, pid, 1);
#endif

				return true;
			}
			return false;
		}	
	private:
		process_life_guard(const process_life_guard &);
		process_life_guard &operator=(const process_life_guard &);

		process_life_guard()
			: pids_file_lock_fd(bipc::ipcdetail::invalid_file()), pids_lock_fd(bipc::ipcdetail::invalid_file()), self_pid(bipc::ipcdetail::get_current_process_id()), im_a_locker(false)
		{
			bipc::permissions p;
			p.set_unrestricted();


			std::string lockNameBase;
			get_pids_locking_file_base_name(lockNameBase);
			std::string mainFileName = (lockNameBase + "/main.lck").c_str(); 
			std::string pidsFileName = (lockNameBase + "/pids.lck").c_str();
			//ensure dir exists
			bipc::ipcdetail::create_directory(lockNameBase.c_str());
			//main global lock fd
			pids_file_lock_fd = bipc::ipcdetail::create_or_open_file(mainFileName.c_str(), bipc::read_write, p);
			if(pids_file_lock_fd == bipc::ipcdetail::invalid_file()){
				throw TSException("Robust mutex can't open main lock file!");
			}

			//lock it in scope
			{ 
				//IPC_DBG_TRACE("start locking " << mainFileName);
				scoped_file_lock scoped_lock(pids_file_lock_fd); 

				pids_lock_fd = bipc::ipcdetail::create_new_file(pidsFileName.c_str(), bipc::read_write, p);
				if(pids_lock_fd != bipc::ipcdetail::invalid_file()){
					//IPC_DBG_TRACE("create new " << pidsFileName);
					_deleter d(new char[MAX_PIDS_NUMBER]);
					memset(d._ptr, 0, MAX_PIDS_NUMBER);
					bipc::ipcdetail::write_file(pids_lock_fd, d._ptr, MAX_PIDS_NUMBER);
				}else{
					//IPC_DBG_TRACE("use existing " << pidsFileName);
					pids_lock_fd = bipc::ipcdetail::open_existing_file(pidsFileName.c_str(), bipc::read_write);
				}
			}
			if(pids_lock_fd == bipc::ipcdetail::invalid_file()){
				throw TSException("Robust mutex can't open pids lock file!");
			}

			//lock my slot, if we did lock im_a_locker will become true, if not, there is other instance of singleton in the same process
#ifdef _WIN32
			bipc::ipcdetail::try_acquire_file_lock_ex(pids_lock_fd, im_a_locker, self_pid, 1);
#else
			bipc::ipcdetail::try_acquire_file_lock(pids_lock_fd, im_a_locker, self_pid, 1);
#endif

			IPC_DBG_TRACE("locking " << self_pid << " in " << pidsFileName << " -> " << (im_a_locker ? "locker" : "non locker"));
		}

		~process_life_guard()
		{
			if(im_a_locker){
#ifdef _WIN32
				bipc::ipcdetail::release_file_lock_ex(pids_lock_fd, self_pid, 1);	
#else
				bipc::ipcdetail::release_file_lock(pids_lock_fd, self_pid, 1);	
#endif

				IPC_DBG_TRACE("released proccess lock " << self_pid);
			}
			bipc::ipcdetail::close_file(pids_lock_fd);
			bipc::ipcdetail::close_file(pids_file_lock_fd);
		}   
	};
}//helper

struct robust_mutex_state{
	static const boost::uint32_t correct_state = 0;
	static const boost::uint32_t fixing_state  = 1;
	static const boost::uint32_t broken_state  = 2;
};

template<typename mutexT>
class set_name_trait{
public:

	void name(mutexT & m, const char* n){
		m.name(n);
	}
};

template<typename mutexT>
class fix_it_trait{
public:
	void fix_it(mutexT & m){
		m.fix_it();
	}
};

class robust_recursive_mutex;

class TSRDGRobustMutex
{
	typedef TSRDGRobustMutex self_type;

	friend class fix_it_trait<self_type>;
	friend class set_name_trait<self_type>;
	//the TSRDGSpinCondition call force_unlock
	friend class TSRDGSpinCondition;

	TSRDGRobustMutex(const TSRDGRobustMutex &);
	TSRDGRobustMutex &operator=(const TSRDGRobustMutex &);
public:
	typedef TSRDGRobustMutex internal_mutex_type;

	TSRDGRobustMutex();
	~TSRDGRobustMutex();

	void lock();
	bool try_lock();
	bool timed_lock(const boost::posix_time::ptime &abs_time);
	void unlock();
	void take_ownership(){};

	boost::uint32_t get_state();		
	const char* name() const;

	internal_mutex_type &internal_mutex()
	{  return *this;   }

	const internal_mutex_type &internal_mutex() const
	{  return *this;   }

private:
	//eventual name for mutex
	char _name[MAX_PATH + 1];

	//The pid of the owner, this field will be also lock gate
	volatile boost::uint32_t _owner;
	//The state of the mutex (correct, fixing, broken)
	volatile boost::uint32_t _state;

	//just some time check process alive
	static const unsigned int _spin_threshold = 255u;

	//invalid handle
	static const boost::uint32_t _invalidProcesshandle = (boost::uint32_t)bipc::ipcdetail::OS_process_id_t(0);

	bool robust_check();
	void force_unlock();
	bool check_if_owner_dead_and_take_ownership_atomically();
	bool init_life_guard();
	void fix_it(); //who need call must have trait for it!!!
	void name(const char*);
};

/************************************************************************/
/*  IPC types                                                           */
/************************************************************************/

typedef TSRDGRobustMutex robust_ipc_mutex;

/************************************************************************/
/*  IMPL                                                                */
/************************************************************************/

inline const char* TSRDGRobustMutex::name() const { return _name; }
inline void TSRDGRobustMutex::name(const char* n) { 
	strncpy(_name, n, MAX_PATH);
	size_t len = strlen(n);
	_name[len < MAX_PATH ? len : MAX_PATH] = 0;
}

inline TSRDGRobustMutex::TSRDGRobustMutex()
	: _owner(TSRDGRobustMutex::_invalidProcesshandle), _state(robust_mutex_state::correct_state)
{
	_name[0] = 0;
}

inline TSRDGRobustMutex::~TSRDGRobustMutex(){}

inline void TSRDGRobustMutex::lock(void)
{
	TSRDG_IPC_HANDLE_BROKEN_STATE();

	//This function provokes intermodule_singleton instantiation
	if(!this->init_life_guard()){
		throw TSException("Can't make life guard");
	}

	bipc::spin_wait swait;
	boost::uint32_t currentPid = bipc::ipcdetail::get_current_process_id();

	do{
		// 0 -> current
		boost::uint32_t prev_s = bipc::ipcdetail::atomic_cas32(const_cast<boost::uint32_t*>(&_owner), currentPid, 0);

		if (_owner == currentPid && prev_s == 0){
			//locked
			break;
		}else{
			// relinquish current time slice
			swait.yield();
			//Do the dead owner checking each spin_threshold lock tries
			if(0 == (swait.count() & _spin_threshold)){
				// Check if owner dead and take ownership if possible
				// it return true if ownership was done internal mutex is still locked!!!!
				// if ownership is taken then locks count will be set to 1
				if(this->robust_check()){
					break;
				}
			}
		}
	}while (true);		
}

inline bool TSRDGRobustMutex::try_lock(void)
{
	TSRDG_IPC_HANDLE_BROKEN_STATE();

	//This function provokes intermodule_singleton instantiation
	if(!this->init_life_guard()){
		throw TSException("Can't make life guard");
	}

	boost::uint32_t currentPid = bipc::ipcdetail::get_current_process_id();
	// 0 -> current
	boost::uint32_t prev_s = bipc::ipcdetail::atomic_cas32(const_cast<boost::uint32_t*>(&_owner), currentPid, 0);

	//if not locked do one time also robust check
	return _owner == currentPid && prev_s == 0 ? true :  this->robust_check();
}

inline bool TSRDGRobustMutex::timed_lock(const boost::posix_time::ptime &abs_time)
{
	if(abs_time == boost::posix_time::pos_infin){
		this->lock();
		return true;
	}
	//Obtain current count and target time
	boost::posix_time::ptime now = bipc::microsec_clock::universal_time();

	bipc::spin_wait swait;
	do{
		if(this->try_lock()){
			break;
		}
		now = bipc::microsec_clock::universal_time();

		if(now >= abs_time){
			return false;
		}
		// relinquish current time slice
		swait.yield();
	}while (true);

	return true;
}

inline void TSRDGRobustMutex::unlock(void)
{  
	//If in "fixing" state, unlock and mark the mutex as unrecoverable
	//so next locks will fail and all threads will be notified that the
	//data protected by the mutex was not recoverable.
	boost::uint32_t oldState = bipc::ipcdetail::atomic_cas32(&this->_state, robust_mutex_state::broken_state, robust_mutex_state::fixing_state);

#if defined(TSRDG_IPC_STRICT_ROBUST_MUTEX_BROKEN_POLICY)
	if(oldState == robust_mutex_state::fixing_state){
		IPC_DBG_TRACE("Marked BROKEN ! (" << _name << TS_TEXT(")");
		throw TSException("Robust mutex is broken !");
	}
#endif

	boost::uint32_t currentPid = bipc::ipcdetail::get_current_process_id();
	if(	bipc::ipcdetail::atomic_read32(&this->_owner) != currentPid){
		STD_COUT << TS_TEXT("Error unlock");
	}
#if defined(TSRDG_IPC_STRICT_ROBUST_MUTEX_BROKEN_POLICY)
	if(	bipc::ipcdetail::atomic_read32(&this->_owner) != currentPid){
		throw TSException("Not owner atemp to unlock !" );
	}
#endif

	// current -> 0
	bipc::ipcdetail::atomic_cas32(const_cast<boost::uint32_t*>(&_owner), 0, currentPid);
}	

inline void TSRDGRobustMutex::force_unlock()
{
	bipc::ipcdetail::atomic_write32(const_cast<boost::uint32_t*>(&_owner), 0);
}

inline void TSRDGRobustMutex::fix_it()
{
	// This function supposes the previous state was "fixing"
	// and the current process holds the mutex
	// so this should be called on locked mutex in fixing state!!!!
	if(bipc::ipcdetail::atomic_read32(&this->_state) != robust_mutex_state::fixing_state &&
		bipc::ipcdetail::atomic_read32(&this->_owner) != (boost::uint32_t)bipc::ipcdetail::get_current_process_id()){
			throw TSException("Robust mutex is broken !");
	}
	//If that's the case, just update mutex state
	bipc::ipcdetail::atomic_write32(&this->_state, robust_mutex_state::correct_state);
	IPC_DBG_TRACE("putting mutex in correct state ... (" << _name << ")");
}

inline boost::uint32_t TSRDGRobustMutex::get_state()
{
	//Notifies if a owner recovery has been performed in the last lock()
	return bipc::ipcdetail::atomic_read32(&this->_state);
};


inline bool TSRDGRobustMutex::robust_check()
{
	//If the old owner was dead, and we've acquired ownership, mark
	//the mutex as 'fixing'. This means that a "consistent()" is needed
	//to avoid marking the mutex as "broken" when the mutex is unlocked.
	if(!this->check_if_owner_dead_and_take_ownership_atomically()){
		return false;
	}
	IPC_DBG_TRACE("putting mutex in fixing state ... (" << _name << ")");
	bipc::ipcdetail::atomic_write32(&this->_state, robust_mutex_state::fixing_state);
	return true;
}


inline bool TSRDGRobustMutex::check_if_owner_dead_and_take_ownership_atomically()
{
	boost::uint32_t cur_owner = bipc::ipcdetail::get_current_process_id();
	boost::uint32_t old_owner = bipc::ipcdetail::atomic_read32(&this->_owner), old_owner2;


	// The cas loop guarantees that only one thread from this or another process
	// will succeed taking ownership
	// we will accept only mutex with non invalid owner handle
	do{
		//Check if owner process is dead 
		if(! helper::process_life_guard::instance()->is_procces_dead(old_owner)){
			// current owner is running, and its not and invalid handle so
			// we let it be and go away
			return false;
		}
		//If it's dead, try to mark this process as the owner in the owner field
		old_owner2 = old_owner;
		old_owner = bipc::ipcdetail::atomic_cas32(&this->_owner, cur_owner, old_owner);
	}while(old_owner2 != old_owner);

	IPC_DBG_TRACE("Mutex (" << _name << TS_TEXT(")") << TS_TEXT(" taken from : ") << old_owner << TS_TEXT(" by: ") << cur_owner);
	return true;

}


inline bool TSRDGRobustMutex::init_life_guard()
{
	helper::process_life_guard* guard = helper::process_life_guard::instance();
	return guard != 0;
}

#include <boost/interprocess/detail/config_end.hpp>
#else
typedef boost::interprocess::interprocess_mutex TSRDGRobustMutex;
#endif



#endif

#endif  //_WRS_KERNEL
