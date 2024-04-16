/*
 * Kuumba C++ Core
 *
 * $Id: Core.h 22625 2008-03-09 22:51:49Z tvk $
 */
#ifndef Core_h
#define Core_h

/*
 * Core.h
 *    "Cosmic" include for all kcc-based applications & components
 *      - never included in a header file
 *      - always the first include in a source file
 */

/**
 * Defines:
 *    KCC_WINDOWS  Compiling for the Windows platform
 *    KCC_LINUX    Compiling for the Linux platform
 *    KCC_CORE     Compiling Core library
 *    KCC_DEBUG    Compiling with debugging on
 *    KCC_NO_RTTI  Compiling without RTTI (to minimize size)
 */
#if defined(KCC_WINDOWS)
#   define KCC_EXPLORT_DECL __declspec(dllexport)
#   if defined(KCC_CORE)
#       define KCC_CORE_EXPORT KCC_EXPLORT_DECL
#   else
#       define KCC_CORE_EXPORT
#   endif
#   define CINTERFACE              // fix symbol conflict when including native win32 headers
#   define _CRT_SECURE_NO_WARNINGS // disable vc9 security warnings in 3rd-party libraries
#   pragma warning(disable : 4290) // throw declaration used (not supported by VC++)
#   pragma warning(disable : 4251) // STL classes throw DLL interface errors
#   pragma warning(disable : 4275) // warning when exported class derives from pure-virtual interface
#elif defined (KCC_LINUX)
#   define KCC_EXPLORT_DECL
#   define KCC_CORE_EXPORT
#else
#   error Compiling for undefined platform. Use KCC_WINDOWS or KCC_LINUX.
#endif

/* Standard C++ libraries */
#include <stdarg.h>
#include <memory>
#include <string>
#include <vector>
#include <stack>
#include <list>
#include <map>
#include <set>
#include <queue>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <locale>
#include <cstdlib>
#include <cmath>
#include <limits>
#include <stdexcept>

/* Core: abstract data types */
#include <inc/core/AutoPtr.h>
#include <inc/core/StringTypes.h>
#include <inc/core/ISODate.h>
#include <inc/core/Exception.h>
#include <inc/core/Strings.h>
#include <inc/core/Dictionary.h>
#include <inc/core/Properties.h>

/* Core: component foundation */
#if !defined(interface)
#   define interface struct
#endif
#include <inc/core/IComponent.h>

/* Core infrastructure: components */
#include <inc/core/IRODOM.h>
#include <inc/core/IRegex.h>

/* Core infrastructure: foundation */
#include <inc/core/MD5.h>
#include <inc/core/UUID.h>
#include <inc/core/URL.h>
#include <inc/core/Platform.h>
#include <inc/core/Timer.h>
#include <inc/core/Log.h>
#include <inc/core/Thread.h>
#include <inc/core/Socket.h>
#include <inc/core/HTTP.h>
#include <inc/core/DOMReader.h>
#include <inc/core/DOMWriter.h>
#include <inc/core/ComponentFactory.h>
#include <inc/core/ComponentModule.h>
#include <inc/core/Components.h>

namespace kcc
{
    /**
     * Core: system root object
     *
     * INITIALIZATION
     *   init() - call once at the top of any executable (not components)
     *
     * CONFIGURATION PROPERTIES
     *   kcc.systemDir - abosolute path to kcc components & config files
     *   kcc.locale    - name of locale to use
     *   kcc.rodom     - name of default RODOM component
     *   kcc.regex     - name of default REGEX component
     *
     * GENERAL ACCESSORS/MODIFIERS:
     *   properties() - system global properties
     *   rodom()      - rodom provider (ownership NOT consumed)
     *   regex()      - regex provider (ownership NOT consumed)
     *
     * MODULE STATE ACCESSORS/MODIFIERS:
     *   Keep track of module state variables (file statics).
     *
     *   ModuleState - base class for module state
     *   state()     - module state by name
     *   KCC_STATE   - accessor to state object for module
     *
     *   This allows all statics creation order to be controlled and to be cleaned prior to
     *   application shutdown so leaks may be tracked.
     *
     *   ModuleState derivitive class names must be globally unique!!!
     *
     *   USAGE:
     *     ... kccBar.cpp ...
     *     struct BarModuleState : kcc::ModuleState
     *     {
     *         ...
     *     };
     *
     *     void kccBar::bar()
     *     {
     *         KCC_STATE(BarModuleState).foo(...);
     *     }
     *
     * @author Ted V. Kremer
     */
    class KCC_CORE_EXPORT Core
    {
    public:
        /**
         * Initialize infrastructure (MUST BE CALLED)
         * @param config system properties to inialize with
         * @param scm configuration management id of executable file
         */
        static void init(const Properties& config, const Char* scm);

        //
        // General accessors/modifiers
        //

        /**
         * Accessor to system properties
         * @return system properties
         */
        static const Properties& properties();

        /**
         * Accessor to system rodom
         * @return rodom instance (ownership NOT consumed)
         * @throw Exception if unable to load component
         */
        static IRODOM* rodom() throw (Exception);

        /**
         * Accessor to regular expression (ownership NOT consumed)
         * @return regular expression component
         * @throw Exception if unable to load component
         */
        static IRegex* regex() throw (Exception);

        //
        // Module state management
        //

        /**
         * Module state abstract class.
         * Will expand a typesafe instance() method into all subclasses.
         */
        struct ModuleState
        {
            /** Singleton (GOF) - Implementation, use KCC_STATE() instead */
            template<class _C> static _C& instance(const Char* name)
            {
                kcc::Mutex::Lock lock(kcc::Core::sentinel());
                _C* state = (_C*) kcc::Core::state(name);
                if (state == NULL)
                {
                    state = new _C;
                    kcc::Core::state(name, state);
                }
                return *state;
            }

        protected:
            /* Implementation */
            friend struct SystemState;
            virtual ~ModuleState() {}
        };

    private:
        /* Implementation */
        Core();
        friend struct ModuleState;
        static Mutex&       sentinel();
        static ModuleState* state(const Char* n);
        static void         state(const Char* n, ModuleState* s);
    };
}

/**
 * Assertion macro (debug only)
 * @param _TEST condition to assert on if false
 * @param _CONTEXT assertion context
 * @param _NAME assertion content name
 * @param _MESSAGE assrtion message
 */
#if defined(KCC_DEBUG)
#   define KCC_ASSERT(_TEST, _CONTEXT, _NAME, _MESSAGE) { if (!(_TEST)) kcc::Log::assertion(_CONTEXT, _NAME, _MESSAGE);  }
#else
#   define KCC_ASSERT(_TEST, _CONTEXT, _NAME, _MESSAGE)
#endif

/**
 * Accessor to module state
 * @param _C class of state object
 */
#define KCC_STATE(_C) _C::instance<_C>(#_C)

/**
 * System configurations
 */
#define KCC_CORE_SYSTEMDIR "kcc.systemDir"
#define KCC_CORE_LOCALE    "kcc.locale"
#define KCC_CORE_RODOM     "kcc.rodom"
#define KCC_CORE_REGEX     "kcc.regex"

/**
 * Application configurations
 */
#define KCC_APPLICATION_NAME  "kcc.appName"
#define KCC_APPLICATION_SCM   "kcc.appSCM"
#define KCC_APPLICATION_DEBUG "kcc.appDebug"

#endif // Core_h
