#include "CLucene/StdHeader.h"
#include "ConjunctionScorer.h"

CL_NS_USE(index)
CL_NS_USE(util)
CL_NS_DEF(search)

  Scorer* ConjunctionScorer::first() { 
	 if ( scorers.end() == scorers.begin() )
		return NULL;

	 return *scorers.begin(); 
  } //get First
  Scorer* ConjunctionScorer::last() {
	 if ( scorers.end() == scorers.begin() )
		return NULL;

	CL_NS_STD(list)<Scorer*>::iterator i = scorers.end();
	i--;
	 return *i; 
  } //get Last

  class _ScorerSorter:public CL_NS(util)::Arrays::_Arrays<Scorer*>{
	bool equals(Scorer* o1,Scorer* o2) const{
		return o1->doc() == o2->doc();
	}
	int32_t compare(Scorer* o1,Scorer* o2) const{
		return o1->doc() - o2->doc();
	}
  };
  _ScorerSorter __ScorerSorter;

  void ConjunctionScorer::sortScorers() {
    // move scorers to an array
    int32_t size = scorers.size();
    Scorer** array = _CL_NEWARRAY(Scorer*,size+1);
	scorers.toArray(array);
    scorers.clear();                              // empty the list

	//todo: do this:
    // note that this comparator is not consistent with equals!
	__ScorerSorter.sort(array,size,0,size);

    for (int32_t i = 0; i<size; i++) {
      scorers.push_back(array[i]);                  // re-build list, now sorted
    }
	_CLDELETE_ARRAY(array);
  }

  bool ConjunctionScorer::doNext() {
	CL_ISVALIDORNULLOBJECT(first(),"first() is not VALID");
	CL_ISVALIDORNULLOBJECT(last(),"last() is not VALID");
    while (more && first()->doc() < last()->doc()) { // find doc w/ all clauses
      more = first()->skipTo(last()->doc());      // skip first upto last
	  Scorer* scorer = *scorers.begin();
	  scorers.delete_front();
      scorers.push_back(scorer);   // move first to last
    }
    return more;                                // found a doc with all clauses
  }

  
  void ConjunctionScorer::init()  {
    more = scorers.size() > 0;

    // compute coord factor
    coord = getSimilarity()->coord(scorers.size(), scorers.size());

    // move each scorer to its first entry
	CL_NS_STD(list)<Scorer*>::iterator i = scorers.begin();
    while (more && i!=scorers.end()) {
      more = ((Scorer*)*i)->next();
	  i++;
    }

    if (more)
      sortScorers();                              // initial sort of list

    firstTime = false;
  }

	ConjunctionScorer::ConjunctionScorer(Similarity* similarity):
		firstTime(true),
		more(true),
		Scorer(similarity),
		scorers(false)
	{
    }
	ConjunctionScorer::~ConjunctionScorer(){
		scorers.setDoDelete(true);
	}

	TCHAR *CL_NS(search)::Scorer::toString(void){
		return _T("ConjunctionScorer");
	}
	

  void ConjunctionScorer::add(Scorer* scorer){
    scorers.put(scorer);
  }


  int32_t ConjunctionScorer::doc() { return first()->doc(); }

  bool ConjunctionScorer::next()  {
    if (firstTime) {
      init();
    } else if (more) {
      more = last()->next();                       // trigger further scanning
    }
    return doNext();
  }
  
  bool ConjunctionScorer::skipTo(int32_t target) {
    CL_NS_STD(list)<Scorer*>::iterator i = scorers.begin();
    while (more && i!=scorers.end()) {
      more = ((Scorer*)*i)->skipTo(target);

	  i++;
    }
    if (more)
      sortScorers();                              // re-sort scorers
    return doNext();
  }

  float_t ConjunctionScorer::score() {
    float_t score = 0.0f;                           // sum scores
    CL_NS_STD(list)<Scorer*>::iterator i = scorers.begin();
	while (i!=scorers.end()){
      score += ((Scorer*)*i)->score();

	  i++;
	}
    score *= coord;
    return score;
  }



CL_NS_END
