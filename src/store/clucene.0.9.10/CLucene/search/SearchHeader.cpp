#include "CLucene/StdHeader.h"
#include "SearchHeader.h"
#include "BooleanQuery.h"

CL_NS_USE(index)
CL_NS_DEF(search)

//static
Query* Query::mergeBooleanQueries(Query** queries) {
	//todo: was using hashset???
    CL_NS(util)::CLVector<BooleanClause*> allClauses;
    int32_t i = 0;
    while ( queries[i] != NULL ){
		BooleanQuery* bq = (BooleanQuery*)queries[i];
		BooleanClause** clauses = bq->getClauses();
		int32_t j = 0;
		while ( clauses[j] != NULL ){
			allClauses.put(clauses[j]);
			j++;
		}
		_CLDELETE_ARRAY(clauses);
		i++;
    }

    BooleanQuery* result = _CLNEW BooleanQuery();
    CL_NS(util)::CLVector<BooleanClause*>::iterator itr = allClauses.begin();
    while (itr != allClauses.end() ) {
		result->add(*itr);
    }
    return result;
}

/** Expert: Constructs an initializes a Weight for a top-level query. 
    Returns a weight, the query object of the weight must be deleted.
*/
Weight* Query::weight(Searcher* searcher){
    Query* query = searcher->rewrite(this);
    Weight* weight = query->createWeight(searcher);
    float_t sum = weight->sumOfSquaredWeights();
    float_t norm = getSimilarity(searcher)->queryNorm(sum);
    weight->normalize(norm);
    return weight;
}

TopFieldDocs::~TopFieldDocs(){
	if ( fields != NULL ){
		int i=0;
		while ( fields[i]!=NULL ){
			_CLDELETE(fields[i]);
			i++;
		}
		_CLDELETE_ARRAY(fields);
	}
}


TopDocs::TopDocs(const int32_t th, ScoreDoc **sds):
    totalHits(th){
//Func - Constructor
//Pre  - sds may or may not be NULL
//       sdLength >= 0
//Post - The instance has been created

	scoreDocs       = sds;
	deleteScoreDocs = true;

}

TopDocs::~TopDocs(){
//Func - Destructor
//Pre  - true
//Post - The instance has been destroyed

	//Check if scoreDocs is valid
	if (scoreDocs){
        if ( deleteScoreDocs ){
			for ( int32_t i=0;scoreDocs[i]!=NULL;i++ ){
				_CLDELETE(scoreDocs[i]);
			}
		}
        _CLDELETE_ARRAY(scoreDocs);
	}
}

CL_NS_END
