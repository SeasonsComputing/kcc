#ifndef _lucene_search_MultiTermQuery_
#define _lucene_search_MultiTermQuery_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#include "CLucene/util/StringBuffer.h"
#include "CLucene/index/IndexReader.h"
#include "CLucene/index/Term.h"
#include "CLucene/index/Terms.h"
#include "FilteredTermEnum.h"
#include "SearchHeader.h"
#include "BooleanQuery.h"
#include "TermQuery.h"

CL_NS_DEF(search)
    /**
     * A {@link Query} that matches documents containing a subset of terms provided
     * by a {@link FilteredTermEnum} enumeration.
     * <P>
     * <code>MultiTermQuery</code> is not designed to be used by itself.
     * <BR>
     * The reason being that it is not intialized with a {@link FilteredTermEnum}
     * enumeration. A {@link FilteredTermEnum} enumeration needs to be provided.
     * <P>
     * For example, {@link WildcardQuery} and {@link FuzzyQuery} extend
     * <code>MultiTermQuery</code> to provide {@link WildcardTermEnum} and
     * {@link FuzzyTermEnum}, respectively.
     */
    class MultiTermQuery: public Query {
    private:
        CL_NS(index)::Term* term;
    protected:
        MultiTermQuery(const MultiTermQuery& clone);

		/** Construct the enumeration to be used, expanding the pattern term. */
		virtual FilteredTermEnum* getEnum(CL_NS(index)::IndexReader* reader) = 0;
    public:
        /** Constructs a query for terms matching <code>term</code>. */
        MultiTermQuery(CL_NS(index)::Term* t);

        /* DSR:CL_BUG_LEAK: This destructor needed to be virtual so it would
        ** get called for instances of derived classes. */
        virtual ~MultiTermQuery();

		/** Returns the pattern term. */
		//todo: should return reference???
		CL_NS(index)::Term* getTerm() const { return term; }

		Query* combine(Query** queries);
				
				
        //marked public by search highlighter
        //todo: removed, but may be needed for highlighter: BooleanQuery* getQuery();


        /** Prints a user-readable version of this query. */
        TCHAR* toString(const TCHAR* field);

		Query* rewrite(CL_NS(index)::IndexReader* reader);
    };
CL_NS_END
#endif
