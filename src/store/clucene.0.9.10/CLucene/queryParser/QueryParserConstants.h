#ifndef _lucene_queryParser_QueryParserConstants_
#define _lucene_queryParser_QueryParserConstants_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

CL_NS_DEF(queryParser)

	enum QueryTokenTypes
	{
		AND_,
		OR,
		NOT,
		PLUS,
		MINUS,
		LPAREN,
		RPAREN,
		COLON,
		CARAT,
		QUOTED,
		TERM,
		SLOP,
#ifndef NO_FUZZY_QUERY
		FUZZY,
#endif
#ifndef NO_PREFIX_QUERY
		PREFIXTERM,
#endif
#ifndef NO_WILDCARD_QUERY
		WILDTERM,
#endif
#ifndef NO_RANGE_QUERY
		RANGEIN,
		RANGEEX,
#endif
		NUMBER,
		EOF_
	};

CL_NS_END
#endif
