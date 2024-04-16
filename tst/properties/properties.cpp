#include <inc/core/Core.h>

#define KCC_FILE    "properties"
#define KCC_VERSION "$Id: properties.cpp 22624 2008-03-09 22:47:44Z tvk $"

void dump(const kcc::String& subj, kcc::StringMap& p)
{
    kcc::StringStream out;
    out << subj << "\n=================\n";
    for (kcc::StringMap::iterator i = p.begin(); i != p.end(); i++)
        out << i->first << ": " << i->second << "\n";
    kcc::Log::out(out.str());
}

int main(int argc, const char* argv[])
{
    kcc::Properties props;
    props.set("kcc.logVerbosity", (long) kcc::Log::V_INFO_3);
    props.set("kcc.logMax",       1L);
    props.set("kcc.LogName",      KCC_FILE);
    if (argc > 1) props.load(argc, argv, false);
    kcc::Core::init(props, KCC_VERSION);

    kcc::Log::Scope scope(KCC_FILE, "main");
    kcc::Log::out("begin properties testing");
    try
    {
        kcc::StringMap p1, p2;
        kcc::String path("../../../tst/properties/properties.xml");

        kcc::Properties props;
        if (!props.load(path)) throw kcc::Exception("load failed");
        props.asMap(p1);
        dump("Properties", p1);
        
        kcc::String xml;
        kcc::Strings::loadText(path, xml);
        kcc::AutoPtr<kcc::IDOMNode> root(kcc::Core::rodom()->parseXML(xml));
        kcc::DOMReader r(root);
        kcc::AutoPtr<kcc::IDOMNodeList> nodes(r.nodes(root->getFirstChild(), "Property"));
        for (long i = 0; i < nodes->getLength(); i++) 
        {
            const kcc::IDOMNode* n = nodes->getItem(i);
            p2[r.attr(n, "key")] = r.attr(n, "value");
        }
        dump("XML", p1);
        
        bool match = p1 == p2 && p2 == p1;
        kcc::Log::out("properties == xml: %s", (match ? "yes" : "no"));

        kcc::Log::out("\ncompleted properties testing");
    }
    catch (std::exception& e)
    {
        kcc::Log::exception(e);
        return 1;
    }

    return 0;
}
