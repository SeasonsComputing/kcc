#include <inc/core/Core.h>

#define KCC_FILE    "string"
#define KCC_VERSION "$Id: string.cpp 21778 2007-12-27 00:55:36Z tvk $"

int main(int argc, const char* argv[])
{
    kcc::Properties props;
    props.set("kcc.logVerbosity", (long) kcc::Log::V_INFO_3);
    props.set("kcc.logMax",       1L);
    props.set("kcc.LogName",      KCC_FILE);
    if (argc > 1) props.load(argc, argv, false);
    kcc::Core::init(props, KCC_VERSION);

    kcc::Log::Scope scope(KCC_FILE, "main");
    kcc::Log::out("begin string testing");
    try
    {
        kcc::String s;
        std::time_t t = 0L;
        kcc::StringVector tokens;
        kcc::String seps;

        //
        // toLower, toUpper
        //

        s.assign("LOWER 3124 xyz");
        kcc::Log::out("\n---toLower [%s]", kcc::Strings::toLower(s).c_str());
        s.assign("upper 4321 XYZ");
        kcc::Log::out("\n---toUpper [%s]", kcc::Strings::toUpper(s).c_str());

        //
        // trimws, trim
        //

        s.assign("  xx trimmed\t\n");
        kcc::Log::out("\n---trimws [%s]", kcc::Strings::trimws(s).c_str());
        s.assign("{!*[2002]}\\");
        kcc::Log::out("\n---trim [%s]", kcc::Strings::trim(s).c_str());

        //
        // printf
        //

        std::time(&t);
        kcc::Log::out(kcc::Strings::printf("\n---printf [%s %d 0x%x]", "X", 1, 5150));

        //
        // tokenize, csv
        //

        s.assign("nothing");
        seps.assign("*");

        kcc::Log::out("\n---tokenize '%s' using '%s'", s.c_str(), seps.c_str());
        kcc::Strings::tokenize(s, seps, tokens);
        for (kcc::StringVector::size_type i = 0; i < tokens.size(); i++)
            kcc::Log::out("[%s]", tokens[i].c_str());

        s.assign("foo=bar");
        seps.assign("=");

        kcc::Log::out("\n---tokenize '%s' using '%s'", s.c_str(), seps.c_str());
        kcc::Strings::tokenize(s, seps, tokens);
        for (kcc::StringVector::size_type i = 0; i < tokens.size(); i++)
            kcc::Log::out("[%s]", tokens[i].c_str());

        s.assign("  foo bar.zar\tbat\nlax  ");
        seps.assign(" \t\n.");

        kcc::Log::out("\n---tokenize '%s' using '%s'", s.c_str(), seps.c_str());
        kcc::Strings::tokenize(s, seps, tokens);
        for (kcc::StringVector::size_type i = 0; i < tokens.size(); i++)
            kcc::Log::out("[%s]", tokens[i].c_str());

        s.assign("foo,bar,5150,some text,\"now is \"\"the time\"\" for all\ngood men,to,come,to\nthe,,aid\",2122");
        seps.assign(",");

        kcc::Log::out("\n---csv '%s' using '%s'", s.c_str(), seps.c_str());
        kcc::Strings::csv(s, tokens, seps.at(0));
        for (kcc::StringVector::size_type i = 0; i < tokens.size(); i++)
            kcc::Log::out("[%s]", tokens[i].c_str());

        s.assign("foo|bar|5150|some text|\"now is \"\"the time\"\" for all\ngood men|to|come|to\nthe||aid\"|2122");
        seps.assign("|");

        kcc::Log::out("\n---csv '%s' using '%s'", s.c_str(), seps.c_str());
        kcc::Strings::csv(s, tokens, seps.at(0));
        for (kcc::StringVector::size_type i = 0; i < tokens.size(); i++)
            kcc::Log::out("[%s]", tokens[i].c_str());

        s.assign
        (
            "1,bar,5150,,1,,2122,\"text1\"\xd\xa" // linux eol
            "2,bar,5150,,2,,2122,\xd\xa"          // linux eol
            "3,bar,5150,,3,,2122,\"text3\"\xd\xa" // linux eol
            "4,bar,5150,,4,,2122,\xa"             // win32 eol
            "5,bar,5150,,5,,2122,\"text5\"\xa"    // win32 eol
        );
        seps.assign(",");

        kcc::Log::out("\n---multi-line csv using '%s'\n{\n%s}", seps.c_str(), s.c_str());

        kcc::StringStream csv(s);
        while (kcc::Strings::csv(csv, tokens, seps.at(0)))
        {
            kcc::Log::out("[\"%s\"]", kcc::Strings::join("\",\"", tokens.begin(), tokens.end()).c_str());
        }

        //
        // join
        //

        tokens.clear();
        tokens.push_back("apple");
        tokens.push_back("orange");
        tokens.push_back("grape");
        kcc::Log::out("\n---join");
        kcc::String join(kcc::Strings::join(":", tokens.begin(), tokens.end()));
        kcc::Log::out(join.c_str());

        //
        // set
        //

        kcc::Log::out("\n---StringSet");
        kcc::StringSet set;
        set.insert("zoo");
        set.insert("apple");
        set.insert("lollipop");
        for (
            kcc::StringSet::iterator i = set.begin();
            i != set.end();
            i++)
        {
            kcc::Log::out("[%s]", (*i).c_str());
        }

        //
        // RC
        //

        kcc::StringRC x("tvk");
        kcc::StringMultisetRC xs;
        xs.insert(x);
        xs.insert(x);
        xs.insert(x);
        xs.insert(x);
        kcc::Log::out("\n---StringMultisetRC size = %d, rc = %d", xs.size(), x.rc());

        kcc::StringRC y = x + x;
        kcc::Log::out("\n---StringRC y = x + x --> %d", y.rc());

        kcc::StringRC p = x;
        kcc::StringRC z("tvk");
        kcc::Log::out("\n---StringRC op == (rc'd) %d", (x == p));
        kcc::Log::out("\n---StringRC op == (data) %d", (x == z));
        
        //
        // select, match
        //

        #define SELDEF(n) (n==kcc::Strings::SD_ENUM?"enum":"equal")
        kcc::String val, exp;

        // select: default -- enum
        kcc::Strings::SelectDefault sd = kcc::Strings::SD_ENUM;
        val = "false";
        exp = "true|false"; 
        kcc::Log::out(
            "\n---select value=[%s] expr=[%s] def=[%s] match=[%d]", 
            val.c_str(), exp.c_str(), SELDEF(sd), kcc::Strings::select(val, exp, sd));
        val = "null";
        exp = "true|null|false";
        kcc::Log::out(
            "\n---select value=[%s] expr=[%s] def=[%s] match=[%d]", 
            val.c_str(), exp.c_str(), SELDEF(sd), kcc::Strings::select(val, exp, sd));
        val = "bar";
        exp = "bar";
        kcc::Log::out(
            "\n---select value=[%s] expr=[%s] def=[%s] match=[%d]", 
            val.c_str(), exp.c_str(), SELDEF(sd), kcc::Strings::select(val, exp, sd));
        val = "bar";
        exp = "not|gonna|find";
        kcc::Log::out(
            "\n---select value=[%s] expr=[%s] def=[%s] match=[%d]", 
            val.c_str(), exp.c_str(), SELDEF(sd), kcc::Strings::select(val, exp, sd));

        // select: default -- equal
        sd = kcc::Strings::SD_EQUAL;
        val = "bar";
        exp = "bar"; 
        kcc::Log::out(
            "\n---select value=[%s] expr=[%s] def=[%s] match=[%d]", 
            val.c_str(), exp.c_str(), SELDEF(sd), kcc::Strings::select(val, exp, sd));
        val = "barX";
        kcc::Log::out(
            "\n---select value=[%s] expr=[%s] def=[%s] match=[%d]", 
            val.c_str(), exp.c_str(), SELDEF(sd), kcc::Strings::select(val, exp, sd));

        // select: equal
        val = "bar";
        exp = "@:bar";
        kcc::Log::out(
            "\n---select value=[%s] expr=[%s] def=[%s] match=[%d]", 
            val.c_str(), exp.c_str(), SELDEF(sd), kcc::Strings::select(val, exp, sd));
        val = "barX";
        kcc::Log::out(
            "\n---select value=[%s] expr=[%s] def=[%s] match=[%d]", 
            val.c_str(), exp.c_str(), SELDEF(sd), kcc::Strings::select(val, exp, sd));

        // select: not-equal
        val = "zap";
        exp = "!:bar";
        kcc::Log::out(
            "\n---select value=[%s] expr=[%s] def=[%s] match=[%d]", 
            val.c_str(), exp.c_str(), SELDEF(sd), kcc::Strings::select(val, exp, sd));
        val = "bar";
        kcc::Log::out(
            "\n---select value=[%s] expr=[%s] def=[%s] match=[%d]", 
            val.c_str(), exp.c_str(), SELDEF(sd), kcc::Strings::select(val, exp, sd));

        // select: begins-with
        val = "bar.Zing";
        exp = "^:bar.";
        kcc::Log::out(
            "\n---select value=[%s] expr=[%s] def=[%s] match=[%d]", 
            val.c_str(), exp.c_str(), SELDEF(sd), kcc::Strings::select(val, exp, sd));
        val = "barX.Zing";
        kcc::Log::out(
            "\n---select value=[%s] expr=[%s] def=[%s] match=[%d]", 
            val.c_str(), exp.c_str(), SELDEF(sd), kcc::Strings::select(val, exp, sd));

        // select: ends-with
        val = "bar.Zing";
        exp = "$:.Zing";
        kcc::Log::out(
            "\n---select value=[%s] expr=[%s] def=[%s] match=[%d]", 
            val.c_str(), exp.c_str(), SELDEF(sd), kcc::Strings::select(val, exp, sd));
        val = "bar.ZingX";
        kcc::Log::out(
            "\n---select value=[%s] expr=[%s] def=[%s] match=[%d]", 
            val.c_str(), exp.c_str(), SELDEF(sd), kcc::Strings::select(val, exp, sd));

        // select: contains
        val = "bar.Zing.blat";
        exp = "~:.Zing.";
        kcc::Log::out(
            "\n---select value=[%s] expr=[%s] def=[%s] match=[%d]", 
            val.c_str(), exp.c_str(), SELDEF(sd), kcc::Strings::select(val, exp, sd));

        // select: enum
        val = "bar";
        exp = "|:zap|bar|foo";
        kcc::Log::out(
            "\n---select value=[%s] expr=[%s] def=[%s] match=[%d]", 
            val.c_str(), exp.c_str(), SELDEF(sd), kcc::Strings::select(val, exp, sd));

        // done
        kcc::Log::out("\ncompleted string testing");
    }
    catch (std::exception& e)
    {
        kcc::Log::exception(e);
        return 1;
    }

    return 0;
}
