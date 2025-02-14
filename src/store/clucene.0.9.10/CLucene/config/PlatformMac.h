// It is internal CLucene header - DO NOT include it directly

#ifndef PLATFORM_DEFAULT_READER_ENCODING
    #define PLATFORM_DEFAULT_READER_ENCODING CL_NS(util)::Reader::ENCODING_ASCII
#endif

# define PATH_DELIMITER _T("/")
# define PATH_DELIMITERA "/"
# define PATH_DELIMITERC '/'

#  if (__GNUC__ < 3) && !defined( __APPLE_CC__)
// GCC strange "ignore std" mode works better if you pretend everything
// is in the std namespace, for the most part.
#    define LUCENE_NO_STDC_NAMESPACE
#  endif

#undef _T //apple has something else strange here...
