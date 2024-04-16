#include <inc/core/Core.h>
#include <inc/inet/IHTTP.h>

#define KCC_FILE    "httpd"
#define KCC_VERSION "$Id: httpd.cpp 20678 2007-09-12 15:32:56Z tvk $"

struct RESTService : kcc::IHTTPResponse
{
    void onResponse(const kcc::HTTPRequest& request, kcc::IHTTPRequestReader* in, kcc::IHTTPResponseWriter* out)
    {
        kcc::StringStream xml;
        kcc::DOMWriter w(xml);
        w.start("Request");
        w.attr("method", request.method);
        w.attr("path",   request.path);
        w.attr("ip",     request.ip);
        w.attr("port",   kcc::Strings::printf("%d", request.port));
        w.start("Attributes");
        for (
            kcc::Dictionary::const_iterator i = request.attributes.begin();
            i != request.attributes.end();
            i++)
        {
            w.start("Attribute");
            w.attr("key",   i->first);
            w.attr("value", i->second);
            w.end("Attribute");
        }
        w.end("Attributes");
        w.start("Parameters");
        for (
            kcc::Dictionary::const_iterator i = request.parameters.begin();
            i != request.parameters.end();
            i++)
        {
            w.start("Parameters");
            w.attr("key",   i->first);
            w.attr("value", i->second);
            w.end("Parameters");
        }
        w.end("Parameters");
        w.end("Request");

        // stream response
        out->xml(xml);
    }
};

struct RESTRegistrar : kcc::IHTTPResponse
{
    void onResponse(const kcc::HTTPRequest& request, kcc::IHTTPRequestReader* in, kcc::IHTTPResponseWriter* out)
    {
        std::cout << "RESTRegistrar" << std::endl << "[" << std::endl;
        for (kcc::Dictionary::const_iterator i = request.parameters.begin(); i != request.parameters.end(); i++)
            std::cout << "\t" << i->first << ": " << i->second << std::endl;
        std::cout << std::endl << "]" << std::endl;
        out->response(kcc::HTTP::C_OK);
    }
};

int main(int argc, const char* argv[])
{
    std::cout <<
        "httpd - Kuumba test rest service." << std::endl << 
        "httpd host=(host) port=(port)" << std::endl;

    // initialize kcc
    kcc::Properties props;
    props.set("kcc.logName",      KCC_FILE);
    props.set("kcc.logMax",       3L);
    props.set("kcc.logVerbosity", (long) kcc::Log::V_INFO_3);
    if (argc > 1) props.load(argc, argv, false);
    kcc::Core::init(props, KCC_VERSION);

    // run rest service
    kcc::Log::Scope scope(KCC_FILE, "main");
    try
    {
        // command line
        kcc::String host (props.get("host", "127.0.0.1"));
        int         port = kcc::Strings::parseInteger(props.get("port", "5151"));
        kcc::String http (props.get("http",    "k_httpserver"));
        bool        remoteShutdown = props.get("remoteShutdown", KCC_PROPERTY_FALSE) == KCC_PROPERTY_TRUE;
        kcc::Log::out("rest service @ %s:%d ['stop' to exit]", host.c_str(), port);
        
        // create handlers
        kcc::IHTTPServerFactory* httpFactory = KCC_FACTORY(kcc::IHTTPServerFactory, http);
        kcc::AutoPtr<kcc::IHTTPResponseDispatcher> dispatcher(httpFactory->constructDispatcher());
        kcc::AutoPtr<kcc::IHTTPResponseShutdown>   shutdown;
        kcc::AutoPtr<kcc::IHTTPResponse>           service(httpFactory->constructServiceStatus());
        RESTService                                rest;
        RESTRegistrar                              registrar;
        
        // register handlers
        kcc::IHTTPResponseDispatcher::ResponseHandlers& handlers = dispatcher->handlers();
        handlers["/"]          = &rest;
        handlers["/registrar"] = &registrar;
        handlers["/service"]   = service;
        if (remoteShutdown)
        {
            shutdown = httpFactory->constructShutdown();
            handlers["/shutdown"] = shutdown;
            std::cout << "remote shutdown: dispatch '/shutdown' to shutdown" << std::endl;
        }
        else
        {
            std::cout << "local shutdown: type 'stop' to shutdown" << std::endl;
        }
        std::cout.flush();
        
        // http server
        kcc::AutoPtr<kcc::IHTTPServer> s(httpFactory->constructServer());
        s->init(host, port, dispatcher);
        s->start();

        // run server until stopped
        if (remoteShutdown) 
        {
            shutdown->waitForShutdown();
        }
        else
        {
            kcc::String msg;
            while (msg != "stop") std::cin >> msg;
        }

        // stop server
        std::cout << "rest service stopping..." << std::endl;
        std::cout.flush();
        s->stop();
        std::cout << "rest service stopped" << std::endl;
    }
    catch (std::exception& e)
    {
        kcc::Log::exception(e);
        return 1;
    }

    return 0;
}
