#ifndef _lucene_search_Explanation
#define _lucene_search_Explanation

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

CL_NS_DEF(search)

   #define LUCENE_SEARCH_EXPLANATION_DESC_LEN 200
   class Explanation :LUCENE_BASE {
   private:
      float_t value;                            // the value of this node
      TCHAR description[LUCENE_SEARCH_EXPLANATION_DESC_LEN];                     // what it represents
      CL_NS(util)::CLArrayList<Explanation*,CL_NS(util)::Deletor::Object<Explanation> > details;                      // sub-explanations

      ///todo: mem leaks
      TCHAR* toString(int32_t depth);
      Explanation(Explanation& copy);
   public:
      Explanation() {}
      ~Explanation();

      Explanation(float_t value, TCHAR* description);

      Explanation* clone(){ return _CLNEW Explanation(*this); };

      /** The value assigned to this explanation node. */
      float_t getValue() { return value; }
        
      /** Sets the value assigned to this explanation node. */
      void setValue(float_t value) { this->value = value; }

      /** A description of this explanation node. */
      TCHAR* getDescription() { return description; } ///<returns reference
        
      /** Sets the description of this explanation node. */
      void setDescription(TCHAR* description) {
         _tcsncpy(this->description,description,LUCENE_SEARCH_EXPLANATION_DESC_LEN);
      }

      /** The sub-nodes of this explanation node. */
      Explanation** getDetails();

      /** Adds a sub-node to this explanation node. */
      void addDetail(Explanation* detail);

      /** Render an explanation as text. */
      TCHAR* toString();

      /** Render an explanation as HTML. */
      ///todo: mem leaks
      TCHAR* toHtml();
   };

CL_NS_END
#endif
