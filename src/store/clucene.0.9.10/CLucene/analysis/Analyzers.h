#ifndef _lucene_analysis_Analyzers_
#define _lucene_analysis_Analyzers_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#include "CLucene/util/Reader.h"
#include "AnalysisHeader.h"
#include "CLucene/util/Misc.h"
#include "PorterStemmer.h"

CL_NS_DEF(analysis)
    class CharTokenizer:public Tokenizer {
	private:
		int32_t offset, bufferIndex, dataLen;
		TCHAR buffer[LUCENE_MAX_WORD_LEN+1];
		TCHAR ioBuffer[LUCENE_IO_BUFFER_SIZE+1];
		//CL_NS(util)::Reader* input; input is in tokenizer base class (bug fix thanks to Andy Osipienko)
	protected:
		// Returns true iff a character should be included in a token.  This
		// tokenizer generates as tokens adjacent sequences of characters which
		// satisfy this predicate.  Characters for which this is false are used to
		// define token boundaries and are not included in tokens.
		virtual bool isTokenChar(const TCHAR c) const = 0;

		// Called on each token character to normalize it before it is added to the
		// token.  The default implementation does nothing.  Subclasses may use this
		// to, e.g., lowercase tokens. 
		virtual TCHAR normalize(const TCHAR c) const;
	
	public:
		CharTokenizer(CL_NS(util)::Reader* in);

      virtual ~CharTokenizer(){
      }
    
		// Returns the next token in the stream, or null at EOS. 
		// *** This is not a pointer. Use of it must deleted.
		bool next(Token* token);
	};
	
	class PorterStemmerFilter: public TokenFilter {
	public:
		PorterStemmerFilter(TokenStream* in, bool deleteTokenStream):
		TokenFilter(in,deleteTokenStream) {}
		~PorterStemmerFilter(){}
		bool next(Token* t);
   };
	
	
	class LetterTokenizer:public CharTokenizer {
	public:
		// Construct a new LetterTokenizer. 
		LetterTokenizer(CL_NS(util)::Reader* in):
		  CharTokenizer(in) {}
	
	    ~LetterTokenizer(){}
	protected:
		// Collects only characters which satisfy
		// {@link Character#isLetter(TCHAR)}.
		bool isTokenChar(const TCHAR c) const;
	};
	
	
	
	
	
	// LowerCaseTokenizer performs the function of LetterTokenizer
	// and LowerCaseFilter together.  It divides text at non-letters and converts
	// them to lower case.  While it is functionally equivalent to the combination
	// of LetterTokenizer and LowerCaseFilter, there is a performance advantage
	// to doing the two tasks at once, hence this (redundant) implementation.
	// <P>
	// Note: this does a decent job for most European languages, but does a terrible
	// job for some Asian languages, where words are not separated by spaces.
	class LowerCaseTokenizer:public LetterTokenizer {
	public:
		// Construct a new LowerCaseTokenizer. 
		LowerCaseTokenizer(CL_NS(util)::Reader* in):
			LetterTokenizer(in) {}

	    ~LowerCaseTokenizer(){}
	protected:
		// Collects only characters which satisfy
		// {@link Character#isLetter(TCHAR)}.
		TCHAR normalize(const TCHAR chr) const;
	};
	
	

	class WhitespaceTokenizer: public CharTokenizer {
	public:
		// Construct a new WhitespaceTokenizer. 
		WhitespaceTokenizer(CL_NS(util)::Reader* in):CharTokenizer(in) {}
		~WhitespaceTokenizer(){}
	protected:
		// Collects only characters which do not satisfy
		// {@link Character#isWhitespace(TCHAR)}.
		bool isTokenChar(const TCHAR c) const;
	};
	
	
	// An Analyzer that uses WhitespaceTokenizer. 
    class WhitespaceAnalyzer: public Analyzer {
     public:
      TokenStream* tokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader);
	  ~WhitespaceAnalyzer(){}
    };


    class SimpleAnalyzer: public Analyzer {
	public:
		TokenStream* tokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader);
		~SimpleAnalyzer(){}
	};
	
	
	
   /**
   * Normalizes token text to lower case.
   *
   * @version $Id: Analyzers.h 15199 2007-03-09 17:57:17Z tvk $
   */
	class LowerCaseFilter: public TokenFilter {
	public:
		LowerCaseFilter(TokenStream* in, bool deleteTokenStream):TokenFilter(in,deleteTokenStream) {}
		~LowerCaseFilter(){}
		bool next(Token* token);
    };



    // Removes stop words from a token stream.
	class StopFilter: public TokenFilter {
	private:
		//bvk: i found this to work faster with a non-hash table. the number of items
		//in the stop table is not like to make it worth having hashing.
		CL_NS(util)::CLSetList<TCHAR*>* table;
	public:
		// Constructs a filter which removes words from the input
		//	TokenStream that are named in the array of words. 
		StopFilter(TokenStream* in, bool deleteTokenStream, TCHAR* stopWords[]);

		~StopFilter(){}

		// Constructs a filter which removes words from the input
		//	TokenStream that are named in the Hashtable.
		StopFilter(TokenStream* in, bool deleteTokenStream, CL_NS(util)::CLSetList<TCHAR*>* stopTable):
			TokenFilter(in, deleteTokenStream),
			table(stopTable)
		{} 
		  
		
		/**
		* Builds a Hashtable from an array of stop words, appropriate for passing
		*	into the StopFilter constructor.  This permits this table construction to
		*	be cached once when an Analyzer is constructed. 
		* Note: the stopWords list must be a static list because the strings are not copied
		*
		* @swig stopWords
		*/
		static void fillStopTable(CL_NS(util)::CLSetList<TCHAR*>* stopTable,
                                          TCHAR** stopWords);

		/**
		* Returns the next input Token whose termText() is not a stop word.
		*
		* @swig token byref
		*/ 
		bool next(Token* token);
	};
	
	
	
	
    //An array containing some common English words that are usually not
	//useful for searching.
	
    // Filters LetterTokenizer with LowerCaseFilter and StopFilter. 
    class StopAnalyzer: public Analyzer {
        CL_NS(util)::CLSetList<TCHAR*> stopTable;
    
    public:
        // Builds an analyzer which removes words in ENGLISH_STOP_WORDS. 
        StopAnalyzer();
        ~StopAnalyzer();
        
        // Builds an analyzer which removes words in the provided array. 
        StopAnalyzer( TCHAR** stopWords );
        // Filters LowerCaseTokenizer with StopFilter. 
        TokenStream* tokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader);
		
        static TCHAR* ENGLISH_STOP_WORDS[];
    };
    
    
    
    /**
     * This analyzer is used to facilitate scenarios where different
     * fields require different analysis techniques.  Use {@link #addAnalyzer}
     * to add a non-default analyzer on a field name basis.
     * 
     * <p>Example usage:
     * 
     * <pre>
     *   PerFieldAnalyzerWrapper aWrapper =
     *      new PerFieldAnalyzerWrapper(new StandardAnalyzer());
     *   aWrapper.addAnalyzer("firstname", new KeywordAnalyzer());
     *   aWrapper.addAnalyzer("lastname", new KeywordAnalyzer());
     * </pre>
     * 
     * <p>In this example, StandardAnalyzer will be used for all fields except "firstname"
     * and "lastname", for which KeywordAnalyzer will be used.
     * 
     * <p>A PerFieldAnalyzerWrapper can be used like any other analyzer, for both indexing
     * and query parsing.
     */
    class PerFieldAnalyzerWrapper : public Analyzer {
    private:
      Analyzer* defaultAnalyzer;
      CL_NS(util)::CLHashMap<const TCHAR*, Analyzer*, CL_NS(util)::Compare::TChar,
          CL_NS(util)::Deletor::tcArray,CL_NS(util)::Deletor::Void<Analyzer> > analyzerMap;
    public:
      /**
       * Constructs with default analyzer.
       *
       * @param defaultAnalyzer Any fields not specifically
       * defined to use a different analyzer will use the one provided here.
       */
      PerFieldAnalyzerWrapper(Analyzer* defaultAnalyzer);
        ~PerFieldAnalyzerWrapper();
    
      /**
       * Defines an analyzer to use for the specified field.
       *
       * @param fieldName field name requiring a non-default analyzer
       * @param analyzer non-default analyzer to use for field
       */
      void addAnalyzer(const TCHAR* fieldName, Analyzer* analyzer);
      TokenStream* tokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader);
};

CL_NS_END
#endif
