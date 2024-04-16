#ifndef _lucene_util_VoidList_
#define _lucene_util_VoidList_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

CL_NS_DEF(util)

	template<typename _kt,typename _base,typename _valueDeletor> 
	class __CLList:public _base,LUCENE_BASE {
	private:
		bool dv;
		typedef _base base;
	public:
		typedef typename _base::const_iterator const_iterator;
		typedef typename _base::iterator iterator;

		virtual ~__CLList(){
			clear();
		}

		__CLList ( const bool deleteValue ):
			dv(deleteValue)
		{
		}

		void setDoDelete(bool val){ dv=val; }

		//sets array to the contents of this array.
		//array must be size+1, otherwise memory may be overwritten
		void toArray(_kt* into){
			int i=0;
			for ( iterator itr=base::begin();itr!=base::end();itr++ ){
				into[i] = *itr;
				i++;
			}
			into[i] = NULL;
		}

		//add the item at the most efficient point in the list
		void put( _kt k ){ 
			base::push_back(k); 
		}
		
		void set(int32_t i, _kt val) {
			if ( dv ) 
				_valueDeletor::doDelete((*this)[i]);
			(*this)[i] = val;
		}

		//todo: check this
		void delete_back(){
			if ( base::size() > 0 ){
				iterator itr = base::end();
				if ( itr != base::begin()) 
					itr --;
				_kt key = *itr;
				base::erase(itr);
				if ( dv ) 
					_valueDeletor::doDelete(key);
			}
		}

		void delete_front(){
			if ( base::size() > 0 ){
				iterator itr = base::begin();
				_kt key = *itr;
				base::erase(itr);
				if ( dv ) 
					_valueDeletor::doDelete(key);
			}
		}

		void clear(){
			if ( dv ){
				iterator itr = base::begin();
				while ( itr != base::end() ){
					_valueDeletor::doDelete(*itr);
					itr++;
				}
			}
			base::clear();
		}

		void remove(int32_t i){
			iterator itr=base::begin();
			itr+=i;
			_kt key = *itr;
			base::erase( itr );
			if ( dv ) 
				_valueDeletor::doDelete(key);
		}

	};

//growable arrays of Objects (like a collection or list)
//a list, so can contain duplicates
//it grows in chunks... todo: check jlucene for initial size of array, and growfactors
template<typename _kt, typename _valueDeletor=CL_NS(util)::Deletor::Dummy> 
class CLVector:public __CLList<_kt, CL_NS_STD(vector)<_kt> , _valueDeletor>
{
public:
	CLVector ( const bool deleteValue=true ):
	  __CLList<_kt, CL_NS_STD(vector)<_kt> , _valueDeletor>(deleteValue)
	{
	}
};

//An array-backed implementation of the List interface
//a list, so can contain duplicates
//*** a very simple list - use <valarray>
//(This class is roughly equivalent to Vector, except that it is unsynchronized.)
#define CLArrayList CLVector 
#define CLHashSet CLHashList

//implementation of the List interface, provides access to the first and last list elements in O(1) 
//no comparator is required... and so can contain duplicates
//a simple list with no comparator
//*** a very simple list - use <list>
#ifdef LUCENE_DISABLE_HASHING
   #define CLHashList CLSetList 
#else

template<typename _kt,
	typename _Comparator=CL_NS(util)::Compare::TChar,
	typename _valueDeletor=CL_NS(util)::Deletor::Dummy> 
class CLHashList:public __CLList<_kt, CL_NS_HASHING(hash_set)<_kt,_Comparator> , _valueDeletor>
{
public:
	CLHashList ( const bool deleteValue=true ):
	  __CLList<_kt, CL_NS_HASHING(hash_set)<_kt,_Comparator> , _valueDeletor>(deleteValue)
	{
	}
};
#endif

template<typename _kt, typename _valueDeletor=CL_NS(util)::Deletor::Dummy> 
class CLLinkedList:public __CLList<_kt, CL_NS_STD(list)<_kt> , _valueDeletor>
{
public:
	CLLinkedList ( const bool deleteValue=true ):
	  __CLList<_kt, CL_NS_STD(list)<_kt> , _valueDeletor>(deleteValue)
	{
	}
};
template<typename _kt,
	typename _Comparator=CL_NS(util)::Compare::TChar,
	typename _valueDeletor=CL_NS(util)::Deletor::Dummy> 
class CLSetList:public __CLList<_kt, CL_NS_STD(set)<_kt,_Comparator> , _valueDeletor>
{
public:
	CLSetList ( const bool deleteValue=true ):
	  __CLList<_kt, CL_NS_STD(set)<_kt,_Comparator> , _valueDeletor>(deleteValue)
	{
	}
};

CL_NS_END
#endif
