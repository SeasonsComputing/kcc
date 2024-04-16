#include "CLucene/StdHeader.h"
#include "TermInfosReader.h"

#include "Term.h"
#include "Terms.h"
#include "SegmentTermEnum.h"
#include "CLucene/store/Directory.h"
#include "FieldInfos.h"
#include "TermInfo.h"
#include "TermInfosWriter.h"
#include "CLucene/util/Misc.h"
#include "CLucene/util/Arrays.h"

CL_NS_USE(store)
CL_NS_USE(util)
CL_NS_DEF(index)


  TermInfosReader::TermInfosReader(Directory* dir, const char* seg, FieldInfos* fis):
      directory (dir),fieldInfos (fis){
  //Func - Constructor.
  //       Reads the TermInfos file (.tis) and eventually the Term Info Index file (.tii)
  //Pre  - dir is a reference to a valid Directory 
  //       Fis contains a valid reference to an FieldInfos instance
  //       seg != NULL and contains the name of the segment
  //Post - An instance has been created and the index named seg has been read. (Remember
  //       a segment is nothing more then an independently readable index)
	    
      CND_PRECONDITION(seg != NULL, "seg is NULL");

	  //Initialize the name of the segment
      segment    =  seg;
      //There are no indexTerms yet
      indexTerms    = NULL;
	  //So there are no indexInfos
	  indexInfos    = NULL;
	  //So there are no indexPointers
	  indexPointers = NULL; 	
      //Create a filname fo a Term Info File
	  char* TermInfosFile = Misc::segmentname(segment,".tis");

      //Create an SegmentTermEnum for storing all the terms read of the segment
      enumerator = _CLNEW SegmentTermEnum( directory->openFile( TermInfosFile ), fieldInfos, false);

	  //Check if enumerator points to a valid instance
      CND_CONDITION(enumerator != NULL, "No memory could be allocated for enumerator");

      //TermInfosFile is not needed anymore
      _CLDELETE_LCaARRAY(TermInfosFile);

      //Get the size of the enumeration and store it in size
      size =  enumerator->size;
		
      //Read the segment
      readIndex();
  }

  TermInfosReader::~TermInfosReader(){
  //Func - Destructor
  //Pre  - true
  //Post - The instance has been destroyed

      //Close the TermInfosReader to be absolutly sure that enumerator has been closed
	  //and the arrays indexTerms, indexPointers and indexInfos and  their elements 
	  //have been destroyed
      close();
  }


  void TermInfosReader::close() {
  //Func - Close the enumeration of TermInfos
  //Pre  - true
  //Post - The _enumeration has been closed and the arrays

	  //Check if indexTerms and indexInfos exist
     if (indexTerms && indexInfos){
          //Iterate through arrays indexTerms and indexPointer to
	      //destroy their elements
#ifdef _DEBUG
         for ( int32_t i=0; i<indexTermsLength;i++ ){
			 if ( indexTerms[i].__cl_refcount != 1 )
				 CND_PRECONDITION(indexTerms[i].__cl_refcount==1,"TermInfosReader term was references more than internally");
         //   _CLDECDELETE(indexTerms[i]);
            //_CLDELETE(indexInfos[i]);
         }
#endif
         //Delete the arrays
         _CLDELETE_ARRAY(indexTerms);
         _CLDELETE_ARRAY(indexInfos);
     }

      //Delete the arrays
      _CLDELETE_ARRAY(indexPointers);

      if (enumerator != NULL){
        enumerator->close();

	    //Get a pointer to InputStream used by the enumeration but 
	    //instantiated in the constructor by directory.open( TermInfosFile )
        InputStream *Is = enumerator->input;

        //Delete the enumuration enumerator
        _CLDELETE(enumerator);
        //Delete the InputStream 
        _CLDELETE(Is);	
      }
  }

  int64_t TermInfosReader::Size() {
  //Func - Return the size of the enumeration of TermInfos
  //Pre  - true
  //Post - size has been returened

      return size;
  }


  Term* TermInfosReader::get(const int32_t position) {
  //Func - Returns the nth term in the set
  //Pre  - position > = 0
  //Post - The n-th term in the set has been returned

      SCOPED_LOCK_MUTEX(getInt_LOCK);
      
      CND_PRECONDITION(position >= 0,"position contains a negative number");

      Term* ret = NULL;
	
      
	  //Check if the size is 0 because then there are no terms
      if (size == 0) 
          return NULL;
    
      //if
      if ( 
          //an enumeration exists
          enumerator != NULL && 
          //which has a current term
          enumerator->term(false) != NULL && 
          //where the position of the request term is larger or equal to current position
          //in the enumeration
          position >= enumerator->position &&
          //but where the position of the requested term  is smaller than the 
          //last one in the current enumeration
		  position < (enumerator->position + enumerator->getIndexInterval() )
		 ){

          //Get the Term at position
          ret = scanEnum(position);

          //Seek can be avoided so return the found term
          return ret;		  
		 }

    //Reposition
    seekEnum(position / enumerator->skipInterval); 

	//Get the Term at position
    ret = scanEnum(position);

	//Return the found term
    return ret;
  }

  TermInfo* TermInfosReader::get(const Term* term){
  //Func - Returns a TermInfo for a term
  //Pre  - term holds a valid reference to term
  //Post - if term can be found its TermInfo has been returned otherwise NULL

    SCOPED_LOCK_MUTEX(getTerm_LOCK);
    
    //If the size of the enumeration is 0 then no Terms have been read
	if (size == 0){      
		return NULL;
	}

	TermInfo* ret;

    // optimize sequential access: first try scanning cached enumerator w/o seeking

    //if
    if (
	      //the current term of the enumeration enumerator is not at the end AND
      	enumerator->term(false) != NULL	 && 
      	(
            //there exists a previous current called prev and term is positioned after this prev OR
            ( enumerator->prev != NULL && term->compareTo(enumerator->prev) > 0) || 
            //term is positioned at the same position as the current of enumerator or at a higher position
            term->compareTo(enumerator->term(false)) >= 0 )
      	)
     {

		//Calculate the offset for the position
		int64_t _enumOffset = (enumerator->position/enumerator->indexInterval)+1;

		// but before end of block
		if (
			//the length of indexTerms (the number of terms in enumerator) equals
			//_enum_offset OR
			indexTermsLength == _enumOffset	 || 
			//term is positioned in front of term found at _enumOffset in indexTerms
			term->compareTo(&indexTerms[_enumOffset]) < 0){

			//no need to seek, retrieve the TermInfo for term
			ret = scanEnum(term);			  

			return ret;
        }
    }

    //Reposition current term in the enumeration 
    seekEnum(getIndexOffset(term));

	//Retrieve the TermInfo for term
    ret = scanEnum(term);

    return ret;
  }


  int64_t TermInfosReader::getPosition(const Term* term) {
  //Func - Returns the position of a Term in the set
  //Pre  - term holds a valid reference to a Term
  //       enumerator != NULL
  //Post - If term was found then its position is returned otherwise -1
      SCOPED_LOCK_MUTEX(getPosition_LOCK);

      CND_PRECONDITION(enumerator != NULL,"enumerator is NULL");

	  //if the enumeration is empty then return -1
	  if (size == 0){
		  return -1;
      }

      //Retrieve the indexOffset for term
      int32_t indexOffset = getIndexOffset(term);
      seekEnum(indexOffset);

      while(term->compareTo(enumerator->term(false)) > 0 && enumerator->next()) {}

      if ( term->equals(enumerator->term(false)) ) //todo: changed to equals, but check if this is more efficient
          return enumerator->position;
      else
          return -1;
  }

  SegmentTermEnum* TermInfosReader::getTerms(){
  //Func - Returns an enumeration of all the Terms and TermInfos in the set
  //Pre  - enumerator != NULL
  //Post - enumerator has been cloned a reference to the clone has been returned
      
      SCOPED_LOCK_MUTEX(getTerms_LOCK);
      
      CND_PRECONDITION(enumerator != NULL,"enumerator is NULL");

      //if the position is not at the start
      if (enumerator->position != -1){			  
          //reset the position of the current within the enumeration to the start
          seekEnum(0);				  
	  }

      //TODO2: check this code
	   //Clone the enumeration of terms
      SegmentTermEnum* cln = enumerator->clone();

	  //Check if cln points to a valid instance
	  CND_CONDITION(cln != NULL,"cln is NULL");

	  //Return the reference to the clone
      return cln;
  }


  SegmentTermEnum* TermInfosReader::getTerms(const Term* term) {
  //Func - Returns an enumeration of terms starting at or after the named term.
  //Pre  - term holds a valid reference to a Term
  //       enumerator != NULL
  //Post - An enumeration of terms starting at or after the named term has been returned

      SCOPED_LOCK_MUTEX(getTermsTerm_LOCK);
      
      CND_PRECONDITION(enumerator != NULL,"enumerator is NULL");

      //Seek enumerator to term
      TermInfo* termInfo = get(term);

      //Seek enumerator to term; delete the new TermInfo that's returned.
      if(termInfo){
          _CLDELETE( termInfo );
      }

      //Clone the entire enumeration
      SegmentTermEnum* cln = enumerator->clone();

      //Check if cln points to a valid instance
      CND_CONDITION(cln != NULL,"cln is NULL");


      return cln;
  }

  void TermInfosReader::readIndex() {
  //Func - Reads the term info index file or .tti file.
  //       This file contains every IndexInterval-th entry from the .tis file, 
  //       along with its location in the "tis" file. This is designed to be read entirely 
  //       into memory and used to provide random access to the "tis" file.
  //Pre  - indexTerms    = NULL
  //       indexInfos    = NULL
  //       indexPointers = NULL
  //Post - The term info index file has been read into memory

      CND_PRECONDITION(indexTerms == NULL,    "indexTerms is not NULL");
      CND_PRECONDITION(indexInfos == NULL,    "indexInfos is not NULL");
      CND_PRECONDITION(indexPointers == NULL, "indexPointers is not NULL");

      //Create the name of the term info index file and store it in buf
	  const char* buf = Misc::segmentname(segment,".tii");
	  //Open the file and return an InputStream reference to it
      InputStream* is = directory->openFile( buf );

      //Check if is points to a valid instance
      CND_CONDITION(is != NULL,"is is NULL");

	  //buf is not necessary anymore so have it deleted
      _CLDELETE_LCaARRAY(buf);

      //Instantiate an enumeration
	  SegmentTermEnum* indexEnum = _CLNEW SegmentTermEnum(is, fieldInfos, true);

      //Check if indexEnum points to a valid instance
      CND_CONDITION(indexEnum != NULL,"No memory could be allocated for indexEnum");

      try {
          indexTermsLength = (size_t)indexEnum->size;

		  //Instantiate an array indexTerms which contains pointers to read Terms
 		  //bvk: instantiate in a big block, instead of one by one...
          indexTerms    = _CL_NEWARRAY(Term,indexTermsLength);
          //Check if is indexTerms is a valid array
          CND_CONDITION(indexTerms != NULL,"No memory could be allocated for indexTerms");
		  //Instantiate an array indexInfos which contains pointer to TermInfo instances
		  //bvk: instantiate in a big block, instead of one by one...
          indexInfos    = _CL_NEWARRAY(TermInfo,indexTermsLength);
          //Check if is indexInfos is a valid array
          CND_CONDITION(indexInfos != NULL,"No memory could be allocated for indexInfos");
          //Instantiate an array indexPointers that contains pointers to the term info index file
          indexPointers = _CL_NEWARRAY(int64_t,indexTermsLength);
          //Check if is indexPointers is a valid array
          CND_CONDITION(indexPointers != NULL,"No memory could be allocated for indexPointers");

		  //Iterate through the terms of indexEnum
          for (int32_t i = 0; indexEnum->next(); i++){
              indexTerms[i].set(indexEnum->term(false)->field(),indexEnum->term(false)->text());
              indexEnum->getTermInfo(&indexInfos[i]);
              indexPointers[i] = indexEnum->indexPointer;
          }
    }_CLFINALLY(
          indexEnum->close(); 
          _CLDELETE( indexEnum ); 
		  //Close and delete the InputStream is. The close is done by the destructor.
          _CLDELETE( is );
          );
  }


  int32_t TermInfosReader::getIndexOffset(const Term* term){
  //Func - Returns the offset of the greatest index entry which is less than or equal to term.
  //Pre  - term holds a reference to a valid term
  //       indexTerms != NULL
  //Post - The new offset has been returned

      //Check if is indexTerms is a valid array
      CND_PRECONDITION(indexTerms != NULL,"indexTerms is NULL");

      int32_t lo = 0;					  
      int32_t hi = indexTermsLength - 1;

      while (hi >= lo) {
          //Start in the middle betwee hi and lo
          int32_t mid = (lo + hi) >> 1;

          //Check if is indexTerms[mid] is a valid instance of Term
          CND_PRECONDITION(&indexTerms[mid] != NULL,"indexTerms[mid] is NULL");
          CND_PRECONDITION(mid < indexTermsLength != NULL,"indexTerms[mid] is NULL");

		  //Determine if term is before mid or after mid
          int32_t delta = term->compareTo(&indexTerms[mid]);
          if (delta < 0){
              //Calculate the new hi   
              hi = mid - 1;
          }else{
			  if (delta > 0){
                  //Calculate the new lo 
                  lo = mid + 1;
			      }
			  else{
                  //term has been found so return its position
                  return mid;
                  }
              }
          }
    // the new starting offset
    return hi;
  }

  void TermInfosReader::seekEnum(const int32_t indexOffset) {
  //Func - Reposition the current Term and TermInfo to indexOffset
  //Pre  - indexOffset >= 0
  //       indexTerms    != NULL
  //       indexInfos    != NULL
  //       indexPointers != NULL
  //Post - The current Term and Terminfo have been repositioned to indexOffset

      CND_PRECONDITION(indexOffset >= 0, "indexOffset contains a negative number");
      CND_PRECONDITION(indexTerms != NULL,    "indexTerms is NULL");
      CND_PRECONDITION(indexInfos != NULL,    "indexInfos is NULL");
      CND_PRECONDITION(indexPointers != NULL, "indexPointers is NULL");

	  enumerator->seek( 
          indexPointers[indexOffset],
		  (indexOffset * enumerator->indexInterval) - 1,
          &indexTerms[indexOffset], 
		  &indexInfos[indexOffset]
	      );
  }


  TermInfo* TermInfosReader::scanEnum(const Term* term) {
  //Func - Scans the Enumeration of terms for term and returns the corresponding TermInfo instance if found.
  //       The search is started from the current term.
  //Pre  - term contains a valid reference to a Term
  //       enumerator != NULL
  //Post - if term has been found the corresponding TermInfo has been returned otherwise NULL
  //       has been returned

      CND_PRECONDITION(enumerator != NULL, "enumerator is NULL");

      //move the iterator of enumerator to the position where term is expected to be
      //enumerator->scanTo(term);
	  while (term->compareTo(enumerator->term(false)) > 0 && enumerator->next()){
	  }

      //Check if the at the position the Term term can be found
	  if (enumerator->term(false) != NULL && term->equals(enumerator->term(false)) ){ //todo: changed to equals, but check if this is more efficient
		  //Return the TermInfo instance about term
          return enumerator->getTermInfo();
     }else{
          //term was not found so no TermInfo can be returned
          return NULL;
     }
  }

  Term* TermInfosReader::scanEnum(const int32_t position) {
  //Func - Scans the enumeration to the requested position and returns the
  //       Term located at that position
  //Pre  - position > = 0
  //       enumerator != NULL
  //Post - The Term at the requested position has been returned

      CND_PRECONDITION(position >= 0, "position is a negative number");
      CND_PRECONDITION(enumerator != NULL, "enumerator is NULL");

	  //As long the position of the enumeration enumerator is smaller than the requested one
      while(enumerator->position < position){
		  //Move the current of enumerator to the next
		  if (!enumerator->next()){
			  //If there is no next it means that the requested position was to big
              return NULL;
              }
	      }

	  //Return the Term a the requested position
	  return enumerator->term();
  }

CL_NS_END
