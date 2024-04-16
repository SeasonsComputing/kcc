/*
 * Kuumba C++ Core
 *
 * $Id: httptextquery.cpp 22776 2008-03-24 20:36:12Z tvk $
 */
#include <inc/core/Core.h>
#include <inc/inet/IHTTP.h>
#include <inc/store/ITextStore.h>
#include <inc/store/TextQueryXml.h>
#include <inc/store/TextQueryRest.h>

#define KCC_FILE    "httptextquery"
#define KCC_VERSION "$Id: httptextquery.cpp 22776 2008-03-24 20:36:12Z tvk $"

namespace kcc
{
    // Constants
    static const String                 k_home            ("/");
    static const String                 k_sep             (",");
    static const long                   k_defDocsPerPage = 25L;
    static const TextDocument::Contents k_defContents    = TextDocument::C_METADATA;
    static const float                  k_reviveFactor   = 0.333F;

    // Query cursor
    typedef SharedPtr<struct QueryCursorValue> QueryCursor;
    struct QueryCursorValue
    {
        // Ctor
        QueryCursorValue(const String& id, const String& expression, long contents) 
            : 
            m_id(id), 
            m_expression(expression), 
            m_contents(k_defContents), 
            m_row(-1L),
            m_size(0L), 
            m_total(0L),
            m_accessed(0L),
            m_expired(false),
            m_created(false)
        {
            if (contents >= 0L) m_contents = (TextDocument::Contents) contents;
            std::time(&m_accessed);
        }
        
        // Accessors
        const String& id()                { return m_id; }
        const String& expression()        { return m_expression; }
        TextDocument::Contents contents() { return m_contents; }
        long row()               
        { 
            Mutex::Lock lock(m_sentinel);
            long r = m_row;
            return r; 
        }
        long size()
        {
            Mutex::Lock lock(m_sentinel);
            long s = m_size;
            return s; 
        }
        long total()
        { 
            Mutex::Lock lock(m_sentinel);
            long t = m_total;
            return t; 
        }
        bool expired()
        { 
            Mutex::Lock lock(m_sentinel);
            bool e = m_expired;
            return e; 
        }
        bool created()
        { 
            Mutex::Lock lock(m_sentinel);
            bool c = m_created;
            return c; 
        }
        std::time_t accessed()
        { 
            Mutex::Lock lock(m_sentinel);
            std::time_t a = m_accessed;
            return a; 
        }

