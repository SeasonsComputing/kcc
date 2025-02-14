#include "CLucene/StdHeader.h"
#include "SegmentHeader.h"

#include "Terms.h"

CL_NS_USE(util)
CL_NS_DEF(index)

  SegmentTermPositions::SegmentTermPositions(SegmentReader* Parent):
      SegmentTermDocs(Parent){
  //Func - Constructor
  //Pre  - Parent != NULL
  //Post - The instance has been created

       CND_PRECONDITION(Parent != NULL, "Parent is NULL");

       proxStream = ((SegmentReader*)parent)->proxStream->clone();

       CND_CONDITION(proxStream != NULL,"proxStream is NULL");

       position  = 0;
       proxCount = 0;
  }

  SegmentTermPositions::~SegmentTermPositions() {
  //Func - Destructor
  //Pre  - true
  //Post - The intance has been closed

      close();
  }

  TermDocs* SegmentTermPositions::__asTermDocs(){
	  return (TermDocs*) this;
  }
  TermPositions* SegmentTermPositions::__asTermPositions(){
	  return (TermPositions*) this;
  }

  void SegmentTermPositions::seek(TermInfo* ti) {
    SegmentTermDocs::seek(ti);
    if (ti != NULL)
      proxStream->seek(ti->proxPointer);
    //bvk: jlucene changes this
    //else
    proxCount = 0;
  }

  void SegmentTermPositions::close() {
  //Func - Frees the resources
  //Pre  - true
  //Post - The resources  have been freed

      SegmentTermDocs::close();
	  //Check if proxStream still exists
	  if(proxStream){
          proxStream->close();         
          _CLDELETE( proxStream );
	      }
  }

  int32_t SegmentTermPositions::nextPosition() {
    /* DSR:CL_BUG: Should raise exception if proxCount == 0 at the
    ** beginning of this method, as in
    **   if (--proxCount == 0) throw ...;
    ** The JavaDocs for TermPositions.nextPosition declare this constraint,
    ** but CLucene doesn't enforce it. */
    proxCount--;
    return position += proxStream->readVInt();
  }

  bool SegmentTermPositions::next() {
    for (int32_t f = proxCount; f > 0; f--)		  // skip unread positions
      proxStream->readVInt();

    if (SegmentTermDocs::next()) {				  // run super
      proxCount = _freq;				  // note frequency
      position = 0;				  // reset position
      return true;
    }
    return false;
  }

  int32_t SegmentTermPositions::read(int32_t* docs, int32_t* freqs, int32_t length) {
    _CLTHROWA(CL_ERR_InvalidState,"TermPositions does not support processing multiple documents in one call. Use TermDocs instead.");
  }

  ///\todo hack!!!
  void SegmentTermPositions::seek(Term* term){
    //todo: hack!!!
    //supposed to call base class which calls seek(terminfo) of this class
    //how to get around this???

    TermInfo* ti = ((SegmentReader*)parent)->tis->get(term);
    seek(ti);
    _CLDELETE( ti );
  }
  
  void SegmentTermPositions::seek(TermEnum* termEnum){
    TermInfo* ti=NULL;
    //todo: hack!!!
    //supposed to call base class which calls seek(terminfo) of this class
    //but base class is not calling this class (calls it's own implementation) - grrr!!!
    //SegmentTermDocs::seek(term); (used to be just this line)

    // use comparison of fieldinfos to verify that termEnum belongs to the same segment as this SegmentTermDocs
    try{
      SegmentTermEnum* te = (SegmentTermEnum*)termEnum;
      te->fieldInfos = parent->fieldInfos;
      ti = te->getTermInfo();
    }catch(...){
      ti = parent->tis->get(termEnum->term(false));//todo: check this
    }
    seek(ti);
	_CLDELETE(ti);
  }
  int32_t SegmentTermPositions::doc() const{
    return SegmentTermDocs::doc();
  }
  int32_t SegmentTermPositions::freq() const{
    return SegmentTermDocs::freq();
  }

/*  int32_t SegmentTermPositions::read(int32_t* docs, int32_t* freqs, int32_t length){
    return SegmentTermDocs::read(docs,freqs,length);
  };*/
  
  bool SegmentTermPositions::skipTo(const int32_t target){
    return SegmentTermDocs::skipTo(target);
  }

  void SegmentTermPositions::skippingDoc() {
    for (int32_t f = _freq; f > 0; f--)		  // skip all positions
      proxStream->readVInt();
  }

  void SegmentTermPositions::skipProx(int64_t proxPointer){
    proxStream->seek(proxPointer);
    proxCount = 0;
  }

CL_NS_END
