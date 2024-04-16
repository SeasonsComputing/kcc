/*
 * Kuumba C++ Core
 *
 * $Id: IXMLSerializable.h 23232 2008-04-21 14:36:55Z tvk $
 */
#ifndef IXML_h
#define IXML_h

#include <inc/xml/XMLValue.h>
#include <inc/xml/XMLConstraint.h>
#include <inc/xml/XMLType.h>

namespace kcc
{
    /** XML serialization exception */
    KCC_COMPONENT_EXCEPTION(XMLSerializeException);
    
    /** 
     * XML metadata 
     *
     * XML meta-structure requires that XML language be structured as follows:
     *    <Type attr-value="..." ... >
     *        <attr-list ... />
     *        ...
     *        <attr-atomic ... />
     *        ...
     *    </Type>
     *
     * A strong binding between code-generation and persistence structure simplifies usage and coding
     * at the expense of more verbose XML languages (by-design).
     *
     * Design is essentially an XML'ized DTD
     *
     * NOTE: comment nodes are stripped when reading or writing
     */
    struct XMLMetadata
    {
        StringRC    type;
        StringMapRC attributes;  // attribute : attr-type           (value|atomic|list|cdata)
        StringMapRC values;      // attribute : attr-allowed-values (()|(enum1|enum2)|(number|isodate|boolean[true|false])|(rx:{regex}))
        StringMapRC constraints; // attribute : attr-constraint     (optional|required)
        XMLMetadata(const StringRC& t = Strings::emptyRC()) : type(t) {}
    };

    /**
     * XML Serialization
     *
     * @author Ted V. Kremer
     */
    interface IXMLSerializable : IComponent
    {
        /**
         * Serialize into DOM writer
         * @param w DOM writer to serialize into
         * @throws XMLSerializeException if error serializing
         */
        virtual void toXML(DOMWriter& w) throw (XMLSerializeException) = 0;

        /**
         * Deserialize from DOM reader
         * @param node parent node
         * @param r DOM reader to deserialize from
         * @throws XMLSerializeException if error deserializing
         */
        virtual void fromXML(const IDOMNode* node, DOMReader& r) throw (XMLSerializeException) = 0;

        /**
         * Validate DOM constrainsts
         * @throws XMLSerializeException if invalid
         */
        virtual void validate() throw (XMLSerializeException) = 0;

        /**
         * Accessor to metadata
         * @return xml metadata
         */
        virtual const XMLMetadata& metadata() = 0;
    };
}

#endif // IXML_h
