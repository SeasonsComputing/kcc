/*
 * Kuumba C++ Core
 *
 * $Id: ExistRepository.cpp 20962 2007-10-06 18:27:12Z tvk $
 */
#include <inc/core/Core.h>
#include <inc/store/IXMLRepository.h>

#define KCC_FILE    "ExistRepository"
#define KCC_VERSION "$Id: ExistRepository.cpp 20962 2007-10-06 18:27:12Z tvk $"

namespace kcc
{
    // Properties
    static const String k_keyRepositoryBaseURI("ExistRepository.baseURI");
    static const String k_keyRepositoryPath   ("ExistRepository.path");

    // Constants
    static const String k_existResult    ("exist:result");
    static const String k_existCount     ("exist:count");
    static const String k_existHits      ("exist:hits");
    static const String k_existValue     ("exist:value");
    static const String k_existCollection("exist:collection");
    static const String k_existResource  ("exist:resource");
    static const String k_existDir       ("exist:result/exist:collection");
    static const String k_existName      ("name");
    static const String k_existCreated   ("created");
    static const String k_existModified  ("last-modified");
    static const String k_existQueryCount("count(%s)");
    static const String k_existQuery     ("_query=");
    static const String k_existNext      ("&_start=%d&_howmany=%d");
    static const String k_existExt       (".xml");
    static const String k_existPathSep   ("/");
    
    // k_docName: helper to append doc ext if missing
    inline String k_docName(const String& docName)
    {
        static const String k_rx_invalid(":"), k_rx_replace("_");
        String n(docName);
        Core::regex()->replace(n, k_rx_invalid, k_rx_replace);
        if (n.find(k_existExt) == String::npos)
            return n + k_existExt;
        else
            return n;
    }
    
    // ExistRepository component provider
    struct ExistRepository : IXMLRepository
    {
        // Attributes
        String m_uri;
        String m_path;
        String m_fullPath;

        // init: initialize repository path. path is absolute and begins and end with '/'
        bool init(const Properties& config)
        {
            Log::Scope scope(KCC_FILE, "init");
            
            String uri(config.get(k_keyRepositoryBaseURI, Strings::empty()));
            if (uri.empty())
            {
                Log::error("repository baseURI not specified");
                return false;
            }
            // remove trailing '/' (if missing)
            if (uri.substr(uri.size() - 1) == k_existPathSep) uri = uri.substr(0, uri.size() - 1);
            
            String path(config.get(k_keyRepositoryPath, Strings::empty()));
            if (path.empty())
            {
                Log::error("repository path not specified");
                return false;
            }
            // add leading & trailing '/' (if missing)
            if (path.substr(0,1) != k_existPathSep) path = k_existPathSep + path;
            if (path.substr(path.size() - 1) != k_existPathSep) path += k_existPathSep;
            
            // init
            m_uri      = uri;
            m_path     = path;
            m_fullPath = m_uri + m_path;
            Log::info2("ExistRepository initialized: path=[%s]", m_fullPath.c_str());
            return true;
        }

        // replace: replace xml resource
        void replace(IXMLSerializable* d, const String& n) throw (XMLRepositoryException)
        {
            Log::Scope scope(KCC_FILE, "replace");
            try
            {
                d->validate();
                StringStream xml;
                DOMWriter w(xml); 
                d->toXML(w);
                replace(xml.str(), n);
            }
            catch (Exception& e)
            {
                throw XMLRepositoryException(e.what());
            }
        }

        // replace: replace xml resource
        void replace(const IDOMNode* d, const String& n) throw (XMLRepositoryException)
        {
            Log::Scope scope(KCC_FILE, "replace");
            try
            {
                StringStream xml;
                DOMWriter w(xml); 
                w.node(d);
                replace(xml.str(), n);
            }
            catch (Exception& e)
            {
                throw XMLRepositoryException(e.what());
            }
        }

