/*
 * Kuumba C++ Core
 *
 * $Id: RODOM.cpp 21066 2007-10-15 15:58:24Z tvk $
 */
#include <inc/core/Core.h>
#include "DOM.h"

#define KCC_FILE    "RODOM"
#define KCC_VERSION "$Id: RODOM.cpp 21066 2007-10-15 15:58:24Z tvk $"

namespace kcc
{
    // Constants
    const int MAX_TAG = 512;
    static const Char*  k_domDOCUMENT  = "#document";
    static const Char*  k_domHTML      = "html";
    static const String k_domCDATA     ("[CDATA[");
    static const String k_htmlScript   ("script");
    static const String k_htmlScriptEnd("</script>");

    //
    // HTML element tags
    // elements that have forbidden end tags have no tags
    // listed for them in the rows below.
    //
    // elements that have optional end tags have a null-terminated list of
    // tags that close them, either explicitly (their own end tag) or
    // implicitly (a tag from some other element).
    //
    // elements that have required end tags simply list their single end
    // tag.  Yes, it's easy, given a start tag, to know what it's end tag
    // is (the same tag, but with a leading '/'); however, the code in
    // parse_html_tag() in html.c is made simpler by explicitly giving the
    // end tag here rather than having to construct a temporary string
    // prepending a '/' to the tag.
    //
    static const Char* F = "F";
    static const Char* O = "O";
    static const Char* R = "R";
    static const Char* k_endTags[] =
    {
        "#document",        R,    "/#document",
        "a",                R,    "/a",
        "abbr",             R,    "/abbr",
        "acronym",          R,    "/acronym",
        "address",          R,    "/address",
        "applet",           R,    "/applet",        // deprecated
        "area",             F,
        "b",                R,    "/b",
        "base",             F,
        "basefont",         F,                      // deprecated
        "bdo",              R,    "/bdo",
        "big",              R,    "/big",
        "blink",            R,    "/blink",         // nonstandard
        "blockquote",       R,    "/blockquote",
        "body",             O,    "/body", "/html", 0,
        "br",               F,
        "button",           R,    "/button",
        "caption",          R,    "/caption",
        "center",           R,    "/center",        // deprecated
        "cite",             R,    "/cite",
        "code",             R,    "/code",
        "col",              F,
        "colgroup",         O,
            "colgroup", "/colgroup",
            "tbody", "tfoot", "thead",
            "tr", "/table",
            0,
        "dd",               O,
            "dd", "/dd", "/dl", 
            "dt", "/dt", 
            0,
        "del",              R,    "/del",
        "dfn",              R,    "/dfn",
        "dir",              R,    "/dir",            // deprecated
        "div",              R,    "/div",
        "dl",               R,    "/dl",
        "dt",               O,    "dt", "/dt", "/dl", 0,
        "em",               R,    "/em",
        "embed",            R,    "/embed",         // nonstandard
        "fieldset",         R,    "/fieldset",
        "font",             R,    "/font",          // deprecated
        "form",             R,    "/form",
        "frame",            F,
        "frameset",         R,    "/frameset",
        "h",                R,    "/h",             // XHTML 2.0
        "h1",               R,    "/h1",
        "h2",               R,    "/h2",
        "h3",               R,    "/h3",
        "h4",               R,    "/h4",
        "h5",               R,    "/h5",
        "h6",               R,    "/h6",
        "head",             O,
            "body", "/body", "/head", "/html",
            0,
        "hr",               F,
        "html",             O,
            "/html",
            0,
        "i",                R,    "/i",
        "iframe",           R,    "/iframe",
        "ilayer",           R,    "/ilayer",        // nonstandard
        "img",              F,
        "input",            F,
        "ins",              R,    "/ins",
        "isindex",          F,                      // deprecated
        "kbd",              R,    "/kbd",
        "keygen",           F,                      // nonstandard
        "label",            R,    "/label",
        "layer",            R,    "/layer",         // nonstandard
        "legend",           R,    "/legend",
        "li",               O,
            "li", "/li",
            "/ol", "/ul",
            0,
        "line",             R,    "/line",          // XHTML 2.0
        "link",             F,
        "map",              R,    "/map",
        "menu",             R,    "/menu",          // deprecated
        "meta",             F,
        "multicol",         R,    "/multicol",      // nonstandard
        "name",             R,    "/name",          // XHTML 2.0
        "nl",               R,    "/nl",            // XHTML 2.0
        "nobr",             F,                      // nonstandard
        "noembed",          R,    "/noembed",       // nonstandard
        "noframes",         R,    "/noframes",
        "nolayer",          R,    "/nolayer",       // nonstandard
        "noscript",         R,    "/noscript",
        "object",           R,    "/object",
        "ol",               R,    "/ol",
        "optgroup",         R,    "/optgroup",
        "option",           O,
            "/optgroup",
            "option", "/option",
            "/select",
            0,
        "p",                O,
            "address", "/address",
            "applet", "/applet",
            "blockquote", "/blockquote",
            "/body",
            "br",
            "caption", "/caption",
            "center", "/center",
            "dd", "/dd",
            "div", "/div",
            "dl", "/dl",
            "dt", "/dt",
            "embed", "/embed",
            "form", "/form",
            "frame", "/frame",
            "frameset", "/frameset",
            "h1", "/h1",
            "h2", "/h2",
            "h3", "/h3",
            "h4", "/h4",
            "h5", "/h5",
            "h6", "/h6",
            "hr",
            "/html",
            "layer", "/layer",
            "li", "/li",
            "map", "/map",
            "multicol", "/multicol",
            "noembed", "/noembed",
            "noframes", "/noframes",
            "nolayer", "/nolayer",
            "noscript", "/noscript",
            "object", "/object",
            "ol", "/ol",
            "p", "/p",
            "plaintext",
            "pre", "/pre",
            "script", "/script",
            "select", "/select",
            "style", "/style",
            "table", "/table",
            "tbody", "/tbody"
            "td", "/td",
            "tfoot", "/tfoot"
            "th", "/th",
            "thead", "/thead"
            "tr", "/tr",
            "ul", "/ul",
            "xmp", "/xmp",
            0,
        "param",            F,
        "plaintext",        F,                      // deprecated
        "pre",              R,    "/pre",
        "q",                R,    "/q",             // deprecated
        "quote",            R,    "/quote",         // XHTML 2.0
        "ruby",             R,    "/ruby",          // ruby elements
        "rb",               R,    "/rb",
        "rbc",              R,    "/rbc",
        "rp",               R,    "/rp",
        "rt",               R,    "/rt",
        "rtc",              R,    "/rtc",
        "s",                R,    "/s",             // deprecated
        "samp",             R,    "/samp",
        "script",           R,    "/script",
        "section",          R,    "/section",       // XHTML 2.0
        "select",           R,    "/select",
        "server",           R,    "/server",        // nonstandard
        "small",            R,    "/small",
        "spacer",           F,                      // nonstandard
        "span",             R,    "/span",
        "strike",           R,    "/strike",        // deprecated
        "strong",           R,    "/strong",
        "style",            R,    "/style",
        "sub",              R,    "/sub",
        "sup",              R,    "/sup",
        "table",            R,    "/table",
        "tbody",            O,
            "tbody", "/tbody",
            0,
        "td",               O,
            "tbody", "/tbody",
            "td", "/td",
            "tfoot", "/tfoot",
            "th",
            "/table",
            "tr", "/tr",
            0,
        "textarea",         R,    "/textarea",
        "tfoot",            O,
            "tbody", "/tfoot", "thead",
            0,
        "th",               O,
            "tbody", "/tbody",
            "td",
            "tfoot", "/tfoot",
            "th", "/th",
            "/table",
            "tr", "/tr",
            0,
        "thead",            O,
            "tbody", "tfoot", "/thead",
            0,
        "title",            R,    "/title",
        "tr",               O,
            "tbody", "/tbody",
            "tfoot", "/tfoot",
            "/thead",
            "tr", "/tr",
            "/table",
            0,
        "tt",               R,    "/tt",
        "u",                R,    "/u",             // deprecated
        "ul",               R,    "/ul",
        "var",              R,    "/var",
        "wbr",              F,                      // nonstandard
        "xmp",              R,    "/xmp",           // deprecated
        "xml",              R,    "/xml",           // nonstandard
        0
    };

