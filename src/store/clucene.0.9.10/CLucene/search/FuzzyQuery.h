#ifndef _lucene_search_FuzzyQuery_
#define _lucene_search_FuzzyQuery_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#include "CLucene/index/IndexReader.h"
#include "CLucene/index/Term.h"
#include "MultiTermQuery.h"

#ifndef NO_FUZZY_QUERY


CL_NS_DEF(search)

  // class FuzzyQuery implements the fuzzy search query
  class FuzzyQuery: public MultiTermQuery {
    private:
      CL_NS(index)::Term* fuzzyTerm;

	  float_t minimumSimilarity;
	  int32_t prefixLength;
	  FuzzyQuery(const FuzzyQuery& clone);
   public:
	  static float_t defaultMinSimilarity;

     /**
	* Create a new FuzzyQuery that will match terms with a similarity 
	* of at least <code>minimumSimilarity</code> to <code>term</code>.
	* If a <code>prefixLength</code> &gt; 0 is specified, a common prefix
	* of that length is also required.
	* 
	* @param term the term to search for
	* @param minimumSimilarity a value between 0 and 1 to set the required similarity
	*  between the query term and the matching terms. For example, for a
	*  <code>minimumSimilarity</code> of <code>0.5</code> a term of the same length
	*  as the query term is considered similar to the query term if the edit distance
	*  between both terms is less than <code>length(term)*0.5</code>
	* @param prefixLength length of common (non-fuzzy) prefix
	* @throws IllegalArgumentException if minimumSimilarity is &gt; 1 or &lt; 0
	* or if prefixLength &lt; 0 or &gt; <code>term.text().length()</code>.
	*/
     FuzzyQuery(CL_NS(index)::Term* term, float_t minimumSimilarity=defaultMinSimilarity, int32_t prefixLength=0);
	 //Destructor
     ~FuzzyQuery();

	 //Prepares a fuzzy query
     TCHAR* toString(const TCHAR* field);

	  //Returns the name "FuzzyQuery"
	  static const TCHAR* getClassName();
     const TCHAR* getQueryName() const;

	  Query* clone();
	  bool equals(Query * other) const;
	  size_t hashCode() const;

	  /**
		* Returns the minimum similarity that is required for this query to match.
		* @return float value between 0.0 and 1.0
		*/
		float_t getMinSimilarity() const;

		/**
		* Returns the prefix length, i.e. the number of characters at the start
		* of a term that must be identical (not fuzzy) to the query term if the query
		* is to match that term. 
		*/
		int32_t getPrefixLength() const;

  protected:
	  FilteredTermEnum* getEnum(CL_NS(index)::IndexReader* reader);
  };

   /** FuzzyTermEnum is a subclass of FilteredTermEnum for enumerating all 
  *  terms that are similiar to the specified filter term.
  *
  *  Term enumerations are always ordered by Term.compareTo().  Each term in
  *  the enumeration is greater than all that precede it.
  */
  class FuzzyTermEnum: public FilteredTermEnum {
  private:
		float_t distance;
		bool _endEnum;

		CL_NS(index)::Term* searchTerm; 
		TCHAR* text;
		int32_t textLen;
		TCHAR* prefix;
		int32_t prefixLength;
		float_t minimumSimilarity;
		double scale_factor;

		
		/**
		* This static array saves us from the time required to create a new array
		* everytime editDistance is called.
		*/
		int32_t* e;
		int32_t eWidth;
		int32_t eHeight;

		/******************************
		* Compute Levenshtein distance
		******************************/
    
		/** Finds and returns the smallest of three integers */
		static int32_t min3(const int32_t a, const int32_t b, const int32_t c);

		/**
		Levenshtein distance also known as edit distance is a measure of similiarity
		between two strings where the distance is measured as the number of character 
		deletions, insertions or substitutions required to transform one string to 
		the other string. 
		<p>This method takes in four parameters; two strings and their respective 
		lengths to compute the Levenshtein distance between the two strings.
		The result is returned as an integer.
		*/ 
		int32_t editDistance(const TCHAR* s, const TCHAR* t, const int32_t n, const int32_t m) ;

    protected:
      /**
		The termCompare method in FuzzyTermEnum uses Levenshtein distance to 
		calculate the distance between the given term and the comparing term. 
		*/
      bool termCompare(CL_NS(index)::Term* term) ;

	  /** Returns the difference between the distance and the fuzzy threshold
       *  multiplied by the scale factor
	   */
      float_t difference();
	  
    public:
	
	/**
	* Empty prefix and minSimilarity of 0.5f are used.
	* 
	* @param reader
	* @param term
	* @throws IOException
	* @see #FuzzyTermEnum(IndexReader, Term, float_t, int32_t)
	*/
      FuzzyTermEnum(CL_NS(index)::IndexReader* reader, CL_NS(index)::Term* term, float_t minSimilarity=FuzzyQuery::defaultMinSimilarity, int32_t prefixLength=0);
	  /** Destructor */
      ~FuzzyTermEnum();
      //Returns the fact if the current term in the enumeration has reached the end
      bool endEnum();
      /** Close the enumeration */
      void close();
  };
CL_NS_END
#endif
#endif
