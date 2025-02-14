#ifndef _lucene_search_FieldCache_
#define _lucene_search_FieldCache_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#include "CLucene/index/IndexReader.h"
#include "Sort.h"


CL_NS_DEF(search)

class FieldCacheAuto; //predefine

/**
 * Expert: Maintains caches of term values.
 *
 * <p>Created: May 19, 2004 11:13:14 AM
 *
 * @author  Tim Jones (Nacimiento Software)
 * @since   lucene 1.4
 * @version $Id: FieldCache.h 15199 2007-03-09 17:57:17Z tvk $
 */
class FieldCache :LUCENE_BASE {
public:
   virtual ~FieldCache(){
   }

	/** Expert: Stores term text values and document ordering data. */
	class StringIndex:LUCENE_BASE {
	public:
		/** All the term values, in natural order. */
		TCHAR** lookup;

		/** For each document, an index into the lookup array. */
		int32_t* order;

        int count;

		/** Creates one of these objects 
            Consumes all memory given.
        */
		StringIndex (int32_t* values, TCHAR** lookup, int count) {
            this->count = count;
			this->order = values;
			this->lookup = lookup;
		}

        ~StringIndex(){
            _CLDELETE_ARRAY(order);
            
            for ( int i=0;i<count;i++ )
                _CLDELETE_CARRAY(lookup[i]);
            _CLDELETE_ARRAY(lookup);
        }
	};


  /** Indicator for FieldCache::StringIndex values in the cache.
  NOTE: the value assigned to this constant must not be
        the same as any of those in SortField!!
  */
  static int32_t STRING_INDEX;

  /** Expert: The cache used internally by sorting and range query classes. */
  static FieldCache* DEFAULT;

  /** Checks the internal cache for an appropriate entry, and if none is
   * found, reads the terms in <code>field</code> as integers and returns an array
   * of size <code>reader.maxDoc()</code> of the value each document
   * has in the given field.
   * @param reader  Used to get field values.
   * @param field   Which field contains the integers.
   * @return The values in the given field for each document.
   * @throws IOException  If any error occurs.
   */
  virtual FieldCacheAuto* getInts (CL_NS(index)::IndexReader* reader, const TCHAR* field) = 0;

  /** Checks the internal cache for an appropriate entry, and if
   * none is found, reads the terms in <code>field</code> as floats and returns an array
   * of size <code>reader.maxDoc()</code> of the value each document
   * has in the given field.
   * @param reader  Used to get field values.
   * @param field   Which field contains the floats.
   * @return The values in the given field for each document.
   * @throws IOException  If any error occurs.
   */
  virtual FieldCacheAuto* getFloats (CL_NS(index)::IndexReader* reader, const TCHAR* field) = 0;

  /** Checks the internal cache for an appropriate entry, and if none
   * is found, reads the term values in <code>field</code> and returns an array
   * of size <code>reader.maxDoc()</code> containing the value each document
   * has in the given field.
   * @param reader  Used to get field values.
   * @param field   Which field contains the strings.
   * @return The values in the given field for each document.
   * @throws IOException  If any error occurs.
   */
  virtual FieldCacheAuto* getStrings (CL_NS(index)::IndexReader* reader, const TCHAR* field) = 0;

  /** Checks the internal cache for an appropriate entry, and if none
   * is found reads the term values in <code>field</code> and returns
   * an array of them in natural order, along with an array telling
   * which element in the term array each document uses.
   * @param reader  Used to get field values.
   * @param field   Which field contains the strings.
   * @return Array of terms and index into the array for each document.
   * @throws IOException  If any error occurs.
   */
   virtual FieldCacheAuto* getStringIndex (CL_NS(index)::IndexReader* reader, const TCHAR* field) = 0;

  /** Checks the internal cache for an appropriate entry, and if
   * none is found reads <code>field</code> to see if it contains integers, floats
   * or strings, and then calls one of the other methods in this class to get the
   * values.  For string values, a FieldCache::StringIndex is returned.  After
   * calling this method, there is an entry in the cache for both
   * type <code>AUTO</code> and the actual found type.
   * @param reader  Used to get field values.
   * @param field   Which field contains the values.
   * @return int32_t[], float_t[] or FieldCache::StringIndex.
   * @throws IOException  If any error occurs.
   */
   virtual FieldCacheAuto* getAuto (CL_NS(index)::IndexReader* reader, const TCHAR* field) = 0;

  /** Checks the internal cache for an appropriate entry, and if none
   * is found reads the terms out of <code>field</code> and calls the given SortComparator
   * to get the sort values.  A hit in the cache will happen if <code>reader</code>,
   * <code>field</code>, and <code>comparator</code> are the same (using <code>equals()</code>)
   * as a previous call to this method.
   * @param reader  Used to get field values.
   * @param field   Which field contains the values.
   * @param comparator Used to convert terms into something to sort by.
   * @return Array of sort objects, one for each document.
   * @throws IOException  If any error occurs.
   */
   virtual FieldCacheAuto* getCustom (CL_NS(index)::IndexReader* reader, const TCHAR* field, SortComparator* comparator) = 0;
};

/** A class holding an AUTO field. In java lucene an Object
	is used, but we use this.
	contentType:
	1 - integer array
	2 - float array
	3 - FieldCache::StringIndex object
	This class is also used when returning getInt, getFloat, etc
	because we have no way of returning the size of the array and this
	class can be used to determine the array size
*/	
class FieldCacheAuto:LUCENE_BASE{
public:
	enum{
		INT_ARRAY=1,
		FLOAT_ARRAY=2,
		STRING_INDEX=3,
		STRING_ARRAY=4,
		COMPARABLE_ARRAY=5,
		SORT_COMPARATOR=6,
		SCOREDOC_COMPARATOR=7
	};

	FieldCacheAuto(int32_t len, int32_t type);
	~FieldCacheAuto();
	///if contents should be deleted too, depending on type
	bool ownContents;
	int32_t contentLen; //number of items in the list
	uint8_t contentType;
	int32_t* intArray; //item 1
	float_t* floatArray; //item 2
	FieldCache::StringIndex* stringIndex; //item 3
	TCHAR** stringArray; //item 4
	CL_NS(util)::Comparable** comparableArray; //item 5
	SortComparator* sortComparator; //item 6
	ScoreDocComparator* scoreDocComparator; //item 7

};


CL_NS_END

#endif
