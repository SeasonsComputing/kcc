#include "CLucene/StdHeader.h"
#include "Term.h"
#include "CLucene/util/StringIntern.h"

CL_NS_USE(util)
CL_NS_DEF(index)
	
	Term::Term(){
		//Intern fld and assign it to field
		_field = LUCENE_BLANK_STRING;
		internF = false;
		cachedHashCode = 0;

		textLen = 0;		
		
		//Duplicate txt and assign it to text
#ifdef LUCENE_TERM_TEXT_LENGTH
      _text[0]=0;
#else
		_text = LUCENE_BLANK_STRING;
		dupT = false;
#endif
	}

	Term::Term(const TCHAR* fld, const TCHAR* txt,const bool internField){
    //Func - Constructor.
    //       Constructs a Term with the given field and text. Field and text are not copied
	//       Field and text are deleted in destructor only if intern is false. 
    //Pre  - fld != NULL and contains the name of the field
    //       txt != NULL and contains the value of the field
	//       internF is true or false and indicates if term Field is interned or not
	//       internT is true or false and indicates if term Text is interned or not
    //       canDelete defaults to true but can be false and indicates to the IGarbageCollector that the Term can be deleted when finalized
    //Post - An instance of Term has been created.Field and txt have not been copied but assigned

        CND_PRECONDITION(fld != NULL, "fld contains NULL");
        CND_PRECONDITION(txt != NULL, "txt contains NULL");

		//todo: this needs to be re-thought. i suggest allowing construction
		//using an iterator containing the field field which can then
		//be directly manipulated.
		//if ( internField )
			_field  = CLStringIntern::intern(fld  CL_FILELINE);
		//else
		//	_field = fld;
		internF = internField;
		cachedHashCode = 0;

		textLen = _tcslen(txt);
		
		//duplicate text if necessary
#ifdef LUCENE_TERM_TEXT_LENGTH
		if ( textLen > LUCENE_TERM_TEXT_LENGTH )
		   textLen = LUCENE_TERM_TEXT_LENGTH;
		_tcsncpy(_text,txt,textLen+1);
        _text[textLen]=0;
#else
		dupT  = true;
		//if ( duplicateText ){
			if ( txt[0]==0 ){
				_text = LUCENE_BLANK_STRING;
				dupT = false;
			}else
				_text = stringDuplicate(txt);
		/*}else
			_text = txt;*/
		textLenBuf = textLen;
#endif

		//this->_canDelete = canDelete;
	}
	
	Term::~Term(){
	//Func - Destructor.
	//Pre  - true
	//Post - The instance has been destroyed. field and text have been deleted if pre(intrn) is false

		//Unintern field
		//if ( internF )
			CLStringIntern::unintern(_field);
		_field = NULL;

#ifndef LUCENE_TERM_TEXT_LENGTH
		//Deletetext if it is the owner
		if ( dupT && _text)
			_CLDELETE_CARRAY( _text );
#endif
	}

    const TCHAR* Term::field() const {
    //Func - Returns the field of this term, an interned string. The field indicates
    //       the part of a document which this term came from. 
    //Pre  - true
    //Post - field has been returned

        return _field;
    }

    const TCHAR* Term::text() const {
    //Func - Returns the text of this term.  In the case of words, this is simply the
    //       text of the word.  In the case of dates and other types, this is an
    //       encoding of the object as a string.
	//Pre  - true
	//Post - text has been returned

        return _text;
    }

	void Term::set(const TCHAR* fld, const TCHAR* txt,const bool internField){
    //Func - Resets the field and text of a Term.
    //Pre  - fld != NULL and contains the name of the field
    //       txt != NULL and contains the value of the field
	//       internF is true or false
	//       internT is true or false
    //Post - field and text of Term have been reset

        CND_PRECONDITION(fld != NULL, "fld contains NULL");
        CND_PRECONDITION(txt != NULL, "txt contains NULL");

		//save field for unintern later
		const TCHAR* oldField = _field;
		bool oldInternF = internF;
		cachedHashCode = 0;

        textLen = _tcslen(txt);

		//Delete text if it is the owner
#ifdef LUCENE_TERM_TEXT_LENGTH
		if ( textLen > LUCENE_TERM_TEXT_LENGTH )
		   textLen = LUCENE_TERM_TEXT_LENGTH;
		_tcsncpy(_text,txt,textLen+1);
        _text[textLen]=0;
#else

		//assign new text
		bool olddupT = dupT;
		dupT = true;

		//if ( duplicateText ){
			//if the term text buffer is bigger than what we have
			if ( !olddupT && _text ){
				_text = NULL;
				textLenBuf = 0;
			}else if ( _text && textLen > textLenBuf){
				_CLDELETE_ARRAY( _text );
				textLenBuf = 0;
			}

			if ( _text==NULL ){
				if ( txt[0] == 0 ){
					//if the string is blank and we aren't re-using the buffer...
					_text = LUCENE_BLANK_STRING;
					dupT = false;
				}else{
					//duplicate the text
					_text  = stringDuplicate(txt);
					textLenBuf = textLen;
				}
			}else{
				//re-use the buffer
				_tcscpy(_text,txt);
			}

#endif

	    //Set Term Field
		//if ( internField )
			_field = CLStringIntern::intern(fld  CL_FILELINE);
		//else
		//	_field = fld;
		internF = internField;

		//unintern old field after interning new one, 
		//if ( oldInternF )
			CLStringIntern::unintern(oldField);
	}

   /** Compares two terms, returning true iff they have the same
      field and text. */
   bool Term::equals(const Term* other) const{
	   if ( cachedHashCode != 0 && other->cachedHashCode != 0 &&
			other->cachedHashCode != cachedHashCode )
			return false;

      if ( _field==other->_field ){
         //this can be quicker than using compareTo, because checks
         //field length first
		  if ( textLen == other->textLen ){
				
			  return (_tcscmp(_text,other->_text)==0);
		  }else
            return false;
      }else
         return false;
   }

   size_t Term::hashCode(){
		if ( cachedHashCode != 0 )
			return cachedHashCode;

		cachedHashCode = Misc::thashCode(_field) + Misc::thashCode(_text,textLen);
		return cachedHashCode;
   }


	int32_t Term::compareTo(const Term* other) const {
    //Func - Compares two terms, to see if this term belongs before,is equal to or after
	//       after the argument term.
	//Pre  - other is a reference to another term
	//Post - A negative integer is returned if this term belongs before the argument, 
    //       zero is returned if this term is equal to the argument, and a positive integer 
	//       if this term belongs after the argument.

		//Check ret to see if text needs to be compared
		if ( _field == other->_field ){ // fields are interned
			//Compare text with text of other and return the result
			return _tcscmp(_text,other->_text);
		}else
			return _tcscmp(_field,other->_field);
	}

	TCHAR* Term::toString() const{
	//Func - Forms the contents of Field and term in some kind of tuple notation
	//       <field:text>
	//Pre  - true
	//Post - a string formatted as <field:text> is returned if pre(field) is NULL and
	//       text is NULL the returned string will be formatted as <:>

		return CL_NS(util)::Misc::join( _field, _T(":"), _text);
	}

CL_NS_END
