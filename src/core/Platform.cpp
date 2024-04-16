/*
 * Kuumba C++ Core
 *
 * $Id: Platform.cpp 22625 2008-03-09 22:51:49Z tvk $
 */
#include <inc/core/Core.h>

#if defined(KCC_WINDOWS)
#   include "windows.h"
#   include "psapi.h"
#elif defined(KCC_LINUX)
#   include "unistd.h"
#   include "dlfcn.h"
#   include "dirent.h"
#   include "sys/stat.h"
#   include "sys/time.h"
#endif

#define KCC_FILE "Platform"

namespace kcc
{
    // Constants
    static const Char   k_chPathSep = '/';
    static const String k_pathSep    ("/");
    static const String k_pathSelf   (".");
    static const String k_pathParent ("..");
    static const String k_findAll    ("*.*");
    static const int    k_szPath    = 4096;

    // file path separator
    const String& Platform::fsSep() { return k_pathSep; }

    // fsRename: rename file or dir
    bool Platform::fsRename(const String& from, const String& to)
    {
        return std::rename(from.c_str(), to.c_str()) == 0;
    }

    // fsFullPath: build full path accounting for separator
    String Platform::fsFullPath(const String& p, const String& n)
    {
        String path(Platform::fsNormalize(p));
        if (path.length() > 1 && path[path.size() - 1] == k_chPathSep)
            return path + n;
        else if (path.length() > 1)
            return path + k_pathSep + n;
        else
            return n;
    }

    // fsDirCreateEmpty: create or clean directory
    bool Platform::fsDirCreateEmpty(const String& p)
    {
        if (p.empty()) return false;
        String path(Platform::fsNormalize(p));
        Platform::fsRemoveAll(path); // empty contents of deep path only
        
        // build dir fifo stack
        StringList mkdirs;
        StringVector split;
        Strings::tokenize(path, k_pathSep, split);
        for (StringVector::iterator i = split.begin(); i != split.end(); i++)
        {
            const String& name = *i;
            if (i == split.begin() && name.find(":") != String::npos) continue; // skip drive
            mkdirs.push_back(name);
        }
        if (mkdirs.empty()) return false;
        
        // adjust for absolute path
        if (path[0] == k_chPathSep)
        {
            String& root = mkdirs.front();
            mkdirs.front() = k_pathSep + root;
        }

        // create dirs in fifo order
        Platform::File f;
        String fullPath;
        bool ok = true;
        while (ok && !mkdirs.empty())
        {
            fullPath = Platform::fsFullPath(fullPath, mkdirs.front());
            if (!Platform::fsFile(fullPath, f)) ok = Platform::fsDirCreate(fullPath);
            mkdirs.pop_front();
        }
        return ok;
    }
    
    // fsDirAbsolute: get absolute path from relative
    bool Platform::fsDirAbsolute(const String& relative, String& absolute)
    {
        String cwd;
        if (!Platform::fsDirCurrent(cwd))     return false;
        if (!Platform::fsDirChange(relative)) return false;
        Platform::fsDirCurrent(absolute);
        Platform::fsDirChange(cwd);
        return true;
    }

    // fsRemoveAll: remove file, if dir remove all contents then dir
    void Platform::fsRemoveAll(const String& p)
    {
        Platform::File file;
        Platform::fsFile(p, file);
        if (file.dir) 
        {
            Platform::Files files;
            Platform::fsDir(p, files);
            for (Files::iterator f = files.begin(); f != files.end(); f++)
                Platform::fsRemoveAll(Platform::fsFullPath(p, f->name));
        }
        Platform::fsRemove(p);
    }

    // fsSplit: split full path into path and name components
    void Platform::fsSplit(const String& fp, String& p, String& f)
    {
        f.clear();
        p.clear();
        if (fp.empty()) return;
        
        // find last path sep and split
        String fullPath(Platform::fsNormalize(fp));
        String::size_type sep = fullPath.rfind(k_pathSep);
        if (sep == String::npos) f = fullPath;
        else
        {
            p.assign(fullPath.begin(), fullPath.begin() + sep + 1);
            f.assign(fullPath.begin() + sep + 1, fullPath.end());
        }
    }

