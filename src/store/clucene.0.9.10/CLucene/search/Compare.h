#ifndef _lucene_search_Compare_
#define _lucene_search_Compare_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#include "FieldSortedHitQueue.h"

CL_NS_DEF(search)


class ScoreDocComparators:LUCENE_BASE {
public:
    ~ScoreDocComparators(){
    }

	class Relevance:public ScoreDocComparator {
	public:
		int32_t compare (ScoreDoc* i, ScoreDoc* j) {
			if (i->score > j->score) return -1;
			if (i->score < j->score) return 1;
			return 0;
		}
		CL_NS(util)::Comparable* sortValue (ScoreDoc* i) {
			return _CLNEW CL_NS(util)::Compare::Float (i->score);
		}
		int32_t sortType() {
			return SortField::SCORE;
		}
	};

	class IndexOrder:public ScoreDocComparator{
	public:
		IndexOrder():
			ScoreDocComparator()
		{

		}
		int32_t compare (ScoreDoc* i, ScoreDoc* j) {
			if (i->doc < j->doc) return -1;
			if (i->doc > j->doc) return 1;
			return 0;
		}
		CL_NS(util)::Comparable* sortValue (ScoreDoc* i) {
			return _CLNEW CL_NS(util)::Compare::Int32(i->doc);
		}
		int32_t sortType() {
			return SortField::DOC;
		}
	};


	class String: public ScoreDocComparator {
		FieldCache::StringIndex* index;
#ifdef _CL__CND_DEBUG
		int32_t length;
#endif
	public:
		String(FieldCache::StringIndex* index, int32_t len)
		{
#ifdef _CL__CND_DEBUG
			this->length = len;
#endif
			this->index = index;
		}

		int32_t compare (ScoreDoc* i, ScoreDoc* j) {
			CND_PRECONDITION(i->doc<length, "i->doc>=length")
			CND_PRECONDITION(j->doc<length, "j->doc>=length")
			int32_t fi = index->order[i->doc];
			int32_t fj = index->order[j->doc];
			if (fi < fj) 
				return -1;
			if (fi > fj) 
				return 1;
			return 0;
		}

		CL_NS(util)::Comparable* sortValue (ScoreDoc* i) {
			return _CLNEW CL_NS(util)::Compare::TChar(index->lookup[index->order[i->doc]]);
		}

		int32_t sortType() {
			return SortField::STRING;
		}
	};

	class Int32:public ScoreDocComparator{
		int32_t* fieldOrder;
#ifdef _CL__CND_DEBUG
		int32_t length;
#endif
	public:
		Int32(int32_t* fieldOrder, int32_t len)
		{
			this->fieldOrder = fieldOrder;
#ifdef _CL__CND_DEBUG
			this->length = len;
#endif
		}


		int32_t compare (ScoreDoc* i, ScoreDoc* j) {
			CND_PRECONDITION(i->doc<length, "i->doc>=length")
			CND_PRECONDITION(j->doc<length, "j->doc>=length")
			int32_t fi = fieldOrder[i->doc];
			int32_t fj = fieldOrder[j->doc];
			if (fi < fj) return -1;
			if (fi > fj) return 1;
			return 0;
		}

		CL_NS(util)::Comparable* sortValue (ScoreDoc* i) {
			CND_PRECONDITION(i->doc<length, "i->doc>=length")
			return _CLNEW CL_NS(util)::Compare::Int32(fieldOrder[i->doc]);
		}

		int32_t sortType() {
			return SortField::INT;
		}
	};

	class Float:public ScoreDocComparator {
		float_t* fieldOrder;
#ifdef _CL__CND_DEBUG
		int32_t length;
#endif
	public:
		Float(float_t* fieldOrder, int32_t len)
		{
			this->fieldOrder = fieldOrder;
#ifdef _CL__CND_DEBUG
			this->length = len;
#endif
		}

		int32_t compare (ScoreDoc* i, ScoreDoc* j) {
			CND_PRECONDITION(i->doc<length, "i->doc>=length")
			CND_PRECONDITION(j->doc<length, "j->doc>=length")
			float_t fi = fieldOrder[i->doc];
			float_t fj = fieldOrder[j->doc];
			if (fi < fj) return -1;
			if (fi > fj) return 1;
			return 0;
		}

		CL_NS(util)::Comparable* sortValue (ScoreDoc* i) {
			CND_PRECONDITION(i->doc<length, "i->doc>=length")
			return _CLNEW CL_NS(util)::Compare::Float(fieldOrder[i->doc]);
		}

		int32_t sortType() {
			return SortField::FLOAT;
		}
	};
};


CL_NS_END
#endif
