/*
 * Kuumba C++ Core
 *
 * $Id: HTTP.h 22688 2008-03-16 17:43:07Z tvk $
 */
#ifndef HTTP_h
#define HTTP_h

namespace kcc
{
    /** 
     * HTTP request helper 
     *
     * @author Ted V. Kremer
     */
    struct HTTPRequest
    {
        String     method;
        String     path;
        String     ip;
        int        port;
        Dictionary attributes;
        Dictionary parameters;
        HTTPRequest() : port(URL::PORT_NONE) {}
    };

    /**
     * HTTP Utilities
     * @see HTTPDispatch for a simplified HTTP object request/response helper
     *
     * @author Ted V. Kremer
     */
    class KCC_CORE_EXPORT HTTP
    {
    public:
        /** HTTP Response Codes */
        enum Code 
        { 
            C_OK           = 200, 
            C_NOT_MODIFIED = 304, 
            C_FORBIDDEN    = 403, 
            C_NOT_FOUND    = 404, 
            C_ERROR        = 500
        };
        
        /* HTTP Methods */
        static const String& GET();
        static const String& POST();
        static const String& DELETE();
        static const String& PUT();
        
        /**
         * Dispatch HTTP header
         * @param client connected & unread socket to fetch request from
         * @param url disparch url
         * @param method HTTP method
         * @param headers additional HTTP connection header
         * @throws Socket::Failed exception if error
         */
        static void dispatch(
            Socket& client, const URL& url, const String& method, 
            const Dictionary& headers = Dictionary::empty()) throw (Socket::Failed);

        /** 
         * Retrieve HTTP request header
         * @param client connected & unread socket to fetch request from
         * @param request output param where request header is places
         * @throws Socket::Failed exception if error
         */
        static void request(Socket& client, HTTPRequest& request) throw (Socket::Failed);

        /** 
         * Read request headers. Read a line delimted by EOL until EOL+EOL reached.
         * @param client connected & unread socket to fetch request from
         * @param headers header rows read
         * @throws Socket::Failed exception if error
         */
        static void requestHeaders(Socket& client, StringVector& headers, int& size) throw (Socket::Failed);
            
        /**
         * Send data to HTTP
         * @param data data to send
         * @throws Socket::Failed exception if error
         */
        static void write(Socket& client, const String& data) throw (Socket::Failed);
        static void write(Socket& client, std::istream& data) throw (Socket::Failed);

        /** 
         * Retrieve HTTP content (content type MUST be of text type, caller MUST validate PRIOR to calling)
         * @param client connected & unread socket to fetch request from
         * @param attributes request attributes
         * @param data output param to place text content
         * @throws Socket::Failed exception if error
         */
        static void content(Socket& client, const Dictionary& attributes, String& data) throw (Socket::Failed);

        /**
         * Write HTTP response header
         * @param client connected socket to write to
         * @param headers response headers
         * @param len response total length
         * @param response code
         * @throws Socket::Failed exception if error
         */
        static void response(Socket& client, const Dictionary& headers, int len, int response = 200) throw (Socket::Failed);

        /** 
         * Helper wrappers for GET/PUT/POST/DELETE of xml data
         * @param url url of xml host
         * @param sendXml xml to send
         * @param receivedXml xml received
         * @return HTTP response code
         * @throws Socket::Failed exception if error
         */
        static int getxml   (const URL& url, String& receivedXml) throw (Socket::Failed);
        static int putxml   (const URL& url, const String& sendXml) throw (Socket::Failed);
        static int postxml  (const URL& url, const String& sendXml, String& receivedXml) throw (Socket::Failed);
        static int deletexml(const URL& url) throw (Socket::Failed);

        /**
         * Set HTTP headers
         * @param headers headers to set directives
         * @param content HTTP content type, e.g. "text/xml", "text/html"
         * @param nocache set no-cache headers
         */
        static void setHeaders(Dictionary& headers, const String& contentType, bool nocache);
        
        /**
         * Set xml headers and disable caching
         * @param headers headers to set directives
         */
        static void setHeadersXml(Dictionary& headers);

        /**
         * Encode and set cookies into HTTP header
         * @param cookies data to set in headers
         * @param headers response headers to set cookies into
         * @param age cookie age in seconds
         * @param path path for cookie
         */
        static void setCookies(const Dictionary& cookies, Dictionary& headers, long age = 0L, const String& path = Platform::fsSep());

        /**
         * Get and decode cookies from HTTP attributes
         * @param cookies data to get from headers (OUT param)
         * @param attributes request attributes to get cookies from
         * @param clear clear cookies collection prior to populating
         */
        static void getCookies(Dictionary& cookies, const Dictionary& attributes, bool clear = true);

        /**
         * Parse attribute value with parameters (e.g. Key: value; param-key=param-value; param-key=param-value)
         * @param fullValue full value from attribute
         * @param value actual value (out-param)
         * @param params attribute value params
         * @return true if parsed successfully
         */        
        static bool parseAttributeValue(const String& fullValue, String& value, Dictionary& params);

    private:
        HTTP();
    };

    /** 
     * HTTP dispatch helper 
     *
     * @author Ted V. Kremer
     */
    class KCC_CORE_EXPORT HTTPDispatch
    {
    public:
        /**
         * Construct dispatch
         */
        HTTPDispatch() {}
        
        /**
         * Send dispatch to HTTP
         * @param url disparch url
         * @param method HTTP method
         * @param headers additional HTTP connection header
         * @throws Socket::Failed exception if error
         */
        inline void send(
            const URL& url, const String& method, 
            const Dictionary& headers = Dictionary::empty()) throw (Socket::Failed)
        {
            HTTP::dispatch(m_client, url, method, headers);
        }
            
        /**
         * Send data to HTTP dispatch
         * @param data data to send
         * @throws Socket::Failed exception if error
         */
        inline void write(const String& data) throw (Socket::Failed) { HTTP::write(m_client, data); }
        inline void write(std::istream& data) throw (Socket::Failed) { HTTP::write(m_client, data); }

        /**
         * Get response from HTTP dispatch
         * @param request response attributes
         * @return HTTP response code
         * @throws Socket::Failed exception if error
         */            
        inline int response(HTTPRequest& request) throw (Socket::Failed)
        {
            HTTP::request(m_client, request);
            return Strings::parseInteger(request.path);
        }
        
        /**
         * Retrieve HTTP content from client
         * @param attributes request attributes
         * @param data output param to place text content
         * @throws Socket::Failed exception if error
         */
        inline void content(const Dictionary& attributes, String& receivedData) throw (Socket::Failed)
        {
            HTTP::content(m_client, attributes, receivedData); 
        }

    private:
        HTTPDispatch(const HTTPDispatch&);
        HTTPDispatch& operator = (const HTTPDispatch&);

        // Attributes
        Socket m_client;
    };
}

#endif // HTTP_h
