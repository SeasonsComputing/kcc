#ifndef _lucene_document_DateField_
#define _lucene_document_DateField_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

CL_NS_DEF(document)

//here are some constants used throughout clucene
//make date strings long enough to last a millenium
#define DATEFIELD_DATE_MAX _ILONGLONG(31536000000000) //1000L*365*24*60*60*1000

#define DATEFIELD_DATE_LEN 9 ////Long.toString(DATEFIELD_DATE_MAX, Character.MAX_RADIX).length()

  /**
 * Provides support for converting dates to strings and vice-versa.
 * The strings are structured so that lexicographic sorting orders by date,
 * which makes them suitable for use as field values and search terms.
 * 
 * <P>
 * Note that you do not have to use this class, you can just save your
 * dates as strings if lexicographic sorting orders them by date. This is
 * the case for example for dates like <code>yyyy-mm-dd hh:mm:ss</code>
 * (of course you can leave out the delimiter characters to save some space).
 * The advantage with using such a format is that you can easily save dates
 * with the required granularity, e.g. leaving out seconds. This saves memory
 * when searching with a RangeQuery or PrefixQuery, as Lucene
 * expands these queries to a BooleanQuery with potentially very many terms. 
 * 
 * <P>
 * Note: dates before 1970 cannot be used, and therefore cannot be
 * indexed when using this class.
 */
  class DateField :LUCENE_BASE {
  private:
    static void initDS();
  public:
  	~DateField();

    /**
   * Converts a millisecond time to a string suitable for indexing.
   * @throws RuntimeException if the time specified in the
   * method argument is negative, that is, before 1970
   */
    static TCHAR* timeToString(const int64_t time);

    /** Converts a string-encoded date into a millisecond time. */
    static int64_t stringToTime(const TCHAR* s);

  };
CL_NS_END
#endif