        // replace: replace xml resource
        void replace(const String& d, const String& n) throw (XMLRepositoryException)
        {
            Log::Scope scope(KCC_FILE, "replace");
            if (n.empty()) throw XMLRepositoryException("empty name param");
            URL uput(m_fullPath);
            uput.path += k_docName(n);
            int code = HTTP::C_ERROR;
            try
            {
                code = HTTP::putxml(uput, d);
                if (code != HTTP::C_OK) throw Exception("xmldb replace failed");
            }
            catch (Exception& e)
            {
                Log::error("xmldb put error: code=[%d] url=[%s]", code, uput.fullPath().c_str());
                throw XMLRepositoryException(e.what());
            }
        }

        // find: find all xml resources
        void find(IXMLRepositoryFinder* f, const String& q) throw (XMLRepositoryException)
        {
            Log::Scope scope(KCC_FILE, "find");
            try
            {
                long start = 1L, hits = 0L, max = 256L; // TODO: add Properties params to method, make max a param
                AutoPtr<IDOMNode> root;
                long count = next(q, root, start, max, hits);
                bool more = hits > 0;
                while (more)
                {
                    DOMReader rdr(root);
                    const IDOMNodeList* nodes = rdr.doc(k_existResult)->getChildNodes();
                    long sz = nodes->getLength();
                    for (long i = 0; i < sz; i++) f->onFind(nodes->getItem(i), rdr);
                    start += count;
                    more = start <= hits;
                    if (more) count = next(q, root, start, max, hits);
                }
            }
            catch (Exception& e)
            {
                throw XMLRepositoryException(e.what());
            }
        }
        
        // find: find xml resource
        IDOMNode* find(const String& q) throw (XMLRepositoryException)
        {
            Log::Scope scope(KCC_FILE, "find");
            IDOMNode* root = NULL;
            try
            {
                String doc;
                if (!find(doc, q)) throw Exception("xmldb get failed");
                root = Core::rodom()->parseXML(doc);
                if (root == NULL) throw Exception("xmldb get failed. unable to parse returned xml.");
            }
            catch (Exception& e)
            {
                throw XMLRepositoryException(e.what());
            }
            return root;
        }
        
        // find: find xml resource
        bool find(String& doc, const String& q) throw (XMLRepositoryException)
        {
            Log::Scope scope(KCC_FILE, "find");
            URL uget(m_fullPath);
            uget.query = k_existQuery + q;
            int code = HTTP::C_ERROR;
            try
            {
                code = HTTP::getxml(uget, doc);
                if (code != HTTP::C_OK && code != HTTP::C_NOT_FOUND) throw Exception("xmldb get failed");
            }
            catch (Exception& e)
            {
                Log::error("xmldb get error: code=[%d] url=[%s]", code, uget.fullPath().c_str());
                throw XMLRepositoryException(e.what());
            }
            return code == HTTP::C_OK;
        }

        // remove: remove xml respource
        void remove(const String& n) throw (XMLRepositoryException)
        {
            Log::Scope scope(KCC_FILE, "remove");
            if (n.empty()) throw XMLRepositoryException("empty name param");
            URL udel(m_fullPath);
            udel.path += k_docName(n);
            int code = HTTP::C_ERROR;
            try
            {
                code = HTTP::deletexml(udel);
                if (code != HTTP::C_OK) throw Exception("xmldb remove failed");
            }
            catch (Exception& e)
            {
                Log::error("xmldb delete error: code=[%d] url=[%s]", code, udel.fullPath().c_str());
                throw XMLRepositoryException(e.what());
            }
        }
        
        // count: query resource count
        long count(const String& q) throw (XMLRepositoryException) 
        {
            Log::Scope scope(KCC_FILE, "count");
            long count = 0L;
            try
            {
                AutoPtr<IDOMNode> root(find(Strings::printf(k_existQueryCount.c_str(), q.c_str())));
                DOMReader r(root);
                const IDOMNode* value = r.node(r.doc(k_existResult), k_existValue);
                count = Strings::parseInteger(r.text(value));
            }
            catch (Exception& e)
            {
                throw XMLRepositoryException(e.what());
            }
            return count;
        }

