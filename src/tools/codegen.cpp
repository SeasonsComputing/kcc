/*
 * Kuumba C++ Core
 *
 * $Id: codegen.cpp 19161 2007-06-27 18:39:28Z tvk $
 */
#include <inc/core/Core.h>
#include <inc/xml/IXMLTransform.h>

#define KCC_FILE    "codegen"
#define KCC_VERSION "$Id: codegen.cpp 19161 2007-06-27 18:39:28Z tvk $"

// main: entry point into codegen console application
int main(int argc, const char* argv[])
{
    std::cout <<
        KCC_FILE << " - Kuumba Code Generation" << std::endl << std::endl;

    // initialize kcc
    kcc::Properties props;
    props.set("kcc.logName",      KCC_FILE);
    props.set("kcc.logMax",       1L);
    props.set("kcc.logVerbosity", (long) kcc::Log::V_INFO_1);
    if (argc > 1) props.load(argc, argv, false);
    kcc::Core::init(props, KCC_VERSION);

    // run codegen
    kcc::Log::Scope scope(KCC_FILE, "main");
    try
    {
        // command line
        kcc::String xml   (props.get("xml",    kcc::Strings::empty()));
        kcc::String xslt  (props.get("xslt",   kcc::Strings::empty()));
        kcc::String output(props.get("output", kcc::Strings::empty()));
        kcc::String ext   (props.get("ext",    kcc::Strings::empty()));
        if (xml.empty() || xslt.empty() || output.empty() || ext.empty())
        {
            std::cout << "Usage: codegen xml=full-path xslt=full-path output=path ext=(cpp|h|js)" << std::endl;
            return 1;
        }
        kcc::String xsltPath(kcc::Platform::fsFullPath(xslt, ext + "." + "xsl"));
        if (!kcc::Platform::fsExists(xsltPath)) throw kcc::Exception("xslt file not found: " + xsltPath);
        if (!kcc::Platform::fsExists(xml))      throw kcc::Exception("xml file not found: " + xml);

        // apply transform
        kcc::AutoPtr<kcc::IXSLTApply> xform(KCC_FACTORY(kcc::IXMLTransformFactory, props.get("transform", "k_transform"))->constructApply());
        xform->loadXml(xml);
        xform->loadXslt(xsltPath);

        // call template for each type
        kcc::StringMap params;
        kcc::DOMFileReader r;
        if (!r.load(xml)) throw kcc::Exception("error reading xml: " + xml);
        kcc::AutoPtr<kcc::IDOMNodeList> types(r.nodes("Schema", "Type"));
        for (long i = 0L; i < types->getLength(); i++)
        {
            kcc::String type  (r.attr(types->getItem(i), "name"));
            kcc::String typext(r.attr(types->getItem(i), "ext"));
            if (!kcc::Strings::match(ext, typext)) continue;
            kcc::String path  (kcc::Platform::fsNormalize(output + kcc::Platform::fsSep() + type + "." + ext));
            std::cout << "Generating: " << path << std::endl;

            // transform
            std::ofstream out;
            out.open(path.c_str());
            if (out.fail()) throw kcc::Exception("unable to generate to path: " + path);
            params["template.param.type"]      = type;
            params["template.param.generated"] = kcc::ISODate::local().isodate();
            xform->apply(params, out);
            out.close();
        }
    }
    catch (std::exception& e)
    {
        kcc::Log::exception(e);
        return 1;
    }

    return 0;
}
