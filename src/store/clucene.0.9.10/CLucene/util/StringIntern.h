#ifndef _lucene_util_StringIntern_H
#define _lucene_util_StringIntern_H

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#include "VoidMap.h"
CL_NS_DEF(util)
typedef CL_NS(util)::CLHashMap<const TCHAR*,int,Compare::TChar,Deletor::tcArray ,Deletor::NullVal<int> > __wcsintrntype;
typedef CL_NS(util)::CLHashMap<const char*,int,Compare::Char,Deletor::acArray ,Deletor::NullVal<int> > __strintrntype;

  /** Functions for intern'ing strings. This
  is a process of pooling strings thus using less memory,
  and furthermore allows intern'd strings to be directly
  compared:
   string1==string2, rather than _tcscmp(string1,string2)
  */
  class CLStringIntern{
  public:
#ifndef LUCENE_HIDE_INTERNAL
	  //internal:
	  static __wcsintrntype stringPool;
	  static __strintrntype stringaPool;
#endif

	//internalise an ucs2 string and return an iterator for fast un-iteration
	static __wcsintrntype::iterator internitr(const TCHAR* str CL_FILELINEPARAM);
	//uninternalise a string using an iterator 
	static bool uninternitr(__wcsintrntype::iterator itr);
	
	///Internalise the specified string.
	///\return Returns a pointer to the internalised string
	static const char* internA(const char* str CL_FILELINEPARAM);
	///Uninternalise the specified string. Decreases
	///the reference count and frees the string if 
	///reference count is zero
	///\returns true if string was destroyed, otherwise false
	static bool uninternA(const char* str);

	///Internalise the specified string.
	///\return Returns a pointer to the internalised string
	static const TCHAR* intern(const TCHAR* str CL_FILELINEPARAM);
	
	///Uninternalise the specified string. Decreases
	///the reference count and frees the string if 
	///reference count is zero
	///\returns true if string was destroyed, otherwise false
	static bool unintern(const TCHAR* str);

    static void shutdown();
  };

CL_NS_END
#endif
