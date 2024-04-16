/*
 * Kuumba C++ Core
 *
 * $Id: Log.cpp 22762 2008-03-24 16:50:17Z tvk $
 */
#include <inc/core/Core.h>

#define KCC_FILE "Log"

namespace kcc
{
    // Pseudo-Constant: write to standard out
    #if defined(KCC_DEBUG)
        static bool k_stdOut = true;
    #else
        static bool k_stdOut = false;
    #endif

    // Properties
    static const String k_keyVerbosity(KCC_LOG_VERBOSITY);
    static const String k_keyMaxSize  (KCC_LOG_MAXSIZE);
    static const String k_keyMaxFiles (KCC_LOG_MAXFILES);
    static const String k_keyPath     (KCC_LOG_PATH);
    static const String k_keyName     (KCC_LOG_NAME);
    static const String k_keyExt      (KCC_LOG_EXT);
    static const String k_keyStdOut   (KCC_LOG_STDOUT);
    static const String k_keyDecorator(KCC_LOG_DECORATOR);
    static const String k_keyAppSCM   (KCC_APPLICATION_SCM);
    static const String k_keyAppDebug (KCC_APPLICATION_DEBUG);
    static const long   k_defMaxSize   = 1024L * 1024L * 2L; // 2 meg boundaries
    static const long   k_defMaxFiles  = 7L;
    static const String k_defPath     ("logs");
    static const String k_defName     ("log");
    static const String k_defExt      ("xml");
    
    // Constants
    static const Char* k_error   = KCC_LOG_ERROR;
    static const Char* k_warning = KCC_LOG_WARNING;
    static const Char* k_info4   = KCC_LOG_INFO4;
    static const Char* k_info3   = KCC_LOG_INFO3;
    static const Char* k_info2   = KCC_LOG_INFO2;
    static const Char* k_info1   = KCC_LOG_INFO1;
    static const Char* k_out     = KCC_LOG_OUT;
    static const Log::Verbosity k_logDefVerbosity = Log::V_INFO_1;
    static const std::size_t    k_szLogDetail     = 1024*4; // 4K log message

    //
    // LogModuleState Implementation
    //

    /** Track state static class */
    struct LogModuleState : Core::ModuleState
    {
        // Helper structure for scope stack
        struct ScopeStack : std::vector<Log::Scope*>
        {
            ScopeStack()             { reserve(32);  }
            void push(Log::Scope* s) { push_back(s); }
            Log::Scope* top()        { return empty() ? NULL : back(); }
            Log::Scope* pop()        
            { 
                if (empty()) return NULL;
                Log::Scope* t = back(); 
                pop_back();
                return t; 
            }
        };
        typedef std::map<long, ScopeStack> ThreadScopeStack;
        typedef std::list<ScopeStack*>     ThreadScopeStackOrdered;

        // Helper structure to track marked scope
        struct ScopeMarker { long threadId; String context; String name; };
        typedef std::vector<ScopeMarker> ScopeMarkers;

        // Attributes
        ScopeMarkers             m_markers;
        ThreadScopeStack         m_scopes;
        ThreadScopeStackOrdered  m_scopesOrdered;
        Log::Verbosity           m_verbosity;
        String                   m_file;
        std::FILE*               m_out;
        Mutex                    m_sentinel;
        long                     m_maxSize;
        long                     m_entries;
        AutoPtr<ILogDecorator>   m_decorator;

        // LogModuleState: create log file
        LogModuleState() :
            m_verbosity(Log::V_ERROR),
            m_out(NULL),
            m_maxSize(0L),
            m_entries(0L)
        {
            // load decorator
            const String& decoratorComp = Core::properties().get(k_keyDecorator, Strings::empty());
            if (!decoratorComp.empty())
            {
                try
                {
                    m_decorator = KCC_COMPONENT(ILogDecorator, decoratorComp);
                }
                catch (std::exception& e)
                {
                    std::cerr << 
                        "Unable to construct log decortor: " << 
                        "component=[" << decoratorComp << "] " <<
                        "what=[" << e.what() << "] " << std::endl;
                    std::abort();
                }
            }

            // create log
            create(); 
        }

        // ~LogModuleState: close log file
        ~LogModuleState() { close(); }
        
