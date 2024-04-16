/*
 * Kuumba C++ Core
 *
 * $Id: HTTPServer.cpp 22625 2008-03-09 22:51:49Z tvk $
 */
#include <inc/core/Core.h>
#include <inc/inet/IHTTP.h>

#define KCC_FILE    "HTTPServer"
#define KCC_VERSION "$Id: HTTPServer.cpp 22625 2008-03-09 22:51:49Z tvk $"

namespace kcc
{
    // Constants
    static const String k_httpHost      ("Host");
    static const String k_notifyStartup ("startup");
    static const String k_notifyShutdown("shutdown");
    static const String k_notifyAction  ("action");
    static const String k_notifyService ("service");
    static const String k_notifyHost    ("host");
    static const String k_notifyPort    ("port");
    static const String k_notifyWhen    ("when");
    static const String k_notifySep     (",");
    
    // Helper class to parse an incoming request and delegate to a response handler
    struct HTTPHandler : Thread, IHTTPRequestReader, IHTTPResponseWriter
    {
        // Attributes
        Socket         m_client;
        IHTTPResponse* m_reponse;
        HTTPHandler(Socket::Handle h, IHTTPResponse* r, long send, long recv) :
            Thread("HTTPHandler"), m_client(h), m_reponse(r)
        {
            Log::Scope scope(KCC_FILE, "HTTPHandler::HTTPHandler");
            m_client.setTimeout(Socket::T_SEND,    send, 0);
            m_client.setTimeout(Socket::T_RECEIVE, recv, 0);
        }

        // invoke: parse request and delegate to response
        void invoke()
        {
            Log::Scope scope(KCC_FILE, "HTTPHandler::invoke");
            try
            {
                // log request start
                String addr(Strings::printf("%s:%d", m_client.ip().c_str(), m_client.port()));
                if (Log::verbosity() >= Log::V_INFO_4)
                    Log::info4("request started: client=[%s]", addr.c_str());
                
                // dispatch request to handler
                Timer t;
                t.start();
                HTTPRequest request;
                HTTP::request(m_client, request);
                m_reponse->onResponse(request, this, this);
                m_client.close();
                t.stop();
                
                // log request completion
                if (Log::verbosity() >= Log::V_INFO_4)
                {
                    static const String::size_type k_sz = 50;
                    String uri;
                    uri.reserve(1024);
                    uri += request.path;
                    Dictionary::const_iterator begin = request.parameters.begin();
                    for (Dictionary::const_iterator p = begin; p != request.parameters.end(); p++)
                    {
                        if (p != begin) uri += "&";
                        else            uri += "?";
                        if (p->second.size() > k_sz) uri += p->first + "=" + p->second.substr(0, k_sz) + "...";
                        else if (p->second.empty())  uri += p->first;
                        else                         uri += p->first + "=" + p->second;
                        
                    }
                    Log::info4(
                        "request completed: client=[%s] method=[%s] time=[%.3f] uri=[%s]", 
                        addr.c_str(), request.method.c_str(), t.secs(), uri.c_str());
                }
            }
            catch (Exception& e)
            {
                Log::exception(e);
                m_client.close();
            }
        }

        // Implementation
        void content (const Dictionary& attrs, String& data)     throw (Socket::Failed) { HTTP::content(m_client, attrs, data); }
        void request (StringVector& headers, int& size)          throw (Socket::Failed) { HTTP::requestHeaders(m_client, headers, size); }
        void read    (char* buf, int sz, int& actual)            throw (Socket::Failed) { m_client.read(buf, sz, actual); }
        void write   (const char* buf, int sz)                   throw (Socket::Failed) { m_client.write(buf, sz); }
        void response(const Dictionary& hdrs, int len, int resp) throw (Socket::Failed) { HTTP::response(m_client, hdrs, len, resp); }
        void response(int resp)                                  throw (Socket::Failed) { HTTP::response(m_client, Dictionary::empty(), 0, resp); }
        void response(std::istream& in, const Dictionary& hdrs, int resp) throw (Socket::Failed)
        {
            Log::Scope scope(KCC_FILE, "HTTPHandler::response");

            // stream size (required for well-formed HTTP header, Content-Length, attribute)
            in.seekg(0, std::ios::end);
            int sz = (int)in.tellg();
            in.seekg(0, std::ios::beg);
            
            // stream header and data
            response(hdrs, sz, resp);
            HTTP::write(m_client, in);
        }
        void xml(std::istream& in) throw (Socket::Failed)
        {
            Log::Scope scope(KCC_FILE, "HTTPHandler::xml");
            Dictionary headers;
            HTTP::setHeadersXml(headers);
            response(in, headers, HTTP::C_OK);
        }
    };

