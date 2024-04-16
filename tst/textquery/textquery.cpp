#include <inc/core/Core.h>
#include <inc/store/ITextStore.h>

#define KCC_FILE    "textquery"
#define KCC_VERSION "$Id: textquery.cpp 15199 2007-03-09 17:57:17Z tvk $"

void results(kcc::ITextQuery* query, const kcc::String& expression)
{
    kcc::Log::Scope scope(KCC_FILE, "results");
    kcc::Log::out("textquery: expr=[%s]", expression.c_str());
    long rows = 0L;
    kcc::TextDocument txtdoc;
    kcc::AutoPtr<kcc::ITextResults> tr(query->query(expression, kcc::TextDocument::C_ALL));
    while (tr->next())
    {
        tr->results(txtdoc);

        // dump results
        std::cout << "\nresults \n[\nmetadata: ";
        for (kcc::StringMap::iterator i = txtdoc.metadata.begin(); i != txtdoc.metadata.end(); i++)
        {
            if (i != txtdoc.metadata.begin()) std::cout << ", ";
            std::cout << i->first << "=" << i->second;
        }
        std::cout << "\ntext: " << txtdoc.text;
        std::cout << "\ncounts: ";
        for (kcc::TextDocument::Terms::iterator i = txtdoc.terms.begin(); i != txtdoc.terms.end(); i++)
        {
            if (i != txtdoc.terms.begin()) std::cout << ", ";
            std::cout << i->first << "=" << i->second;
        }
        std::cout << "\n]\n";
        rows++;
    }
    std::cout << "\nrows: received=[" << rows << "] actual=[" << tr->total() << "]\n";
}

int main(int argc, const char* argv[])
{
    kcc::Properties props;
    props.set("kcc.logVerbosity", (long) kcc::Log::V_INFO_3);
    props.set("kcc.logMax",       1L);
    props.set("kcc.logName",      KCC_FILE);
    if (argc > 1) props.load(argc, argv, false);
    kcc::Core::init(props, KCC_VERSION);

    kcc::String host(props.get("host", "localhost:8080"));
    kcc::String expr(props.get("expr", "travelocity AND (orbitz OR expedia)"));

    kcc::Log::Scope scope(KCC_FILE, "main");
    try
    {
        kcc::Properties queryConfig;
        queryConfig.set("TextQueryClient.connections", host);
        kcc::AutoPtr<kcc::ITextQuery> query(KCC_COMPONENT(kcc::ITextQuery, "k_textqueryclient"));
        query->init(queryConfig);
        results(query, expr);
    }
    catch (std::exception& e)
    {
        kcc::Log::exception(e);
        return 1;
    }

    return 0;
}