    // Helper to sort html map tag names
    struct HTMLTagLess : std::binary_function<const Char*, const Char*, bool>
    {
        bool operator() (const Char* a, const Char* b) const { return std::strcmp(a, b) < 0; }
    };

    // Helper class to manage possible end tags of an HTML element
    struct HTMLElement
    {
        // Attributes
        enum EndTagRule { R_FORBIDDEN, R_OPTIONAL, R_REQUIRED };
        typedef std::set<const Char*, HTMLTagLess> EndTags;
        EndTagRule  m_rule;
        const Char* m_tag;
        EndTags     m_endTags;
        HTMLElement(const Char* tag, EndTagRule r) : m_rule(r), m_tag(tag)  {}

        // Accessors
        EndTags& endTags()                    { return m_endTags; }
        EndTagRule rule() const               { return m_rule; }
        bool endTag(const Char* endTag) const { return m_endTags.find(endTag) != m_endTags.end(); }
    };

    // Helper class to map from an HTML element start tag to possible end tags
    struct HTMLElementMap : std::map<const Char*, HTMLElement, HTMLTagLess>
    {
        // HTMLElementMap: populate map from table
        HTMLElementMap()
        {
            for (register const Char* const *p = k_endTags; *p; ++p)
            {
                HTMLElement::EndTagRule r;
                if (*p[1] == *F)      r = HTMLElement::R_FORBIDDEN;
                else if (*p[1] == *O) r = HTMLElement::R_OPTIONAL;
                else                  r = HTMLElement::R_REQUIRED;
                HTMLElement &e = insert(value_type(*p++, HTMLElement(*p, r))).first->second;
                if (r == HTMLElement::R_OPTIONAL)
                {
                    // Elements that have optional end tags have a null-terminated list of closing tags.
                    while (*++p) e.endTags().insert(*p);
                }
                else if (r == HTMLElement::R_REQUIRED)
                {
                    // Elements that have required and tags have a single closing tag
                    e.endTags().insert(*++p);
                }
            }
        }
    };

