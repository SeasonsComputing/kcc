#include <inc/core/Core.h>

#define KCC_FILE    "ramusage"
#define KCC_VERSION "$Id: ramusage.cpp 20678 2007-09-12 15:32:56Z tvk $"

int main(int argc, const char* argv[])
{
    kcc::Properties props;
    props.set("kcc.logVerbosity", (long) kcc::Log::V_INFO_3);
    props.set("kcc.logMax",       1L);
    props.set("kcc.LogName",      KCC_FILE);
    if (argc > 1) props.load(argc, argv, false);
    kcc::Core::init(props, KCC_VERSION);

    kcc::Log::Scope scope(KCC_FILE, "main");
    kcc::Log::out("begin ramusage testing");
    try
    {
        kcc::Log::out("\nSTART:\t%d\n=============", kcc::Platform::procMemInUseKB());
        std::cout << "PRESS ANY KEY";
        kcc::String line;
        std::getline(std::cin, line);
        {
            kcc::StringVector allocs;
            kcc::Timers t;
            for (int i = 0; i < 25; i++)
            {
                t.start();
                long ram = kcc::Platform::procMemInUseKB();
                t.stop();
                allocs.push_back(kcc::String());
                allocs.back().resize(1024*1024*10, '@'); // 10MB
                kcc::Log::out("%2d\t%10d\t\t%.3f", i, ram, t.secs());
            }
            kcc::Log::out("============= time-total: %.3f time-avg: %.3f", t.sum(), t.mean());
            std::cout << "PRESS ANY KEY";
            std::getline(std::cin, line);
        }
        kcc::Log::out("=============\nEND:\t%d", kcc::Platform::procMemInUseKB());
    }
    catch (std::bad_alloc& e)
    {
        kcc::Log::exception(e);
        return 2;
    }
    catch (std::exception& e)
    {
        kcc::Log::exception(e);
        return 1;
    }

    return 0;
}