        // results: query results
        void results(ITextStore* store, StringStream& buf, long offset, long max) throw (TextException)
        {
            Mutex::Lock lock(m_sentinel);
            Log::Scope scope(KCC_FILE, "QueryCursorValue::results");
            
            Timer t;
            t.start();
            
            // begin query if needed
            if (m_expired || !m_created)
            {
                if (!m_created) offset = 0L; // on creation position at 0
            
                // re/query if expired
                Log::info3("query begin: id=[%s] expr=[%s]", m_id.c_str(), m_expression.c_str());
                m_query.reset(store->query(m_expression, m_contents));
                m_expired = false;
                m_total   = m_query->total();
                m_created = true;
                
                // reclaim text resources for empty result set
                if (m_total == 0L) m_query.reset();
            }

            // next page & max docs/page
            if (max    <  0L)      max    = k_defDocsPerPage;   // default docs/page
            if (offset <  0L)      offset = m_row + max;        // next page
            if (offset >= m_total) m_row  = m_total;            // past end
            
            // log access
            std::time(&m_accessed);

            // write documents as xml
            DOMWriter w(buf);
            w.start(TextQueryXml::root());
            w.attr(TextQueryXml::rootService(), KCC_FILE);
            w.attr(TextQueryXml::rootWhen(),    ISODate::local().isodatetime());
            
            m_size = 0L;
            if (m_total > 0L && max > 0L)
            {
                // seek to position if out of order
                if (offset != m_query->row()) m_query->seek(offset);
                m_row = offset;

                // fetch results up to max or end
                bool cont = true;
                TextDocument txtdoc;
                while (cont)
                {
                    m_query->results(txtdoc);
                    w.start(TextQueryXml::document());
                    w.attr(TextQueryXml::documentRow(), Strings::printf("%d", offset));
                    if (!txtdoc.text.empty())
                    {
                        w.start(TextQueryXml::text());
                        w.text(txtdoc.text);
                        w.end(TextQueryXml::text());
                    }
                    for (StringMap::iterator i = txtdoc.metadata.begin(); i != txtdoc.metadata.end(); i++)
                    {
                        w.start(TextQueryXml::metadata());
                        w.attr(TextQueryXml::metadataKey(),   i->first);
                        w.attr(TextQueryXml::metadataValue(), i->second);
                        w.end(TextQueryXml::metadata());
                    }
                    for (TextDocument::Terms::iterator i = txtdoc.terms.begin(); i != txtdoc.terms.end(); i++)
                    {
                        w.start(TextQueryXml::term());
                        w.attr(TextQueryXml::termTerm(),      i->first);
                        w.attr(TextQueryXml::termFrequency(), Strings::printf("%d", i->second));
                        w.end(TextQueryXml::term());
                    }
                    for (TextDocument::Matches::iterator i = txtdoc.matches.begin(); i != txtdoc.matches.end(); i++)
                    {
                        w.start(TextQueryXml::match());
                        w.attr(TextQueryXml::matchStartOffset(), Strings::printf("%d", i->startOffset));
                        w.attr(TextQueryXml::matchEndOffset(),   Strings::printf("%d", i->endOffset));
                        w.end(TextQueryXml::match());
                    }
                    w.end(TextQueryXml::document());
                    
                    offset++;
                    m_size++;
                    cont = m_size < max && offset < m_total;
                    if (cont) m_query->next();
                }
            }
            w.start(TextQueryXml::status());
            w.attr(TextQueryXml::statusId(),         m_id);
            w.attr(TextQueryXml::statusExpression(), m_expression);
            w.attr(TextQueryXml::statusContents(),   Strings::printf("%d",   m_contents));
            w.attr(TextQueryXml::statusRow(),        Strings::printf("%d",   m_row));
            w.attr(TextQueryXml::statusSize(),       Strings::printf("%d",   m_size));
            w.attr(TextQueryXml::statusTotal(),      Strings::printf("%d",   m_total));
            w.attr(TextQueryXml::time(),             Strings::printf("%.3f", t.now()));
            w.end(TextQueryXml::status());
            
            w.end(TextQueryXml::root());

            Log::info4(
                "query results: id=[%s] row=[%d] size=[%d] total=[%d] accessed=[%s] time=[%.3f]", 
                m_id.c_str(), m_row, m_size, m_total,
                ISODate::local(m_accessed).isodatetime().c_str(), t.secs());
        }
        
        // status: write query status as XML
        void status(DOMWriter& w)
        {
            Mutex::Lock lock(m_sentinel);
            w.start(TextQueryXml::status());
            w.attr(TextQueryXml::statusId(),         m_id);
            w.attr(TextQueryXml::statusExpression(), m_expression);
            w.attr(TextQueryXml::statusSize(),       Strings::printf("%d", m_size));
            w.attr(TextQueryXml::statusRow(),        Strings::printf("%d", m_row));
            w.attr(TextQueryXml::statusTotal(),      Strings::printf("%d", m_total));
            w.attr(TextQueryXml::statusContents(),   Strings::printf("%d", m_contents));
            w.attr(TextQueryXml::statusAccessed(),   ISODate::local(m_accessed).isodatetime());
            w.attr(TextQueryXml::statusExpired(),    (m_expired ? "true" : "false"));
            w.end(TextQueryXml::status());
        }
        
        // expire: expire query
        bool expire(long timeout) throw (TextException)
        {
            Mutex::Lock lock(m_sentinel);
            if (m_expired) return true; // already expired
            if (m_created && (long)(std::time(NULL) - m_accessed) > timeout)            
            {
                m_query.reset();
                m_expired = true;
            }
            return m_expired;
        }

