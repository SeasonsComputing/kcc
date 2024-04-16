/*
 * Kuumba C++ Core
 *
 * $Id: HTTP.cpp 23211 2008-04-16 23:24:04Z tvk $
 */
#include <inc/core/Core.h>

#define KCC_FILE "HTTP"

namespace kcc
{
    // Constants
    static const String::size_type k_szPacket           = 1024*4;       //   4kb max
    static const String::size_type k_szMaxHeader        = 1024*256;     // 256kb max
    static const String::size_type k_szMaxContent       = 1024*1024*64; //  64mb max
    static const int               k_packetRetries      = 8;
    static const String            k_schemeHTTP         = "http";
    static const String            k_schemeFile         = "file";
    static const String            k_sepHeader          (" ");
    static const String            k_sepQuery           ("?");
    static const String            k_sepAttr            (": ");
    static const int               k_httpPort           = 80;
    static const String            k_httpEOL            ("\r\n");
    static const String            k_httpGet            ("GET");
    static const String            k_httpPut            ("PUT");
    static const String            k_httpPost           ("POST");
    static const String            k_httpDelete         ("DELETE");
    static const String            k_httpAccept         ("Accept");
    static const String            k_httpContentType    ("Content-Type");
    static const String            k_httpContentLength  ("Content-Length");
    static const String            k_httpContentTypeXml ("text/xml");
    static const String            k_httpContentTypeForm("application/x-www-form-urlencoded");
    static const String            k_httpLastModified   ("Last-Modified");
    static const String            k_httpCache          ("Cache-Control");
    static const String            k_httpNeverCache     ("must-revalidate,max-age=0,no-cache");
    static const String            k_httpPragma         ("Pragma");
    static const String            k_httpNoCache        ("no-cache");
    static const String            k_httpCookieSet      ("Set-Cookie");
    static const String            k_httpCookieGet      ("Cookie");
    static const String            k_httpParamSep       (";");
    static const String            k_httpParamVal       ("=");
    static const String            k_httpCookieAV       ("; Expires=%s; Path=%s");
    static const String            k_httpResponse(
        "HTTP/1.0 %d OK\r\n"
        "Date: %s\r\n"
        "Server: KCC/1.0.0\r\n"
        "Content-Length: %d\r\n");  // EOL added after headers appended
    static const String k_httpDispatch(
        " HTTP/1.0\r\n"
        "Date: %s\r\n"
        "Host: %s:%d\r\n");         // EOL added after headers appended

    //
    // HTTP implementation
    //

    // Methods
    const String& HTTP::GET()    { return k_httpGet;    }
    const String& HTTP::POST()   { return k_httpPost;   }
    const String& HTTP::DELETE() { return k_httpDelete; }
    const String& HTTP::PUT()    { return k_httpPut;    }

    // dispatch: dispatch http header
    void HTTP::dispatch(
        Socket& client,
        const URL& url, 
        const String& method, 
        const Dictionary& headers) throw (Socket::Failed)
    {
        Log::Scope scope(KCC_FILE, "dispatch");

        // build dispatch params
        int port = (url.port == URL::PORT_NONE) ? k_httpPort : url.port;
        String uri = url.path;
        if (!url.query.empty()) uri += k_sepQuery + url.query;
        String header;
        header.reserve(k_szMaxHeader);
        header += method + " " + uri;
        header += Strings::printf(k_httpDispatch.c_str(), ISODate::utc().gmtdatetime().c_str(), url.host.c_str(), port);
        for (Dictionary::const_iterator i = headers.begin(); i != headers.end(); i++) 
            header += i->first + k_sepAttr + i->second + k_httpEOL;
        header += k_httpEOL;

        // send dispatch
        client.connect(url.host, port);
        client.write(header);
    }
    
    // request: fetch http request header from client socket
    void HTTP::request(Socket& client, HTTPRequest& request) throw (Socket::Failed)
    {
        Log::Scope scope(KCC_FILE, "request");

        // client
        request.ip   = client.ip();
        request.port = client.port();
        Dictionary& attrs  = request.attributes;
        Dictionary& params = request.parameters;

        // request raw header
        int size = 0;
        StringVector headers;
        HTTP::requestHeaders(client, headers, size);

        // header prologue: method, query, uri
        StringVector prologue;
        Strings::tokenize(headers.at(0), k_sepHeader, prologue);
        if (prologue.size() < 2) throw Socket::Failed("can't parse http method header");
        request.method = prologue.at(0);
        String uri = prologue.at(1);
        String::size_type querySep = uri.find(k_sepQuery);
        String            query;
        if (querySep != String::npos) query = uri.substr(querySep + 1);
        request.path = uri.substr(0, querySep);

        // request attrs
        for (StringVector::size_type i = 1; i < headers.size(); i++)
        {
            const String& line = headers.at(i);
            String::size_type sep = line.find(k_sepAttr);
            if (sep != String::npos) attrs(line.substr(0, sep)) = line.substr(sep+2); // do not decode attr's
        }

        // form params
        if (request.method == k_httpPost && attrs[k_httpContentType] == k_httpContentTypeForm)
        {
            String form;
            HTTP::content(client, attrs, form);
            URL::parameters(form, params, false);
        }

        // query params
        URL::parameters(query, params, false);
    }

