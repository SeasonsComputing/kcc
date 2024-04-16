/*
 * Kuumba C++ Core
 *
 * $Id: Regex.cpp 20962 2007-10-06 18:27:12Z tvk $
 */
#include <inc/core/Core.h>
#include "RegexImport.h"

#define KCC_FILE    "Regex"
#define KCC_VERSION "$Id: Regex.cpp 20962 2007-10-06 18:27:12Z tvk $"

namespace kcc
{
    // Constants
    static const int k_maxMatches = 2048;
    
    // kcc_flagsBoost: convert to boost regex flags
    inline boost::regbase::flag_type kcc_flagsBoost(IRegex::Options ops)
    {
        boost::regbase::flag_type flags = 0;
        if ((ops & IRegex::O_ESCAPE_IN_LISTS) != 0) flags |= boost::regbase::escape_in_lists;
        if ((ops & IRegex::O_CHAR_CLASSES) != 0)    flags |= boost::regbase::char_classes;
        if ((ops & IRegex::O_INTERVALS) != 0)       flags |= boost::regbase::intervals;
        if ((ops & IRegex::O_BK_REFS) != 0)         flags |= boost::regbase::bk_refs;
        if ((ops & IRegex::O_NOCOLLATE) != 0)       flags |= boost::regbase::nocollate;
        if ((ops & IRegex::O_ICASE) != 0)           flags |= boost::regbase::icase;
        return flags;
    }
    
    // kcc_rxid: build unique id of regex
    inline StringRC kcc_rxid(const String& rx, IRegex::Options ops) 
    {
        return MD5::hash(rx + Strings::printf("__%d", ops));
    }

    // Helper class to manage regex cache
    struct RegexModuleState : Core::ModuleState
    {
        // Attributes
        typedef SharedPtr<IRegexExpression> RxValue;
        typedef std::map<String, RxValue>   RxCache;
        Mutex   m_sentinel;
        RxCache m_cache;
        ~RegexModuleState() { flushAll(); }

        // get: get a cached regular expression
        RxValue get(const String& regex, IRegex::Options ops) throw (IRegex::Failed)
        {
            Mutex::Lock lock(m_sentinel);
            Log::Scope scope(KCC_FILE, "RegexModuleState::get");
            StringRC id(kcc_rxid(regex, ops));
            RxValue rx;
            RxCache::iterator find = m_cache.find(id);
            if (find != m_cache.end()) rx = find->second;
            else
            {
                rx.reset(build(regex, ops));
                if (id != rx->id()) throw IRegex::Failed("regex expression id mismatch");
                m_cache[rx->id()] = rx;
            }
            return rx;
        }
        
        // flush: flush compiled expression
        void flush(IRegexExpression* rx)
        {
            Mutex::Lock lock(m_sentinel);
            Log::Scope scope(KCC_FILE, "RegexModuleState::flush");
            RxCache::iterator find = m_cache.find(rx->id());
            if (find != m_cache.end()) m_cache.erase(find);
            else                       
            {
                Log::warning("compiled regex not found, deleting instance: id=[" + rx->id() + "]");
                delete rx;
            }
        }
        
        // flush: flush compiled expression
        void flushAll()
        {
            Mutex::Lock lock(m_sentinel);
            m_cache.clear();
        }
        
        // build: construct and compile expression
        IRegexExpression* build(const String& regex, IRegex::Options ops) throw (IRegex::Failed);
    };

    // Regex implementation
    struct Regex : IRegex
    {
        // Lifecycle management
        void flush   (IRegexExpression* rx) { KCC_STATE(RegexModuleState).flush(rx);  }
        void flushAll()                     { KCC_STATE(RegexModuleState).flushAll(); }
        
        // Accessor
        IRegexExpression* expression(const String& regex, Options ops) 
            throw (IRegex::Failed)
        {
            return KCC_STATE(RegexModuleState).get(regex, ops);
        }
    
        // split: split regex into vector
        void split(String& target, const String& regex, StringVector& out, Options ops, bool clear)
            throw (IRegex::Failed)
        {
            return expression(regex, ops)->split(target, out, clear);
        }

        // split: split regex into vector
        void split(const String& target, const String& regex, StringVector& out, Options ops, bool clear)
            throw (IRegex::Failed)
        {
            expression(regex, ops)->split(target, out, clear);
        }

        // match: match target on regex
        bool match(const String& target, const String& regex, Options ops)
            throw (IRegex::Failed)
        {
            return expression(regex, ops)->match(target);
        }

        // match: match target on regex
        bool match(const String& target, const String& regex, StringParts& out, Options ops, bool clear)
            throw (IRegex::Failed)
        {
            return expression(regex, ops)->match(target, out, clear);
        }

        // replace: replace target string against a regex
        String replace(const String& target, const String& regex, const String& format, Options ops)
            throw (IRegex::Failed)
        {
            return expression(regex, ops)->replace(target, format);
        }

        // replace: replace target string against a regex
        void replace(String& target, const String& regex, const String& format, Options ops)
            throw (IRegex::Failed)
        {
            expression(regex, ops)->replace(target, format);
        }
    };

