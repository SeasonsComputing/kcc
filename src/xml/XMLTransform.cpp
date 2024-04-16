/*
 * Kuumba C++ Core
 *
 * $Id: XMLTransform.cpp 22625 2008-03-09 22:51:49Z tvk $
 */
#include <inc/core/Core.h>
#include <inc/xml/IXMLTransform.h>

namespace libxml
{
    #include "libxslt/transform.h"
    #include "libxslt/xsltutils.h"
    #include "libexslt/exslt.h"
};

#define KCC_FILE    "XMLTransform"
#define KCC_VERSION "$Id: XMLTransform.cpp 22625 2008-03-09 22:51:49Z tvk $"

namespace kcc
{
    // Configuration 
    static const String k_keyCache ("XMLTransform.cache");
    static const long   k_defCache = KCC_PROPERTY_TRUE;
    
    // Constants
    const int SZ = 1024*4;

    // Helper to manage libxml/libxslt init & clean-up
    static struct LibXmlClean
    { 
        LibXmlClean()
        {
            libxml::xmlInitThreads();
            libxml::xmlSubstituteEntitiesDefault(1);
            *libxml::__xmlLoadExtDtdDefaultValue() = 1;
            libxml::exsltRegisterAll();
        }
        ~LibXmlClean()
        {
            libxml::xsltCleanupGlobals();
            libxml::xmlCleanupParser();
            libxml::xmlCleanupThreads();
        }
    } k_clean;

    // Helper to manage xslt params
    struct XSLTParams
    {
        char** m_params;
        XSLTParams(const StringMap& xsltParams) : m_params(NULL)
        {
            m_params = new char* [xsltParams.size() * 2 + 1];  // +1 for null array terminator
            int p = 0;
            for (StringMap::const_iterator i = xsltParams.begin(); i != xsltParams.end(); i++)
            {
                // key
                m_params[p] = new char[i->first.size()+1];  // +1 for null
                std::strcpy(m_params[p], i->first.c_str());
                p++;

                // value
                String value = "'" + i->second + "'";   // encode
                m_params[p] = new char[value.size()+1]; // +1 for null
                std::strcpy(m_params[p], value.c_str());
                p++;
            }
            m_params[p] = NULL;
        }
        ~XSLTParams()
        {
            int p = 0;
            while (m_params[p] != NULL) delete [] m_params[p++];
            delete [] m_params;
        }
        const char** params() { return (const char**) m_params; }
    };

    // Implementation of XSLTApply
    struct XSLTApply : IXSLTApply
    {
        // Attributes
        Mutex                     m_sentinel; // libxml/xslt are not thread safe
        libxml::xmlDocPtr         m_xml;
        libxml::xsltStylesheetPtr m_xslt;
        XSLTApply() : m_xml(NULL), m_xslt(NULL) {}
        ~XSLTApply()
        {
            Mutex::Lock lock(m_sentinel);
            if (m_xml  != NULL) libxml::xmlFreeDoc(m_xml);
            if (m_xslt != NULL) libxml::xsltFreeStylesheet(m_xslt);
        }

        // loadXml: load xml from istream
        void loadXml(std::istream& xml) throw (IXMLTransform::TransformException)
        {
            Mutex::Lock lock(m_sentinel);
            Log::Scope scope(KCC_FILE, "XMLApply::loadXml");
            if (m_xml != NULL) libxml::xmlFreeDoc(m_xml);
            xml.seekg(0, std::ios::end);
            int sz = (int)xml.tellg();
            xml.seekg(0, std::ios::beg);
            kcc::String data;
            data.reserve(sz);
            char buf[SZ+1];
            while (xml.good() && !xml.eof())
            {
                xml.read(buf, SZ);
                buf[xml.gcount()] = 0;
                data += buf;
            }
            xml.seekg(0, std::ios::beg);
            m_xml = libxml::xmlParseMemory(data.c_str(), data.size());
            if (m_xml == NULL) throw IXMLTransform::TransformException(libxml::xmlGetLastError()->message);
        }

