#include "CLucene/StdHeader.h"
#include "PhraseScorer.h"

#include "PhraseQueue.h"
#include "PhrasePositions.h"
#include "Scorer.h"
#include "Similarity.h"

CL_NS_USE(index)
CL_NS_USE(util)
CL_NS_DEF(search)


	PhraseScorer::PhraseScorer(Weight* weight, TermPositions** tps, 
		int32_t* positions, Similarity* similarity, uint8_t* norms):
		Scorer(similarity)
	{
	//Func - Constructor
	//Pre  - tps != NULL and is an array of TermPositions
	//       tpsLength >= 0
	//       n != NULL
	//Post - The instance has been created

		CND_PRECONDITION(tps != NULL,"tps is NULL");
		
		//norms are only used if phraseFreq returns more than 0.0
		//phraseFreq should only return more than 0.0 if norms != NULL
		//CND_PRECONDITION(n != NULL,"n is NULL");

		firstTime = true;
		more = true;
		this->norms = norms;
		this->weight = weight;
		this->value = weight->getValue();

		//reset internal pointers
		first   = NULL;
		last    = NULL;

		//use pq to build a sorted list of PhrasePositions
		int32_t i = 0;
		while(tps[i] != NULL){
			PhrasePositions *pp = _CLNEW PhrasePositions(tps[i], positions[i]);
			CND_CONDITION(pp != NULL,"Could not allocate memory for pp");

			//Store PhrasePos into the PhrasePos pq
			if (last != NULL) {			  // add next to end of list
				last->_next = pp;
			} else
				first = pp;
			last = pp;

			i++;
		}

		pq = _CLNEW PhraseQueue(i); //i==tps.length
		CND_CONDITION(pq != NULL,"Could not allocate memory for pq");
	}

	PhraseScorer::~PhraseScorer() {
	//Func - Destructor
	//Pre  - true
	//Post - The instance has been destroyed

		//The PhraseQueue pq (which is a PriorityQueue) pq is actually empty at present, the elements
		//having been transferred by pqToList() to the linked list starting with
		//first.  The nodes of that linked list are deleted by the destructor of
		//first, rather than the destructor of pq.
		_CLDELETE(first);
		_CLDELETE(pq);
	}
/*
	void PhraseScorer::score(HitCollector* results, const int32_t end){
	//Func -
	//Pre  - first != NULL
	//       last != NULL
	//Post -

		CND_PRECONDITION(first != NULL,"first is NULL");
		CND_PRECONDITION(last  != NULL,"last is NULL");

		// find doc with all the terms
		while (last->doc < end) {

			// scan forward in first
			while (first->doc < last->doc){
				do  {
					first->next();
				} while (first->doc < last->doc);

				//Make the current first the last node
				firstToLast();
				if (last->doc >= end){
					return;
				}
			}

			//found doc with all terms

			//check for phrase
			float_t freq = phraseFreq();

			if (freq > 0.0){
				//Compute score
				float_t score = Similarity::tf(freq)*weight;
				//Normalize
				score *= Similarity::normf(norms[first->doc]);
				//Add to results
				results->collect(first->doc, score);
			}
			//Resume scanning
			last->next();
		}
	}*/


	bool PhraseScorer::next(){
		if (firstTime) {
			init();
			firstTime = false;
		} else if (more) {
			more = last->next(); // trigger further scanning
		}
		return doNext();
	}

	// next without initial increment
	bool PhraseScorer::doNext() {
		while (more) {
			while (more && first->doc < last->doc) {      // find doc w/ all the terms
				more = first->skipTo(last->doc);            // skip first upto last
				firstToLast();                            // and move it to the end
			}

			if (more) {
				// found a doc with all of the terms
				freq = phraseFreq();                      // check for phrase
				if (freq == 0.0f)                         // no match
					more = last->next();                     // trigger further scanning
				else
					return true;                            // found a match
			}
		}
		return false;                                 // no more matches
	}

	float_t PhraseScorer::score() {
		//System.out.println("scoring " + first.doc);
		float_t raw = getSimilarity()->tf(freq) * value; // raw score
		return raw * Similarity::decodeNorm(norms[first->doc]); // normalize
	}

	bool PhraseScorer::skipTo(int32_t target) {
		for (PhrasePositions* pp = first; more && pp != NULL; pp = pp->_next) {
			more = pp->skipTo(target);
		}
		if (more)
			sort();                                     // re-sort
		return doNext();
	}

	void PhraseScorer::init() {
		for (PhrasePositions* pp = first; more && pp != NULL; pp = pp->_next) 
			more = pp->next();
		if(more)
			sort();
	}
	  
	void PhraseScorer::sort() {
		pq->clear();
		for (PhrasePositions* pp = first; pp != NULL; pp = pp->_next)
			pq->put(pp);
		pqToList();
	}



	void PhraseScorer::pqToList(){
	//Func - Transfers the PhrasePositions from the PhraseQueue pq to
	//       the PhrasePositions list with first as its first element
	//Pre  - pq != NULL
	//       first = NULL
	//       last = NULL
	//Post - All PhrasePositions have been transfered to the list
	//       of PhrasePositions of which the first element is pointed to by first
	//       and the last element is pointed to by last

		CND_PRECONDITION(pq != NULL,"pq is NULL");
		
		last = first = NULL;

		PhrasePositions* PhrasePos = NULL;

		//As long pq is not empty
		while (pq->top() != NULL){
			//Pop a PhrasePositions instance
			PhrasePos = pq->pop();

			// add next to end of list
			if (last != NULL) {
				last->_next = PhrasePos;
			} else {
				first = PhrasePos;
			}

			//Let last point to the new last PhrasePositions instance just added
			last = PhrasePos;
			//Reset the next of last to NULL
			last->_next = NULL;
		}

		//Check to see that pq is empty now
		CND_CONDITION(pq->size()==0, "pq is not empty while it should be");
	}

	void PhraseScorer::firstToLast(){
	//Func - Moves first to the end of the list
	//Pre  - first is NULL or points to an PhrasePositions Instance
	//       last  is NULL or points to an PhrasePositions Instance
	//       first and last both are NULL or both are not NULL
	//Post - The first element has become the last element in the list

		CND_PRECONDITION(((first==NULL && last==NULL) ||(first !=NULL && last != NULL)),
					   "Either first or last is NULL but not both");

		//Check if first and last are valid pointers
		if(first && last){
			last->_next = first;
			last = first;
			first = first->_next;
			last->_next = NULL;
		}
	}


	Explanation* PhraseScorer::explain(int32_t _doc) {
		Explanation* tfExplanation = _CLNEW Explanation();

		while (next() && doc() < _doc){
		}

		float_t phraseFreq = (doc() == _doc) ? freq : 0.0f;
		tfExplanation->setValue(getSimilarity()->tf(phraseFreq));

		StringBuffer buf;
		buf.append(_T("tf(phraseFreq="));
		buf.appendFloat(phraseFreq,2);
		buf.append(_T(")"));
		tfExplanation->setDescription(buf.getBuffer());

		return tfExplanation;
	}

	TCHAR* PhraseScorer::toString() { 
		StringBuffer buf;
		buf.append(_T("scorer("));

		TCHAR* tmp = weight->toString();
		buf.append(tmp);
		_CLDELETE_CARRAY(tmp);

		buf.append(_T(")"));

		return buf.toString();
	}

CL_NS_END
