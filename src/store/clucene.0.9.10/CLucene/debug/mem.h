#ifndef _lucene_debug_lucenebase_
#define _lucene_debug_lucenebase_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif


//some memory pool definitions
#ifdef LUCENE_ENABLE_MEMORY_POOL
 #define LUCENE_MP_DEFINE(cls) extern lucene::debug::pool<> cls ## _mempool
#else
 #define LUCENE_MP_DEFINE(cls)
#endif

//Macro for creating new objects
//todo: currently mempool and lucenebase are exclusive of each other
//this doesnt need to be the case...
#if defined(LUCENE_ENABLE_MEMORY_POOL)
   #define _CLNEW new
   #define LUCENE_BASE public CL_NS(debug)::LuceneVoidBase
   #define LUCENE_POOLEDBASE(sc) public CL_NS(debug)::LucenePoolBase<&sc ## _mempool>
#elif defined(LUCENE_ENABLE_MEMLEAKTRACKING)
   #define _CLNEW new(__FILE__, __LINE__)
   #define LUCENE_BASE public CL_NS(debug)::LuceneBase
   #define LUCENE_POOLEDBASE(sc) LUCENE_BASE
#elif defined(LUCENE_ENABLE_REFCOUNT)
   #define _CLNEW new
   #define LUCENE_BASE public CL_NS(debug)::LuceneBase
   #define LUCENE_POOLEDBASE(sc) LUCENE_BASE
#else
   #define _CLNEW new
   #define LUCENE_BASE public CL_NS(debug)::LuceneVoidBase
   #define LUCENE_POOLEDBASE(sc) LUCENE_BASE
   #define LUCENE_BASE_CHECK(obj) (obj)->dummy__see_mem_h_for_details
#endif
#define _CL_POINTER(x) (x->__cl_addref()>=0?x:x) //return a add-ref'd object
#define LUCENE_REFBASE public CL_NS(debug)::LuceneBase //this is the base of classes who *always* need refcounting

#if defined(_DEBUG)
  #if !defined(LUCENE_BASE_CHECK)
		#define LUCENE_BASE_CHECK(x)
	#endif
#else
	#undef LUCENE_BASE_CHECK
	#define LUCENE_BASE_CHECK(x)
#endif

//Macro for creating new arrays
#ifdef LUCENE_ENABLE_MEMLEAKTRACKING
	#define _CL_NEWARRAY(type,size) (type*)CL_NS(debug)::LuceneBase::__cl_voidpadd(new type[(size_t)size],__FILE__,__LINE__,(size_t)size);
	#define _CLDELETE_ARRAY(x) if (x!=NULL){CL_NS(debug)::LuceneBase::__cl_voidpremove((void*)x,__FILE__,__LINE__); delete [] x; x=NULL;}
	#define _CLDELETE_LARRAY(x) if (x!=NULL){CL_NS(debug)::LuceneBase::__cl_voidpremove((void*)x,__FILE__,__LINE__);delete [] x;}
	#ifndef _CLDELETE_CARRAY
		#define _CLDELETE_CARRAY(x) if (x!=NULL){CL_NS(debug)::LuceneBase::__cl_voidpremove((void*)x,__FILE__,__LINE__);delete [] x; x=NULL;}
		#define _CLDELETE_LCARRAY(x) if (x!=NULL){CL_NS(debug)::LuceneBase::__cl_voidpremove((void*)x,__FILE__,__LINE__);delete [] x;}
	#endif
#else
	#define _CL_NEWARRAY(type,size) new type[size]
	#define _CLDELETE_ARRAY(x) if (x!=NULL){delete [] x; x=NULL;}
	#define _CLDELETE_LARRAY(x) if (x!=NULL){delete [] x;}
	#ifndef _CLDELETE_CARRAY //todo: check msvc6
		#define _CLDELETE_CARRAY(x) if (x!=NULL){delete [] x; x=NULL;}
		#define _CLDELETE_LCARRAY(x) if (x!=NULL){delete [] x;}
	#endif
#endif
//a shortcut for deleting a carray and all its contents
#define _CLDELETE_CARRAY_ALL(x) {for(int xcda=0;x[xcda]!=NULL;xcda++)_CLDELETE_CARRAY(x[xcda]);}_CLDELETE_ARRAY(x);
#ifndef _CLDELETE_CaARRAY
		#define _CLDELETE_CaARRAY _CLDELETE_CARRAY
		#define _CLDELETE_LCaARRAY _CLDELETE_LCARRAY
#endif

//Macro for deleting
#ifdef LUCENE_ENABLE_REFCOUNT
	//todo: fix comments, if(CL_NS(debug)::lucene_refCount_CanDelete(x,__FILE__,__LINE__))
		//If lucene_refCount_CanDelete returns true then we can delete. 
		//this is useful for managed objects, where the object would not expect
		//to see itself deleted when passed to another object. The managed
		//object would call __cl_addref() when initiating the object so that
		//any call to _CLDELETE will not delete it until _CLDECDELETE
		
	#ifdef _DEBUG
		#define _CLDELETE(x) if (x!=NULL){ if ( _lucene_run_objectcheck ){ CL_ISVALIDOBJECT(x,#x " is INVALID");} CND_PRECONDITION((x)->__cl_refcount>=0,"__cl_refcount was < 0"); if ((x)->__cl_decref() <= 0)delete x; x=NULL; }
		#define _CLLDELETE(x) if (x!=NULL){ if ( _lucene_run_objectcheck ) {CL_ISVALIDOBJECT(x,#x " is INVALID");} CND_PRECONDITION((x)->__cl_refcount>=0,"__cl_refcount was < 0"); if ((x)->__cl_decref() <= 0)delete x; }
	#else
		#define _CLDELETE(x) if (x!=NULL){ CND_PRECONDITION((x)->__cl_refcount>=0,"__cl_refcount was < 0"); if ((x)->__cl_decref() <= 0)delete x; x=NULL; }
		#define _CLLDELETE(x) if (x!=NULL){ CND_PRECONDITION((x)->__cl_refcount>=0,"__cl_refcount was < 0"); if ((x)->__cl_decref() <= 0)delete x; }
	#endif