    // Helper class to keep track of parsing stack
    struct ParseNode
    {
        // Attributes
        DOMNode* m_node;
        const HTMLElement* m_element;
        ParseNode(DOMNode* n, const HTMLElement* e) : m_node(n), m_element(e) {}

        // Accessors
        DOMNode* node()              { return m_node; }
        const HTMLElement* element() { return m_element; }
    };
    typedef std::stack<ParseNode> ParseNodeStack;

    // Helper class to manage RODOM lite module state
    struct RODOMModuleState : Core::ModuleState
    {
        // Attributes
        HTMLElementMap m_elements;

        // Accessors
        const HTMLElementMap& htmlElements() { return m_elements; }
    };

    // Node cache used by parsing (optimization)
    typedef std::map<String, DOMNode*> NodeCache;

    // Lightweight and portable implementation of IRODOM
    struct RODOM : IRODOM
    {
        // parse: parse XML string into RODOM
        IDOMNode* parseXML(const String& text) throw (IRODOM::ParseFailed)
        {
            Log::Scope scope(KCC_FILE, "parseXML");
            return parse(text.data(), text.data() + text.size(), true);
        }

        // parse: parse HTML string into RODOM
        IDOMNode* parseHTML(const String& text) throw (IRODOM::ParseFailed)
        {
            Log::Scope scope(KCC_FILE, "parseHTML");
            return parse(text.data(), text.data() + text.size(), false);
        }

        // moveToTagEnd: move c to end of tag allowing for embedded quotes
        void moveToTagEnd(register const Char*& c, register const Char* end)
        {
            register Char quote = 0;
            while (c != end)
            {
                if (quote != 0)
                {
                    // skip until close quote found
                    if (*c == quote) quote = 0;
                }
                else if (*c == '\"' || *c == '\'')
                {
                    // found open quote, ignore until close found
                    quote = *c;
                }

                // end of tag
                if (*c++ == '>') break;
            }
        }

