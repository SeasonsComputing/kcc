/*
 * Kuumba C++ Core
 *
 * $Id: TextStore.cpp 22760 2008-03-24 15:51:02Z tvk $
 */
#include <inc/core/Core.h>
#include <inc/store/ITextStore.h>
#include "TextStoreImport.h"

#define KCC_FILE    "TextStore"
#define KCC_VERSION "$Id: TextStore.cpp 22760 2008-03-24 15:51:02Z tvk $"

namespace kcc
{
    // Lucene clean-up
    static struct CLuceneClean
    { 
        ~CLuceneClean() 
        { 
            ::_lucene_shutdown(); 
            #ifdef _DEBUG
                lucene::debug::LuceneBase::__cl_ClearMemory();
            #endif
        } 
    } k_clean;

    // Configuration 
    static const String k_keyPath          ("TextStore.path");
    static const String k_keyCreate        ("TextStore.create");
    static const String k_keyDocsPerOpt    ("TextStore.docsPerOptimization");
    static const String k_keyMaxFieldLength("TextStore.maxFieldLength");
    static const String k_keyMaxMergeDocs  ("TextStore.maxMergeDocs");
    static const String k_keyMergeFactor   ("TextStore.mergeFactor");
    static const String k_keyStopWords     ("TextStore.stopWords");
    static const String k_defPath          ("store");
    static const long   k_defCreate         = KCC_PROPERTY_FALSE;
    static const long   k_defDocsPerOpt     = -1L;
    static const long   k_defMaxFieldLength = 8096L;
    static const long   k_defMaxMergeDocs   = -1L;
    static const long   k_defMergeFactor    = -1L;
    static const String k_defStopWords(
        "a,an,and,are,as,at,"
        "be,but,by,for,if,"
        "in,into,is,it,no,not,"
        "of,on,or,s,such,"
        "t,that,the,their,then,there,these,they,this,to,"
        "was,will,with"
    );

    // Constants
    static const String k_text("#text");
    
    // Utility class to manage shared query state
    struct QuerySharedState
    {
        // Finder
        struct Finder
        {
            // Accessor: locks state
            Finder(QuerySharedState& s) : m_queryState(s) { m_queryState.m_sentinel->lock(); }
            ~Finder()                                     { m_queryState.m_sentinel->unlock(); }
            
            // Analyzer
            inline lucene::analysis::standard::StandardAnalyzer* analyzer() { return m_queryState.m_analyzer; }
            
            // Searcher: creates if locked, out of date, or uncreated
            inline lucene::search::IndexSearcher* searcher()
            {
				m_queryState.prepare();
                return m_queryState.m_searcher; 
            }
            
        private:
            Finder(const Finder&);
            Finder& operator = (const Finder&);
        
            // Attributes
            QuerySharedState& m_queryState;
        };
        
        // Init
        QuerySharedState() :
            m_lastModified(0L), 
            m_path(Strings::emptyRC()),
            m_sentinel(new Mutex()),
            m_searcher(NULL),
            m_analyzer(NULL) 
        {}
        void init(
            const String& p, 
            lucene::analysis::standard::StandardAnalyzer* s) 
        { 
            m_path     = p; 
            m_analyzer = s;
        }

		// prepare: prepare finder for searching
		void prepare()
		{
            Log::Scope scope(KCC_FILE, "QuerySharedState::prepare");
			Mutex::Lock lock(*m_sentinel);
            if (m_path.empty()) throw TextException("shared state not initialized with path");
            
            // if index is locked or has modified create new searcher to ensure
            // most recent segments are available
            lucene::index::IndexReader* r = NULL;
            bool initial  = m_searcher == NULL;
            bool modified = false;
            bool locked   = false;
            if (!initial)
            {
                r        = m_searcher->getReader();
                modified = (long)r->lastModified(r->getDirectory()) != m_lastModified;
                locked   = r->isLocked(r->getDirectory());
            }
            
            // lazy creation of searcher
            if (initial || modified || locked)
            {
				Log::info3(
					"index searcher created: initial=[%d] modified=[%d] locked=[%d]",
					initial, modified, locked);
                m_searcher.reset(new lucene::search::IndexSearcher(m_path.c_str()));
                r = m_searcher->getReader();
                m_lastModified = (long)r->lastModified(r->getDirectory());
            }
		}
        
