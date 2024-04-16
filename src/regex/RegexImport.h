/*
 * Kuumba C++ Core
 *
 * $Id: RegexImport.h 15199 2007-03-09 17:57:17Z tvk $
 */

#if defined(KCC_WINDOWS)
#   pragma warning(push, 1)
#else
#   pragma GCC system_header
#endif
#define BOOST_REGEX_NO_LIB
#define BOOST_REGEX_STATIC_LINK
#define BOOST_REGEX_NO_EXTERNAL_TEMPLATES
#define BOOST_RE_REGEX_HPP
#include <boost/regex/v3/regex.hpp>
#include <boost/regex/src.cpp>
#if defined(KCC_WINDOWS)
#   pragma warning(pop)
#endif
