/*
 * Kuumba C++ Core
 *
 * $Id: IHTTP.h 22761 2008-03-24 15:51:26Z tvk $
 */
#ifndef IHTTP_h
#define IHTTP_h

namespace kcc
{
    /**
     * HTTP request reader
     * 
     * @author Ted V. Kremer
     */
    interface IHTTPRequestReader : IComponent
    {
        /**
         * Read request headers. Read a line delimted by EOL until EOL+EOL reached.
         * @param headers header rows read
         * @param size size of full header (# of bytes read)
         * @throws Socket::Failed exception if error
         */
        virtual void request(StringVector& headers, int& size) throw (Socket::Failed) = 0; 

        /**
         * Read text content from http (content type MUST be text, consumer MUST validate PRIOR to calling)
         * @param attributes request attributes
         * @param data output param to place text content
         * @throws Socket::Failed exception if error
         */
        virtual void content(const Dictionary& attributes, String& data) throw (Socket::Failed) = 0; 

        /**
         * Read raw data from http
         * @param buf buffer to read into
         * @param sz size of buffer
         * @param actual actual bytes read
         * @throws Socket::Failed exception if error
         */
        virtual void read(char* buf, int sz, int& actual) throw (Socket::Failed) = 0;
    };
     
    /**
     * HTTP response writer
     * 
     * @author Ted V. Kremer
     */
    interface IHTTPResponseWriter : IComponent
    {
        /**
         * Write complete response (headers & content)
         * @param in response to write
         * @param headers response headers
         * @param response code
         * @throws Socket::Failed exception if error
         */
        virtual void response(std::istream& in, const Dictionary& headers, int response = HTTP::C_OK) throw (Socket::Failed) = 0;

        /**
         * Write response code only response
         * @param response code
         * @throws Socket::Failed exception if error
         */
        virtual void response(int response) throw (Socket::Failed) = 0;

        /**
         * Write response headers
         * @param headers response headers
         * @param len response total length
         * @param response code
         * @throws Socket::Failed exception if error
         */
        virtual void response(const Dictionary& headers, int len, int response = HTTP::C_OK) throw (Socket::Failed) = 0;

        /**
         * Writer response
         * @param buf buffer to writer
         * @param sz size of buffer
         * @throws Socket::Failed exception if error
         */
        virtual void write(const char* buf, int sz) throw (Socket::Failed) = 0;

        /**
         * Write xml (client-caching disabled)
         * @param in xml to write
         * @throws Socket::Failed exception if error
         */
        virtual void xml(std::istream& in) throw (Socket::Failed) = 0;
    };

    /**
     * HTTP response specification
     *
     * @author Ted V. Kremer
     */
    interface IHTTPResponse : IComponent
    {
        /**
         * Callback for an HTTP request
         * @param request request attributes
         * @param in request reader
         * @param out response writer
         */
        virtual void onResponse(const HTTPRequest& request, IHTTPRequestReader* in, IHTTPResponseWriter* out) = 0;
    };
    
    /**
     * HTTP shutdown handler
     * e.g. 
     *   kcc::String host = ...
     *   int port = ...
     *   kcc::IHTTPServerFactory* httpFactory = ...
     *
     *   // create handlers
     *   kcc::AutoPtr<kcc::IHTTPResponseDispatcher> dispatcher(httpFactory->constructDispatcher());
     *   kcc::AutoPtr<kcc::IHTTPResponseShutdown>   shutdown(httpFactory->constructShutdown());
     *   ...
     *   
     *   // register handlers
     *   kcc::IHTTPResponseDispatcher::ResponseHandlers& handlers = d->handlers();
     *   ...
     *   handlers["/shutdown"] = shutdown;
     *   
     *   // http server
     *   kcc::AutoPtr<kcc::IHTTPServer> s(httpFactory->constructServer());
     *   s->init(host, port, dispatcher);
     *   s->start();
     *   shutdown->waitForShutdown();
     *   s->stop();
     *
     * @author Ted V. Kremer
     */
    interface IHTTPResponseShutdown : IHTTPResponse
    {
        /**
         * Wait for shutdown. Blocking call.
         */
        virtual void waitForShutdown() = 0;
    };

    /**
     * HTTP response dispatcher. HTTP Response that delegates path to handler.
     * e.g. 
     *     "/" -> &handlerHome, "/service" -> &handlerService
     *
     * Ownership of handlers is NOT consumed.
     *
     * @author Ted V. Kremer
     */
    interface IHTTPResponseDispatcher : IHTTPResponse
    {
        /** Map from path to response */
        typedef std::map<String, IHTTPResponse*> ResponseHandlers;

        /**
         * Accessor to handlers map
         */        
        virtual ResponseHandlers& handlers() = 0;
    };

    /**
     * HTTP server specification
     *
     * @author Ted V. Kremer
     */
    interface IHTTPServer : IComponent
    {
        /**
         * Initialize server paramters
         * @param host host name for server
         * @param port port to listen for HTTP request on
         * @param response reponse provider to process request's response
         * @param notify URL. Will send an HTTP GET to {URL}?action=(startup|shutdown)&host={host}&port={port}
         * @param queue queue size of incoming socket listeners
         * @param sendWaitSec timeout for send (secs)
         * @param recvWaitSec timeout for receive (secs)
         * @return true if initialized successfully
         */
        virtual bool init(
            const String& host, int port, IHTTPResponse* response, 
            const String& notifyURL = Strings::empty(),
            int queue = 32, long sendWaitSec = 0, long recvWaitSec = 0) = 0;
        
        /**
         * Accessors
         */
        virtual const String& ip  () = 0;
        virtual int           port() = 0;

        /** 
         * Start server 
         * @throws Socket::Failed exception if error
         */
        virtual void start() throw (Socket::Failed) = 0;

        /** 
         * Stop server 
         * @throws Socket::Failed exception if error
         */
        virtual void stop() throw (Socket::Failed) = 0;
    };
    
    /**
     * HTTP server factory specification
     *
     * @author Ted V. Kremer
     */
    interface IHTTPServerFactory : IComponentFactory
    {
        /**
         * Construct HTTP server (ownership IS consumed)
         */
        virtual IHTTPServer* constructServer() = 0;
    
        /**
         * Construct HTTP response dispatcher (ownership IS consumed)
         */
        virtual IHTTPResponseDispatcher* constructDispatcher() = 0;
        
        /**
         * Construct response shutdown
         * @param service service name (optional)
         * @return handler (ownership IS consumed)
         */
        virtual IHTTPResponseShutdown* constructShutdown(const String& service = Strings::empty()) = 0;
        
        /**
         * Construct service process-status
         * @return handler (ownership IS consumed)
         */
        virtual IHTTPResponse* constructServiceStatus() = 0;
    };
}

#endif // IHTTP_h