        // parseAttributes: parse attributes into currNode
        void parseAttributes(
            register const Char* c,
            register const Char* end,
            DOMNode* currNode,
            bool xml)
        {
            // skip element name
            while (c != end && !Strings::isSpace(*c++))
                ;

            // parse attributues
            while (c != end)
            {
                //
                // attribute name
                //

                // move to start of name: 1st alpha char
                while (c != end && !Strings::isAlpha(*c))
                    ++c;
                if (c == end) break; // no attribute name found
                const Char* const name_begin = c;

                // move to end of name: 1st ' ' or '='
                while (c != end && !Strings::isSpace(*c) && *c != '=')
                    ++c;
                const Char* const name_end = c;

                // attribute name found
                String name(name_begin, name_end);
                // html name is unform-case: lower
                if (!xml) Strings::toLower(name);

                // move to start of attribute separator: first non-' '
                while (c != end && Strings::isSpace(*c))
                    ++c;

                //
                // attribute value
                //

                // html valueless attribute: set its value to be its own name (per HTML 4.0 spec)
                if (!xml) 
                {
                    
                    if (c == end || *c != '=')
                    {
                        currNode->addAttributeImpl(
                            new DOMNode(
                                IDOMNode::ATTRIBUTE_NODE,
                                name,
                                name));
                        continue;
                    }
                }

                // move to start of value: past ' ' and '='
                while (c != end && (Strings::isSpace(*c) || *c == '='))
                    ++c;
                if (c == end) break; // nothing after '='

                // determine the span of the attribute's value: if it started
                // with a quote, it's terminated only by the matching closing
                // quote; if not, it's terminated by a whitespace character or
                // the first quote found
                //
                // more lenient than the HTML 4.0 specification in that
                // it allows non-quoted values to contain characters other than
                // the set [A-Za-z0-9.-], i.e., any non-whitespace character.
            
                // determine quote type
                Char quote = 0;
                if (*c == '"' || *c == '\'')
                {
                    quote = *c;
                    c++;
                }

                // move to value end: 1st matching quote or ' '
                Char misplacedquote = 0;
                const Char* const b = c;
                for (; c != end; ++c)
                {
                    if (quote)
                    {
                        // stop at matching quote only
                        if (*c == quote) break;
                    }
                    else
                    {
                        if (misplacedquote)
                        {
                            // closing misplaced quote: end of value
                            if (*c == misplacedquote) break;
                        }
                        else if (*c == '"' || *c == '\'')
                        {
                            // misplaced quote, skip ' ' and look for closing misplaced quote
                            misplacedquote = *c;
                            // special case: empty quoted string or closing quote with no opening
                            if (c+1 == end || *(c+1) == misplacedquote) break;
                        }
                        else if (Strings::isSpace(*c))
                        {
                            // whitespace found: end of value
                            break;               
                        }
                    }
                }

                // attribute value found
                String value(b, c);

                // add attribute a child node
                currNode->addAttributeImpl(new DOMNode(IDOMNode::ATTRIBUTE_NODE, name, value));
                if (c == end) break;
                ++c;
            }
        }

