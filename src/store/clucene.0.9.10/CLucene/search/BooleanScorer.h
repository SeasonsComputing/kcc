#ifndef _lucene_search_BooleanScorer_
#define _lucene_search_BooleanScorer_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#include "Scorer.h"

CL_NS_DEF(search)
	
	class BooleanScorer: public Scorer {
	private:
			
		class Bucket: LUCENE_BASE {
		public:
			int32_t	doc;				  // tells if bucket is valid
			float_t	score;				  // incremental score
			int32_t	bits;					  // used for bool constraints
			int32_t	coord;					  // count of terms in score
			Bucket*	next;				  // next valid bucket

			Bucket();
			~Bucket();
		};

		class SubScorer: LUCENE_BASE {
		public:
		bool done;
				Scorer* scorer;
				bool required;
				bool prohibited;
				HitCollector* collector;
				SubScorer* next;
				SubScorer(Scorer* scr, const bool r, const bool p, HitCollector* c, SubScorer* nxt);
				~SubScorer();
		};

		class BucketTable:LUCENE_BASE {		
		private:
			BooleanScorer* scorer;
		public:
			Bucket* buckets;

			BucketTable(BooleanScorer* scr);
			//void collectHits(HitCollector* results);
			int32_t size() const;
			HitCollector* newCollector(const int32_t mask);

			Bucket* first;			  // head of valid list

			void clear();
			~BucketTable();
	      
		};

		class Collector: public HitCollector {
		private:
			BucketTable* bucketTable;
			int32_t mask;
		public:
			Collector(const int32_t mask, BucketTable* bucketTable);
			
			void collect(const int32_t doc, const float_t score);
		};

		SubScorer* scorers;
		BucketTable* bucketTable;

		int32_t maxCoord;
		int32_t nextMask;
	public:
		LUCENE_STATIC_CONSTANT(int32_t,BucketTable_SIZE=1024);
		int32_t requiredMask;
		int32_t prohibitedMask;
		float_t* coordFactors;

    BooleanScorer(Similarity* similarity);
		~BooleanScorer();
		void add(Scorer* scorer, const bool required, const bool prohibited);

	private:
		void computeCoordFactors();
		//void score(HitCollector* results, const int32_t maxDoc);

      int32_t end;
		Bucket* current;

	public:
		int32_t doc() { return current->doc; }

		bool next();

		float_t score();
		bool skipTo(int32_t target);
		Explanation* explain(int32_t doc);
		TCHAR* toString();
	};







CL_NS_END
#endif
