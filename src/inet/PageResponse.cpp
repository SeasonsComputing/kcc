/*
 * Kuumba C++ Core
 *
 * $Id: PageResponse.cpp 22776 2008-03-24 20:36:12Z tvk $
 */
#include <inc/core/Core.h>
#include <inc/inet/IPageResponse.h>

#define KCC_FILE    "PageResponse"
#define KCC_VERSION "$Id: PageResponse.cpp 22776 2008-03-24 20:36:12Z tvk $"

namespace kcc
{
    // Configuration
    static const String k_keyXformComponent("PageResponse.xformComponent");
    static const String k_keyAppPath       ("PageResponse.appPath");
    static const String k_defXformComponent("k_transform");

    // Constants
    static const std::size_t k_sz = 1024;
    static const String k_html("text/html");
    static const String k_xml ("text/xml");
    static const String k_text("text/plain");
    static const String k_mimes[][2] = 
    {
        { ".png",   "image/png"             },
        { ".ico",   "image/ico"             },
        { ".jpg",   "image/jpeg"            },
        { ".jpeg",  "image/jpeg"            },
        { ".gif",   "image/gif"             },
        { ".css",   "text/css"              },
        { ".wav",   "audio/wav"             },
        { ".js",    "text/javascript"       },
        { ".xhtml", "application/xhtml+xml" },
        { ".svg",   "image/svg+xml"         },
        { ".html",  k_html                  },
        { ".htm",   k_html                  },
        { ".xml",   k_xml                   },
        { ".xsl",   k_xml                   },
        { ".xslt",  k_xml                   },
        {}
    };
    static const String k_httpLastModified      ("Last-Modified");
    static const String k_httpIfModifiedSince   ("If-Modified-Since");
    static const String k_httpValueLength       ("length");
    static const String k_httpContentType       ("Content-Type");
    static const String k_httpContentLength     ("Content-Length");
    static const String k_httpContentDisposition("Content-Disposition");
    static const String k_httpContentTypeMPForm ("multipart/form-data");
    static const String k_httpMPFormBoundary    ("boundary=");
    static const String k_httpMPMarker          ("--");
    static const String k_httpMPFormTrailer     (k_httpMPMarker + "\r\n");

    //
    // Declarations
    //

    // Page response
    struct PageResponse : IPageResponse
    {
        // Attributes
        Handlers               m_handlers;
        StringMap              m_xforms;
        String                 m_appPath;
        AutoPtr<IXMLTransform> m_transform;

        // Implementation
        bool init(const Properties& config, const Handlers& handlers, const StringMap& xforms);
        bool init(const Properties& config, const Handlers& handlers);
        String fullPath(const String& path);
        void onResponse(const HTTPRequest& request, IHTTPRequestReader* in, IHTTPResponseWriter* out);
    };

    // Page reader
    struct PageReader : IPageReader
    {
        // Attributes
        PageResponse*       m_resp;
        IHTTPRequestReader* m_in;
        PageReader(PageResponse* resp, IHTTPRequestReader* in) : m_resp(resp), m_in(in) {}

        // Implementation
        void request(StringVector& headers, int& size)      throw (Socket::Failed);
        void content(const Dictionary& attrs, String& data) throw (Socket::Failed);
        void read(char* buf, int sz, int& actual)           throw (Socket::Failed);
        IPageFormReader* beginForm(const Dictionary& attributes) throw (Socket::Failed);
    };

    // Page form reader
    struct PageFormReader : IPageFormReader
    {
        // Attributes
        enum { szMarker = 64, szContent = 2048 };
        IHTTPRequestReader* m_in;
        bool                m_marker;
        char                m_markerBuf[szMarker];
        char                m_contentBuf[szContent];
        int                 m_markerOff;
        int                 m_contentOff;
        int                 m_size;
        int                 m_contentLength;
        String              m_boundary;
        int                 m_szBoundary;
        PageFormReader(IHTTPRequestReader* in) : 
            m_in(in), m_marker(false), m_markerOff(0), m_contentOff(0), m_size(0), m_contentLength(-1), m_szBoundary(0) 
        {}
        ~PageFormReader() { end(); }

