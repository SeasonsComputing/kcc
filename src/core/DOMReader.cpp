/*
 * Kuumba C++ Core
 *
 * $Id: DOMReader.cpp 21696 2007-12-07 17:19:31Z tvk $
 */
#include <inc/core/Core.h>

#define KCC_FILE "DOMReader"

namespace kcc
{
    // Constants
    static const size_t k_szBuf = 1024;
    static const String k_empty;

    //
    // DOMReader Implementation
    //

    // ctor/dtor
    DOMReader::DOMReader(const IDOMNode* root) : m_root(root) {}
    DOMReader::~DOMReader() {}

    // root: accessor to DOM root
    const IDOMNode* DOMReader::root() { return m_root; }

    // query: query DOM node using search expression 
    const IDOMNode* DOMReader::nodePathQuery(const String& path, const IDOMNode* node)
    {
        const IDOMNode* start = (node == NULL) ? m_root : node;

        // get tags
        StringVector tags;
        Strings::tokenize(path, Platform::fsSep(), tags);
        if (tags.size() == 0) return NULL;

        // navigate path
        const IDOMNode* find = NULL;
        StringVector::size_type sz = tags.size();
        for (StringVector::size_type i = 0; i < sz; i++)
        {
            find = nodeOp(start, tags.at(i));
            if (find == NULL) return NULL;
            start = find;
        }

        return find;
    }

    // doc: query doc node
    const IDOMNode* DOMReader::doc(const String& tagDoc)
        throw (DOMReader::NotFoundException)
    {
        Log::Scope scope(KCC_FILE, "doc");
        return node(m_root, tagDoc);
    }

    // node: query node
    const IDOMNode* DOMReader::node(const IDOMNode* root, const String& tag)
        throw (DOMReader::NotFoundException)
    {
        Log::Scope scope(KCC_FILE, "node");
        if (root == NULL) throw DOMReader::NotFoundException("DOMNode is null");
        AutoPtr<IDOMNodeList> nodes(root->findNodesByTagName(tag));
        if (nodes->getLength() != 1) throw DOMReader::NotFoundException(tag);
        return nodes->getItem(0L);
    }

    // nodeOp: query optional node
    const IDOMNode* DOMReader::nodeOp(const IDOMNode* root, const String& tag)
        throw (DOMReader::NotFoundException)
    {
        Log::Scope scope(KCC_FILE, "nodeOp");
        if (root == NULL) throw DOMReader::NotFoundException("DOMNode is null");
        AutoPtr<IDOMNodeList> nodes(root->findNodesByTagName(tag));
        return (nodes->getLength() != 1) ? NULL : nodes->getItem(0L);
    }

    // nodeTx: query node text
    String DOMReader::nodeTx(const IDOMNode* root, const String& tag)
        throw (DOMReader::NotFoundException)
    {
        return text(node(root, tag));
    }

    // nodes: retrieve nodes from root
    IDOMNodeList* DOMReader::nodes(const IDOMNode* root, const String& tagNode)
    {
        Log::Scope scope(KCC_FILE, "nodes");
        if (root == NULL) throw DOMReader::NotFoundException("DOMNode is null");
        return root->findNodesByTagName(tagNode);
    }

    // nodes: retrieve nodes from root
    IDOMNodeList* DOMReader::nodes(const String& tagDoc, const String& tagNode)
        throw (DOMReader::NotFoundException)
    {
        Log::Scope scope(KCC_FILE, "nodes");
        return nodes(doc(tagDoc), tagNode);
    }

    // cdata: retrieve CDATA from child node
    String DOMReader::cdata(const IDOMNode* node)
    {
        Log::Scope scope(KCC_FILE, "cdata");
        if (node == NULL) throw DOMReader::NotFoundException("DOMNode is null");
        const IDOMNode* txt = node->getFirstChild();
        return (txt == NULL || txt->getNodeType() != IDOMNode::CDATA_SECTION_NODE) ? k_empty : txt->getNodeValue();
    }

