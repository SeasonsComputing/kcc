#ifndef _lucene_search_Sort_
#define _lucene_search_Sort_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#include "CLucene/index/IndexReader.h"
#include "ScoreDoc.h"

CL_NS_DEF(search)


 class SortField; //predefine
 class Sort;

/**
 * Expert: Compares two ScoreDoc objects for sorting.
 *
 * <p>Created: Feb 3, 2004 9:00:16 AM 
 * 
 * @author  Tim Jones (Nacimiento Software)
 * @since   lucene 1.4
 * @version $Id: Sort.h 15199 2007-03-09 17:57:17Z tvk $
 */
 class ScoreDocComparator:LUCENE_BASE {
 public:
	 virtual ~ScoreDocComparator();
//    CL_NS(util)::Comparable** cachedValues;
//    ScoreDocComparator(CL_NS(util)::Comparable** cachedValues);

	/**
	 * Compares two ScoreDoc objects and returns a result indicating their
	 * sort order.
	 * @param i First ScoreDoc
	 * @param j Second ScoreDoc
	 * @return <code>-1</code> if <code>i</code> should come before <code>j</code><br><code>1</code> if <code>i</code> should come after <code>j</code><br><code>0</code> if they are equal
	 * @see java.util.Comparator
	 */
    virtual int32_t compare (ScoreDoc* i, ScoreDoc* j) = 0;

	/**
	 * Returns the value used to sort the given document.  The
	 * object returned must implement the java.io.Serializable
	 * interface.  This is used by multisearchers to determine how to collate results from their searchers.
	 * @see FieldDoc
	 * @param i Document
	 * @return Serializable object
	 */
    virtual CL_NS(util)::Comparable* sortValue (ScoreDoc* i) = 0;

	
	/**
	 * Returns the type of sort.  Should return <code>SortField.SCORE</code>, <code>SortField.DOC</code>, <code>SortField.STRING</code>, <code>SortField.INTEGER</code>, 
	 * <code>SortField::FLOAT</code> or <code>SortField.CUSTOM</code>.  It is not valid to return <code>SortField.AUTO</code>.
	 * This is used by multisearchers to determine how to collate results from their searchers.
	 * @return One of the constants in SortField.
	 * @see SortField
	 */
    virtual int32_t sortType() = 0;

	/** Special comparator for sorting hits according to computed relevance (document score). */
	static ScoreDocComparator* RELEVANCE;

	/** Special comparator for sorting hits according to index order (document number). */
	static ScoreDocComparator* INDEXORDER;
 };

/**
 * Expert: returns a comparator for sorting ScoreDocs.
 *
 * <p>Created: Apr 21, 2004 3:49:28 PM
 * 
 * @author  Tim Jones
 * @version $Id: Sort.h 15199 2007-03-09 17:57:17Z tvk $
 * @since   1.4
 */
class SortComparatorSource:LUCENE_BASE {
public:
   virtual ~SortComparatorSource(){
   }

   //return a reference to a string describing the name of the comparator
   //this is used in the explanation
   virtual TCHAR* getName() = 0;

   virtual size_t hashCode() = 0;

  /**
   * Creates a comparator for the field in the given index.
   * @param reader Index to create comparator for.
   * @param fieldname  Field to create comparator for.
   * @return Comparator of ScoreDoc objects.
   * @throws IOException If an error occurs reading the index.
   */
   virtual ScoreDocComparator* newComparator (CL_NS(index)::IndexReader* reader, const TCHAR* fieldname) = 0;
};


/**
 * Abstract base class for sorting hits returned by a Query.
 *
 * <p>This class should only be used if the other SortField
 * types (SCORE, DOC, STRING, INT, FLOAT) do not provide an
 * adequate sorting.  It maintains an internal cache of values which
 * could be quite large.  The cache is an array of Comparable,
 * one for each document in the index.  There is a distinct
 * Comparable for each unique term in the field - if
 * some documents have the same term in the field, the cache
 * array will have entries which reference the same Comparable.
 *
 * <p>Created: Apr 21, 2004 5:08:38 PM
 *
 * @author  Tim Jones
 * @version $Id: Sort.h 15199 2007-03-09 17:57:17Z tvk $
 * @since   1.4
 */
class SortComparator: public SortComparatorSource {
	ScoreDocComparator* _this;
public:
	ScoreDocComparator* newComparator (CL_NS(index)::IndexReader* reader, TCHAR* fieldname);
  
