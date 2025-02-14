#include "CLucene/StdHeader.h"
#include "IndexSearcher.h"

#include "SearchHeader.h"
#include "Scorer.h"
#include "FieldDoc.h"
#include "CLucene/store/Directory.h"
#include "CLucene/document/Document.h"
#include "CLucene/index/IndexReader.h"
#include "CLucene/index/Term.h"
#include "CLucene/util/BitSet.h"
#include "HitQueue.h"
#include "FieldSortedHitQueue.h"

CL_NS_USE(index)
CL_NS_USE(util)
CL_NS_USE(document)

CL_NS_DEF(search)


	SimpleTopDocsCollector::SimpleTopDocsCollector(const CL_NS(util)::BitSet* bs, HitQueue* hitQueue, int32_t* totalhits,const int32_t ndocs, const float_t minScore):
		bits(bs),
		hq(hitQueue),
		nDocs(ndocs),
		totalHits(totalhits),
		ms(minScore)
	{
	}
	SimpleTopDocsCollector::~SimpleTopDocsCollector(){
	}
	void SimpleTopDocsCollector::collect(const int32_t doc, const float_t score){
		if (score > 0.0f &&			  // ignore zeroed buckets
			(bits==NULL || bits->get(doc))) {	  // skip docs not in bits
			totalHits[0]++;
			if (hq->size() < nDocs || (ms==-1.0f || score >= ms)) {
				//todo: could use a struct for scoredoc and then
				//set the value, then call reorder...?
			ScoreDoc* sd = _CLNEW ScoreDoc(doc, score);
				if ( !hq->insert(sd) )	  // update hit queue
					_CLDELETE(sd);
				if ( ms != -1.0f )
					minScore = ((ScoreDoc*)hq->top())->score; // maintain minScore
			}
		}
	}


	SortedTopDocsCollector::SortedTopDocsCollector(const CL_NS(util)::BitSet* bs, FieldSortedHitQueue* hitQueue, int32_t* totalhits,const int32_t ndocs):
		bits(bs),
		hq(hitQueue),
		nDocs(ndocs),
		totalHits(totalhits)
	{
	}
	SortedTopDocsCollector::~SortedTopDocsCollector(){
	}
	void SortedTopDocsCollector::collect(const int32_t doc, const float_t score){
		if (score > 0.0f &&			  // ignore zeroed buckets
			(bits==NULL || bits->get(doc))) {	  // skip docs not in bits
			totalHits[0]++;
			FieldDoc* fd = _CLNEW FieldDoc(doc, score); //todo: see jlucene way... with fields def???
			if ( !hq->insert(fd) )	  // update hit queue
				_CLDELETE(fd);
		}
	}

  SimpleFilteredCollector::SimpleFilteredCollector(BitSet* bs, HitCollector* collector):
    bits(bs),
    results(collector)
  {
  }
  SimpleFilteredCollector::~SimpleFilteredCollector(){
  }

  void SimpleFilteredCollector::collect(const int32_t doc, const float_t score) {
    if (bits->get(doc)) {		  // skip docs not in bits
      results->collect(doc, score);
    }
  }

  IndexSearcher::IndexSearcher(const char* path){
  //Func - Constructor
  //       Creates a searcher searching the index in the named directory.  */
  //Pre  - path != NULL
  //Post - The instance has been created

      CND_PRECONDITION(path != NULL, "path is NULL");

      reader = IndexReader::open(path);
      readerOwner = true;
  }
  
  IndexSearcher::IndexSearcher(CL_NS(store)::Directory* directory){
  //Func - Constructor
  //       Creates a searcher searching the index in the specified directory.  */
  //Pre  - path != NULL
  //Post - The instance has been created

      CND_PRECONDITION(directory != NULL, "directory is NULL");

      reader = IndexReader::open(directory);
      readerOwner = true;
  }

  IndexSearcher::IndexSearcher(IndexReader* r){
  //Func - Constructor
  //       Creates a searcher searching the index with the provide IndexReader
  //Pre  - path != NULL
  //Post - The instance has been created

      reader      = r;
      readerOwner = false;
  }

  IndexSearcher::~IndexSearcher(){
  //Func - Destructor
  //Pre  - true
  //Post - The instance has been destroyed

	  close();
  }

  void IndexSearcher::close(){
  //Func - Frees resources associated with this Searcher.
  //Pre  - true
  //Post - The resources associated have been freed
      if (readerOwner && reader){
          reader->close();
          _CLDELETE(reader);
      }
  }

  // inherit javadoc
  int32_t IndexSearcher::docFreq(const Term* term) const{
  //Func - 
  //Pre  - reader != NULL
  //Post -

      CND_PRECONDITION(reader != NULL, "reader is NULL");

      return reader->docFreq(term);
  }

  
  // inherit javadoc
  Document* IndexSearcher::doc(const int32_t i) {
  //Func - Retrieves i-th document found
  //       For use by HitCollector implementations.
  //Pre  - reader != NULL
  //Post - The i-th document has been returned

      CND_PRECONDITION(reader != NULL, "reader is NULL");

      return reader->document(i);
  }

  // inherit javadoc
  int32_t IndexSearcher::maxDoc() const {
  //Func - Return total number of documents including the ones marked deleted
  //Pre  - reader != NULL
  //Post - The total number of documents including the ones marked deleted 
  //       has been returned

      CND_PRECONDITION(reader != NULL, "reader is NULL");

      return reader->maxDoc();
  }

  TopDocs* IndexSearcher::_search(Query* query, Filter* filter, const int32_t nDocs){
  //Func -
  //Pre  - reader != NULL
  //Post -

      CND_PRECONDITION(reader != NULL, "reader is NULL");

	  Weight* weight = query->weight(this);
      Scorer* scorer = weight->scorer(reader);
	  if (scorer == NULL){
		  ScoreDoc** sds = _CL_NEWARRAY(ScoreDoc*,1);
		  sds[0]=NULL;
          return _CLNEW TopDocs(0, sds);
	  }

      BitSet* bits = filter != NULL ? filter->bits(reader) : NULL;
      HitQueue* hq = _CLNEW HitQueue(nDocs);

	  //Check hq has been allocated properly
	  CND_CONDITION(hq != NULL, "Could not allocate memory for HitQueue hq");

	  int32_t* totalHits = _CL_NEWARRAY(int32_t,1);
      totalHits[0] = 0;

      SimpleTopDocsCollector hitCol(bits,hq,totalHits,nDocs,0.0f);
      scorer->score( &hitCol );
      _CLDELETE(scorer);

      int32_t scoreDocsLength = hq->size();

		ScoreDoc** scoreDocs = _CL_NEWARRAY(ScoreDoc*,scoreDocsLength+1);

		for (int32_t i = scoreDocsLength-1; i >= 0; i--)	  // put docs in array
			scoreDocs[i] = hq->pop();
		scoreDocs[scoreDocsLength] = NULL;

      int32_t totalHitsInt = totalHits[0];

      _CLDELETE(hq);
      _CLDELETE(bits);
      _CLDELETE_ARRAY(totalHits);
	  Query* wq = weight->getQuery();
	  if ( query != wq ) //query was re-written
		  _CLLDELETE(wq);
	  _CLDELETE(weight);

      return _CLNEW TopDocs(totalHitsInt, scoreDocs);
  }

  // inherit javadoc
  TopFieldDocs* IndexSearcher::_search(Query* query, Filter* filter, int32_t nDocs,
         const Sort* sort) {
    Weight* weight = query->weight(this);
    Scorer* scorer = weight->scorer(reader);
    if (scorer == NULL){
      ScoreDoc** sds = _CL_NEWARRAY(ScoreDoc*,0);
		return _CLNEW TopFieldDocs(0, sds, sort->getFields());
	}

    BitSet* bits = filter != NULL ? filter->bits(reader) : NULL;
    FieldSortedHitQueue* hq =
      _CLNEW FieldSortedHitQueue(reader, sort->getFields(), nDocs);
    int32_t* totalHits = _CL_NEWARRAY(int32_t,1);
	totalHits[0]=0;
    
	SortedTopDocsCollector hitCol(bits,hq,totalHits,nDocs);
	scorer->score(&hitCol);
    _CLDELETE(scorer);

	int32_t size = hq->size();
    ScoreDoc** scoreDocs = _CL_NEWARRAY(ScoreDoc*,size+1);
    for (int32_t i = hq->size()-1; i >= 0; i--)	  // put docs in array
      scoreDocs[i] = hq->fillFields ((FieldDoc*) hq->pop());
	scoreDocs[size] = NULL;

    Query* wq = weight->getQuery();
	if ( query != wq ) //query was re-written
		_CLLDELETE(wq);
	_CLDELETE(weight);

    SortField** hqFields = hq->getFields();
    int32_t totalHits0 = totalHits[0];
    _CLDELETE(hq);
    _CLDELETE_ARRAY(totalHits);
    return _CLNEW TopFieldDocs(totalHits0, scoreDocs, hqFields);
  }

  void IndexSearcher::_search(Query* query, Filter* filter, HitCollector* results){
  //Func - _search an index and fetch the results
  //       Applications should only use this if they need all of the
  //       matching documents.  The high-level search API (search(Query)) is usually more efficient, 
  //       as it skips non-high-scoring hits.
  //Pre  - query is a valid reference to a query
  //       filter may or may not be NULL
  //       results is a valid reference to a HitCollector and used to store the results
  //Post - filter if non-NULL, a bitset used to eliminate some documents

      BitSet* bs = NULL;
      SimpleFilteredCollector* fc = NULL; 

      if (filter != NULL){
          bs = filter->bits(reader);
          fc = _CLNEW SimpleFilteredCollector(bs, results);
       }

      //todo: need to delete the returned weight???
      Scorer* scorer = query->weight(this)->scorer(reader);
      if (scorer != NULL) {
		  if (fc == NULL){
              scorer->score(results);
		  }else{
              scorer->score((HitCollector*)fc);
		  }
          _CLDELETE(scorer); 
      }

    _CLDELETE(fc); 
    _CLDELETE(bs);
  }

  Query* IndexSearcher::rewrite(Query* original) {
        Query* query = original;
		Query* last = original;
        for (Query* rewrittenQuery = query->rewrite(reader); 
				rewrittenQuery != query;
				rewrittenQuery = query->rewrite(reader)) {
			query = rewrittenQuery;
			if ( query != last && last != original ){
				_CLDELETE(last);
			}
			last = query;
        }
        return query;
    }

    Explanation* IndexSearcher::explain(Query* query, int32_t doc){
        Weight* weight = query->weight(this);
        Explanation* ret = weight->explain(reader, doc);

        Query* wq = weight->getQuery();
	    if ( query != wq ) //query was re-written
		  _CLLDELETE(wq);
        _CLDELETE(weight);

        return ret;
    }

CL_NS_END