        // parse: parse text returning root node
        DOMNode* parse(register const Char* c, register const Char* end, bool xml)
        {
            // build document root
            NodeCache nodeCache;
            ParseNodeStack nodeStack;
            DOMNode* const rootNode = DOMNodeFactory::constructDocumentNode();
            nodeStack.push(ParseNode(rootNode, NULL));

            // parse document adding to root
            DOMNode* currNode = rootNode;
            while (c != end)
            {
                const Char* const b = c;
                if (*c == '<')
                {
                    c++;
                    if (c == end) break;

                    // tag
                    if (Strings::isAlpha(*c) || *c == '/')
                    {
                        if (xml) parseXMLTag(nodeStack, c, end, rootNode, currNode); 
                        else     parseHTMLTag(nodeStack, c, end, rootNode, currNode, nodeCache);
                        continue;
                    }
                    c++;
                    if (c == end) break;

                    // comment
                    if (*c == '-' && c + 1 != end && c[1] == '-')
                    {
                        c += 2;
                        while (c < end)
                        {
                            if (*c++ == '-' && c != end && *c == '-')
                            {
                                const Char* const d = c;

                                // skip past space
                                while (++c != end && Strings::isSpace(*c))
                                    ;

                                // found end
                                if (c == end || *c++ == '>') break;
                                c = d;
                            }
                        }
                        if (c != end) // end of comment
                        {
                            DOMNode* comment = DOMNodeFactory::constructCommentNode();
                            comment->value() = String(b+4, c-3);
                            currNode->addNodeImpl(comment);
                        }
                        continue;
                    }

                    // CDATA
                    int const len = k_domCDATA.length();
                    if (*c == '[' && c + len < end && std::memcmp(c, k_domCDATA.data(), len) == 0)
                    {
                        c += len;
                        while (c < end)
                        {
                            if (*c++ == ']' && c != end && *c == ']')
                            {
                                const Char* const d = c;

                                // skip past space
                                while (++c != end && Strings::isSpace(*c))
                                    ;

                                // found end
                                if (c == end || *c++ == '>') break;
                                c = d;
                            }
                        }
                        if (c < end) // end of doc
                        {
                            DOMNode* cdata = DOMNodeFactory::constructCDATANode();
                            cdata->value() = String(b+len+2, c-3);
                            currNode->addNodeImpl(cdata);
                        }
                        continue;
                    }

                    // XML: < always begins a tag, HTML: < is text
                    if (xml)
                    {
                        moveToTagEnd(c, end);
                        goto createTextNode;
                    }
                }

                // SGML text
                for (; c != end; c++)
                {
                    // a new SGML tag has been found so stop parsing text
                    if (*c == '<') break;
                }

    createTextNode:
                {
                    // XML: ws between nodes is discarded, HTML: ws is meaningful
                    String data(b, c);
                    if (!xml || !Strings::isws(data))
                    {
                        DOMNode* text = DOMNodeFactory::constructTextNode();
                        text->value() = data;
                        currNode->addNodeImpl(text);
                    }
                }
            }

            return rootNode;
        }

