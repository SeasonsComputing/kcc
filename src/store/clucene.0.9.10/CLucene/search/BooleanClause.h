#ifndef _lucene_search_BooleanClause_
#define _lucene_search_BooleanClause_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif
#include "SearchHeader.h"

CL_NS_DEF(search)
	// A clause in a BooleanQuery. 
	class BooleanClause:LUCENE_BASE {
	public:
		class Compare:public CL_NS_STD(binary_function)<const BooleanClause*,const BooleanClause*,bool>
		{
		public:
			bool operator()( const BooleanClause* val1, const BooleanClause* val2 ){
				return val1->equals(val2);
			}
		};


		bool deleteQuery;

		// The query whose matching documents are combined by the boolean query. 
		Query* query;
	   
        int32_t getClauseCount();

		// If true, documents documents which <i>do not</i>
		//	match this sub-query will <i>not</i> match the boolean query. 
		bool required;
	      
		// If true, documents documents which <i>do</i>
		//	match this sub-query will <i>not</i> match the boolean query. 
		bool prohibited;
	      
		// Constructs a BooleanClause with query <code>q</code>, required
		//	<code>r</code> and prohibited <code>p</code>.  
		BooleanClause(Query* q, const bool DeleteQuery,const bool req, const bool p):
			query(q),
			required(req),
			prohibited(p),
			deleteQuery(DeleteQuery)
		{
		}

		BooleanClause(const BooleanClause& clone):
			query(clone.query->clone()),
			required(clone.required),
			prohibited(clone.prohibited),
			deleteQuery(true)
		{
		}

		BooleanClause* clone(){
			BooleanClause* ret = _CLNEW BooleanClause(*this);
			return ret;
		}

		~BooleanClause(){
			if ( deleteQuery )
				_CLDELETE( query );
		}

		/** Returns true iff <code>o</code> is equal to this. */
		bool equals(const BooleanClause* other) const {
			return this->query->equals(other->query) //todo: changed to equals, but check if this is more efficient
				&& (this->required == other->required)
				&& (this->prohibited == other->prohibited);
		}

		size_t hashCode(){
			return query->hashCode() ^ (this->required?1:0) ^ (this->prohibited?2:0);
		}
	};

	
CL_NS_END
#endif

