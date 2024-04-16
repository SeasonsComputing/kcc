#include <inc/core/Core.h>

#define KCC_FILE    "client"
#define KCC_VERSION "$Id: client.cpp 20677 2007-09-12 15:21:44Z tvk $"

int main(int argc, const char* argv[])
{
    kcc::Properties props;
    props.set("kcc.logVerbosity", (long) kcc::Log::V_INFO_3);
    props.set("kcc.logMax",       2L);
    props.set("kcc.LogName",      KCC_FILE);
    if (argc > 1) props.load(argc, argv, false);
    kcc::Core::init(props, KCC_VERSION);

    kcc::String host("localhost");
    if (argc > 1) host = argv[1];
    int port = 5150;
    if (argc > 2) port = kcc::Strings::parseInteger(argv[2]);

    kcc::Log::Scope scope(KCC_FILE, "main");
    kcc::Log::out("client testing. commands [time, text, stop]");
    try
    {
        while (true)
        {
            kcc::String msg;
            std::getline(std::cin, msg);
            if (msg == "stop") break;

            kcc::Socket s(host, port);
            s.connect();
            if (msg == "time")
            {
                long time = 0L;
                s.write(1L);
                s.read(time);
                kcc::Log::out("server time: %s", kcc::ISODate::local((std::time_t)time).isodatetime().c_str());
            }
            else if (msg == "text")
            {
                s.write(2L);
                char text[256];
                std::memset(text, 0, 255);
                int actual;
                s.read(text, 255, actual);
                kcc::Log::out("server text: %s", text);
            }
            else if (msg == "die badly")
            {
                s.write(5150L);
                kcc::Log::out("forced server exception");
            }
            else
            {
                std::cout << "unrecognized server command: " << msg << std::endl;
            }
        }
        kcc::Log::out("completed client testing");
    }
    catch (std::exception& e)
    {
        kcc::Log::exception(e);
        return 1;
    }

    return 0;
}