        // parseHTMLTag: parse the html tag
        void parseHTMLTag(
            ParseNodeStack& nodeStack,
            register const Char*& c,
            register const Char* end,
            DOMNode* const rootNode,
            DOMNode*& currNode,
            NodeCache& nodeCache)
        {
            if (c == end) return;
            const Char* const tagBegin = c;
            moveToTagEnd(c, end);
            const Char* const tagEnd = c - 1;
            bool const isEndTag = *tagBegin == '/';

            // read tag name
            Char name[MAX_TAG+1]; // +1 for NULL
            {
                // search tag for 1st whitespace or end
                register Char* to = name;
                register const Char* from = tagBegin;
                while (from != tagEnd && !Strings::isSpace(*from) && from - tagBegin < MAX_TAG)
                    *to++ = Strings::toLower(*from++);

                // check for malformed tags that don't put a space on closed elements
                // e.g. '<foo/>' which should be '<foo />
                if (*(from-1) == '/' && *from == '>') to--;

                *to = '\0';
            }

            //
            // END TAG: 
            //  - check if current open tag should be closed according to HTML 2.0 rules
            //
            // OPEN TAG:
            //  - create document, if needed
            //  - fix-up document, if needed
            //  - create node and parse attributes for tag
            //
            if (isEndTag)
            {
                // check if end tag should close current node (according to HTML 2.0 rules)
                if (nodeStack.size() > 1 && nodeStack.top().element()->endTag(name))
                {
                    nodeStack.pop();
                    currNode = nodeStack.top().node();
                }
            }
            else
            {
                // locate element for tag
                const HTMLElementMap& elements = KCC_STATE(RODOMModuleState).htmlElements();
                HTMLElementMap::const_iterator const find = elements.find(name);

                // didn't find the element in our HTML 2.0 tag list: ignore it.
                //
                // NOTE: We really should do something better because this could
                // potentially mess up the proper closing of elements, but,
                // since we know nothing about this element, there's nothing
                // better that can be done.
                if (find == elements.end()) return;

                // fetch html element for tag
                const HTMLElement& e = find->second;
                String tagName(name);

                //
                // FIX-UP HTML DOCUMENT:
                //  - build html document
                //  - always place body & head as children of html
                //  - always place meta, title, style as children of head
                //

                // build base document if needed
                if (currNode == rootNode)
                {
                    AutoPtr<IDOMNodeList> lHtml(rootNode->findNodesByTagName(k_domHTML));
                    if (lHtml->getLength() == 0)
                    {
                        // add place-holder html tree
                        DOMNode* htmlNode = new DOMNode(IDOMNode::ELEMENT_NODE, k_domHTML);
                        currNode->addNodeImpl(htmlNode);
                        currNode = htmlNode;
                        nodeStack.push(ParseNode(currNode, &elements.find(name)->second));
                    }
                }

                // merge html
                if (tagName == k_domHTML)
                {
                    while (currNode->name() != k_domDOCUMENT)
                    {
                        nodeStack.pop();
                        currNode = nodeStack.top().node();
                    }
                    DOMReader find(rootNode);
                    currNode = (DOMNode*)find.nodePathQuery("/html");
                    nodeStack.push(ParseNode(currNode, &elements.find(k_domHTML)->second));
                    parseAttributes(tagBegin, tagEnd, currNode, false);
                    return;
                }

                // merge head
                if (tagName == "head")
                {
                    resetCurrentToHtmlChild(nodeStack, elements, "head", currNode);
                    parseAttributes(tagBegin, tagEnd, currNode, false);
                    return;
                }

                // merge body
                if (tagName == "body")
                {
                    resetCurrentToHtmlChild(nodeStack, elements, "body", currNode);
                    parseAttributes(tagBegin, tagEnd, currNode, false);
                    return;
                }

                // merge frameset
                if (tagName == "frameset")
                {
                    resetCurrentToHtmlChild(nodeStack, elements, "frameset", currNode);
                    parseAttributes(tagBegin, tagEnd, currNode, false);
                    return;
                }

                // always place head tags in head
                if (
                    currNode->name() != "head" &&
                    (tagName == "meta" || tagName == "style" || tagName == "title"))
                {
                    resetCurrentToHtmlChild(nodeStack, elements, "head", currNode);
                }

                // always contain head & body (if not frameset or head tag)
                if (
                    currNode->name() != "body" &&
                    tagName != "frameset" &&
                    tagName != "meta" && 
                    tagName != "style" && 
                    tagName != "title" && 
                    tagName != k_htmlScript)
                {
                    // creat head if needed
                    DOMNode* head = nodeCache["head"];
                    if (head == NULL)
                    {
                        DOMReader find(rootNode);
                        head = (DOMNode*)find.nodePathQuery("/html/head");
                        if (head == NULL) 
                        {
                            resetCurrentToHtmlChild(nodeStack, elements, "head", currNode);
                            nodeCache["head"] = currNode;
                        }
                    }

                    // create body
                    DOMNode* body = nodeCache["body"];
                    if (body == NULL)
                    {
                        DOMReader find(rootNode);
                        body = (DOMNode*)find.nodePathQuery("/html/body");
                        if (body == NULL) 
                        {
                            resetCurrentToHtmlChild(nodeStack, elements, "body", currNode);
                            nodeCache["body"] = currNode;
                        }
                    }
                }

                // create node for element
                DOMNode* node = new DOMNode(IDOMNode::ELEMENT_NODE, name);
                parseAttributes(tagBegin, tagEnd, node, false);
                currNode->addNodeImpl(node);

                // element's end tag isn't forbidden, so make it the new current node.
                if (e.rule() != HTMLElement::R_FORBIDDEN)
                {
                    nodeStack.push(ParseNode(node, &e));
                    currNode = node;
                }

                // script tag so skip to script close (this accounts for embeded < or > in script
                if (tagName == k_htmlScript)
                {
                    int const len = k_htmlScriptEnd.length();
                    for (; c < end - len; c++)
                    {
                        if (*c == '<' && Strings::toLower(String(c, c + len)) == k_htmlScriptEnd)
                        {
                            DOMNode* text = DOMNodeFactory::constructTextNode();
                            text->value() = String(tagEnd + 1, c);
                            currNode->addNodeImpl(text);
                            break;
                        }
                    }
                }
            }
        }

