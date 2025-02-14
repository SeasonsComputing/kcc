#ifndef _lucene_search_BooleanQuery_
#define _lucene_search_BooleanQuery_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#include "ConjunctionScorer.h"
#include "CLucene/index/IndexReader.h"
#include "CLucene/util/StringBuffer.h"
#include "SearchHeader.h"
#include "BooleanClause.h"
#include "BooleanScorer.h"
#include "Scorer.h"

CL_NS_DEF(search)

  typedef CL_NS(util)::CLVector<BooleanClause*,CL_NS(util)::Deletor::Object<BooleanClause> > _clauses;
	class BooleanQuery;//predefine
	
    // A Query that matches documents matching boolean combinations of other
    // queries, typically {@link TermQuery}s or {@link PhraseQuery}s.
	class BooleanQuery:public Query {
	private:
		_clauses clauses;

		class BooleanWeight: public Weight {
		private:
			Searcher* searcher;
			CL_NS(util)::CLVector<Weight*,CL_NS(util)::Deletor::Object<Weight> > weights;
			_clauses* clauses;
			BooleanQuery* parentQuery;
		public:
			BooleanWeight(Searcher* searcher, 
				CL_NS(util)::CLVector<BooleanClause*,CL_NS(util)::Deletor::Object<BooleanClause> >* clauses, 
				BooleanQuery* parentQuery);
			~BooleanWeight();
			Query* getQuery();
			float_t getValue();
			float_t sumOfSquaredWeights();
			void normalize(float_t norm);
			Scorer* scorer(CL_NS(index)::IndexReader* reader);
			Explanation* explain(CL_NS(index)::IndexReader* reader, int32_t doc);
		};//booleanweight

    protected:
      Weight* createWeight(Searcher* searcher) {
         return _CLNEW BooleanWeight(searcher,&clauses,this);
      }
      BooleanQuery(const BooleanQuery& clone);
	public:
    /** Constructs an empty boolean query. */
        BooleanQuery();
        
        ~BooleanQuery();
        
        const TCHAR* getQueryName() const;
				static const TCHAR* getClassName();
        
         /** Return the maximum number of clauses permitted, 1024 by default.
            * Attempts to add more than the permitted number of clauses cause {@link
            * TooManyClauses} to be thrown.*/
         static int32_t getMaxClauseCount();

         /** Set the maximum number of clauses permitted. */
         static void setMaxClauseCount(int32_t maxClauseCount);

       /** Adds a clause to a boolean query.  Clauses may be:
		* <ul>
		* <li><code>required</code> which means that documents which <i>do not</i>
		* match this sub-query will <i>not</i> match the boolean query;
		* <li><code>prohibited</code> which means that documents which <i>do</i>
		* match this sub-query will <i>not</i> match the boolean query; or
		* <li>neither, in which case matched documents are neither prohibited from
		* nor required to match the sub-query. However, a document must match at
		* least 1 sub-query to match the boolean query.
		* </ul>
		* It is an error to specify a clause as both <code>required</code> and
		* <code>prohibited</code>.
		*
		* @see #getMaxClauseCount()
		*/
		void add(Query* query, const bool required, const bool prohibited){
			add(query,false,required,prohibited);
		}
		void add(Query* query, const bool deleteQuery, const bool required, const bool prohibited);
		
		
		
		/** Returns the set of clauses in this query. */
		BooleanClause** getClauses();

		/** Adds a clause to a boolean query.
		* @see #getMaxClauseCount()
		*/
		void add(BooleanClause* clause);
	    
	    /* DSR:PROPOSED: Give client code access to clauses.size() so we know how
	    ** large the array returned by getClauses is. 
	    //todo: Not necessary, count the size of the clauses is*/
	    int32_t getClauseCount();


		Query* rewrite(CL_NS(index)::IndexReader* reader);
		Query* clone();
		bool equals(Query* o) const;
			
	  	/** Prints a user-readable version of this query. */
		TCHAR* toString(const TCHAR* field);
		/** Returns a hash code value for this object.*/
		size_t hashCode() const;
    };

CL_NS_END
#endif
