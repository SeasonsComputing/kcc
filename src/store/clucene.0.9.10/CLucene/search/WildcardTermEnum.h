#ifndef _lucene_search_WildcardTermEnum_
#define _lucene_search_WildcardTermEnum_
#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#ifndef NO_WILDCARD_QUERY

#include "CLucene/index/IndexReader.h"
#include "CLucene/index/Term.h"
#include "CLucene/index/Terms.h"
#include "FilteredTermEnum.h"

CL_NS_DEF(search)
    /**
     * Subclass of FilteredTermEnum for enumerating all terms that match the
     * specified wildcard filter term->
     * <p>
     * Term enumerations are always ordered by term->compareTo().  Each term in
     * the enumeration is greater than all that precede it.
     */
	class WildcardTermEnum: public FilteredTermEnum {
    private:
        CL_NS(index)::Term* __term;
        TCHAR* pre;
        int32_t preLen;
        bool fieldMatch;
        bool _endEnum;

        /********************************************
        * const TCHAR* equality with support for wildcards
        ********************************************/

        protected:
        bool termCompare(CL_NS(index)::Term* term) ;

        public:

        /**
		* Creates a new <code>WildcardTermEnum</code>.  Passing in a
		* {@link Term Term} that does not contain a
		* <code>WILDCARD_CHAR</code> will cause an exception to be thrown.
		*/
        WildcardTermEnum(CL_NS(index)::IndexReader* reader, CL_NS(index)::Term* term);
        ~WildcardTermEnum();

        float_t difference() ;

        bool endEnum() ;

        /**
         * Determines if a word matches a wildcard pattern.
         * <small>Work released by Granta Design Ltd after originally being done on
         * company time.</small>
         */
        static bool wildcardEquals(const TCHAR* pattern, int32_t patternIdx, const TCHAR* str, int32_t stringIdx);

        void close();
    };
CL_NS_END
#endif
#endif