    private:
        // Attributes
        String                 m_id;
        String                 m_expression;
        TextDocument::Contents m_contents;
        long                   m_row;
        long                   m_size;
        long                   m_total;
        std::time_t            m_accessed;
        AutoPtr<ITextResults>  m_query;
        bool                   m_expired;
        bool                   m_created;
        Mutex                  m_sentinel;
    };

    // Query utility
    typedef std::map<String, QueryCursor> NamedQueryCursors;
    struct QueryCursorPriority { bool operator () (QueryCursor a, QueryCursor b) { return a->accessed() > b->accessed(); } };
    typedef std::priority_queue<QueryCursor, std::vector<QueryCursor>, QueryCursorPriority> ExpiredQueryCursors;

    // Query service
    struct QueryService : IHTTPResponse
    {
        // Ctor
        QueryService(
            ITextStore* store, 
            long expire, 
            long maxCursors,
            long memInUseWarnPer,
            long memInUseMax)
            : 
            m_store(store), 
            m_expire(expire), 
            m_nextId(1L), 
            m_maxCursors(maxCursors),
            m_memInUseWarnKB((long)(memInUseMax * (memInUseWarnPer / 100.0))),
            m_memInUseMaxKB(memInUseMax)
        {}

        // onResponse: handle response
        void onResponse(const HTTPRequest& request, IHTTPRequestReader* in, IHTTPResponseWriter* out)
        {
            Log::Scope scope(KCC_FILE, "QueryService::onResponse");
            try
            {
                // expire cursors if needed
                expire(); // TODO: background thread?

                // response
                bool ok = true;
                StringStream xml;
                if (request.path == TextQueryRest::close()) 
                {
                    // close all open connections
                    close();
                    message(xml, TextQueryXml::rootMessage(), "all open queries closed");
                }
                else if (request.path == TextQueryRest::status()) 
                {
                    // dump server status
                    status(xml, request.parameters.exists(TextQueryRest::statusDetail()));
                }
                else if (request.path == TextQueryRest::query())
                {
                    // query
                    String id          = request.parameters[TextQueryRest::queryId()];
                    String expr        = Strings::cleanws(request.parameters[TextQueryRest::queryExpression()]);
                    bool   flush       = request.parameters.exists(TextQueryRest::queryFlush());
                    String strContents = request.parameters[TextQueryRest::queryContents()];
                    String strMax      = request.parameters[TextQueryRest::queryMax()];
                    String strRow      = request.parameters[TextQueryRest::queryRow()];
                    long   row         = strRow.empty()      ? -1L : Strings::parseInteger(strRow);
                    long   contents    = strContents.empty() ? -1L : Strings::parseInteger(strContents);
                    long   max         = strMax.empty()      ? -1L : Strings::parseInteger(strMax);
                    query(xml, id, row, expr, flush, contents, max);
                }
                else
                {
                    // invalid command
                    Log::warning("invalid query command: path=[%s]", request.path.c_str());
                    out->response(HTTP::C_NOT_FOUND);
                    ok = false;
                }
                
                // response if success
                if (ok) out->xml(xml);
            }
            catch (Exception& e)
            {
                Log::exception(e);
                StringStream err;
                message(err, TextQueryXml::rootError(), e.what());
                out->xml(err);
            }
        }

        // expire: expire queries
        void expire()
        {
            Mutex::Lock lock(m_sentinel);
            Log::Scope scope(KCC_FILE, "QueryService::expire");
            
            // collect expired queries
            ExpiredQueryCursors expired;
            for (NamedQueryCursors::iterator i = m_cursors.begin(); i != m_cursors.end(); i++)
            {
                QueryCursor q = i->second;
                if (q->expire(m_expire))
                { 
                    expired.push(q);
                    Log::info3(
                        "query expired: id=[%s] accessed=[%s]", 
                        q->id().c_str(), 
                        ISODate::local(q->accessed()).isodatetime().c_str());
                }
            }
            
            // prune cursors in oldest accessed order retaining newer expired queries for potential revive
            ExpiredQueryCursors::size_type reviveThreshold = (ExpiredQueryCursors::size_type) std::ceil(m_maxCursors * k_reviveFactor);
            while (!expired.empty() && expired.size() > reviveThreshold)
            {
                QueryCursor q = expired.top();
                Log::info3(
                    "query flushing expired: id=[%s] accessed=[%s] threshold=[%d]", 
                    q->id().c_str(), 
                    ISODate::local(q->accessed()).isodatetime().c_str(),
                    reviveThreshold);
                m_cursors.erase(q->id());
                expired.pop();
            }
        }

