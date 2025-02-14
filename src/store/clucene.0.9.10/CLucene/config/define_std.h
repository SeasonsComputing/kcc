#ifndef lucene_define_std
#define lucene_define_std
//including this file will define all definitions
//to only use native functions. Do this if your compiler
//contains all the functions that clucene needs (of course
//you can undefine some of these later if you dont *actually*
//have them.

//we support long files - 64 bit file functions
#define _LARGE_FILES

//support namespaces
#define _CL_HAVE_NAMESPACES

//support try/catch blocks
#define _CL_HAVE_FUNCTION_TRY_BLOCKS

//the normal headers
#define STDC_HEADERS
#define _CL_HAVE_STDARG_H
#define _CL_HAVE_ALGORITHM
#define _CL_HAVE_FUNCTIONAL
#define _CL_HAVE_MATH_H
#define _CL_HAVE_STL
#define _CL_HAVE_HASH_MAP
#define _CL_HAVE_HASH_SET
#define _CL_HAVE_MAP
#define _CL_HAVE_SET
#define _CL_HAVE_LIST
#define _CL_HAVE_VECTOR
#define _CL_HAVE_STDEXCEPT
#define _CL_HAVE_ERRNO_H
#define _CL_HAVE_SYS_STAT_H
#define _CL_HAVE_FCNTL_H

//character & std tchar support
#define _CL_HAVE_TCHAR_H
#ifdef _UCS2
    #define _CL_HAVE_WCTYPE_H
   #define _CL_HAVE_WCHAR_H
#else
	#define _CL_HAVE_CTYPE_H
#endif

//already have the normal structures
#define _CL_HAVE_FLOAT_T
#define _CL_HAVE_INTPTR_T

//system dependant:
#define _CL_HAVE_STRING_H //could be HAVE_STRINGS_H && HAVE_STRCHR
#define _CL_HAVE_SYS_TIMEB_H
#define _CL_HAVE_TIME_H

//todo: this is a bit of a hack.
#if defined (_WIN32) || defined (__WIN32) || defined (WIN32) || defined (__WIN32__)
    #define _CL_HAVE_IO_H
    #define _CL_HAVE_DIRECT_H
    #define _CL_HAVE_WINDOWS_H
#else
    #define _CL_HAVE_UNISTD_H
#endif

////////////////////////////////////////////////
//now for individual functions. some compilers
//miss these, so must individually define what
//we have
////////////////////////////////////////////////

//string functions
#define _CL_HAVE_STRLWR
#define _CL_HAVE_WCSLWR
#define _CL_HAVE_WCSCASECMP
#define _CL_HAVE_STRCASECMP

//formatting functions
#define _CL_HAVE_SNWPRINTF
#define _CL_HAVE_VSNWPRINTF
#define _CL_HAVE_WPRINTF
#define _CL_HAVE_SNPRINTF
#define _CL_HAVE_PRINTF


//conversion functions
#define _CL_HAVE_STRTOLL
#define _CL_HAVE_WCSTOLL
#define _CL_HAVE_WCSTOD
#define _CL_HAVE_LLTOA
#define _CL_HAVE_LLTOW
#define _CL_HAVE_INTPTR_T

//these ones are not standard (msvc)
//so you will probably need to undefine
//if you are not using msvc
#define _CL_HAVE_FILELENGTH


#endif