    // Helper class to listen for incoming HTTP requests
    struct HTTPListener : Thread
    {
        // Attributes
        SocketServer   m_server;
        IHTTPResponse* m_response;
        long           m_send;
        long           m_recv;
        HTTPListener(const String& host, int port, IHTTPResponse* r, int queue, long send, long recv) :
            Thread("HTTPListener"),
            m_server(host, port, queue),
            m_response(r), m_send(send), m_recv(recv)
        {}

        // invoke: delegate requests to request handler
        void invoke()
        {
            Log::Scope scope(KCC_FILE, "HTTPListener::invoke");
            try
            {
                while (true)
                {
                    Socket::Handle h = m_server.accept();
                    (new HTTPHandler(h, m_response, m_send, m_recv))->go();
                }
            }
            catch (SocketServer::Closed&)
            {
                // server closed normally
            }
            catch (std::exception& e)
            {
                Log::exception(e);
                m_server.close();
            }
        }
        
        // Accessors
        const String& ip()   { return m_server.ip();   }
        const String& host() { return m_server.host(); }
        int           port() { return m_server.port(); }

        // start: start http server
        void start() throw (Socket::Failed)
        { 
            Log::Scope scope(KCC_FILE, "HTTPListener::start");
            m_server.listen();
            go(); 
        }

        // stop: stop the server by closing the connection to force blocking accept() call 
        //       to fail and throw an exception which will cause a break out of invoke() loop
        void stop() 
        { 
            Log::Scope scope(KCC_FILE, "HTTPListener::stop");
            m_server.close(); 
            Thread::sleep(1000L); // wait for socket to close
        }
    };
    
    // Component provider for shutdown handler
    struct ResponseShutdown : IHTTPResponseShutdown
    {
        // Attributes
        Monitor m_monitor;
        String  m_service;
        ResponseShutdown(const String& service) : m_service(service) {}

        // waitForShutdown: init and wait for shutdown command        
        void waitForShutdown() 
        { 
            Log::Scope scope(KCC_FILE, "HTTPListener::waitForShutdown");
            m_monitor.init();
            m_monitor.wait(); 
        }
        
        // onResponse: shutdown
        void onResponse(const HTTPRequest& request, IHTTPRequestReader* in, IHTTPResponseWriter* out)
        {
            Log::Scope scope(KCC_FILE, "HTTPListener::onResponse");

            // stream response
            StringStream buf;
            DOMWriter w(buf);
            w.start(KCC_FILE);
            w.attr(k_notifyService,  m_service);
            w.attr(k_notifyShutdown, request.attributes[k_httpHost]);
            w.attr(k_notifyWhen,     ISODate::local().isodatetime());
            w.end(KCC_FILE);
            out->xml(buf);
            
            // notify of shutdown
            m_monitor.notify();
        }        
    };
    
