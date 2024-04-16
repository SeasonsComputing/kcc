/*
 * Kuumba C++ Core
 *
 * $Id: DOMWriter.cpp 22887 2008-03-28 16:18:16Z tvk $
 */
#include <inc/core/Core.h>

#define KCC_FILE "DOMWriter"

namespace kcc
{
    // Contants
    static const String k_trueLabel  ("true");
    static const String k_falseLabel ("false");
    static const String k_trueScalar ("1");
    static const String k_falseScalar("0");
    
    // ctor/dtor
    DOMWriter::DOMWriter(std::ostream& out, bool noPrologue) : 
        m_out(out), m_open(false)
    { 
        if (!noPrologue) m_out << "<?xml version='1.0' encoding='UTF-8' ?>\n"; 
    }
    DOMWriter::~DOMWriter() {}

    // start: start an DOM element
    void DOMWriter::start(const String& tag)
    {
        if (m_open) m_out << ">\n";
        m_open = true;
        m_out << "<" << tag;
    }

    // end: end an DOM element
    void DOMWriter::end(const String& tag)
    {
        if (m_open) m_out << " />\n";
        else        m_out << "</" << tag <<  ">\n";
        m_open = false;
    }

    // cdata: write an DOM CDATA
    void DOMWriter::cdata(const String& value)
    {
        if (m_open) 
        {
            m_out << ">\n";
            m_open = false;
        }
        m_out << "<![CDATA[" << value << "]]>\n";
    }

    // text: write an DOM text
    void DOMWriter::text(const String& value, bool encode)
    {
        if (m_open) 
        {
            m_out << ">"; // no CR for text
            m_open = false;
        }
        m_out << (encode ? Strings::xmlEncode(value) : value);
    }

    // comment: Write an DOM comment
    void DOMWriter::comment(const String& value)
    {
        if (m_open) 
        {
            m_out << ">\n";
            m_open = false;
        }
        m_out << "<!-- " << value << " -->\n";
    }

    // attr: write an DOM attribute (element must have begun)
    void DOMWriter::attr(const String& tag, const String& value, bool encode) throw (DOMWriter::NotOpenException)
    {
        if (!m_open) throw DOMWriter::NotOpenException(tag);
        if (!value.empty()) m_out << " " << tag << "='" << (encode ? Strings::xmlEncode(value) : value) << "'";
    }
    void DOMWriter::attr(const String& tag, const Char* value, bool encode) throw (DOMWriter::NotOpenException)
    {
        if (!m_open) throw DOMWriter::NotOpenException(tag);
        if (value != NULL && *value != 0) m_out << " " << tag << "='" << (encode ? Strings::xmlEncode(value) : value) << "'";
    }
    void DOMWriter::attr(const String& tag, long value, const Char* f) throw (DOMWriter::NotOpenException) 
    { 
        attr(tag, Strings::printf(f, value)); 
    }
    void DOMWriter::attr(const String& tag, double value, const Char* f) throw (DOMWriter::NotOpenException) 
    { 
        attr(tag, Strings::printf(f, value)); 
    }
    void DOMWriter::attr(const String& tag, bool value, bool label) throw (DOMWriter::NotOpenException) 
    { 
        attr(tag, value ? (label ? k_trueLabel : k_falseLabel) : (label ? k_trueScalar : k_falseScalar)); 
    }
    void DOMWriter::attr(const String& tag, const ISODate& value, ISODate::Format f) throw (DOMWriter::NotOpenException) 
    { 
        attr(tag, value.isodatetime(f)); 
    }

    // write: serialize node to stream
    void DOMWriter::node(const IDOMNode* root)
    {
        if (m_open) 
        {
            m_out << ">\n";
            m_open = false;
        }
        dumpNode(root, m_out);
    }
    void DOMWriter::node(const String& root)
    {
        if (m_open) 
        {
            m_out << ">\n";
            m_open = false;
        }
        m_out << root;
    }

    // dumpNode: write node tree to stream (recursive)
    void DOMWriter::dumpNode(const IDOMNode* node, std::ostream& out)
    {
        // NOTE: we don't know if the node's text or attr's are encoded or not so we 
        //       pass through without encoding!
        if      (node->getNodeType() == IDOMNode::COMMENT_NODE)       comment(node->getNodeValue());
        else if (node->getNodeType() == IDOMNode::TEXT_NODE)          text(node->getNodeValue(), false);
        else if (node->getNodeType() == IDOMNode::CDATA_SECTION_NODE) cdata(node->getNodeValue());
        else
        {
            start(node->getNodeName());
            if (node->hasAttributes())
            {
                const IDOMNamedNodeMap* attrs = node->getAttributes();
                for (long i = 0; i < attrs->getLength(); i++)
                {
                    const IDOMNode* a = attrs->getItem(i);
                    attr(a->getNodeName(), a->getNodeValue(), false);
                }
            }
            if (node->hasChildNodes())
            {
                const IDOMNodeList* children = node->getChildNodes();
                long sz = children->getLength();
                for (long i = 0L; i < sz; i++) dumpNode(children->getItem(i), out);
            }
            end(node->getNodeName());            
        }
    }
}
