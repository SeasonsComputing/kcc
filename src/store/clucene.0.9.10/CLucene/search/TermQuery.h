#ifndef _lucene_search_TermQuery_
#define _lucene_search_TermQuery_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#include "SearchHeader.h"
#include "Scorer.h"
#include "CLucene/index/Term.h"
#include "TermScorer.h"
#include "CLucene/index/IndexReader.h"
#include "CLucene/util/StringBuffer.h"
#include "CLucene/index/Terms.h"

CL_NS_DEF(search)


    /** A Query that matches documents containing a term.
	This may be combined with other terms with a {@link BooleanQuery}.
	*/
    class TermQuery: public Query {
    private:
		CL_NS(index)::Term* term;

		
		class TermWeight: public Weight {
		private:
			Searcher* searcher;
			float_t value;
			float_t idf;
			float_t queryNorm;
			float_t queryWeight;
			TermQuery* _this;
			CL_NS(index)::Term* _term;

		public:
			TermWeight(Searcher* searcher, TermQuery* _this, CL_NS(index)::Term* _term);
			~TermWeight();
			TCHAR* toString();
			Query* getQuery() { return (Query*)_this; }
			float_t getValue() { return value; }

			float_t sumOfSquaredWeights();
			void normalize(float_t queryNorm);
			Scorer* scorer(CL_NS(index)::IndexReader* reader);
			Explanation* explain(CL_NS(index)::IndexReader* reader, int32_t doc);
		};

    protected:
        Weight* createWeight(Searcher* searcher);
        TermQuery(const TermQuery& clone);
	public:
		// Constructs a query for the term <code>t</code>. 
		TermQuery(CL_NS(index)::Term* t);
		~TermQuery();

		static const TCHAR* getClassName();
		const TCHAR* getQueryName() const;
	    
		//added by search highlighter
		CL_NS(index)::Term* getTerm();
	    
		// Prints a user-readable version of this query. 
		TCHAR* toString(const TCHAR* field);

		bool equals(Query* other) const;
		Query* clone();

		/** Returns a hash code value for this object.*/
		size_t hashCode() const;
    };
CL_NS_END
#endif

