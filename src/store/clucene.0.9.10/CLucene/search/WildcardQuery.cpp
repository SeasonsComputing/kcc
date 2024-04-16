#include "CLucene/StdHeader.h"
#ifndef NO_WILDCARD_QUERY
#include "WildcardQuery.h"

CL_NS_USE(index)
CL_NS_DEF(search)


  WildcardQuery::WildcardQuery(Term* term): 
	MultiTermQuery( term ){
  //Func - Constructor
  //Pre  - term != NULL
  //Post - Instance has been created

  }

  WildcardQuery::~WildcardQuery(){
  //Func - Destructor
  //Pre  - true
  //Post - true

  }

  const TCHAR* WildcardQuery::getQueryName() const{
  //Func - Returns the string "WildcardQuery"
  //Pre  - true
  //Post - The string "WildcardQuery" has been returned
	return getClassName();
  }

  const TCHAR* WildcardQuery::getClassName(){
      return _T("WildcardQuery");
  }


  FilteredTermEnum* WildcardQuery::getEnum(IndexReader* reader) {
    return _CLNEW WildcardTermEnum(reader, getTerm());
  }

	WildcardQuery::WildcardQuery(const WildcardQuery& clone):
		MultiTermQuery(clone)
	{
	}

	Query* WildcardQuery::clone(){
		return _CLNEW WildcardQuery(*this);
	}
	size_t WildcardQuery::hashCode() const{
		//todo: we should give the query a seeding value... but
		//need to do it for all hascode functions
		return Similarity::floatToByte(getBoost()) ^ getTerm()->hashCode();
	}
	bool WildcardQuery::equals(Query* other) const{
		if (!(other->instanceOf(WildcardQuery::getClassName())))
			return false;

		WildcardQuery* tq = (WildcardQuery*)other;
		return (this->getBoost() == tq->getBoost())
			&& getTerm()->equals(tq->getTerm());
	}
CL_NS_END
#endif