        // status: server status
        void status(StringStream& xml, bool detail) throw (TextException)
        {
            Mutex::Lock lock(m_sentinel);
            Log::Scope scope(KCC_FILE, "QueryService::status");

            Timer t;
            t.start();

            DOMWriter w(xml);
            w.start(TextQueryXml::root());
            w.attr(TextQueryXml::rootService(),    KCC_FILE);
            w.attr(TextQueryXml::rootIndex(),      m_store->indexRepository());
            w.attr(TextQueryXml::rootMaxCursors(), Strings::printf("%d", m_maxCursors));
            w.attr(TextQueryXml::rootCursors(),    Strings::printf("%d", m_cursors.size()));
            w.attr(TextQueryXml::rootExpire(),     Strings::printf("%d", m_expire));
            w.attr(TextQueryXml::rootWhen(),       ISODate::local().isodatetime());
            if (detail)
            {
                // get status prior to writing so it's included in the header response time
                long         docs = m_store->indexDocuments();
                StringStream bufQueries;
                DOMWriter    wQueries(bufQueries, true);
                for (NamedQueryCursors::iterator i = m_cursors.begin(); i != m_cursors.end(); i++)
                {
                    QueryCursor q = i->second;
                    q->status(wQueries);
                }
                String xmlQueries(bufQueries.str());
                
                // index size (TODO: ?? refactor to k_textstore component ??)
                unsigned long sz = 0L;
                Platform::Files files;
                Platform::fsDir(m_store->indexRepository(), files);
                for (Platform::Files::iterator f = files.begin(); f != files.end(); f++) sz += f->size;

                // write results with response timing
                w.attr(TextQueryXml::rootDocuments(), Strings::printf("%d",   docs));
                w.attr(TextQueryXml::statusSize(),    Strings::printf("%ld",  sz));
                w.attr(TextQueryXml::time(),          Strings::printf("%.3f", t.now()));
                w.node(xmlQueries);

                Log::info3(
                    "status: index=[%s] maxCursors=[%d] cursors=[%d] expire=[%d] docs=[%d] size=[%ld] time=[%.3f]", 
                    m_store->indexRepository().c_str(), m_maxCursors, m_cursors.size(), m_expire, docs, sz, t.secs());
            }

            w.end(TextQueryXml::root());
        }

        // message: write message
        void message(StringStream& xml, const String& type, const String& msg)
        {
            DOMWriter w(xml);
            w.start(TextQueryXml::root());
            w.attr(TextQueryXml::rootService(), KCC_FILE);
            w.attr(TextQueryXml::rootWhen(),    ISODate::local().isodatetime());
            w.attr(type, msg);
            w.end(TextQueryXml::root());
        }

