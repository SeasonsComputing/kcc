/*
 * Kuumba C++ Core
 *
 * $Id: BZip2.cpp 22625 2008-03-09 22:51:49Z tvk $
 */
#include <inc/core/Core.h>
#include <inc/zip/IZip.h>

namespace bzip2
{
    #if defined(KCC_WINDOWS)
    #   include "winbz2/bzlib.h"
    #elif defined(KCC_LINUX)
    #   include "bzlib.h"
    #endif
};

#define KCC_FILE    "BZip2"
#define KCC_VERSION "$Id: BZip2.cpp 22625 2008-03-09 22:51:49Z tvk $"

namespace kcc
{
    // buffer size
    const int SZ = 4096;

    //
    // BZip implementation
    //

    struct BZip2
    {
        // Attributes
        int     m_status;
        FILE*   m_file;
        bzip2::BZFILE* m_bz;

        // ctor/dtor
        BZip2() : m_status(BZ_OK), m_file(NULL), m_bz(NULL) {}
        virtual ~BZip2() { close(); }

        // ok: query status of bzip
        bool ok() { return m_status == BZ_OK; }

        // close: close file stream
        void close()
        {
            if (m_file != NULL) std::fclose(m_file);
            m_file = NULL;
        }
    };

    //
    // BZipReader implementation
    //

    struct BZip2Reader : BZip2, IZipReader
    {
        // Attributes
        char* m_buf;

        // ctor/dtor
        BZip2Reader() : m_buf(new char[SZ]) {}

        // ok: query status of bzip
        bool ok() { return BZip2::ok(); }

        // close: close bzip reader stream
        void close()
        {
            if (m_bz != NULL)  bzip2::BZ2_bzReadClose(&m_status, m_bz);
            if (m_buf != NULL) delete [] m_buf;
            m_bz  = NULL;
            m_buf = NULL;
            BZip2::close();
        }

        // open: open bzip reader stream
        bool open(const String& file)
        {
            Log::Scope scope(KCC_FILE, "open");

            // open file
            m_file = std::fopen(file.c_str(), "rb");
            if (m_file == NULL)
            {
                Log::error("unable to open input file: %s", file.c_str());
                return false;
            }

            // open bzip stream
            m_bz = bzip2::BZ2_bzReadOpen(&m_status, m_file, 0, 0, NULL, 0);
            if (m_bz == NULL || !ok())
            {
                Log::error("unable to open bzip read stream");
                return false;
            }

            return true;
        }

        // read: read from bzip stream
        void  read(char* buf, int sz, int& actual) { actual = bzip2::BZ2_bzRead(&m_status, m_bz, buf, sz); }
        const char* read(int& actual)              { read(m_buf, SZ-1, actual); return m_buf; }
    };

    //
    // BZipWriter implementation
    //

    struct BZip2Writer : BZip2, IZipWriter
    {
        // ok: query status of bzip
        bool ok() { return BZip2::ok(); }

        // close: close bzip writer stream
        void close()
        {
            if (m_bz != NULL) bzip2::BZ2_bzWriteClose(&m_status, m_bz, 0, NULL, NULL);
            m_bz = NULL;
            BZip2::close();
        }

        // open: open bzip writer stream
        bool open(const String& file, int blocksz)
        {
            Log::Scope scope(KCC_FILE, "open");

            // open file
            m_file = std::fopen(file.c_str(), "wb");
            if (m_file == NULL)
            {
                Log::error("unable to open output file: %s", file.c_str());
                return false;
            }

            // open bzip stream
            m_bz = bzip2::BZ2_bzWriteOpen(&m_status, m_file, blocksz, 0, 0);
            if (m_bz == NULL || !ok())
            {
                Log::error("unable to open bzip write stream");
                return false;
            }

            return true;
        }

        // write: write to bzip stream
        void write(const char* buf, int sz) { bzip2::BZ2_bzWrite(&m_status, m_bz, (void*)buf, sz); }
    };

    //
    // BZip2 factory implementation
    //

    struct BZip2Factory : IZipFactory
    {
        IComponent* construct()       { return constructReader(); }
        IZipReader* constructReader() { return new BZip2Reader; }
        IZipWriter* constructWriter() { return new BZip2Writer; }
    };

    KCC_COMPONENT_FACTORY_CUST(BZip2Factory)
    
    KCC_COMPONENT_FACTORY_METADATA_BEGIN_CUSTOM(BZip2Factory, IZipFactory, BZip2Reader, IZipReader)
        KCC_COMPONENT_FACTORY_METADATA_PROPERTY(KCC_COMPONENT_FACTORY_SCM, KCC_VERSION)
    KCC_COMPONENT_FACTORY_METADATA_END
}
