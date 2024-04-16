/*
 * Kuumba C++ Core
 *
 * $Id: TextQueryClient.cpp 22694 2008-03-17 18:49:19Z tvk $
 */
#include <inc/core/Core.h>
#include <inc/store/ITextStore.h>
#include <inc/store/TextQueryXml.h>
#include <inc/store/TextQueryRest.h>

#define KCC_FILE    "TextQueryClient"
#define KCC_VERSION "$Id: TextQueryClient.cpp 22694 2008-03-17 18:49:19Z tvk $"

namespace kcc
{
    // Configuration 
    static const String k_keyConnections("TextQueryClient.connections");
    static const String k_keyMaxDocs    ("TextQueryClient.maxDocs");
    static const long   k_defMaxDocs    = 25L;

    // Constants
    static const String k_sep     (",");
    static const String k_urlValue("=");
    static const String k_urlSep  ("&");

    // Helper class for results
    struct ResultSet
    {
        URL    url;
        String id;
        bool   next;
        long   row;
        long   total;
        long   size;
        long   start;
        String error;
        TextDocuments documents;
        ResultSet(const URL& _url) : url(_url), next(false), row(-1L), total(0L), size(0L), start(-1L) {}
        inline bool operator == (const String& rhs) const { return id == rhs; }
    };
    typedef std::list<ResultSet> Results;

    // Results of text query
    struct TextQueryClientResults : ITextResults
    {
        // Helper thread to run initial query in parallel
        struct QueryBeginThread : Thread
        {
            TextQueryClientResults* client;
            ResultSet&              results;
            Monitor&                done;
            QueryBeginThread(TextQueryClientResults* c, ResultSet& r, Monitor& d)
                : Thread("QueryBeginThread"), client(c), results(r), done(d) 
            {
                done.init();
            }
            virtual void invoke()
            {
                Log::Scope scope(KCC_FILE, "QueryBeginThread::begin");
                try
                {
                    client->query(results, 0L, 0L);
                }
                catch (TextException&)
                {
                    // error handled by query client
                }
                done.notify();
            }
        };

        // Attributes
        long                   m_total;
        long                   m_row;
        long                   m_maxDocs;
        TextDocument::Contents m_contents;
        String                 m_expression;
        Results                m_results;
        Results::iterator      m_itResults;
        TextQueryClientResults() : 
            m_total(0L), m_row(-1L), m_maxDocs(1L), 
            m_contents(TextDocument::C_TEXT|TextDocument::C_METADATA)
        {}

        // dtor: flush any pending results
        ~TextQueryClientResults()
        {
            Log::Scope scope(KCC_FILE, "~TextQueryClientResults");
            Log::info3("TextQueryClient cleanup: cursors=[%d]", m_results.size());
            String xml;
            for (Results::iterator i = m_results.begin(); i != m_results.end(); i++)
            {
                ResultSet& rs = *i;
                try
                {
                    if (!rs.id.empty())
                    {
                        rs.url.query = Strings::printf("%s=%s&%s", 
                            TextQueryRest::queryId().c_str(), 
                            rs.id.c_str(), 
                            TextQueryRest::queryFlush().c_str());
                        HTTP::getxml(rs.url, xml);
                    }
                }
                catch (Exception& e)
                {
                    Log::exception(e);
                }
            }
            m_results.clear();
        }

        // begin: begin query
        void begin(const StringVector& connections, long maxDocs, const String& expression, TextDocument::Contents contents)
            throw (TextException)
        {
            Log::Scope scope(KCC_FILE, "TextQueryClientResults::begin");

            // init
            m_maxDocs    = maxDocs;
            m_expression = URL::encode(expression);
            m_contents   = contents;
            for (StringVector::const_iterator i = connections.begin(); i != connections.end(); i++)
            {
                String con(Strings::toLower(*i));
                if (con.find("http://") != 0) con = "http://" + *i;
                else                          con = *i;
                URL url(con);
                url.path = TextQueryRest::query();
                m_results.push_back(ResultSet(url));
            }
            Log::info3("query begin: expr=[%s]", expression.c_str());

            // initial query
            Timer t;
            t.start();
            m_total = 0L;
            m_row   = -1L;

            // dispatch initial text query to each service
            //  - return no documents, just queue up the index cache
            Monitor done;
            for (Results::iterator i = m_results.begin(); i != m_results.end(); i++)
                (new QueryBeginThread(this, *i, done))->go();
            done.wait();

            // check for errors            
            String errors;
            for (Results::iterator i = m_results.begin(); i != m_results.end(); i++) 
            {
                if (!i->error.empty())
                {
                    if (!errors.empty()) errors += "\n";
                    errors += i->error.c_str();
                }
            }
            if (!errors.empty()) throw TextException(errors);            

            // fetch total documents            
            for (Results::iterator i = m_results.begin(); i != m_results.end(); i++) 
                m_total += i->total;
            if (m_total == 0L) m_total = -1L; // invalidate results (nothing to iterate)
            else
            {
                // iterate results from beginning
                m_itResults = m_results.begin();
                Log::info4("begin results: total=[%d] time=[%.3f]", m_total, t.now());
            }
        }