        // verbosity: modifier to current log verbosity
        void verbosity(Log::Verbosity v) 
        {
            Mutex::Lock lock(m_sentinel);
            m_verbosity = v;

            // record log verbosity change
            if (m_verbosity >= Log::V_INFO_1)
            {
                String msg(Strings::printf("verbosity changed: %s", Log::verbosityName(m_verbosity)));
                Log::Scope scope(KCC_FILE, "verbosity");
                write(k_info1, msg.c_str(), k_stdOut);
            }
        }

        // verbosity: accessor to current log verbosity
        Log::Verbosity verbosity() 
        { 
            Mutex::Lock lock(m_sentinel);
            Log::Verbosity v = m_verbosity;
            return v; 
        }
        
        // file: accessor to current log file
        String file() 
        { 
            Mutex::Lock lock(m_sentinel);
            return m_file; 
        }

        // entries: accessor to current log entries
        long entries() 
        { 
            Mutex::Lock lock(m_sentinel);
            long e = m_entries;
            return e; 
        }

        // history: retrieve log history files        
        void history(StringSet& logs, bool clear = true)
        {
            Mutex::Lock lock(m_sentinel);
            if (clear) logs.clear();
            String logPath = path();
            String logName = name();
            String logExt  = ext();
            Platform::Files files;
            Platform::fsDir(logPath, files);
            for (
                Platform::Files::iterator i = files.begin();
                i != files.end();
                i++)
            {
                const Platform::File& f = *i;
                if (
                    f.name.substr(0, logName.length()) == logName &&
                    f.name.substr(f.name.length() - logExt.length(), f.name.length()) == logExt)
                {
                    String l(Platform::fsFullPath(logPath, f.name));
                    if (l != m_file) logs.insert(l); // don't add current (incomplete) log
                }
            }
        }
        
        // Accessors
        String         path()      { return Core::properties().get(k_keyPath, k_defPath); }
        String         name()      { return Core::properties().get(k_keyName, k_defName); }
        String         ext()       { return Core::properties().get(k_keyExt,  k_defExt); }
        int            maxFiles()  { return (int)Core::properties().get(k_keyMaxFiles, k_defMaxFiles); }
        long           maxSize()   { return Core::properties().get(k_keyMaxSize, k_defMaxSize); }
        ILogDecorator* decorator() { return m_decorator; }

        // create: create log
        void create()
        {
            Mutex::Lock lock(m_sentinel);

            // initialize log
            m_entries      = 0L;
            m_verbosity    = Log::verbosityEnum(Core::properties().get(k_keyVerbosity, Strings::empty()));
            m_maxSize      = maxSize();
            int    max     = maxFiles();
            String logPath = path();
            String logName = name();
            String logExt  = ext();
            
            // override std-out setting
            if (Core::properties().exists(k_keyStdOut))
                k_stdOut = Core::properties().get(k_keyStdOut, KCC_PROPERTY_TRUE) == KCC_PROPERTY_TRUE;

            // make sure path exists
            Platform::fsDirCreate(logPath);

            // build sorted collection of historical log file names
            StringSet logs;
            history(logs);

            // remove old log files based on max
            int c = 0;
            for (
                StringSet::reverse_iterator li = logs.rbegin();
                li != logs.rend();
                li++)
            {
                const String& log = *li;
                if (++c >= max) Platform::fsRemove(log);
            }

            // open log file
            if (max < 2) m_file = Platform::fsFullPath(logPath, logName) + "." + logExt;
            else
            {
                std::time_t now;
                std::time(&now);
                m_file =
                    Platform::fsFullPath(logPath, logName) +
                    Strings::printf("%d", now) +
                    "." + logExt;
            }
            m_out = std::fopen(m_file.c_str(), "w");
            if (NULL == m_out)
            {
                std::cerr << "Unable to open log file" << std::endl;
                std::abort();
            }
            std::fprintf(
                m_out, 
                "<?xml version='1.0' encoding='UTF-8' ?>\n"
                "<Log>\n");
            std::fflush(m_out);
            
            // record log creation
            if (m_verbosity >= Log::V_INFO_1)
            {
                String msg(Strings::printf(
                    "app-scm=[%s] app-debug=[%d] created=[%s] name=[%s] file=[%s] verbosity=[%s] max=[%d]",
                    Core::properties().get(k_keyAppSCM, Strings::empty()).c_str(),
                    Core::properties().get(k_keyAppDebug, KCC_PROPERTY_FALSE),
                    ISODate::local().isodatetime().c_str(), 
                    logName.c_str(), m_file.c_str(), 
                    Log::verbosityName(m_verbosity), max));
                Log::Scope scope(KCC_FILE, "create", false);
                push(&scope);
                write(k_info1, msg.c_str(), k_stdOut);
                pop(&scope);
            }
        }

