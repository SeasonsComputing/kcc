#include "CLucene/StdHeader.h"
#include "SegmentHeader.h"

#include "CLucene/util/BitVector.h"
#include "CLucene/store/InputStream.h"
#include "Term.h"

CL_NS_DEF(index)

  SegmentTermDocs::SegmentTermDocs(SegmentReader* Parent){
  //Func - Constructor
  //Pre  - Paren != NULL
  //Post - The instance has been created

      CND_PRECONDITION(Parent != NULL,"Parent is NULL");

      parent      = Parent;
      deletedDocs =  parent->deletedDocs;

      _doc         = 0;
      _freq        = 0;
	  count =		 0;
	  df		   = 0;

      skipInterval=0;
      numSkips=0;
      skipCount=0;
      skipStream=NULL;
      skipDoc=0;
      freqPointer=0;
      proxPointer=0;
      skipPointer=0;
      haveSkipped=false;

      freqStream  = parent->freqStream->clone();
      skipInterval = parent->tis->getSkipInterval();
   }

  SegmentTermDocs::~SegmentTermDocs() {
  //Func - Destructor
  //Pre  - true
  //Post - The instance has been destroyed

      close();
  }

  TermPositions* SegmentTermDocs::__asTermPositions(){
	  return NULL;
  }

  void SegmentTermDocs::seek(Term* term) {
    TermInfo* ti = parent->tis->get(term);
    seek(ti);
    _CLDELETE(ti);
  }

  void SegmentTermDocs::seek(TermEnum* termEnum){
    TermInfo* ti=NULL;
    
    // use comparison of fieldinfos to verify that termEnum belongs to the same segment as this SegmentTermDocs
    try{
      //todo: this is a really dodgy way of doing this
      //we try and convert the Term Enum to a segment term enum... if its
      //not a segment term enum, then we throw and assume it is not a segmenttermenum
      //should have a way of knowing what type the TermEnum is...
      SegmentTermEnum* te = (SegmentTermEnum*)termEnum;
      te->fieldInfos = parent->fieldInfos;
      ti = te->getTermInfo();
    }catch(...){
      ti = parent->tis->get(termEnum->term(false));//todo: check this
    }
    
    seek(ti);
	_CLDELETE(ti);
  }
  void SegmentTermDocs::seek(const TermInfo* ti) {
     count = 0;
    if (ti == NULL) {
      df = 0;
    } else {
      df = ti->docFreq;
      _doc = 0;
      skipDoc = 0;
      skipCount = 0;
      numSkips = df / skipInterval;
      freqPointer = ti->freqPointer;
      proxPointer = ti->proxPointer;
      skipPointer = freqPointer + ti->skipOffset;
      freqStream->seek(freqPointer);
      haveSkipped = false;
    }
  }

  void SegmentTermDocs::close() {

      //Check if freqStream still exists
	  if (freqStream != NULL){
      freqStream->close();
      _CLDELETE( freqStream );
	  }
     if (skipStream != NULL){
      skipStream->close();
      _CLDELETE( skipStream );
     }
  }

  int32_t SegmentTermDocs::doc()const { return _doc; }
  int32_t SegmentTermDocs::freq()const { return _freq; }

  bool SegmentTermDocs::next() {
    while (true) {
      if (count == df)
        return false;

      const int32_t docCode = freqStream->readVInt();
      _doc += (uint32_t)docCode >> 1; //unsigned shift
      if ((docCode & 1) != 0)			  // if low bit is set
        _freq = 1;				  // _freq is one
      else
        _freq = freqStream->readVInt();		  // else read _freq
      count++;

      if ( (deletedDocs == NULL) || (deletedDocs->get(_doc) == false ) )
        break;
      skippingDoc();
    }
    return true;
  }

  int32_t SegmentTermDocs::read(int32_t* docs, int32_t* freqs, int32_t length) {
    int32_t i = 0;

	//todo: use optimized mmap implementation like jlucene GCJ
    while (i<length && count < df) {
      const int32_t docCode = freqStream->readVInt();
      _doc += (uint32_t)docCode >> 1;
      if ((docCode & 1) != 0)			  // if low bit is set
        _freq = 1;				  // _freq is one
      else
        _freq = freqStream->readVInt();		  // else read _freq
      count++;

      if (deletedDocs == NULL || !deletedDocs->get(_doc)) {
        docs[i] = _doc;
        freqs[i] = _freq;
        i++;
      }
    }
    return i;
  }

  bool SegmentTermDocs::skipTo(const int32_t target){
    if (df >= skipInterval) {                      // optimized case
      if (skipStream == NULL)
         skipStream = (CL_NS(store)::InputStream*) freqStream->clone(); // lazily clone

      if (!haveSkipped) {                          // lazily seek skip stream
        skipStream->seek(skipPointer);
        haveSkipped = true;
      }

      // scan skip data
      int32_t lastSkipDoc = skipDoc;
      int64_t lastFreqPointer = freqStream->getFilePointer();
      int64_t lastProxPointer = -1;
      int32_t numSkipped = -1 - (count % skipInterval);

      while (target > skipDoc) {
        lastSkipDoc = skipDoc;
        lastFreqPointer = freqPointer;
        lastProxPointer = proxPointer;
        
        if (skipDoc != 0 && skipDoc >= _doc)
          numSkipped += skipInterval;
        
        if(skipCount >= numSkips)
          break;

        skipDoc += skipStream->readVInt();
        freqPointer += skipStream->readVInt();
        proxPointer += skipStream->readVInt();

        skipCount++;
      }
      
      // if we found something to skip, then skip it
      if (lastFreqPointer > freqStream->getFilePointer()) {
        freqStream->seek(lastFreqPointer);
        skipProx(lastProxPointer);

        _doc = lastSkipDoc;
        count += numSkipped;
      }

    }

    // done skipping, now just scan

    do {
      if (!next())
        return false;
    } while (target > _doc);
    return true;
  }


CL_NS_END
