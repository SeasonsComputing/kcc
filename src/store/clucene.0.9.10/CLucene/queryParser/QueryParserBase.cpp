#include "CLucene/StdHeader.h"
#include "QueryParserBase.h"

#include "CLucene/search/BooleanClause.h"
#include "CLucene/util/VoidList.h"
#include "CLucene/analysis/AnalysisHeader.h"
#include "CLucene/analysis/Analyzers.h"
#include "CLucene/index/Term.h"
#include "CLucene/search/TermQuery.h"
#include "CLucene/search/PhraseQuery.h"
#include "CLucene/search/RangeQuery.h"
#include "CLucene/search/PrefixQuery.h"
#include "CLucene/search/FuzzyQuery.h"

#include "CLucene/analysis/standard/StandardFilter.h"
#include "CLucene/analysis/standard/StandardTokenizer.h"


CL_NS_USE(search)
CL_NS_USE(util)
CL_NS_USE(analysis)
CL_NS_USE2(analysis,standard)
CL_NS_USE(index)

CL_NS_DEF(queryParser)

	QueryParserBase::QueryParserBase(){
	//Func - Constructor
	//Pre  - true
	//Post - instance has been created with PhraseSlop = 0

		PhraseSlop = 0;
	}

	QueryParserBase::~QueryParserBase(){
	//Func - Destructor
	//Pre  - true
	//Post - The instance has been destroyed
	}

	
	TCHAR* QueryParserBase::discardEscapeChar(const TCHAR* source){
		int len = _tcslen(source);
		TCHAR* dest = _CL_NEWARRAY(TCHAR, len+1);

		int j = 0;
		for (int i = 0; i < len; i++) {
			if ((source[i] != '\\') || (i > 0 && source[i-1] == '\\')) {
				dest[j++]=source[i];
			}
		}
		dest[j]=0;

		return dest;
	}

	

	void QueryParserBase::AddClause(CLVector<BooleanClause*>* clauses, int32_t conj, int32_t mods, Query* q){
	//Func - Adds the next parsed clause.
	//Pre  -
	//Post -

	  bool required;
	  bool prohibited;

	  // If this term is introduced by AND, make the preceding term required,
	  // unless it's already prohibited.
	  if (conj == CONJ_AND) {
		// 2004.11.01: SF 1051984:
		// If attempting to parse a query such as "a AND b", where the first
		// clause is a stopword (and thus, is filtered out), the following code
		// assumed when it reached the second clause that the previous clause
		// had not been filtered out, so it undershot clauses and crashed.
		const uint32_t nPreviousClauses = clauses->size();
		if (nPreviousClauses > 0) {
		  BooleanClause* c = (*clauses)[nPreviousClauses-1];
		  if (!c->prohibited)
			c->required = true;
		}
	  }

	  // We might have been passed a NULL query; the term might have been
	  // filtered away by the analyzer.
	  if (q == NULL)
		return;

	  // We set REQUIRED if we're introduced by AND or +; PROHIBITED if
	  // introduced by NOT or -; make sure not to set both.
	  prohibited = (mods == MOD_NOT);
	  required = (mods == MOD_REQ);
	  if (conj == CONJ_AND && !prohibited)
		required = true;

	  clauses->push_back(_CLNEW BooleanClause(q,true, required, prohibited));
	}

	void QueryParserBase::throwParserException(const TCHAR* message, TCHAR ch, int32_t col, int32_t line )
	{
		TCHAR msg[1024];
		_sntprintf(msg,1024,message,ch,col,line);
		_CLTHROWT (CL_ERR_Parse, msg );
	}

	Query* QueryParserBase::GetFieldQuery(const TCHAR* field, Analyzer* analyzer, const TCHAR* queryText){
	//Func - Returns a query for the specified field.
	//       Use the analyzer to get all the tokens, and then build a TermQuery,
	//       PhraseQuery, or nothing based on the term count
	//Pre  - field != NULL
	//       analyzer contains a valid reference to an Analyzer
	//       queryText != NULL and contains the query
	//Post - A query instance has been returned for the specified field

		CND_PRECONDITION(field != NULL, "field is NULL");
		CND_PRECONDITION(queryText != NULL, "queryText is NULL");

		//Instantiate a stringReader for queryText
		StringReader reader(queryText);

		TokenStream* source = analyzer->tokenStream(field, &reader);

		CND_CONDITION(source != NULL,"source is NULL");

		StringArrayConstWithDeletor v;

		Token t;
		bool ret = false;
		//Get the tokens from the source
		while (true){
		   try{
			   ret = source->next(&t);
		   }catch(...){ //todo: only catch IO Err???
			   ret = false;
		   }

		   if (ret == false)
			  break;

		   v.push_back(discardEscapeChar(t.termText()));
		}

		//Check if there are any tokens retrieved
		if (v.size() == 0){
			_CLDELETE(source);
			return NULL;
		}else{
			if (v.size() == 1){
				Term* t = _CLNEW Term(field, v[0]);
				Query* ret = _CLNEW TermQuery( t );
				_CLDECDELETE(t);

				_CLDELETE(source);
				return ret;
				}
			else{
				PhraseQuery* q = _CLNEW PhraseQuery;
				q->setSlop(PhraseSlop);

				StringArrayConst::iterator itr = v.begin();
				while ( itr != v.end() ){
					const TCHAR* data = *itr;
					Term* t = _CLNEW Term(field, data);
					q->add(t);
					_CLDECDELETE(t);
					itr++;
					}
				_CLDELETE(source);
				return q;
				}
			}
	}

