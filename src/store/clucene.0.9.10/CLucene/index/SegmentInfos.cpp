#include "CLucene/StdHeader.h"
#include "SegmentInfos.h"

#include "CLucene/store/Directory.h"
#include "CLucene/util/VoidMap.h"
#include "CLucene/util/Misc.h"

CL_NS_USE(store)
CL_NS_USE(util)
CL_NS_DEF(index)

	
   SegmentInfo::SegmentInfo(const char* Name, const int32_t DocCount, CL_NS(store)::Directory* Dir): 
	docCount(DocCount),dir(Dir){
	//Func - Constructor. Initialises SegmentInfo.
	//Pre  - Name holds the unique name in the directory Dir
	//       DocCount holds the number of documents in the segment
	//       Dir holds the Directory where the segment resides
	//Post - The instance has been created. name contains the duplicated string Name.
	//       docCount = DocCount and dir references Dir
			STRCPY_AtoA(name,Name,CL_MAX_DIR);
  }
   SegmentInfo::~SegmentInfo(){
   }


  SegmentInfos::SegmentInfos(bool deleteMembers) :
      infos(deleteMembers){
  //Func - Constructor
  //       This (bool) form of constructor has been added to allow client to instantiate
  //       SegmentInfos object that does not delete its members upon its own deletion.  
  //       This change was prompted by a bug in IndexWriter::addIndexes.
  //Pre  - deleteMembers indicates if the instance to be created must delete
  //       all SegmentInfo instances it manages when the instance is destroyed or not
  //       true -> must delete, false may not delete
  //Post - An instance of SegmentInfos has been created.
  
      //initialize counter to 0
      counter = 0;
      version = 0;
  }

  SegmentInfos::~SegmentInfos(){
  //Func - Destructor
  //Pre  - true
  //Post - The instance has been destroyed. Depending on the constructor used
  //       the SegmentInfo instances that this instance managed have been deleted or not.

	  //Clear the list of SegmentInfo instances - make sure everything is deleted
	  infos.setDoDelete(true); //todo: should this always be set to true?
      infos.clear();
  }
  
  SegmentInfo* SegmentInfos::info(int32_t i) {
  //Func - Returns a reference to the i-th SegmentInfo in the list.
  //Pre  - i >= 0
  //Post - A reference to the i-th SegmentInfo instance has been returned

      CND_PRECONDITION(i >= 0, "i contains negative number");

	  //Get the i-th SegmentInfo instance
      SegmentInfo *ret = infos[i];
      //Condition check to see if the i-th SegmentInfo has been retrieved
      CND_CONDITION(ret != NULL,"No SegmentInfo instance found");

      return ret;
  }

  void SegmentInfos::clearto(int32_t min){
	  while ( infos.size() > min ){
			segmentInfosType::iterator itr = infos.end();
			if ( itr != infos.begin())
				itr --;
			_CLLDELETE((*itr));
			infos.erase(itr);
	  }
  }
  void SegmentInfos::add(SegmentInfo* info){
	infos.push_back(info);
  }
  int32_t SegmentInfos::size(){
	  return infos.size();
  }

  void SegmentInfos::read(Directory* directory){
  //Func - Reads segments file that resides in directory. 
  //Pre  - directory contains a valid reference
  //Post - The segments file has been read and for each segment found
  //       a SegmentsInfo intance has been created and stored.

	  //Open an InputStream to the segments file
      InputStream* input = directory->openFile("segments");
	  //Check if input is valid
	  if (input){
        try {
            int32_t format = input->readInt();

            if(format < 0){     // file contains explicit format info
               // check that it is a format we can understand
               if (format < FORMAT){
                  TCHAR err[30];
                  _sntprintf(err,30,_T("Unknown format version: %d"),format);
                  _CLTHROWT(CL_ERR_Runtime,err);
               }
               version = input->readLong(); // read version
               counter = input->readInt(); // read counter
            }
            else{     // file is in old format without explicit format info
               counter = format;
            }	  

            //Temporary variable for storing the name of the segment
            TCHAR tname[CL_MAX_PATH];
            char aname[CL_MAX_PATH];
            SegmentInfo* si  = NULL;

            //read segmentInfos
            for (int32_t i = input->readInt(); i > 0; i--){ 
                // read the name of the segment
                input->readString(tname, CL_MAX_PATH); 
				    STRCPY_TtoA(aname,tname,CL_MAX_PATH);

                //Instantiate a new SegmentInfo Instance
                si = _CLNEW SegmentInfo(aname, input->readInt(),directory);

                //Condition check to see if si points to an instance
                CND_CONDITION(si != NULL, "Memory allocation for si failed")	;

                //store SegmentInfo si
                infos.push_back(si);
             } 

            if(format >= 0){ // in old format the version number may be at the end of the file
               if (input->getFilePointer() >= input->length())
                  version = 0; // old file format without version number
               else
                  version = input->readLong(); // read version
            }
        } _CLFINALLY(
            //destroy the inputStream input. The destructor of InputStream will 
		    //also close the Inputstream input
            _CLDELETE( input );
            );
	  }
  }

#ifndef CLUCENE_LITE
  void SegmentInfos::write(Directory* directory){
  //Func - Writes a new segments file based upon the SegmentInfo instances it manages
  //Pre  - directory is a valid reference to a Directory
  //Post - The new segment has been written to disk
    
	  //Open an OutputStream to the segments file
	  OutputStream* output = directory->createFile("segments.new");
	   //Check if output is valid
	  if (output){
          try {
		   output->writeInt(FORMAT); // write FORMAT
           output->writeLong(++version); // every write changes the index
           output->writeInt(counter); //Write the counter

			  //Write the number of SegmentInfo Instances
			  //which is equal to the number of segments in directory as
			  //each SegmentInfo manages a single segment
			  output->writeInt(infos.size());			  

			  SegmentInfo *si = NULL;

			  //temporary value for wide segment name
			  TCHAR tname[CL_MAX_PATH];

			  //Iterate through all the SegmentInfo instances
           for (uint32_t i = 0; i < infos.size(); i++) {
				  //Retrieve the SegmentInfo
               si = info(i);
               //Condition check to see if si has been retrieved
               CND_CONDITION(si != NULL,"No SegmentInfo instance found");

				  //Write the name of the current segment
              STRCPY_AtoT(tname,si->name,CL_MAX_PATH);
				  output->writeString(tname,_tcslen(tname));

				  //Write the number of documents in the segment 
              output->writeInt(si->docCount);
           }
         } _CLFINALLY(
              output->close();
              _CLDELETE( output );
         );

          // install new segment info
          directory->renameFile("segments.new","segments");
	  }
  }
#endif //CLUCENE_LITE

  
  int64_t SegmentInfos::readCurrentVersion(Directory* directory){
    InputStream* input = directory->openFile("segments");
    int32_t format = 0;
    int64_t version = 0;
    try {
      format = input->readInt();
      if(format < 0){
         if(format < FORMAT){
            TCHAR err[30];
            _sntprintf(err,30,_T("Unknown format version: %d"),format);
            _CLTHROWT(CL_ERR_Runtime,err);
         }
         version = input->readLong(); // read version
       }
     }
     _CLFINALLY( input->close(); _CLDELETE(input); );
     
     if(format < 0)
      return version;

    // We cannot be sure about the format of the file.
    // Therefore we have to read the whole file and cannot simply seek to the version entry.
    SegmentInfos* sis = _CLNEW SegmentInfos();
    sis->read(directory);
    version = sis->getVersion();
    _CLDELETE(sis);
    return version;
  }

CL_NS_END
