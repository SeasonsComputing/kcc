#include "CLucene/StdHeader.h"

#include "CLucene/util/StringBuffer.h"
#include "CLucene/util/Misc.h"

bool _lucene_disable_debuglogging = true; //if LUCENE_ENABLE_CONSTRUCTOR_LOG is on, dont do log if this is true
bool _lucene_run_objectcheck = false; //run a memory check before deleting objects
int _lucene_counter_break = -1; //to break at this item, change this 
							 //and put break points at points below

CL_NS_USE(util)
CL_NS_DEF(debug)

#ifdef LUCENE_ENABLE_MEMLEAKTRACKING
int32_t _instance_counter = 0; //counter for initnumber

struct _file{
   int32_t refcount; ///times this has been used
   char* value; ///reference to the the basefile
}; //structure for name counting
struct _pointers{
   _file* file;
   int32_t initline;
   int32_t initnumber;
};//structure for pointer-filename references

typedef CL_NS(util)::CLSet<const char*,_file*,Compare::Char,Deletor::Dummy,Deletor::Void<_file> > defFile;
typedef CL_NS(util)::CLSet<LuceneBase*,_pointers*,Compare::Void<LuceneBase>,Deletor::Dummy,Deletor::Void<_pointers> > defPointer;
typedef CL_NS(util)::CLSet<void*,_pointers*,Compare::Void<void>,Deletor::Dummy,Deletor::Void<_pointers> > defVoid;

defFile LuceneBase_Files(false,true); //list of filenames used
defPointer LuceneBase_Pointers(false,true); //list of pointers counted
defVoid LuceneBase_Voids(false,true); //list of arbitary data added

//variables to trim filenames to just the base names
char _files_trim_string[CL_MAX_DIR];
int32_t _files_trim_start=-1;

//trim the filename and return the refcounted _file* structure
_file* get_file(const char* file){
	if ( _files_trim_start == -1 ){
		//this trims the start of the name file name so
		//that the whole of the filename is not stored - more asthetic :)
		//need to find the base
		_files_trim_start = strlen(__FILE__) - 21; //(length of debug/memtracking.cpp)
		strcpy(_files_trim_string,__FILE__);
		_files_trim_string[_files_trim_start] = 0;
	}
	if ( strncmp(file,_files_trim_string,_files_trim_start) == 0 ){
		//this file should be within the same directory area as we found lucenebase.cpp
		//to be, lets trim the start
		file+=_files_trim_start;
	}

   //now return an existing files structure (with refcount++) or create a new one
   defFile::iterator itr = LuceneBase_Files.find((const char*)file);
   if ( itr != LuceneBase_Files.end() ){
      _file* bf = itr->second;
      bf->refcount++;
      return bf;
   }else{
      _file* ref = new _file;
      ref->value = new char[strlen(file)+1]; //cannot use _CL_NEWARRAY otherwise recursion
	  strcpy(ref->value,file);

      ref->refcount = 1;
	  LuceneBase_Files.insert(pair<const char*,_file*>(ref->value,ref));
	  return ref;
   }
}

void remove_file(_file* bf){
	bf->refcount--;
    if ( bf->refcount <= 0 ){
			defFile::iterator fi = LuceneBase_Files.find(bf->value);
			CND_PRECONDITION(fi!=LuceneBase_Files.end(),"fi==NULL");
			delete[] bf->value;
	    LuceneBase_Files.removeitr(fi);
    }
}

#ifdef LUCENE_ENABLE_CONSTRUCTOR_LOG
	void constructor_log(const char* type,const char* file,const int line, const int size){
		if ( _lucene_disable_debuglogging ){
			FILE* f = fopen("clucene.log","a");
			char buf[CL_MAX_DIR+5];
			sprintf(buf,"%s,%s,%d,%d\n",type,file,line,size);
			fwrite(buf,sizeof(char),strlen(buf),f);
			fclose(f);
		}
	}
	#define CONSTRUCTOR_LOG(type,file,line,size) constructor_log(type,file,line,size)
#else
	#define CONSTRUCTOR_LOG(type,file,line,size)
#endif

