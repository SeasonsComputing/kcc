#ifndef _lucene_search_Filter_
#define _lucene_search_Filter_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#include "CLucene/index/IndexReader.h"
#include "CLucene/util/BitSet.h"

CL_NS_DEF(search)
  // Abstract base class providing a mechanism to restrict searches to a subset
  // of an index.
  class Filter: LUCENE_BASE {
  public:
    virtual ~Filter(){
	}

    // Returns a BitSet with true for documents which should be permitted in
    //search results, and false for those that should not.
    virtual CL_NS(util)::BitSet* bits(CL_NS(index)::IndexReader* reader)=0;

	//Creates a user-readable version of this query and returns it as as string
	virtual TCHAR* toString()=0;
  };
CL_NS_END
#endif