        // loadXml: load xml from path
        void loadXml(const String& xmlPath) throw (IXMLTransform::TransformException)
        {
            Mutex::Lock lock(m_sentinel);
            Log::Scope scope(KCC_FILE, "XMLApply::loadXml");
            if (m_xml != NULL) libxml::xmlFreeDoc(m_xml);
            m_xml = libxml::xmlParseFile(xmlPath.c_str());
            if (m_xml == NULL) throw IXMLTransform::TransformException(libxml::xmlGetLastError()->message);
        }
        
        // loadXslt: load xslt from istream
        void loadXslt(std::istream& xslt) throw (IXMLTransform::TransformException)
        {
            Mutex::Lock lock(m_sentinel);
            Log::Scope scope(KCC_FILE, "XMLApply::loadXslt");
            if (m_xslt != NULL) libxml::xsltFreeStylesheet(m_xslt);
            xslt.seekg(0, std::ios::end);
            int sz = (int)xslt.tellg();
            xslt.seekg(0, std::ios::beg);
            kcc::String data;
            data.reserve(sz);
            char buf[SZ+1];
            while (xslt.good() && !xslt.eof())
            {
                xslt.read(buf, SZ);
                buf[xslt.gcount()] = 0;
                data += buf;
            }
            xslt.seekg(0, std::ios::beg);
            libxml::xmlDocPtr xsltDoc = libxml::xmlParseMemory(data.c_str(), data.size());
            if (xsltDoc == NULL) throw IXMLTransform::TransformException(libxml::xmlGetLastError()->message);
            m_xslt = libxml::xsltParseStylesheetDoc(xsltDoc);
            libxml::xmlFreeDoc(xsltDoc);
            if (m_xslt == NULL) throw IXMLTransform::TransformException(libxml::xmlGetLastError()->message);
        }
        
        // loadXslt: load xslt from path
        void loadXslt(const String& xsltPath) throw (IXMLTransform::TransformException)
        {
            Mutex::Lock lock(m_sentinel);
            Log::Scope scope(KCC_FILE, "XMLApply::loadXslt");
            if (m_xslt != NULL) libxml::xsltFreeStylesheet(m_xslt);
            m_xslt = libxml::xsltParseStylesheetFile((const libxml::xmlChar *)xsltPath.c_str());
            if (m_xslt == NULL) throw IXMLTransform::TransformException(libxml::xmlGetLastError()->message);
        }

        // apply: apply transformation
        void apply(const StringMap& params, std::ostream& out) throw (IXMLTransform::TransformException)
        {
            Mutex::Lock lock(m_sentinel);
            Log::Scope scope(KCC_FILE, "XMLApply::apply");
            if (m_xml  == NULL) throw IXMLTransform::TransformException("xml document not loaded: must call loadXml() before apply()");
            if (m_xslt == NULL) throw IXMLTransform::TransformException("xslt document not loaded: must call loadXslt() before apply()");
            XSLTParams xp(params);
            libxml::xmlDocPtr res = libxml::xsltApplyStylesheet(m_xslt, m_xml, xp.params());
            if (res == NULL) throw IXMLTransform::TransformException(libxml::xmlGetLastError()->message);
            libxml::xmlChar* dump = NULL;
            int sz = 0;
            if (libxml::xsltSaveResultToString(&dump, &sz, res, m_xslt) < 0) 
            {
                libxml::xmlFreeDoc(res);
                throw IXMLTransform::TransformException(libxml::xmlGetLastError()->message);
            }
            if (sz > 0 && dump != NULL)
            {
                out << dump;
                out.flush();
                libxml::xmlFree(dump);
            }
            libxml::xmlFreeDoc(res);
        }
    };

    // Helper structure to manager compiled stylesheets
    struct CompiledStylesheetValue
    {
        // Attributes
        std::time_t modified;
        XSLTApply   apply;
        CompiledStylesheetValue(const String& xsltPath) 
            throw (IXMLTransform::TransformException)
        {
            Platform::File xslt;
            if (!Platform::fsFile(xsltPath, xslt)) throw IXMLTransform::TransformException("xslt not found: path=[" + xsltPath + "]");
            modified = xslt.modified;
            apply.loadXslt(xsltPath);
        }
        