    private:
        QuerySharedState& operator = (const QuerySharedState&);
    
        // Attributes
        long                                          m_lastModified;
        StringRC                                      m_path;
        kcc::SharedPtr<Mutex>                         m_sentinel;
        kcc::SharedPtr<lucene::search::IndexSearcher> m_searcher;
        lucene::analysis::standard::StandardAnalyzer* m_analyzer;
    };
    
    // Utility class to manage query matches
    struct QueryMatch
    {
        StringVector terms;
        int          dist;
        QueryMatch(int d = 0) : dist(d) {}
    };
    typedef std::list<QueryMatch> QueryMatches;
    
    // Utility class to manage text tokens
    struct TextToken
    {
        String token;
        long   start;
        long   end;
        TextToken(const String& t, long s, long e) : token(t), start(s), end(e) {}
    };
    typedef std::list<TextToken> TextTokens;

    // Results of text query
    struct TextResults : ITextResults
    {
        // Attributes
        QuerySharedState       m_queryState;
        Mutex                  m_sentinel;
        TextDocument::Contents m_contents;
        long                   m_row;
        lucene::search::Query* m_query;
        lucene::search::Hits*  m_hits;
        QueryMatches           m_matches;
        TextResults(QuerySharedState state, TextDocument::Contents contents) 
            : 
            m_queryState(state),
            m_contents(contents), 
            m_row(-1L), 
            m_query(NULL), 
            m_hits(NULL)
        {}
        ~TextResults() 
        {
            Mutex::Lock lock(m_sentinel);
            _CLDELETE(m_hits);
            _CLDELETE(m_query);
        }

        // begin: begin query
        void begin(const String& expr) throw (TextException)
        {
            Mutex::Lock lock(m_sentinel);
            Log::Scope scope(KCC_FILE, "TextResults::begin");
            if (m_hits != NULL) throw TextException("attempt to begin() again. only once allowed");
            try
            {
                QuerySharedState::Finder finder(m_queryState);
                m_query = lucene::queryParser::QueryParser::parse(expr.c_str(), k_text.c_str(), finder.analyzer());
		        m_hits  = finder.searcher()->search(m_query);
                m_row   = -1L;
                queryMatchBuild(finder);
                
                // log expressions
                if (Log::verbosity() >= Log::V_INFO_4)
                {
                    // raw query
                    Log::info4("raw query: expr=[%s]", expr.c_str());

                    // expanded query
                    TCHAR* eq = m_query->toString();
                    Log::info4("expanded query: expr=[%s]", eq);
                    _CLDELETE_ARRAY(eq);
                    
                    // rewritten query
                    lucene::search::Query* q = m_query->rewrite(finder.searcher()->getReader());
                    TCHAR* rq = q->toString();
                    Log::info4("rewritten query: expr=[%s]", rq);
                    _CLDELETE_ARRAY(rq);
                    if (q != m_query) _CLDELETE(q);
                }
            }
            catch (CLuceneError& e)
            {
                throw TextException(e.what());
            }
        }

        // total: total results
        long total()
        {
            Mutex::Lock lock(m_sentinel);
            long t = m_hits->length();
            return t; 
        }

        // row: return current row
        long row()
        { 
            Mutex::Lock lock(m_sentinel);
            long r = m_row;
            return r; 
        }

        // next: check for next row
        bool next() throw (TextException)
        {
            Mutex::Lock lock(m_sentinel);
            m_row++;
            bool n = m_row < m_hits->length();
            return n;
        }
        
