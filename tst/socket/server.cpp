#include <inc/core/Core.h>

#define KCC_FILE    "server"
#define KCC_VERSION "$Id: server.cpp 23304 2008-04-28 15:13:12Z tvk $"

struct Command : kcc::Thread
{
    kcc::Socket c;
    Command(kcc::Socket::Handle h) : c(h) {}
    void invoke()
    {
        kcc::Log::Scope scope(KCC_FILE, "Command::invoke");
        try
        {
            long cmd = 0L;
            c.read(cmd);
            kcc::Log::out("server received %d", cmd);

            std::time_t time;
            std::time(&time);
            switch (cmd)
            {
                case 1L:
                {
                    c.write((long)time);                    
                    kcc::Log::out("server wrote %s", kcc::ISODate::local(time).isodatetime().c_str());
                    break;
                }
                case 2L:
                {
                    kcc::String text(kcc::Strings::printf(
                        "hello from server @ %s",
                        kcc::ISODate::local(time).isodatetime().c_str()));
                    c.write(text);
                    kcc::Log::out("server wrote %s", text.c_str());
                    break;
                }
                case 5150L:
                {
                    // test uncaught exception
                    throw "die badly";
                }
                default:
                {
                    kcc::Log::out("unrecognized server command");
                }
            }
        }
        catch (kcc::Exception& e)
        {
            kcc::Log::exception(e);
        }
    }
};

struct Server : kcc::Thread
{
    kcc::SocketServer s;
    Server(const kcc::String& host, int port) : s(host, port) {}
    void invoke()
    {
        kcc::Log::Scope scope(KCC_FILE, "Server::invoke");
        try
        {
            s.listen();
            while (true) 
            {
                kcc::Socket::Handle h = s.accept();
                (new Command(h))->go();
            }
        }
        catch (kcc::SocketServer::Closed&)
        {
            // server closed normally
        }
        catch (kcc::Exception& e)
        {
            kcc::Log::exception(e);
        }
    }
    void start() { go(); }
    void stop()  { s.close(); }
};

int main(int argc, const char* argv[])
{
    kcc::Properties props;
    props.set("kcc.logVerbosity", (long) kcc::Log::V_INFO_3);
    props.set("kcc.logMax",       1L);
    props.set("kcc.LogName",      KCC_FILE);
    if (argc > 1) props.load(argc, argv, false);
    kcc::Core::init(props, KCC_VERSION);

    kcc::String host("localhost");
    if (argc > 1) host = argv[1];
    int port = 5150;
    if (argc > 2) port = kcc::Strings::parseInteger(argv[2]);

    kcc::Log::Scope scope(KCC_FILE, "main");
    kcc::Log::out("server testing on [%s]. commands [stop]", kcc::Socket::getHostName().c_str());
    try
    {
        Server* s = new Server(host, port);
        s->start();

        kcc::String msg;
        while (msg != "stop") std::getline(std::cin, msg);

        s->stop();
        kcc::Thread::sleep(1000L);

        kcc::Log::out("completed server testing");
    }
    catch (std::exception& e)
    {
        kcc::Log::exception(e);
        return 1;
    }

    return 0;
}