////////////////////////////////////////////////////////////////////////////////
// the _CLNEW&_CLDELETE new/delete operators
////////////////////////////////////////////////////////////////////////////////
void* LuceneBase::operator new (size_t size, const char * file, int32_t line)
{
   void* p = malloc (size);
   LuceneBase* lb = (LuceneBase*)p;

   //create the pointer struct
   _file* br = get_file(file);
   _pointers* bp = new _pointers;
   bp->file = br;
   bp->initnumber = _instance_counter++;
   bp->initline = line;

   //associate this object with the pointer
   lb->__cl_initnum = bp->initnumber;

   //break if necessary
	if ( _lucene_counter_break == lb->__cl_initnum )
		CLDebugBreak(); //put break point here

	//add the pointer object
	LuceneBase_Pointers.insert(pair<LuceneBase*,_pointers*>(lb, bp));

	CONSTRUCTOR_LOG("newobj",file,line,size);
   return p;
} 
void LuceneBase::operator delete (void *p, char const * file, int32_t line)
{
	LuceneBase* lb=(LuceneBase*)p;
    defPointer::iterator itr = LuceneBase_Pointers.find(lb);
    if ( itr != LuceneBase_Pointers.end() ){
      _pointers* bp = itr->second;
	  remove_file(bp->file);

      LuceneBase_Pointers.removeitr(itr);
    }else{
       //break
    }
	free(p); 
} 

///////////////////////////////////////////////////////////////////////////
// the generic new/delete operators
///////////////////////////////////////////////////////////////////////////
void* LuceneBase::operator new (size_t size)
{
   void* p = malloc (size);
	LuceneBase* lb = (LuceneBase*)p;

   //create the pointer struct
   _file* br = get_file("undefined");
   _pointers* bp = new _pointers;
   bp->file = br;
   bp->initnumber = _instance_counter++;
   bp->initline = -1;

   //associate this object with the pointer
   lb->__cl_initnum = bp->initnumber;

   //break if necessary
	if ( _lucene_counter_break == lb->__cl_initnum )
		CLDebugBreak(); //put break point here. todo: use win32 break feature, if available

	//add the pointer object
	LuceneBase_Pointers.insert(pair<LuceneBase*,_pointers*>(lb,bp));
	
	CONSTRUCTOR_LOG("newobj","unknown",-1,size);
   return p;
} 
void LuceneBase::operator delete (void *p)
{
	LuceneBase* lb=(LuceneBase*)p;

	defPointer::iterator itr = LuceneBase_Pointers.find(lb);
	if ( itr != LuceneBase_Pointers.end() ){
		_pointers* bp = itr->second;
		remove_file(bp->file);
		LuceneBase_Pointers.removeitr(itr);
	}else{
		//todo: ?? break
	}
	free(p); 
}

///////////////////////////////////////////////////////////////////////////
// other memtracking functions
///////////////////////////////////////////////////////////////////////////
void LuceneBase::__cl_unregister(void* obj){
	LuceneBase* lb=(LuceneBase*)obj;
    defPointer::iterator itr = LuceneBase_Pointers.find(lb);
	CND_PRECONDITION(itr != LuceneBase_Pointers.end(),"__cl_unregister object not found");
	_pointers* bp = itr->second;
    LuceneBase_Pointers.removeitr(itr);
}

void* LuceneBase::__cl_voidpadd(void* data, const char* file, int line,size_t size){
	_file* br = get_file(file);
	_pointers* bp = new _pointers;
	bp->file = br;
	bp->initnumber = _instance_counter++;
	bp->initline = line;

	LuceneBase_Voids.insert(pair<void*,_pointers*>(data,bp));
	CONSTRUCTOR_LOG("newarr",file,line,size);
	return data;
}
void LuceneBase::__cl_voidpremove(void* data, const char* file, int line){
	defVoid::iterator itr = LuceneBase_Voids.find(data);
    if ( itr != LuceneBase_Voids.end() ){
      _pointers* bp = itr->second;
      remove_file(bp->file);
      LuceneBase_Voids.removeitr(itr);
    }else{
       printf("Data deleted when not added with _CL_NEWARRAY in %s at %d\n",file,line);
    } 
}


