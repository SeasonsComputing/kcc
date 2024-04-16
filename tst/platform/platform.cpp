#include <inc/core/Core.h>

#define KCC_FILE    "platform"
#define KCC_VERSION "$Id: platform.cpp 20677 2007-09-12 15:21:44Z tvk $"

#define FS_OK(_T,_M) if (!(_M)) { throw kcc::Exception(_T); }

void fileOut(const kcc::Platform::File& f)
{
    std::cout 
        << kcc::Strings::printf("name: %-25s", f.name.c_str())
        << " created: "  << kcc::ISODate::local(f.created)
        << " modified: " << kcc::ISODate::local(f.modified)
        << " accessed: " << kcc::ISODate::local(f.accessed)
        << " size: " << f.size
        << " dir: " << f.dir
        << std::endl;
}

void fileList(const kcc::String& p)
{
    std::cout << " ******* LISTING: " << p << "\n";
    kcc::Platform::Files files;
    kcc::Platform::fsDir(p, files);
    for (
        kcc::Platform::Files::iterator i = files.begin();
        i != files.end();
        i++)
    {
        fileOut(*i);
    }
}

int main(int argc, const char* argv[])
{
    kcc::Properties props;
    props.set("kcc.logVerbosity", (long) kcc::Log::V_INFO_3);
    props.set("kcc.logMax",       1L);
    props.set("kcc.LogName",      KCC_FILE);
    if (argc > 1) props.load(argc, argv, false);
    kcc::Core::init(props, KCC_VERSION);

    kcc::Log::Scope scope(KCC_FILE, "main");
    kcc::Log::out("begin platform testing");
    try
    {
        kcc::Platform::fsDirCreateEmpty("tst-out"); // ok if doesn't exist; don't check error

        std::cout << "testing dir create, rename, remove" << std::endl;
        kcc::Platform::File f;
        FS_OK("  1. fsFile", kcc::Platform::fsFile("tst-out", f));
        fileOut(f);
        
        FS_OK("  2. fsDirCreate", kcc::Platform::fsDirCreate("tst-out/test_create_dir"));
        FS_OK("  3. fsRename",    kcc::Platform::fsRename("tst-out/test_create_dir", "tst-out/test_rename_dir"));
        
        FS_OK("  4. fsDirCreate", kcc::Platform::fsDirCreate("tst-out/test_delete_dir"));
        FS_OK("  5. fsRemove",    kcc::Platform::fsRemove("tst-out/test_delete_dir"));

        std::cout << "testing file list" << std::endl;
        fileList(".");

        std::cout << "testing create all empty" << std::endl;
        FS_OK("  6. fsDirCreateEmpty", kcc::Platform::fsDirCreateEmpty("tst-out/createempty/foo/bar/zap"));

        std::cout << "testing remove all" << std::endl;
        kcc::Platform::fsRemoveAll("tst-out/createempty/foo/bar");
        FS_OK("  7. fsRemoveAll", !kcc::Platform::fsExists("tst-out/createempty/foo/bar/zap"));

        std::cout << "testing normalize & split" << std::endl;
        kcc::String fp, fn;
        kcc::String path(kcc::Platform::fsNormalize("c:\\foo\\bar\\blat.h"));
        kcc::Platform::fsSplit(path, fp, fn);
        std::cout << "testing normalize: " << path << std::endl;
        std::cout << "split: " << fp << "\t" << fn << std::endl;
        
        std::cout << "testing dir relative and absolute" << std::endl;
        kcc::Platform::File where;
        FS_OK("  8. fsFile",      kcc::Platform::fsFile(".", where));
        fileOut(where);        
        FS_OK("  9. fsDirChange", kcc::Platform::fsDirChange("tst-out"));
        FS_OK(" 10. fsFile",      kcc::Platform::fsFile(".", where));
        fileOut(where);
        FS_OK(" 11. fsDirChange", kcc::Platform::fsDirChange(".."));        
        FS_OK(" 12. fsFile",      kcc::Platform::fsFile(".", where));
        fileOut(where);
        fileList("/");
        FS_OK(" 13. fsDirAbsolute",  kcc::Platform::fsDirAbsolute(".", fp));
        std::cout << "full path: " << fp << "\n";
        fp = fp + kcc::Platform::fsSep() + "tst-out/absolute";
        FS_OK(" 14. fsDirCreate", kcc::Platform::fsDirCreateEmpty(fp));
        FS_OK(" 15. fsFile",      kcc::Platform::fsFile(fp, where));
        fileOut(where);

        kcc::Log::out("completed platform testing");
    }
    catch (std::exception& e)
    {
        kcc::Log::exception(e);
        return 1;
    }

    return 0;
}
