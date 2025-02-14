#include "CLucene/StdHeader.h"
#include "FieldDocSortedHitQueue.h"


CL_NS_USE(util)
CL_NS_DEF(search)

bool FieldDocSortedHitQueue::lessThan (FieldDoc* docA, FieldDoc* docB) {
	int32_t n = fieldsLen;
	int32_t c = 0;
	float_t f1,f2,r1,r2;
	int32_t i1,i2;
	const TCHAR *s1, *s2;

	for (int32_t i=0; i<n && c==0; ++i) {
		int32_t type = fields[i]->getType();
		if (fields[i]->getReverse()) {
			switch (type) {
				case SortField::SCORE:
					r1 = ((Compare::Float*)docA->fields[i])->getValue();
					r2 = ((Compare::Float*)docB->fields[i])->getValue();
					if (r1 < r2) c = -1;
					if (r1 > r2) c = 1;
					break;
				case SortField::DOC:
				case SortField::INT:
					i1 = ((Compare::Int32*)docA->fields[i])->getValue();
					i2 = ((Compare::Int32*)docB->fields[i])->getValue();
					if (i1 > i2) c = -1;
					if (i1 < i2) c = 1;
					break;
				case SortField::STRING:
					s1 = ((Compare::TChar*) docA->fields[i])->getValue();
					s2 = ((Compare::TChar*) docB->fields[i])->getValue();
					if (s2 == NULL) c = -1;      // could be NULL if there are
					else if (s1 == NULL) c = 1;  // no terms in the given field
					else c = _tcscmp(s2,s1); //else if (fields[i].getLocale() == NULL) {

					/*todo: collators not impl
					} else {
						c = collators[i].compare (s2, s1);
					}*/
					break;
				case SortField::FLOAT:
					f1 = ((Compare::Float*)docA->fields[i])->getValue();
					f2 = ((Compare::Float*)docB->fields[i])->getValue();
					if (f1 > f2) c = -1;
					if (f1 < f2) c = 1;
					break;
				case SortField::CUSTOM:
					c = docB->fields[i]->compareTo (docA->fields[i]);
					break;
				case SortField::AUTO:
					// we cannot handle this - even if we determine the type of object (float_t or
					// Integer), we don't necessarily know how to compare them (both SCORE and
					// float_t both contain floats, but are sorted opposite of each other). Before
					// we get here, each AUTO should have been replaced with its actual value.
					_CLTHROWA (CL_ERR_Runtime,"FieldDocSortedHitQueue cannot use an AUTO SortField");
				default:
					_CLTHROWA (CL_ERR_Runtime, "invalid SortField type"); //todo: rich error... : "+type);
			}
		} else {
			switch (type) {
				case SortField::SCORE:
					r1 = ((Compare::Float*)docA->fields[i])->getValue();
					r2 = ((Compare::Float*)docB->fields[i])->getValue();
					if (r1 > r2) c = -1;
					if (r1 < r2) c = 1;
					break;
				case SortField::DOC:
				case SortField::INT:
					i1 = ((Compare::Int32*)docA->fields[i])->getValue();
					i2 = ((Compare::Int32*)docB->fields[i])->getValue();
					if (i1 < i2) c = -1;
					if (i1 > i2) c = 1;
					break;
				case SortField::STRING:
					s1 = ((Compare::TChar*) docA->fields[i])->getValue();
					s2 = ((Compare::TChar*) docB->fields[i])->getValue();
					// NULL values need to be sorted first, because of how FieldCache.getStringIndex()
					// works - in that routine, any documents without a value in the given field are
					// put first.
					if (s1 == NULL) c = -1;      // could be NULL if there are
					else if (s2 == NULL) c = 1;  // no terms in the given field
					else c = _tcscmp(s1,s2); //else if (fields[i].getLocale() == NULL) {
					
					/* todo: collators not implemented } else {
						c = collators[i].compare (s1, s2);
					}*/
					break;
				case SortField::FLOAT:
					f1 = ((Compare::Float*)docA->fields[i])->getValue();
					f2 = ((Compare::Float*)docB->fields[i])->getValue();
					if (f1 < f2) c = -1;
					if (f1 > f2) c = 1;
					break;
				case SortField::CUSTOM:
					c = docA->fields[i]->compareTo (docB->fields[i]);
					break;
				case SortField::AUTO:
					// we cannot handle this - even if we determine the type of object (float_t or
					// Integer), we don't necessarily know how to compare them (both SCORE and
					// float_t both contain floats, but are sorted opposite of each other). Before
					// we get here, each AUTO should have been replaced with its actual value.
					_CLTHROWA (CL_ERR_Runtime,"FieldDocSortedHitQueue cannot use an AUTO SortField");
				default:
					_CLTHROWA (CL_ERR_Runtime,"invalid SortField type"); //todo: rich error... : "+type);
			}
		}
	}
	return c > 0;
}

void FieldDocSortedHitQueue::setFields (SortField** fields) {
	SCOPED_LOCK_MUTEX(setFields_LOCK);
	if (this->fields == NULL) {
		this->fields = fields;
		_countsize();
		//this->collators = hasCollators (fields);
	}
}

FieldDocSortedHitQueue::~FieldDocSortedHitQueue(){
}

	
CL_NS_END