    // Component provider for service process-status
    struct ResponseServiceStatus : IHTTPResponse
    {
        // onResponse: service handler
        void onResponse(const HTTPRequest& request, IHTTPRequestReader* in, IHTTPResponseWriter* out)
        {
            Log::Scope scope(KCC_FILE, "ResponseServiceStatus::onResponse");
            static const String k_restProperties("properties");
            static const String k_restVerbosity ("verbosity");
            static const String k_restLog       ("log");
            static const String k_restLogHistory("logHistory");
            static const String k_restResources ("resources");
            static const String k_restVersion   ("version");
            static const String k_paramName     ("name");
            static const String k_paramRow      ("row");
            static const String k_paramMax      ("max");
            if (request.parameters.exists(k_restProperties))
            {
                // service global properties
                StringStream properties;
                Core::properties().toXML(properties);
                out->xml(properties);
            }
            else if (request.parameters.exists(k_restVerbosity))
            {
                // service verbosity change
                long start = Log::entries(); // display verbosity change log entries
                Log::verbosity(Log::verbosityEnum(request.parameters[k_restVerbosity]));
                log(out, start, 10L);
            }
            else if (request.parameters.exists(k_restLog))
            {
                // service log query
                long row = request.parameters.exists(k_paramRow) ? Strings::parseInteger(request.parameters[k_paramRow]) : 0L;
                long max = request.parameters.exists(k_paramMax) ? Strings::parseInteger(request.parameters[k_paramMax]) : 64L;
                log(out, row, max);
            }
            else if (request.parameters.exists(k_restLogHistory) && request.parameters.exists(k_paramName))
            {
                // service historical log (complete, no paging)
                std::ifstream in(request.parameters[k_paramName].c_str());
                if (!in.good())
                {
                    out->response(HTTP::C_NOT_FOUND);
                    return;
                }
                out->xml(in);
            }
            else if (request.parameters.exists(k_restLogHistory))
            {
                // service historical log list
                static const String k_xmlLogs     ("Logs");
                static const String k_xmlEntry    ("Entry");
                static const String k_xmlEntryName("name");
                StringSet logs;
                Log::history(logs);
                StringStream buf;
                DOMWriter w(buf);
                w.start(k_xmlLogs);
                w.attr(k_notifyService, request.attributes[k_httpHost]);
                w.attr(k_notifyWhen,    ISODate::local().isodatetime());
                for (StringSet::iterator i = logs.begin(); i != logs.end(); i++) 
                {
                    w.start(k_xmlEntry);
                    w.attr(k_xmlEntryName, *i);
                    w.end(k_xmlEntry);
                }
                w.end(k_xmlLogs);
                out->xml(buf);
            }
            else if (request.parameters.exists(k_restResources))
            {
                // service resources in use
                static const String k_xmlResources        ("Resources");
                static const String k_xmlResourcesMemInUse("memInUseKB");
                StringStream buf;
                DOMWriter w(buf);
                w.start(k_xmlResources);
                w.attr(k_notifyService,        request.attributes[k_httpHost]);
                w.attr(k_notifyWhen,           ISODate::local());
                w.attr(k_xmlResourcesMemInUse, (long)Platform::procMemInUseKB());
                w.end(k_xmlResources);
                out->xml(buf);
            }
            else if (request.parameters.exists(k_restVersion))
            {
                // service & module versions
                static const String k_xmlVersion      ("Version");
                static const String k_xmlVersionName  ("appName");
                static const String k_xmlVersionSCM   ("appSCM");
                static const String k_xmlModule       ("Module");
                static const String k_xmlModuleID     ("id");
                static const String k_xmlModuleSCM    ("scm");
                static const String k_xmlModuleVersion("version");
                static const String k_keyAppName(KCC_APPLICATION_NAME);
                static const String k_keyAppSCM (KCC_APPLICATION_SCM);
                StringStream buf;
                DOMWriter w(buf);
                w.start(k_xmlVersion);
                w.attr(k_notifyService,  request.attributes[k_httpHost]);
                w.attr(k_notifyWhen,     ISODate::local().isodatetime());
                w.attr(k_xmlVersionName, Core::properties().get(k_keyAppName, kcc::Strings::empty()));
                w.attr(k_xmlVersionSCM,  Core::properties().get(k_keyAppSCM, kcc::Strings::empty()));
                StringVector ids;
                Components::moduleIds(ids);
                for (StringVector::iterator i = ids.begin(); i != ids.end(); i++)
                {
                    const ComponentModule& m = Components::module(*i);
                    const ComponentModule::ComponentDefinition& d = m.definition();
                    w.start(k_xmlModule);
                    w.attr(k_xmlModuleID,      d.id);
                    w.attr(k_xmlModuleSCM,     d.metadata.get(KCC_COMPONENT_FACTORY_SCM, Strings::empty()));
                    w.attr(k_xmlModuleVersion, d.metadata.get(KCC_COMPONENT_FACTORY_VERSION, Strings::empty()));
                    w.end(k_xmlModule);
                }
                w.end(k_xmlVersion);
                out->xml(buf);
            }
            else if (request.parameters.size() == 0)
            {
                // service home
                static const String k_xmlService("Service");
                StringStream buf;
                DOMWriter w(buf);
                w.start(k_xmlService);
                w.attr(k_notifyService, request.attributes[k_httpHost]);
                w.attr(k_notifyWhen,    ISODate::local().isodatetime());
                w.end(k_xmlService);
                out->xml(buf);
            }
            else
            {
                out->response(HTTP::C_NOT_FOUND);
            }
        }
        
