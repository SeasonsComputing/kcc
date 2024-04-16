#if !defined(lucene_compiler_h)
#define lucene_compiler_h

#if defined(_MBCS) || defined(_ASCII)
 #undef _ASCII
 #undef _UCS2
 #define _ASCII
#elif defined(_UNICODE)
 #define _UCS2
#elif !defined(_UCS2)
 #define _UCS2
#endif

//msvc needs unicode define so that it uses unicode library
#ifdef _UCS2
	#undef _UNICODE
	#define _UNICODE
	#undef _ASCII
#else
	#undef _UNICODE
	#undef _UCS2
#endif


////////////////////////////////////////////////////////////////////
//  Figure out what compiler we are using
////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER) && !defined(__MWERKS__) && !defined (__ICL) && !defined (__COMO__)
    #define _CLCOMPILER_MSVC _MSC_VER
#endif

#ifdef __GNUC__
    #include "CLucene/config/CompilerGcc.h"

#elif defined(_CLCOMPILER_MSVC)
    /* Microsoft Visual C++ */
    #include "CLucene/config/CompilerMsvc.h"

#else
    //Unable to identify the compiler, issue error diagnostic. 
    //Edit <CLucene/config/LuceneMycomp.h> to set STLport up for your compiler.
    //Uncomment this next line
    #error "Unable to identify the compiler, issue error diagnostic. Edit <CLucene/config/CompilerMycomp.h> to set Lucene up for your compiler."
    #include "CLucene/config/LuceneMycomp.h"
#endif /* end of compiler choice */
////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////
//   Now include platform specific definitions
////////////////////////////////////////////////////////////////////

/* Operating system recognition (basic) */
#if defined (__unix) || defined (__linux__) || defined (__QNX__) || defined (_AIX) || defined (__NetBSD__) || defined (__OpenBSD__) || defined (__Lynx__)
    #undef  _UNIX
    #define _UNIX 1
    #include "CLucene/config/PlatformUnix.h"

#elif defined(macintosh) || defined (_MAC) || defined(__APPLE__)
    #undef _MAC
    #define _MAC  1
    #include "CLucene/config/PlatformMac.h"

#elif defined (_WIN32) || defined (__WIN32) || defined (WIN32) || defined (__WIN32__)
    #undef _WIN32
    #define _WIN32
    #include "CLucene/config/PlatformWin32.h"

#elif defined (__WIN16) || defined (WIN16) || defined (_WIN16)
    #undef _WIN16
    #define _WIN16
    #error "CLucene has not been tested on this platform. Please send a report to the lucene administrators if you are able to successfully compile"
#else
    #error "CLucene could not identify the platform."
#endif /* platforms */



////////////////////////////////////////////////////////////////////
//   Now we take all that we have learnt, and define some things
////////////////////////////////////////////////////////////////////

//lets just say that we can always do unicode! :)
#ifdef CL_CHARSET_GUESS
	#define _UCS2
#endif

#if defined(_ASCII)
 #undef _UCS2
#elif defined(_UCS2)
 #undef _ASCII
#endif


#ifdef IS_BIG_ENDIAN
	#ifdef _LUCENE_PRAGMA_WARNINGS
	 #pragma message ("Big endian build hasn't been tested yet. Please send in your success/failure to the project.")
	#else
	 #warning "Big endian build hasn't been tested yet. Please send in your success/failure to the project."
	#endif
#endif

#ifndef  _LUCENE_NO_NEW_STYLE_CASTS
    #define __CONST_CAST(typ,var) const_cast<typ>(var)
    #define __REINTERPRET_CAST(typ,var) reinterpret_cast<typ>(var)
#else
    #define __CONST_CAST(typ,var) ((typ)(var))
    #define __REINTERPRET_CAST,var) ((typ)(var))
#endif

#ifndef _CL_ILONG
 #define _CL_ILONG(x)       x ## L
#endif
#ifndef _ILONGLONG
 #define _ILONGLONG(x)   x ## LL
#endif

