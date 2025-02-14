#ifndef _lucene_util_PriorityQueue_
#define _lucene_util_PriorityQueue_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif
CL_NS_DEF(util)

// A PriorityQueue maintains a partial ordering of its elements such that the
// least element can always be found in constant time.  Put()'s and pop()'s
//  require log(size) time. 
template <class _type,typename _valueDeletor> class PriorityQueue:LUCENE_BASE {
	private:
		_type* heap; //(was object[])
		size_t _size;
		bool dk;
		size_t maxSize;

		void upHeap(){
			size_t i = _size;
			_type node = heap[i];			  // save bottom node (WAS object)
			int32_t j = ((uint32_t)i) >> 1;
			while (j > 0 && lessThan(node,heap[j])) {
				heap[i] = heap[j];			  // shift parents down
				i = j;
				j = ((uint32_t)j) >> 1;
			}
			heap[i] = node;				  // install saved node
		}
		void downHeap(){
			size_t i = 1;
			_type node = heap[i];			  // save top node
			size_t j = i << 1;				  // find smaller child
			size_t k = j + 1;
			if (k <= _size && lessThan(heap[k], heap[j])) {
				j = k;
			}
			while (j <= _size && lessThan(heap[j],node)) {
				heap[i] = heap[j];			  // shift up child
				i = j;
				j = i << 1;
				k = j + 1;
				if (k <= _size && lessThan(heap[k], heap[j])) {
					j = k;
				}
			}
			heap[i] = node;				  // install saved node
		}

	protected:
		PriorityQueue(){
			this->_size = 0;
			this->dk = false;
			this->heap = NULL;
            this->maxSize = 0;
		}

		// Determines the ordering of objects in this priority queue.  Subclasses
		//	must define this one method. 
		virtual bool lessThan(_type a, _type b)=0;

		// Subclass constructors must call this. 
		void initialize(const int32_t maxSize, bool deleteOnClear){
			_size = 0;
			dk = deleteOnClear;
			int32_t heapSize = maxSize + 1;
			heap = _CL_NEWARRAY(_type,heapSize);
            this->maxSize = maxSize;
		}

	public:
		virtual ~PriorityQueue(){
			clear();
			_CLDELETE_ARRAY(heap);
		}

	 /**
      * Adds an Object to a PriorityQueue in log(size) time.
      * If one tries to add more objects than maxSize from initialize
      * a RuntimeException (ArrayIndexOutOfBound) is thrown.
      */
      void put(_type element){
			CND_PRECONDITION(_size<maxSize,"add is out of bounds");

			_size++;	
			heap[_size] = element;		
			upHeap();
		}

      /**
      * Adds element to the PriorityQueue in log(size) time if either
      * the PriorityQueue is not full, or not lessThan(element, top()).
      * @param element
      * @return true if element is added, false otherwise.
      */
      bool insert(_type element){
         if(_size < maxSize){
            put(element);
            return true;
        }else if(_size > 0 && !lessThan(element, top())){
			if ( dk ){
				_valueDeletor::doDelete(heap[1]);
			}
            heap[1] = element;
            adjustTop();
            return true;
         }else
            return false;
      }

		/**
		* Returns the least element of the PriorityQueue in constant time. 
		*/
		_type top(){
			if (_size > 0)
				return heap[1];
			else
				return NULL;
		}

		/** Removes and returns the least element of the PriorityQueue in log(size)
		*	time.  
		*/
		_type pop(){
			if (_size > 0) {
				_type result = heap[1];			  // save first value
				heap[1] = heap[_size];			  // move last to first

				heap[_size] = (_type)0;			  // permit GC of objects
				_size--;
				downHeap();				  // adjust heap
				return result;
			} else
				return (_type)NULL;
		}

		/**Should be called when the object at top changes values.  Still log(n)
		   worst case, but it's at least twice as fast to <pre>
		    { pq.top().change(); pq.adjustTop(); }
		   </pre> instead of <pre>
		    { o = pq.pop(); o.change(); pq.push(o); }
		   </pre>
		*/
		void adjustTop(){
			downHeap();
		}
		    

		/**
		* Returns the number of elements currently stored in the PriorityQueue.
		*/ 
		size_t size(){
			return _size;
		}
		  
		/** 
		* Removes all entries from the PriorityQueue. 
		*/
		void clear(){
			for (size_t i = 1; i <= _size; i++){
				if ( dk ){
					_valueDeletor::doDelete(heap[i]);
				}
			}
			_size = 0;
		}
	};

CL_NS_END
#endif
