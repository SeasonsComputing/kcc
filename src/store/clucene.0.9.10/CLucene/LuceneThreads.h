#ifndef _LuceneThreads_h
#define  _LuceneThreads_h
#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

CL_NS_DEF(util)
#if defined(_CL_DISABLE_MULTITHREADING)
  #define SCOPED_LOCK_MUTEX(theMutex)
  #define LOCK_MUTEX(theMutex)
  #define UNLOCK_MUTEX(x)
  #define UNLOCK_MUTEX2(x) //this is used to close an index when scoping is not being used

  #define DEFINE_MUTEX(x)
  #define STATIC_DEFINE_MUTEX(x)
#else

#if defined(_LUCENE_THREADMUTEX)
   //do nothing

#elif defined(_CLCOMPILER_MSVC) && !defined(LUCENE_THREAD_HACK) //todo: does mingw32 support, etc?
   #if !defined(LUCENE_USE_WINDOWS_H) && !defined(_WINDOWS_)
      //we have not explicity included windows.h and windows.h has
      //not been included (check _WINDOWS_), then we must define
      //our own definitions to the thread locking functions:
      struct CRITICAL_SECTION
      {
         struct critical_section_debug * DebugInfo;
         long LockCount;
         long RecursionCount;
         void * OwningThread;
         void * LockSemaphore;
      #if defined(_WIN64)
         unsigned __int64 SpinCount;
      #else
         unsigned long SpinCount;
      #endif
      };

      extern "C" __declspec(dllimport) void __stdcall InitializeCriticalSection(CRITICAL_SECTION *);
      extern "C" __declspec(dllimport) void __stdcall EnterCriticalSection(CRITICAL_SECTION *);
      extern "C" __declspec(dllimport) void __stdcall LeaveCriticalSection(CRITICAL_SECTION *);
      extern "C" __declspec(dllimport) void __stdcall DeleteCriticalSection(CRITICAL_SECTION *);
   #endif

   ///a windows implementation of the lock mutex
   ///todo: boost has a InterlockedExchange way of locking too. More backwards compatible???
   class mutex_win32
   {
   private:
      CRITICAL_SECTION mtx;
   public:
      mutex_win32()
      { InitializeCriticalSection(&mtx); }

      ~mutex_win32()
      { DeleteCriticalSection(&mtx); }

      void lock()
      { EnterCriticalSection(&mtx); }

      void unlock()
      { LeaveCriticalSection(&mtx); }
   };
   #define _LUCENE_THREADMUTEX CL_NS(util)::mutex_win32

#elif defined(_CL_HAVE_PTHREAD_H) && defined(_POSIX_THREADS) && (_POSIX_THREADS+0 >= 0) && !defined(LUCENE_THREAD_HACK)
   #include <pthread.h>
   ///a posix implementation of the lock mutex
   ///todo: boost also has a 'spinlock' mutex which uses atomic_t
   ///might this be faster, or more x-platform?
   class mutex_pthread
   {
   private:
      pthread_mutex_t mtx;
   public:
      mutex_pthread()
      { 
#if defined(__hpux) && defined(_DECTHREADS_)
         pthread_mutex_init(&mtx, pthread_mutexattr_default);
#else
         pthread_mutex_init(&mtx, 0); 
#endif
      }

      ~mutex_pthread()
      { pthread_mutex_destroy(&mtx); }

      void lock()
      { pthread_mutex_lock(&mtx); }

      void unlock()
      { pthread_mutex_unlock(&mtx); }
   };
   #define _LUCENE_THREADMUTEX CL_NS(util)::mutex_pthread

#else //LUCENE_THREAD_HACK
   ///a lock mutex hack
   class mutex_default{
	private:
		bool locked;
	public:
      mutex_default(){	
         locked=false; 
      }
      ~mutex_default(){
      }
      void lock(){
         while ( locked )
            _sleep(1);

         locked=true; //wait till unlocked, then lock
      }
      void unlock(){
#if defined(_DEBUG)
         if ( locked ){
            locked = false;
         } else
            _CLTHROWA(CL_ERR_Runtime, "Mutex wasn't locked");
# else
            locked = false;
# endif // _DEBUG
      }
   };
   #define _LUCENE_THREADMUTEX CL_NS(util)::mutex_default
   #define _LUCENE_THREADMUTEX_USINGDEFAULT
#endif //mutex types

class mutexGuard
{
private:
    _LUCENE_THREADMUTEX& mrMutex;
public:
    mutexGuard( _LUCENE_THREADMUTEX& rMutex ) :
    mrMutex(rMutex)
  {
    mrMutex.lock();
  }
    ~mutexGuard()
  {
    mrMutex.unlock();
  }
};

//todo: This is urgent: we need to have an inter-process lock, not just in-process locking
# define SCOPED_LOCK_MUTEX(theMutex) CL_NS(util)::mutexGuard theMutexGuard(theMutex)
# define LOCK_MUTEX(theMutex) { CL_NS(util)::mutexGuard theMutexGuard(theMutex);
# define UNLOCK_MUTEX(x) }
# define UNLOCK_MUTEX2(x) //this is used to close an index when scoping is not being used

# define DEFINE_MUTEX(x) _LUCENE_THREADMUTEX x
# define STATIC_DEFINE_MUTEX(x) static _LUCENE_THREADMUTEX x

#endif //_CL_DISABLE_MULTITHREADING


CL_NS_END

#endif
