#ifndef _lucene_search_PrefixQuery
#define _lucene_search_PrefixQuery
#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#ifndef NO_PREFIX_QUERY

#include "CLucene/index/Term.h"
#include "CLucene/index/Terms.h"
#include "CLucene/index/IndexReader.h"
#include "SearchHeader.h"
#include "BooleanQuery.h"
#include "TermQuery.h"
#include "CLucene/util/StringBuffer.h"

CL_NS_DEF(search) 
	//PrefixQuery is a Query that matches documents containing terms with a specified prefix.

	class PrefixQuery: public Query {
	private:
		CL_NS(index)::Term* prefix;
	protected:
		PrefixQuery(const PrefixQuery& clone);
	public:

		//Constructor. Constructs a query for terms starting with prefix
		PrefixQuery(CL_NS(index)::Term* Prefix);

		//Destructor
		~PrefixQuery();

		//Returns the name "PrefixQuery"
		const TCHAR* getQueryName()const;
		static const TCHAR* getClassName();

		/** Returns the prefix of this query. */
		CL_NS(index)::Term* getPrefix() { return _CL_POINTER(prefix); }

		Query* combine(Query** queries);
		Query* rewrite(CL_NS(index)::IndexReader* reader);
		Query* clone();
		bool equals(Query * other) const;

		//Creates a user-readable version of this query and returns it as as string
		TCHAR* toString(const TCHAR* field) ;

		size_t hashCode() const;
	};
CL_NS_END
#endif
#endif
