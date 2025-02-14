#ifndef _lucene_analysis_standard_StandardTokenizerConstants
#define _lucene_analysis_standard_StandardTokenizerConstants

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

CL_NS_DEF2(analysis,standard)
  enum TokenTypes {
    _EOF,
    UNKNOWN,
    ALPHANUM,
    APOSTROPHE,
    ACRONYM,
    COMPANY,
    EMAIL,
    HOST,
    NUM
  };

  const static TCHAR* tokenImage[] = {
    _T("<EOF>"),
    _T("<UNKNOWN>"),
    _T("<ALPHANUM>"),
    _T("<APOSTROPHE>"),
    _T("<ACRONYM>"),
    _T("<COMPANY>"),
    _T("<EMAIL>"),
    _T("<HOST>"),
    _T("<NUM>")
    //_T("<P>"),
    //_T("<HAS_DIGIT>"),
    //_T("<ALPHA>"),
    //_T("<LETTER>"),
    //_T("<DIGIT>"),
    //_T("<NOISE>"),
  };

CL_NS_END2
#endif