        // resetCurrentToHtmlChild: reset stack top to html child (head or body)
        void resetCurrentToHtmlChild(
            ParseNodeStack& nodeStack,
            const HTMLElementMap& elements,
            const char* nameToSet,
            DOMNode*& currNode)
        {
            // pop stack to root
            while (currNode->name() != k_domDOCUMENT)
            {
                nodeStack.pop();
                currNode = nodeStack.top().node();
            }

            // push html
            AutoPtr<IDOMNodeList> lHtml(currNode->findNodesByTagName(k_domHTML));
            currNode = (DOMNode*) lHtml->getItem(0);
            nodeStack.push(ParseNode(currNode, &elements.find(k_domHTML)->second));

            // push child, created if needed
            AutoPtr<IDOMNodeList> lChild(currNode->findNodesByTagName(nameToSet));
            if (lChild->getLength() == 0L)
            {
                DOMNode* child = new DOMNode(IDOMNode::ELEMENT_NODE, nameToSet);
                currNode->addNodeImpl(child);
                currNode = child;
            }
            else
            {
                currNode = (DOMNode*) lChild->getItem(0);
            }
            nodeStack.push(ParseNode(currNode, &elements.find(nameToSet)->second));
        }

        // parseXMLTag: parse the xml tag
        void parseXMLTag(
            ParseNodeStack& nodeStack,
            register const Char*& c,
            register const Char* end,
            DOMNode* const rootNode,
            DOMNode*& currNode)
        {
            if (c == end) return;
            const Char* const tagBegin = c;
            moveToTagEnd(c, end);
            const Char* const tagEnd = c - 1;
            bool const isEndTag = *tagBegin == '/';
            bool const isSimpleTag = *(tagEnd-1) == '/';

            // read tag name
            Char name[MAX_TAG+1]; // +1 for NULL
            {
                // search tag for 1st whitespace or end
                register Char* to = name;
                register const Char* from = tagBegin;
                while (from != tagEnd && !Strings::isSpace(*from) && from - tagBegin < MAX_TAG)
                    *to++ = *from++;

                // check for malformed tags that don't put a space on closed elements
                // e.g. '<foo/>' which should be '<foo />
                if (*(from-1) == '/' && *from == '>') to--;

                *to = '\0';
            }

            // end of tag: pop current node off parse stack 
            if (isEndTag)
            {
                if (nodeStack.size() <= 1) 
                {
                    String msg("malformed xml. unbalanced xml node: tag=[");
                    msg += (name+1);
                    msg += "]";
                    throw IRODOM::ParseFailed(msg);
                }
                nodeStack.pop();
                currNode = nodeStack.top().node();
                return;
            }

            // add element
            DOMNode* node = new DOMNode(IDOMNode::ELEMENT_NODE, name);
            parseAttributes(tagBegin, tagEnd, node, true);
            currNode->addNodeImpl(node);

            // simple tag has collapsed start/end tags so don't set current
            if (!isSimpleTag)
            {
                nodeStack.push(ParseNode(node, NULL));
                currNode = node;
            }
        }
    };
    
    //
    // RODOM factory
    //

    KCC_COMPONENT_FACTORY_IMPL(RODOM)
    
    KCC_COMPONENT_FACTORY_METADATA_BEGIN_COMPONENT(RODOM, IRODOM)
        KCC_COMPONENT_FACTORY_METADATA_PROPERTY(KCC_COMPONENT_FACTORY_SCM, KCC_VERSION)
    KCC_COMPONENT_FACTORY_METADATA_END
}