        // Implementation
        void begin(const Dictionary& attrs)       throw (Socket::Failed);
        bool next(Part& part)                     throw (Socket::Failed);
        bool read(char* buf, int sz, int& actual) throw (Socket::Failed);
        void read(String& data)                   throw (Socket::Failed);
        void end()                                throw (Socket::Failed);
        bool readToBoundary(int max, int& total)  throw (Socket::Failed);
    };

    // Page writer
    struct PageWriter : IPageWriter
    {
        // Attributes
        PageResponse*        m_resp;
        IHTTPResponseWriter* m_out;
        PageWriter(PageResponse* resp, IHTTPResponseWriter* out) : m_resp(resp), m_out(out) {}

        // Implmentation
        void response(std::istream& in, const Dictionary& hdr, int resp) throw (Socket::Failed);
        void response(int resp)                                          throw (Socket::Failed);
        void response(const Dictionary& hdr, int len, int resp)          throw (Socket::Failed);
        void write(const char* buf, int sz)                              throw (Socket::Failed);
        void xml(std::istream& xml)                                      throw (Socket::Failed);
        void transform(
            std::istream& xml, 
            const String& xform, 
            const StringMap& params, 
            TransformType type, 
            const Dictionary& hdr) throw (IXMLTransform::TransformException, Socket::Failed);
        void transform(
            const String& xml, 
            const String& xform, 
            const StringMap& params, 
            TransformType type, 
            const Dictionary& hdr) throw (IXMLTransform::TransformException, Socket::Failed);
        void resource(const String& path, const Dictionary& attributes) throw (Socket::Failed);
    };

    //
    // PageResponse Implementation
    //

    // init: initialize page handler
    bool PageResponse::init(const Properties& config, const Handlers& handlers, const StringMap& xforms)
    {
        m_xforms = xforms;
        return init(config, handlers);
    }
    bool PageResponse::init(const Properties& config, const Handlers& handlers)
    {
        Log::Scope scope(KCC_FILE, "init");
        bool ok = true;
        try
        {
            m_handlers  = handlers;
            m_appPath   = Platform::fsNormalize(config.get(k_keyAppPath, Strings::empty()));
            m_transform = KCC_COMPONENT(IXMLTransform, config.get(k_keyXformComponent, k_defXformComponent));
            if (!m_transform->init(config)) return false;
            Log::info2("PageResponse initialized: appPath=[%s]", m_appPath.c_str());
        }
        catch (Exception& e)
        {
            Log::exception(e);
            ok = false;
        }
        return ok;
    }

    // fullPath: get appPath qualified name
    String PageResponse::fullPath(const String& path) { return Platform::fsFullPath(m_appPath, path); }

    // onResponse: handle response
    void PageResponse::onResponse(const HTTPRequest& request, IHTTPRequestReader* in, IHTTPResponseWriter* out)
    {
        Log::Scope scope(KCC_FILE, "onResponse");
        PageReader reader(this, in);
        PageWriter writer(this, out);
        Handlers::iterator page = m_handlers.find(request.path);
        if (page != m_handlers.end()) page->second->onHandle(request, &reader, &writer, this);
        else                          writer.resource(request.path.substr(1), request.attributes);
    }

    //
    // PageReader Implementation
    //

    // Delegates
    void PageReader::request(StringVector& headers, int& size)      throw (Socket::Failed) { m_in->request(headers, size); }
    void PageReader::content(const Dictionary& attrs, String& data) throw (Socket::Failed) { m_in->content(attrs, data); }
    void PageReader::read(char* buf, int sz, int& actual)           throw (Socket::Failed) { m_in->read(buf, sz, actual); }
    IPageFormReader* PageReader::beginForm(const Dictionary& attributes) throw (Socket::Failed) 
    {
        AutoPtr<PageFormReader> fr(new PageFormReader(this));
        fr->begin(attributes);
        return fr.release();
    }

