/*
 * Kuumba C++ Core
 */
#ifndef DOMWriter_h
#define DOMWriter_h

namespace kcc
{
    /**
    * Helper class for writing DOM nodes
    *
    * @author Ted V. Kremer
    */
    class KCC_CORE_EXPORT DOMWriter
    {
    public:
        /** Error writing DOM exception */
        KCC_COMPONENT_EXCEPTION(NotOpenException);

        /**
         * Initialize an DOM writer
         * @param out stream to write to
         * @param noPrologue don't emit an xml prologue
         */
        DOMWriter(std::ostream& out, bool noPrologue = false);
        virtual ~DOMWriter();

        /**
         * Start an DOM element
         * @param tag tag name of element
         */
        void start(const String& tag);

        /**
         * End an DOM element
         * @param tag tag name of element
         */
        void end(const String& tag);

        /**
         * Write an DOM CDATA
         * @param tag attribute name
         * @param value cdata value
         */
        void cdata(const String& value);

        /**
         * Write an DOM text
         * @param tag attribute name
         * @param value text value
         * @param encode encode value
         */
        void text(const String& value, bool encode = true);

        /**
         * Write an DOM comment
         * @param tag attribute name
         * @param value comment value
         */
        void comment(const String& value);

        /**
         * Write an DOM attribute (element must have started)
         * @param tag attribute name
         * @param value attribute value
         * @param encode encode value
         * @exception thrown if node not open
         */
        void attr(const String& tag, const String&  value, bool encode = true)     throw (DOMWriter::NotOpenException);
        void attr(const String& tag, const Char*    value, bool encode = true)     throw (DOMWriter::NotOpenException);
        void attr(const String& tag, long           value, const Char* f = "%d")   throw (DOMWriter::NotOpenException);
        void attr(const String& tag, double         value, const Char* f = "%.4f") throw (DOMWriter::NotOpenException);
        void attr(const String& tag, bool           value, bool label = true)      throw (DOMWriter::NotOpenException);
        void attr(const String& tag, const ISODate& value, ISODate::Format f = ISODate::F_NONE) throw (DOMWriter::NotOpenException);

        /**
         * Serialize a DOM tree
         * @param root node to serialize
         */
        void node(const IDOMNode* root);
        void node(const String& root);

    protected:
        // Attributes
        std::ostream& m_out;
        bool          m_open;
        
        // Implementation
        void dumpNode(const IDOMNode* node, std::ostream& out);

    private:
        DOMWriter(const DOMWriter&);
        DOMWriter& operator = (const DOMWriter&);
    };
}

#endif // DOMWriter_h