        // xform: transform xml path
        void xform(const String& xmlPath, const StringMap& params, std::ostream& out) 
            throw (IXMLTransform::TransformException)
        {
            Mutex::Lock lock(apply.m_sentinel);
            apply.loadXml(xmlPath);
            apply.apply(params, out);
        }

        // xform: transform xml stream
        void xform(std::istream& xml, const StringMap& params, std::ostream& out) 
            throw (IXMLTransform::TransformException)
        {
            Mutex::Lock lock(apply.m_sentinel);
            apply.loadXml(xml);
            apply.apply(params, out);
        }
    };
    typedef SharedPtr<CompiledStylesheetValue>   CompiledStylesheet;
    typedef std::map<String, CompiledStylesheet> Cache;

    // Implementation of XML transformation
    struct XMLTransform : IXMLTransform
    {
        
        // Attributes
        Mutex m_sentinel;
        bool  m_useCache;
        Cache m_cache;
        XMLTransform() : m_useCache(true) {}
        
        // init: init component
        bool init(const Properties& config) 
        {
            Log::Scope scope(KCC_FILE, "init");
            m_useCache = config.get(k_keyCache, k_defCache) == KCC_PROPERTY_TRUE;
            Log::info2("XMLTransform init: cache=[%d]", m_useCache);
            return true; 
        }
        
        // apply: apply transformation
        void apply(const String& xmlPath, const String& xsltPath, const StringMap& params, std::ostream& out) 
            throw (IXMLTransform::TransformException)
        {
            Log::Scope scope(KCC_FILE, "apply");
            stylesheet(xsltPath)->xform(xmlPath, params, out);
        }

        // apply: apply transformation
        void apply(std::istream& xml, const String& xsltPath, const StringMap& params, std::ostream& out) 
            throw (IXMLTransform::TransformException)
        {
            Log::Scope scope(KCC_FILE, "apply");
            stylesheet(xsltPath)->xform(xml, params, out);
        }
        
        // stylesheet: get style sheet for xslt from cache or create and cache
        CompiledStylesheet stylesheet(const String& xsltPath)
            throw (IXMLTransform::TransformException)
        {
            Log::Scope scope(KCC_FILE, "stylesheet");
            CompiledStylesheet ss;
            if (m_useCache)
            {
                Mutex::Lock lock(m_sentinel);
                Cache::iterator find = m_cache.find(xsltPath);
                if (find != m_cache.end()) 
                {
                    Platform::File xslt;
                    if (!Platform::fsFile(xsltPath, xslt)) throw IXMLTransform::TransformException("xslt not found: path=[" + xsltPath + "]");
                    if (xslt.modified == find->second->modified)
                    {
                        Log::info4("using cached compiled stylesheet: xslt=[%s]", xsltPath.c_str());
                        ss = find->second;
                    }
                    else
                    {
                        Log::info4("compiled stylesheet updated, recaching: xslt=[%s]", xsltPath.c_str());
                    }
                }
                if (ss == NULL)                       
                {
                    Log::info4("caching compiled stylesheet: xslt=[%s]", xsltPath.c_str());
                    ss = new CompiledStylesheetValue(xsltPath);
                    m_cache[xsltPath] = ss;
                }
            }
            else
            {
                Log::info4("uncached compiled stylesheet: xslt=[%s]", xsltPath.c_str());
                ss = new CompiledStylesheetValue(xsltPath);
            }
            return ss;
        }
    };
   
    // 
    // XMLTransform factory
    //
    
    struct XMLTransformFactory : IXMLTransformFactory
    {
        IComponent*    construct()          { return constructTransform(); }
        IXMLTransform* constructTransform() { return new XMLTransform(); }
        IXSLTApply*    constructApply()     { return new XSLTApply(); }
    };

    KCC_COMPONENT_FACTORY_CUST(XMLTransformFactory)
    
    KCC_COMPONENT_FACTORY_METADATA_BEGIN_CUSTOM(XMLTransformFactory, IXMLTransformFactory, XMLTransform, IXMLTransform)
        KCC_COMPONENT_FACTORY_METADATA_PROPERTY(KCC_COMPONENT_FACTORY_SCM, KCC_VERSION)
    KCC_COMPONENT_FACTORY_METADATA_END
}
