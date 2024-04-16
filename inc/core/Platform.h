/*
 * Kuumba C++ Core
 *
 * $Id: Platform.h 22156 2008-02-20 14:34:22Z tvk $
 */
#ifndef Platform_h
#define Platform_h

namespace kcc
{
    /**
     * Utility class for native platform operations
     *
     * @author Ted V. Kremer
     */
    class KCC_CORE_EXPORT Platform
    {
    public:
        /*
         * Shared Library Modules
         */

        /** Module handle */
        typedef void* Module;

        /**
         * Bind to a shared library module
         * @param name library name (without extension or prefix as these are per platform)
         * @param path path to library (default will use system search)
         * @param logFailure logs platform message for failure reason
         * @return handle to module (NULL if not found)
         */
        static Module moduleBind(const String& name, const String& path = Strings::empty(), bool logFailure = true);

        /**
         * Accessor to module system name (e.g. {name}.dll or lib{name}.so)
         * @param name location of library
         * @return string module name
         */
        static String moduleSystemName(const String& name);

        /**
         * Unbdind from shared libary module
         * @param module to unbind
         */
        static void moduleUnbind(const Module& module);

        /**
         * Retrieve function from module
         * @param module module to retrieve library from
         * @param name namr of function
         * @param logFailure logs platform message for failure reason
         * @return pointer to function (NULL if not found)
         */
        static void* moduleFunction(const Module& module, const String& name, bool logFailure = true);

        /*
         * File System
         * Path Separator is always a forward slash '/'
         */

        /** File object */
        struct File
        {
            String        name;
            std::time_t   created;
            std::time_t   modified;
            std::time_t   accessed;
            unsigned long size;
            bool          dir;
            File() : name(), created(-1), modified(-1),
                accessed(-1), size(0L), dir(false) {}
            File(const File& f) : name(f.name), created(f.created), modified(f.modified),
                accessed(f.accessed), size(f.size), dir(f.dir) {}
        };

        /** Collection of File objects */
        typedef std::vector<File> Files;

        /**
         * Retrieve list of files and directories (excluding system and hidden files)
         * Does not include current or parent directory names (".", "..")
         * @param root folder retrieve list from
         * @param list where to place the output
         */
        static void fsDir(const String& root, Files& list);

        /**
         * Retrieve file object for a path
         * @param path path of folder
         * @param file where to put file data
         * @return true if file data populated
         */
        static bool fsFile(const String& path, File& file);

        /**
         * Create directory for path
         * @param path path of folder
         * @return true if created, false if already exists or error
         */
        static bool fsDirCreate(const String& path);

        /**
         * Create all directories for path cleaning existing contents
         * @param path path of folder
         * @return true if created, false if error
         */
        static bool fsDirCreateEmpty(const String& path);

        /**
         * Get absolute directory from relative path
         * @param relative path of relative folder
         * @param absolute path of absolute folder
         * @return true if ok, false if error
         */
        static bool fsDirAbsolute(const String& relative, String& absolute);

        /**
         * Change directory to path
         * @param path path of folder
         * @return true if changed, false if error
         */
        static bool fsDirChange(const String& path);

        /**
         * Get current directory
         * @param path path of current folder
         * @return true if ok, false if error
         */
        static bool fsDirCurrent(String& path);

        /**
         * Rename from path to path
         * @param from path for from file
         * @param to path for to file
         * @return true if renamed, false if error
         */
        static bool fsRename(const String& from, const String& to);

        /**
         * Remove file path
         * @param path path of file
         * @return true if removed, false if error
         */
        static bool fsRemove(const String& file);

        /**
         * Remove file path; if dir will remove all children dirs and contained files
         * @param path path of file or dir
         */
        static void fsRemoveAll(const String& path);

        /**
         * Normalize path. Will replace '\' with '/'
         * @param path to normalize
         * @param normalized path
         */
        static String fsNormalize(const String& path);
        
        /**
         * Path separator
         * @return path separator
         */
        static const String& fsSep();

        /**
         * Build full path accounting for separator (accounts for absence or presence of
         * path ending with separator prior to concatenation)
         * @param path directory path
         * @param name file name
         */
        static String fsFullPath(const String& path, const String& name);

        /**
         * Split full path into path and name components
         * @param fullPath full path to split
         * @param path out param of path component
         * @param file out param of file component
         */
        static void fsSplit(const String& fullPath, String& path, String& file);

        /**
         * Query if full path exists
         * @param fullPath full path to test for existance
         * @return true if exists
         */
        static bool fsExists(const String& fullPath);

        /*
         * Process
         */
         
        /**
         * Query process memory in use
         * @return memory in use in kb
         */ 
        static unsigned long procMemInUseKB();
         
        /**
         * Query process time in use
         * @return time in use in secs
         */ 
        static double procTimeInUseSecs();

    private:
        Platform();
    };
}

#endif // Platform_h