#ifndef NO_RANGE_QUERY

	Query* QueryParserBase::GetRangeQuery(const TCHAR* field, Analyzer* analyzer, const TCHAR* queryText, bool inclusive)
	{
		//todo: this must be fixed, [-1--5] (-1 to -5) should yield a result, but won't parse properly
		//because it uses an analyser, should split it up differently...

	  // Use the analyzer to get all the tokens.  There should be 1 or 2.
	  Reader* reader = _CLNEW StringReader(queryText);
	  TokenStream* source = analyzer->tokenStream(field, reader);

	  Term* terms[2];
	  terms[0]=NULL;terms[1]=NULL;
	  Token t;
	  bool tret=true;

	  bool from=true;
	  while(tret)
	  {
		try{
		  tret = source->next(&t);
		}catch (...){
		  tret=false;
		}
		if (tret)
		{
			if ( !from && _tcscmp(t.termText(),_T("TO"))==0 )
				continue;

			TCHAR* escaped = discardEscapeChar(t.termText());
			terms[from? 0 : 1] = _CLNEW Term(field, escaped);
			_CLDELETE_CARRAY(escaped);
			if (from)
				from = false;
			else
				break;
		}
	  }

	  Query* ret = _CLNEW RangeQuery(terms[0], terms[1], inclusive);
	  _CLDECDELETE(terms[0]);
	  _CLDECDELETE(terms[1]);

	  _CLDELETE(source);
	  _CLDELETE(reader);

	  return ret;
	}

#ifndef NO_PREFIX_QUERY
	Query* QueryParserBase::GetPrefixQuery(const TCHAR* field,const TCHAR* termStr, bool lowercaseWildcardTerms){
	//Func - Factory method for generating a query. Called when parser parses
	//       an input term token that uses prefix notation; that is,
	//       contains a single '*' wildcard character as its last character.
	//       Since this is a special case of generic wildcard term,
	//       and such a query can be optimized easily, this usually results in a different
	//       query object.
	//
	//       Depending on settings, a prefix term may be lower-cased automatically.
	//       It will not go through the default Analyzer, however, since normal Analyzers are
	//       unlikely to work properly with wildcard templates. Can be overridden by extending
	//       classes, to provide custom handling for wild card queries, which may be necessary
	//       due to missing analyzer calls.
	//Pre  - field != NULL and field contains the name of the field that the query will use
	//       termStr != NULL and is the  token to use for building term for the query
	//       (WITH or WITHOUT a trailing '*' character!)
	//Post - A PrefixQuery instance has been returned

		CND_PRECONDITION(field != NULL,"field is NULL");
		CND_PRECONDITION(termStr != NULL,"termStr is NULL");

		TCHAR* queryText = stringDuplicate(termStr);
		int32_t queryTextLen = _tcslen(queryText);
		
		//Check if the last char is a *
		if(queryText[queryTextLen-1] == '*'){
			//remove the *
			queryText[queryTextLen-1] = '\0';
		}

		if ( lowercaseWildcardTerms )
			_tcslwr(queryText);

		TCHAR* queryTextEscaped = discardEscapeChar(queryText);

		Term* t = _CLNEW Term(field, queryTextEscaped);

		CND_CONDITION(t != NULL,"Could not allocate memory for term T");

		Query *q = _CLNEW PrefixQuery(t);

		CND_CONDITION(q != NULL,"Could not allocate memory for PrefixQuery q");

		_CLDECDELETE(t);
		_CLDELETE_ARRAY(queryText);
		_CLDELETE_ARRAY(queryTextEscaped);

		return q;
	}
#endif
#ifndef NO_FUZZY_QUERY
	Query* QueryParserBase::GetFuzzyQuery(const TCHAR* field,const TCHAR* termStr, bool lowercaseWildcardTerms){
	//Func - Factory method for generating a query (similar to getPrefixQuery}). Called when parser parses
	//       an input term token that has the fuzzy suffix (~) appended.
	//Pre  - field != NULL and field contains the name of the field that the query will use
	//       termStr != NULL and is the  token to use for building term for the query
	//       (WITH or WITHOUT a trailing '*' character!)
	//Post - A FuzzyQuery instance has been returned

		CND_PRECONDITION(field != NULL,"field is NULL");
		CND_PRECONDITION(field != NULL,"field is NULL");

		TCHAR* queryText = stringDuplicate(termStr);
		size_t queryTextLen = _tcslen(queryText);

		//Check if the last char is a ~
		if(queryText[queryTextLen-1] == '~'){
			//remove the ~
			queryText[queryTextLen-1] = '\0';
		}

		if ( lowercaseWildcardTerms )
			_tcslwr(queryText);

		TCHAR* queryTextEscaped = discardEscapeChar(queryText);

		Term* t = _CLNEW Term(field, queryTextEscaped);

		CND_CONDITION(t != NULL,"Could not allocate memory for term T");

		Query *q = _CLNEW FuzzyQuery(t);

		CND_CONDITION(q != NULL,"Could not allocate memory for FuzzyQuery q");

		_CLDECDELETE(t);
		_CLDELETE_ARRAY(queryText);
		_CLDELETE_ARRAY(queryTextEscaped);

		return q;
	}
#endif


#endif

CL_NS_END
