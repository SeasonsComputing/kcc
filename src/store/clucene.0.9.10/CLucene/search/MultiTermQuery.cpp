#include "CLucene/StdHeader.h"
#include "MultiTermQuery.h"

CL_NS_USE(index)
CL_NS_USE(util)
CL_NS_DEF(search)

/** Constructs a query for terms matching <code>term</code>. */

  MultiTermQuery::MultiTermQuery(Term* t){
  //Func - Constructor
  //Pre  - t != NULL
  //Post - The instance has been created

      CND_PRECONDITION(t != NULL, "t is NULL");

      term  = _CL_POINTER(t);

  }
  MultiTermQuery::MultiTermQuery(const MultiTermQuery& clone){
    //todo: is boost getting correctly cloned?
	term = _CLNEW Term(clone.getTerm()->field(),clone.getTerm()->text(),false);
  }

  MultiTermQuery::~MultiTermQuery(){
  //Func - Destructor
  //Pre  - true
  //Post - The instance has been destroyed

      _CLDECDELETE(term);
  }

	Query* MultiTermQuery::rewrite(IndexReader* reader) {
		FilteredTermEnum* enumerator = getEnum(reader);
		BooleanQuery* query = _CLNEW BooleanQuery();
		try {
            do {
                Term* t = enumerator->term();
                if (t != NULL) {
                    TermQuery* tq = _CLNEW TermQuery(t);	// found a match
                    tq->setBoost(getBoost() * enumerator->difference()); // set the boost
                    query->add(tq,true, false, false);		// add to q
					_CLDECDELETE(t);
                }
            } while (enumerator->next());
        } _CLFINALLY ( enumerator->close(); _CLDELETE(enumerator) );

		//if we only added one clause and the clause is not prohibited then
		//we can just return the query
		if (query->getClauseCount() == 1) {                    // optimize 1-clause queries
			BooleanClause** clauses = query->getClauses();
            BooleanClause* c = clauses[0];
			_CLDELETE_ARRAY(clauses);

			if (!c->prohibited) {			  // just return clause
				c->deleteQuery=false;
				Query* ret = c->query;

				_CLDELETE(query);
				return ret;
            }
		}
		return query;
	}
	
	Query* MultiTermQuery::combine(Query** queries) {
		return Query::mergeBooleanQueries(queries);
    }

    /** Prints a user-readable version of this query. */
    TCHAR* MultiTermQuery::toString(const TCHAR* field){
        StringBuffer buffer;

        if ( _tcscmp(term->field(),field)!=0 ) {
            buffer.append(term->field());
            buffer.append( _T(":"));
        }
        buffer.append(term->text());
        if (getBoost() != 1.0f) {
            buffer.appendChar ( '^' );
            buffer.appendFloat( getBoost(),1);
        }
        return buffer.toString();
    }

CL_NS_END