        // seek: seek to row
        void seek(long row) throw (TextException)
        {
            Mutex::Lock lock(m_sentinel);
            if (row < 0L || row >= m_hits->length()) throw TextException("invalid seek position");
            m_row = row;
        }

        // results: fetch current query results
        void results(TextDocument& txtdoc) throw (TextException)
        {
            Mutex::Lock lock(m_sentinel);
            Log::Scope scope(KCC_FILE, "TextResults::results");
            if (m_row < 0L || m_row >= m_hits->length()) throw TextException("invalid cursor position");
            try
            {
                QuerySharedState::Finder finder(m_queryState);
                
                txtdoc.clear();
			    lucene::document::Document& doc = m_hits->doc(m_row);

                // metadata
                if ((m_contents & TextDocument::C_METADATA) != 0)
                {
                    lucene::document::DocumentFieldEnumeration* fields = doc.fields();
                    while (fields->hasMoreElements())
                    {
                        lucene::document::Field* f = fields->nextElement();
                        if (f->name() != k_text) txtdoc.metadata[f->name()] = f->stringValue();
                    }
                    _CLDELETE(fields);
                }

                // terms
                if ((m_contents & TextDocument::C_TERMS) != 0)
                {
                    long id = m_hits->id(m_row);
                    lucene::index::TermFreqVector* tfv = finder.searcher()->getReader()->getTermFreqVector(id, k_text.c_str());
                    if (tfv != NULL)
                    {
                        const Char** terms  = tfv->getTerms();
                        const int*   counts = tfv->getTermFrequencies();
                        long         sz     = tfv->size();
                        for (long i = 0L; i < sz; i++) txtdoc.terms[terms[i]] = counts[i];
                        _CLDELETE(tfv);
                    }
                }

                // text
                if ((m_contents & TextDocument::C_TEXT) != 0) 
                {
                    const Char* txt = doc.get(k_text.c_str());
                    if (txt != NULL) txtdoc.text = txt;
                }
                
                // matches
                if ((m_contents & TextDocument::C_QUERY_MATCHES) != 0) 
                {
                    TextDocument::Matches matches;
                    selectMatches(finder, txtdoc.text, matches);
                    mergeMatches(matches, txtdoc.matches);
                }
            }
            catch (CLuceneError& e)
            {
                throw TextException(e.what());
            }
            catch (Exception& e)
            {
                throw TextException(e.what());
            }
        }
        
        // selectMatches: select offsets where query expression matches document text
        void selectMatches(QuerySharedState::Finder& finder, const kcc::String& text, TextDocument::Matches& matches)
        {
            TextTokens tokens;
            textTokenize(finder, tokens, text);
            TextTokens::iterator tti = tokens.begin();
            while (tti != tokens.end())
            {
                TextToken& tt = *tti;
                
                // check token for a query selection match
                for (QueryMatches::iterator mi = m_matches.begin(); mi != m_matches.end(); mi++)
                {
                    QueryMatch& m = *mi;
                    bool matched = std::find(m.terms.begin(), m.terms.end(), tt.token) != m.terms.end();
                    if (!matched) continue;

                    // match selections
                    if (m.terms.size() == 1)
                    {
                        // phrase: single term
                        matches.push_back(TextDocument::Match(tti->start, tti->end));
                    }
                    else if (m.dist == 0)
                    {
                        // phrase: exact; expand selection to match bounds in phrase token order
                        long start = tt.start;
                        long end   = tt.start;
                        StringVector::iterator itExpr = m.terms.begin();
                        TextTokens::iterator   itTok  = tti;
                        for (
                            ;
                            itTok != tokens.end() && itExpr != m.terms.end() && matched;
                            itTok++, itExpr++)
                        {
                            TextToken& curr = *itTok;
                            if (curr.token != *itExpr) matched = false;
                            else                       end     = curr.end;
                        }
                        if (matched) matches.push_back(TextDocument::Match(start, end));
                    }
                    else
                    {
                        // phrase: approximate; select only tokens in any order
                        StringSet             exprMatches;
                        TextDocument::Matches potentialMatches;
                        int                   offset = 0;
                        TextTokens::iterator  itTok  = tti;
                        for (
                            ;
                            itTok != tokens.end() && offset <= m.dist;
                            itTok++)
                        {
                            TextToken& curr = *itTok;
                            offset++;
                            if (std::find(m.terms.begin(), m.terms.end(), curr.token) != m.terms.end())
                            {
                                exprMatches.insert(curr.token);
                                potentialMatches.push_back(TextDocument::Match(curr.start, curr.end));
                                offset = 0;
                            }
                        }
                        matched = exprMatches.size() == m.terms.size(); // must match all expr tokens
                        if (matched) matches.insert(matches.end(), potentialMatches.begin(), potentialMatches.end());
                    }
                }
                
                // iterate to next token but not past end
                if (tti != tokens.end()) tti++;
            }
        }
        
