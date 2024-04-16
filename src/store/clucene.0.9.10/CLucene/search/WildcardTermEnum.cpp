#include "CLucene/StdHeader.h"
#ifndef NO_WILDCARD_QUERY
#include "WildcardTermEnum.h"

CL_NS_USE(index)
CL_NS_DEF(search)

    bool WildcardTermEnum::termCompare(Term* term) {
        if ( term!=NULL && __term->field() == term->field() ) {
            const TCHAR* searchText = term->text();
            const TCHAR* patternText = __term->text();
            //todo, should use _tcsncmp here, but we dont use it
            //should be able to do this check more efficiently
			bool found = true;
			for ( int32_t i=0;i<preLen;i++ )
				if ( searchText[i] != pre[i] ){
					found=false;
					break;
				}
			if ( found ){
               return wildcardEquals(patternText+preLen, 0, searchText, preLen);
            }
        }
        _endEnum = true;
        return false;
    }

    /** Creates new WildcardTermEnum */
    WildcardTermEnum::WildcardTermEnum(IndexReader* reader, Term* term):
		FilteredTermEnum(),
		fieldMatch(false),
		_endEnum(false),
	    __term(_CL_POINTER(term))
    {
       
		pre = stringDuplicate(term->text());

		const TCHAR* sidx = _tcschr( pre, LUCENE_WILDCARDTERMENUM_WILDCARD_STRING );
		const TCHAR* cidx = _tcschr( pre, LUCENE_WILDCARDTERMENUM_WILDCARD_CHAR );
		const TCHAR* tidx = sidx;
		if (sidx == NULL) 
			tidx = cidx;
		else if (cidx-pre >= 0) 
			tidx = min(tidx, cidx);
		int32_t idx = (int32_t)(tidx - pre);
		pre[idx]=0; //trim end
		preLen = idx;

		Term* t = _CLNEW Term(__term->field(), pre,false);
		setEnum( reader->terms(t) );
		_CLDECDELETE(t);
  }

    void WildcardTermEnum::close()
    {
       if ( __term != NULL ){
         FilteredTermEnum::close();

         _CLDECDELETE(__term);
         __term = NULL;

         _CLDELETE_CARRAY( pre );
       }
    }
    WildcardTermEnum::~WildcardTermEnum() {
      close();
    }

    float_t WildcardTermEnum::difference() {
        return 1.0f;
    }

    bool WildcardTermEnum::endEnum() {
        return _endEnum;
    }

    /**
     * Determines if a word matches a wildcard pattern.
     * <small>Work released by Granta Design Ltd after originally being done on
     * company time.</small>
     */
    bool WildcardTermEnum::wildcardEquals(const TCHAR* pattern, int32_t patternIdx, const TCHAR* str, int32_t stringIdx)
    {
       //todo: pattern length should already be known...
       int32_t patternLen = _tcslen(pattern);
       int32_t strLen = _tcslen(str);

        for (int32_t p = patternIdx; ; ++p)
        {
            for (int32_t s = stringIdx; ; ++p, ++s)
            {
                // End of str yet?
                bool sEnd = (s >= strLen);
                // End of pattern yet?
                bool pEnd = (p >= patternLen);

                // If we're looking at the end of the str...
                if (sEnd)
                {
                    // Assume the only thing left on the pattern is/are wildcards
                    bool justWildcardsLeft = true;

                    // Current wildcard position
                    int32_t wildcardSearchPos = p;
                    // While we haven't found the end of the pattern,
                // and haven't encountered any non-wildcard characters
                    while (wildcardSearchPos < patternLen && justWildcardsLeft)
                    {
                        // Check the character at the current position
                        TCHAR wildchar = pattern[wildcardSearchPos];
                        // If it's not a wildcard character, then there is more
                  // pattern information after this/these wildcards.

                        if (wildchar != LUCENE_WILDCARDTERMENUM_WILDCARD_CHAR && wildchar != LUCENE_WILDCARDTERMENUM_WILDCARD_STRING)
                        {
                            justWildcardsLeft = false;
                        }
                        else
                        {
                            // Look at the next character
                            wildcardSearchPos++;
                        }
                    }

                    // This was a prefix wildcard search, and we've matched, so
                // return true.
                    if (justWildcardsLeft)
                {
                        return true;
                }
            }

            // If we've gone past the end of the str, or the pattern,
            // return false.
            if (sEnd || pEnd)
            {
                break;
            }

            // Match a single character, so continue.
			if (pattern[p] == LUCENE_WILDCARDTERMENUM_WILDCARD_CHAR)
            {
                continue;
            }

                //
                if (pattern[p] == LUCENE_WILDCARDTERMENUM_WILDCARD_STRING)
                {
                    // Look at the character beyond the '*'.
                    ++p;
                    // Examine the str, starting at the last character.
                    for (int32_t i = strLen; i >= s; --i)
                    {
                        if (wildcardEquals(pattern, p, str, i))
                  {
                            return true;
                  }
                    }
                    break;
                }
        if (pattern[p] != str[s])
            {
                break;
            }
            }
            return false;
      }
    }

CL_NS_END
#endif