    // begin: begin reading page form
    void PageFormReader::begin(const Dictionary& attrs) throw (Socket::Failed)
    {
        Log::Scope scope(KCC_FILE, "PageFormReader::begin");

        try
        {
            // validate type
            const String& ct = attrs[k_httpContentType];
            if (ct.find(k_httpContentTypeMPForm) == String::npos) Socket::Failed Exception("invalid content type: " + ct);
            m_contentLength = Strings::parseInteger(attrs[k_httpContentLength]);

            // boundart marker
            m_boundary.clear();
            String::size_type bsep = ct.find(k_httpMPFormBoundary);
            if (bsep == String::npos) throw Socket::Failed("missing boundary in multi-part form Content-Type");
            m_boundary = k_httpMPMarker + ct.substr(bsep + k_httpMPFormBoundary.size());
            m_szBoundary = (int)m_boundary.size();
            if (m_szBoundary > (int)szMarker) throw Socket::Failed("boundary marker size out of bounds");

            // read to first boundary
            int actual = 0;
            bool part = readToBoundary(szMarker, actual);
            if (!part || actual >= szMarker) throw Socket::Failed("boundary/marker mismatch");
        }
        catch (Socket::Failed&)
        {
            // invalidate iterator
            m_contentLength = -1;
            m_size          = 0;
            m_szBoundary    = 0;
            m_boundary.clear();
            throw;
        }
    }

    // next: read next part header
    bool PageFormReader::next(Part& part) throw (Socket::Failed)
    {
        Log::Scope scope(KCC_FILE, "PageFormReader::next");

        part.contentType.clear();
        part.contentDisposition.clear();
        part.parameters.clear();
        if (m_size >= m_contentLength) return false;
        
        // read boundary header
        bool next      = false;
        int  szTrailer = (int) k_httpMPFormTrailer.size();
        if (m_size < m_contentLength-szTrailer)
        {
            // boundary headers
            StringVector headers;
            int read = 0;
            m_in->request(headers, read);
            m_size += read;
            next = true;
            
            // parse into part features
            for (StringVector::size_type i = 0; i < headers.size(); i++)
            {
                String::size_type ct = headers[i].find(k_httpContentType);
                if (ct == 0)
                {
                    part.contentType = Strings::trim(headers[i].substr(k_httpContentType.size()+1));
                    continue;
                }
                String::size_type cd = headers[i].find(k_httpContentDisposition);
                if (cd == 0)
                {
                    if (
                        !HTTP::parseAttributeValue(
                            headers[i].substr(k_httpContentDisposition.size()+1), 
                            part.contentDisposition, 
                            part.parameters))
                    {
                        Log::warning("error parsing form params header");
                        next = false;
                        break;
                    }
                    continue;
                }
            }
        }
        else
        {
            // read trailer
            char trailer[szMarker];
            int actual = 0;
            m_in->read(trailer, szTrailer, actual);
            m_size += actual;
            bool ok = actual == szTrailer;
            if (ok)
            {
                trailer[szTrailer] = 0;
                ok = trailer == k_httpMPFormTrailer;
            }
            if (!ok) Log::warning("form trailer boundary not found");
        }
        
        // flush remaining content
        if (!next) end();
        
        return next;
    }
    
    // read: read into buf
    bool PageFormReader::read(char* buf, int sz, int& actual) throw (Socket::Failed)
    {
        Log::Scope scope(KCC_FILE, "PageFormReader::read");
        sz = std::min((int)szContent, sz); // maximum read buffer size == szContent
        actual = 0;
        bool part = false;
        if (m_size < m_contentLength)
        {
            part = readToBoundary(sz, actual);
            std::memcpy(buf, m_contentBuf, actual);
        }
        return m_size < m_contentLength && !part;
    }
    
    // read: read string content
    void PageFormReader::read(String& data) throw (Socket::Failed)
    {
        Log::Scope scope(KCC_FILE, "PageFormReader::read");
        data.clear();
        data.reserve(szContent);
        int actual = 0;
        bool part = false;
        while (m_size < m_contentLength && !part)
        {
            part = readToBoundary(szContent-1, actual); // -1 for null
            m_contentBuf[actual] = 0;
            data += m_contentBuf;
        }
    }
    