        // log: service log paging
        void log(IHTTPResponseWriter* out, long row, long max)
        {
            Log::Scope scope(KCC_FILE, "ResponseServiceStatus::log");
            
            // open log for non-exclusive reading
            String log   = Log::file();
            long   eof   = Log::entries();
            long   where = 0L;
            StringStream xml;
            String line;
            std::ifstream in(log.c_str());
            if (!in.good() || eof == 0L)
            {
                Log::error("failed reading log file");
                out->response(HTTP::C_ERROR);
                return;
            }
            
            // header
            xml << 
                "<?xml version='1.0' encoding='UTF-8' ?>\n" <<
                "<Log" <<
                " file='" << Log::file() << "'" <<
                " verbosity='" << Log::verbosityName(Log::verbosity()) << "'" <<
                " entries='" << eof << "'" <<
                ">\n";

            // skip xml prolog and header
            std::getline(in, line);
            std::getline(in, line);

            // read to row
            while (where < row && where < eof && !in.eof())
            {
                do
                {
                    std::getline(in, line);
                } while (line.find("]]></Entry>") == String::npos && !in.eof());
                where++;
            }
            
            // read max
            while (where < eof && max > 0 && !in.eof())
            {
                do
                {
                    std::getline(in, line);
                    xml << line;
                } while (line.find("]]></Entry>") == String::npos && !in.eof());
                where++;
                max--;
            }
            
            // footer
            xml << "</Log>";
            
            // stream response
            out->xml(xml);
        }
    };
    
    // Component provider for reponse dispatcher
    struct ResponseDispatcher : IHTTPResponseDispatcher
    {
        // Attributes
        ResponseHandlers m_handlers;
        ResponseHandlers& handlers() { return m_handlers; }
        
        // onResponse: delegate to handler
        void onResponse(const HTTPRequest& request, IHTTPRequestReader* in, IHTTPResponseWriter* out)
        {
            Log::Scope scope(KCC_FILE, "ResponseDispatcher::onResponse");
            ResponseHandlers::iterator handler = m_handlers.find(request.path);
            if (handler != m_handlers.end()) handler->second->onResponse(request, in, out);
            else                             out->response(HTTP::C_NOT_FOUND);
        }        
    };

    // Component provider to intiate server and response threads
    struct HTTPServer : IHTTPServer
    {
        // Attributes
        HTTPListener*  m_listener;
        IHTTPResponse* m_response;
        String         m_host;
        int            m_port;
        String         m_notifyURL;
        int            m_queue;
        long           m_send;
        long           m_recv;
        Mutex          m_sentinel;
        HTTPServer() : m_listener(NULL) 
        {}

