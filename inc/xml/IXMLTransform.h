/*
 * Kuumba C++ Core
 *
 * $Id: IXMLTransform.h 20788 2007-09-21 13:39:29Z tvk $
 */
#ifndef IXMLTransform_h
#define IXMLTransform_h

namespace kcc
{
    /**
     * XML Transform
     *
     * @author Ted V. Kremer
     */
    interface IXMLTransform : IComponent
    {
        /** XML transform failed exception */
        KCC_COMPONENT_EXCEPTION(TransformException);
        
        /**
         * Initialize component
         * @param config configuration parameters
         * @return true if intialized successfully
         */
        virtual bool init(const Properties& config) = 0;

        /**
         * Apply transformation to XML (convenience wrappers to IXSLTApply)
         * @param xml xml to transform
         * @param xmlPath path to xml file
         * @param xsltPath path to xslt file
         * @param params paraemeters to pass to style sheet
         * @throws XMLTransformException if transformation error
         */
        virtual void apply(
            const String& xmlPath, const String& xsltPath, 
            const StringMap& params, std::ostream& out) throw (IXMLTransform::TransformException) = 0;
        virtual void apply(
            std::istream& xml, const String& xsltPath, 
            const StringMap& params, std::ostream& out) throw (IXMLTransform::TransformException) = 0;
    };
    
    /**
     * XSLT Apply
     *
     * @author Ted V. Kremer
     */
    interface IXSLTApply : IComponent
    {
        /**
         * Load XML. May be called multiple times
         * @param xml xml stream
         * @param xmlPath xml path
         * @throws IXMLTransform::TransformException if error loading
         */
        virtual void loadXml(std::istream& xml) throw (IXMLTransform::TransformException) = 0;
        virtual void loadXml(const String& xmlPath) throw (IXMLTransform::TransformException) = 0;

        /**
         * Load XSLT. May be called multiple times
         * @param xslt xslt stream
         * @param xsltPath xslt path
         * @throws IXMLTransform::TransformException if error loading
         */
        virtual void loadXslt(std::istream& xslt) throw (IXMLTransform::TransformException) = 0;
        virtual void loadXslt(const String& xsltPath) throw (IXMLTransform::TransformException) = 0;
        
        /**
         * Apply XSLT transformation. May be called multiple times.
         * @param params xslt params
         * @param out where to write transformation
         * @throws IXMLTransform::TransformException if error loading
         */
        virtual void apply(const StringMap& params, std::ostream& out) throw (IXMLTransform::TransformException) = 0;
    };

    
    /**
     * XML transformation factory specification
     *
     * @author Ted V. Kremer
     */
    interface IXMLTransformFactory : IComponentFactory
    {
        /**
         * Construct XML transform (default construct) (ownership IS consumed)
         */
        virtual IXMLTransform* constructTransform() = 0;
    
        /**
         * Construct XSLT apply (ownership IS consumed)
         */
        virtual IXSLTApply* constructApply() = 0;
    };
}

#endif // IXMLTransform_h
