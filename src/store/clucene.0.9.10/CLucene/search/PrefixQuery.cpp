#include "CLucene/StdHeader.h"
#ifndef NO_PREFIX_QUERY
#include "PrefixQuery.h"
/*
- reader set to NULL in constructor
- delete query protected in destructor
- delete query in prepare


*/
CL_NS_USE(index)
CL_NS_DEF(search)

  PrefixQuery::PrefixQuery(Term* Prefix){
  //Func - Constructor.
  //       Constructs a query for terms starting with prefix
  //Pre  - Prefix != NULL 
  //Post - The instance has been created

      //Get a pointer to Prefix
      prefix = _CL_POINTER(Prefix);
  }

  PrefixQuery::PrefixQuery(const PrefixQuery& clone):Query(clone){
	prefix = _CL_POINTER(clone.prefix);
  }
  Query* PrefixQuery::clone(){
	  return _CLNEW PrefixQuery(*this);
  }

  PrefixQuery::~PrefixQuery(){
  //Func - Destructor
  //Pre  - true
  //Post - The instance has been destroyed.
    
      //Delete prefix by finalizing it
      _CLDECDELETE(prefix);
  }


	/** Returns a hash code value for this object.*/
	size_t PrefixQuery::hashCode() const {
		return Similarity::floatToByte(getBoost()) ^ prefix->hashCode();
	}

  const TCHAR* PrefixQuery::getQueryName()const{
  //Func - Returns the name "PrefixQuery" 
  //Pre  - true
  //Post - The string "PrefixQuery" has been returned

      return getClassName();
  }
  const TCHAR* PrefixQuery::getClassName(){
  //Func - Returns the name "PrefixQuery" 
  //Pre  - true
  //Post - The string "PrefixQuery" has been returned

      return _T("PrefixQuery");
  }

  bool PrefixQuery::equals(Query * other) const{
	  if (!(other->instanceOf(PrefixQuery::getClassName())))
            return false;

        PrefixQuery* rq = (PrefixQuery*)other;
		bool ret = (this->getBoost() == rq->getBoost())
			&& (this->prefix->equals(rq->prefix));

		return ret;
  }

   Query* PrefixQuery::rewrite(IndexReader* reader){
    BooleanQuery* query = _CLNEW BooleanQuery();
    TermEnum* enumerator = reader->terms(prefix);
	Term* lastTerm = NULL;
    try {
      const TCHAR* prefixText = prefix->text();
      const TCHAR* prefixField = prefix->field();
	  int32_t prefixLen = prefix->textLength();
      do {
        lastTerm = enumerator->term();
		if (lastTerm != NULL && lastTerm->field() == prefixField ){
		  
		  //now see if term->text() starts with prefixText
		  int32_t termLen = lastTerm->textLength();
		  if ( prefixLen>termLen )
			  break; //the prefix is longer than the term, can't be matched

		  //check for prefix match
		  if ( _tcsncmp(lastTerm->text(),prefixText,prefixLen)!=0 )
			  break;

          TermQuery* tq = _CLNEW TermQuery(lastTerm);	  // found a match
          tq->setBoost(getBoost());                // set the boost
          query->add(tq,true,false, false);		  // add to query
        } else
          break;
		_CLDECDELETE(lastTerm);
      } while (enumerator->next());
    }_CLFINALLY(
      enumerator->close();
	  _CLDELETE(enumerator);
	  _CLDECDELETE(lastTerm);
	);
	_CLDECDELETE(lastTerm);


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

  Query* PrefixQuery::combine(Query** queries) {
	  return Query::mergeBooleanQueries(queries);
  }

  TCHAR* PrefixQuery::toString(const TCHAR* field) {
  //Func - Creates a user-readable version of this query and returns it as as string
  //Pre  - field != NULL
  //Post - a user-readable version of this query has been returned as as string

	  //Instantiate a stringbuffer buffer to store the readable version temporarily
      CL_NS(util)::StringBuffer buffer;
	  //check if field equal to the field of prefix
      if( _tcscmp(prefix->field(),field) != 0 ) {
		  //Append the field of prefix to the buffer
          buffer.append(prefix->field());
		  //Append a colon
          buffer.append(_T(":") );
    }
    //Append the text of the prefix
    buffer.append(prefix->text());
	//Append a wildchar character
    buffer.append(_T("*"));
	//if the boost factor is not eaqual to 1
    if (getBoost() != 1.0f) {
		//Append ^
        buffer.append(_T("^"));
		//Append the boost factor
        buffer.appendFloat( getBoost(),1);
    }
	//Convert StringBuffer buffer to TCHAR block and return it
    return buffer.toString();
  }

CL_NS_END
#endif