    // fsNormalize: replace to normalized slashes
    String Platform::fsNormalize(const String& path)
    {
        String norm(path);
        for (String::size_type i = 0; i < norm.length(); i++) 
            if (norm[i] == '\\') norm[i] = k_chPathSep;
        return norm;
    }

    // fsFile: query if path exists
    bool Platform::fsExists(const String& fullPath)
    {
        Platform::File f;
        return Platform::fsFile(fullPath, f);
    }

#if defined(KCC_WINDOWS)

    //
    // Windows Implementation
    //

    // k_ft2tt: convert OS time to C time
    static inline std::time_t k_ft2tt(const FILETIME& ft)
    {
        std::tm    tm;
        FILETIME   lt;
        SYSTEMTIME st;
        ::FileTimeToLocalFileTime(&ft, &lt);
        ::FileTimeToSystemTime(&lt, &st);
        tm.tm_sec   = st.wSecond;
        tm.tm_min   = st.wMinute;
        tm.tm_hour  = st.wHour;
        tm.tm_mday  = st.wDay;
        tm.tm_mon   = st.wMonth - 1;   // tm_mon is 0 based
        tm.tm_year  = st.wYear - 1900; // tm_year is 1900 based
        tm.tm_isdst = -1;
        return std::mktime(&tm);
    }

    // k_lastError: get last error
    static inline String k_lastError()
    {
        LPVOID buf;
        ::FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            GetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
            (LPTSTR) &buf,
            0,
            NULL);
        String err((LPTSTR)buf);
        ::LocalFree(buf);
        return Strings::trimws(err);
    }

    // k_populate: populate a file object from a WIN32_FIND_DATA
    static inline void k_populate(Platform::File& f, const WIN32_FIND_DATA& find)
    {
        f.name     = find.cFileName;
        f.created  = k_ft2tt(find.ftCreationTime);
        f.modified = k_ft2tt(find.ftLastWriteTime);
        f.accessed = k_ft2tt(find.ftLastAccessTime);
        f.size     = find.nFileSizeLow;
        f.dir      = (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    }

    // moduleBind: bind to module
    Platform::Module Platform::moduleBind(const String& n, const String& p, bool logFailure)
    {
        Log::Scope scope(KCC_FILE, "moduleBind");
        Module m = (Module)::LoadLibrary(Platform::fsFullPath(p, n).c_str());
        if (m == NULL && logFailure) Log::warning(k_lastError().c_str());
        return m;
    }

    // moduleFunction: get address of function from module
    void* Platform::moduleFunction(const Module& m, const String& n, bool logFailure)
    {
        void* s = ::GetProcAddress((HMODULE)m, n.c_str());
        if (s == NULL && logFailure) Log::warning(k_lastError().c_str());
        return s;
    }

    // fsDir: popolate list with files
    void Platform::fsDir(const String& p, Files& list)
    {
        list.clear();
        list.reserve(64);
        if (p.empty()) return;
        
        // find files
        String search(Platform::fsFullPath(Platform::fsNormalize(p), k_findAll));
        WIN32_FIND_DATA find;
        HANDLE h = ::FindFirstFile(search.c_str(), &find);
        bool cont = h != INVALID_HANDLE_VALUE;
        while (cont)
        {
            if (find.cFileName[0] != '.')
            {
                File f;
                k_populate(f, find);
                list.push_back(f);
            }
            cont = ::FindNextFile(h, &find) == TRUE;
            if (!cont) FindClose(h);
        }
    }

    // fsFile: popolate file object from path
    bool Platform::fsFile(const String& p, File& f)
    {
        if (p.empty()) return false;
    
        // convert to raw name if directory
        String path(Platform::fsNormalize(p));
        if (path.size() > 1 && path[path.size() - 1] == k_chPathSep) path.erase(path.size() - 1);

        // populate
        WIN32_FIND_DATA find;
        HANDLE h = ::FindFirstFile(path.c_str(), &find);
        if (h == INVALID_HANDLE_VALUE) return false;
        k_populate(f, find);
        ::FindClose(h);
        return true;
    }

    // fsRemove: remove file or dir
    bool Platform::fsRemove(const String& file)
    {
        bool ok = true;
        const Char* path = file.c_str();
        if (::GetFileAttributes(path) & FILE_ATTRIBUTE_DIRECTORY)
            ok = ::RemoveDirectory(path) == TRUE;
        else
            ok = ::DeleteFile(path) == TRUE;
        return ok;
    }

    // delegate to native impl.
    String Platform::moduleSystemName(const String& n) { return n + ".dll"; }
    void   Platform::moduleUnbind(const Module& m)     { ::FreeLibrary((HMODULE)m); }
    bool   Platform::fsDirCreate(const String& p)      { return ::CreateDirectory(p.c_str(), NULL) == TRUE; }
    bool   Platform::fsDirChange(const String& p)      { return ::SetCurrentDirectory(p.c_str()) == TRUE; }
    bool   Platform::fsDirCurrent(String& p)
    {
        char cwd[k_szPath];
        if (!::GetCurrentDirectory(k_szPath, cwd)) return false;
        p = Platform::fsNormalize(cwd);
        String::size_type drv = p.find(":");
        if (drv != String::npos) p.erase(0, drv+1);
        return true;
    }
    
    // procMemInUse: mem used in kb
    unsigned long Platform::procMemInUseKB()
    {
        unsigned long ram = 0L;
        PROCESS_MEMORY_COUNTERS pmc = {0};
        HANDLE p = ::OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, ::GetCurrentProcessId());
        if (p != NULL && ::GetProcessMemoryInfo(p, &pmc, sizeof(pmc)))
        {
            ram = (long) pmc.WorkingSetSize;
            ::CloseHandle(p);
        }
        return ram / 1024L; // KB's
    }
    
    // procTimeInUse: proc time in use in secs
    double Platform::procTimeInUseSecs()
    {
        return ((double)std::clock() / CLOCKS_PER_SEC);
    }