	SortComparator(ScoreDocComparator* _this):SortComparatorSource(){
		this->_this = _this;
	}
	virtual ~SortComparator(){
	}


//protected:todo: jlucene uses protected, but then FieldCacheImpl::getCustom can't access it... might be changed in jlucene 2.0
  /**
   * Returns an object which, when sorted according to natural order,
   * will order the Term values in the correct order.
   * <p>For example, if the Terms contained integer values, this method
   * would return <code>new Integer(termtext)</code>.  Note that this
   * might not always be the most efficient implementation - for this
   * particular example, a better implementation might be to make a
   * ScoreDocLookupComparator that uses an internal lookup table of int.
   * @param termtext The textual value of the term.
   * @return An object representing <code>termtext</code> that sorts 
   * according to the natural order of <code>termtext</code>.
   * @see Comparable
   * @see ScoreDocComparator
   */
   virtual CL_NS(util)::Comparable* getComparable (const TCHAR* termtext) = 0; //todo:

};


/**
 * Stores information about how to sort documents by terms in an individual
 * field.  Fields must be indexed in order to sort by them.
 *
 * <p>Created: Feb 11, 2004 1:25:29 PM
 *
 * @author  Tim Jones (Nacimiento Software)
 * @since   lucene 1.4
 * @version $Id: Sort.h 15199 2007-03-09 17:57:17Z tvk $
 * @see Sort
 */
class SortField:LUCENE_BASE {
public:
   virtual ~SortField();

  /** Sort by document score (relevancy).  Sort values are Float and higher
   * values are at the front. */
   LUCENE_STATIC_CONSTANT(int32_t, SCORE=0);
   
  /** Sort by document number (index order).  Sort values are Integer and lower
   * values are at the front. */
   LUCENE_STATIC_CONSTANT(int32_t, DOC=1);

  /** Guess type of sort based on field contents.  A regular expression is used
   * to look at the first term indexed for the field and determine if it
   * represents an integer number, a floating point number, or just arbitrary
   * string characters. */
   LUCENE_STATIC_CONSTANT(int32_t, AUTO=2);

  /** Sort using term values as Strings.  Sort values are String and lower
   * values are at the front. */
   LUCENE_STATIC_CONSTANT(int32_t, STRING=3);

  /** Sort using term values as encoded Integers.  Sort values are Integer and
   * lower values are at the front. */
   LUCENE_STATIC_CONSTANT(int32_t, INT=4);

  /** Sort using term values as encoded Floats.  Sort values are Float and
   * lower values are at the front. */
   LUCENE_STATIC_CONSTANT(int32_t, FLOAT=5);

  /** Sort using a custom Comparator.  Sort values are any Comparable and
   * sorting is done according to natural order. */
   LUCENE_STATIC_CONSTANT(int32_t, CUSTOM=9);

  // IMPLEMENTATION NOTE: the FieldCache.STRING_INDEX is in the same "namespace"
  // as the above static int values.  Any new values must not have the same value
  // as FieldCache.STRING_INDEX.

  /** Represents sorting by document score (relevancy). */
  static SortField* FIELD_SCORE;

  /** Represents sorting by document number (index order). */
  static SortField* FIELD_DOC;


private:
  const TCHAR* field;
  int32_t type;  // defaults to determining type dynamically
  //Locale* locale;    // defaults to "natural order" (no Locale)
  bool reverse;  // defaults to natural order
  SortComparatorSource* factory;

  SortField (SortField& clone);
public:
  SortField (const TCHAR* field);
  //SortField (const TCHAR* field, bool reverse);
  //todo: we cannot make reverse use default field of =false.
  //because bool and int are the same type in c, overloading is not possible
  SortField (const TCHAR* field, int32_t type, bool reverse); 

  /*
   SortField (TCHAR* field, Locale* locale) {
   SortField (TCHAR* field, Locale* locale, bool reverse);*/

  SortField (const TCHAR* field, SortComparatorSource* comparator, bool reverse=false);

  /** Returns the name of the field.  Could return <code>null</code>
   * if the sort is by SCORE or DOC.
   * @return Name of field, possibly <code>null</code>.
   */
  const TCHAR* getField() {
    return field;
  }
  
  SortField* clone();

  /** Returns the type of contents in the field.
   * @return One of the constants SCORE, DOC, AUTO, STRING, INT or FLOAT.
   */
  int32_t getType() {
    return type;
  }

  /** Returns the Locale by which term values are interpreted.
   * May return <code>null</code> if no Locale was specified.
   * @return Locale, or <code>null</code>.
   */
  /*Locale getLocale() {
    return locale;
  }*/

  /** Returns whether the sort should be reversed.
   * @return  True if natural order should be reversed.
   */
  bool getReverse() {
    return reverse;
  }

  SortComparatorSource* getFactory() {
    return factory;
  }

