#include <inc/core/Core.h>

#define KCC_FILE    "timer"
#define KCC_VERSION "$Id: "

int main(int argc, const char* argv[])
{
    kcc::Properties props;
    props.set("kcc.logVerbosity", (long) kcc::Log::V_INFO_3);
    props.set("kcc.logMax",       1L);
    props.set("kcc.LogName",      KCC_FILE);
    if (argc > 1) props.load(argc, argv, false);
    kcc::Core::init(props, KCC_VERSION);

    kcc::Log::Scope scope(KCC_FILE, "main");
    kcc::Log::out("begin timer testing");
    try
    {
        kcc::NamedTimers t;
        kcc::Log::out("timing...");

        t["total"].start();

        for (int i = 0; i < 3; i++)
        {
            t["standard"].start();
            kcc::Thread::sleep(250L);
            t["standard"].stop();
        }

        t["reentrant"].start();
        kcc::Thread::sleep(500L);
        // REENTRANT IS BROKEN -- DOES NOT WORK!!!
        //t["reentrant"].start();
        //kcc::Thread::sleep(250L);
        //t["reentrant"].stop();
        t["reentrant"].stop();

        t["total"].stop();

        for (kcc::NamedTimers::iterator i = t.begin(); i != t.end(); i++)
        {
            kcc::Log::out(
                "%10s: size=[%d] sum=[%.3f] mean=[%.3f] median=[%.3f]",
                i->first, i->second.size(), i->second.sum(), i->second.mean(), i->second.median());
        }
    }
    catch (std::exception& e)
    {
        kcc::Log::exception(e);
        return 1;
    }

    return 0;
}
