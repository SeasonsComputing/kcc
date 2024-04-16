#ifndef _lucene_search_SloppyPhraseScorer_
#define _lucene_search_SloppyPhraseScorer_
#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#ifndef NO_FUZZY_QUERY

#include "PhraseScorer.h"
#include "CLucene/index/Terms.h"


CL_NS_DEF(search)
	class SloppyPhraseScorer: public PhraseScorer {
	private:
		int32_t slop;

	public:
		SloppyPhraseScorer(Weight* weight, CL_NS(index)::TermPositions** tps, 
			int32_t* positions, Similarity* similarity, 
			int32_t slop, uint8_t* norms);
		~SloppyPhraseScorer(){
		}

	protected:
		float_t phraseFreq();
	};
CL_NS_END
#endif
#endif