    // requestHeaders: retrieve raw http request headers
    void HTTP::requestHeaders(Socket& client, StringVector& headers, int& total) throw (Socket::Failed)
    {
        Log::Scope scope(KCC_FILE, "requestHeaders");
        headers.clear();
        String requestRaw;
        requestRaw.reserve(k_szPacket);
        total = 0;
        bool eof      = false;
        char buf[2]   = { 0, 0 };
        int  szEOF    = (int)k_httpEOL.size()*2;
        int  attempts = 0;
        int  actual   = 0;
        int  max      = (int)k_szMaxHeader;
        client.read(buf, 1, actual);
        total += actual;
        while (total < max)
        {
            requestRaw += buf;
            if (
                total >= szEOF &&
                requestRaw[total-1] == k_httpEOL[1] &&
                requestRaw[total-2] == k_httpEOL[0] &&
                requestRaw[total-3] == k_httpEOL[1] &&
                requestRaw[total-4] == k_httpEOL[0])
            {
                eof = true;
                break; // packet EOF == EOL+EOL
            }
            client.read(buf, 1, actual);
            total += actual;
            if (actual == 0) attempts++;
            if (attempts > k_packetRetries) throw Socket::Failed("http request header not received");
        }
        if (!eof) 
        {
            if (total < max) throw Socket::Failed("http header not read; socket connection broken");
            else             throw Socket::Failed("http header too large");
        }
        Strings::tokenize(requestRaw, k_httpEOL, headers);
        if (headers.size() == 0) throw Socket::Failed("http header empty");
    }

    // write: write string data to client
    void HTTP::write(Socket& client, const String& data) throw (Socket::Failed)
    {
        const Char* offset = data.c_str();
        long        total  = (long) data.size();
        long        pos    = 0L;
        while (pos < total)
        {
            long len = std::min((long)k_szPacket, total-pos);
            client.write(offset + pos, len);
            pos += len;
        }
    }
    
    // write: write stream data to client
    void HTTP::write(Socket& client, std::istream& data) throw (Socket::Failed)
    {
        char buf[k_szPacket];
        while (data.good() && !data.eof())
        {
            data.read(buf, k_szPacket);
            client.write(buf, data.gcount());
        }
    }

    // content: fetch HTTP text content from client
    void HTTP::content(Socket& client, const Dictionary& attrs, String& data) throw (Socket::Failed)
    {
        Log::Scope scope(KCC_FILE, "content");
        data.clear();
        String::size_type sz = 0;
        const String& length = attrs[k_httpContentLength];
        if (length.empty())
        {
            sz = k_szMaxContent;
            data.reserve(k_szPacket);
        }
        else
        {
            sz = Strings::parseInteger(length);
            if (sz > k_szMaxContent) throw Socket::Failed(Strings::printf("maximum content size exceeded: max=[%d] sz=[%d]", k_szMaxContent, sz));
            data.reserve(sz);
        }
        char buf[k_szPacket+1]; // +1 for null
        int actual = 0;
        String::size_type total = 0;
        do
        {
            client.read(buf, k_szPacket, actual);
            if (actual > 0)
            {
                total += actual;
                buf[actual] = 0;
                data += buf;
            }
        } while (actual > 0 && total < sz);
    }

    // response: send HTTP response header
    void HTTP::response(Socket& client, const Dictionary& headers, int len, int response) throw (Socket::Failed)
    {
        Log::Scope scope(KCC_FILE, "response");
        String header;
        header.reserve(k_szMaxHeader);
        header += Strings::printf(k_httpResponse.c_str(), response, ISODate::utc().gmtdatetime().c_str(), len);
        for (Dictionary::const_iterator i = headers.begin(); i != headers.end(); i++) 
            header += i->first + k_sepAttr + i->second + k_httpEOL;
        header += k_httpEOL;
        client.write(header);
    }

    // getxml: get XML contents using HTTP/GET
    int HTTP::getxml(const URL& url, String& receivedXml) throw (Socket::Failed)
    {
        Log::Scope scope(KCC_FILE, "getxml");
        int code = HTTP::C_ERROR;
        if (url.scheme == k_schemeFile)
        {
            code = Strings::loadText(url.path, receivedXml) ? HTTP::C_OK : HTTP::C_NOT_FOUND;
        }
        else
        {
            Dictionary headers;
            headers(k_httpAccept) = k_httpContentTypeXml;
            HTTPDispatch dispatch;
            dispatch.send(url, HTTP::GET(), headers);
            HTTPRequest request;
            code = dispatch.response(request);
            if (code == HTTP::C_OK) dispatch.content(request.attributes, receivedXml);
        }
        return code;
    }

