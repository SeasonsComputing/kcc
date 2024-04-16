/*
 * Kuumba C++ Core
 *
 * $Id: IXMLRepository.h 21779 2007-12-27 00:56:26Z tvk $
 */
#ifndef IXMLRepository_h
#define IXMLRepository_h

#include <inc/xml/IXMLSerializable.h>

namespace kcc
{
    /** XML Repository Exception */
    KCC_COMPONENT_EXCEPTION(XMLRepositoryException);

    /**
     * IXMLRepositoryFinder - (GOF: Visitor) interface for repository finder callbacks
     *
     * @author Ted V. Kremer
     */
    interface IXMLRepositoryFinder : IComponent
    {
        virtual void onFind(const IDOMNode* node, DOMReader& rdr) = 0;
    };

    /**
     * IXMLRepository - Repository services for storing & finding XML documents.
     *
     * An instance of IXMLRespository is analagous to folder/collection of XML documents.
     *
     * @author Ted V. Kremer
     */
    interface IXMLRepository : IComponent
    {
        /**
         * Initialize XML repository instance
         *
         * @param config configuration
         * @return true if initialized successfully
         */
        virtual bool init(const Properties& config) = 0;

        /** 
         * Replace a document in the repository (inserts if doesn't exist)
         * @param doc XML serializable document to replace
         * @param docName name of document (will add .xml extension if not present)
         * @throws XMLRepositoryException if error
         */
        virtual void replace(IXMLSerializable* doc, const String& docName) throw (XMLRepositoryException) = 0;

        /** 
         * Replace a named document in the repository (inserts if doesn't exist)
         * @param doc xml document to insert
         * @param docName name of document (will add .xml extension if not present)
         * @throws XMLRepositoryException if error
         */
        virtual void replace(const IDOMNode* doc, const String& docName) throw (XMLRepositoryException) = 0;

        /** 
         * Replace a named document in the repository (inserts if doesn't exist)
         * @param doc xml document to insert
         * @param docName name of document (will add .xml extension if not present)
         * @throws XMLRepositoryException if error
         */
        virtual void replace(const String& doc, const String& docName) throw (XMLRepositoryException) = 0;
        
        /** 
         * Find a collection of document in the repository
         * @param finder finder used to notify of found XML documents
         * @param query find expression
         * @throws XMLRepositoryException if error
         */
        virtual void find(IXMLRepositoryFinder* finder, const String& query) throw (XMLRepositoryException) = 0;
        
        /**
         * Find document from repository; returns node
         * @param query XQuery/XPath query to find
         * @return root dom node of xml document (ownership IS consumed)
         * @throws XMLRepositoryException if error
         */
        virtual IDOMNode* find(const String& query) throw (XMLRepositoryException) = 0;
        
        /** 
         * Find document from repository; returns node
         * @param doc where to place document content
         * @param query find expression
         * @return true if found
         * @throws XMLRepositoryException if error
         */
        virtual bool find(String& doc, const String& query) throw (XMLRepositoryException) = 0;
        
        /** 
         * Remove a document from the repository. To remove the entire collection pass an empty string.
         * @param docName name of document (will add .xml extension if not present)
         * @throws XMLRepositoryException if error
         */
        virtual void remove(const String& docName) throw (XMLRepositoryException) = 0;

        /**
         * Retrieve count of objects returned by query
         * @param query find expression
         * @return count
         * @throws XMLRepositoryException if error
         */
        virtual long count(const String& query) throw (XMLRepositoryException) = 0;
        
        /**
         * Get resource from repository; returns node
         * @param docName name of document resource to get
         * @return root dom node of xml document (ownership IS consumed)
         * @throws XMLRepositoryException if error
         */
        virtual IDOMNode* resource(const String& docName) throw (XMLRepositoryException) = 0;
        
        /** 
         * Get resource from repository
         * @param doc where to place document content
         * @param docName name of document resource to get
         * @return true if found
         * @throws XMLRepositoryException if error
         */
        virtual bool resource(String& doc, const String& docName) throw (XMLRepositoryException) = 0;
        
        /**
         * Retrieve list of resources for this repository
         * @param files (out-param) list of repository files
         *              Note: not all File attributes will be populated. See component provider for which
         * @param clear clear files list
         * @throws XMLRepositoryException if error
         */
        virtual void resources(Platform::Files& files, bool clear = true) throw (XMLRepositoryException) = 0;
        
        /**
         * Open a child repository
         * @param name name of child repository to open/create
         * @return repository instance for child repository (ownership IS consumed)
         * @throws XMLRepositoryException if error
         */
        virtual IXMLRepository* child(const String& name) throw (XMLRepositoryException) = 0;
    };
}

#endif // IXMLRepository
