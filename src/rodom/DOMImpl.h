/*
 * Kuumba C++ Core
 *
 * $Id: DOMImpl.h 15403 2007-03-16 13:03:39Z tvk $
 */
#ifndef DOMImpl_h
#define DOMImpl_h

namespace kcc
{
    struct DOMNode;

    /** DOM Collections */
    typedef std::vector<DOMNode*>      DOMNodes;
    typedef std::map<String, DOMNode*> DOMAttributes;

    /** DOM Node factory */
    struct DOMNodeFactory
    {
        /**
        * Create node by types
        */
        static DOMNode* constructDocumentNode();
        static DOMNode* constructCommentNode ();
        static DOMNode* constructTextNode    ();
        static DOMNode* constructCDATANode   ();

    private:
        DOMNodeFactory();
    };

    /** DOM NodeList implementation */
    struct DOMNodeList : IDOMNodeList
    {
        // Modifiers
        DOMNodeList(bool clean = true);
        virtual ~DOMNodeList();
        virtual void     addImpl    (DOMNode* n);
        virtual DOMNode* getItemImpl(long index) const;
        virtual DOMNode* removeImpl (long index);

        // Accessors
        const IDOMNode* getItem(long index) const;
        long getLength() const;

    protected:
        // Attributes
        bool     m_clean;
        DOMNodes m_list;

    private:
        DOMNodeList(const DOMNodeList&);
        DOMNodeList& operator = (const DOMNodeList&);
    };

    /** DOM NamedNodeMap implementation */
    struct DOMNamedNodeMap : IDOMNamedNodeMap
    {
        // Modifiers
        DOMNamedNodeMap(bool clean = true);
        virtual ~DOMNamedNodeMap();
        virtual void     addImpl         (DOMNode* n);
        virtual DOMNode* getItemImpl     (long index) const;
        virtual DOMNode* getNamedItemImpl(const String& name) const;
        virtual DOMNode* removeImpl      (long index);

        // Accessors
        const IDOMNode* getItem     (long index) const;
        const IDOMNode* getNamedItem(const String& name) const;
        long            getLength   () const;

    protected:
        // Attributes
        bool          m_clean;
        DOMNodes      m_list;
        DOMAttributes m_map;

    private:
        DOMNamedNodeMap(const DOMNamedNodeMap&);
        DOMNamedNodeMap& operator = (const DOMNamedNodeMap&);
    };

    /** DOM Node implementation */
    struct DOMNode : IDOMNode
    {
        // Modifiers
        DOMNode(
            IDOMNode::Type type,
            const String& name = Strings::empty(),
            const String& value = Strings::empty());
        void             addAttributeImpl(DOMNode* node);
        void             addNodeImpl     (DOMNode* node);
        DOMNodeList&     children        ();
        DOMNamedNodeMap& attributes      ();
        String&          name            ();
        String&          value           ();
        int&             index           ();
        Type&            type            ();
        DOMNode*&        parent          ();

        // Accessors
        const String&           getNodeName       () const;
        const String&           getNodeValue      () const;
        const Type&             getNodeType       () const;
        const IDOMNode*         getParentNode     () const;
        const IDOMNodeList*     getChildNodes     () const;
        const IDOMNode*         getFirstChild     () const;
        const IDOMNode*         getLastChild      () const;
        const IDOMNode*         getPreviousSibling() const;
        const IDOMNode*         getNextSibling    () const;
        const IDOMNamedNodeMap* getAttributes     () const;
        int                     getIndex          () const;
        bool                    hasChildNodes     () const;
        bool                    hasAttributes     () const;
        IDOMNodeList*           findNodesByTagName(const String& tagName) const; // shallow search, ownership consumed

    protected:
        // Attributes
        int             m_index;
        IDOMNode::Type  m_type;
        String          m_name;
        String          m_value;
        DOMNode*        m_parent;
        DOMNodeList     m_children;
        DOMNamedNodeMap m_attributes;

    private:
        DOMNode(const DOMNode&);
        const DOMNode& operator = (const DOMNode&);
    };
}

#endif // DOMImpl_h