    // text: retrieve text from child node: TEXT or CDATA
    String DOMReader::text(const IDOMNode* node)
    {
        Log::Scope scope(KCC_FILE, "text");
        if (node == NULL) throw DOMReader::NotFoundException("DOMNode is null");
        const IDOMNode* txt = node->getFirstChild();
        if (txt == NULL)
            return k_empty;                                     // no text
        else if (txt->getNodeType() == IDOMNode::TEXT_NODE)
            return Strings::xmlDecode(txt->getNodeValue());     // decode text
        else if (txt->getNodeType() == IDOMNode::CDATA_SECTION_NODE)
            return txt->getNodeValue();                         // cdata already encoded
        else
            throw DOMReader::NotFoundException("DOMNode not text");
        return k_empty;                                     
    }

    // comment: retrieve comment from child node
    String DOMReader::comment(const IDOMNode* node)
    {
        Log::Scope scope(KCC_FILE, "comment");
        if (node == NULL) throw DOMReader::NotFoundException("DOMNode is null");
        const IDOMNode* txt = node->getFirstChild();
        return (txt == NULL || txt->getNodeType() != IDOMNode::COMMENT_NODE) ? k_empty : txt->getNodeValue();
    }

    // attr: retrieve text for attr node
    void attr(const IDOMNode* node, const String& tagAttr, String& value) 
        throw (DOMReader::NotFoundException)
    {
        Log::Scope scope(KCC_FILE, "attr");
        if (node == NULL) throw DOMReader::NotFoundException("DOMNode is null");
        value.clear();
        const IDOMNode* txt = node->getAttributes()->getNamedItem(tagAttr);
        if (txt == NULL) throw DOMReader::NotFoundException(tagAttr);
        value = Strings::xmlDecode(txt->getNodeValue());
    }

    // attr: retrieve text for attr node
    String DOMReader::attr(const IDOMNode* node, const String& tagAttr)
        throw (DOMReader::NotFoundException)
    {
        Log::Scope scope(KCC_FILE, "attr");
        if (node == NULL) throw DOMReader::NotFoundException("DOMNode is null");
        const IDOMNode* txt = node->getAttributes()->getNamedItem(tagAttr);
        if (txt == NULL) throw DOMReader::NotFoundException(tagAttr);
        return Strings::xmlDecode(txt->getNodeValue());
    }

    // attrOp: retrieve text optional for attr node
    bool DOMReader::attrOp(const IDOMNode* node, const String& tagAttr, String& value)
    {
        Log::Scope scope(KCC_FILE, "attrOp");
        if (node == NULL) throw DOMReader::NotFoundException("DOMNode is null");
        value.clear();
        const IDOMNode* txt = node->getAttributes()->getNamedItem(tagAttr);
        if (txt == NULL) return false;
        value = Strings::xmlDecode(txt->getNodeValue());
        return true;
    }

    //
    // DOMFileReader Implementation
    //

    // ctor/dtor
    DOMFileReader::DOMFileReader() : DOMReader(NULL) {}
    DOMFileReader::~DOMFileReader() { delete m_root; }

    // load: load xml from path
    bool DOMFileReader::load(const String& path) 
    { 
        String xml;
        bool ok = Strings::loadText(path, xml);
        if (ok) m_root = Core::rodom()->parseXML(xml);
        return ok;
    }

    //
    // DOMURLReader Implementation
    //

    DOMURLReader::DOMURLReader() : DOMReader(NULL) {}
    DOMURLReader::~DOMURLReader() { delete m_root; }
    
    // load: load xml from url
    bool DOMURLReader::load(const URL& url)
    {
        String xml;
        bool ok = HTTP::getxml(url, xml) == HTTP::C_OK;
        if (ok) m_root = Core::rodom()->parseXML(xml);
        return ok;
    }
}
