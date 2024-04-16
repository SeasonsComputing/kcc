/*
 * Kuumba C++ Core
 *
 * $Id: DOM.h 22995 2008-04-03 14:01:11Z tvk $
 */
#ifndef DOM_h
#define DOM_h

namespace kcc
{
    class DOMNode;

    /** DOM Collections */
    typedef std::vector<DOMNode*>      DOMNodes;
    typedef std::map<String, DOMNode*> DOMAttributes;

    /** DOM Node factory */
    class DOMNodeFactory
    {
    public:
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
    class DOMNodeList : public IDOMNodeList
    {
    public:
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
    class DOMNamedNodeMap : public IDOMNamedNodeMap
    {
    public:
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
    class DOMNode : public IDOMNode
    {
    public:
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
        Type                    getNodeType       () const;
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

#endif // DOM_h