////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
//The lucene base memory leak printout functions
////////////////////////////////////////////////////////////
//static
void __internalcl_PrintUnclosedObject(bool isObject, StringBuffer& sb,_pointers* bp,_file* bf, bool print){
	sb.append(_T("   ") );
	sb.appendInt(bp->initnumber);
	if ( isObject ){
		sb.append(_T("(obj). "));
	}else{
		sb.append(_T(". "));
	}

	TCHAR tmp[CL_MAX_DIR];
	/*if ( lb->__cl_initfile < 0 || lb->__cl_initfile >= LuceneBase_Files.size() )
		_tcscpy(tmp,_T("unknown (filename lost), use number to identify object instead"));
	else
		STRCPY_AtoT(tmp,LuceneBase_Files.at(lb->__cl_initfile),CL_MAX_DIR);*/
    STRCPY_AtoT(tmp,bf->value,CL_MAX_DIR);
	sb.append(tmp);
	
	sb.append(_T(", line ") );
	sb.appendInt(bp->initline);
	sb.append(_T("\n"));

	if ( print && sb.length() > 0 ){
		_tprintf(sb.getBuffer());
		sb.clear();
	}
}
TCHAR* __internalcl_GetUnclosedObjects(bool print){
	StringBuffer sb;
   bool unknowns = false;
	if ( LuceneBase_Pointers.size() > 0 ){
		sb.appendInt((int32_t)LuceneBase_Pointers.size());
		sb.append(_T(" clucene objects are still open\n"));

      defPointer::iterator itr = LuceneBase_Pointers.begin();
      while ( itr != LuceneBase_Pointers.end() ){
         //LuceneBase* lb = itr->first;
         _pointers* bp = itr->second;
         _file* bf = bp->file;

		if ( bp->initline == -1 )
			unknowns = true;
		 __internalcl_PrintUnclosedObject(true, sb,bp,bf,print);

         itr++;
	  }

	  defVoid::iterator itr2 = LuceneBase_Voids.begin();
      while ( itr2 != LuceneBase_Voids.end() ){
         _pointers* bp = itr2->second;
         _file* bf = bp->file;

		if ( bp->initline == -1 )
			unknowns = true;
		 __internalcl_PrintUnclosedObject(false, sb,bp,bf,print);

         itr2++;
	  }
	}

	if ( unknowns == true ){
		sb.append(_T("*** Some memory was not created with _CLNEW and was not tracked... ***\n") );
		sb.append(_T("*** Use _CLNEW instead of new when creating CLucene objects ***\n") );
		sb.append(_T("*** Memory may also have not been freed in the current context ***\n") );
	}
	
	if ( print ){
		if ( sb.length() > 0 ){
			_tprintf(sb.getBuffer());
			sb.clear();
		}
		return NULL;
	}else{
		if ( sb.length() > 0 )
			return stringDuplicate(sb.getBuffer());
		else
			return NULL;
	}
}

void LuceneBase::__cl_ClearMemory(){
	while ( LuceneBase_Files.size() > 0 ){
		defFile::iterator fi = LuceneBase_Files.begin();
		_file* f = fi->second;
		delete[] f->value;
        LuceneBase_Files.removeitr (fi);
	}
   LuceneBase_Pointers.clear();
   LuceneBase_Voids.clear();
}
TCHAR* LuceneBase::__cl_GetUnclosedObjects(){
	return __internalcl_GetUnclosedObjects(false);
}
//static
int32_t LuceneBase::__cl_GetUnclosedObjectsCount(){
    return LuceneBase_Pointers.size();
}

const char* LuceneBase::__cl_GetUnclosedObject(int32_t item){
   defPointer::iterator itr=LuceneBase_Pointers.begin();
   int32_t i=0;
   for ( ;itr!=LuceneBase_Pointers.end() && i<item ;itr++ ){
      i++;
   }
   if ( itr != LuceneBase_Pointers.end() )
      return itr->second->file->value;
   else
      return NULL;
}
void LuceneBase::__cl_PrintUnclosedObjects(){
	__internalcl_GetUnclosedObjects(true);
}




bool IsValidLuceneBaseObject(void* d, char* errormsg, bool nullIsInvalid, const char* file, const int32_t line){
	if(d==NULL){
		if ( nullIsInvalid )
			return true;
		else if( errormsg != NULL ){
#ifdef _CL__CND_DEBUG
			CND__EXITCONDITION(false,file,line,CND_STR_CONDITION,errormsg);
#else
			printf("%s\n",errormsg);
#endif
		}
		return !nullIsInvalid;
	}
   //LuceneBase* d = (LuceneBase*)v;

   defPointer::iterator itr=LuceneBase_Pointers.begin();
   for ( ;itr!=LuceneBase_Pointers.end() ;itr++ ){
	   void* i = itr->first;
      if ( i == d )
		  return true;
   }

   defVoid::iterator itr2=LuceneBase_Voids.begin();
   for ( ;itr2!=LuceneBase_Voids.end() ;itr2++ ){
	  void* i = itr2->first;
      if ( i == d )
		  return true;
   }

   if ( errormsg != NULL ){
#ifdef _CL__CND_DEBUG
		CND__EXITCONDITION(false,file,line,CND_STR_CONDITION,errormsg);
#else
		printf("%s\n",errormsg);
#endif
	}

   return false;
}
#endif //LUCENE_ENABLE_MEMLEAKTRACKING
////////////////////////////////////////////////////////////

CL_NS_END
