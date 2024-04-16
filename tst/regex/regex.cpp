#include <inc/core/Core.h>

#define KCC_FILE    "regex"
#define KCC_VERSION "$Id: regex.cpp 20678 2007-09-12 15:32:56Z tvk $"

void match(const kcc::String& expr, const kcc::String& target)
{
    kcc::Log::out(" *** matching: %s ~= %s", target.c_str(), expr.c_str());
    const kcc::String::const_iterator offset = target.begin();
    kcc::StringParts matches;
    if (kcc::Core::regex()->match(target, expr, matches, kcc::IRegex::O_NORMAL|kcc::IRegex::O_ICASE))
    {
        kcc::Log::out("MATCHED");
        for (kcc::StringParts::iterator m = matches.begin(); m != matches.end(); m++)
        {
            kcc::String::size_type offStart = (kcc::String::size_type)(m->first - offset);
            kcc::String::size_type offEnd   = (kcc::String::size_type)(m->second - offset);
            kcc::Log::out("PART: \"%s\" @ %d:%d", kcc::String(m->first, m->second).c_str(), offStart, offEnd);
        }
    }
    else
    {
        kcc::Log::out("NOT MATCHED");
    }
}

int main(int argc, const char** argv)
{
    kcc::Properties props;
    props.set("kcc.logName",      KCC_FILE);
    props.set("kcc.logMax",       1L);
    props.set("kcc.logVerbosity", (long) kcc::Log::V_INFO_3);
    if(argc > 1) props.load(argc, argv, false);
    kcc::Core::init(props, KCC_VERSION);

    kcc::Log::Scope scope(KCC_FILE, "main");
    kcc::Log::out("begin rege testing");
    try
    {

    	kcc::IRegex* rx = kcc::Core::regex();
        kcc::StringVector tokens;
        kcc::StringParts parts;
        kcc::String target, expr;

        //
        // regex: match
        //

        target = "foo.bar.txt";
        expr   = "^(.+)\\.txt$";
        kcc::Log::out("\n---rx match (succeed) '%s' ~= '%s'", target.c_str(), expr.c_str());
        kcc::Log::out("matched: %s", rx->match(target, expr) ? "yes" : "no");

        target = "   1234 word";
        expr   = "^\\s*(\\d+)\\s+(.+)$";
        kcc::Log::out("\n---rx match (with results) '%s' ~= '%s'", target.c_str(), expr.c_str());
        kcc::Log::out("matched: %s", rx->match(target, expr, parts) ? "yes" : "no");
        for (kcc::StringParts::size_type i = 0; i < parts.size(); i++)
            kcc::Log::out("[%s]", kcc::String(parts[i].first, parts[i].second).c_str());

        target = "foo.bar.txt";
        expr   = "^(.+)\\.zap$";
        kcc::Log::out("\n---rx match (fail) '%s' ~= '%s'", target.c_str(), expr.c_str());
        kcc::Log::out("matched: %s\n", rx->match(target, expr) ? "yes" : "no");

        match("foo", "bar foo zap");
        match("willnotmatch", "bar foo zap");
        match("(foo)", "bar foo zap");

        match("^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?", "http://foo.bar.com:8300/zap?zing=blat#5150");
        match("(\\bfoo\\b)", "now is the foo for bar men to zap to the aid of their foo");
        match("(\\bfoo\\b|\\bbar\\b|\\bzap\\b)", "now is the foo for bar men to zap to the aid of their foo");
        match("willnotmatch", "while i appreciate your POV");

        expr = "online \"online program\" free \"free ipod\" game";
        kcc::Strings::csv(expr, tokens, ' ');
        kcc::Strings::rxEscape(tokens);
        expr = "(\\b" + kcc::Strings::join("\\b|\\b", tokens.begin(), tokens.end()) + "\\b)";
        match(
            expr,
            "you know those gay little advertisments on myspace...that act like they are game. "
            "like kiss brad pit and when a free ipod. or throw the football through the hoop and "
            "get a free ring tone. The ones that you know are a bunch if crap...but you usually play "
            "it just once anyways because you can't resist playing such an easy GAME. Well anyways...now "
            "that you know what i am talking about. Blockbuster has one of those now. Catch the dvd from the "
            "mail box and when a month of the online program. ridiculous, and weird.");

        //
        // regex: split
        //

        target = "  foo bar.zar\tbat\nlax  ";
        expr   = "\\W+";
        kcc::Log::out("\n---rx split (immutable) '%s' with '%s'", target.c_str(), expr.c_str());
        rx->split((const kcc::String&)target, expr, tokens);
        for (kcc::StringVector::size_type i = 0; i < tokens.size(); i++)
            kcc::Log::out("[%s]", tokens[i].c_str());

        target = "foo.bar.zap";
        expr   = "\\.+";
        kcc::Log::out("\n---rx split (immutable) '%s' with '%s'", target.c_str(), expr.c_str());
        rx->split((const kcc::String&)target, expr, tokens);
        kcc::Log::out("result after split [%s]", target.c_str());
        for (kcc::StringVector::size_type i = 0; i < tokens.size(); i++)
            kcc::Log::out("[%s]", tokens[i].c_str());

        kcc::Log::out("\n---rx split (mutable) '%s' with '%s'", target.c_str(), expr.c_str());
        rx->split(target, expr, tokens);
        kcc::Log::out("result after split [%s]", target.c_str());
        for (kcc::StringVector::size_type i = 0; i < tokens.size(); i++)
            kcc::Log::out("[%s]", tokens[i].c_str());

        //
        // regex: replace
        //

        kcc::String sub("   boo    ");
        kcc::String rx1("^\\W+");
        kcc::String rx2("\\W+$");
        kcc::Log::out("\n---replace '%s' using '%s' and '%s' with ''", sub.c_str(), rx1.c_str(), rx2.c_str());
        rx->replace(sub, rx1);
        rx->replace(sub, rx2);
        kcc::Log::out("[%s]", sub.c_str());
    }
    catch (std::exception& e)
    {
        kcc::Log::exception(e);
        return 1;
    }
    return 0;
}
