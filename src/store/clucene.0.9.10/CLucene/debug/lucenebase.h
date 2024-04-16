#ifndef _lucene_debug_lucenebase_
#define _lucene_debug_lucenebase_

#ifdef _LUCENE_PRAGMA_ONCE
# pragma once
#endif

CL_NS_DEF(debug)

class LuceneBase{
public:
#ifdef LUCENE_ENABLE_LUCENEBASE
	static void* operator new (size_t size, char const * file, int_t line);
	static void* operator new (size_t size);
	static void operator delete (void *p, char const * file, int_t line);
	static void operator delete (void *p);
	static int_t __cl_GetUnclosedObjectsCount();  ///< gets the number of unclosed objects
	static const char* __cl_GetUnclosedObject(int_t item);  ///< get the name of the nth unclosed object
	static TCHAR* __cl_GetUnclosedObjects();  ///< get a string with the names of the unclosed objects
	static void __cl_PrintUnclosedObjects(); ///< print unclosed objects to the stdout
   static void __cl_ClearMemory(); ///< delete memory in refcounted list

	int_t __cl_initnum; ///< The order that the object was created at.
#else
	static void* operator new (size_t size);
	static void operator delete (void *p);
#endif //LUCENE_ENABLE_LUCENEBASE
#ifdef LUCENE_ENABLE_REFCOUNT
	int __cl_refcount;
	LuceneBase(){
		__cl_refcount=0;
	}
	int __cl_getref(){
		return __cl_refcount;
	}
	void __cl_addref(){
		__cl_refcount++;
	}
	int __cl_decref(){
		return --__cl_refcount;
	}
#endif //LUCENE_ENABLE_REFCOUNT
};

#ifdef LUCENE_ENABLE_REFCOUNT
	//we use this in the _CLDELETE macro
#ifdef _DEBUG
	bool lucene_refCount_CanDelete(const LuceneBase* d, const char* file, const int_t line);
	bool lucene_refCount_CanDelete(const void* d, const char* file, const int_t line);
#else
	bool lucene_refCount_CanDelete(const void* d);
	bool lucene_refCount_CanDelete(const LuceneBase* d);
#endif
#endif

CL_NS_END
#endif //_lucene_debug_lucenebase_
