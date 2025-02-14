#include "CLucene/StdHeader.h"
#include "MultiReader.h"

#include "IndexReader.h"
#include "CLucene/document/Document.h"
#include "Terms.h"
#include "SegmentMergeQueue.h"

CL_NS_USE(store)
CL_NS_DEF(index)
   MultiReader::MultiReader(IndexReader** subReaders):
      IndexReader(subReaders == NULL ? NULL : subReaders[0]->getDirectory()),
      normsCache(true, true)
   {
      initialize(subReaders);
   }

   MultiReader::MultiReader(Directory* directory, SegmentInfos* sis, bool closeDirectory, IndexReader** subReaders):
      IndexReader(directory, sis, closeDirectory),
      normsCache(true, true)
   {
      initialize(subReaders);
   }


  MultiReader::~MultiReader() {
  //Func - Destructor
  //Pre  - true
  //Post - The instance has been destroyed all IndexReader instances
  //       this instance managed have been destroyed to

    _CLDELETE_ARRAY(starts);

    //Iterate through the subReaders and destroy each reader
      if (subReaders && subReadersLength > 0) {
          for (int32_t i = 0; i < subReadersLength; i++) {
              _CLDELETE(subReaders[i]);
              }
          }


    //Destroy the subReaders array
     _CLDELETE_ARRAY(subReaders);
  }

  void MultiReader::initialize(IndexReader** subReaders){
      this->subReadersLength = 0;
      this->subReaders = subReaders;

      //count the subReaders size
      if ( subReaders != NULL ){
         while ( subReaders[subReadersLength] != NULL ){
            subReadersLength++;
         }
      }
      _maxDoc        = 0;
      _numDocs       = -1;
      

      starts = _CL_NEWARRAY(int32_t,subReadersLength + 1);    // build starts array
      for (int32_t i = 0; i < subReadersLength; i++) {
         starts[i] = _maxDoc;

         // compute maxDocs
         _maxDoc += subReaders[i]->maxDoc();      
         if (subReaders[i]->hasDeletions())
            _hasDeletions = true;
      }
    starts[subReadersLength] = _maxDoc;
  }

  TermFreqVector** MultiReader::getTermFreqVectors(int32_t n){
    int32_t i = readerIndex(n);        // find segment num
    return subReaders[i]->getTermFreqVectors(n - starts[i]); // dispatch to segment
  }

  TermFreqVector* MultiReader::getTermFreqVector(int32_t n, const TCHAR* field){
    int32_t i = readerIndex(n);        // find segment num
    return subReaders[i]->getTermFreqVector(n - starts[i], field);
  }


  int32_t MultiReader::numDocs() {
    SCOPED_LOCK_MUTEX(NumDocs_LOCK);
    if (_numDocs == -1) {			  // check cache
      int32_t n = 0;				  // cache miss--recompute
      for (int32_t i = 0; i < subReadersLength; i++)
        n += subReaders[i]->numDocs();		  // sum from readers
      _numDocs = n;
    }
    return _numDocs;
  }

  int32_t MultiReader::maxDoc() const {
    return _maxDoc;
  }

  CL_NS(document)::Document* MultiReader::document(const int32_t n) {
    int32_t i = readerIndex(n);			  // find segment num
    return subReaders[i]->document(n - starts[i]);	  // dispatch to segment reader
  }

  bool MultiReader::isDeleted(const int32_t n) {
    int32_t i = readerIndex(n);			  // find segment num
    return subReaders[i]->isDeleted(n - starts[i]);	  // dispatch to segment reader
  }

  uint8_t* MultiReader::norms(const TCHAR* field){
	SCOPED_LOCK_MUTEX(norms1_LOCK);
    uint8_t* bytes;
    bytes = normsCache.get(field);
    if (bytes != NULL){
      return bytes;				  // cache hit
    }

    bytes = _CL_NEWARRAY(uint8_t,maxDoc());
    for (int32_t i = 0; i < subReadersLength; i++)
      subReaders[i]->norms(field, bytes, starts[i]);

    //Unfortunately the data in the normCache can get corrupted, since it's being loaded with string
    //keys that may be deleted while still in use by the map. To prevent this field is duplicated
	//and then stored in the normCache
	TCHAR* key = STRDUP_TtoT(field);
    //update cache
    normsCache.put(key, bytes);

    return bytes;
  }

  void MultiReader::norms(const TCHAR* field, uint8_t* result, int32_t offset) {
    SCOPED_LOCK_MUTEX(norms2_LOCK);
    uint8_t* bytes = normsCache.get(field);
    if (bytes != NULL){                            // cache hit
       int32_t len = maxDoc();
       //todo: can we do a memcpy instead, faster??? also other places like this
       for ( int32_t i=0;i<len;i++ )
          result[i+offset]=bytes[i];
    }

    for (int32_t i = 0; i < subReadersLength; i++)      // read from segments
      subReaders[i]->norms(field, result, offset + starts[i]);
  }

  
  void MultiReader::doSetNorm(int32_t n, const TCHAR* field, uint8_t value){
    normsCache.remove(field);                         // clear cache
    int32_t i = readerIndex(n);                           // find segment num
    subReaders[i]->setNorm(n-starts[i], field, value); // dispatch
  }

  TermEnum* MultiReader::terms() const {
    return _CLNEW MultiTermEnum(subReaders, starts, NULL);
  }

  TermEnum* MultiReader::terms(const Term* term) const {
    return _CLNEW MultiTermEnum(subReaders, starts, term);
  }

  int32_t MultiReader::docFreq(const Term* t) const {
    int32_t total = 0;				  // sum freqs in Multi
    for (int32_t i = 0; i < subReadersLength; i++)
      total += subReaders[i]->docFreq(t);
    return total;
  }

  TermDocs* MultiReader::termDocs() const {
    TermDocs* ret =  _CLNEW MultiTermDocs(subReaders, starts);
    return ret;
  }

  TermPositions* MultiReader::termPositions() const {
    TermPositions* ret = (TermPositions*)_CLNEW MultiTermPositions(subReaders, starts);
    return ret;
  }

  void MultiReader::doDelete(const int32_t n) {
    SCOPED_LOCK_MUTEX(doDelete_LOCK);
    _numDocs = -1;				  // invalidate cache
    int32_t i = readerIndex(n);			  // find segment num
    subReaders[i]->deleteDoc(n - starts[i]);		  // dispatch to segment reader
    _hasDeletions = true;
  }

  int32_t MultiReader::readerIndex(const int32_t n) const {	  // find reader for doc n:
    int32_t lo = 0;					   // search starts array
    int32_t hi = subReadersLength - 1;	// for first element less
                                    // than n, return its index
    while (hi >= lo) {
      int32_t mid = (lo + hi) >> 1;
      int32_t midValue = starts[mid];
      if (n < midValue)
        hi = mid - 1;
      else if (n > midValue)
        lo = mid + 1;
      else{                                      // found a match
        while (mid+1 < subReadersLength && starts[mid+1] == midValue) {
          mid++;                                  // scan to last match
        }
        return mid;
      }
    }
    return hi;
  }
  void MultiReader::doUndeleteAll(){
    for (int32_t i = 0; i < subReadersLength; i++)
      subReaders[i]->undeleteAll();
    _hasDeletions = false;
  }
  void MultiReader::doCommit() {
    for (int32_t i = 0; i < subReadersLength; i++)
      subReaders[i]->commit(); //todo: check this... might be a jlucene bug too
  }
  
  void MultiReader::doClose() {
    SCOPED_LOCK_MUTEX(doClose_LOCK);
    for (int32_t i = 0; i < subReadersLength; i++){
      subReaders[i]->close();
    }

   //close the dir (which will should only do a decref)
   //this->getDirectory()->close();
  }


   TCHAR** MultiReader::getFieldNames() {
    // maintain a unique set of field names
    CL_NS(util)::CLSetList<TCHAR*> fieldSet;
    for (int32_t i = 0; i < subReadersLength; i++) {
      IndexReader* reader = subReaders[i];
      TCHAR** names = reader->getFieldNames();
      
      // iterate through the field names and add them to the set
      int32_t j=0;
	  while ( names[j] != NULL ){
	    if ( fieldSet.find(names[j]) == fieldSet.end() )
			fieldSet.insert(names[j]); //steal the name buffer
		else{
			_CLDELETE_CARRAY(names[j]);
		}
		j++;
      }
      _CLDELETE_ARRAY(names);
    }
	TCHAR** ret = _CL_NEWARRAY(TCHAR*,fieldSet.size()+1);
	fieldSet.toArray(ret);
    return ret;
  }

  TCHAR** MultiReader::getFieldNames(bool indexed) {
    // maintain a unique set of field names
    CL_NS(util)::CLSetList<TCHAR*> fieldSet;
    for (int32_t i = 0; i < subReadersLength; i++) {
      IndexReader* reader = subReaders[i];
      TCHAR** names = reader->getFieldNames(indexed);

      // iterate through the field names and add them to the set
      int32_t j=0;
	  while ( names[j] != NULL ){
        if ( fieldSet.find(names[j]) == fieldSet.end() )
			fieldSet.insert(names[j]);
		else{
			_CLDELETE_CARRAY(names[j]);
		}
		j++;
      }
      _CLDELETE_ARRAY(names);
    }
	TCHAR** ret = _CL_NEWARRAY(TCHAR*,fieldSet.size()+1);
	fieldSet.toArray(ret);
    return ret;
  }

  TCHAR** MultiReader::getIndexedFieldNames(bool storedTermVector) {
    // maintain a unique set of field names
    CL_NS(util)::CLSetList<TCHAR*> fieldSet;
    for (int32_t i = 0; i < subReadersLength; i++) {
      IndexReader* reader = subReaders[i];
      TCHAR** names = reader->getIndexedFieldNames(storedTermVector);
      
      // iterate through the field names and add them to the set
      int32_t j=0;
	  while ( names[j] != NULL ){
        if ( fieldSet.find(names[j]) == fieldSet.end() )
			fieldSet.insert(names[j]);
		else{
			_CLDELETE_CARRAY(names[j]);
		}

		j++;
      }
      _CLDELETE_ARRAY(names);
    }
	TCHAR** ret = _CL_NEWARRAY(TCHAR*,fieldSet.size()+1);
	fieldSet.toArray(ret);
    return ret;
  }





  
  MultiTermDocs::MultiTermDocs(){
  //Func - Default constructor
  //       Initialises an empty MultiTermDocs.
  //       This constructor is needed to allow the constructor of MultiTermPositions
  //       initialise the instance by itself
  //Pre  - true
  //Post - An empty

      subReaders       = NULL;
      subReadersLength = 0;
      starts        = NULL;
      base          = 0;
      pointer       = 0;
      current       = NULL;
      term          = NULL;
      readerTermDocs   = NULL;
  }

  MultiTermDocs::MultiTermDocs(IndexReader** r, const int32_t* s){
  //Func - Constructor
  //Pre  - if r is NULL then rLen must be 0 else if r != NULL then rLen > 0
  //       s != NULL
  //Post - The instance has been created

      //count readers
      subReadersLength = 0;
      subReaders       = r;

      CND_PRECONDITION(s != NULL, "s is NULL");

      if ( subReaders != NULL ){
         while ( subReaders[subReadersLength] != NULL )
            subReadersLength++;
      }

      starts        = s;
      base          = 0;
      pointer       = 0;
      current       = NULL;
      term          = NULL;

      readerTermDocs   = NULL;

    //Check if there are subReaders
    if(subReaders != NULL && subReadersLength > 0){
      readerTermDocs = _CL_NEWARRAY(TermDocs*, subReadersLength+1);

      CND_CONDITION(readerTermDocs != NULL,"No memory could be allocated for readerTermDocs");

      //Initialize the readerTermDocs pointer array to NULLs
      for ( int32_t i=0;i<subReadersLength+1;i++){
         readerTermDocs[i]=NULL;
      }
    }
  }

  MultiTermDocs::~MultiTermDocs(){
  //Func - Destructor
  //Pre  - true
  //Post - The instance has been destroyed

      close();
  }

  
  TermPositions* MultiTermDocs::__asTermPositions(){
	  return NULL;
  }

  int32_t MultiTermDocs::doc() const {
    return base + current->doc();
  }
  int32_t MultiTermDocs::freq() const {
    return current->freq();
  }

  void MultiTermDocs::seek(TermEnum* termEnum){
    seek(termEnum->term());
  }

  void MultiTermDocs::seek( Term* tterm) {
  //Func - Resets the instance for a new search
  //Pre  - tterm != NULL
  //Post - The instance has been reset for a new search

    CND_PRECONDITION(tterm != NULL, "tterm is NULL");

      //Assigning tterm is done as below for a reason
      //The construction ensures that if seek is called from within
    //MultiTermDocs with as argument this->term (seek(this->term)) that the assignment
    //will succeed and all referencecounters represent the correct situation

    //Get a pointer from tterm and increase its reference counter
      Term *TempTerm = _CL_POINTER(tterm);

      //Finialize term to ensure we decrease the reference counter of the instance which term points to
      _CLDECDELETE(term);

    //Assign TempTerm to term
      term = TempTerm;

      base = 0;
      pointer = 0;
      current = NULL;
  }

  bool MultiTermDocs::next() {
    if (current != NULL && current->next()) {
      return true;
    } else if (pointer < subReadersLength) {
      base = starts[pointer];
      current = termDocs(pointer++);
      return next();
    } else
      return false;
  }

  int32_t MultiTermDocs::read(int32_t* docs, int32_t* freqs, int32_t length) {
    while (true) {
      while (current == NULL) {
        if (pointer < subReadersLength) {		  // try next segment
          base = starts[pointer];
          current = termDocs(pointer++);
        } else {
          return 0;
        }
      }
      int32_t end = current->read(docs, freqs,length);
      if (end == 0) {				  // none left in segment
        current = NULL;
      } else {					  // got some
        int32_t b = base;			  // adjust doc numbers
        for (int32_t i = 0; i < end; i++)
          docs[i] += b;
        return end;
      }
    }
  }

  bool MultiTermDocs::skipTo(const int32_t target) {
    do {
      if (!next())
        return false;
    } while (target > doc());
    return true;
  }

  void MultiTermDocs::close() {
  //Func - Closes all MultiTermDocs managed by this instance
  //Pre  - true
  //Post - All the MultiTermDocs have been closed


      //Check if readerTermDocs is valid
    if (readerTermDocs){
          TermDocs* curTD = NULL;
          //iterate through the readerTermDocs array
          for (int32_t i = 0; i < subReadersLength; i++) {
            //Retrieve the i-th TermDocs instance
            curTD = readerTermDocs[i];

            //Check if it is a valid pointer
            if (curTD != NULL) {
               //Close it
               curTD->close();
               //Upon deletion, no need to set readerTermDocs[i] = NULL; the entire
               //readerTermDocs array is deleted immediately below, so there's no chance
               //that a member will be referenced.
               _CLVDELETE(curTD); //todo: not a clucene object... should be
            }
         }

      _CLDELETE_ARRAY(readerTermDocs);
        }

      //current previously pointed to a member of readerTermDocs; ensure that
      //it doesn't now point to invalid memory.
      current = NULL;
      base          = 0;
      pointer       = 0;

    _CLDECDELETE(term);
  }

  TermDocs* MultiTermDocs::termDocs(const IndexReader* reader) const {
    TermDocs* ret = reader->termDocs();
    return ret;
  }

  TermDocs* MultiTermDocs::termDocs(const int32_t i) const {
    if (term == NULL)
      return NULL;
    TermDocs* result = readerTermDocs[i];
    if (result == NULL){
      readerTermDocs[i] = termDocs(subReaders[i]);
      result = readerTermDocs[i];
    }
    result->seek(term);

    return result;
  }


  MultiTermEnum::MultiTermEnum(
	  IndexReader** subReaders, const int32_t *starts, const Term* t){
  //Func - Constructor
  //       Opens all enumerations of all readers
  //Pre  - readers != NULL and contains an array of IndexReader instances each responsible for
  //       reading a single segment
  //       subReadersLength >= 0 and represents the number of readers in the readers array
  //       starts is an array of
  //Post - An instance of has been created

  //Pre  - if readers is NULL then subReadersLength must be 0 else if readers != NULL then subReadersLength > 0
  //       s != NULL
  //Post - The instance has been created

       int32_t subReadersLength = 0;
       if ( subReaders != NULL ){
         while ( subReaders[subReadersLength] != NULL )
            subReadersLength++;
       }
      CND_PRECONDITION(starts != NULL,"starts is NULL");

    //Temporary variables
      IndexReader*   reader    = NULL;
      TermEnum* termEnum  = NULL;
      SegmentMergeInfo* smi      = NULL;
	  _docFreq = 0;
	  _term = NULL;
      queue                      = _CLNEW SegmentMergeQueue(subReadersLength);

    CND_CONDITION (queue != NULL, "Could not allocate memory for queue");

    //iterate through all the readers
    subReadersLength = 0;
    while ( subReaders[subReadersLength] != NULL ) {
      //Get the i-th reader
      int32_t i = subReadersLength++;
      reader = subReaders[i];

      //Check if the enumeration must start from term t
      if (t != NULL) {
         //termEnum is an enumeration of terms starting at or after the named term t
         termEnum = reader->terms(t);
      }else{
        //termEnum is an enumeration of all the Terms and TermInfos in the set.
        termEnum = reader->terms();
      }

		//Instantiate an new SegmentMerginfo
		smi = _CLNEW SegmentMergeInfo(starts[i], termEnum, reader);

      // Note that in the call termEnum->getTerm(false) below false is required because
      // otherwise a reference is leaked. By passing false getTerm is
      // ordered to return an unowned reference instead. (Credits for DSR)
      if (t == NULL ? smi->next() : termEnum->term(false) != NULL){
         // initialize queue
         queue->put(smi);
      } else{
         //Close the SegmentMergeInfo
         smi->close();
         //And have it deleted
         _CLDELETE(smi);
      }
    }

    //Check if the queue has elements
    if (t != NULL && queue->size() > 0) {
       next();
    }
  }

  MultiTermEnum::~MultiTermEnum(){
  //Func - Destructor
  //Pre  - true
  //Post - All the resource have been freed and the instance has been deleted

    //Close the enumeration
    close();

    //Delete the queue
    _CLDELETE(queue);
  }

  bool MultiTermEnum::next(){
  //Func - Move the current term to the next in the set of enumerations
  //Pre  - true
  //Post - Returns true if term has been moved to the next in the set of enumerations
  //       Returns false if this was not possible

    SegmentMergeInfo* top = queue->top();
    if (top == NULL) {
        _CLDECDELETE(_term); 
        _term = NULL;
        return false;
    }

    //The getTerm method requires the client programmer to indicate whether he
    // owns the returned reference, so we can discard ours
    // right away.
    _CLDECDELETE(_term); 

	//Assign term the term of top and make sure the reference counter is increased
	_term = _CL_POINTER(top->term);
	_docFreq = 0;
	
	//Find the next term
	while (top != NULL && _term->compareTo(top->term) == 0) {
		//don't delete, this is the top
		queue->pop(); 
		// increment freq
		_docFreq += top->termEnum->docFreq();	  
		if (top->next()){
			// restore queue
			queue->put(top);				  
		}else{
			// done with a segment
			top->close();				  
			_CLDELETE(top);
		}
		top = queue->top();
	}
	
	return true;
}


  Term* MultiTermEnum::term(const bool pointer) {
  //Func - Returns the current term of the set of enumerations
  //Pre  - pointer is true or false and indicates if the reference counter
  //       of term must be increased or not
  //       next() must have been called once!
  //Post - pointer = true -> term has been returned with an increased reference counter
  //       pointer = false -> term has been returned

    if (pointer && _term!=NULL)
        return _CL_POINTER(_term);
    else
		return _term;
  }

  int32_t MultiTermEnum::docFreq() const {
  //Func - Returns the document frequency of the current term in the set
  //Pre  - termInfo != NULL
  //       next() must have been called once
  //Post  - The document frequency of the current enumerated term has been returned

      return _docFreq;
  }


  void MultiTermEnum::close() {
  //Func - Closes the set of enumerations in the queue
  //Pre  - queue holds a valid reference to a SegmentMergeQueue
  //Post - The queue has been closed all SegmentMergeInfo instance have been deleted by
  //       the closing of the queue
  //       term has been finalized and reset to NULL

      // Needed when this enumeration hasn't actually been exhausted yet
      _CLDECDELETE(_term);

    //Close the queue This will destroy all SegmentMergeInfo instances!
      queue->close();

  }





  MultiTermPositions::MultiTermPositions(IndexReader** r, const int32_t* s){
  //Func - Constructor
  //Pre  - if r is NULL then rLen must be 0 else if r != NULL then rLen > 0
  //       s != NULL
  //Post - The instance has been created

      subReaders       = r;
      subReadersLength    = 0;
      if ( subReaders != NULL ){
         while ( subReaders[subReadersLength] != NULL )
            subReadersLength ++ ;
      }

      CND_PRECONDITION(s != NULL, "s is NULL");

      starts        = s;
      base          = 0;
      pointer       = 0;
      current       = NULL;
      term          = NULL;

      readerTermDocs   = NULL;

    //Check if there are readers
    if(subReaders != NULL && subReadersLength > 0){
          readerTermDocs = (TermDocs**)_CL_NEWARRAY(SegmentTermPositions*,subReadersLength);

        CND_CONDITION(readerTermDocs != NULL,"No memory could be allocated for readerTermDocs");

          //Initialize the readerTermDocs pointer array
          for ( int32_t i=0;i<subReadersLength;i++){
              readerTermDocs[i]=NULL;
              }
          }
  }

  
  TermDocs* MultiTermPositions::__asTermDocs(){
	  return (TermDocs*) this;
  }
  TermPositions* MultiTermPositions::__asTermPositions(){
	  return (TermPositions*) this;
  }


  TermDocs* MultiTermPositions::termDocs(const IndexReader* reader) const {
    // Here in the MultiTermPositions class, we want this->current to always
    // be a SegmentTermPositions rather than merely a SegmentTermDocs.
    // To that end, we override the termDocs(IndexReader&) method to produce
    // a SegmentTermPositions via the underlying reader's termPositions method
    // rather merely producing a SegmentTermDocs via the reader's termDocs
    // method.
    
    TermPositions* tp = reader->termPositions();
    TermDocs* ret = tp->__asTermDocs();
    
    CND_CONDITION(ret != NULL,
        "Dynamic downcast in MultiTermPositions::termDocs from"
        " TermPositions to TermDocs failed."
      );
    return ret;
  }

  int32_t MultiTermPositions::nextPosition() {
  //Func -
  //Pre  - current != NULL
  //Post -
    CND_PRECONDITION(current != NULL,"current is NULL");
    
    TermPositions* curAsTP = current->__asTermPositions();
    
    CND_CONDITION(curAsTP != NULL,
        "Dynamic downcast in MultiTermPositions::nextPosition from"
        " SegmentTermDocs to TermPositions failed."
      )
    return curAsTP->nextPosition();
  }


CL_NS_END