        // mergeMatches: merge duplicate start bounds preferring largest offset range
        static bool k_compareMatchStart(const TextDocument::Match& l, const TextDocument::Match& r)
        {
            return l.startOffset < r.startOffset;
        }
        static bool k_compareMatchDistance(const TextDocument::Match& l, const TextDocument::Match& r)
        {
            return (l.endOffset-l.startOffset) > (r.endOffset-r.startOffset);
        }
        void mergeMatches(TextDocument::Matches& matches, TextDocument::Matches& merged)
        {
            // no-lock: called by synch'd method
            
            // merge on start order where greatest length is preferred over shorter length
            std::stable_sort(matches.begin(), matches.end(), k_compareMatchStart);
            std::stable_sort(matches.begin(), matches.end(), k_compareMatchDistance);
            merged.clear();
            typedef std::set<long> StartOffsets;
            StartOffsets starts;
            for (TextDocument::Matches::iterator i = matches.begin(); i != matches.end(); i++)
            {
                const TextDocument::Match& m = *i;
                if (starts.find(m.startOffset) != starts.end()) continue; // already merged
                starts.insert(m.startOffset);
                merged.push_back(m);
            }
        }
        
        // textTokenize: tokenize text
        void textTokenize(QuerySharedState::Finder& finder, TextTokens& tokens, const String& text)
        {
            // no-lock: called by synch'd method
            lucene::util::Reader*          reader = _CLNEW lucene::util::StringReader(text.c_str());
            lucene::analysis::TokenStream* source = finder.analyzer()->tokenStream(k_text.c_str(), reader);
            lucene::analysis::Token        token;
            while (source->next(&token))
                tokens.push_back(TextToken(token.termText(), token.startOffset(), token.endOffset()));
            _CLDELETE(source);
            _CLDELETE(reader);
        }

        // queryMatchBuild: build query match
        void queryMatchBuild(QuerySharedState::Finder& finder)
        {
            // no-lock: called by synch'd method
            m_matches.clear();
            if ((m_contents & TextDocument::C_QUERY_MATCHES) != 0) 
            {
                lucene::search::Query* q = m_query->rewrite(finder.searcher()->getReader());
                queryMatchBuild(m_matches, q);
                if (q != m_query) _CLDELETE(q);
            }
        }
        
