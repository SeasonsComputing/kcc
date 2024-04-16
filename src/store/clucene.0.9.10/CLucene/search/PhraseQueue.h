#ifndef _lucene_search_PriorityQueue_
#define _lucene_search_PriorityQueue_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#include "CLucene/util/PriorityQueue.h"
#include "PhrasePositions.h"

CL_NS_DEF(search)
	class PhraseQueue: public CL_NS(util)::PriorityQueue<PhrasePositions*,
		CL_NS(util)::Deletor::Object<PhrasePositions> > {
	public:
		PhraseQueue(const int32_t size) {
			initialize(size,false);
		}
		~PhraseQueue(){
		}

	protected:
		bool lessThan(PhrasePositions* pp1, PhrasePositions* pp2) {
			if (pp1->doc == pp2->doc) 
				return pp1->position < pp2->position;
			else
				return pp1->doc < pp2->doc;
		}
	};
CL_NS_END
#endif
