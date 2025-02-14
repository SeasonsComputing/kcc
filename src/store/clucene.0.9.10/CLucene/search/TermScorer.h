#ifndef _lucene_search_TermScorer_
#define _lucene_search_TermScorer_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#include "Scorer.h"
#include "CLucene/index/Terms.h"
#include "CLucene/search/Similarity.h"
#include "SearchHeader.h"

CL_NS_DEF(search)
    
	class TermScorer: public Scorer {
	private:
		CL_NS(index)::TermDocs* termDocs;
		uint8_t* norms;
		Weight* weight;
		const float_t weightValue;
		int32_t _doc;

		int32_t docs[32];	  // buffered doc numbers
		int32_t freqs[32];	  // buffered term freqs
		int32_t pointer;
		int32_t pointerMax;

		float_t scoreCache[LUCENE_SCORE_CACHE_SIZE];
	public:

		//TermScorer takes TermDocs and delets it when TermScorer is cleaned up
		TermScorer(Weight* weight, CL_NS(index)::TermDocs* td, 
		Similarity* similarity, uint8_t* _norms);

		~TermScorer();

		int32_t doc() { return _doc; }

		bool next();
		bool skipTo(int32_t target);
		Explanation* explain(int32_t doc);
		TCHAR* toString();

		float_t score();
    };
CL_NS_END
#endif
