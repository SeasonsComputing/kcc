#include "CLucene/StdHeader.h"
#include "StandardFilter.h"

#include "../AnalysisHeader.h"
#include "../Analyzers.h"
#include "StandardTokenizerConstants.h"
#include "CLucene/util/StringBuffer.h"

CL_NS_USE(analysis)
CL_NS_USE(util)
CL_NS_DEF2(analysis,standard)

  StandardFilter::StandardFilter(TokenStream* in, bool deleteTokenStream):
    TokenFilter(in, deleteTokenStream)
  {
  }

  StandardFilter::~StandardFilter(){
  }

  bool StandardFilter::next(Token* t) {
    if (!input->next(t))
      return false;

    TCHAR* text = t->_termText;
    const int32_t textLength = t->termTextLength();
    const TCHAR* type = t->type();

//todo: can type be compared directly? does type get taken from the tokenImage, in which case it will be the same referenc
    if ( _tcscmp(type,tokenImage[APOSTROPHE])==0 && 
		( textLength >= 2 && _tcsicmp(text+textLength-2, _T("'s"))==0  ) )
    {
      // remove 's
      //TCHAR* buf = text; //stringDuplicate(text); -//because we are about to delete this token anyway, we can just use its buffer
      text[textLength-2]=0; 
	  t->resetTermTextLen();

      /*//here the buffer is duplicated, so can now delete buf
      Token* ret = _CLNEW Token(buf, t->StartOffset(), t->EndOffset(), type);
#ifndef LUCENE_TOKEN_WORD_LENGTH
      _CLDELETE_ARRAY(buf); //todo: check this
#endif
      // DSR:CL_BUG_LEAK: t was not deleted in this case, so it leaked.
      _CLDELETE(t);*/
      return true;

    } else if ( _tcscmp(type, tokenImage[ACRONYM])==0 ) {		  // remove dots
      /*StringBuffer trimmed;
      for (int32_t i = 0; i < textLength; i++) {
        TCHAR c = text[i];
        if (c != '.')
			trimmed.appendChar(c);
      }
      Token *ret = _CLNEW Token(trimmed.getBuffer(), t->StartOffset(), t->EndOffset(), type);
      //DSR:CL_BUG_LEAK: t was not deleted in this case, so it leaked.
      _CLDELETE(t);*/
		int32_t j = 0;
		for ( int32_t i=0;i<textLength;i++ ){
			if ( text[i] != '.' )
				text[j++]=text[i];
		}
		text[j]=0;
      return true;

    } else {
      return true;
    }
  }

CL_NS_END2
