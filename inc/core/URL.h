/*
 * Kuumba C++ Core
 *
 * $Id: URL.h 22625 2008-03-09 22:51:49Z tvk $
 */
#ifndef URL_h
#define URL_h

namespace kcc
{
    /**
     * URL ADT
     *
     * @author Ted V. Kremer
     */
    class KCC_CORE_EXPORT URL
    {
    public:
        /** No port specified in URL */
        enum URLPort { PORT_NONE = -1 };

        /** Attributes */
        URL() : port(URL::PORT_NONE) {}
        URL(const String& url) { *this = URL::parse(url); }
        URL(
            const String& _scheme,
            const String& _host,
            int           _port,
            const String& _path,
            const String& _query,
            const String& _fragment) :
            scheme(_scheme), host(_host), path(_path), query(_query), fragment(_fragment), port(_port)
        {}
        String scheme, host, path, query, fragment;
        int    port;

        /** 
         * Accessors 
         * @return full path of URL compartments
         */
        String fullPath() const;

        /** Utility */
        inline int compare(const URL& rhs) const { return fullPath().compare(rhs.fullPath()); }

        /** Factory */
        static URL parse(const String& fullPath) throw (Exception);

        /**
         * Get/set query parameters
         * @param query query params to parse
         * @param params where to place parsed params
         * @param clear clear params
         */
        static void parameters(const String& query, Dictionary& params, bool clear = true);
        static void parameters(const Dictionary& params, String& query, bool clear = true);

        /**
         * Encode/Decode an URL  encoded string
         * @param in string to decode
         * @return decoded string
         */
        static String encode(const String& in);
        static String decode(const String& in);
    };
    inline bool operator <  (const URL& lhs, const URL& rhs) { return lhs.compare(rhs) <  0; }
    inline bool operator == (const URL& lhs, const URL& rhs) { return lhs.compare(rhs) == 0; }
    inline std::ostream& operator << (std::ostream& out, const URL& url) { out << url.fullPath(); return out; }
}

#endif // URL_h
