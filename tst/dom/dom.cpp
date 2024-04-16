
#include <inc/core/Core.h>

#define KCC_FILE    "dom"
#define KCC_VERSION "$Id: dom.cpp 20680 2007-09-12 15:40:50Z tvk $"

// scanfiles: scan path for files
void scanfiles(const kcc::String& path, kcc::StringVector& files)
{
    kcc::Platform::Files find;
    kcc::Platform::fsDir(path, find);
    for (
        kcc::Platform::Files::iterator i = find.begin();
        i != find.end();
        i++)
    {
        const kcc::Platform::File& f = *i;
        kcc::String fn = kcc::Platform::fsFullPath(path, f.name);
        if (f.dir)
        {
            scanfiles(fn, files);
        }
        else if (f.name.find(".htm") != kcc::String::npos || f.name.find(".xml") != kcc::String::npos)
        {
            files.push_back(fn);
        }
    }
}

// html: dump formatted html (using html comments, etc.)
void html(const kcc::IDOMNode* n, std::ostream& out, const kcc::String& pad = kcc::Strings::empty())
{
    kcc::String val(kcc::Strings::trimws(n->getNodeValue()));
    if (n->getNodeType() == kcc::IDOMNode::TEXT_NODE)
    {
        if (!val.empty()) out << pad << val << std::endl;
        return;
    }
    if (n->getNodeType() == kcc::IDOMNode::COMMENT_NODE)
    {
        out << pad << "<!--" << n->getNodeValue() << "-->" << std::endl;
        return;
    }
    out << pad << "<" << n->getNodeName();
    const kcc::IDOMNamedNodeMap* a = n->getAttributes();
    for (int i = 0; i < a->getLength(); i++)
    {
        const kcc::IDOMNode* an = a->getItem(i);
        out << " " << an->getNodeName() << "=\"" << an->getNodeValue() << "\"";
    }
    if (n->hasChildNodes())
    {
        if (val.empty() && !n->getNodeValue().empty())
            out << ">" << std::endl;
        else
            out << ">" << std::endl << val;
        kcc::String indent(pad + "  ");
        const kcc::IDOMNodeList* c = n->getChildNodes();
        for (int i = 0; i < c->getLength(); i++)
            html(c->getItem(i), out, indent);
        out << pad << "</" << n->getNodeName() + ">" << std::endl;
    }
    else
    {
        out << "/>" << std::endl;
    }
}

// test: general html & xml parsing
void test(kcc::IRODOM* rodom, const kcc::String& path, std::ostream& out)
{
    long htmlTotals = 0L, htmlDocs = 0L;
    long xmlTotals = 0L, xmlDocs = 0L;
    std::clock_t start = 0L, end = 0L;
    kcc::String pq("/html/head/title");

    kcc::Log::out("building file list: %s", path.c_str());
    kcc::StringVector list;
    scanfiles(path, list);

    kcc::Log::out("parsing files...");
    for (kcc::StringVector::iterator i = list.begin(); i != list.end(); i++)
    {
        kcc::String text;
        kcc::Strings::loadText(*i, text);
        kcc::Platform::File f;
        kcc::Platform::fsFile(*i, f);
        kcc::Log::out("Parsing: name=[%s] size=[%d]", f.name.c_str(), f.size);
        if (f.name.find(".htm") != kcc::String::npos)
        {
            htmlDocs++;
            try
            {
                out << std::endl;
                out << "DUMPING HTML: " << f.name << std::endl;
                out << "************************************************" << std::endl;
                out << std::endl;

                start = std::clock();
                kcc::AutoPtr<kcc::IDOMNode> root(rodom->parseHTML(text));
                end = std::clock();
                htmlTotals += end - start;

                html(root, out);

                kcc::DOMReader r(root);
                const kcc::IDOMNode* n = r.nodePathQuery(pq);
                out << std::endl << "Query [" << pq << "] " << (n == NULL ? "NOT FOUND" : "found") << std::endl;
            }
            catch (kcc::IRODOM::ParseFailed& exc)
            {
                end = std::clock();
                htmlTotals += end - start;
                kcc::Log::error("failed %s [%s]", f.name.c_str(), exc.what());
            }
        }
        else if (f.name.find(".xml") != kcc::String::npos || f.name.find(".xsl") != kcc::String::npos)
        {
            xmlDocs++;
            try
            {
                out << std::endl;
                out << "DUMPING XML: " << f.name << std::endl;
                out << "************************************************" << std::endl;
                out << std::endl;

                start = std::clock();
                kcc::AutoPtr<kcc::IDOMNode> root(rodom->parseXML(text));
                end = std::clock();
                xmlTotals += end - start;

                kcc::DOMWriter xml(out);
                xml.node(root);

                out << std::endl;
            }
            catch (kcc::IRODOM::ParseFailed& exc)
            {
                end = std::clock();
                xmlTotals += end - start;
                kcc::Log::error("failed %s [%s]", f.name.c_str(), exc.what());
            }
        }
    }

    kcc::Log::out("rodom html tot=[%d] secs=[%f] xml tot=[%d] sec=[%f]",
        htmlDocs, (float)htmlTotals/CLOCKS_PER_SEC,
        xmlDocs, (float)xmlTotals/CLOCKS_PER_SEC);
}