        // next: get next xml resource (impl. so let exception propagate)
        long next(const String& q, AutoPtr<IDOMNode>& root, long start, long max, long& hits) 
            throw (XMLRepositoryException, DOMReader::NotFoundException)
        {
            Log::Scope scope(KCC_FILE, "next");
            long count = 0L;
            root = find(q + Strings::printf(k_existNext.c_str(), start, max));
            DOMReader r(root);
            const IDOMNode* result = r.doc(k_existResult);
            hits  = Strings::parseInteger(r.attr(result, k_existHits));
            count = Strings::parseInteger(r.attr(result, k_existCount)); 
            return count;
        }

        // resource: get resource from repository; returns node
        IDOMNode* resource(const String& docName) throw (XMLRepositoryException)
        {
            Log::Scope scope(KCC_FILE, "resource");
            IDOMNode* root = NULL;
            try
            {
                String doc;
                if (!resource(doc, docName)) throw Exception("xmldb get failed");
                root = Core::rodom()->parseXML(doc);
                if (root == NULL) throw Exception("xmldb get failed. unable to parse returned xml.");
            }
            catch (Exception& e)
            {
                throw XMLRepositoryException(e.what());
            }
            return root;
        }
        
        // resource: get resource from repository
        bool resource(String& doc, const String& docName) throw (XMLRepositoryException)
        {
            Log::Scope scope(KCC_FILE, "resource");
            URL uget(m_fullPath);
            uget.path += docName;
            int code = HTTP::C_ERROR;
            try
            {
                code = HTTP::getxml(uget, doc);
                if (code != HTTP::C_OK && code != HTTP::C_NOT_FOUND) throw Exception("xmldb get failed");
            }
            catch (Exception& e)
            {
                Log::error("xmldb get error: code=[%d] url=[%s]", code, uget.fullPath().c_str());
                throw XMLRepositoryException(e.what());
            }
            return code == HTTP::C_OK;
        }
        
        // resources: list resources
        void resources(Platform::Files& files, bool clear) throw (XMLRepositoryException)
        {
            Log::Scope scope(KCC_FILE, "resources");
            if (clear) files.clear();
            try
            {
                // fetch list & parse into DOM
                String doc;
                int code = HTTP::getxml(m_fullPath, doc);
                if (code != HTTP::C_OK) throw Exception("xmldb get failed");
                AutoPtr<IDOMNode> root(Core::rodom()->parseXML(doc));
                
                // map resource & collection to File object
                DOMReader r(root);
                const IDOMNode* dir = r.nodePathQuery(k_existDir);
                if (dir == NULL) throw Exception("path not a repository: path=[" + m_path + "]");
                long sz = dir->getChildNodes()->getLength();
                for (long i = 0L; i < sz; i++)
                {
                    const IDOMNode* n = dir->getChildNodes()->getItem(i);
                    
                    // resource
                    Platform::File res;
                    res.name    = r.attr(n, k_existName);
                    res.created = res.modified = ISODate::iso(r.attr(n, k_existCreated));
                    
                    if (n->getNodeName() == k_existCollection)      // collection == dir
                        res.dir = true;
                    else if (n->getNodeName() == k_existResource)   // resource has modified date
                        res.modified = ISODate::iso(r.attr(n, k_existModified));
                    else
                        throw Exception("unrecognized resource type: " + n->getNodeName());
                        
                    files.push_back(res);
                }
            }
            catch (Exception& e)
            {
                throw XMLRepositoryException(e.what());
            }
        }
        
        // child: open repository
        IXMLRepository* child(const String& name) throw (XMLRepositoryException)
        {
            Log::Scope scope(KCC_FILE, "child");
            AutoPtr<IXMLRepository> childRepository(new ExistRepository);
            Properties config;
            config.set(k_keyRepositoryBaseURI, m_uri);
            config.set(k_keyRepositoryPath,    m_path + name);
            if (!childRepository->init(config)) throw XMLRepositoryException("failed to open child");
            return childRepository.release();
        }
    };
    
    //
    // ExistRepository factory
    //

    KCC_COMPONENT_FACTORY_IMPL(ExistRepository)
    
    KCC_COMPONENT_FACTORY_METADATA_BEGIN_COMPONENT(ExistRepository, IXMLRepository)
        KCC_COMPONENT_FACTORY_METADATA_PROPERTY(KCC_COMPONENT_FACTORY_SCM, KCC_VERSION)
    KCC_COMPONENT_FACTORY_METADATA_END
}
