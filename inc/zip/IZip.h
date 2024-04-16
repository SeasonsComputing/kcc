/*
 * Kuumba C++ Core
 *
 * $Id: IZip.h 15199 2007-03-09 17:57:17Z tvk $
 */
#ifndef IZip_h
#define IZip_h

namespace kcc
{
    /**
     * Zip Reader
     *
     * @author Ted V. Kremer
     */
    interface IZipReader : IComponent
    {
        /**
         * Query status of zip
         * @return true if zip is valid (not end of stream and no error)
         */
        virtual bool ok() = 0;

        /**
         * Close zip
         */
        virtual void close() = 0;

        /**
         * Open zip stream to read from
         * @param file file to read
         * @return true if successfully opened
         */
        virtual bool open(const String& file) = 0;

        /**
         * Read from zip stream
         * @param buf buffer to read into
         * @param sz size of buffer
         * @param actual actual bytes read
         */
        virtual void read(char* buf, int sz, int& actual) = 0;

        /**
         * Convenience reader that uses an internal buffer
         * - returned buf is guaranteed to be at least as big as sz+1
         * - this makes decompressing text easier buf[sz] = 0 to null term the string
         * @param sz out param of bytes read
         * @return pointer to data read (ownership NOT consumed, valid until next read or component destroyed)
         */
        virtual const char* read(int& sz) = 0;
    };

    /**
     * Zip Writer
     *
     * @author Ted V. Kremer
     */
    interface IZipWriter : IComponent
    {
        /**
         * Query status of zip
         * @return true if zip is valid (not end of stream and no error)
         */
        virtual bool ok() = 0;

        /**
         * Close zip
         */
        virtual void close() = 0;

        /**
         * Open bzip stream to write to
         * @param file file to write
         * @param blocksz (1=fast, 9=best)
         * @return true if successfully opened
         */
        virtual bool open(const String& file, int blocksz = 9) = 0;

        /**
         * Write to bzip file
         * @param buf buffer to write to
         * @param sz size of buffer to write
         */
        virtual void write(const char* buf, int sz) = 0;
    };
    
    /**
     * Zip factory interface
     *
     * @author Ted V. Kremer
     */
    interface IZipFactory : IComponentFactory
    {
        /**
         * Construct zip reader (ownership IS consumed)
         * @return zip reader instance
         */
        virtual IZipReader* constructReader() = 0;

        /**
         * Construct zip writer (ownership IS consumed)
         * @return zip writer instance
         */
        virtual IZipWriter* constructWriter() = 0;
    };
}

#endif // IZip_h
