#include "CLucene/StdHeader.h"
#include "Arrays.h"

#ifdef _CL_HAVE_ALGORITHM
# include <algorithm>
#else
# error "Can't compile clucene without <algorithm>"
#endif

#ifdef _CL_HAVE_FUNCTIONAL
# include <functional>
#else
# error "Can't compile clucene without <functional>"
#endif

CL_NS_DEF(util)

CL_NS_END
