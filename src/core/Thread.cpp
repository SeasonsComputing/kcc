/*
 * Kuumba C++ Core
 *
 * $Id: Thread.cpp 17346 2007-05-09 21:07:42Z tvk $
 */
#include <inc/core/Core.h>

#if defined(KCC_WINDOWS)
#   include "windows.h"
#   include "winpthr/pthread.h"
#   include "winpthr/semaphore.h"
#elif defined (KCC_LINUX)
#   include "pthread.h"
#   include "semaphore.h"
#   include "errno.h"
#endif

#define KCC_FILE "Thread"

namespace kcc
{
    // Helper class to manage thread key
    struct ThreadModuleState : Core::ModuleState
    {
        // Attributes
        ::pthread_key_t m_key;
        ThreadModuleState()  { ::pthread_key_create(&m_key, ThreadModuleState::clean); }
        ~ThreadModuleState() { ::pthread_key_delete(m_key); }

        // add: add thread to be cleaned
        void add(Thread* t) { ::pthread_setspecific(m_key, t); }

        // clean: delete thread object
        static void clean(void* arg)
        {
            Thread* t = static_cast<Thread*>(arg);
            delete t;
        }
    };

    //
    // Thread implementation
    //

    // Thread: create as delegate or derivative
    Thread::Thread(const Char* name, IThreadManager* manager)
        : m_name(name), m_thread(new pthread_t), m_id(0L), m_invoker(NULL), m_manager(manager), m_completed(true)
    {}
    Thread::Thread(IThread* i, const Char* name, IThreadManager* manager)
        : m_name(name), m_thread(new pthread_t), m_id(0L), m_invoker(i), m_manager(manager), m_completed(true)
    {
        KCC_ASSERT(NULL != i, KCC_FILE, "Thread", "null invoker");
    }

    // ~Thread: delete thread, checking for proper clean up
    Thread::~Thread()
    {
        Log::Scope scope(KCC_FILE, "~Thread");
        if (!m_completed) Log::warning("thread must only be deleted on return from invoke() or kill()");
        delete (pthread_t*)m_thread;
    }

    // go: run thread
    void Thread::go(bool joinable)
    {
        ::pthread_attr_t attr;
        ::pthread_attr_init(&attr);
        if (joinable)
            ::pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        else
            ::pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        ::pthread_create((pthread_t*)m_thread, &attr, Thread::threadFunc, this);
        ::pthread_attr_destroy(&attr);
    }

    // id: thread id
    unsigned long Thread::id() { return m_id; }

    // current: current thread id
    unsigned long Thread::current() { return (unsigned long)::pthread_self(); }

    // kill: kills thread
    void Thread::kill()
    {
        KCC_ASSERT(m_thread != NULL, KCC_FILE, "kill", "attempt to kill thread that isn't running");
        complete();
        // Linux will call thread attr delete on thread cancel.
        // Windows will NOT, so ensure thread object is cleaned.
        ::pthread_cancel(*((pthread_t*)m_thread));
        #if defined(KCC_WINDOWS)
            ThreadModuleState::clean(this);
        #endif
    }

    // invoke: delegate thread behavior
    void Thread::invoke()
    {
        KCC_ASSERT(NULL != m_invoker, KCC_FILE, "invoke", "null invoker");
        m_invoker->invoke();
    }

    // sleep: sleep for a number of milliseconds
    void Thread::sleep(long ms)
    {
        #if defined(KCC_WINDOWS)
            ::Sleep(ms);
        #elif defined(KCC_LINUX)
            long s  = ms / 1000;
            long ns = (ms-(s*1000)) * 1000000;
            struct ::timespec t = {0};
            t.tv_sec  = s;
            t.tv_nsec = ns;
            ::nanosleep(&t, NULL);
        #endif
    }

    // join: join thread
    bool Thread::join(Thread* t)
    {
        return ::pthread_join(*((pthread_t*)t->m_thread), NULL) == 0;
    }

