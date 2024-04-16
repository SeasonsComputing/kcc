/*
 * Kuumba C++ Core
 *
 * $Id: IPageResponse.h 21714 2007-12-13 03:30:00Z tvk $
 */
#ifndef IPageResponse_h
#define IPageResponse_h

#include <inc/inet/IHTTP.h>
#include <inc/xml/IXMLTransform.h>

namespace kcc
{
    interface IPageResponse;
    interface IPageReader;
    interface IPageWriter;

    /**
     * Page handler
     *
     * @author Ted V. Kremer
     */
    interface IPageHandler : IComponent
    {
        /**
         * Page response handler (GOF: Template)
         *
         * @param response response that triggered page
         * @see IHTTPResponse
         */
        virtual void onHandle(const HTTPRequest& request, IPageReader* in, IPageWriter* out, IPageResponse* response) = 0;
    };

    /**
     * Page multi-part form reader
     *
     * @author Ted V. Kremer
     */
    interface IPageFormReader : IComponent
    {
        /** Form Part */
        struct Part
        {
            String     contentType;
            String     contentDisposition;
            Dictionary parameters;
        };
    
        /** 
         * Get first/next form part
         * @param part form header data
         * @return true if part is available
         * @throws Socket::Failed exception if error
         */
        virtual bool next(Part& part) throw (Socket::Failed) = 0;

        /**
         * Read binary data from form
         * @param buf buffer to read into
         * @param sz size of buffer
         * @param actual actual bytes read
         */
        virtual bool read(char* buf, int sz, int& actual) throw (Socket::Failed) = 0;

        /**
         * Read text data from form (form must have text content, consumer MUST validate Part content type PRIOR to calling)
         * @param data string to read into
         * @throws Socket::Failed exception if error
         */
        virtual void read(String& data) throw (Socket::Failed) = 0;
        
        /**
         * Read to end of part
         * @throws Socket::Failed exception if error
         */
        virtual void end() throw (Socket::Failed) = 0;
    };

    /**
     * Page reader
     *
     * @see IHTTPRequestReader
     * @author Ted V. Kremer
     */
    interface IPageReader : IHTTPRequestReader
    {
        /**
         * Begin reading multi-part form
         * @param attributes http request attributes
         * @return form reader (ownership IS consumed)
         */
        virtual IPageFormReader* beginForm(const Dictionary& attributes) throw (Socket::Failed) = 0;
    };

    /**
     * Page writer
     *
     * @see IHTTPResponseWriter
     * @author Ted V. Kremer
     */
    interface IPageWriter : IHTTPResponseWriter
    {
        /** Transformation Type */
        enum TransformType { TT_HTML, TT_XML, TT_TEXT };

        /**
         * Write transformed xml (client-caching disabled)
         * @param xml xml to transform
         * @param xform transformation path or id
         *              xform is searched in xform map first, otherwise it's a path (file OR url)
         * @param xformParams paraemeters to pass to style sheet
         * @param type type of transformation
         * @param headers response headers
         * @throw IXMLTransform::TransformException if error transforming
         * @throws Socket::Failed exception if error
         */
        virtual void transform(
            std::istream& xml, 
            const String& xform, 
            const StringMap& xformParams, 
            TransformType type,
            const Dictionary& headers) throw (IXMLTransform::TransformException, Socket::Failed) = 0;
        virtual void transform(
            const String& xml, 
            const String& xform, 
            const StringMap& xformParams, 
            TransformType type,
            const Dictionary& headers) throw (IXMLTransform::TransformException, Socket::Failed) = 0;

        /**
         * Stream resource to writer
         * @param path path to resource
         * @param attributes request attributes
         * @throws Socket::Failed exception if error
         */
        virtual void resource(const String& path, const Dictionary& attributes) throw (Socket::Failed) = 0;
    };

    /**
     * Page handler wrapper for response handler (GOF: Adapter)
     *
     * Ownership of response delegate IS consumed.
     *
     * @see IHTTPResponse, IPageHandler
     * @author Ted V. Kremer
     */    
    template<class ResponseHandlerT> struct PageResponseWrapper : IPageHandler
    {
        // Attributes
        ResponseHandlerT* handler;
        PageResponseWrapper(ResponseHandlerT* rh = NULL) : handler(rh) {}
        virtual ~PageResponseWrapper() { if (handler != NULL) delete handler; }
        
        // onHandle: delegate page response to HTTP response
        void onHandle(const HTTPRequest& r, IPageReader* in, IPageWriter* out, IPageResponse*) { handler->onResponse(r, in, out);  }
        
        // Utility
        inline ResponseHandlerT* operator->() { return handler; }
    };

    /**  
     * HTTPResponse provider to stream resource to browser
     *
     * @see IHTTPResponse
     * @author Ted V. Kremer
     */
    interface IPageResponse : IHTTPResponse
    {
        /** Data structure to map request cmd to handler: URI request -> handler */
        typedef std::map<String, IPageHandler*> Handlers;

        /**
         * Initialize response
         * @param config component parameters
         * @param handlers page handlers
         * @param xforms transformations: id -> path (otherwise xform assumes path)
         * @return true if initialized successfully
         */
        virtual bool init(const Properties& config, const Handlers& handlers,  const StringMap& xforms) = 0;
        virtual bool init(const Properties& config, const Handlers& handlers) = 0;

        /**
         * Get full path relative to appPath
         * @param path path to qualify
         * @return full path
         */
        virtual String fullPath(const String& path) = 0;
    };
}

#endif // IPageResponse_h
