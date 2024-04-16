/*
 * Kuumba C++ Core
 *
 * $Id: StringTypes.h 20990 2007-10-09 19:33:43Z tvk $
 */
#ifndef StringTypes_h
#define StringTypes_h

#if defined(KCC_WINDOWS)
#   define kcc_vsprintf _vsnprintf
#elif defined(KCC_LINUX)
#   define kcc_vsprintf vsnprintf
#endif

namespace kcc
{
    /** String ADT's */
    typedef char                          Char;
    typedef std::basic_string<Char>       String;
    typedef std::vector<String>           StringVector;
    typedef std::list<String>             StringList;
    typedef std::set<String>              StringSet;
    typedef std::stack<String>            StringStack;
    typedef std::map<String, String>      StringMap;
    typedef std::multiset<String>         StringMultiset;
    typedef std::multimap<String, String> StringMultimap;
    typedef std::basic_stringstream<Char> StringStream;
    typedef std::pair<String::const_iterator, String::const_iterator> StringPart;
    typedef std::vector<StringPart>       StringParts;

    /**
     * Immutable ref-counted wrapper to string (NOT MT-safe)
     *
     * @author Ted V. Kremer
     */
    class StringRC
    {
    public:
        /**
         * Create ref-counted string
         *
         * @param s string to ref-count
         * @param rhs RC string to attach to
         */
        StringRC(const Char* s)       : m_data(new Data(s)) {}
        StringRC(const String& s)     : m_data(new Data(s)) {}
        StringRC(const StringRC& rhs) : m_data(NULL)        { assign(rhs); }
        ~StringRC() { clean(); }
        StringRC& operator = (const StringRC& rhs) { if (this != &rhs) { assign(rhs); } return *this; }

        /**
         * Accessors
         */
        inline operator const String& () const { return m_data->str; }
        inline const String& str      () const { return m_data->str; }
        inline const Char*   c_str    () const { return m_data->str.c_str(); }

        /**
         * Utility
         */
        inline bool empty  ()                    const { return m_data->str.empty(); }
        inline int  compare(const StringRC& rhs) const { return m_data->str.compare(rhs.m_data->str); }
        inline int  rc     ()                    const { return m_data->rc; }

    private:
        // Implemenation
        inline void assign(const StringRC& rhs)
        {
            clean();
            m_data = rhs.m_data;
            m_data->rc++;
        }
        inline void clean()
        {
            if (m_data != NULL)
            {
                m_data->rc--;
                if (m_data->rc==0) { delete m_data; m_data = NULL; }
            }
        }

        // Attributes
        struct Data
        {
            String str;
            int    rc;
            Data(const String& s) : str(s), rc(1) {}
        } *m_data;
    };
    inline const bool     operator <  (const StringRC& lhs, const StringRC& rhs) { return lhs.compare(rhs)<0; }
    inline const bool     operator == (const StringRC& lhs, const StringRC& rhs) { return (const String&)lhs == (const String&)rhs; }
    inline const bool     operator != (const StringRC& lhs, const StringRC& rhs) { return (const String&)lhs != (const String&)rhs; }
    inline const StringRC operator +  (const StringRC& lhs, const StringRC& rhs) { return StringRC((const String&)lhs + (const String&)rhs); }
    inline std::ostream&  operator << (std::ostream& out, const StringRC& s)     { out << (const kcc::String&) s; return out; }

    /** RC String types */
    typedef std::set<StringRC>              StringSetRC;
    typedef std::map<StringRC, String>      StringMapRC;
    typedef std::multiset<StringRC>         StringMultisetRC;
    typedef std::multimap<StringRC, String> StringMultimapRC;
}

#endif // StringTypes_h