        // init: intialize server properties
        bool init(const String& host, int port, IHTTPResponse* response, const String& notifyURL, int queue, long sendWaitSec, long recvWaitSec)
        {
            Log::Scope scope(KCC_FILE, "init");
            m_listener  = NULL;
            m_host      = host;
            m_port      = port;
            m_response  = response;
            m_notifyURL = notifyURL;
            m_queue     = queue;
            m_send      = sendWaitSec;
            m_recv      = recvWaitSec;
            Log::info2(
                "HTTPServer initialized: service=[%s:%d] notify=[%s] queue=[%d] sendWait=[%d] recvWait=[%d]", 
                m_host.c_str(), m_port, m_notifyURL.c_str(), m_queue, m_send, m_recv);
            return true;
        }
        
        // Accessors
        const String& ip()   { return m_listener == NULL ? m_host : m_listener->ip(); }
        int           port() { return m_listener == NULL ? m_port : m_listener->port(); }

        // start: start or restart server
        void start() throw (Socket::Failed)
        {
            Log::Scope scope(KCC_FILE, "start");
            KCC_ASSERT(NULL == m_listener, KCC_FILE, "start", "HTTPServer already running");
            Mutex::Lock lock(m_sentinel);
            
            // start server listener
            m_listener = new HTTPListener(m_host, m_port, m_response, m_queue, m_send, m_recv);
            try
            {
                m_listener->start();
            }
            catch (Socket::Failed&)
            {
                delete m_listener;
                m_listener = NULL;
                throw;
            }
            
            // notify of startup
            try
            {
                notify(k_notifyStartup);
            }
            catch (Socket::Failed&)
            {
                m_listener->stop();
                m_listener = NULL;
                throw;
            }
        }

        // stop: stop server
        void stop() throw (Socket::Failed)
        {
            Log::Scope scope(KCC_FILE, "stop");
            Mutex::Lock lock(m_sentinel);
            if (m_listener == NULL) return;

            // stop server listener                
            m_listener->stop();
            m_listener = NULL;
            
            // notify of shutdown
            notify(k_notifyShutdown);
       }
       
       // notify: notify of action
       void notify(const String& action) throw (Socket::Failed)
       {
            if (m_notifyURL.empty()) return;
            StringVector urls;
            Strings::tokenize(m_notifyURL, k_notifySep, urls);
            for (StringVector::iterator i = urls.begin(); i != urls.end(); i++)
            {
                URL url(*i);
                Dictionary params;
                URL::parameters(url.query, params);
                params(k_notifyAction) = action;
                params(k_notifyHost)   = ip();
                params(k_notifyPort)   = Strings::printf("%d", port());
                params(k_notifyWhen)   = ISODate::local().isodatetime();
                URL::parameters(params, url.query);
                HTTPDispatch dispatch;
                dispatch.send(url, HTTP::GET());
            }
       }
    };
    
    //
    // HTTPServer factory
    //
    
    struct HTTPServerFactory : IHTTPServerFactory
    {
        IComponent*              construct()                        { return constructServer(); }
        IHTTPServer*             constructServer()                  { return new HTTPServer(); }
        IHTTPResponseDispatcher* constructDispatcher()              { return new ResponseDispatcher(); }
        IHTTPResponseShutdown*   constructShutdown(const String& s) { return new ResponseShutdown(s); }
        IHTTPResponse*           constructServiceStatus()           { return new ResponseServiceStatus(); }
    };

    KCC_COMPONENT_FACTORY_CUST(HTTPServerFactory)
    
    KCC_COMPONENT_FACTORY_METADATA_BEGIN_CUSTOM(IHTTPServerFactory, HTTPServerFactory, HTTPServer, IHTTPServer)
        KCC_COMPONENT_FACTORY_METADATA_PROPERTY(KCC_COMPONENT_FACTORY_SCM, KCC_VERSION)
    KCC_COMPONENT_FACTORY_METADATA_END
}