#else
	#define _CLDELETE(x) if (x!=NULL){ LUCENE_BASE_CHECK(x); delete x; x=NULL; }
	#define _CLLDELETE(x) if (x!=NULL){ LUCENE_BASE_CHECK(x); delete x; }
#endif

//_CLDECDELETE deletes objects which are *always* refcounted
#ifdef _DEBUG
	#define _CLDECDELETE(x) if (x!=NULL){ if ( _lucene_run_objectcheck ){ CL_ISVALIDOBJECT(x,#x " is INVALID")}; CND_PRECONDITION((x)->__cl_refcount>0,"__cl_refcount was < 0"); if ((x)->__cl_decref() <= 0)delete x; x=NULL; }
	#define _CLLDECDELETE(x) if (x!=NULL){ if ( _lucene_run_objectcheck ){ CL_ISVALIDOBJECT(x,#x " is INVALID")}; CND_PRECONDITION((x)->__cl_refcount>0,"__cl_refcount was < 0"); if ((x)->__cl_decref() <= 0)delete x; }
#else
	#define _CLDECDELETE(x) if (x!=NULL){ CND_PRECONDITION((x)->__cl_refcount>=0,"__cl_refcount was < 0"); if ((x)->__cl_decref() <= 0)delete x; x=NULL; }
	#define _CLLDECDELETE(x) if (x!=NULL){ CND_PRECONDITION((x)->__cl_refcount>=0,"__cl_refcount was < 0"); if ((x)->__cl_decref() <= 0)delete x; }
#endif
//_VDelete should be used for deleting non-clucene objects.
//when using reference counting, _CLDELETE casts the object
//into a LuceneBase*.
#define _CLVDELETE(x) if(x!=NULL){delete x; x=NULL;}


CL_NS_DEF(debug)
class LuceneVoidBase{
	public:
	#ifdef _DEBUG
		//a compile time check to make sure that _CLDELETE and _CLDECDELETE is being
		//used correctly.
		int dummy__see_mem_h_for_details; 
	#endif
        virtual ~LuceneVoidBase(){};
};

//Lucenebase is the superclass of all clucene objects. It provides
//memory debugging tracking and/or reference counting
class LuceneBase{
public:
#ifdef LUCENE_ENABLE_MEMLEAKTRACKING
	static void* operator new (size_t size);
	static void operator delete (void *p);
	int32_t __cl_initnum; ///< The order that the object was created at. This is then used to do a lookup in the objects list

	static void* operator new (size_t size, char const * file, int32_t line);
	static void operator delete (void *p, char const * file, int32_t line);

	static void* __cl_voidpadd(void* data, const char* file, int line, size_t size); ///<add arbitary data to the lucenbase_list and returns the same data
	static void __cl_voidpremove(void* data, const char* file, int line);///<remove arbitary data to the lucenbase_list
	static void __cl_unregister(void* obj); ///<un register object from the mem leak and ref count system

	static int32_t __cl_GetUnclosedObjectsCount();  ///< gets the number of unclosed objects
	static const char* __cl_GetUnclosedObject(int32_t item);  ///< get the name of the nth unclosed object
	static TCHAR* __cl_GetUnclosedObjects();  ///< get a string with the names of the unclosed objects
	static void __cl_PrintUnclosedObjects(); ///< print unclosed objects to the stdout
  
  ///This will clear memory relating to refcounting
	///other tools can be used to more accurately identify
	///memory leaks. This should only be called just
	///before closing, and after retrieving the
	///unclosed object list
  static void __cl_ClearMemory();

#endif //LUCENE_ENABLE_MEMLEAKTRACKING

	int __cl_refcount;
	LuceneBase(){
		__cl_refcount=1;
	}
	inline int __cl_getref(){
		return __cl_refcount;
	}
	inline int __cl_addref(){
		__cl_refcount++;
		return __cl_refcount;
	}
	inline int __cl_decref(){
		__cl_refcount--;
		return __cl_refcount;
	}
    virtual ~LuceneBase(){};
};

  #ifdef LUCENE_ENABLE_MEMLEAKTRACKING
  	/* Checks that the specified object is a valid object by looking through
		 * the list of currently open objects.
		 * If errormsg!=NULL, the function uses the CND_DEBUG to throw an error
		 *
		 * todo: this is not working reliably yet...
		 */
		bool IsValidLuceneBaseObject(void* v, char* errormsg, bool nullIsInvalid, const char* file, const int32_t line);
		#define CL_ISVALIDOBJECT(x,msg) CL_NS(debug)::IsValidLuceneBaseObject(x,msg,false,__FILE__,__LINE__);
		#define CL_ISVALIDORNULLOBJECT(x,msg) CL_NS(debug)::IsValidLuceneBaseObject(x,msg,true,__FILE__,__LINE__);
	#else
		#define CL_ISVALIDOBJECT(x,msg)
		#define CL_ISVALIDORNULLOBJECT(x,msg)
	#endif
CL_NS_END
#endif //_lucene_debug_lucenebase_
