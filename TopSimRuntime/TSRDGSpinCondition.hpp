#ifndef TSRDGSPINCONDITION_HPP
#define TSRDGSPINCONDITION_HPP

#ifndef  _WRS_KERNEL

#include <boost/interprocess/detail/config_begin.hpp>
#include <boost/interprocess/detail/workaround.hpp>
#include <boost/interprocess/sync/spin/mutex.hpp>
#include <boost/interprocess/detail/posix_time_types_wrk.hpp>
#include <boost/interprocess/detail/atomic.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/exceptions.hpp>
#include <boost/interprocess/detail/os_thread_functions.hpp>
#include <boost/interprocess/sync/spin/wait.hpp>
#include <boost/move/move.hpp>
#include <boost/cstdint.hpp>

#include "TSRDGRobustMutex.hpp"

class TSRDGSpinCondition
{
   TSRDGSpinCondition(const TSRDGSpinCondition &);
   TSRDGSpinCondition &operator=(const TSRDGSpinCondition &);
   public:
   TSRDGSpinCondition();
   ~TSRDGSpinCondition();
 
   void notify_one();

   template <typename L>
   bool timed_wait(L& lock, const boost::posix_time::ptime &abs_time)
   {
      if (!lock)
         throw TSException("boost::interprocess::lock_exception");
      //Handle infinity absolute time here to avoid complications in do_timed_wait
      if(abs_time == boost::posix_time::pos_infin){
         this->wait(lock);
         return true;
      }
      return this->do_timed_wait(abs_time, *lock.mutex());
   }

   template <typename L, typename Pr>
   bool timed_wait(L& lock, const boost::posix_time::ptime &abs_time, Pr pred)
   {
      if (!lock)
         throw TSException("boost::interprocess::lock_exception");
      //Handle infinity absolute time here to avoid complications in do_timed_wait
      if(abs_time == boost::posix_time::pos_infin){
         this->wait(lock, pred);
         return true;
      }
      while (!pred()){
         if (!this->do_timed_wait(abs_time, *lock.mutex()))
            return pred();
      }
      return true;
   }

   template <typename L>
   void wait(L& lock)
   {
      if (!lock)
         throw TSException("boost::interprocess::lock_exception");
      do_wait(*lock.mutex());
   }

   template <typename L, typename Pr>
   void wait(L& lock, Pr pred)
   {
      if (!lock)
         throw TSException("boost::interprocess::lock_exception");

      while (!pred())
         do_wait(*lock.mutex());
   }

   template<class InterprocessMutex>
   void do_wait(InterprocessMutex &mut);

   template<class InterprocessMutex>
   bool do_timed_wait(const boost::posix_time::ptime &abs_time, InterprocessMutex &mut);

   private:
   template<class InterprocessMutex>
   bool do_timed_wait(bool tout_enabled, const boost::posix_time::ptime &abs_time, InterprocessMutex &mut);

   enum { SLEEP = 0, NOTIFY_ONE };
   //  m_enter_mut;
   volatile boost::uint32_t    m_command;
   volatile boost::uint32_t    m_num_waiters;
   void notify(boost::uint32_t command);
};

inline TSRDGSpinCondition::TSRDGSpinCondition()
{
   //Note that this class is initialized to zero.
   //So zeroed memory can be interpreted as an initialized
   //condition variable
   m_command      = SLEEP;
   m_num_waiters  = 0;
}

inline TSRDGSpinCondition::~TSRDGSpinCondition()
{
   
}

inline void TSRDGSpinCondition::notify_one()
{
   this->notify(NOTIFY_ONE);
}

inline void TSRDGSpinCondition::notify(boost::uint32_t command)
{
	boost::interprocess::ipcdetail::atomic_cas32(const_cast<boost::uint32_t*>(&m_command), command, SLEEP);
}

template<class InterprocessMutex>
inline void TSRDGSpinCondition::do_wait(InterprocessMutex &mut)
{
   this->do_timed_wait(false, boost::posix_time::ptime(), mut);
}

template<class InterprocessMutex>
inline bool TSRDGSpinCondition::do_timed_wait
   (const boost::posix_time::ptime &abs_time, InterprocessMutex &mut)
{
   return this->do_timed_wait(true, abs_time, mut);
}

template<class InterprocessMutex>
inline bool TSRDGSpinCondition::do_timed_wait(bool tout_enabled,
                                     const boost::posix_time::ptime &abs_time,
                                     InterprocessMutex &mut)
{
   boost::posix_time::ptime now = boost::interprocess::microsec_clock::universal_time();

   if(tout_enabled){
      if(now >= abs_time) return false;
   }

   //The enter mutex guarantees that while executing a notification,
   //no other thread can execute the do_timed_wait method.
   {
      //We increment the waiting thread count protected so that it will be
      //always constant when another thread enters the notification logic.
      //The increment marks this thread as "waiting on spin_condition_extend"
      boost::interprocess::ipcdetail::atomic_inc32(const_cast<boost::uint32_t*>(&m_num_waiters));

	  //We unlock the external mutex atomically with the increment
	  mut.unlock();
   }

   //By default, we suppose that no timeout has happened
   bool timed_out  = false;

   //Loop until a notification indicates that the thread should
   //exit or timeout occurs
   while(1){
      //The thread sleeps/spins until a spin_condition_extend commands a notification
      //Notification occurred, we will lock the checking mutex so that
      boost::interprocess::spin_wait swait;
      while(boost::interprocess::ipcdetail::atomic_read32(&m_command) == SLEEP){
         swait.yield();

         //Check for timeout
         if(tout_enabled){
            now = boost::interprocess::microsec_clock::universal_time();

            if(now >= abs_time){
               //If we can lock the mutex it means that no notification
               //is being executed in this spin_condition_extend variable
              // timed_out = m_enter_mut.try_lock();

               //If locking fails, indicates that another thread is executing
               //notification, so we play the notification game
               if(!timed_out){
                  //There is an ongoing notification, we will try again later
                  continue;
               }
               //No notification in execution, since enter mutex is locked.
               //We will execute time-out logic, so we will decrement count,
               //release the enter mutex and return false.
               break;
            }
         }
      }

      //If a timeout occurred, the mutex will not execute checking logic
      if(tout_enabled && timed_out){
         //Decrement wait count
         boost::interprocess::ipcdetail::atomic_dec32(const_cast<boost::uint32_t*>(&m_num_waiters));
         break;
      }
      else{
         boost::uint32_t result = boost::interprocess::ipcdetail::atomic_cas32
                        (const_cast<boost::uint32_t*>(&m_command), SLEEP, NOTIFY_ONE);
         if(result == SLEEP){
            //Other thread has been notified and since it was a NOTIFY one
            //command, this thread must sleep again
            continue;
         }
         else if(result == NOTIFY_ONE){
            //If it was a NOTIFY_ONE command, only this thread should
            //exit. This thread has atomically marked command as sleep before
            //so no other thread will exit.
            //Decrement wait count.
            boost::interprocess::ipcdetail::atomic_dec32(const_cast<boost::uint32_t*>(&m_num_waiters));
            break;
         }
      }
   }

   //Lock external again before returning from the method
   mut.lock();
   return !timed_out;
}

#include <boost/interprocess/detail/config_end.hpp>

#endif   //BOOST_INTERPROCESS_DETAIL_SPIN_CONDITION_EXTEND_HPP

#endif // _WRS_KERNEL