        // next: query if any results remaining
        bool next() throw (TextException)
        { 
            Log::Scope scope(KCC_FILE, "TextQueryClientResults::next");

            if (m_total == -1L || m_row >= m_total) return false;

            // check current resultset cache first
            m_itResults->row++;
            m_row++;
            if (m_itResults->size > 0L && m_itResults->row < m_itResults->size) return true;

            // refresh cache
            bool next = false;
            do
            {
                if (m_itResults->start + m_itResults->row < m_itResults->total)
                {
                    query(*m_itResults, m_itResults->start + m_itResults->row, m_maxDocs);
                    next = true;
                    break;
                }
                m_itResults++;
                if (m_itResults != m_results.end() && m_itResults->start < 0L) 
                    m_itResults->start = m_itResults->row = 0L; // move to initial row if newly created
            }
            while (m_itResults != m_results.end());
            
            return next;
        }

        // total: total across all results
        long total() { return m_total; }

        // row: virtual row across all results
        long row() { return m_row; }
        
        // seek: seek to row
        void seek(long row) throw (TextException) 
        { 
            Log::Scope scope(KCC_FILE, "TextQueryClientResults::seek");
            
            if (m_total == -1L) throw TextException("query state not valid: seek() called on invalid cursor.");
            if (row < 0L || row >= m_total) throw TextException("row seek out of bounds");
            
            long total  = 0L;
            long offset = row;    // offset into resultset of seek row
            for (Results::iterator i = m_results.begin(); i != m_results.end(); i++)
            {
                long start = total;
                long end   = total + i->total;
                if (row >= start && row < end)
                {
                    // seek row to result offset
                    m_row = row;
                    m_itResults = i;
                    query(*m_itResults, offset, m_maxDocs);
                }
                else
                {
                    // reset result cursor
                    i->row = i->start = -1L;
                    i->documents.clear();
                }
                offset -= i->total;
                total  += i->total;
            }
        }

        // results: fetch next document from cache
        void results(TextDocument& txtdoc) throw (TextException)
        {
            Log::Scope scope(KCC_FILE, "TextQueryClientResults::results");
            
            if (
                m_total == -1L ||
                m_row == -1L || 
                m_row >= m_total ||
                m_itResults == m_results.end()) 
            {
                throw TextException("query state not valid: results() called on invalid cursor.");
            }
            
            if (m_itResults->size <= 0L || m_itResults->row >= m_itResults->size)
            {
                throw TextException("query state not valid: results() called on invalid result set cache.");
            }
            txtdoc = m_itResults->documents[m_itResults->row];
        }