        // queryMatchBuild: decompose query building match
        void queryMatchBuild(QueryMatches& matches, lucene::search::Query* q)
        {
            // no-lock: called by synch'd method
            if (q->instanceOf("BooleanQuery"))
            {
                lucene::search::BooleanQuery* bq = (lucene::search::BooleanQuery*) q;
                lucene::search::BooleanClause** clauses = bq->getClauses();
                int32_t sz = bq->getClauseCount();
                for (int32_t i = 0; i < sz; i++) 
                {
                    if (!clauses[i]->prohibited) queryMatchBuild(matches, clauses[i]->query);
                }
                _CLDELETE_ARRAY(clauses);
            }
            else if (q->instanceOf("PhraseQuery"))
            {
                lucene::search::PhraseQuery* pq = (lucene::search::PhraseQuery*) q;
                QueryMatch m(pq->getSlop());
                int32_t sz = pq->terms.size();
                for (int32_t i = 0; i < sz; i++) 
                {
                    if (k_text == pq->terms[i]->field()) m.terms.push_back(pq->terms[i]->text());
                }
                queryMatchBuildAdd(matches, m);
            }
            else if (q->instanceOf("TermQuery"))
            {
                lucene::search::TermQuery* tq = (lucene::search::TermQuery*) q;
                lucene::index::Term* t = tq->getTerm();
                if (k_text == t->field())
                {
                    QueryMatch m;
                    m.terms.push_back(t->text());
                    queryMatchBuildAdd(matches, m);
                }
                _CLDECDELETE(t);
            }
        }
        
        // queryMatchBuildAdd: add match if unique
        void queryMatchBuildAdd(QueryMatches& matches, QueryMatch& m)
        {
            // no-lock: called by synch'd method
            bool found = false;
            StringVector::size_type sz = m.terms.size();
            for (QueryMatches::iterator i = matches.begin(); i != matches.end(); i++)
            {
                if (i->terms.size() != sz) continue;
                bool termsMatch = true;
                for (StringVector::size_type j = 0; j < sz; j++)
                {
                    if (i->terms[j] != m.terms[j])
                    {
                        termsMatch = false;
                        break;
                    }
                }
                if (termsMatch) 
                {
                    found = true;
                    break;
                }
            }
            if (!found) matches.push_back(m);
        }
    };

    // Text store indexing & query
    struct TextStore : ITextStore
    {
        // Attributes
        QuerySharedState m_queryState;
        Mutex            m_sentinel;
        String           m_path;
        bool             m_create;
        long             m_docsPerOpt;
        long             m_docsIndexed;
        long             m_maxFieldLength;
        long             m_maxMergeDocs;
        long             m_mergeFactor;
        Char**           m_stopWordsArray;
        StringVector     m_stopWords;
        lucene::index::IndexWriter*                   m_writer;
        lucene::analysis::standard::StandardAnalyzer* m_analyzer;
        TextStore() : 
            m_create(false), 
            m_docsPerOpt(-1L), 
            m_docsIndexed(0L),
            m_maxFieldLength(-1L), 
            m_maxMergeDocs(-1L), 
            m_mergeFactor(-1L),
            m_stopWordsArray(NULL),
            m_writer(NULL),
            m_analyzer(NULL)
        {}
        ~TextStore() 
        { 
            Mutex::Lock lock(m_sentinel);
            if (m_writer != NULL)         _CLDELETE(m_writer);
            if (m_analyzer != NULL)       _CLDELETE(m_analyzer);
            if (m_stopWordsArray != NULL) delete [] m_stopWordsArray;
        }