    // Expression implementation
    struct Expression : IRegexExpression
    {
        // Attributes
        StringRC              m_id;
        String                m_expr;
        IRegex::Options       m_ops;
        AutoPtr<boost::regex> m_rx;
        Expression() : m_id(kcc::Strings::emptyRC()), m_ops(IRegex::O_NORMAL) {}
        ~Expression()
        {
            Log::Scope scope(KCC_FILE, "Expression::~Expression");
            Log::info4("deleting regex: id=[%s]", m_id.c_str());
        }

        // compile: compile expression
        void compile(const String& expr, IRegex::Options ops) throw (IRegex::Failed)
        {
            Log::Scope scope(KCC_FILE, "Expression::compile");
            
            // compile/recompile expression
            m_id   = kcc_rxid(expr, ops);
            m_expr = expr;
            m_ops  = ops;
            try
            {
                bool recompile = m_rx != NULL;
                boost::regbase::flag_type flags = kcc_flagsBoost(m_ops);
                m_rx.reset(new boost::reg_expression<Char, boost::regex_traits<Char>, BOOST_DEFAULT_ALLOCATOR(Char)>(expr, flags));
                Log::info4(
                    "%s regex: id=[%s] ops=[%d] rx=[%s]", 
                    (recompile ? "recompiling" : "compiled"), 
                    m_id.c_str(), 
                    m_ops, 
                    m_expr.substr(0, 1024).c_str());
            }
            catch (std::exception& e)
            {
                throw IRegex::Failed(e.what(), Strings::empty(), expr);
            }
        }
        
        // validate: validate expression is compiled
        inline void validate() throw (IRegex::Failed) 
        { 
            if (m_rx == NULL) throw IRegex::Failed("regex not compiled");
        }
        
        // Accessors
        const String&   id        () { return m_id; }
        const String&   expression() { return m_expr; }
        IRegex::Options options   () { return m_ops; }
        
        // split: regex split
        void split(const String& target, StringVector& out, bool clear) throw (IRegex::Failed)
        {
            String t(target);
            split(t, out, clear);
        }
        
        // split: regex split
        void split(String& target, StringVector& out, bool clear) throw (IRegex::Failed)
        {
            Log::Scope scope(KCC_FILE, "Expression::split");
            validate();
            if (clear) out.clear();
            try
            {
                boost::regex_split(std::back_inserter(out), target, *m_rx);
            }
            catch (std::exception& e)
            {
                throw IRegex::Failed(e.what(), target, m_expr);
            }
        }
        
        // match: regex match
        bool match(const String& target) throw (IRegex::Failed)
        {
            Log::Scope scope(KCC_FILE, "Expression::match");
            validate();
            try
            {
                boost::smatch what;
                return boost::regex_search(target, what, *m_rx);
            }
            catch (std::exception& e)
            {
                throw IRegex::Failed(e.what(), target, m_expr);
            }
        }
        
        // match: regex match
        bool match(const String& target, StringParts& out, bool clear) throw (IRegex::Failed)
        {
            Log::Scope scope(KCC_FILE, "Expression::match");
            validate();
            if (clear) out.clear();
            try
            {
                std::string::const_iterator start   = target.begin();
                std::string::const_iterator end     = target.end(); 
                unsigned int                flags   = boost::match_default;
                boost::smatch               what;
                bool                        matches = false;
                while (boost::regex_search(start, end, what, *m_rx, flags)) 
                {
                    matches = true;
                
                    // add matches
                    for (boost::smatch::size_type i = 1; i < what.size(); i++) 
                        out.push_back(StringPart(what[i].first, what[i].second));
                        
                    // get next match from previous start
                    start = what[0].second; 
                    flags |= boost::match_prev_avail; 
                    flags |= boost::match_not_bob; 
                } 
                return matches;
            }
            catch (std::exception& e)
            {
                throw IRegex::Failed(e.what(), target, m_expr);
            }
        }
        
        // replace: regex replace
        String replace(const String& target, const String& format) throw (IRegex::Failed)
        {
            String t(target);
            replace(t, format);
            return t;
        }
        
        // replace: regex replace
        void replace(String& target, const String& format) throw (IRegex::Failed)
        {
            Log::Scope scope(KCC_FILE, "Expression::replace");
            validate();
            try
            {
                target = boost::regex_merge(target, *m_rx, format);
            }
            catch (std::exception& e)
            {
                throw IRegex::Failed(e.what(), target, m_expr);
            }
        }
    };
    
    // build: build expression using default impl.
    IRegexExpression* RegexModuleState::build(const String& regex, IRegex::Options ops) 
        throw (IRegex::Failed)
    {
        AutoPtr<IRegexExpression> rx(new Expression());
        rx->compile(regex, ops);
        return rx.release();
    }
    
    //
    // Regex factory
    //
    
    struct RegexFactory : IRegexFactory
    {
        IComponent*       construct()           { return constructRegex(); }
        IRegex*           constructRegex()      { return new Regex(); }
        IRegexExpression* constructExpression() { return new Expression(); }
    };

    KCC_COMPONENT_FACTORY_CUST(RegexFactory)
    
    KCC_COMPONENT_FACTORY_METADATA_BEGIN_CUSTOM(RegexFactory, IRegexFactory, Regex, IRegex)
        KCC_COMPONENT_FACTORY_METADATA_PROPERTY(KCC_COMPONENT_FACTORY_SCM, KCC_VERSION)
    KCC_COMPONENT_FACTORY_METADATA_END
}
