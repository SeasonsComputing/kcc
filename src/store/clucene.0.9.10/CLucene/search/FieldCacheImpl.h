#ifndef _lucene_search_FieldCacheImpl_
#define _lucene_search_FieldCacheImpl_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#include "CLucene/index/IndexReader.h"
#include "FieldCache.h"
#include "Sort.h"


CL_NS_DEF(search)


/**
 * Expert: The default cache implementation, storing all values in memory.
 * A WeakHashMap is used for storage.
 *
 * <p>Created: May 19, 2004 4:40:36 PM
 *
 * @author  Tim Jones (Nacimiento Software)
 * @since   lucene 1.4
 * @version $Id: FieldCacheImpl.h 15199 2007-03-09 17:57:17Z tvk $
 */
class FieldCacheImpl: public FieldCache {
public:
	/** Expert: Every key in the internal cache is of this type. */
	class FileEntry:LUCENE_BASE {
		const TCHAR* field;        // which Field
		int32_t type;            // which SortField type
		SortComparatorSource* custom;       // which custom comparator
	public:
		/** Creates one of these objects. */
		FileEntry (const TCHAR* field, int32_t type);
	   
		/** Creates one of these objects for a custom comparator. */
		FileEntry (const TCHAR* field, SortComparatorSource* custom);
		~FileEntry();

		int32_t getType(){ return type; }
	   
		/** Two of these are equal iff they reference the same field and type. */
		bool equals (FileEntry* other);
	   
		/** Composes a hashcode based on the field and type. */
		size_t hashCode();

		int32_t compareTo(const FileEntry* other) const;

        class Compare:LUCENE_BASE, public CL_NS(util)::Compare::_base //<Term*>
		{
		public:
			bool operator()( FileEntry* f1, FileEntry* f2 ) const{
				return ( f1->compareTo(f2) < 0 );
			}
			size_t operator()( FileEntry* t ) const{
				return t->hashCode();
			}
		};
	}; 

    FieldCacheImpl();
    ~FieldCacheImpl();
private:
	
	//internal:
	///the type that is stored in the field cache. can't use a typedef because
	///the decorated name would become too long
	class fieldcacheCacheReaderType: public CL_NS(util)::CLHashMap<FileEntry*,
		FieldCacheAuto*, 
        FileEntry::Compare,
		CL_NS(util)::Deletor::Object<FileEntry>, 
		CL_NS(util)::Deletor::Object<FieldCacheAuto> >
	{

	public:
		fieldcacheCacheReaderType();
		~fieldcacheCacheReaderType();
	};

	//note: typename gets too long if using cacheReaderType as a typename
	typedef CL_NS(util)::CLHashMap<CL_NS(index)::IndexReader*, 
		fieldcacheCacheReaderType*, 
		CL_NS(util)::Compare::Void<CL_NS(index)::IndexReader>,
		CL_NS(util)::Deletor::Object<CL_NS(index)::IndexReader>, 
		CL_NS(util)::Deletor::Object<fieldcacheCacheReaderType> > fieldcacheCacheType; 

  /** The internal cache. Maps FileEntry to array of interpreted term values. **/
  fieldcacheCacheType cache;
  
  DEFINE_MUTEX(_this);


  /** See if an object is in the cache. */
  FieldCacheAuto* lookup (CL_NS(index)::IndexReader* reader, const TCHAR* field, int32_t type) ;
 
  /** See if a custom object is in the cache. */
  FieldCacheAuto* lookup (CL_NS(index)::IndexReader* reader, const TCHAR* field, SortComparatorSource* comparer);

  /** Put an object into the cache. */
  void store (CL_NS(index)::IndexReader* reader, const TCHAR* field, int32_t type, FieldCacheAuto* value);

  /** Put a custom object into the cache. */
  void store (CL_NS(index)::IndexReader* reader, const TCHAR* field, SortComparatorSource* comparer, FieldCacheAuto* value);
  
public:

  // inherit javadocs
  FieldCacheAuto* getInts (CL_NS(index)::IndexReader* reader, const TCHAR* field);

  // inherit javadocs
  FieldCacheAuto* getFloats (CL_NS(index)::IndexReader* reader, const TCHAR* field);

  // inherit javadocs
  FieldCacheAuto* getStrings (CL_NS(index)::IndexReader* reader, const TCHAR* field);

  // inherit javadocs
  FieldCacheAuto* getStringIndex (CL_NS(index)::IndexReader* reader, const TCHAR* field);

  // inherit javadocs
  FieldCacheAuto* getAuto (CL_NS(index)::IndexReader* reader, const TCHAR* field);

  // inherit javadocs
  FieldCacheAuto* getCustom (CL_NS(index)::IndexReader* reader, const TCHAR* field, SortComparator* comparator);

};


CL_NS_END
#endif
