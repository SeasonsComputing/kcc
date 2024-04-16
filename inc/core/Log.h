/*
 * Kuumba C++ Core
 *
 * $Id: Log.h 22762 2008-03-24 16:50:17Z tvk $
 */
#ifndef Log_h
#define Log_h

namespace kcc
{
    interface ILogDecorator;

    /**
     * Logging - 5 levels of verbosity (higher levels include all lower levels)
     *    info4     - inner loop metrics
     *    info3     - component metrics
     *    info2     - component status message or application metrics
     *    info1     - application status message
     *    warning   - component or application unxepected state
     *    error     - component or application error condition (always logged)
     *    exception - log exception including scope trace (always logged)
     *
     * Defines:
     *    KCC_LOG_BRIEF - compiling away log verbosity > info3
     *
     * USAGE:
     *   ... Foo.cpp ...
     *   #define KCC_FILE "Foo"
     *   void Foo::bar()
     *   {
     *      kcc::Log::Scope scope(KCC_FILE, "bar");
     *       ...
     *      kcc::Log::info3("some message");
     *   }
     *
     * @author Ted V. Kremer
     */
    class KCC_CORE_EXPORT Log
    {
    public:
        /**
         * Verbosity levels
         */
        enum Verbosity { V_ERROR, V_WARNING, V_INFO_1, V_INFO_2, V_INFO_3, V_INFO_4 };

        /**
         * Logging scope. Can be set to provide additional information.
         */
        class KCC_CORE_EXPORT Scope
        {
        public:
            /**
             * Scope: create a scope object
             * @param context context of scope
             * @param name scope name
             */
            Scope(const Char* context, const Char* name, bool manage = true);
            ~Scope();

            // Accessors
            inline unsigned long threadId() { return m_threadId; }
            inline const Char*   context()  { return m_context; }
            inline const Char*   name()     { return m_name; }

        private:
            // Attributes
            bool          m_managed;
            unsigned long m_threadId;
            const Char*   m_context;
            const Char*   m_name;
        };
        
        /** Modifiers */
        static void verbosity(Verbosity v);

        /** Accessors */
        static Verbosity      verbosity();
        static String         path();
        static String         name();
        static String         ext();
        static String         file();
        static long           entries();
        static long           maxSize();
        static int            maxFiles();
        static void           history(StringSet& logs, bool clear = true);
        static ILogDecorator* decorator(); // ownership NOT consumed
        
        /** Utility */
        static const Char* verbosityName(Log::Verbosity verbosity);
        static Verbosity   verbosityEnum(const String& verbosity);

        /** 
         * Scope management 
         *   scopeMark() will begin marking the scope stack until
         *   scopeComplete() is called. scopeDump() will write the current
         *   scope without completing the scope stack.
         */
        static void scopeMark    ();
        static void scopeComplete(const Char* message);
        static void scopeDump    (const Char* message);

        /** Logging levels */
        static void info1    (const Char* format, ...);
        static void info2    (const Char* format, ...);
        static void info3    (const Char* format, ...);
        static void warning  (const Char* format, ...);
        static void error    (const Char* format, ...);
        static void out      (const Char* format, ...);
        static void info1    (const String& msg) { Log::info1(msg.c_str());   }
        static void info2    (const String& msg) { Log::info2(msg.c_str());   }
        static void info3    (const String& msg) { Log::info3(msg.c_str());   }
        static void warning  (const String& msg) { Log::warning(msg.c_str()); }
        static void error    (const String& msg) { Log::error(msg.c_str());   }
        static void out      (const String& msg) { Log::out(msg.c_str());     }
        static void exception(const std::exception& e);
        
        // Optimize away high verbosity logging
        #if defined(KCC_LOG_BRIEF)
            static void info4(const Char* format, ...) {}
            static void info4(const String& msg)       {}
        #else
            static void info4(const Char* format, ...);
            static void info4(const String& msg) { Log::info4(msg.c_str()); }
        #endif

        /**
         * Assertion: will abort process
         * @param context context of assertion
         * @param name name of assertion
         * @param message assertion message
         */
        static void assertion(const Char* context, const Char* name, const Char* message);

    private:
        Log();
    };

    /**
     * Decorate log writing to allow delegation to an alternate log store (GOF: Decorator) 
     *
     * @author Ted V. Kremer
     */
    interface ILogDecorator : IComponent
    {
        /**
         * Notification of log writing
         * NOTE: pointers are only valid during the call; make copies if needed
         * @param prefix   verbosity prefix (info1-info4, warning, error, exception, out)
         * @param threadId id of logging thread      (scope.threadId)
         * @param context  context of logging thread (scope.context)
         * @param name     name of logging context   (scope.name)
         * @param when     when logged (ISO date time)
         * @param text     log message text
         * @return true if kcc logger should log to file
         */
        virtual bool onWrite(
            const Char* prefix, 
            long        threadId, 
            const Char* context, 
            const Char* name, 
            const Char* when,
            const Char* text) = 0;
    };
}

/**
 * Log verbosity
 */
#define KCC_LOG_ERROR   "error"
#define KCC_LOG_WARNING "warning"
#define KCC_LOG_INFO4   "info4"
#define KCC_LOG_INFO3   "info3"
#define KCC_LOG_INFO2   "info2"
#define KCC_LOG_INFO1   "info1"
#define KCC_LOG_OUT     "out"

/**
 * Log configurations
 */
#define KCC_LOG_VERBOSITY "kcc.logVerbosity"
#define KCC_LOG_MAXSIZE   "kcc.logMaxSize"
#define KCC_LOG_MAXFILES  "kcc.logMax"  
#define KCC_LOG_PATH      "kcc.logPath"  
#define KCC_LOG_NAME      "kcc.logName"  
#define KCC_LOG_EXT       "kcc.logExt"  
#define KCC_LOG_STDOUT    "kcc.logStdOut"  
#define KCC_LOG_DECORATOR "kcc.logDecorator"  

#endif // Log_h
