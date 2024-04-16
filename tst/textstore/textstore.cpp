#include <inc/core/Core.h>
#include <inc/store/ITextStore.h>

#define KCC_FILE    "textstore"
#define KCC_VERSION "$Id: textstore.cpp 23042 2008-04-07 20:10:40Z tvk $"

bool indexDocument(kcc::ITextStore* store, const kcc::String& line) throw (kcc::TextException)
{
    int yr = 0, mo = 0, dy = 0;
    kcc::StringVector csv;
    kcc::Strings::csv(kcc::Strings::trimws(line), csv);
    if (csv.size() != 3) return false;
    std::sscanf(csv[2].c_str(), "%d/%d/%d", &mo, &dy, &yr);
    kcc::ISODate date(yr, mo, dy);
    kcc::TextDocument txtdoc;
    txtdoc.text                  = csv[1];
    txtdoc.metadata["id"]        = csv[0];
    txtdoc.metadata["md5"]       = kcc::MD5::hash(txtdoc.text);
    txtdoc.metadata["date_enum"] = date.isodate();
    txtdoc.metadata["date_text"] = date.isodate();
    static kcc::TextDocument::MetadataFields fields;
    if (fields.empty())
    {
        fields["id"]        = kcc::TextDocument::MD_ENUM;
        fields["md5"]       = kcc::TextDocument::MD_ENUM;
        fields["date_enum"] = kcc::TextDocument::MD_ENUM;
        fields["date_text"] = kcc::TextDocument::MD_TEXT;
    }
    store->indexText(
        txtdoc, fields, 
        kcc::TextDocument::C_TEXT|kcc::TextDocument::C_METADATA|kcc::TextDocument::C_TERMS);
    return true;
}

void index(kcc::ITextStore* store, const kcc::String& input, long maxDocs) throw (kcc::TextException)
{
    kcc::Log::Scope scope(KCC_FILE, "index");
    long total = 0L;
    kcc::String line;
    store->indexOpen();
    std::ifstream in(input.c_str());
    while (!in.eof() && in.good())
    {
        std::getline(in, line);
        if (indexDocument(store, line))
        {
            total++;
            if (maxDocs > 0L && total >= maxDocs) break;
        }
    }
    in.close();
    store->indexClose();
}

void query(kcc::ITextStore* store, const kcc::String& expression) throw (kcc::TextException)
{
    kcc::Log::Scope scope(KCC_FILE, "results");
    std::cout << "query:\n\t[" << expression << "]\n";
    long rows = 0L;
    kcc::TextDocument txtdoc;
    kcc::AutoPtr<kcc::ITextResults> tr(store->query(
        expression, 
        kcc::TextDocument::C_TEXT|kcc::TextDocument::C_METADATA|
        kcc::TextDocument::C_TERMS|kcc::TextDocument::C_QUERY_MATCHES));
    while (tr->next())
    {
        tr->results(txtdoc);

        std::cout << "********************* *********************\n";

        // matches: highlight in reverse so text inserts don't change offsets in front
        for (kcc::TextDocument::Matches::reverse_iterator i = txtdoc.matches.rbegin(); i != txtdoc.matches.rend(); i++)
        {
            kcc::TextDocument::Match& m = *i;
            txtdoc.text.insert(m.endOffset,   "</match>");
            txtdoc.text.insert(m.startOffset, "<match>");
        }
        std::cout << "text:\n\t[" << txtdoc.text << "]\n";

        std::cout << "metadata:\n\t[";
        for (kcc::StringMap::iterator i = txtdoc.metadata.begin(); i != txtdoc.metadata.end(); i++)
        {
            if (i != txtdoc.metadata.begin()) std::cout << ", ";
            std::cout << i->first << "=" << i->second;
        }
        std::cout << "]\n" << std::endl;


        std::cout << "terms:\n\t[";
        for (kcc::TextDocument::Terms::iterator i = txtdoc.terms.begin(); i != txtdoc.terms.end(); i++)
        {
            if (i != txtdoc.terms.begin()) std::cout << ", ";
            std::cout << i->first << "=" << i->second;
        }
        std::cout << "]\n";

        rows++;
    }
    std::cout << "rows: received=[" << rows << "] actual=[" << tr->total() << "]\n";
}

const kcc::Char* k_usage =
    "Usage:\n"
    "    textstore path={path} action=(create|insert|optimize|query) {params...}\n";
int main(int argc, const char* argv[])
{
    kcc::Properties props;
    props.set("kcc.logVerbosity", (long) kcc::Log::V_INFO_3);
    props.set("kcc.logMax",       10L);
    props.set("kcc.logName",      KCC_FILE);
    if (argc > 1) props.load(argc, argv, false);
    kcc::Core::init(props, KCC_VERSION);

    kcc::Log::Scope scope(KCC_FILE, "main");
    try
    {
        // action param
        const kcc::String& action = props.get("action", kcc::Strings::empty());
        if (!kcc::Strings::match(action, "create|insert|optimize|query"))
        {
            std::cerr << "Missing or invalid 'action' parameter.\n" << k_usage;
            return 1;
        }
        
        // path param: path|TextStore.path
        const kcc::String& path = props.exists("path") ? 
            props.get("path", kcc::Strings::empty()) : 
            props.get("TextStore.path", kcc::Strings::empty());
        if (path.empty())
        {
            std::cerr << "Missing 'path' parameter.\n" << k_usage;
            return 1;
        }
        props.set("TextStore.path", path);
        
        // component
        static const kcc::String& k_defComponent("k_textstore");
        const kcc::String& component = props.get("component", k_defComponent);
        kcc::AutoPtr<kcc::ITextStore> store(KCC_COMPONENT(kcc::ITextStore, component));
        
        // action
        if (action == "create")
        {
            const kcc::String& input = props.get("input", kcc::Strings::empty());
            kcc::Platform::fsDirCreateEmpty(path);
            props.set("TextStore.create", "1");
            if (!store->init(props)) throw kcc::Exception("store init failed");
            store->indexOpen();
            store->indexClose();
        }
        else if (action == "insert")
        {
            const kcc::String& input = props.get("input", kcc::Strings::empty());
            if (input.empty() || !kcc::Platform::fsExists(input))
            {
                std::cerr << "Missing or invalid 'input' parameter\n." << k_usage;
                return 1;
            }
            if (!store->init(props)) throw kcc::Exception("store init failed");
            index(store, input, props.get("maxDocs", -1L));
        }
        else if (action == "optimize")
        {
            if (!store->init(props)) throw kcc::Exception("store init failed");
            store->indexOpen();
            store->indexOptimize();
            store->indexClose();
        }
        else if (action == "query")
        {
            const kcc::String& expr = props.get("expr", kcc::Strings::empty());
            if (expr.empty())
            {
                std::cerr << "Missing 'expr' parameter\n." << k_usage;
                return 1;
            }
            if (!store->init(props)) throw kcc::Exception("store init failed");
            if (expr == "@expr")
            {
                kcc::String q;
                do
                {
                    std::cout << "\nexpression (CR to exit):\n";
                    std::getline(std::cin, q);
                    if (!q.empty()) query(store, q);
                }
                while (!q.empty());
            }
            else
            {
                query(store, expr);
            }
        }
    }
    catch (std::exception& e)
    {
        kcc::Log::exception(e);
        return 1;
    }

    return 0;
}
