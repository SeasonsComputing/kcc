/*
 * Kuumba C++ Core
 *
 * $Id: IRODOM.h 22995 2008-04-03 14:01:11Z tvk $
 */
#ifndef IRODOM_h
#define IRODOM_h

namespace kcc
{
    interface IDOMNode;
    interface IDOMNodeList;
    interface IDOMNamedNodeMap;

    /**
     * Read-only DOM parser
     *
     * @author Ted V. Kremer
     */
    interface IRODOM : IComponent
    {
        /** Parse failed exception */
        KCC_COMPONENT_EXCEPTION(ParseFailed);

        /**
         * Construct DOM from XML input
         * @param xml input to parse
         * @return document root node or null if invalid or parse error (ownership IS consumed)
         */
        virtual IDOMNode* parseXML(const String& xml) throw (IRODOM::ParseFailed) = 0;

        /**
         * Construct DOM from HTML input
         * @param html input to parse
         * @return document root node or null if invalid or parse error (ownership IS consumed)
         */
        virtual IDOMNode* parseHTML(const String& html) throw (IRODOM::ParseFailed) = 0;
    };

    /*
     * Core DOM Interfaces (ownership not consumed, DO NOT DELETE)
     * Conforms to W3C Node, NodeList, NamedNodeMap
     */

    /** Read-only Node in DOM tree */
    interface IDOMNode : IComponent
    {
        /**
         * Node type
         * IRODOM providers may choose to support a subset of these type
         */
        enum Type
        {
            ELEMENT_NODE                = 1,
            ATTRIBUTE_NODE              = 2,
            TEXT_NODE                   = 3,
            CDATA_SECTION_NODE          = 4,
            ENTITY_REFERENCE_NODE       = 5,
            ENTITY_NODE                 = 6,
            PROCESSING_INSTRUCTION_NODE = 7,
            COMMENT_NODE                = 8,
            DOCUMENT_NODE               = 9,
            DOCUMENT_TYPE_NODE          = 10,
            DOCUMENT_FRAGMENT_NODE      = 11,
            NOTATION_NODE               = 12,
        };
        virtual const String&           getNodeName       () const = 0;
        virtual const String&           getNodeValue      () const = 0;
        virtual Type                    getNodeType       () const = 0;
        virtual const IDOMNode*         getParentNode     () const = 0;
        virtual const IDOMNodeList*     getChildNodes     () const = 0;
        virtual const IDOMNode*         getFirstChild     () const = 0;
        virtual const IDOMNode*         getLastChild      () const = 0;
        virtual const IDOMNode*         getPreviousSibling() const = 0;
        virtual const IDOMNode*         getNextSibling    () const = 0;
        virtual const IDOMNamedNodeMap* getAttributes     () const = 0;
        virtual int                     getIndex          () const = 0;
        virtual bool                    hasChildNodes     () const = 0;
        virtual bool                    hasAttributes     () const = 0;
        virtual IDOMNodeList*           findNodesByTagName(const String& tagName) const = 0; // shallow search, ownership IS consumed
    };

    /** List of DOM nodes */
    interface IDOMNodeList : IComponent
    {
        virtual const IDOMNode* getItem  (long index) const = 0;
        virtual long            getLength()           const = 0;
    };

    /** Map of DOM nodes */
    interface IDOMNamedNodeMap : IComponent
    {
        virtual const IDOMNode* getItem     (long index)         const = 0;
        virtual const IDOMNode* getNamedItem(const String& name) const = 0;
        virtual long            getLength   ()                   const = 0;
    };
}

#endif // IRODOM_h