        // exceptionMark: get scope stack for exception message
        void scopeMark()
        {
            Mutex::Lock lock(m_sentinel);
            if (!m_markers.empty()) return; // keep root stack only
            for (ThreadScopeStackOrdered::iterator i = m_scopesOrdered.begin(); i != m_scopesOrdered.end(); i++)
            {
                ScopeStack* stack = *i;
                for (ScopeStack::iterator j = stack->begin(); j != stack->end(); j++)
                {
                    m_markers.push_back(ScopeMarker());
                    ScopeMarker& m = m_markers.back();
                    m.threadId = (*j)->threadId();
                    m.context  = (*j)->context();
                    m.name     = (*j)->name();
                }
            }
        }

        // scopeComplete: write scope stack and complete tracking
        void scopeComplete(const Char* prefix, const Char* text)
        {
            Mutex::Lock lock(m_sentinel);
            scopeDump(prefix, text);
            m_markers.clear();
        }

        // scopeDump: write scope stack
        void scopeDump(const Char* prefix, const Char* text)
        {
            Mutex::Lock lock(m_sentinel);
            String msg(text);
            msg += " \nscopeStack \n{";
            for (int i = m_markers.size() - 1; i >= 0; i--)
            {
                ScopeMarker& m = m_markers[i];
                msg += " \n\t[";
                msg += Strings::printf("%ld", m.threadId);
                msg += "] ";
                msg += m.context;
                msg += "::";
                msg += m.name;
            }
            msg += " \n} \n";
            write(prefix, msg.c_str(), false, true);
        }

        // close: close log
        void close()
        {
            Mutex::Lock lock(m_sentinel);
            std::fprintf(m_out, "</Log>\n");
            std::fflush(m_out);
            std::fclose(m_out);
        }

        // roll: roll log over
        void roll()
        {
            Mutex::Lock lock(m_sentinel);
            close();
            create();
        }

        // push: add to scope stack
        void push(Log::Scope* s)
        {
            Mutex::Lock lock(m_sentinel);
            ScopeStack& stack = m_scopes[s->threadId()];
            if (stack.empty()) m_scopesOrdered.push_back(&stack);
            stack.push(s);
        }

        // pop: remove from scope stack
        void pop(Log::Scope* s)
        {
            Mutex::Lock lock(m_sentinel);
            ScopeStack& stack = m_scopes[s->threadId()];
            stack.pop();
            if (stack.empty()) 
            {
                ThreadScopeStackOrdered::iterator find = 
                    std::find(m_scopesOrdered.begin(), m_scopesOrdered.end(), &stack);
                if (find != m_scopesOrdered.end())
                {
                    m_scopesOrdered.erase(find);
                }
                else
                {
                    std::fprintf(stderr, "kcc::Log::pop - internal state error: scope not found in ordered stack\n");
                    std::fflush(stderr);
                }
            }
        }

