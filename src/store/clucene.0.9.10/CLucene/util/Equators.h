#ifndef _lucene_util_Equators_
#define _lucene_util_Equators_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

CL_NS_DEF(util)

////////////////////////////////////////////////////////////////////////////////
// Equators
////////////////////////////////////////////////////////////////////////////////
class Equals{
public:
	class Int32:public CL_NS_STD(binary_function)<const int32_t*,const int32_t*,bool>
	{
	public:
		bool operator()( const int32_t val1, const int32_t val2 ) const{
			return (val1)==(val2);
		}
	};
	
	  ///\exclude
	class Char:public CL_NS_STD(binary_function)<const char*,const char*,bool>
	{
	public:
		bool operator()( const char* val1, const char* val2 ) const{
			bool ret = (strcmp( val1,val2 ) == 0);
			return ret;
		}
	};
#ifdef _UCS2
   ///\exclude
	class WChar: public CL_NS_STD(binary_function)<const wchar_t*,const wchar_t*,bool>
	{
	public:
		bool operator()( const wchar_t* val1, const wchar_t* val2 ) const{
			bool ret = (_tcscmp( val1,val2 ) == 0);
			return ret;
		}
	};
	class TChar: public WChar{
	};
#else
	class TChar: public Char{
	};
#endif


   ///\exclude
	class Void:public CL_NS_STD(binary_function)<const void*,const void*,bool>
	{
	public:
		bool operator()( const void* val1, const void* val2 ) const{
			bool ret = val1==val2;
			return ret;
		}
	};
};


////////////////////////////////////////////////////////////////////////////////
// Comparors
////////////////////////////////////////////////////////////////////////////////
	
class Comparable:LUCENE_BASE{
public:
   virtual ~Comparable(){
   }
   
	virtual int32_t compareTo(void* o) = 0;
};



class Compare{
public:
	class _base
	{	// traits class for hash containers
	public:
		enum
		{	// parameters for hash table
			bucket_size = 4,	// 0 < bucket_size
			min_buckets = 8
		};	// min_buckets = 2 ^^ N, 0 < N

		_base()
		{
		}
	};

	class Int32:public _base, public Comparable{
	int32_t value;
	public:
		int32_t getValue(){ return value; }
		Int32(int32_t val){
			value = val;
		}
		Int32(){
			value = 0;
		}
		int32_t compareTo(void* o){
			try{
				Int32* other = (Int32*)o;
				if (value == other->value)
					return 0;
				// Returns just -1 or 1 on inequality; doing math might overflow.
				return value > other->value ? 1 : -1;
			}catch(...){
				_CLTHROWA(CL_ERR_Runtime, "Couldnt compare types");
			}  
		}
		
		bool operator()( int32_t t1, int32_t t2 ) const{
			return t1 > t2 ? true : false;
		}
		size_t operator()( int32_t t ) const{
			return t;
		}
	};

	
	class Float:public Comparable{
		float_t value;
	public:
		float_t getValue(){
			return value;
		}
		Float(float_t val){
			value = val;
		}
		int32_t compareTo(void* o){
			try{
				Float* other = (Float*)o;
				if (value == other->value)
					return 0;
				// Returns just -1 or 1 on inequality; doing math might overflow.
				return value > other->value ? 1 : -1;
			}catch(...){
				_CLTHROWA(CL_ERR_Runtime,"Couldnt compare types");
			}  
		}
	};


	class Char: public _base //<char*>
	{
	public:
		bool operator()( const char* val1, const char* val2 ) const{
			if ( val1==val2)
				return false;
			bool ret = (strcmp( val1,val2 ) < 0);
			return ret;
		}
		size_t operator()( const char* val1) const{
			return CL_NS(util)::Misc::ahashCode(val1);
		}
	};

#ifdef _UCS2
	class WChar: public _base //<wchar_t*>
	{
	public:
		bool operator()( const wchar_t* val1, const wchar_t* val2 ) const{
			if ( val1==val2)
				return false;
			bool ret = (_tcscmp( val1,val2 ) < 0);
			return ret;
		}
		size_t operator()( const wchar_t* val1) const{
			return CL_NS(util)::Misc::whashCode(val1);
		}
	};
#endif

	class TChar: public _base, public Comparable{
	  const TCHAR* s;
	public:
		const TCHAR* getValue(){ return s; }

		TChar(){
			s=NULL;
		}
		TChar(const TCHAR* str){
			this->s = str;
		}
		int32_t compareTo(void* o){
			try{
				TChar* os = (TChar*)o;
				return _tcscmp(s,os->s);
			}catch(...){
				_CLTHROWA(CL_ERR_Runtime,"Couldnt compare types");
			}  

		}
		
		bool operator()( const TCHAR* val1, const TCHAR* val2 ) const{
			if ( val1==val2)
				return false;
			bool ret = (_tcscmp( val1,val2 ) < 0);
			return ret;
		}
		size_t operator()( const TCHAR* val1) const{
			return CL_NS(util)::Misc::thashCode(val1);
		}

	};


	template<typename _cl>
	class Void:public _base //<const void*,const void*,bool>
	{
	public:
		int32_t compareTo(_cl* o){
			if ( this == o )
				return o;
			else
				return this > o ? 1 : -1;
		}
		bool operator()( _cl* t1, _cl* t2 ) const{
			return t1 > t2 ? true : false;
		}
		size_t operator()( _cl* t ) const{
			return (size_t)t;
		}
	};
};

////////////////////////////////////////////////////////////////////////////////
// allocators
////////////////////////////////////////////////////////////////////////////////
class Deletor{
public:

template<typename _kt>
	class Array{
	public:
		static void doDelete(_kt* arr){
			_CLDELETE_LARRAY(arr);
		}
	};
	class tcArray{
	public:
		static void doDelete(const TCHAR* arr){
			_CLDELETE_CARRAY(arr);
		}
	};
	class acArray{
	public:
		static void doDelete(const char* arr){
			_CLDELETE_CaARRAY(arr);
		}
	};

	class Unintern{
	public:
		static void doDelete(TCHAR* arr);
	};
	template<typename _kt>
	class Object{
	public:
		static void doDelete(_kt* obj){
			_CLLDELETE(obj);
		}
	};
	template<typename _kt>
	class Void{
	public:
		static void doDelete(_kt* obj){
			_CLVDELETE(obj);
		}
	};
	class Dummy{
	public:
		static void doDelete(const void* nothing){
			//todo: remove all occurances where it hits this point
			//CND_WARNING(false,"Deletor::Dummy::doDelete run, set deleteKey or deleteValue to false");
		}
	};
	class DummyInt32{
	public:
		static void doDelete(const int32_t nothing){
		}
	};
	class DummyFloat{
	public:
		static void doDelete(const float_t nothing){
		}
	};
	template <typename _type>
	class NullVal{
	public:
		static void doDelete(const _type nothing){
			//todo: remove all occurances where it hits this point
			//CND_WARNING(false,"Deletor::Dummy::doDelete run, set deleteKey or deleteValue to false");
		}
	};
};
////////////////////////////////////////////////////////////////////////////////

CL_NS_END
#endif