        // query: new or continue query
        void query(
            StringStream& xml, 
            const String& id, long row, 
            const String& expr, bool flush, 
            long contents, long max) throw (TextException)
        {
            Log::Scope scope(KCC_FILE, "QueryService::query");
            QueryCursor qry;
            if (!id.empty())
            {
                // existing query
                Mutex::Lock lock(m_sentinel);
                NamedQueryCursors::iterator find = m_cursors.find(id);
                if (find != m_cursors.end()) qry = find->second;
                if (qry != NULL)
                {
                    // flush
                    if (flush)
                    {
                        m_cursors.erase(id);
                        qry.reset();
                        String msg("query flushed: id=[" + id + "]");
                        Log::info3(msg);
                        message(xml, TextQueryXml::rootMessage(), msg);
                    }
                }
                else
                {
                    String msg("query end, id not found, or expired: id=[" + id + "]");
                    Log::info3(msg);
                    message(xml, TextQueryXml::rootError(), msg);
                }
            }
            else
            {
                // new query
                Mutex::Lock lock(m_sentinel);
                if (!expr.empty())
                {
                    long memInUseKB = Platform::procMemInUseKB();
                    if (memInUseKB < m_memInUseMaxKB && memInUseKB > m_memInUseWarnKB) 
                    {
                        // warn mem in use
                        Log::warning(
                            "approaching maximum mem resource: memInUseKB=[%d] memInUseMaxKB=[%d]", 
                            memInUseKB, m_memInUseMaxKB);
                    }
                    if (memInUseKB > m_memInUseMaxKB)
                    {
                        // max mem in use
                        String msg(Strings::printf(
                            "maximum query mem resource exceeded: memInUseKB=[%d] memInUseMaxKB=[%d]", 
                            memInUseKB, m_memInUseMaxKB));
                        Log::error(msg);
                        message(xml, TextQueryXml::rootError(), msg);
                    }
                    else if (m_cursors.size() >= (NamedQueryCursors::size_type)m_maxCursors)
                    {
                        // max cursors
                        String msg(Strings::printf(
                            "maximum query cursors exceeded: maxCursors=[%d]", 
                            m_maxCursors));
                        Log::error(msg);
                        message(xml, TextQueryXml::rootError(), msg);
                    }
                    else
                    {
                        // create query definition
                        qry = QueryCursor(new QueryCursorValue(Strings::printf("%x", m_nextId++), expr, contents));
                        m_cursors[qry->id()] = qry;
                    }
                }
                else
                {
                    message(xml, TextQueryXml::rootError(), "missing expression");
                }
            }

            // query results using query mutex instead of collection mutex
            if (qry != NULL) qry->results(m_store, xml, row, max);
        }

        // close: close all query cursors
        void close()
        {
            Mutex::Lock lock(m_sentinel);
            Log::Scope scope(KCC_FILE, "QueryService::close");
            Log::info3("closing all queries: cached=[%d]", m_cursors.size());
            for (NamedQueryCursors::iterator i = m_cursors.begin(); i != m_cursors.end(); i++)
                Log::info3("query close: id=[%s] rc=[%d]", i->first.c_str(), i->second.rc());
            m_cursors.clear();
        }

    private:
        // Attributes
        NamedQueryCursors m_cursors;
        ITextStore*       m_store;
        Mutex             m_sentinel;
        long              m_expire;
        long              m_nextId;
        long              m_maxCursors;
        long              m_memInUseWarnKB;
        long              m_memInUseMaxKB;
    };

    // Query home handler
    struct QueryHome : IHTTPResponse
    {
        // onResponse: handle response
        void onResponse(const HTTPRequest& request, IHTTPRequestReader* in, IHTTPResponseWriter* out)
        {
            Log::Scope scope(KCC_FILE, "QueryHome::onResponse");
            StringStream xml;
            DOMWriter r(xml);
            r.start(TextQueryXml::root());
            r.attr(TextQueryXml::rootService(), KCC_FILE);
            r.attr(TextQueryXml::rootWhen(),    ISODate::local().isodatetime());
            r.end(TextQueryXml::root());
            out->xml(xml);
        }
    };
}

