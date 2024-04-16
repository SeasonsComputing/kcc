#include <inc/core/Core.h>
#include <inc/zip/IZip.h>

#define KCC_FILE    "bz2"
#define KCC_VERSION "$Id: bz2.cpp 15199 2007-03-09 17:57:17Z tvk $"

int main(int argc, const char* argv[])
{
    kcc::Properties props;
    props.set("kcc.logVerbosity", (long) kcc::Log::V_INFO_3);
    props.set("kcc.logMax",       1L);
    props.set("kcc.LogName",      KCC_FILE);
    kcc::Core::init(props, KCC_VERSION);

    bool c = false;
    bool d = false;
    kcc::String inf;
    kcc::String outf;
    if (argc == 4)
    {
        kcc::Properties config;
        config.load(argc, argv);
        c = config.get("c", KCC_PROPERTY_FALSE) == KCC_PROPERTY_TRUE;
        d = config.get("d", KCC_PROPERTY_FALSE) == KCC_PROPERTY_TRUE;
        inf.assign(config.get("in", kcc::Strings::empty()));
        outf.assign(config.get("out", kcc::Strings::empty()));
    }

    if ((!c && !d) || inf.empty() || outf.empty())
    {
        std::cerr << "Usage: bz2 c|d in=input out=output\n";
        return 1;
    }

    try
    {
        kcc::IZipFactory* zip = KCC_FACTORY(kcc::IZipFactory, "k_bzip2");

        if (c)
        {
            char buf[1024];
            FILE* in = std::fopen(inf.c_str(), "rb");
            if (in == NULL) throw kcc::Exception("unable to open in file");
            kcc::AutoPtr<kcc::IZipWriter> out(zip->constructWriter());
            if (!out->open(outf)) throw kcc::Exception("unable to open out file");
            while (!feof(in) && out->ok())
            {
                int read = (int)std::fread(buf, sizeof(char), 1024, in);
                out->write(buf, read);
            }
            out->close();
            std::fclose(in);
        }

        if (d)
        {
            FILE* out = std::fopen(outf.c_str(), "wb");
            if (out == NULL) throw kcc::Exception("unable to open out file");
            kcc::AutoPtr<kcc::IZipReader> in(zip->constructReader());
            if (!in->open(inf)) throw kcc::Exception("unable to open in file");
            while (in->ok())
            {
                int read;
                char* buf = in->read(read);
                std::fwrite(buf, sizeof(char), read, out);
            }
            in->close();
            std::fclose(out);
        }
    }
    catch (std::exception& e)
    {
        kcc::Log::exception(e);
        return 1;
    }

    return 0;
}
