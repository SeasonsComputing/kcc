#ifndef _lucene_search_WildcardQuery_
#define _lucene_search_WildcardQuery_
#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#ifndef NO_WILDCARD_QUERY

#include "CLucene/index/IndexReader.h"
#include "CLucene/index/Term.h"
#include "MultiTermQuery.h"
#include "WildcardTermEnum.h"

CL_NS_DEF(search)

    /** Implements the wildcard search query. Supported wildcards are <code>*</code>, which
		 * matches any character sequence (including the empty one), and <code>?</code>,
		 * which matches any single character. Note this query can be slow, as it
		 * needs to iterate over all terms. In order to prevent extremely slow WildcardQueries,
		 * a Wildcard term must not start with one of the wildcards <code>*</code> or
		 * <code>?</code>.
		 * 
		 * @see WildcardTermEnum
		 */
    class WildcardQuery: public MultiTermQuery {
    protected:
    	FilteredTermEnum* getEnum(CL_NS(index)::IndexReader* reader);
		WildcardQuery(const WildcardQuery& clone);
    public:
		WildcardQuery(CL_NS(index)::Term* term);
		~WildcardQuery();
    
	    //Returns the string "WildcardQuery"
    	const TCHAR* getQueryName() const;
		static const TCHAR* getClassName();
		
		size_t hashCode() const;
		bool equals(Query* other) const;
		Query* clone();
    };

CL_NS_END
#endif
#endif
