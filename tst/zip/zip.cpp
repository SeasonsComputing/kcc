#include <inc/core/Core.h>
#include <inc/zip/IZip.h>

#define KCC_FILE    "zip"
#define KCC_VERSION "$Id: zip.cpp 17018 2007-05-02 19:45:34Z tvk $"

// buffer size
const int SZ = 1024;

int main(int argc, const char* argv[])
{
    kcc::Properties props;
    props.set("kcc.logVerbosity", (long) kcc::Log::V_INFO_3);
    props.set("kcc.logMax",       1L);
    props.set("kcc.LogName",      KCC_FILE);
    if (argc > 1) props.load(argc, argv, false);
    kcc::Core::init(props, KCC_VERSION);

    // command line params
    bool c = props.get("c", KCC_PROPERTY_FALSE) == KCC_PROPERTY_TRUE;
    bool d = props.get("d", KCC_PROPERTY_FALSE) == KCC_PROPERTY_TRUE;
    kcc::String inf (props.get("in",   kcc::Strings::empty()));
    kcc::String outf(props.get("out",  kcc::Strings::empty()));
    kcc::String type(props.get("type", "bzip2"));
    if ((!c && !d) || (c && d) || inf.empty() || outf.empty() || type.empty())
    {
        std::cerr << "Usage: zip c|d in=input out=output (type=gzip|bzip2)\n";
        return 1;
    }

    kcc::Log::Scope scope(KCC_FILE, "main");
    try
    {
        // zip provider
        kcc::IZipFactory* zip = KCC_FACTORY(kcc::IZipFactory, (type == "bzip2" ? "k_bzip2" : "k_zlib"));
        
        if (c)
        {
            // compress
            char buf[SZ];
            FILE* in = std::fopen(inf.c_str(), "rb");
            if (in == NULL) throw kcc::Exception("unable to open in file");
            kcc::AutoPtr<kcc::IZipWriter> out(zip->constructWriter());
            if (!out->open(outf)) throw kcc::Exception("unable to open out file");
            while (!feof(in) && out->ok())
            {
                int read = (int)std::fread(buf, sizeof(char), SZ, in);
                out->write(buf, read);
            }
            out->close();
            std::fclose(in);
        }
        
        if (d)
        {
            // decompress
            FILE* out = std::fopen(outf.c_str(), "wb");
            if (out == NULL) throw kcc::Exception("unable to open out file");
            kcc::AutoPtr<kcc::IZipReader> in(zip->constructReader());
            if (!in->open(inf)) throw kcc::Exception("unable to open in file");

            // alternate buffers to make sure both read API's work
            char secondBuf[SZ];
            bool useSecondBuffer = false;            
            while (in->ok())
            {
                int read;
                if (useSecondBuffer)
                {
                    const char* buf = in->read(read);
                    std::fwrite(buf, sizeof(char), read, out);
                    useSecondBuffer = !useSecondBuffer;
                }
                else
                {
                    in->read(secondBuf, SZ, read);
                    std::fwrite(secondBuf, sizeof(char), read, out);
                    useSecondBuffer = !useSecondBuffer;
                }
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
