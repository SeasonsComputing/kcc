#include <inc/core/Core.h>
#include <inc/inet/IPageResponse.h>

#define KCC_FILE    "page"
#define KCC_VERSION "$Id: page.cpp 15199 2007-03-09 17:57:17Z tvk $"

struct PageHandler : kcc::IPageHandler
{
    void onHandle(
        const kcc::HTTPRequest& request, 
        kcc::IPageReader* in, 
        kcc::IPageWriter* out, 
        kcc::IPageResponse* response)
    {
        kcc::Log::Scope scope(KCC_FILE, "onHandle");

        kcc::StringStream xml;
        kcc::DOMWriter w(xml);
        w.start("Page");

        // get cookies
        kcc::Dictionary cookies;
        kcc::HTTP::getCookies(cookies, request.attributes);
        w.start("CookiesGet");
        for (
            kcc::Dictionary::const_iterator i = cookies.begin();
            i != cookies.end();
            i++)
        {
            w.start("Cookie");
            w.attr("key",   i->first);
            w.attr("value", i->second);
            w.end("Cookie");
        }
        w.end("CookiesGet");

        // set cookies
        cookies.clear();
        w.start("CookiesSet");
        for (
            kcc::Dictionary::const_iterator i = request.parameters.begin();
            i != request.parameters.end();
            i++)
        {
            w.start("Cookie");
            w.attr("key",   i->first);
            w.attr("value", i->second);
            w.end("Cookie");
            cookies(i->first) = i->second;
        }

        w.end("CookiesSet");
        w.end("Page");

        kcc::StringMap xparams;
        xparams["testParam"] = "success: xsl value received";
        kcc::Dictionary headers;
        kcc::HTTP::setCookies(cookies, headers, 15);
        out->transform(xml, "page", xparams, kcc::IPageWriter::TT_HTML, headers);
    }
};

int main(int argc, const char* argv[])
{
    std::cout <<
        "page - Kuumba test page service." << std::endl << 
        "page host=(host) port=(port) appPath=(path to app files)" << std::endl;

    // initialize kcc
    kcc::Properties props;
    props.set("kcc.logName",      KCC_FILE);
    props.set("kcc.logMax",       3L);
    props.set("kcc.logVerbosity", (long) kcc::Log::V_INFO_3);
    if (argc > 1) props.load(argc, argv, false);
    kcc::Core::init(props, KCC_VERSION);

    // run page service
    kcc::Log::Scope scope(KCC_FILE, "main");
    try
    {
        // command line
        kcc::String host   (props.get("host",    "127.0.0.1"));
        int         port   = kcc::Strings::parseInteger(props.get("port", "5150"));
        kcc::String http   (props.get("http",    "k_httpserver"));
        bool        remoteShutdown = props.get("remoteShutdown", KCC_PROPERTY_FALSE) == KCC_PROPERTY_TRUE;
        kcc::String page   (props.get("page",    "k_pageresponse"));
        kcc::String appPath(props.get("appPath", "../../../tst/page"));
        kcc::Log::out("page service @ %s:%d ['stop' to exit]", host.c_str(), port);
    
        // create handlers
        kcc::IHTTPServerFactory* httpFactory = KCC_FACTORY(kcc::IHTTPServerFactory, http);
        kcc::AutoPtr<kcc::IHTTPResponseShutdown> shutdown;

        // register handlers
        PageHandler pageHandler;
        kcc::IPageResponse::Handlers handlers;
        handlers["/"] = &pageHandler;

        // transforms
        kcc::StringMap xforms;
        xforms["page"] = "page.xsl";

        // page response
        kcc::Properties pageConfig;
        pageConfig.set("PageResponse.appPath", appPath);
        kcc::AutoPtr<kcc::IPageResponse> dispatcher(KCC_COMPONENT(kcc::IPageResponse, page));
        if (!dispatcher->init(pageConfig, handlers, xforms)) throw kcc::Exception("failed to create page response");

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
        std::cout << "page service stopping..." << std::endl;
        std::cout.flush();
        s->stop();
        std::cout << "page service stopped" << std::endl;
    }
    catch (std::exception& e)
    {
        kcc::Log::exception(e);
        return 1;
    }

    return 0;
}
