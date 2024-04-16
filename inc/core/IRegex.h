/*
 * Kuumba C++ Core
 *
 * $Id: IRegex.h 22625 2008-03-09 22:51:49Z tvk $
 */
#ifndef IRegex_h
#define IRegex_h

namespace kcc
{
    interface IRegexExpression;

    /**
     * Regular expressions. Provider compiles and caches all expressions. 
     * For fine grained control of regex lifetime use an IRegexExpression.
     *
     * @author Ted V. Kremer
     */
    interface IRegex : IComponent
    {
        /** Regular expression failed exception */
        KCC_COMPONENT_EXCEPTION_BEGIN(Failed)
            String target;
            String regex;
            Failed(const String& what, const String& _target, const String& _regex) 
                : Exception(what), target(_target), regex(_regex)
            {}
        KCC_COMPONENT_EXCEPTION_END

        /**
         * Regular expression option flags
         */
        enum OptionFlags
        {
            O_ESCAPE_IN_LISTS = 1,
            O_CHAR_CLASSES    = 2,
            O_INTERVALS       = 4,
            O_BK_REFS         = 8,
            O_NOCOLLATE       = 16,
            O_ICASE           = 32,
            O_NORMAL          = O_ESCAPE_IN_LISTS|O_CHAR_CLASSES|O_INTERVALS|O_BK_REFS|O_NOCOLLATE
        };
        typedef unsigned int Options;
        
        /**
         * Lifecyle management
         */
        virtual void flush   (IRegexExpression* rx) = 0; // rx invalid on return
        virtual void flushAll() = 0;
        
        /**
         * Accessor to compiled regex expression. Will compile and cache expression if not cached.
         * @param regex regular expression 
         * @param ops regular expression options
         * @retrun regex expression (ownership NOT consumed)
         * @throw Failed if unable to execute expression
         */
        virtual IRegexExpression* expression(const String& regex, Options ops) throw (IRegex::Failed) = 0;
        
        /**
         * Split a string into a vector of strings based on regex
         * @param target target string
         * @param regex regular expression to split on
         * @param out out collection of split strings (cleared on entry)
         * @param ops regular expression options
         * @param clear clear out vector
         * @throw Failed if unable to execute expression
         */
        virtual void split(const String& target, const String& regex, StringVector& out, Options ops = IRegex::O_NORMAL, bool clear = true) throw (IRegex::Failed) = 0;
        virtual void split(String& target, const String& regex, StringVector& out, Options ops = IRegex::O_NORMAL, bool clear = true) throw (IRegex::Failed) = 0;

        /**
         * Match a target string against a regex
         * @param target target string
         * @param regex regular expression to split on
         * @param out vector of match targets
         * @param ops regular expression options
         * @param clear clear out parts
         * @return true if a match on target
         * @throw Failed if unable to execute expression
         */
        virtual bool match(const String& target, const String& regex, Options Options = IRegex::O_NORMAL) throw (IRegex::Failed) = 0;
        virtual bool match(const String& target, const String& regex, StringParts& out, Options Options = IRegex::O_NORMAL, bool clear = true) throw (IRegex::Failed) = 0;

        /**
         * Replace a target string against a regex
         * @param target target string
         * @param regex regular expression to split on
         * @param format format string to use for replacement
         * @param ops regular expression options
         * @return relpaces string
         * @throw Failed if unable to execute expression
         */
        virtual String replace(const String& target, const String& regex, const String& format = Strings::empty(), Options ops = IRegex::O_NORMAL) throw (IRegex::Failed) = 0;
        virtual void   replace(String& target, const String& regex, const String& format = Strings::empty(), Options ops = IRegex::O_NORMAL) throw (IRegex::Failed) = 0;
    };
    
    /**
     * Regex expression
     *
     * @author Ted V. Kremer
     */
    interface IRegexExpression : IComponent
    {
        /**
         * Compile regex expression
         * @param expr regular expression to split on
         * @param ops regular expression options
         * @throw Failed if unable to compile expression
         */
        virtual void compile(const String& expr, IRegex::Options ops = IRegex::O_NORMAL) throw (IRegex::Failed) = 0;
        
        /** Accessors */
        virtual const String&   id        () = 0;
        virtual const String&   expression() = 0;
        virtual IRegex::Options options   () = 0;
    
        /**
         * Split a string into a vector of strings based on regex
         * @param target target string
         * @param out out collection of split strings (cleared on entry)
         * @param clear clear out vector
         * @throw Failed if unable to execute expression
         */
        virtual void split(const String& target, StringVector& out, bool clear = true) throw (IRegex::Failed) = 0;
        virtual void split(String& target, StringVector& out, bool clear = true) throw (IRegex::Failed) = 0;

        /**
         * Match a target string against a regex
         * @param target target string
         * @param out out collection of split strings (cleared on entry)
         * @param clear clear out parts
         * @return true if a match on target
         * @throw Failed if unable to execute expression
         */
        virtual bool match(const String& target) throw (IRegex::Failed) = 0;
        virtual bool match(const String& target, StringParts& out, bool clear = true) throw (IRegex::Failed) = 0;

        /**
         * Replace a target string against a regex
         * @param target target string
         * @param regex regular expression to split on
         * @param format format string to use for replacement
         * @return relpaces string
         * @throw Failed if unable to execute expression
         */
        virtual String replace(const String& target, const String& format = Strings::empty()) throw (IRegex::Failed) = 0;
        virtual void   replace(String& target, const String& format = Strings::empty()) throw (IRegex::Failed) = 0;
    };

    /**
     * Regex factory specification
     *
     * @author Ted V. Kremer
     */
    interface IRegexFactory : IComponentFactory
    {
        /**
         * Construct regex (default construct) (ownership IS consumed)
         */
        virtual IRegex* constructRegex() = 0;
    
        /**
         * Construct regex expression (ownership IS consumed)
         */
        virtual IRegexExpression* constructExpression() = 0;
    };
}

#endif // IRegex_h
