#include <inc/core/Core.h>

#define KCC_FILE    "thread"
#define KCC_VERSION "$Id: thread.cpp 20677 2007-09-12 15:21:44Z tvk $"

struct Manager : kcc::IThreadManager
{
    kcc::Monitor& m_end;
    Manager(kcc::Monitor& end) : m_end(end) {}
    void onThreadDeleted(const kcc::Char* name)
    {
        kcc::Log::Scope scope(KCC_FILE, "Manager::onThreadDeleted()");
        kcc::Log::out("deleting thread: %s", name);
        if (kcc::String(name) != "c") m_end.notify();
    }
};

struct Thread : kcc::Thread
{
    int           m_size;
    kcc::Monitor& m_start;
    kcc::Monitor& m_end;
    Thread(const kcc::Char* n, kcc::IThreadManager* m, int size, kcc::Monitor& start, kcc::Monitor& end)
        : kcc::Thread(n, m), m_size(size), m_start(start), m_end(end)
    { 
        m_end.init(); 
    }
    Thread(const kcc::Char* n, int size, kcc::Monitor& start, kcc::Monitor& end)
        : kcc::Thread(n), m_size(size), m_start(start), m_end(end)
    { 
        m_end.init(); 
    }
    void invoke()
    {
        m_start.wait();
        for (int i = 0; i < m_size; i++)
        {
            // thread kill() will invalidate scope object and throw off scope stack
            // so ensure it is not on the stack when killed
            {
                kcc::Log::Scope scope(KCC_FILE, "Thread::invoke");
                kcc::Log::info1("thread [%s] %d", name(), i);
            }
            if (kcc::String(name()) == "c" && i == 15)
            {
                m_end.notify();
                while (true) kcc::Thread::sleep(255); // wait for kill thread
            }
        }
    }
};

struct Joiner : kcc::IThread
{
    kcc::Monitor start;
    Joiner() { start.init(); }
    void invoke()
    {
        start.wait();
        kcc::Log::Scope scope(KCC_FILE, "Joiner::invoke");
        for (int i = 0; i < 50; i++)
        {
            kcc::Log::info1("joiner %d", i);
        }
    }
};

int main(int argc, const char* argv[])
{
    kcc::Properties props;
    props.set("kcc.logVerbosity", (long) kcc::Log::V_INFO_3);
    props.set("kcc.logMax",       1L);
    props.set("kcc.LogName",      KCC_FILE);
    if (argc > 1) props.load(argc, argv, false);
    kcc::Core::init(props, KCC_VERSION);

    kcc::Log::Scope scope(KCC_FILE, "main");
    kcc::Log::out("begin thread testing");
    try
    {
        kcc::Monitor end;
        Manager m(end);

        kcc::Monitor start;
        start.init();
        Thread* c = NULL;

        (new Thread("a", &m, 100, start, end))->go();
        (c = new Thread("c", &m, 20, start, end))->go();
        (new Thread("b", &m, 25, start, end))->go();
        (new Thread("d", &m, 100, start, end))->go();

        start.notifyAll(); // start all at once
        end.wait();        // wait for all to complete

        c->kill();         // all completed so test thread kill

        Joiner j;
        kcc::Thread* run = new kcc::Thread(&j, "joiner");
        run->go(true);
        j.start.notify(); // don't let finish prior to join
        kcc::Thread::join(run);
 
        kcc::Log::out("completed thread testing");
    }
    catch (std::exception& e)
    {
        kcc::Log::exception(e);
        return 1;
    }

    return 0;
}