//define whats the values of item intergers *should* be. we can check this in a test
#define LUCENE_INT64_MAX_SHOULDBE _ILONGLONG(0x7FFFFFFFFFFFFFFF)
#define LUCENE_INT32_MAX_SHOULDBE 0x7FFFFFFFL
#define LUCENE_UINT8_MAX_SHOULDBE 0xff

#ifdef PATH_MAX
 #define CL_MAX_PATH PATH_MAX
#elif defined(MAX_PATH)
 #define CL_MAX_PATH MAX_PATH
#elif defined(_MAX_PATH)
 #define CL_MAX_PATH _MAX_PATH
#else
 #define CL_MAX_PATH 256    /* Should be safe for any weird systems that do not define it */
#endif

#ifndef MAX_DIR
 #ifdef DIR_MAX
  #define MAX_DIR DIR_MAX
 #elif defined(_MAX_DIR)
  #define MAX_DIR _MAX_DIR
 #else
  #define MAX_DIR 32    /* Should be safe for any weird systems that do not define it */
 #endif
#endif // MAX_DIR

#define CL_MAX_DIR MAX_DIR*CL_MAX_PATH //32 directories * 256 bytes...

#ifdef _LARGE_FILES
    #define LUCENE_MAX_FILELENGTH LUCENE_INT64_MAX_SHOULDBE
#else
    #define LUCENE_MAX_FILELENGTH LUCENE_INT32_MAX_SHOULDBE
#endif

//end of lucene_compiler_h
#elif !defined(lucene_compiler_h2)
#define lucene_compiler_h2
//here we include the compiler header again, this gives the header a
//second chance at including stuff, after the main inclusions are complete

    #if defined (__GNUC__)
        #include "CLucene/config/CompilerGcc.h"
    
    #elif defined(_CLCOMPILER_MSVC)
        /* Microsoft Visual C++ */
        #include "CLucene/config/CompilerMsvc.h"
        
    #else
        //Unable to identify the compiler, issue error diagnostic. 
        //Edit <CLucene/config/LuceneMycomp.h> to set STLport up for your compiler.
        //Uncomment this next line
        #error "Unable to identify the compiler, issue error diagnostic. Edit <CLucene/config/CompilerMycomp.h> to set Lucene up for your compiler."
        #include "CLucene/config/LuceneMycomp.h"
    #endif /* end of compiler choice */
    
		#ifndef _CL_HAVE_FLOAT_T
		 #ifdef _CL_HAVE_LONG_DOUBLE
		    typedef long double float_t;    /* `float' expressions are evaluated as `long double'.  */
		 #else
		    typedef double float_t;
		 #endif
		#endif

		/*todo: but need to define SIZEOF_VOID_P #if (SIZEOF_VOID_P > 4 && SIZEOF_VOID_P <= 8)
			#ifndef _CL_HAVE_INTPTR_T
				typedef int64_t intptr_t;
			#endif
		#elif (SIZEOF_VOID_P > 2 && SIZEOF_VOID_P <= 4)
			#ifndef _CL_HAVE_INTPTR_T
				typedef int32_t intptr_t;
			#endif
		#else
			#error "void * is either >8 bytes or <= 2.  In either case, I am confused."
		#endif*/
		#ifndef _CL_HAVE_INTPTR_T
			typedef int64_t intptr_t;
		#endif
    
//end of lucene_compiler_h2
#elif !defined(lucene_compiler_h3)
#define lucene_compiler_h3
//here we include the compiler header again, this gives the header a
//third chance at including stuff, after the main inclusions are complete
 
    #if defined (__GNUC__ )
        #include "CLucene/config/CompilerGcc.h"
    
    #elif defined(_CLCOMPILER_MSVC)
        /* Microsoft Visual C++ */
        #include "CLucene/config/CompilerMsvc.h"
        
    #else
        //Unable to identify the compiler, issue error diagnostic. 
        //Edit <CLucene/config/LuceneMycomp.h> to set STLport up for your compiler.
        //Uncomment this next line
        #error "Unable to identify the compiler, issue error diagnostic. Edit <CLucene/config/CompilerMycomp.h> to set Lucene up for your compiler."
        #include "CLucene/config/LuceneMycomp.h"
    #endif /* end of compiler choice */
   
#endif
