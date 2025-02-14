#include "CLucene/StdHeader.h"

#include "SearchHeader.h"
#include "CLucene/document/Document.h"
#include "CLucene/index/IndexReader.h"
#include "Filter.h"
#include "ScoreDoc.h"
#include "CLucene/search/SearchHeader.h"

CL_NS_USE(document)
CL_NS_USE(util)
CL_NS_USE(index)

CL_NS_DEF(search)

	HitDoc::HitDoc(const float_t s, const int32_t i)
	{
	//Func - Constructor
	//Pre  - true
	//Post - The instance has been created

		next  = NULL;
		prev  = NULL;
		doc   = NULL;
		score = s;
		id    = i;
	}

	HitDoc::~HitDoc(){
	//Func - Destructor
	//Pre  - true
	//Post - The instance has been destroyed

		_CLDELETE(doc);
	}


	Hits::Hits(Searcher* s, Query* q, Filter* f, const Sort* _sort):
		query(q), searcher(s), filter(f), sort(_sort)
	{
	//Func - Constructor
	//Pre  - s contains a valid reference to a searcher s
	//       q contains a valid reference to a Query
	//       f is NULL or contains a pointer to a filter
	//Post - The instance has been created

		_length  = 0;
		first   = NULL;
		last    = NULL;
		numDocs = 0;
		maxDocs = 200;

		//retrieve 100 initially
		getMoreDocs(50);
	}

	Hits::~Hits(){

	}
	int32_t Hits::length() const {
		return _length;
	}

	/** Returns the stored fields of the n<sup>th</sup> document in this set.
	<p>Documents are cached, so that repeated requests for the same element may
	return the same Document object. 
    *
    * @memory Memory belongs to the hits object. Don't delete the return value.
    */
	Document& Hits::doc(const int32_t n){
		HitDoc* hitDoc = getHitDoc(n);

		// Update LRU cache of documents
		remove(hitDoc);				  // remove from list, if there
		addToFront(hitDoc);				  // add to front of list
		if (numDocs > maxDocs) {			  // if cache is full
			HitDoc* oldLast = last;
			remove(last);				  // flush last

			_CLDELETE( oldLast->doc );
			oldLast->doc = NULL;
		}

		if (hitDoc->doc == NULL)
			hitDoc->doc = searcher->doc(hitDoc->id);	  // cache miss: read document

		return *hitDoc->doc;
	}

	/** Returns the id for the nth document in this set. */
	int32_t Hits::id (const int32_t n){
		return getHitDoc(n)->id;
	}

    float_t Hits::score(const int32_t n){
		return getHitDoc(n)->score;
	}

	void Hits::getMoreDocs(const int32_t m){
		int32_t min = m;
		{
			// It's inconceivable that the number of hits would exceed the
			// capacity of int32_t, so the following cast is safe.
			int32_t nHits = static_cast<int32_t>(hitDocs.size());
			if (nHits > min)
				min = nHits;
		}

		int32_t n = min * 2;				  // double # retrieved
		TopDocs* topDocs = NULL;
		if ( sort==NULL )
			topDocs = (TopDocs*)((Searchable*)searcher)->_search(query, filter, n);
		else
			topDocs = ((Searchable*)searcher)->_search(query, filter, n, sort);
		_length = topDocs->totalHits;
		ScoreDoc** scoreDocs = topDocs->scoreDocs;

		float_t scoreNorm = 1.0f;
		//Check that scoreDocs is a valid pointer before using it
		if (scoreDocs[0] != NULL){
			if (_length > 0 && scoreDocs[0]->score > 1.0f){
				scoreNorm = 1.0f / scoreDocs[0]->score;
			}

			//todo: we are moving the scoredocs onto the hitDocs in reverse
			//jlucene knows the length of scoreDocs, but we run through from the
			//front, and insert the documents always at the front
			int32_t i = hitDocs.size();
			while( i<_length && scoreDocs[i] != NULL){
				hitDocs.push_back(_CLNEW HitDoc(scoreDocs[i]->score*scoreNorm, scoreDocs[i]->doc));
				i++;
			}
		}

		_CLDELETE(topDocs);
	}

	HitDoc* Hits::getHitDoc(const int32_t n){
		CND_PRECONDITION(n >= 0, "n must not be negative");
		if (n >= _length){
		    TCHAR buf[100];
            _sntprintf(buf, 100,_T("Not a valid hit number: %d"),n);
			_CLTHROWT(CL_ERR_IndexOutOfBounds, buf );
		}
		if (static_cast<uint32_t>(n) >= hitDocs.size())
			getMoreDocs(n);

		return ((HitDoc*)(hitDocs[n]));
	}

	void Hits::addToFront(HitDoc* hitDoc) {  // insert at front of cache
		if (first == NULL)
			last = hitDoc;
		else
			first->prev = hitDoc;

		hitDoc->next = first;
		first = hitDoc;
		hitDoc->prev = NULL;

		numDocs++;
	}

	void Hits::remove(HitDoc* hitDoc) {	  // remove from cache
		if (hitDoc->doc == NULL)			  // it's not in the list
			return;					  // abort

		if (hitDoc->next == NULL)
			last = hitDoc->prev;
		else
			hitDoc->next->prev = hitDoc->prev;

		if (hitDoc->prev == NULL)
			first = hitDoc->next;
		else
			hitDoc->prev->next = hitDoc->next;

		numDocs--;
	}
CL_NS_END
