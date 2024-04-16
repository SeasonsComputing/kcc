/*
 * Kuumba C++ Core
 *
 * $Id: xform.cpp 20740 2007-09-17 14:56:30Z tvk $
 */
#include <inc/core/Core.h>
#include <inc/xml/IXMLTransform.h>

#define KCC_FILE    "xform"
#define KCC_VERSION "$Id: xform.cpp 20740 2007-09-17 14:56:30Z tvk $"

// main: entry point into xslt transformation console application
int main(int argc, const char* argv[])
{
    std::cout <<
        "xform - Kuumba XML Transformation" << std::endl << std::endl;

    // initialize kcc
    kcc::Properties props;
    props.set("kcc.logName",      KCC_FILE);
    props.set("kcc.logMax",       1L);
    props.set("kcc.logVerbosity", (long) kcc::Log::V_INFO_1);
    if (argc > 1) props.load(argc, argv, false);
    kcc::Core::init(props, KCC_VERSION);

    // run xform
    kcc::Log::Scope scope(KCC_FILE, "main");
    try
    {
        // command line
        kcc::String xml   (props.get("xml",    kcc::Strings::empty()));
        kcc::String xslt  (props.get("xslt",   kcc::Strings::empty()));
        kcc::String output(props.get("output", kcc::Strings::empty()));
        kcc::String params(props.get("params", kcc::Strings::empty()));
        if (xml.empty() || xslt.empty())
        {
            std::cout << "Usage: xform xml=full-path xslt=full-path output=(full-path) params=key:value,..." << std::endl;
            return 1;
        }

        // xsl params        
        kcc::StringMap    xparams;
        kcc::StringVector split, kv;
        kcc::Strings::tokenize(params, ",", split);
        for (kcc::StringVector::iterator i = split.begin(); i != split.end(); i++)
        {
            kcc::Strings::tokenize(params, ":", kv);
            if (kv.size() != 2) throw kcc::Exception("invalid param format");
            xparams[kv[0]] = kv[1];
        }

        // transform
        kcc::AutoPtr<kcc::IXMLTransform> xform(KCC_COMPONENT(kcc::IXMLTransform, props.get("transform", "k_transform")));
        
        // transform
        if (output.empty())
        {
            xform->apply(xml, xslt, xparams, std::cout);
        }
        else
        {
            std::ofstream out;
            out.open(output.c_str());
            if (out.fail()) throw kcc::Exception("unable to generate to path: " + output);
            xform->apply(xml, xslt, xparams, out);
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
