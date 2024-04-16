/*
 * Kuumba C++ Core
 *
 * $Id: Exception.cpp 15199 2007-03-09 17:57:17Z tvk $
 */
#include <inc/core/Core.h>

#define KCC_FILE "Exception"

namespace kcc
{
    Exception::Exception(const String& msg) throw () : m_msg(msg) { Log::scopeMark(); }
    Exception::~Exception() throw () {}
    const char* Exception::what() const throw () { return m_msg.c_str(); }
}
