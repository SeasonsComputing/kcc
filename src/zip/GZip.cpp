/*
 * Kuumba C++ Core
 *
 * $Id: GZip.cpp 22626 2008-03-09 23:29:37Z tvk $
 */
#include <inc/core/Core.h>
#include <inc/zip/IZip.h>

namespace gzip
{
    #if defined(KCC_WINDOWS)
    #    include "winzlib/zlib.h"
    #elif defined(KCC_LINUX)
    #   include "zlib.h"
    #endif
};
#include "errno.h"

#define KCC_FILE    "GZip"
#define KCC_VERSION "$Id: GZip.cpp 22626 2008-03-09 23:29:37Z tvk $"

namespace kcc
{
    // buffer size
    const int SZ = 4096;

    //
    // GZip implementation
    // zlib (see zlib.h) implementation using gz* methods.
    // gz* methods use the stdio (required for thread safety)
    //

    struct GZip
    {
        // Attributes
        int          m_status;
        gzip::gzFile m_gz;

        // ctor/dtor
        GZip() : m_status(Z_OK), m_gz(NULL) {}
        virtual ~GZip() { close(); }

        // ok: query status of gzip
        bool ok() { return m_status == Z_OK; }

        // close: close file stream
        void close()
        {
            if (m_gz != NULL) gzip::gzclose(m_gz);
            m_gz     = NULL;
            m_status = Z_STREAM_END;
        }

        // open: open gzip writer stream
        bool open(const String& file, const char *mode)
        {
            Log::Scope scope(KCC_FILE, "open");

            // open gzip stream
            m_gz = gzip::gzopen(file.c_str(), mode);
            if (m_gz == NULL)
            {
                // type of failure
                if (errno == 0) 
                {
                    Log::error("unable to open gzip write stream, because of zlib Z_MEM_ERROR");
                    m_status = Z_MEM_ERROR;
                }
                else 
                {
                    Log::error("unable to open gzip write stream");
                    m_status = Z_ERRNO;
                }
                return false;
            }
            
            return true;
        }
    };

    //
    // GZipReader implementation
    //

    struct GZipReader : GZip, IZipReader
    {
        // Attributes
        char* m_buf;

        // ctor/dtor
        GZipReader() : m_buf(new char[SZ]) {}

        // ok: query status of gzip
        bool ok() { return GZip::ok(); }

        // close: close gzip reader stream
        void close()
        {
            if (m_buf != NULL) delete [] m_buf;
            m_buf = NULL;
            GZip::close();
        }

        // open: open gzip reader stream
        bool open(const String& file) { return GZip::open(file, "rb"); }

        // read: read from gzip stream
        void read(char* buf, int sz, int& actual) 
        { 
            m_status = Z_OK;
            actual = gzip::gzread(m_gz, buf, sz); 
            if (actual == 0)  
            {
                m_status = Z_STREAM_END;
            }
            else if (actual <= -1) 
            {
                // Failed read report in status and 
                // 0 out actual number read for consumer
                m_status = actual;
                actual = 0;
            }
        }

        // read: read using internal buffer m_buf GZipReader
        const char* read(int& actual) { read(m_buf, SZ-1, actual); return m_buf; }
    };

    //
    // GZipWriter implementation
    //

    struct GZipWriter : GZip, IZipWriter
    {
        // ok: query status of gzip
        bool ok() { return GZip::ok(); }

        // close: close gzip writer stream
        void close() { GZip::close(); }

        // open: open gzip writer stream.  
        bool open(const String& file, int blocksz)
        {
            if (blocksz > 9)      blocksz = 9;
            else if (blocksz < 1) blocksz = 1;
            return GZip::open(file, Strings::printf("wb%i", blocksz).c_str());
        }

        // write: write to gzip stream
        void write(const char* buf, int sz) 
        { 
            if (gzip::gzwrite(m_gz, (void*)buf, sz) == 0) m_status = Z_ERRNO; 
        }
    };

    //
    // GZip factory
    //

    struct GZipFactory : IZipFactory
    {
        IComponent* construct()       { return constructReader(); }
        IZipReader* constructReader() { return new GZipReader; }
        IZipWriter* constructWriter() { return new GZipWriter; }
    };

    KCC_COMPONENT_FACTORY_CUST(GZipFactory)
    
    KCC_COMPONENT_FACTORY_METADATA_BEGIN_CUSTOM(GZipFactory, IZipFactory, GZipReader, IZipReader)
        KCC_COMPONENT_FACTORY_METADATA_PROPERTY(KCC_COMPONENT_FACTORY_SCM, KCC_VERSION)
    KCC_COMPONENT_FACTORY_METADATA_END
}