        // init: initialize text store
        bool init(const Properties& config)
        {
            Mutex::Lock lock(m_sentinel);
            Log::Scope scope(KCC_FILE, "init");
            
            if (!m_path.empty())
            {
                Log::error("text store already initialized");
                return false;
            }

            // init
            String path(config.get(k_keyPath, k_defPath));
            Platform::File f;
            if (!Platform::fsFile(path, f) || !f.dir)
            {
                Log::error("index repository path not found: path=[%s]", path.c_str());
                return false;
            }
            m_path           = path;
            m_create         = config.get(k_keyCreate,         k_defCreate) == KCC_PROPERTY_TRUE;
            m_docsPerOpt     = config.get(k_keyDocsPerOpt,     k_defDocsPerOpt);
            m_maxFieldLength = config.get(k_keyMaxFieldLength, k_defMaxFieldLength);
            m_maxMergeDocs   = config.get(k_keyMaxMergeDocs,   k_defMaxMergeDocs);
            m_mergeFactor    = config.get(k_keyMergeFactor,    k_defMergeFactor);
            Log::info2(
                "TextStore initialized: clucene=[%s] path=[%s] create=[%d] docs/optimize=[%d] maxFieldLength=[%d] maxMergeDocs=[%d] mergeFactor=[%d]", 
                KCC_CLUCENE_VERSION, m_path.c_str(), m_create, m_docsPerOpt, m_maxFieldLength, m_maxMergeDocs, m_mergeFactor);
            
            // analyzer
            String stopWords(config.get(k_keyStopWords, k_defStopWords));
            Strings::csv(stopWords, m_stopWords);
            m_stopWordsArray = new Char* [m_stopWords.size() + 1]; // +1 for EOF
            for (StringVector::size_type i = 0; i < m_stopWords.size(); i++) m_stopWordsArray[i] = (Char*)m_stopWords[i].c_str();
            m_stopWordsArray[m_stopWords.size()] = NULL; // EOF
            Log::info3("stop words=[%d:%s]", m_stopWords.size(), stopWords.c_str());
            m_analyzer = _CLNEW lucene::analysis::standard::StandardAnalyzer(m_stopWordsArray);
            
            // shared query state
            m_queryState.init(path, m_analyzer);

            return true;
        }

        // indexOpen: open index
        void indexOpen() throw (TextException)
        {
            Mutex::Lock lock(m_sentinel);
            Log::Scope scope(KCC_FILE, "indexOpen");
            if (m_writer != NULL) throw TextException("index already open");
            try
            {
                if (m_create) Platform::fsDirCreateEmpty(m_path);
                m_writer = _CLNEW lucene::index::IndexWriter(m_path.c_str(), m_analyzer, m_create);
                if (m_maxFieldLength > 0L) m_writer->setMaxFieldLength(m_maxFieldLength);
                if (m_maxMergeDocs > 0L)   m_writer->setMaxMergeDocs(m_maxMergeDocs);
                if (m_mergeFactor > 0L)    m_writer->setMergeFactor(m_mergeFactor);
                Log::info3("index opened: created=[%s] path=[%s]", (m_create ? "yes" : "no"), m_path.c_str());
                m_create = false;
            }
            catch (CLuceneError& e)
            {
                throw TextException(e.what());
            }
        }
        
        // indexRepository: get index repository
        const String& indexRepository() { return m_path; }

        // indexDocouments: get number of documents
        long indexDocuments() throw (TextException)
        {
            Mutex::Lock lock(m_sentinel);
            Log::Scope scope(KCC_FILE, "indexDocuments");
            long count = 0L;
            try
            {
                if (m_writer != NULL)
                {
                    count = m_writer->docCount();
                }
                else
                {
                    QuerySharedState::Finder finder(m_queryState);
                    count = finder.searcher()->getReader()->numDocs();
                }
            }
            catch (CLuceneError& e)
            {
                throw TextException(e.what());
            }
            return count;
        }
        
        // indexText: add text to index
        void indexText(TextDocument& txtdoc, TextDocument::Contents contents) throw (TextException)
        {
            static const TextDocument::MetadataFields k_emptyFields;
            indexText(txtdoc, k_emptyFields, contents);
        }
        