#elif defined(KCC_LINUX)

    //
    // Linux Implementation
    //

    // k_populate: populate a file object from a stat struct
    static inline void k_populate(Platform::File& f, const String& name, const struct stat& find)
    {
        f.name     = name;
        f.created  = std::min(find.st_ctime, find.st_mtime);
        f.modified = std::max(find.st_ctime, find.st_mtime);
        f.accessed = find.st_atime;
        f.size     = S_ISDIR(find.st_mode) ? 0L : (long) find.st_size;
        f.dir      = S_ISDIR(find.st_mode);
    }

    // moduleBind: bind to module
    Platform::Module Platform::moduleBind(const String& n, const String& p, bool logFailure)
    {
        Log::Scope scope(KCC_FILE, "moduleBind");
        Module m = ::dlopen(Platform::fsFullPath(p, n).c_str(), RTLD_NOW|RTLD_GLOBAL);
        if (m == NULL && logFailure) Log::warning(::dlerror());
        return m;
    }

    // moduleFunction: get address of function from module
    void* Platform::moduleFunction(const Module& m, const String& n, bool logFailure)
    {
        Log::Scope scope(KCC_FILE, "moduleFunction");
        void* s = ::dlsym(m, n.c_str());
        if (s == NULL && logFailure) Log::warning(::dlerror());
        return s;
    }

    // fsDir: popolate list with files
    void Platform::fsDir(const String& root, Files& list)
    {
        list.clear();
        list.reserve(64);
        String search(root);
        if (search.rfind(k_pathSep) != search.length()) search += k_pathSep;
        DIR* d = ::opendir(search.c_str());
        if (d == NULL) return;
        char buf[sizeof(struct dirent) + PATH_MAX];
        struct dirent *dbuf = (struct dirent *)buf;
        struct dirent *e = NULL;
        ::readdir_r(d, dbuf, &e);
        while (e != NULL)
        {
            if (e->d_name[0] != '.')
            {
                Platform::File f;
                struct stat find = {0};
                if (::stat((search + e->d_name).c_str(), &find) == 0)
                {
                    k_populate(f, e->d_name, find);
                    list.push_back(f);
                }
            }
            ::readdir_r(d, dbuf, &e);
        }
        ::closedir(d);
    }

    // fsFile: popolate file object from path
    bool Platform::fsFile(const String& p, File& f)
    {
        if (p.empty()) return false;
    
        // convert to raw name if directory
        String path(p);
        if (path.size() > 1 && path[path.size() - 1] == k_chPathSep) path.erase(path.size() - 1);
        
        // file name
        String fn;        
        if (path[path.size() - 1] == '.')
        {
            // name from relative path
            
            // NOTE:
            //   * to get the name we have to go the parent and find the ino id 
            //   * TODO: is there a better way?
            
            // ino id of path
            DIR* d = ::opendir(path.c_str());
            if (d == NULL) return false;
            struct dirent *e = ::readdir(d);
            ino_t id = e->d_ino;
            ::closedir(d);

            // find ino id in parent list for path
            String parent(path + "/..");
            d = ::opendir(parent.c_str());
            if (d == NULL) return false;
            char buf[sizeof(struct dirent) + PATH_MAX];
            struct dirent *dbuf = (struct dirent *)buf;
            ::readdir_r(d, dbuf, &e);
            while (e != NULL)
            {
                if (e->d_ino == id)
                {
                    fn = e->d_name;
                    break;
                }
                ::readdir_r(d, dbuf, &e);
            }
            ::closedir(d);
        }
        else
        {
            String fp;
            Platform::fsSplit(path, fp, fn);
        }
        if (fn.empty()) return false;

        // populate
        struct stat find = {0};
        if (::stat(path.c_str(), &find) < 0) return false;
        k_populate(f, fn, find);
        return true;
    }

    // delegate to to native impl.
    String Platform::moduleSystemName(const String& n) { return "lib" + n + ".so"; }
    void   Platform::moduleUnbind(const Module& m)     { ::dlclose(m); }
    bool   Platform::fsRemove(const String& file)      { return ::remove(file.c_str()) == 0; }
    bool   Platform::fsDirCreate(const String& p)      { return ::mkdir(p.c_str(), S_IRWXU|S_IRWXG) == 0; }
    bool   Platform::fsDirChange(const String& p)      { return ::chdir(p.c_str()) == 0; }
    bool   Platform::fsDirCurrent(String& p)
    {
        char cwd[k_szPath];
        if (::getcwd(cwd, k_szPath) == NULL) return false;
        p = cwd;
        return true;
    }
    
    // procMemInUse: mem used in kb
    unsigned long Platform::procMemInUseKB()
    {
        static const String k_split(" ");
        static const int    k_fieldUsed = 1;
        unsigned long ram = 0L;
        String pidram(Strings::printf("/proc/%d/statm", ::getpid()));
        std::ifstream in(pidram.c_str());
        if (in.good())
        {
            // get ram info
            String raminfo;
            std::getline(in, raminfo);
            
            // tokenize into fields
            StringVector ramfields;
            Strings::tokenize(raminfo, k_split, ramfields);
            if (ramfields.size() > 2) ram = Strings::parseInteger(ramfields[k_fieldUsed]);
        }
        return (ram * ::getpagesize()) / 1024L; // KB's
    }
    
    // procTimeInUse: proc time in use in secs
    double Platform::procTimeInUseSecs()
    {
        struct timeval t = {0};
        ::gettimeofday(&t, NULL);
        return t.tv_sec + ((double)t.tv_usec / 1000000);
    }

#endif
}