// main: entry point into httptextquery service
int main(int argc, const char* argv[])
{
    std::cout <<
        KCC_FILE << " - Kuumba text index HTTP query service (Lucene wrapper)" << std::endl << std::endl;

    // initialize kcc
    kcc::Properties props;
    props.set("kcc.logName",      KCC_FILE);
    props.set("kcc.logMax",       99L);
    props.set("kcc.logVerbosity", (long) kcc::Log::V_INFO_1);
    if (argc > 1) props.load(argc, argv, false);
    kcc::Core::init(props, KCC_VERSION);

    // run httpmodeler
    kcc::Log::Scope scope(KCC_FILE, "main");
    try
    {
        // command line
        kcc::String host            (props.get("host", "127.0.0.1"));
        int         port            = (int)props.get("port", 8100L);
        kcc::String http            (props.get("http", "k_httpserver"));
        bool        remoteShutdown  = props.get("remoteShutdown", KCC_PROPERTY_TRUE) == KCC_PROPERTY_TRUE;
        kcc::String notifyURL       (props.get("notifyURL", kcc::Strings::empty()));
        kcc::String store           (props.get("store", "k_textstore"));
        kcc::String path            (props.get("path", kcc::Strings::empty()));
        long        expires         = props.get("expires", 60L*9L);              // 9 minutes
        long        cursors         = props.get("cursors", 64L);                 // 64 cursors
        long        memInUseWarnPer = props.get("memInUseWarnPer", 90L);         // 90%
        long        memInUseMaxKB   = props.get("memInUseMaxKB",   1024L*1024L); // 1GB
        if (path.empty()) path = props.get("TextStore.path", kcc::Strings::empty());
        path = kcc::Platform::fsNormalize(path);
        kcc::Log::out(
            "%s: service=[%s:%d] remoteShutdown=[%d] notifyURL=[%s] "
            "path=[%s] expires=[%d] cursors=[%d] memInUseWarnPer=[%d] memInUseMaxKB=[%d]",
            KCC_FILE, host.c_str(), port, remoteShutdown, notifyURL.c_str(),
            path.c_str(), expires, cursors, memInUseWarnPer, memInUseMaxKB);

        // text store
        if (!path.empty()) props.set("TextStore.path", path);
        kcc::AutoPtr<kcc::ITextStore> textStore(KCC_COMPONENT(kcc::ITextStore, store));
        if (!textStore->init(props)) return 1;

        // response handlers
        kcc::IHTTPServerFactory* httpFactory = KCC_FACTORY(kcc::IHTTPServerFactory, http);
        kcc::AutoPtr<kcc::IHTTPResponseDispatcher> dispatcher(httpFactory->constructDispatcher());
        kcc::AutoPtr<kcc::IHTTPResponseShutdown>   shutdown;
        kcc::AutoPtr<kcc::IHTTPResponse>           service(httpFactory->constructServiceStatus());
        kcc::QueryHome    home;
        kcc::QueryService query(textStore, expires, cursors, memInUseWarnPer, memInUseMaxKB);
        kcc::IHTTPResponseDispatcher::ResponseHandlers& handlers = dispatcher->handlers();
        handlers[kcc::k_home]                   = &home;
        handlers[kcc::TextQueryRest::query()]   = &query;
        handlers[kcc::TextQueryRest::status()]  = &query;
        handlers[kcc::TextQueryRest::close()]   = &query;
        handlers[kcc::TextQueryRest::service()] = service;
        if (remoteShutdown)
        {
            shutdown = httpFactory->constructShutdown(KCC_FILE);
            handlers[kcc::TextQueryRest::shutdown()] = shutdown;
            std::cout << "remote shutdown: dispatch '" << kcc::TextQueryRest::shutdown() << "' to shutdown" << std::endl;
        }
        else
        {
            std::cout << "local shutdown: type 'stop' to shutdown" << std::endl;
        }
        std::cout.flush();

        // http server
        kcc::AutoPtr<kcc::IHTTPServer> s(httpFactory->constructServer());
        if (!s->init(host, port, dispatcher, notifyURL, 512)) return 1;
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
        std::cout << KCC_FILE << " stopping..." << std::endl;
        std::cout.flush();
        s->stop();
        query.close();
        std::cout << KCC_FILE << " stopped" << std::endl;
    }
    catch (std::bad_alloc& e)
    {
        kcc::Log::exception(e);
        return 2;
    }
    catch (std::exception& e)
    {
        kcc::Log::exception(e);
        return 1;
    }

    return 0;
}