    // end: read to end of part
    void PageFormReader::end() throw (Socket::Failed)
    {
        Log::Scope scope(KCC_FILE, "PageFormReader::end");
        char ch     = 0;
        int  actual = 0;
        while (m_size < m_contentLength)
        {
            m_in->read(&ch, 1, actual);
            m_size += actual;
        }
    }

    // readToBoundary: read from stream until boundary located
    bool PageFormReader::readToBoundary(int max, int& total) throw (Socket::Failed)
    {
        total = 0;
        m_contentOff = 0;
        char ch     = 0;
        int  actual = 0;
        bool part   = false;
        while (total < max && m_size < m_contentLength && m_contentOff < szContent && !part)
        {
            m_in->read(&ch, 1, actual);
            m_size += actual;
            if (m_marker)
            {
                m_markerBuf[m_markerOff++] = ch;
                if (m_markerOff == m_szBoundary)
                {
                    if (std::memcmp(m_markerBuf, m_boundary.c_str(), m_szBoundary) == 0)
                    {
                        part        = true;
                        m_marker    = false;
                        m_markerOff = 0;
                    }
                    else                 
                    {
                        int markerSize = m_markerOff;
                        int overrun = 0;
                        if (total + m_markerOff > max) 
                        {
                            overrun = (total+m_markerOff) - max;
                            markerSize -= overrun;
                        }
                        std::memcpy(m_contentBuf+m_contentOff, m_markerBuf, markerSize);
                        m_contentOff += markerSize;
                        total        += markerSize;
                        if (overrun > 0)
                        {
                            std::memmove(m_markerBuf, m_markerBuf+markerSize, overrun);
                            m_markerOff = overrun;
                        }
                        else
                        {
                            m_marker    = false;
                            m_markerOff = 0;
                        }
                    }
                }
            }
            else
            {
                if (ch == '-')
                {
                    m_marker = true;
                    m_markerBuf[m_markerOff++] = ch;
                }
                else
                {
                    m_contentBuf[m_contentOff++] = ch;
                    total++;
                }
            }
        }
        return part;
    }

    //
    // Implemantion of page writer
    //

    // Delegates
    void PageWriter::response(std::istream& in, const Dictionary& hdr, int resp) throw (Socket::Failed) { m_out->response(in, hdr, resp); }
    void PageWriter::response(int resp)                                          throw (Socket::Failed) { m_out->response(resp); }
    void PageWriter::response(const Dictionary& hdr, int len, int resp)          throw (Socket::Failed) { m_out->response(hdr, len, resp); }
    void PageWriter::write   (const char* buf, int sz)                           throw (Socket::Failed) { m_out->write(buf, sz); }
    void PageWriter::xml     (std::istream& in)                                  throw (Socket::Failed) { m_out->xml(in); }

    // transform: transform xml using xform
    void PageWriter::transform(
        std::istream& xml, 
        const String& xform, 
        const StringMap& params, 
        TransformType type, 
        const Dictionary& headers) throw (IXMLTransform::TransformException, Socket::Failed)
    {
        Log::Scope scope(KCC_FILE, "PageWriter::transform");

        // check for xform in map first, then consider it a resource
        StringMap::iterator find = m_resp->m_xforms.find(xform);
        String fp = (find != m_resp->m_xforms.end()) ? m_resp->fullPath(find->second) : xform;

        // transform
        StringStream buf;
        m_resp->m_transform->apply(xml, fp, params, buf);
        
        // stream transformation
        String mime = k_html;
        if      (type == IPageWriter::TT_XML)  mime = k_xml;
        else if (type == IPageWriter::TT_TEXT) mime = k_text;
        Log::info4("streaming transformation: xform=[%s] mime=[%s] size=[%d]", fp.c_str(), mime.c_str(), (int)buf.tellp());
        Dictionary respHeaders(headers);
        HTTP::setHeaders(respHeaders, mime, true);
        m_out->response(buf, respHeaders);
    }