    // putxml: put XML contents using HTTP/PUT
    int HTTP::putxml(const URL& url, const String& sendXml) throw (Socket::Failed)
    {
        Log::Scope scope(KCC_FILE, "putxml");
        Dictionary headers;
        headers(k_httpContentLength) = Strings::printf("%d", sendXml.size());
        HTTPDispatch dispatch;
        dispatch.send(url, HTTP::PUT(), headers);
        dispatch.write(sendXml);
        HTTPRequest request;
        return dispatch.response(request);
    }

    // postxml: dispatch XML contents using HTTP/POST
    int HTTP::postxml(const URL& url, const String& sendXml, String& receivedXml) throw (Socket::Failed)
    {
        Log::Scope scope(KCC_FILE, "postxml");
        Dictionary headers;
        headers(k_httpAccept)        = k_httpContentTypeXml;
        headers(k_httpContentLength) = Strings::printf("%d", sendXml.size());
        HTTPDispatch dispatch;
        dispatch.send(url, HTTP::POST(), headers);
        dispatch.write(sendXml);
        HTTPRequest request;
        int code = dispatch.response(request);
        if (code == HTTP::C_OK) dispatch.content(request.attributes, receivedXml);
        return code;
    }

    // deletexml: put XML resource contents using HTTP/DELETE
    int HTTP::deletexml(const URL& url) throw (Socket::Failed)
    {
        Log::Scope scope(KCC_FILE, "deletexml");
        HTTPDispatch dispatch;
        dispatch.send(url, HTTP::DELETE());
        HTTPRequest request;
        return dispatch.response(request);
    }

    // setHeaders: set http headers
    void HTTP::setHeaders(Dictionary& headers, const String& contentType, bool nocache)
    {
        headers(k_httpContentType) = contentType;
        if (nocache)
        {
            headers(k_httpLastModified) = ISODate::utc().gmtdatetime();
            headers(k_httpPragma)       = k_httpNoCache;
            headers(k_httpCache)        = k_httpNeverCache;
        }
    }

    // setHeadersXml: set xml headers and disable caching
    void HTTP::setHeadersXml(Dictionary& headers) { HTTP::setHeaders(headers, k_httpContentTypeXml, true); }

    // setCookies: set cookies into header
    void HTTP::setCookies(const Dictionary& cookies, Dictionary& headers, long age, const String& path)
    {
        Log::Scope scope(KCC_FILE, "setCookies");
        std::time_t t = std::time(NULL) + age;
        String expires(ISODate::utc(t).gmtdatetime());
        String av = Strings::printf(k_httpCookieAV.c_str(), expires.c_str(), path.c_str());
        for (Dictionary::const_iterator i = cookies.begin(); i != cookies.end(); i++)
            headers(k_httpCookieSet) = i->first + k_httpParamVal + URL::encode(i->second) + av;
    }

    // getCookies: get cookies from header
    void HTTP::getCookies(Dictionary& cookies, const Dictionary& headers, bool clear)
    {
        Log::Scope scope(KCC_FILE, "getCookies");
        if (clear) cookies.clear();
        Dictionary::const_iterator cookie = headers.find(k_httpCookieGet);
        if (cookie == headers.end()) return;
        StringVector attrs;
        Strings::tokenize(cookie->second, k_httpParamSep, attrs);
        if (attrs.empty()) Log::warning("error parsing cookie: %s", cookie->second.c_str());
        for (StringVector::const_iterator a = attrs.begin(); a != attrs.end(); a++)
        {
            String::size_type sep = a->find(k_httpParamVal);
            if (sep == String::npos) 
                Log::warning("error parsing cookie value: %s", a->c_str());
            else
                cookies(a->substr(0, sep)) = URL::decode(a->substr(sep+1));
        }
    }
    
    // parseAttributeValue: parse attribute value params
    bool HTTP::parseAttributeValue(const String& fullValue, String& value, Dictionary& params)
    {
        StringVector split;
        Strings::tokenize(fullValue, k_httpParamSep, split);
        if (split.size() > 0) value = split[0];
        if (split.size() < 2) return true; // empty is valid value
        for (StringVector::size_type j = 1; j < split.size(); j++)
        {
            String::size_type sep = split[j].find(k_httpParamVal);
            if (sep == String::npos) return false;
            String pv = Strings::trimws(split[j].substr(sep+1));
            if (pv.size() > 2 && pv[0] == '\"' && pv[pv.size()-1] == '\"') pv = pv.substr(1, pv.size()-2); // trim quotes
            params(Strings::trimws(split[j].substr(0, sep))) = pv;
        }
        return true;
    }
}
