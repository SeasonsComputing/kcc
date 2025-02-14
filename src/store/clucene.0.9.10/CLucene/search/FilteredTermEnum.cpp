#include "CLucene/StdHeader.h"

#include "FilteredTermEnum.h"

CL_NS_USE(index)
CL_NS_DEF(search)


    FilteredTermEnum::FilteredTermEnum(){
	//Func - Constructor
	//Pre  - true
	//Post - Instance has been created
		
        currentTerm = NULL;
        actualEnum = NULL;
    }

    FilteredTermEnum::~FilteredTermEnum() {
    //Func - Destructor
	//Pre  - true
	//Post - The instance has been destroyed
      
        close();
    }

	int32_t FilteredTermEnum::docFreq() const {
	//Func - Returns the docFreq of the current Term in the enumeration.
	//Pre  - next() must have been called at least once
	//Post - if actualEnum is NULL result is -1 otherwise the frequencey is returned

		if (actualEnum == NULL){
			return -1;
		}
        return actualEnum->docFreq();
    }
    
    bool FilteredTermEnum::next() {
    //Func - Increments the enumeration to the next element.  
	//Pre  - true
	//Post - Returns True if the enumeration has been moved to the next element otherwise false

		//The actual enumerator is not initialized!
		if (actualEnum == NULL){
			return false; 
		    }

		//Finalize the currentTerm and reset it to NULL
       _CLDECDELETE( currentTerm );

		//Iterate through the enumeration
        while (currentTerm == NULL) {
            if (endEnum()) 
				return false;
            if (actualEnum->next()) {
                //Order term not to return reference ownership here. */
                Term* term = actualEnum->term(false);
				//Compare the retrieved term
                if (termCompare(term)){
					//Matched so finalize the current
                    _CLDECDELETE(currentTerm);
					//Get a reference to the matched term
                    currentTerm = _CL_POINTER(term);
                    return true;
                    }
                }
            else 
                return false;
        }
        _CLDECDELETE(currentTerm);
        currentTerm = NULL;

        return false;
    }

    Term* FilteredTermEnum::term(const bool pointer) {
	//Func - Returns the current Term in the enumeration.
	//Pre  - next() must have been called at least once
	//       pointer is true or false
	//Post - if pre(pointer) is true the reference counter of currentTerm is increased
	//       and current Term is returned otherwise currentTerm is only returned

        if ( pointer && currentTerm!=NULL )
            return _CL_POINTER(currentTerm);
        else
            return currentTerm;
    }

    void FilteredTermEnum::close(){
	//Func - Closes the enumeration to further activity, freeing resources.
	//Pre  - true
	//Post - The Enumeration has been closed

		//Check if actualEnum is valid
		if (actualEnum){
			//Close the enumeration
            actualEnum->close();
		    }

        //Destroy the enumeration
        _CLDELETE(actualEnum);

		//Destroy currentTerm
        _CLDECDELETE(currentTerm);
    }

	void FilteredTermEnum::setEnum(TermEnum* actualEnum) {
	//Func - Sets the actual Enumeration
	//Pre  - actualEnum != NULL
	//Post - The instance has been created

		CND_PRECONDITION(actualEnum != NULL,"actualEnum is NULL");

		_CLDELETE(this->actualEnum);

        this->actualEnum = actualEnum;

        // Find the first term that matches
        //Ordered term not to return reference ownership here.
        Term* term = actualEnum->term(false);
        if (term != NULL && termCompare(term)){
            _CLDECDELETE(currentTerm);
            currentTerm = _CL_POINTER(term);
        }else{
            next();
		  }
    }

CL_NS_END
