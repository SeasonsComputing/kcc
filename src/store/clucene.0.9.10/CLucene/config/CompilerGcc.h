#if !defined(_lucene_COMPILER_GCC)
#define _lucene_COMPILER_GCC

// It is internal CLucene header - DO NOT include it directly
#if !defined(_SUPPRESS_MAKE_BASED_CONFIG)
   #include "CLucene/clucene-config.h" //make clucene-config.h file
#endif

//re-define some of the _CL prefixes...
#if defined(_CL__ASCII) && !defined(_ASCII)
 #define _ASCII _CL__ASCII
#endif
#if defined(_CL__UCS2) && !defined(_UCS2)
 #define _UCS2 _CL__UCS2
#endif
#if defined(_CL__DEBUG) && !defined(_DEBUG)
 #define _DEBUG _CL__DEBUG
#endif

#if defined(_CL__FILE_OFFSET_BITS) && !defined(_FILE_OFFSET_BITS)
 #define _FILE_OFFSET_BITS _CL__FILE_OFFSET_BITS
#endif
#if defined(_CL__LARGE_FILES) && !defined(_LARGE_FILES)
 #define _LARGE_FILES _CL__LARGE_FILES
#endif

#if defined(__cl__uint32_t) && !defined(_uint32_t)
 #define _uint32_t
#endif
#if defined(__cl__uint64_t) && !defined(_uint64_t)
 #define _uint64_t __cl__uint64_t
#endif
#if defined(__cl__uint8_t) && !defined(_uint8_t)
 #define _uint8_t __cl__uint8_t
#endif
//end redefine defaults


#if defined(_ASCII)
 #undef _UCS2
#elif defined(_UCS2)
//
#else
 #define CL_CHARSET_GUESS
#endif

#ifdef _CL_HAVE_NO_FUNCTION_TRY_BLOCKS
    #undef  _LUCENE_DISABLE_EXCEPTIONS
    #define _LUCENE_DISABLE_EXCEPTIONS
    
    todo: could define JXTA style exception handling here
#else
    #define _BASE_THROW_TYPE CL_NS_STD(runtime_error)
#endif

#if defined(HAVE_NO_FLOAT_BYTE) && !defined(LUCENE_FALLBACK_BYTE_FLOAT_ENCODING)
    #define LUCENE_FALLBACK_BYTE_FLOAT_ENCODING
#endif

#ifndef _CL_HAVE_NAMESPACES
	#define DISABLE_NAMESPACE
#endif

#define CL_NS_HASHING(func) __gnu_cxx::func //todo: is this best way?
#define LUCENE_DISABLE_HASHING

//cnd-debug exit command
#define debugFatalExit(ret) exit(ret)

#define fileSeek lseek
#define fileSize _filelength
#define fileStat stat
#define fileHandleStat fstat
#ifdef _CL_HAVE_TELL
    #define fileTell tell
#else
    //ftell (and probably soon ftell64) are POSIX standard functions, but tell and
    //tell64 are not, so we define fileTell in terms of fileSeek.
    #define fileTell(fhandle) fileSeek(fhandle, 0, SEEK_CUR)
#endif

#ifndef _CL_HAVE_WCHAR_T
    typedef unsigned short wchar_t;
#endif

#ifdef __CYGWIN__
   //cygwin seems to incorrectly define that it has wprintf???
   #undef _CL_HAVE_WPRINTF
#endif


///////////////////////////////////////////////////////////////////////////////
//end _lucene_COMPILER_GCC1
#elif !defined(_lucene_COMPILER_GCC2)
#define _lucene_COMPILER_GCC2
    //second inclusion

    //types
    #if defined(_CL_HAVE_INTTYPES_H)
        #include <inttypes.h>
    #endif
    #if defined(_CL_HAVE_SYS_TYPES_H)
        #include <sys/types.h>
    #endif
    #include <limits.h>
    
  //second chance to fix default settings
	//this must be defined later, otherwise it messes up
	//the standard libraries
	#if !defined(__MINGW32__)
	    #define _close ::close
	    #define _read ::read
	#endif


  //now that int64_t is defined, we can define this...
	#ifndef _CL_HAVE_FILELENGTH
	    #undef fileSize
	    #define fileSize lucene_filelength
	    int64_t lucene_filelength(int handle);
	#endif
    
#elif !defined(_lucene_COMPILER_GCC3)
#define _lucene_COMPILER_GCC3
    //third inclusion
    
	#if !defined(__MINGW32__)
	    //define replacements
	    #define O_RANDOM 0
	    #undef O_BINARY
	    #define O_BINARY 0
	    #define _S_IREAD  0444
	    #define _S_IWRITE 0333  // write and execute permissions
	    
	    //some functions that are needed - not charset dependent and not tchar type functions
	    #define _open open
	    #define _sleep(x) usleep(x*1000) //_sleep should be in millis, usleep is in micros
	    #define _write write
	    #define _snprintf snprintf
	    
	    //clucene uses ascii for filename interactions
	    #define _fullpath(abs,rel,len) realpath(rel,abs)
	    #define _mkdir(x) mkdir(x,0777)
	    #define _unlink unlink
	    #define _topen open
	    #define _filelength filelength
	#endif
	//also required by mingw
	#define _rename rename
   
#endif
