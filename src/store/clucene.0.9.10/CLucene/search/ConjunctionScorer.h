#ifndef _lucene_search_ConjunctionScorer_
#define _lucene_search_ConjunctionScorer_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif
#include "Scorer.h"
#include "Similarity.h"

CL_NS_DEF(search)

/** Scorer for conjunctions, sets of queries, all of which are required. */
class ConjunctionScorer: public Scorer {
private:
  CL_NS(util)::CLLinkedList<Scorer*,CL_NS(util)::Deletor::Object<Scorer> > scorers;
  bool firstTime;
  bool more;
  float_t coord;

  Scorer* first();
  Scorer* last();
  void sortScorers();
  bool doNext();
  void init();
public:
  ConjunctionScorer(Similarity* similarity);
  virtual ~ConjunctionScorer();
  TCHAR* toString(void){
	return _T("ConjunctionScorer");
  }
  void add(Scorer* scorer);
  int32_t doc();
  bool next();
  bool skipTo(int32_t target);
  float_t score();
  virtual Explanation* explain(int32_t doc) {
    _CLTHROWA(CL_ERR_UnsupportedOperation,"UnsupportedOperationException()");
  }


};

CL_NS_END
#endif
