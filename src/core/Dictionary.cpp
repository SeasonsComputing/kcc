/*
 * Kuumba C++ Core
 *
 * $Id: Dictionary.cpp 15199 2007-03-09 17:57:17Z tvk $
 */
#include <inc/core/Core.h>

#define KCC_FILE "Dictionary"

namespace kcc
{
    // empty: empty properties object
    const Dictionary& Dictionary::empty()
    {
        static const Dictionary k_empty;
        return k_empty;
    }
}