    // transform: transform xml using xform
    void PageWriter::transform(
        const String& xml, 
        const String& xform, 
        const StringMap& params, 
        TransformType type, 
        const Dictionary& headers) throw (IXMLTransform::TransformException, Socket::Failed)
    {
        Log::Scope scope(KCC_FILE, "PageWriter::transform");

        // check for xform in map first, then consider it a resource
        StringMap::iterator find = m_resp->m_xforms.find(xform);
        String fp = (find != m_resp->m_xforms.end()) ? m_resp->fullPath(find->second) : xform;

        // transform
        StringStream buf;
        m_resp->m_transform->apply(xml, fp, params, buf);
        
        // stream transformation
        String mime = k_html;
        if      (type == IPageWriter::TT_XML)  mime = k_xml;
        else if (type == IPageWriter::TT_TEXT) mime = k_text;
        Log::info4("streaming transformation: xml=[%s] xform=[%s] mime=[%s] size=[%d]", xml.c_str(), fp.c_str(), mime.c_str(), (int)buf.tellp());
        Dictionary respHeaders(headers);
        HTTP::setHeaders(respHeaders, mime, true);
        m_out->response(buf, respHeaders);
    }

    // resource: stream resource
    void PageWriter::resource(const String& path, const Dictionary& attributes) throw (Socket::Failed)
    {
        Log::Scope scope(KCC_FILE, "PageWriter::resource");

        // validate file exists
        String fp = m_resp->fullPath(path);
        Platform::File res;
        if (!Platform::fsFile(fp, res) || res.dir) 
        {
            m_out->response(HTTP::C_NOT_FOUND);
            return;
        }

        // modified
        String modified(ISODate::utc(res.modified).gmtdatetime());
        const String& sinceParam = attributes[k_httpIfModifiedSince];
        if (!sinceParam.empty())
        {
            String     since;
            Dictionary params;
            if (HTTP::parseAttributeValue(sinceParam, since, params))
            {
                unsigned long sinceSize = (unsigned long)Strings::parseInteger(params[k_httpValueLength]);
                if (modified == since && res.size == sinceSize)
                {
                    Log::info4(
                        "resource not modified: resource=[%s] resouce-size=[%ld] since=[%s] since-size=[%ld]", 
                        fp.c_str(), res.size, since.c_str(), sinceSize);
                    m_out->response(HTTP::C_NOT_MODIFIED);
                    return;
                }
            }
        }

        // mimes
        String mime(k_text);
        int i = 0;
        while (!k_mimes[i][0].empty())
        {
            String::size_type ext = k_mimes[i][0].size();
            String::size_type sz  = res.name.size();
            if (sz > ext && res.name.substr(sz - ext) == k_mimes[i][0])
            {
                mime = k_mimes[i][1];
                break;
            }
            i++;
        }
        
        // stream response
        Log::info4("streaming resource: path=[%s] mime=[%s] size=[%d] modified=[%s]", fp.c_str(), mime.c_str(), res.size, modified.c_str());
        Dictionary headers;
        headers(k_httpContentType)  = mime;
        headers(k_httpLastModified) = modified;
        response(headers, res.size, HTTP::C_OK);
        const std::size_t k_sz = 4096;
        char buf[k_sz];
        std::FILE* in = std::fopen(fp.c_str(), "rb");
        while (!feof(in))
        {
            int actual = std::fread(buf, 1, k_sz, in);
            m_out->write(buf, actual);
        }
        std::fclose(in);
    }
    
    //
    // PageResponse factory
    //

    KCC_COMPONENT_FACTORY_IMPL(PageResponse)
    
    KCC_COMPONENT_FACTORY_METADATA_BEGIN_COMPONENT(PageResponse, IPageResponse)
        KCC_COMPONENT_FACTORY_METADATA_PROPERTY(KCC_COMPONENT_FACTORY_SCM, KCC_VERSION)
    KCC_COMPONENT_FACTORY_METADATA_END
}