        // write: write to log
        void write(const Char* prefix, const Char* text, bool toStdOut = false, bool toStdErr = false, bool logFormatStdOut = true)
        {
            Mutex::Lock lock(m_sentinel);

            // fetch scope for log text
            unsigned long threadId = 0L;
            const Char*   context  = "{context-missing}";
            const Char*   name     = "{name-missing}";
            if (!m_scopes.empty())
            {
                unsigned long curr  = Thread::current();
                ScopeStack&   stack = m_scopes[curr];
                if (stack.empty())
                {
                    std::fprintf(stderr, "kcc::Log::write - internal state error: thread id not found in scope stack. missing Log::Scope?\n");
                    std::fflush(stderr);
                }
                else
                {
                    Log::Scope* s = stack.top();
                    if (s == NULL)
                    {
                        std::fprintf(stderr, "kcc::Log::write - internal state error: scope stack empty for thread id. missing Log::Scope?\n");
                        std::fflush(stderr);
                    }
                    else
                    {
                        threadId = s->threadId();
                        context  = s->context();
                        name     = s->name();

                        // verify thread state when debugging
                        #if defined(KCC_DEBUG)
                            if (threadId != curr)
                            {
                                std::fprintf(stderr, "kcc::Log::write - internal state error: scope thread id NOT current thread id\n");
                                std::fflush(stderr);
                            }
                        #endif
                    }
                }
            }

            // dump log text
            m_entries++;
            String when(ISODate::local().isodatetime());
            bool toLog = true;
            if (!m_decorator.null()) toLog = m_decorator->onWrite(prefix, threadId, context, name, when.c_str(), text);
            if (toLog)
            {
                std::fprintf(
                    m_out,
                    "<Entry what='%s' thd='%ld' ctx='%s' name='%s' when='%s'><![CDATA[%s]]></Entry>\n",
                    prefix, threadId, context, name, when.c_str(), text);
                std::fflush(m_out);
            }

            // dump to std out
            if (toStdOut || toStdErr)
            {
                if (toStdOut)
                {
                    if (logFormatStdOut)
                        std::fprintf(stdout, "%08ld %-10s %-20s %-25s %s %s\n", threadId, prefix, context, name, when.c_str(), text);
                    else
                        std::fprintf(stdout, "%s\n", text);
                    std::fflush(stdout);
                }
                if (toStdErr)
                {
                    std::fprintf(stderr, "%08ld %-10s %-20s %-25s %s %s\n", threadId, prefix, context, name, when.c_str(), text);
                    std::fflush(stderr);
                }
            }

            // check max size and roll if over
            if (std::ftell(m_out) > m_maxSize) roll();
        }
    };

    //
    // Scope Implementation
    //

    // Scope: add scope to scope-stack
    Log::Scope::Scope(const Char* context, const Char* name, bool manage)
        : m_managed(manage), m_threadId(Thread::current()), m_context(context), m_name(name)
    {
        if (m_managed) KCC_STATE(LogModuleState).push(this);
    }

    // ~Scope: remove scope from scope-stack
    Log::Scope::~Scope() { if (m_managed) KCC_STATE(LogModuleState).pop(this); }

    //
    // Log Implementation
    //
    
    // Modifiers
    void Log::verbosity(Log::Verbosity v) { KCC_STATE(LogModuleState).verbosity(v); }
    
    // Accessors
    Log::Verbosity Log::verbosity() { return KCC_STATE(LogModuleState).verbosity(); }
    String         Log::path()      { return KCC_STATE(LogModuleState).path(); }
    String         Log::name()      { return KCC_STATE(LogModuleState).name(); }
    String         Log::ext()       { return KCC_STATE(LogModuleState).ext(); }
    String         Log::file()      { return KCC_STATE(LogModuleState).file(); }
    long           Log::entries()   { return KCC_STATE(LogModuleState).entries(); }
    long           Log::maxSize()   { return KCC_STATE(LogModuleState).maxSize(); }
    int            Log::maxFiles()  { return KCC_STATE(LogModuleState).maxFiles(); }
    void           Log::history(StringSet& logs, bool clear) { return KCC_STATE(LogModuleState).history(logs, clear); }
    ILogDecorator* Log::decorator() { return KCC_STATE(LogModuleState).decorator(); }

    // Utility
    const Char* Log::verbosityName(Log::Verbosity v)
    {
        const Char* n = k_error;
        if      (v == Log::V_INFO_4)  n = k_info4;
        else if (v == Log::V_INFO_3)  n = k_info3;
        else if (v == Log::V_INFO_2)  n = k_info2;
        else if (v == Log::V_INFO_1)  n = k_info1;
        else if (v == Log::V_WARNING) n = k_warning;
        return n;
    }
    Log::Verbosity Log::verbosityEnum(const String& v)
    {
        Verbosity e = Log::V_ERROR;
        if      (v == k_info4)   e = Log::V_INFO_4;
        else if (v == k_info3)   e = Log::V_INFO_3;
        else if (v == k_info2)   e = Log::V_INFO_2;
        else if (v == k_info1)   e = Log::V_INFO_1;
        else if (v == k_warning) e = Log::V_WARNING;
        else                         
        {
            Log::Verbosity scalar = (Log::Verbosity) Strings::parseInteger(v);
            e = (scalar >= Log::V_ERROR && scalar <= Log::V_INFO_4) ? scalar : k_logDefVerbosity;
        }
        return e;
    }

#if !defined(KCC_LOG_BRIEF)
    // info4: write formated info
    void Log::info4(const Char* format, ...)
    {
        if (KCC_STATE(LogModuleState).verbosity() < Log::V_INFO_4) return;
        Char buf[k_szLogDetail];
        va_list arg;
        va_start(arg, format);
        kcc_vsprintf(buf, k_szLogDetail, format, arg);
        va_end(arg);
        buf[k_szLogDetail-1] = 0;
        KCC_STATE(LogModuleState).write(k_info4, buf, k_stdOut);
    }
#endif

