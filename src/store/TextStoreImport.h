/*
 * Kuumba C++ Core
 *
 * $Id: TextStoreImport.h 17518 2007-05-11 23:40:44Z tvk $
 */

#if defined(KCC_WINDOWS)
#   pragma warning(push, 1)
#else
#   pragma GCC system_header
#endif
#if defined(KCC_CLUCENE_LIBRARY)
#   include <CLucene.h>
#else
#   include "CLucene/CLMonolithic.cpp"
#endif
#include "clucene-config.h"
#define KCC_CLUCENE_VERSION _CL_VERSION
#if defined(KCC_WINDOWS)
#   pragma warning(pop)
#endif
