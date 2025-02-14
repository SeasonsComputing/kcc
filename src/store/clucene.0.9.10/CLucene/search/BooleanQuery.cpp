#include "CLucene/StdHeader.h"
#include "BooleanQuery.h"

#include "BooleanClause.h"
#include "CLucene/index/IndexReader.h"
#include "CLucene/util/StringBuffer.h"
#include "CLucene/util/Arrays.h"
#include "SearchHeader.h"
#include "BooleanScorer.h"
#include "Scorer.h"

CL_NS_USE(index)
CL_NS_USE(util)
CL_NS_DEF(search)

	BooleanQuery::BooleanQuery():
		clauses(true)
	{
    }

	BooleanQuery::BooleanQuery(const BooleanQuery& clone)
	{
		for ( uint32_t i=0;i<clone.clauses.size();i++ ){
			BooleanClause* clause = clone.clauses[i]->clone();
			clause->deleteQuery=true;//todo: is this right?
			add(clause);
		}
	}

    BooleanQuery::~BooleanQuery(){
		clauses.clear();
    }

	size_t BooleanQuery::hashCode() const {
		//todo: do cachedHashCode, and invalidate on add/remove clause
		size_t ret = 0;
		for (uint32_t i = 0 ; i < clauses.size(); i++) {
			BooleanClause* c = clauses[i];
			ret = 31 * ret + c->hashCode();
		}
		ret = ret ^ Similarity::floatToByte(getBoost());
		return ret;
	}

    const TCHAR* BooleanQuery::getQueryName() const{
      return getClassName();
    }
	const TCHAR* BooleanQuery::getClassName(){
      return _T("BooleanQuery");
    }

   /**
   * Default value is 1024.  Use <code>org.apache.lucene.maxClauseCount</code>
   * system property to override.
   */
   int32_t booleanquery_maxClauseCount = LUCENE_BOOLEANQUERY_MAXCLAUSECOUNT;
   int32_t BooleanQuery::getMaxClauseCount(){
      return booleanquery_maxClauseCount;
   }

   void BooleanQuery::setMaxClauseCount(int32_t maxClauseCount){
      booleanquery_maxClauseCount = maxClauseCount;
   }

  void BooleanQuery::add(Query* query, const bool deleteQuery, const bool required, const bool prohibited) {
		BooleanClause* bc = _CLNEW BooleanClause(query,deleteQuery,required, prohibited);
		try{
			add(bc);
		}catch(...){ //todo: only catch IO Err???
			_CLDELETE(bc);
			throw;
		}
  }

  void BooleanQuery::add(BooleanClause* clause) {
    if (clauses.size() >= getMaxClauseCount())
      _CLTHROWA(CL_ERR_TooManyClauses,"Too Many Clauses");

    clauses.push_back(clause);
  }

  /*void BooleanQuery::prepare(IndexReader* reader) {
    for (uint32_t i = 0 ; i < clauses.size(); i++) {
      BooleanClause* c = clauses[i];
      c->query->prepare(reader);
    }
  }

  void BooleanQuery::normalize(const float_t norm) {
    for (uint32_t i = 0 ; i < clauses.size(); i++) {
      BooleanClause* c = clauses[i];
      if (!c->prohibited)
        c->query->normalize(norm);
    }
  }
  float_t BooleanQuery::sumOfSquaredWeights(Searcher* searcher){
    float_t sum = 0.0f;

    for (uint32_t i = 0 ; i < clauses.size(); i++) {
      BooleanClause* c = clauses[i];
      if (!c->prohibited)
        sum += c->query->sumOfSquaredWeights(searcher); // sum sub-query weights
    }

    return sum;
  }
*/


  int32_t BooleanQuery::getClauseCount() {
    return (int32_t) clauses.size();
  }

  /*Scorer* BooleanQuery::scorer(IndexReader* reader){
    if (clauses.size() == 1) {			  // optimize 1-term queries
      BooleanClause* c = clauses[0];
      if (!c->prohibited)			  // just return term scorer
        return c->query->scorer(reader);
    }

    BooleanScorer* result = _CLNEW BooleanScorer;
    for (uint32_t i = 0 ; i < clauses.size(); i++) {
      BooleanClause* c = clauses[i];
      Scorer* subScorer = c->query->scorer(reader);
      if (subScorer != NULL)
        result->add(subScorer, c->required, c->prohibited);
      else if (c->required)
        return NULL;
    }

    return result;
  }*/

  TCHAR* BooleanQuery::toString(const TCHAR* field) {
    StringBuffer buffer;
    if (getBoost() != 1.0) {
      buffer.append(_T("("));
    }

    for (uint32_t i = 0 ; i < clauses.size(); i++) {
      BooleanClause* c = clauses[i];
      if (c->prohibited)
        buffer.append(_T("-"));
      else if (c->required)
        buffer.append(_T("+"));

      if ( c->query->instanceOf(BooleanQuery::getClassName()) ) {	  // wrap sub-bools in parens
        buffer.append(_T("("));

        const TCHAR* buf = c->query->toString(field);
        buffer.append(buf);
        _CLDELETE_CARRAY( buf );

        buffer.append(_T(")"));
      } else {
        const TCHAR* buf = c->query->toString(field);
        buffer.append(buf);
        _CLDELETE_CARRAY( buf );
      }
      if (i != clauses.size()-1)
        buffer.append(_T(" "));

      if (getBoost() != 1.0) {
         buffer.append(_T(")^"));
         buffer.appendFloat(getBoost(),1);
      }
    }
    return buffer.toString();
  }





	BooleanClause** BooleanQuery::getClauses()
	{
		size_t size=clauses.size();
		BooleanClause** ret = _CL_NEWARRAY(BooleanClause*,size+1);
		for ( uint32_t i=0;i<clauses.size();i++ )
			ret[i] = clauses[i];
		ret[size]=NULL;
		return ret;
	}
	  Query* BooleanQuery::rewrite(IndexReader* reader) {
         if (clauses.size() == 1) {                    // optimize 1-clause queries
            BooleanClause* c = clauses[0];
            if (!c->prohibited) {			  // just return clause
				Query* query = c->query->rewrite(reader);    // rewrite first

				if (getBoost() != 1.0f) {                 // incorporate boost
					if (query == c->query)                   // if rewrite was no-op
						query = query->clone();         // then clone before boost
					query->setBoost(getBoost() * query->getBoost());
				}

				//the returned query 'owns' the object now, dont delete it.
				c->deleteQuery=false;
				return query;
            }
         }

         BooleanQuery* clone = NULL;                    // recursively rewrite
		 for (uint32_t i = 0 ; i < clauses.size(); i++) {
            BooleanClause* c = clauses[i];
            Query* query = c->query->rewrite(reader);
            if (query != c->query) {                     // clause rewrote: must clone
               if (clone == NULL)
                  clone = (BooleanQuery*)this->clone();
			   //todo: check if delete query should be on...
			   //in fact we should try and get rid of these
			   //for compatibility sake
               clone->clauses.set (i, _CLNEW BooleanClause(query, true, c->required, c->prohibited));
            }
         }
         if (clone != NULL) {
			 return clone;                               // some clauses rewrote
         } else
            return this;                                // no clauses rewrote
      }


      Query* BooleanQuery::clone() {
		 BooleanQuery* clone = _CLNEW BooleanQuery(*this);
         return clone;
      }

      /** Returns true iff <code>o</code> is equal to this. */
      bool BooleanQuery::equals(Query* o)const {
         if (!(o->instanceOf(BooleanQuery::getClassName())))
            return false;
         const BooleanQuery* other = (BooleanQuery*)o;

		 bool ret = (this->getBoost() == other->getBoost());
		 if ( ret ){
			 CLListEquals<BooleanClause,BooleanClause::Compare,
				 const _clauses,
				 const _clauses> comp;
			 ret = comp(&this->clauses,&other->clauses);
		 }
		return ret;
      }

	float_t BooleanQuery::BooleanWeight::getValue() { return parentQuery->getBoost(); }
	Query* BooleanQuery::BooleanWeight::getQuery() { return (Query*)parentQuery; }





	BooleanQuery::BooleanWeight::BooleanWeight(Searcher* searcher, 
		CLVector<BooleanClause*,Deletor::Object<BooleanClause> >* clauses, BooleanQuery* parentQuery)
	{
		this->searcher = searcher;
		this->parentQuery = parentQuery;
		this->clauses = clauses;
		for (uint32_t i = 0 ; i < clauses->size(); i++) {
			BooleanClause* c = (*clauses)[i];
			weights.put((*clauses)[i]->query->createWeight(searcher));
		}
	}
	BooleanQuery::BooleanWeight::~BooleanWeight(){
		this->weights.clear();
	}

    float_t BooleanQuery::BooleanWeight::sumOfSquaredWeights() {
      float_t sum = 0.0f;
      for (uint32_t i = 0 ; i < weights.size(); i++) {
        BooleanClause* c = (*clauses)[i];
        Weight* w = weights[i];
        if (!c->prohibited)
          sum += w->sumOfSquaredWeights();         // sum sub weights
      }
      sum *= parentQuery->getBoost() * parentQuery->getBoost();             // boost each sub-weight
      return sum ;
    }

    void BooleanQuery::BooleanWeight::normalize(float_t norm) {
      norm *= parentQuery->getBoost();                         // incorporate boost
      for (uint32_t i = 0 ; i < weights.size(); i++) {
        BooleanClause* c = (*clauses)[i];
        Weight* w = weights[i];
        if (!c->prohibited)
          w->normalize(norm);
      }
    }

    Scorer* BooleanQuery::BooleanWeight::scorer(IndexReader* reader){
      // First see if the (faster) ConjunctionScorer will work.  This can be
      // used when all clauses are required.  Also, at this point a
      // BooleanScorer cannot be embedded in a ConjunctionScorer, as the hits
      // from a BooleanScorer are not always sorted by document number (sigh)
      // and hence BooleanScorer cannot implement skipTo() correctly, which is
      // required by ConjunctionScorer.
      bool allRequired = true;
      bool noneBoolean = true;
	  { //msvc6 scope fix
		  for (uint32_t i = 0 ; i < weights.size(); i++) {
			BooleanClause* c = (*clauses)[i];
			if (!c->required)
			  allRequired = false;
			if (c->query->instanceOf(BooleanQuery::getClassName()))
			  noneBoolean = false;
		  }
	  }

      if (allRequired && noneBoolean) {           // ConjunctionScorer is okay
        ConjunctionScorer* result =
          _CLNEW ConjunctionScorer(parentQuery->getSimilarity(searcher));
        for (uint32_t i = 0 ; i < weights.size(); i++) {
          Weight* w = weights[i];
          Scorer* subScorer = w->scorer(reader);
          if (subScorer == NULL)
            return NULL;
          result->add(subScorer);
        }
        return result;
      }

      // Use good-old BooleanScorer instead.
      BooleanScorer* result = _CLNEW BooleanScorer(parentQuery->getSimilarity(searcher));

	  { //msvc6 scope fix
		  for (uint32_t i = 0 ; i < weights.size(); i++) {
			BooleanClause* c = (*clauses)[i];
			Weight* w = weights[i];
			Scorer* subScorer = w->scorer(reader);
			if (subScorer != NULL)
			  result->add(subScorer, c->required, c->prohibited);
			else if (c->required)
			  return NULL;
		  }
	  }

      return result;
    }

	Explanation* BooleanQuery::BooleanWeight::explain(IndexReader* reader, int32_t doc){
      Explanation* sumExpl = _CLNEW Explanation();
      sumExpl->setDescription(_T("sum of:"));
      int32_t coord = 0;
      int32_t maxCoord = 0;
      float_t sum = 0.0f;
      for (uint32_t i = 0 ; i < weights.size(); i++) {
        BooleanClause* c = (*clauses)[i];
        Weight* w = weights[i];
        Explanation* e = w->explain(reader, doc);
        if (!c->prohibited) 
           maxCoord++;
        if (e->getValue() > 0) {
          if (!c->prohibited) {
            sumExpl->addDetail(e);
            sum += e->getValue();
            coord++;
          } else {
            _CLDELETE(e);
            _CLDELETE(sumExpl);
            return _CLNEW Explanation(0.0f, _T("match prohibited"));
          }
        } else if (c->required) {
          _CLDELETE(e);
          _CLDELETE(sumExpl);
          return _CLNEW Explanation(0.0f, _T("match required"));
        }else
          _CLDELETE(e);
      }
      sumExpl->setValue(sum);

      if (coord == 1){                               // only one clause matched
        Explanation** expls = sumExpl->getDetails();
        _CLDELETE(sumExpl);
        sumExpl = expls[0];          // eliminate wrapper
        _CLDELETE_ARRAY(expls);
      }

	  float_t coordFactor = parentQuery->getSimilarity(searcher)->coord(coord, maxCoord);
      if (coordFactor == 1.0f)                      // coord is no-op
        return sumExpl;                             // eliminate wrapper
      else {
        Explanation* result = _CLNEW Explanation();
        result->setDescription( _T("product of:"));
        result->addDetail(sumExpl);

        StringBuffer explbuf;
        explbuf.append(_T("coord("));
        explbuf.appendInt(coord);
        explbuf.append(_T("/"));
        explbuf.appendInt(maxCoord);
        explbuf.append(_T(")"));
        result->addDetail(_CLNEW Explanation(coordFactor, explbuf.getBuffer()));
        result->setValue(sum*coordFactor);
        return result;
      }
    }


CL_NS_END
