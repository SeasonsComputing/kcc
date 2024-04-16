/*
 * Kuumba C++ Core
 *
 * $Id: Core.cpp 21993 2008-02-05 18:55:20Z tvk $
 */
#include <inc/core/Core.h>

// Windows - track memory leaks for C-runtime and COM
#if defined(KCC_WINDOWS) && defined(KCC_DEBUG)
#   include "windows.h"
#   include "crtdbg.h"
    static _CrtMemState k_memState = {0}; // CRT memory state checkpoint
#endif

#define KCC_FILE "Core"

namespace kcc
{
    // Properties
    static const String k_keySystemDir(KCC_CORE_SYSTEMDIR);
    static const String k_keyLocale   (KCC_CORE_LOCALE);
    static const String k_keyRODOM    (KCC_CORE_RODOM);
    static const String k_keyRegex    (KCC_CORE_REGEX);
    static const String k_keyAppSCM   (KCC_APPLICATION_SCM);
    static const String k_keyAppDebug (KCC_APPLICATION_DEBUG);
    static const String k_defLocale   ("C");
    static const String k_defRODOM    ("k_rodom");
    static const String k_defRegex    ("k_regex");

    //
    // SystemState implementation
    //

    // Helper class to aggregate global system state
    struct SystemState
    {
        // ctor/dtor
        SystemState(const Properties& config, const Char* scm) : 
            m_properties(config) 
        {
            // scm
            m_properties.set(k_keyAppSCM, scm);
            
            // debugging
            #if defined(KCC_DEBUG)
                long debugging = KCC_PROPERTY_TRUE;
            #else
                long debugging = KCC_PROPERTY_FALSE;
            #endif
            m_properties.set(k_keyAppDebug, debugging);
        }
        ~SystemState()
        {
            Mutex::Lock lock(m_sentinel);
            m_rodom.reset();
            m_regex.reset();
            while (!m_moduleStates.empty())
            {
                delete m_moduleStates.top();
                m_moduleStates.pop();
            }
        }

        // sentinel: accessor to global state mutex
        inline Mutex& sentinel() { return m_sentinel; }

        // module: get module state by name
        Core::ModuleState* module(const Char* n)
        {
            Mutex::Lock lock(m_sentinel);
            Core::ModuleState* s = m_namedModuleStates[n];
            return s;
        }

        // module: set module state by name
        void module(const Char* n, Core::ModuleState* s)
        {
            Mutex::Lock lock(m_sentinel);
            if (m_namedModuleStates[n] == NULL) m_moduleStates.push(s);
            m_namedModuleStates[n] = s;
        }

        // properties: accessor to properties
        Properties& properties() { return m_properties; }

        // rodom: lazy creation of system rodom
        IRODOM* rodom() throw (Exception)
        {
            Mutex::Lock lock(m_sentinel);
            if (m_rodom == NULL)
            {
                String rodom(Core::properties().get(k_keyRODOM, k_defRODOM));
                m_rodom = KCC_COMPONENT(IRODOM, rodom);
            }
            return m_rodom;
        }

        // regex: lazy creation of system regex
        IRegex* regex() throw (Exception)
        { 
            Mutex::Lock lock(m_sentinel);
            if (m_regex == NULL)
            {
                String regex(Core::properties().get(k_keyRegex, k_defRegex));
                m_regex = KCC_COMPONENT(IRegex, regex);
            }
            return m_regex; 
        }

    private:
        // Attributes
        typedef std::map<String, Core::ModuleState*> NamedModuleState;
        typedef std::stack<Core::ModuleState*>       ModuleStates;
        Mutex            m_sentinel;
        NamedModuleState m_namedModuleStates;
        ModuleStates     m_moduleStates;
        Properties       m_properties;
        AutoPtr<IRODOM>  m_rodom;
        AutoPtr<IRegex>  m_regex;
    };

    //
    // SystemStateMgr implementation
    //

    /*
     * The system root cosmic object is created first and deleted last,
     * will log memory leaks (Windows only), and cleans module states prior
     * to system shut down.
     */
    static struct SystemStateMgr
    {
        SystemStateMgr() : m_state(NULL) {}
        ~SystemStateMgr() { exit(); }

        // system root state accessor
        SystemState& state()
        {
            if (m_state == NULL)
            {
                std::cerr << "Core::init() must be called" << std::endl;
                std::abort();
            }
            return *m_state;
        }

        // init: initialize system state
        void init(const Properties& config, const Char* scm)
        {
            if (m_state != NULL)
            {
                std::cerr << "Core::init() already called" << std::endl;
                std::abort();
            }

            // install Window mem leak checking when debugging
            #if defined(KCC_WINDOWS) && defined(KCC_DEBUG)
                ::_CrtMemCheckpoint(&::k_memState);
            #endif

            // initialize system
            m_state = new SystemState(config, scm);

            // system path
            String sp(config.get(k_keySystemDir, Strings::empty()));
            if (!sp.empty()) Platform::fsDirChange(sp);

            // set locale
            String l(config.get(k_keyLocale, k_defLocale));
            std::locale::global(std::locale(l.c_str()));

            // initialize logging
            Log::Scope scope(KCC_FILE, "init");
        }

        // exit: cleanup system state
        void exit()
        {
            if (m_state == NULL) return;

            // log exit
            {
                Log::Scope scope(KCC_FILE, "exit");
                Log::info1("kcc cleaning up");
            }

            #if defined(KCC_WINDOWS) && defined(KCC_DEBUG)
                // save final log file prior to state delete
                String log(Log::file());
            #endif

            // clean-up system root
            delete m_state;
            m_state = NULL;

            // Windows exit
            #if defined(KCC_WINDOWS) && defined(KCC_DEBUG)
                // append memory leaks to log file as XML comment
                HANDLE h = ::CreateFile(
                    log.c_str(),
                    GENERIC_WRITE, FILE_SHARE_READ,
                    NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                ::SetFilePointer(h, ::GetFileSize(h, NULL), NULL, FILE_BEGIN);

                // clean objects so not to appear as leak
                log.~log();

                // open XML comment
                {
                    DWORD bytes;
                    String msg("<!-- \n");
                    ::WriteFile(h, (void*)msg.c_str(), (DWORD)msg.length(), &bytes, NULL);
                }

                // write memory leaks to std err and log file
                ::_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
                ::_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
                ::_CrtMemDumpAllObjectsSince(&::k_memState);
                ::_CrtSetReportFile(_CRT_WARN, h);
                ::_CrtMemDumpAllObjectsSince(&::k_memState);

                // close XML comment
                {
                    DWORD bytes;
                    String msg("-->\n");
                    ::WriteFile(h, (void*)msg.c_str(), (DWORD)msg.length(), &bytes, NULL);
                }

                ::CloseHandle(h);
            #endif
        }
    
    private:
        // Attributes
        SystemState* m_state;
    } k_system;

    //
    //  Implementation
    //

    void Core::init(const Properties& properties, const Char* scm) { k_system.init(properties, scm); }
    const Properties& Core::properties()            { return k_system.state().properties(); }
    IRODOM* Core::rodom() throw (Exception)         { return k_system.state().rodom(); }
    IRegex* Core::regex() throw (Exception)         { return k_system.state().regex(); }
    Mutex& Core::sentinel()                         { return k_system.state().sentinel(); }
    Core::ModuleState* Core::state(const Char* n)   { return k_system.state().module(n); }
    void Core::state(const Char* n, ModuleState* s) { k_system.state().module(n, s); }
}
