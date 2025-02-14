#ifndef _lucene_util_Arrays_
#define _lucene_util_Arrays_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#include "VoidList.h"

CL_NS_DEF(util)
	class Arrays{
	public:
		template<typename _type>
		class _Arrays {
		protected:
			//used by binarySearch to check for equality
			virtual bool equals(_type a,_type b) const = 0; 
			virtual int32_t compare(_type a,_type b) const = 0;
		public:
		   virtual ~_Arrays(){
		   }

			void arraycopy (const _type* src, const int32_t srcStart, _type* dest, const int32_t destStart, const int32_t length) const
			{
				for ( int32_t i=0;i<length;i++ )
					dest[destStart+i] = src[srcStart+i];
			}
			int32_t binarySearch(const _type* a, const int32_t arraylen, const _type key) const
			{
				int32_t low = 0;
				int32_t hi = arraylen - 1;
				int32_t mid = 0;
				while (low <= hi)
				{
					mid = (low + hi) >> 1;

					int32_t c = _tcscmp(a[mid],key);
					if (c==0)
						return mid;
					else if (c > 0)
						hi = mid - 1;
					else // This gets the insertion point right on the last loop.
						low = ++mid;
				}
				return -mid - 1;
			}

			void sort(_type* a, int32_t alen, int32_t fromIndex, int32_t toIndex) const{
				CND_PRECONDITION(fromIndex < toIndex,"fromIndex >= toIndex");
				CND_PRECONDITION(fromIndex >= 0,"fromIndex < 0");
					
					// First presort the array in chunks of length 6 with insertion
				// sort. A mergesort would give too much overhead for this length.
				for (int32_t chunk = fromIndex; chunk < toIndex; chunk += 6)
				{
					int32_t end = min(chunk + 6, toIndex);
					for (int32_t i = chunk + 1; i < end; i++)
					{
						if (compare(a[i - 1], a[i]) > 0)
						{
							// not already sorted
							int32_t j = i;
							_type elem = a[j];
							do
							{
								a[j] = a[j - 1];
								j--;
							}
							while (j > chunk && compare(a[j - 1], elem) > 0);
								a[j] = elem;
						}
					}
				}

				int32_t len = toIndex - fromIndex;
				// If length is smaller or equal 6 we are done.
				if (len <= 6)
					return;

				_type* src = a;
				_type* dest = _CL_NEWARRAY(_type,alen);
				_type* t = NULL; // t is used for swapping src and dest

				// The difference of the fromIndex of the src and dest array.
				int32_t srcDestDiff = -fromIndex;

				// The merges are done in this loop
				for (int32_t size = 6; size < len; size <<= 1)
				{
					for (int32_t start = fromIndex; start < toIndex; start += size << 1)
					{
						// mid is the start of the second sublist;
						// end the start of the next sublist (or end of array).
						int32_t mid = start + size;
						int32_t end = min(toIndex, mid + size);

						// The second list is empty or the elements are already in
						// order - no need to merge
						if (mid >= end || compare(src[mid - 1], src[mid]) <= 0)
						{
							arraycopy(src, start, dest, start + srcDestDiff, end - start);

							// The two halves just need swapping - no need to merge
						}
						else if (compare(src[start], src[end - 1]) > 0)
						{
							arraycopy(src, start,
											dest, end - size + srcDestDiff, size);
							arraycopy(src, mid,
											dest, start + srcDestDiff, end - mid);

						}else{
							// Declare a lot of variables to save repeating
							// calculations.  Hopefully a decent JIT will put these
							// in registers and make this fast
							int32_t p1 = start;
							int32_t p2 = mid;
							int32_t i = start + srcDestDiff;

							// The main merge loop; terminates as soon as either
							// half is ended
							while (p1 < mid && p2 < end)
							{
								dest[i++] = src[(compare(src[p1], src[p2]) <= 0
									? p1++ : p2++)];
							}

							// Finish up by copying the remainder of whichever half
							// wasn't finished.
							if (p1 < mid)
								arraycopy(src, p1, dest, i, mid - p1);
							else
								arraycopy(src, p2, dest, i, end - p2);
						}
					}
					// swap src and dest ready for the next merge
					t = src;
					src = dest;
					dest = t;
					fromIndex += srcDestDiff;
					toIndex += srcDestDiff;
					srcDestDiff = -srcDestDiff;
				}

				// make sure the result ends up back in the right place.  Note
				// that src and dest may have been swapped above, so src
				// contains the sorted array.
				if (src != a)
				{
					// Note that fromIndex == 0.
					arraycopy(src, 0, a, srcDestDiff, toIndex);
				}
			}
		};

		//an string template of the arrays
		class String:public _Arrays<const TCHAR*>{
		public:
			bool equals(const TCHAR* a,const TCHAR* b) const{
				return _tcscmp(a,b)==0;
			}
			int32_t compare(const TCHAR* a,const TCHAR* b) const{
				return _tcscmp(a,b);
			}
			virtual ~String(){
			}
		};
		//an byte template of the arrays
		class Byte:public _Arrays<uint8_t>{
		public:
			bool equals(uint8_t a,uint8_t b) const{
				return a==b;
			}
			int32_t compare(uint8_t a,uint8_t b) const{
				return a-b;
			}
			virtual ~Byte(){
			}
		};
		//an byte template of the arrays
		class Char:public _Arrays<TCHAR>{
		public:
			bool equals(TCHAR a,TCHAR b) const{
				return a==b;
			}
			int32_t compare(TCHAR a,TCHAR b) const{
				return a-b;
			}
			virtual ~Char(){
			}
		};
	};


	
	template <typename _kt, typename _comparator, 
		typename class1, typename class2>
	class CLListEquals:
		public CL_NS_STD(binary_function)<class1*,class2*,bool>
	{
	typedef typename class1::const_iterator _itr1;
	typedef typename class2::const_iterator _itr2;
	public:
		CLListEquals(){
		}
		bool operator()( class1* val1, class2* val2 ) const{
			static _comparator comp; //todo: does this mean it doesn't need to be constructed every time???
			if ( val1 == val2 )
				return true;
			size_t size = val1->size();
			if ( size != val2->size() )
				return false;

			_itr1 itr1 = val1->begin();
			_itr2 itr2 = val2->begin();
			while ( --size >= 0 ){
				if ( !comp(*itr1,*itr2) )
					return false;
				itr1++;
				itr2++;
			}
			return true;
		}
	};
CL_NS_END
#endif
