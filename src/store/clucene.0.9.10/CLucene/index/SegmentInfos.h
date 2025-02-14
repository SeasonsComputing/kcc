#ifndef _lucene_index_SegmentInfos_
#define _lucene_index_SegmentInfos_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#include "CLucene/util/VoidList.h"
#include "CLucene/store/Directory.h"

CL_NS_DEF(index)

	class SegmentInfo :LUCENE_BASE{
	private:
		//Directory where the segment resides
		CL_NS(store)::Directory* dir;		
	public:
		///Gets the Directory where the segment resides
		CL_NS(store)::Directory* getDir() const{ return dir; } 

    //Unique name in directory dir
		char name[CL_MAX_DIR];	
		//Number of docs in the segment
		const int32_t docCount;						  

		SegmentInfo(const char* Name, const int32_t DocCount, CL_NS(store)::Directory* Dir);

		~SegmentInfo();
	};

	typedef CL_NS(util)::CLVector<SegmentInfo*,CL_NS(util)::Deletor::Object<SegmentInfo> > segmentInfosType;
  //SegmentInfos manages a list of SegmentInfo instances
  //Each SegmentInfo contains information about a segment in a directory.
  //
  //The active segments in the index are stored in the segment info file. 
  //An index only has a single file in this format, and it is named "segments". 
  //This lists each segment by name, and also contains the size of each segment.
  //The format of the file segments is defined as follows:
  //
  //                                        SegCount
  //Segments --> SegCount, <SegName, SegSize>
  //
  //SegCount, SegSize --> UInt32
  //
  //SegName --> String
  //
  //SegName is the name of the segment, and is used as the file name prefix 
  //for all of the files that compose the segment's index.
  //
  //SegSize is the number of documents contained in the segment index. 
  //
  //Note:
  //At http://jakarta.apache.org/lucene/docs/fileformats.html the definition
  //of all file formats can be found. Note that java lucene currently 
  //defines Segments as follows:
  //
  //Segments --> Format, Version, SegCount, <SegName, SegSize>SegCount
  //        
  //Format, SegCount, SegSize --> UInt32        
  //      
  //Format and Version have not been implemented yet
	class SegmentInfos: LUCENE_BASE {
      /** The file format version, a negative number. */
      /* Works since counter, the old 1st entry, is always >= 0 */
      LUCENE_STATIC_CONSTANT(int32_t,FORMAT=-1);

      int64_t version; //counts how often the index has been changed by adding or deleting docs
      segmentInfosType infos;
    public:
      //Constructor
      //This (bool) form of constructor has been added to allow client to instantiate
      //SegmentInfos object that does not delete its members upon its own deletion.  
      //This change was prompted by a bug in IndexWriter::addIndexes.
      SegmentInfos(bool deleteMembers=false);

	  //Destructor
      ~SegmentInfos();

#ifndef LUCENE_HIDE_INTERNAL
      int32_t counter;  // used to name new segments
#endif

		  //delete and clears objects 'from' from to 'to'
			void clearto(int32_t to);
			
			//count of segment infos
			int32_t size();
			//add a segment info
			void add(SegmentInfo* info);
	  	//Returns a reference to the i-th SegmentInfo in the list.
      SegmentInfo* info(int32_t i);

      /**
      * version number when this SegmentInfos was generated.
      */
      int64_t getVersion() { return version; }
      
      static int64_t readCurrentVersion(CL_NS(store)::Directory* directory);


		  //Reads segments file that resides in directory
		  void read(CL_NS(store)::Directory* directory);

#ifndef CLUCENE_LITE
	  //Writes a new segments file based upon the SegmentInfo instances it manages
      void write(CL_NS(store)::Directory* directory);

#endif
  };
CL_NS_END
#endif