    // info3: write formated info
    void Log::info3(const Char* format, ...)
    {
        if (KCC_STATE(LogModuleState).verbosity() < Log::V_INFO_3) return;
        Char buf[k_szLogDetail];
        va_list arg;
        va_start(arg, format);
        kcc_vsprintf(buf, k_szLogDetail, format, arg);
        va_end(arg);
        buf[k_szLogDetail-1] = 0;
        KCC_STATE(LogModuleState).write(k_info3, buf, k_stdOut);
    }

    // info2: write formated info
    void Log::info2(const Char* format, ...)
    {
        if (KCC_STATE(LogModuleState).verbosity() < Log::V_INFO_2) return;
        Char buf[k_szLogDetail];
        va_list arg;
        va_start(arg, format);
        kcc_vsprintf(buf, k_szLogDetail, format, arg);
        va_end(arg);
        buf[k_szLogDetail-1] = 0;
        KCC_STATE(LogModuleState).write(k_info2, buf, k_stdOut);
    }

    // info1: write formated info
    void Log::info1(const Char* format, ...)
    {
        if (KCC_STATE(LogModuleState).verbosity() < Log::V_INFO_1) return;
        Char buf[k_szLogDetail];
        va_list arg;
        va_start(arg, format);
        kcc_vsprintf(buf, k_szLogDetail, format, arg);
        va_end(arg);
        buf[k_szLogDetail-1] = 0;
        KCC_STATE(LogModuleState).write(k_info1, buf, k_stdOut);
    }

    // warning: write formated warning
    void Log::warning(const Char* format, ...)
    {
        if (KCC_STATE(LogModuleState).verbosity() < Log::V_WARNING) return;
        Char buf[k_szLogDetail];
        va_list arg;
        va_start(arg, format);
        kcc_vsprintf(buf, k_szLogDetail, format, arg);
        va_end(arg);
        buf[k_szLogDetail-1] = 0;
        KCC_STATE(LogModuleState).write(k_warning, buf, k_stdOut);
    }

    // error: write formated error
    void Log::error(const Char* format, ...)
    {
        Char buf[k_szLogDetail];
        va_list arg;
        va_start(arg, format);
        kcc_vsprintf(buf, k_szLogDetail, format, arg);
        va_end(arg);
        buf[k_szLogDetail-1] = 0;
        KCC_STATE(LogModuleState).write(k_error, buf, false, true);
    }

    // scopeMark/Complete/Dump: scope stack management
    void Log::scopeMark()                        { KCC_STATE(LogModuleState).scopeMark(); }
    void Log::scopeComplete(const Char* message) { KCC_STATE(LogModuleState).scopeComplete("scope", message); }
    void Log::scopeDump(const Char* message)     { KCC_STATE(LogModuleState).scopeDump("scope", message); }

    // exception: write exception
    void Log::exception(const std::exception& e) { KCC_STATE(LogModuleState).scopeComplete("exception", e.what()); }

    // assert: write assertion
    void Log::assertion(const Char* context, const Char* name, const Char* message)
    {
        Log::Scope scope(context, name); // make sure assertion is scoped
        KCC_STATE(LogModuleState).scopeMark();
        KCC_STATE(LogModuleState).scopeComplete("assert", message);
        std::abort();
    }

    // out: write formated info and std out
    void Log::out(const Char* format, ...)
    {
        Char buf[k_szLogDetail];
        va_list arg;
        va_start(arg, format);
        kcc_vsprintf(buf, k_szLogDetail, format, arg);
        va_end(arg);
        buf[k_szLogDetail-1] = 0;
        KCC_STATE(LogModuleState).write(k_out, buf, true, false, false);
    }
}
