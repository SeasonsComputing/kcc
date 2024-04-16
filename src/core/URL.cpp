/*
 * Kuumba C++ Core
 *
 * $Id: URL.cpp 21978 2008-02-01 22:18:27Z tvk $
 */
#include <inc/core/Core.h>

#define KCC_FILE "URL"

namespace kcc
{
    // Constants
    static const String            k_url_rx        ("^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?");
    static const String::size_type k_sz            = 1024;
    static const String            k_urlSchemeSep  ("://");
    static const String            k_urlPortSep    (":");
    static const String            k_urlQuerySep   ("?");
    static const String            k_urlFragmentSep("#");
    static const String            k_sepParam      ("&");
    static const String            k_sepValue      ("=");

    // fullPath: build full path of URL
    String URL::fullPath() const
    {
        String fp;
        fp.reserve(k_sz);
        if (!scheme.empty())        fp.append(scheme).append(k_urlSchemeSep);
        fp.append(host);
        if (port != URL::PORT_NONE) fp.append(k_urlPortSep).append(Strings::printf("%d", port));
        fp.append(path);
        if (!query.empty())         fp.append(k_urlQuerySep).append(query);
        if (!fragment.empty())      fp.append(k_urlFragmentSep).append(fragment);
        return fp;
    }

    // parameters: get parameters from url query
    void URL::parameters(const String& query, Dictionary& params, bool clear)
    {
        if (clear) params.clear();
        StringVector paramsSplit, tokens;
        Strings::tokenize(query, k_sepParam, paramsSplit);
        for (StringVector::size_type i = 0; i < paramsSplit.size(); i++)
        {
            Strings::tokenize(paramsSplit.at(i), k_sepValue, tokens);
            params(tokens[0]) = (tokens.size() == 2) ? URL::decode(tokens.at(1)) : Strings::empty();
        }
    }

    // parameters: build query params
    void URL::parameters(const Dictionary& params, String& query, bool clear)
    {
        if (clear)
        {
            query.clear();
            query.reserve(512);
        }
        for (Dictionary::const_iterator p = params.begin(); p != params.end(); p++)
        {
            if (p != params.begin()) query += k_sepParam;
            query += p->first + k_sepValue + URL::encode(p->second);
        }
    }

    // parse: parse URL into compartments
    URL URL::parse(const String& fullPath) throw (Exception)
    {
        URL url;
        StringParts match;
        Core::regex()->match(fullPath, k_url_rx, match);
        url.scheme.assign(match[1].first, match[1].second);
        String authority(match[3].first, match[3].second);
        url.path.assign(match[4].first, match[4].second);
        url.query.assign(match[6].first, match[6].second);
        url.fragment.assign(match[8].first, match[8].second);
        String::size_type ps = authority.find(k_urlPortSep);
        if (ps != String::npos)
        {
            url.host = authority.substr(0, ps);
            url.port = Strings::parseInteger(authority.substr(ps+1));
        }
        else
        {
            url.host = authority;
            url.port = URL::PORT_NONE;
        }
        return url;
    }

    // encode: encode url string
    String URL::encode(const String& in)
    {
        static const String k_valid("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789$-_.+!*'(),");
        //static const String k_unsafe("<>\"#%{}|\\^~[]`");
        //static const String k_reserved(";/?:@=&");
        String s;
        String::size_type sz = in.length();
        for (String::size_type i = 0; i < sz; i++)
        {
            Char c = in[i];
            if (k_valid.find(c) != String::npos) s += c;
            else                                 s += Strings::printf("%%%x", c);
        }
        return s;
    }

    // decode: decode url-encoded string
    String URL::decode(const String& in)
    {
        String s;
        String::size_type sz = in.length();
        for (String::size_type i = 0; i < sz; i++)
        {
            Char c = in[i];
            if (c == '%')
            {
                int parse;
                std::sscanf(in.substr(i+1, 2).c_str(), "%x", &parse);
                s += (Char) parse;
                i += 2;
            }
            else
            {
                s += c;
            }
        }
        return s;
    }
}