    // threadFunc: thread function callback
    void* Thread::threadFunc(void* arg)
    {
        Log::Scope scope(KCC_FILE, "invoke");
        Thread* t = static_cast<Thread*>(arg);
        t->m_id = Thread::current();
        KCC_STATE(ThreadModuleState).add(t);
        try
        {
            Log::info4("thread started: id=[%ld] name=[%s]", t->id(), (t->m_name == NULL ? "{empty}" : t->m_name));
            t->invoke();
        }
        catch (Exception& e)
        {
            Log::error("uncaught kcc::Exception in thread: [%s]", e.what());
            Log::exception(e);
        }
        catch (std::exception& e)
        {
            Log::error("uncaught std::exception in thread: [%s]", e.what());
            Log::exception(e);
        }
        catch (...)
        {
            Log::error("uncaught exception [unknown exception]");
        }
        t->complete();
        return NULL;
    }

    // complete: complete thread
    void Thread::complete()
    {
        m_invoker   = NULL;
        m_completed = true;
        if (m_manager != NULL) m_manager->onThreadDeleted(m_name);
    }

    //
    // Mutex implementation
    //

    // ctor/dtor
    Mutex::Mutex() : m_mutex(new ::pthread_mutex_t)
    {
        ::pthread_mutexattr_t attr;
        ::pthread_mutexattr_init(&attr);
        ::pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        ::pthread_mutex_init((::pthread_mutex_t*)m_mutex, &attr);
        ::pthread_mutexattr_destroy(&attr);
    }
    Mutex::~Mutex()
    {
        ::pthread_mutex_destroy((pthread_mutex_t*)m_mutex);
        delete (pthread_mutex_t*)m_mutex;
    }

    // lock management
    bool Mutex::tryLock() { return ::pthread_mutex_trylock((pthread_mutex_t*)m_mutex) == 0; }
    void Mutex::lock()    { ::pthread_mutex_lock((pthread_mutex_t*)m_mutex); }
    void Mutex::unlock()  { ::pthread_mutex_unlock((pthread_mutex_t*)m_mutex); }

    //
    // SynchCondition implementation
    //

    // ctor/dtor
    SynchCondition::SynchCondition() : m_cond(new ::pthread_cond_t)
    {
        ::pthread_cond_init((pthread_cond_t*)m_cond, NULL);
    }
    SynchCondition::~SynchCondition()
    {
        ::pthread_cond_destroy((pthread_cond_t*)m_cond);
        delete (pthread_cond_t*)m_cond;
    }

    // begin: begin event
    void SynchCondition::init()
    {
        Mutex::Lock lock(m_busy);
        onBegin();
    }

    // notify: end event, notify one
    void SynchCondition::notify()
    {
        Mutex::Lock lock(m_busy);
        onEnd();
        ::pthread_cond_signal((pthread_cond_t*)m_cond);
    }

    // notifyAll: end event, notify all
    void SynchCondition::notifyAll()
    {
        Mutex::Lock lock(m_busy);
        onEnd();
        ::pthread_cond_broadcast((pthread_cond_t*)m_cond);
    }

    // wait: wait for all events to complete
    void SynchCondition::wait()
    {
        Mutex::Lock lock(m_busy);
        while (onTest()) ::pthread_cond_wait((pthread_cond_t*)m_cond, (pthread_mutex_t*)m_busy.m_mutex);
    }

    //
    // Monitor implementation (Template methods (GOF) synch'd by caller)
    //

    Monitor::Monitor() : m_count(0) {}
    void Monitor::onBegin() { m_count++; }
    bool Monitor::onTest()  { return m_count > 0; }

    // onEnd: check for balanced begin/end when ending
    void Monitor::onEnd()
    {
        m_count--;
        KCC_ASSERT(m_count >= 0, "Monitor", "onEnd", "unbalanced init/notify");
    }

    // count: fetch synch count
    int Monitor::count()
    {
        int c = 0;
        {
            Mutex::Lock lock(m_busy);
            c = m_count;
        }
        return c;
    }
}
