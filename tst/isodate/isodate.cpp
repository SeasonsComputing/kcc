#include <inc/core/Core.h>

#define KCC_FILE    "isodate"
#define KCC_VERSION "$Id: isodate.cpp 20676 2007-09-12 15:12:45Z tvk $"

inline kcc::Char YN(bool ok) { return ok ? 'Y' : 'N'; }

int main(int argc, const char* argv[])
{
    kcc::Properties props;
    props.set("kcc.logVerbosity", (long) kcc::Log::V_INFO_3);
    props.set("kcc.logMax",       1L);
    props.set("kcc.LogName",      KCC_FILE);
    if (argc > 1) props.load(argc, argv, false);
    kcc::Core::init(props, KCC_VERSION);

    kcc::Log::Scope scope(KCC_FILE, "main");
    kcc::Log::out("begin isodate testing");
    try
    {
        kcc::ISODate loc(kcc::ISODate::local());
        kcc::ISODate utc(kcc::ISODate::utc());
        kcc::String val;

        //
        // valid
        //

        kcc::Log::out("\n**** VALID **** ");
        kcc::StringRC dates[] =
        {
            "2006-10-10",
            "08:12:12",
            "2006-10-10 08:12:12",
            kcc::ISODate::local().isodatetime(kcc::ISODate::F_TIMEZONE),
            kcc::ISODate::local().isodatetime(kcc::ISODate::F_ZULU),
            kcc::ISODate::utc().isodatetime(),
            "bogus",
            "06/99/21",
            "06/99/21 GMT 12:21:21",
            ""
        };
        for (int i = 0; !dates[i].empty(); i++)
        {
            const kcc::String& value = dates[i];
            bool valid = kcc::ISODate::valid(value);
            kcc::Log::out(
                "value:%-30s format-valid:%c value-valid:%c", 
                value.c_str(), 
                YN(valid),
                (valid ? YN(kcc::ISODate::iso(value).valid()) : YN(false)));
        }
        
        //
        // format
        //
        
        kcc::Log::out("\n**** EXTENDED FORMAT **** ");
        kcc::Log::out("local       :%-25s", loc.isodatetime().c_str());
        kcc::Log::out("local (tz)  :%-25s", loc.isodatetime(kcc::ISODate::F_TIMEZONE).c_str());
        kcc::Log::out("local (zulu):%-25s", loc.isodatetime(kcc::ISODate::F_ZULU).c_str());
        kcc::Log::out("utc         :%-25s", utc.isodatetime().c_str());
        
        kcc::Log::out("\n**** BASIC FORMAT **** ");
        kcc::Log::out("local       :%-25s", loc.rawdatetime().c_str());
        kcc::Log::out("local (tz)  :%-25s", loc.rawdatetime(kcc::ISODate::F_TIMEZONE).c_str());
        kcc::Log::out("local (zulu):%-25s", loc.rawdatetime(kcc::ISODate::F_ZULU).c_str());
        kcc::Log::out("utc         :%-25s", utc.rawdatetime().c_str());
        
        //
        // parse
        //

        kcc::Log::out("\n**** PARSE **** ");
        
        val = kcc::ISODate::local().isodatetime();
        loc = kcc::ISODate::iso(val);
        kcc::Log::out("value       :%-25s", val.c_str());
        kcc::Log::out("local       :%-25s", loc.isodatetime().c_str());
        kcc::Log::out("local (tz)  :%-25s", loc.isodatetime(kcc::ISODate::F_TIMEZONE).c_str());
        kcc::Log::out("local (zulu):%-25s", loc.isodatetime(kcc::ISODate::F_ZULU).c_str());
        
        val = kcc::ISODate::local().isodatetime(kcc::ISODate::F_ZULU);
        loc = kcc::ISODate::iso(val);
        kcc::Log::out("\nvalue       :%-25s", val.c_str());
        kcc::Log::out("local       :%-25s", loc.isodatetime().c_str());
        kcc::Log::out("local (tz)  :%-25s", loc.isodatetime(kcc::ISODate::F_TIMEZONE).c_str());
        kcc::Log::out("local (zulu):%-25s", loc.isodatetime(kcc::ISODate::F_ZULU).c_str());
        
        val = kcc::ISODate::local().isodatetime(kcc::ISODate::F_TIMEZONE);
        loc = kcc::ISODate::iso(val);
        kcc::Log::out("\nvalue       :%-25s", val.c_str());
        kcc::Log::out("local       :%-25s", loc.isodatetime().c_str());
        kcc::Log::out("local (tz)  :%-25s", loc.isodatetime(kcc::ISODate::F_TIMEZONE).c_str());
        kcc::Log::out("local (zulu):%-25s", loc.isodatetime(kcc::ISODate::F_ZULU).c_str());
        
        val = "2006-06-19T14:52:00Z";
        loc = kcc::ISODate(val);
        kcc::Log::out("\nvalue       :%-25s", val.c_str());
        kcc::Log::out("local       :%-25s", loc.isodatetime().c_str());
        kcc::Log::out("local (tz)  :%-25s", loc.isodatetime(kcc::ISODate::F_TIMEZONE).c_str());
        kcc::Log::out("local (zulu):%-25s", loc.isodatetime(kcc::ISODate::F_ZULU).c_str());
        
        val = "2006-06-19T14:52:13.123-06:00";
        loc = kcc::ISODate(val);
        kcc::Log::out("\nvalue       :%-25s", val.c_str());
        kcc::Log::out("local       :%-25s", loc.isodatetime().c_str());
        kcc::Log::out("local (tz)  :%-25s", loc.isodatetime(kcc::ISODate::F_TIMEZONE).c_str());
        kcc::Log::out("local (zulu):%-25s", loc.isodatetime(kcc::ISODate::F_ZULU).c_str());

        kcc::Log::out("\ncompleted isodate testing");
    }
    catch (std::exception& e)
    {
        kcc::Log::exception(e);
        return 1;
    }

    return 0;
}
