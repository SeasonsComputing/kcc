/*
 * Kuumba C++ Core
 *
 * $Id: DOM.cpp 22995 2008-04-03 14:01:11Z tvk $
 */
#include <inc/core/Core.h>
#include "DOM.h"

#define KCC_FILE "DOM"

namespace kcc
{
    //
    // NodeFactory implementation
    //

    static const String k_domDocument("#document");
    static const String k_domComment ("#comment");
    static const String k_domText    ("#text");
    static const String k_domCDATA   ("#cdata-section");
    DOMNode* DOMNodeFactory::constructDocumentNode() { return new DOMNode(IDOMNode::DOCUMENT_NODE, k_domDocument);   }
    DOMNode* DOMNodeFactory::constructCommentNode()  { return new DOMNode(IDOMNode::COMMENT_NODE,  k_domComment);    }
    DOMNode* DOMNodeFactory::constructTextNode()     { return new DOMNode(IDOMNode::TEXT_NODE,     k_domText);       }
    DOMNode* DOMNodeFactory::constructCDATANode()    { return new DOMNode(IDOMNode::CDATA_SECTION_NODE, k_domCDATA); }

    //
    // DOMNode implementation
    //

    // ctor/dtor
    DOMNode::DOMNode(IDOMNode::Type type, const String& name, const String& value) :
        m_index(-1), m_type(type), m_name(name), m_value(value), m_parent(NULL)
    {}

    // DOMNode - accessors/modifiers
    const String& DOMNode::getNodeName() const          { return m_name; }
    const String& DOMNode::getNodeValue() const         { return m_value; }
    IDOMNode::Type DOMNode::getNodeType() const         { return m_type; }
    const IDOMNode* DOMNode::getParentNode() const      { return m_parent; }
    const IDOMNodeList* DOMNode::getChildNodes() const  { return &m_children; }
    const IDOMNode* DOMNode::getFirstChild() const      { return m_children.getItem(0); }
    const IDOMNode* DOMNode::getLastChild() const       { return m_children.getItem(m_children.getLength() - 1); }
    const IDOMNode* DOMNode::getPreviousSibling() const { return m_parent->m_children.getItem(m_index - 1); }
    const IDOMNode* DOMNode::getNextSibling() const     { return m_parent->m_children.getItem(m_index + 1); }
    const IDOMNamedNodeMap* DOMNode::getAttributes() const { return &m_attributes; }
    int DOMNode::getIndex() const                       { return m_index; }
    bool DOMNode::hasChildNodes() const                 { return m_children.getLength() > 0; }
    bool DOMNode::hasAttributes() const                 { return m_attributes.getLength() > 0; }
    DOMNodeList&     DOMNode::children()                { return m_children; }
    DOMNamedNodeMap& DOMNode::attributes()              { return m_attributes; }
    String& DOMNode::name()                             { return m_name; }
    String& DOMNode::value()                            { return m_value; }
    int& DOMNode::index()                               { return m_index; }
    IDOMNode::Type& DOMNode::type()                     { return m_type; }
    DOMNode*& DOMNode::parent()                         { return m_parent; }
    
    // findNodesByTagName: find nodes, shallow search, ownership consumeed
    IDOMNodeList* DOMNode::findNodesByTagName(const String& tagName) const
    {
        DOMNodeList* nodes = new DOMNodeList(false); // don't clean children
        long sz = m_children.getLength();
        for (long i = 0; i < sz; i++)
        {
            DOMNode* n = m_children.getItemImpl(i);
            if (n->getNodeName() == tagName) nodes->addImpl(n);
        }
        return nodes;
    }

    // addAttributeImpl: add dom attribute node
    void DOMNode::addAttributeImpl(DOMNode* node)
    {
        node->m_index = m_attributes.getLength();
        m_attributes.addImpl(node);
    }

    // addNodeImpl: add dom node
    void DOMNode::addNodeImpl(DOMNode* node)
    {
        node->m_parent = this;
        node->m_index = m_children.getLength();
        m_children.addImpl(node);
    }

    //
    // DOMNodeList implementation
    //

    // ctor/dtor
    DOMNodeList::DOMNodeList(bool clean) : m_clean(clean) {}
    DOMNodeList::~DOMNodeList()
    {
        if (m_clean)
        {
            for (DOMNodes::iterator i = m_list.begin(); i != m_list.end(); i++)
                delete *i;
        }
    }

    // DOMNodeList - accessors/modifiers
    long DOMNodeList::getLength() const                    { return (long)m_list.size(); }
    DOMNode* DOMNodeList::getItemImpl(long index) const    { return (DOMNode*)getItem(index); }
    void DOMNodeList::addImpl(DOMNode* n)                  { m_list.push_back(n); }
    const IDOMNode* DOMNodeList::getItem(long index) const { return (index < 0 || index >= getLength()) ? NULL : m_list[index]; }

    // removeImpl: remove a node
    DOMNode* DOMNodeList::removeImpl(long index)
    {
        DOMNode* node = getItemImpl(index);
        if (node != NULL)
        {
            node->parent() = NULL;
            node->index() = -1;
            m_list.erase(m_list.begin() + index);
        }
        return node;
    }

    //
    // DOMNamedNodeMap implementation
    //

    // ctor/dtor
    DOMNamedNodeMap::DOMNamedNodeMap(bool clean) : m_clean(clean) {}
    DOMNamedNodeMap::~DOMNamedNodeMap()
    {
        if (m_clean)
        {
            for (DOMNodes::iterator i = m_list.begin(); i != m_list.end(); i++)
                delete *i;
        }
    }

    // DOMNamedNodeMap - accessors/modifiers
    long DOMNamedNodeMap::getLength() const                    { return (long)m_map.size(); }
    DOMNode* DOMNamedNodeMap::getItemImpl(long index) const    { return (DOMNode*)getItem(index); }
    DOMNode* DOMNamedNodeMap::getNamedItemImpl(const String& name) const { return (DOMNode*)getNamedItem(name); }
    const IDOMNode* DOMNamedNodeMap::getItem(long index) const { return (index < 0 || index >= getLength()) ? NULL : m_list[index]; }

    // getNamedItem: accessor to named node
    const IDOMNode* DOMNamedNodeMap::getNamedItem(const String& name) const
    {
        DOMAttributes::const_iterator i = m_map.find(name);
        return (i == m_map.end()) ? NULL : i->second;
    }

    // removeImpl: remove a node
    DOMNode* DOMNamedNodeMap::removeImpl(long index)
    {
        DOMNode* node = getItemImpl(index);
        if (node != NULL)
        {
            node->parent() = NULL;
            node->index() = -1;
            m_list.erase(m_list.begin() + index);
            m_map.erase(node->getNodeName());
        }
        return node;
    }

    // addImpl: add named node
    void DOMNamedNodeMap::addImpl(DOMNode* n) { m_list.push_back(n); m_map[n->getNodeName()] = n; }
}