        // query: update results cache
        void query(ResultSet& results, long row, long max) throw (TextException)
        {
            Log::Scope scope(KCC_FILE, "TextQueryClientResults::query");
            try
            {
                // fetch first or subsequent page of results
                if (results.id.empty())
                {
                    results.url.query.clear();
                    results.url.query.reserve(4096);
                    results.url.query += 
                        TextQueryRest::queryExpression() + k_urlValue +
                        m_expression + k_urlSep +
                        TextQueryRest::queryContents() + k_urlValue +
                        Strings::printf("%d", m_contents);
                    Log::info4("cursor begin: host=[%s:%d]", results.url.host.c_str(), results.url.port);
                }
                else
                {
                    results.url.query = TextQueryRest::queryId() + k_urlValue + results.id;
                    Log::info4(
                        "cursor results: host=[%s:%d] id=[%s] total=[%d] row=[%d]", 
                        results.url.host.c_str(), results.url.port, results.id.c_str(), results.total, row);
                }
                results.url.query += 
                    k_urlSep + TextQueryRest::queryMax() + k_urlValue + Strings::printf("%d", max) +
                    k_urlSep + TextQueryRest::queryRow() + k_urlValue + Strings::printf("%d", row);
                String xml;
                HTTP::getxml(results.url, xml);

                // parse xml
                AutoPtr<IDOMNode> root(Core::rodom()->parseXML(xml));
                DOMReader r(root);
                const IDOMNode* doc = r.doc(TextQueryXml::root());

                // error
                String error;
                if (r.attrOp(doc, TextQueryXml::rootError(), error)) throw TextException(error);
                
                // status
                const IDOMNode* status = r.node(doc, TextQueryXml::status());
                results.id    = r.attr(status, TextQueryXml::statusId());
                results.total = Strings::parseInteger(r.attr(status, TextQueryXml::statusTotal()));
                results.start = Strings::parseInteger(r.attr(status, TextQueryXml::statusRow()));
                results.size  = Strings::parseInteger(r.attr(status, TextQueryXml::statusSize()));
                results.row   = 0L;

                // documents
                String k, v;
                results.documents.clear();
                results.documents.reserve(results.size);
                AutoPtr<IDOMNodeList> docs(r.nodes(doc, TextQueryXml::document()));
                long sz = docs->getLength();
                if (sz != results.size)
                { 
                    Log::error("xml corrupted during streaming, rows: expected=[%d] received=[%d]", results.size, sz);
                    results.size = sz;
                }
                for (long i = 0L; i < sz && i < results.total; i++)
                {
                    const IDOMNode* d = docs->getItem(i);
                    results.documents.push_back(TextDocument());
                    TextDocument& txtdoc = results.documents.back();

                    // text
                    const IDOMNode* txt = r.nodeOp(d, TextQueryXml::text());
                    if (txt != NULL) txtdoc.text = r.text(txt);

                    // metadata
                    AutoPtr<IDOMNodeList> md(r.nodes(d, TextQueryXml::metadata()));
                    long mdsz = md->getLength();
                    for (long j = 0L; j < mdsz; j++)
                    {
                        const IDOMNode* m = md->getItem(j);
                        k = r.attr(m, TextQueryXml::metadataKey());
                        r.attrOp(m, TextQueryXml::metadataValue(), v);
                        txtdoc.metadata[k] = v;
                    }

                    // terms
                    AutoPtr<IDOMNodeList> tfv(r.nodes(d, TextQueryXml::term()));
                    long tfvsz = tfv->getLength();
                    for (long j = 0L; j < tfvsz; j++)
                    {
                        const IDOMNode* t = tfv->getItem(j);
                        k = r.attr(t, TextQueryXml::termTerm());
                        r.attrOp(t, TextQueryXml::termFrequency(), v);
                        txtdoc.terms[k] = Strings::parseInteger(v);
                    }
                    
                    // matches
                    AutoPtr<IDOMNodeList> match(r.nodes(d, TextQueryXml::match()));
                    long matchsz = match->getLength();
                    for (long j = 0L; j < matchsz; j++)
                    {
                        const IDOMNode* m = match->getItem(j);
                        long so = Strings::parseInteger(r.attr(m, TextQueryXml::matchStartOffset()));
                        long eo = Strings::parseInteger(r.attr(m, TextQueryXml::matchEndOffset()));
                        txtdoc.matches.push_back(TextDocument::Match(so, eo));
                    }
                }
                results.error.clear();
            }
            catch (Exception& e)
            {
                // invalidate results cache and cursor
                results.row = results.start = -1L;
                results.size = 0L;
                results.documents.clear();
                results.error = e.what();
                throw TextException(e.what());
            }
        }
    };

    // Results of text query
    struct TextQueryClient : ITextQuery
    {
        // Attributes
        StringVector m_connections;
        long         m_maxDocs;

        // init: initialize query
        bool init(const Properties& config)
        {
            Log::Scope scope(KCC_FILE, "init");

            // init
            String cons(config.get(k_keyConnections, Strings::empty()));
            Strings::tokenize(cons, k_sep, m_connections);
            if (m_connections.empty())
            {
                Log::error("no connections specified");
                return false;
            }
            m_maxDocs = config.get(k_keyMaxDocs, k_defMaxDocs);

            Log::info2("TextQueryClient initialized: connections=[%d] maxDocs=[%d]", m_connections.size(), m_maxDocs);
            return true;
        }

        // query: execute text query
        ITextResults* query(const String& expression, TextDocument::Contents contents) throw (TextException)
        {
            AutoPtr<TextQueryClientResults> tr(new TextQueryClientResults());
            tr->begin(m_connections, m_maxDocs, expression, contents);
            return tr.release();
        }
    };
    
    //
    // TextQueryClient factory
    //

    KCC_COMPONENT_FACTORY_IMPL(TextQueryClient)
    
    KCC_COMPONENT_FACTORY_METADATA_BEGIN_COMPONENT(TextQueryClient, ITextQuery)
        KCC_COMPONENT_FACTORY_METADATA_PROPERTY(KCC_COMPONENT_FACTORY_SCM, KCC_VERSION)
    KCC_COMPONENT_FACTORY_METADATA_END
}
