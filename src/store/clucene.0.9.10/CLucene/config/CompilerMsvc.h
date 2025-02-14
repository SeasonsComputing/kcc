#if !defined(_lucene_COMPILER_MSVC)
#define _lucene_COMPILER_MSVC

// It is internal CLucene header - DO NOT include it directly

#include "CLucene/config/define_std.h"

#if (_MSC_VER >= 1300)
//>= 7.0
	# pragma warning(disable: 4512) // This would be very annoying
	#define CL_NS_HASHING(func) stdext::func

#elif (_MSC_VER >= 1200)
//6.0
#ifdef LUCENE_ENABLE_MEMLEAKTRACKING
	#define _CLDELETE_CARRAY(x) if (x!=NULL){CL_NS(debug)::LuceneBase::__cl_voidpremove((void*)x,__FILE__,__LINE__);delete[] __CONST_CAST(TCHAR*,x); x=NULL;}
	#define _CLDELETE_CaARRAY(x) if (x!=NULL){CL_NS(debug)::LuceneBase::__cl_voidpremove((void*)x,__FILE__,__LINE__);delete[] __CONST_CAST(char*,x); x=NULL;}
	#define _CLDELETE_LCARRAY(x) if (x!=NULL){CL_NS(debug)::LuceneBase::__cl_voidpremove((void*)x,__FILE__,__LINE__);delete[] __CONST_CAST(TCHAR*,x);}
	#define _CLDELETE_LCaARRAY(x) if (x!=NULL){CL_NS(debug)::LuceneBase::__cl_voidpremove((void*)x,__FILE__,__LINE__);delete[] __CONST_CAST(char*,x);}
#else
	#define _CLDELETE_CARRAY(x) if (x!=NULL){delete[] __CONST_CAST(TCHAR*,x); x=NULL;}
	#define _CLDELETE_CaARRAY(x) if (x!=NULL){delete[] __CONST_CAST(char*,x); x=NULL;}
	#define _CLDELETE_LCARRAY(x) if (x!=NULL){delete[] __CONST_CAST(TCHAR*,x);}
	#define _CLDELETE_LCaARRAY(x) if (x!=NULL){delete[] __CONST_CAST(char*,x);}
	
#endif
	#define LUCENE_STATIC_CONSTANT(type, assignment) enum { assignment }

	# pragma warning(disable: 4786) // This would be very annoying
	namespace std{
		# undef min // just in case
		# undef max // just in case
		
		#define min(a,b) (a>b?b:a)
		#define max(a,b) (a>b?a:b)
	}
	
	//only 7.0+ has these function	
	#undef _CL_HAVE_LLTOA
	#undef _CL_HAVE_LLTOAW
	#undef _CL_HAVE_INTPTR_T
	#undef _CL_HAVE_WCSTOLL
	#undef _CL_HAVE_STRTOLL
	#undef _CL_HAVE_HASH_MAP
	#undef _CL_HAVE_HASH_SET

#else
# error "This version of MSVC has not been tested. Please uncomment this line to try anyway. Please send a report to the Clucene's administration if successful"
#endif


#define _BASE_THROW_TYPE exception
#define debugFatalExit(ret) exit(ret)
#if _MSC_VER >= 1020
 #define _LUCENE_PRAGMA_ONCE
#endif
#define _LUCENE_PRAGMA_WARNINGS //tell lucene to display warnings using pragmas instead of #warning



//msvc supports large files
#ifdef _LARGE_FILES
# define fileSize _filelengthi64
# define fileSeek _lseeki64
# define fileTell _telli64
# define fileStat _stati64
# define fileHandleStat _fstati64
#else
# define fileSize _filelength
# define fileSeek _lseek
# define fileTell _tell
# define fileStat _stat
# define fileHandleStat _fstat
#endif

//_rename is not defined???
#define _rename rename

//java long type
typedef __int64 int64_t; 
typedef unsigned __int64 uint64_t;
//#define LUCENE_INT64_MAX _I64_MAX

//java int type
typedef int int32_t;
typedef unsigned int uint32_t;
//#define LUCENE_INT32_MAX INT_MAX

//java byte type
typedef unsigned char uint8_t;
//#define LUCENE_UINT8_MAX UCHAR_MAX

//floating point type
typedef double float_t;

#define _CL_ILONG(x) x ## L
#define _ILONGLONG(x) x ## i64


#elif !defined(_lucene_COMPILER_MSVC2)
#define _lucene_COMPILER_MSVC2
    //second inclusion


#elif !defined(_lucene_COMPILER_MSVC2)
#define _lucene_COMPILER_MSVC2
  //third inclusion

#endif
