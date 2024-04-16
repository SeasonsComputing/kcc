#ifndef _lucene_index_TermInfosReader_
#define _lucene_index_TermInfosReader_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#include "Terms.h"
#include "SegmentTermEnum.h"
#include "CLucene/store/Directory.h"
#include "FieldInfos.h"
#include "TermInfo.h"
#include "TermInfosWriter.h"

CL_NS_DEF(index)
   /**
   * Porting status: todo: JLucene uses a localthread, which have a different 
   * state associated with every. Basically this class is still at v1.2.
   * The changes involve syncronisation issues... so beware when
   * using this with multiple threads.   
   * Hrmm... will need to look into this one more.
   * Ideas anyone???
   */
	class TermInfosReader :LUCENE_BASE{
	private:
		CL_NS(store)::Directory* directory;
		const char* segment;
		FieldInfos* fieldInfos;

		SegmentTermEnum* enumerator;
		int64_t size;

		Term* indexTerms;
        int32_t indexTermsLength;
		TermInfo* indexInfos;
		int64_t* indexPointers;

		
		DEFINE_MUTEX(getInt_LOCK);
		DEFINE_MUTEX(getTerm_LOCK);
		DEFINE_MUTEX(getPosition_LOCK);
		DEFINE_MUTEX(getTerms_LOCK);
		DEFINE_MUTEX(getTermsTerm_LOCK);
	public:
		//Constructor.
        //Reads the TermInfos file (.tis) and eventually the Term Info Index file (.tii)
		TermInfosReader(CL_NS(store)::Directory* dir, const char* segment, FieldInfos* fis);
		//Destructor
		~TermInfosReader();
		//Close the enumeration of TermInfos
		void close();
		
		//Return the size of the enumeration of TermInfos
		int64_t Size();

		int32_t getSkipInterval() { return enumerator->skipInterval; }

		// Returns the nth term in the set. 
		// synchronized
		Term* get(const int32_t position);

		// Returns the TermInfo for a Term in the set
		// synchronized
		TermInfo* get(const Term* term);
		
		// Returns the position of a Term in the set
		// synchronized 
		int64_t getPosition(const Term* term);

		//Returns an enumeration of all the Terms and TermInfos in the set
		// synchronized 
		SegmentTermEnum* getTerms();

		// Returns an enumeration of terms starting at or after the named term. 
		// synchronized 
		SegmentTermEnum* getTerms(const Term* term);
	private:
		//Reads the term info index file or .tti file.
		void readIndex();

		// Returns the offset of the greatest index entry which is less than term.
		int32_t getIndexOffset(const Term* term);

		//Reposition the current Term and TermInfo to indexOffset
		void seekEnum(const int32_t indexOffset);  

		//Scans the Enumeration of terms for term and returns the corresponding TermInfo instance if found.
        //The search is started from the current term.
		TermInfo* scanEnum(const Term* term);

        //Scans the enumeration to the requested position and returns the
        //Term located at that position
		Term* scanEnum(const int32_t position);
	};
CL_NS_END
#endif