  const TCHAR* toString() const {
	CL_NS(util)::StringBuffer buffer;
    switch (type) {
      case SCORE: buffer.append(_T("<score>"));
                  break;

      case DOC: buffer.append(_T("<doc>"));
                break;

      case CUSTOM: buffer.append (_T("<custom:\""));
         buffer.append( field );
         buffer.append( _T("\": "));
         buffer.append(factory->getName());
         buffer.append(_T(">"));
         break;

      default: buffer.append( _T("\""));
            buffer.append( field );
            buffer.append( _T("\"") );
               break;
    }

    //if (locale != null) buffer.append ("("+locale+")"); todo:
    if (reverse) buffer.appendChar('!');

    return buffer.toString();
  }
};



/**
 * Encapsulates sort criteria for returned hits.
 *
 * <p>The fields used to determine sort order must be carefully chosen.
 * Documents must contain a single term in such a field,
 * and the value of the term should indicate the document's relative position in
 * a given sort order.  The field must be indexed, but should not be tokenized,
 * and does not need to be stored (unless you happen to want it back with the
 * rest of your document data).  In other words:
 *
 * <dl><dd><code>document.add (new Field ("byNumber", Integer.toString(x), false, true, false));</code>
 * </dd></dl>
 *
 * <p><h3>Valid Types of Values</h3>
 *
 * <p>There are three possible kinds of term values which may be put into
 * sorting fields: Integers, Floats, or Strings.  Unless
 * {@link SortField SortField} objects are specified, the type of value
 * in the field is determined by parsing the first term in the field.
 *
 * <p>Integer term values should contain only digits and an optional
 * preceeding negative sign.  Values must be base 10 and in the range
 * <code>Integer.MIN_VALUE</code> and <code>Integer.MAX_VALUE</code> inclusive.
 * Documents which should appear first in the sort
 * should have low value integers, later documents high values
 * (i.e. the documents should be numbered <code>1..n</code> where
 * <code>1</code> is the first and <code>n</code> the last).
 *
 * <p>Float term values should conform to values accepted by
 * {@link Float Float.valueOf(String)} (except that <code>NaN</code>
 * and <code>Infinity</code> are not supported).
 * Documents which should appear first in the sort
 * should have low values, later documents high values.
 *
 * <p>String term values can contain any valid String, but should
 * not be tokenized.  The values are sorted according to their
 * {@link Comparable natural order}.  Note that using this type
 * of term value has higher memory requirements than the other
 * two types.
 *
 * <p><h3>Object Reuse</h3>
 *
 * <p>One of these objects can be
 * used multiple times and the sort order changed between usages.
 *
 * <p>This class is thread safe.
 *
 * <p><h3>Memory Usage</h3>
 *
 * <p>Sorting uses of caches of term values maintained by the
 * internal HitQueue(s).  The cache is static and contains an integer
 * or float array of length <code>IndexReader.maxDoc()</code> for each field
 * name for which a sort is performed.  In other words, the size of the
 * cache in bytes is:
 *
 * <p><code>4 * IndexReader.maxDoc() * (# of different fields actually used to sort)</code>
 *
 * <p>For String fields, the cache is larger: in addition to the
 * above array, the value of every term in the field is kept in memory.
 * If there are many unique terms in the field, this could
 * be quite large.
 *
 * <p>Note that the size of the cache is not affected by how many
 * fields are in the index and <i>might</i> be used to sort - only by
 * the ones actually used to sort a result set.
 *
 * <p>The cache is cleared each time a new <code>IndexReader</code> is
 * passed in, or if the value returned by <code>maxDoc()</code>
 * changes for the current IndexReader.  This class is not set up to
 * be able to efficiently sort hits from more than one index
 * simultaneously.
 *
 * <p>Created: Feb 12, 2004 10:53:57 AM
 *
 * @author  Tim Jones (Nacimiento Software)
 * @since   lucene 1.4
 * @version $Id: Sort.h 15199 2007-03-09 17:57:17Z tvk $
 */
class Sort:LUCENE_BASE {
	// internal representation of the sort criteria
	SortField** fields;
	void clear();
public:
	~Sort();

	/** Represents sorting by computed relevance. Using this sort criteria
	 * returns the same results as calling {@link Searcher#search(Query) Searcher#search()}
	 * without a sort criteria, only with slightly more overhead. */
	static Sort* RELEVANCE;

	/** Represents sorting by index order. */
	static Sort* INDEXORDER;

	Sort();
	Sort (const TCHAR* field, bool reverse=false);
	Sort (const TCHAR** fields);
	Sort (SortField* field);
	Sort (SortField** fields);
	void setSort (const TCHAR* field, bool reverse=false);
	void setSort (const TCHAR** fieldnames);
	void setSort (SortField* field);
	void setSort (SortField** fields);

   const TCHAR* toString() const;

   //todo: check jlucene way
   SortField** getFields() const{
	   return fields;
   }
};



 

CL_NS_END
#endif
