#include "CLucene/StdHeader.h"
#include "HitQueue.h"

#include "CLucene/util/PriorityQueue.h"
#include "ScoreDoc.h"

CL_NS_DEF(search)
	

	HitQueue::HitQueue(const int32_t size) {
		initialize(size,true);
	}
	HitQueue::~HitQueue(){
	}

	bool HitQueue::lessThan(ScoreDoc* hitA, ScoreDoc* hitB) {
		//ScoreDoc* hitA = (ScoreDoc*)a;
		//ScoreDoc* hitB = (ScoreDoc*)b;
		if (hitA->score == hitB->score)
			return hitA->doc > hitB->doc; 
		else
			return hitA->score < hitB->score;
	}
CL_NS_END