        // indexText: add text to index
        void indexText(TextDocument& txtdoc, const TextDocument::MetadataFields& fields, TextDocument::Contents contents) throw (TextException)
        {
            Mutex::Lock lock(m_sentinel);
            Log::Scope scope(KCC_FILE, "indexText");
            if (m_writer == NULL) throw TextException("index not open");
            lucene::document::Document* doc = NULL;
            try
            {
                doc = _CLNEW lucene::document::Document();

                // metadata: always stored & indexed; no term counts; tokenized if not an enumeration
                if ((contents & TextDocument::C_METADATA) != 0)
                {
                    if (fields.empty())
                    {
                        // all fields tokenized
                        for (StringMap::const_iterator i = txtdoc.metadata.begin(); i != txtdoc.metadata.end(); i++)
                        {
                            doc->add(
                                *new lucene::document::Field(
                                    i->first.c_str(), 
                                    i->second.c_str(), 
                                    true, true, true, false));
                        }
                    }
                    else
                    {
                        // per field tokenization
                        TextDocument::MetadataFields::const_iterator notFound = fields.end();
                        for (StringMap::const_iterator i = txtdoc.metadata.begin(); i != txtdoc.metadata.end(); i++)
                        {
                            const String& key = i->first;
                            bool tokenize = true;
                            TextDocument::MetadataFields::const_iterator md = fields.find(key);
                            if (md != notFound && md->second == TextDocument::MD_ENUM) tokenize = false;
                            doc->add(
                                *new lucene::document::Field(
                                    key.c_str(), 
                                    i->second.c_str(), 
                                    true, true, tokenize, false));
                        }
                    }
                }

                // text: always tokenized & indexed; text stored if specified, terms counts stored if specified
                bool text = (contents & TextDocument::C_TEXT)  != 0;
                bool tfv  = (contents & TextDocument::C_TERMS) != 0;
                doc->add(*new lucene::document::Field(k_text.c_str(), txtdoc.text.c_str(), text, true, true, tfv));

                // index document
                m_writer->addDocument(doc);
                _CLDELETE(doc);
                doc = NULL;

                // auto-optimize on boundaries
                m_docsIndexed++;
                if (m_docsPerOpt != -1L && m_docsIndexed % m_docsPerOpt == 0L) indexOptimize();
            }
            catch (CLuceneError& e)
            {
                if (doc != NULL) _CLDELETE(doc);
                throw TextException(e.what());
            }
        }
        
        // indexIsOpen: is index open
        bool indexIsOpen() throw (TextException) 
        { 
            Mutex::Lock lock(m_sentinel);
            bool o = m_writer != NULL;
            return o; 
        }

        // indexOptimize: optimize index
        void indexOptimize() throw (TextException)
        {
            Mutex::Lock lock(m_sentinel);
            Log::Scope scope(KCC_FILE, "indexOptimize");
            if (m_writer == NULL) throw TextException("index not open");
            try
            {
                m_writer->optimize();
                Log::info3("index optimized: documents=[%d]", m_writer->docCount());
            }
            catch (CLuceneError& e)
            {
                throw TextException(e.what());
            }
        }

        // indexClose: close index
        void indexClose() throw (TextException)
        {
            Mutex::Lock lock(m_sentinel);
            Log::Scope scope(KCC_FILE, "indexClose");
            if (m_writer == NULL) throw TextException("index not open");
            try
            {
                m_writer->close();
                _CLDELETE(m_writer);
                m_writer = NULL;
                Log::info3("index closed");
            }
            catch (CLuceneError& e)
            {
                _CLDELETE(m_writer);
                m_writer = NULL;
                throw TextException(e.what());
            }
        }

        // query: execute text query
        ITextResults* query(const String& expression, TextDocument::Contents contents) throw (TextException) 
        {
            Log::Scope scope(KCC_FILE, "query");
            m_queryState.prepare();
            AutoPtr<TextResults> tr(new TextResults(m_queryState, contents)); 
            tr->begin(expression);
            return tr.release();
        }
    };
    
    //
    // TextStore factory
    //

    KCC_COMPONENT_FACTORY_IMPL(TextStore)
    
    KCC_COMPONENT_FACTORY_METADATA_BEGIN_COMPONENT(TextStore, ITextStore)
        KCC_COMPONENT_FACTORY_METADATA_PROPERTY(KCC_COMPONENT_FACTORY_SCM, KCC_VERSION)
    KCC_COMPONENT_FACTORY_METADATA_END
}
