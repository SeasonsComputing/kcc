/*
 * Kuumba C++ Core
 *
 * $Id: Thread.h 15199 2007-03-09 17:57:17Z tvk $
 */
#ifndef Thread_h
#define Thread_h

namespace kcc
{
    /**
     * Interface for thread handler
     *
     * @author Ted V. Kremer
     */
    interface IThread : IComponent
    {
        /** 
         * Callback when thread is started
         */
        virtual void invoke() = 0;
    };

    /**
     * Interface for thread manager
     *
     * @author Ted V. Kremer
     */
    interface IThreadManager : IComponent
    {
        /**
         * Called when thread is deleted
         * @param name name of deleted thread
         */
        virtual void onThreadDeleted(const Char* name) = 0;
    };

    /**
     * Thread object invoker. Threads must be created on the
     * heap. Thread objects are automatically destructed when
     * the invoker is completed. Never delete a thread.
     *
     * @author Ted V. Kremer
     */
    class KCC_CORE_EXPORT Thread : public IThread
    {
    public:
        /**
         * Used for invoker delegation
         * @param invoker invoker to handle thread (ownership NOT consumed)
         * @param name thread name
         * @param manager thread manager (NOT CONSUMED)
         */
        Thread(IThread* invoker, const Char* name = NULL, IThreadManager* manager = NULL);

        /**
         * DO NOT CALL. Threads are cleaned when invoke()
         * returns or kill() is called
         */
        virtual ~Thread();

        /**
         * Query thread name
         * @return token name (or NULL if not set)
         */
        inline const Char* name() const { return m_name; }

        /**
         * Begin thread exectution
         * @param joinable is thread joinable (to allow Thread::join) or detached
         */
        void go(bool joinable = false);

        /**
         * Query unique thread id
         * @return thread id
         */
        unsigned long id();

        /**
         * Cancel thread execution. Will trigger a destructor call.
         */
        void kill();

        /** Delegate thread handler */
        virtual void invoke();

        /**
         * Return current thread id
         * @return thread id of calling thread
         */
        static unsigned long current();

        /**
         * Utility method to sleep calling thread
         * @param ms milliseconds to sleep for
         */
        static void sleep(long ms);

        /**
         * Joins thread with calling thread
         * @param thread thread to join to (must have been created joinable)
         * @return true if successful
         */
        static bool join(Thread* thread);

    protected:
        /**
         * Use for derivatives
         * @param name thread name
         * @param manager thread manager (NOT CONSUMED)
         */
        Thread(const Char* name = NULL, IThreadManager* manager = NULL);

    private:
        Thread(const Thread&);
        Thread& operator = (const Thread&);

        /** Thread callback function */
        static void* threadFunc(void* arg);

        /** Complete thread execution, notify manager, and delete thread object */
        void complete();

        // Attributes
        const Char*     m_name;
        void*           m_thread;
        unsigned long   m_id;
        IThread*        m_invoker;
        IThreadManager* m_manager;
        bool            m_completed;
    };

    /**
     * Syncronization mutex object for managing critical sections
     *
     * @author Ted V. Kremer
     */
    class KCC_CORE_EXPORT Mutex
    {
    public:
        /** Utility class to automatically lock/unlock on stack boundary */
        struct Lock
        {
            Mutex& m;
            Lock(Mutex& _m) : m(_m) { m.lock();   }
            ~Lock()                 { m.unlock(); }
        };

        /** Map init/destroy of mutex to ctor/dtor */
        Mutex();
        ~Mutex();

        /** Lock/unlock mutex */
        void lock();
        void unlock();

        /**
         * Attempt lock
         * @return true if locked
         */
        bool tryLock();

    private:
        Mutex(const Mutex&);
        Mutex& operator = (const Mutex&);

        // Attributes
        friend class SynchCondition;
        void* m_mutex;
    };

    /**
     * Thread synchronization condition abstract class. This class provides
     * signaled event completion (POSIX conditions).
     *
     * Derivatives override the template methods to manage what the
     * condition is to wait on. All locking and condition signaling
     * and waiting is managed by this class.
     *
     * @author Ted V. Kremer
     */
    class KCC_CORE_EXPORT SynchCondition
    {
    public:
        /** Synchronization condition ctor/dtor */
        SynchCondition();
        virtual ~SynchCondition();

        /** Initialize a condition beginning */
        void init();

        /** Notify of an condition ending */
        void notify();
        void notifyAll();

        /** Wait for all conditions to complete */
        void wait();

    protected:
        // Template methods (GOF) to manage condition
        virtual void onBegin() = 0;
        virtual void onEnd()   = 0;
        virtual bool onTest()  = 0;

        // Attributes
        Mutex m_busy;
        void* m_cond;

    private:
        SynchCondition(const SynchCondition&);
        SynchCondition& operator = (const SynchCondition&);
    };

    /**
     * Thread monitor
     *
     * @author Ted V. Kremer
     */
    class KCC_CORE_EXPORT Monitor : public SynchCondition
    {
    public:
        /** Initialize thread condition */
        Monitor();

        /** Accessor to count */
        int count();

    protected:
        // Implementation
        virtual void onBegin();
        virtual void onEnd();
        virtual bool onTest();

    private:
        Monitor(const Monitor&);
        Monitor& operator = (const Monitor&);

        // Attributes
        int m_count;
    };
}

#endif // Thread_h