void htmlspeed(kcc::IRODOM* rodom, const kcc::String& path)
{
    long htmlTotals = 0L, htmlDocs = 0L;
    std::clock_t start = 0L, end = 0L;

    kcc::Log::out("building file list: %s", path.c_str());
    kcc::StringVector list;
    scanfiles(path, list);
    kcc::Platform::fsDirCreateEmpty("tst-out/domspdtst");

    kcc::Log::out("parsing files...");
    for (kcc::StringVector::iterator i = list.begin(); i != list.end(); i++)
    {
        kcc::Platform::File f;
        kcc::Platform::fsFile(*i, f);
        if (f.name.find(".htm") == kcc::String::npos) continue;
        htmlDocs++;
        try
        {
            kcc::String text;
            kcc::Strings::loadText(*i, text);

            start = std::clock();
            kcc::AutoPtr<kcc::IDOMNode> root(rodom->parseHTML(text));
            end = std::clock();
            htmlTotals += end - start;

            if (htmlDocs % 500 == 0)
            {
                kcc::String fp(kcc::Platform::fsFullPath("tst-out/domspdtst", f.name));
                kcc::Log::out("Writing: path=[%s] size=[%d] total=[%d]", fp.c_str(), f.size, htmlDocs);
                std::ofstream out(fp.c_str());
                kcc::AutoPtr<kcc::IDOMNodeList> head(root->findNodesByTagName("html"));
                kcc::DOMWriter xml(out);
                xml.node(head->getItem(0));
                out.close();
                out.open((fp + ".orig").c_str());
                out << text;
                out.close();
            }
            if (htmlDocs > 2000) break;
        }
        catch (kcc::IRODOM::ParseFailed& exc)
        {
            end = std::clock();
            htmlTotals += end - start;
            kcc::Log::error("failed %s [%s]", f.name.c_str(), exc.what());
        }
    }

    float secs = (float)htmlTotals/CLOCKS_PER_SEC;
    float avg  = secs / htmlDocs;
    kcc::Log::out("rodom html tot=[%d] secs=[%f] avg=[%f]", htmlDocs, secs, avg);
}

// htmlclean: clean html by parsing (which will fix-up html) and rewriting
void htmlclean(kcc::IRODOM* rodom, const kcc::String& path)
{
    kcc::String text;
    kcc::Strings::loadText(path, text);
    kcc::AutoPtr<kcc::IDOMNode> root(rodom->parseHTML(text));
    std::ofstream out((path + ".clean").c_str());
    kcc::AutoPtr<kcc::IDOMNodeList> head(root->findNodesByTagName("html"));
    html(head->getItem(0), out);
    out.close();
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
    kcc::Log::out("begin dom testing");
    try
    {
        std::ofstream out("tst-out/domtest.txt");
        test(kcc::Core::rodom(), "tst-in", out);
        out.close();

        /*
        htmlspeed(kcc::Core::rodom(), "/Work/Sandbox/tst-dom-spd/cache-205");
        //htmlclean(kcc::Core::rodom(), "tst-out/domspdtst/00e29575af86d7ce5bbe0ac45dc27099.html.orig");
        //htmlclean(kcc::Core::rodom(), "tst-out/domspdtst/test.html");
        */

        kcc::Log::out("completed dom testing");
    }
    catch (std::exception& e)
    {
        kcc::Log::exception(e);
        return 1;
    }

    return 0;
}
