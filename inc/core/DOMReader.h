/*
 * Kuumba C++ Core
 */
#ifndef DOMReader_h
#define DOMReader_h

namespace kcc
{
    /**
     * Helper class for reading DOM nodes
     *
     * @author Ted V. Kremer 
     */
    class KCC_CORE_EXPORT DOMReader
    {
    public:
        /** Error reading DOM exception */
        KCC_COMPONENT_EXCEPTION(NotFoundException);

        /**
         * Initialize an DOM reader
         * @param root root node of DOM tree
         */
        DOMReader(const IDOMNode* root);
        virtual ~DOMReader();

        /**
         * Accessor to DOM root
         * @return DOM root
         */
        const IDOMNode* root();

        /**
         * Query DOM node using path expression 
         * @param path unix shell path e.g. /html/body/title or relative table/tbody
         * @param node node to search from (default is root)
         * @return node or null if not found
         */
        const IDOMNode* nodePathQuery(const String& path, const IDOMNode* node = NULL);

        /**
         * Query DOM document node
         * @param tagDoc tag of document node
         * @return document node
         * @exception DOMReader::NotFoundException if error getting document
         */
        const IDOMNode* doc(const String& tagDoc) throw (DOMReader::NotFoundException);

        /**
         * Query DOM node
         * @param root node to search
         * @param tag tag of node
         * @return node to locate
         * @exception DOMReader::NotFoundException if node not found or more than 1 found
         */
        const IDOMNode* node(const IDOMNode* root, const String& tag) throw (DOMReader::NotFoundException);

        /**
         * Query optional DOM node
         * @param root node to search
         * @param tag tag of node (or null if not found)
         * @return node to locate
         * @exception DOMReader::NotFoundException if error
         */
        const IDOMNode* nodeOp(const IDOMNode* root, const String& tag) throw (DOMReader::NotFoundException);

        /**
         * Query DOM node text
         * @param root node to search
         * @param tag tag of node
         * @return text of node
         * @exception DOMReader::NotFoundException if node not found or more than 1 found
         */
        String nodeTx(const IDOMNode* root, const String& tag) throw (DOMReader::NotFoundException);

        /**
         * Query root node children for specific tags (shallow search)
         * @param root node to search
         * @oaram tagNode node to search for
         * @return list of nodes matching tag (ownership IS consumed)
         */
        IDOMNodeList* nodes(const IDOMNode* root, const String& tagNode);

        /**
         * Query document root node children for specific tags (shallow search)
         * @param tagDoc tag of document node
         * @oaram tagNode node to search for
         * @return list of nodes matching tag (ownership IS consumed)
         * @exception DOMReader::NotFoundException if error getting nodes
         */
        IDOMNodeList* nodes(const String& tagDoc, const String& tagNode) throw (DOMReader::NotFoundException);

        /**
         * Search first node child first for text
         * @param node node to get text for
         * @return text of node or empty string if no text node
         */
        String text(const IDOMNode* node);

        /**
         * Search first node child first for CDATA
         * @param node node to get text for
         * @return text of node or empty string if no text node
         */
        String cdata(const IDOMNode* node);

        /**
         * Search node child first comment
         * @param node node to get text for
         * @return text of node or empty string if no text node
         */
        String comment(const IDOMNode* node);

        /**
         * Search node attribute tag and retrieve its text
         * @param node node to get attribute text for
         * @param tagAttr tag of attribute
         * @return text of node
         * @exception DOMReader::NotFoundException if no text found
         */
        String attr(const IDOMNode* node, const String& tagAttr) throw (DOMReader::NotFoundException);
        void   attr(const IDOMNode* node, const String& tagAttr, String& value) throw (DOMReader::NotFoundException);

        /**
         * Query optional DOM attribute
         * @param node node to search
         * @param tag tag of attribute (or null if not found)
         * @param value out param of attribute value if exists
         * @return true if attribute exists
         */
        bool attrOp(const IDOMNode* node, const String& tagAttr, String& value);

    protected:
        // Attributes
        const IDOMNode* m_root;

    private:
        DOMReader(const DOMReader&);
        DOMReader& operator = (const DOMReader&);
    };

    /**
     * Helper class for reading DOM nodes from a file (dom tree is loaded and managed)
     *
     * @author Ted V. Kremer 
     */
    class KCC_CORE_EXPORT DOMFileReader : public DOMReader
    {
    public:
        /**
         * ctor, dtor - manage dom tree
         */
        DOMFileReader();
        virtual ~DOMFileReader();

        /**
         * Initialize an DOM reader
         * @param path path to document to load
         */
        bool load(const String& path);
    };

    /**
     * Helper class for reading DOM nodes from a URL (dom tree is loaded and managed)
     *
     * @author Ted V. Kremer 
     */
    class KCC_CORE_EXPORT DOMURLReader : public DOMReader
    {
    public:
        /**
         * ctor, dtor - manage dom tree
         */
        DOMURLReader();
        virtual ~DOMURLReader();

        /**
         * Initialize an DOM reader
         * @param url url to document to load
         */
        bool load(const URL& url);
    };
}

#endif // DOMReader_h
