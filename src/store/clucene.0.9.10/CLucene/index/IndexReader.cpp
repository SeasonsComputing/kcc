#include "CLucene/StdHeader.h"
#include "IndexReader.h"

#include "CLucene/store/Directory.h"
#include "CLucene/store/FSDirectory.h"
#include "CLucene/store/Lock.h"
#include "CLucene/document/Document.h"
#include "CLucene/search/Similarity.h"
#include "SegmentInfos.h"
#include "MultiReader.h"
#include "Terms.h"

CL_NS_USE(util)
CL_NS_USE(store)
CL_NS_DEF(index)

  IndexReader::IndexReader(Directory* dir):
   directory(dir){
  //Constructor.
  //Func - Creates an instance of IndexReader
  //Pre  - true
  //Post - An instance has been created with writeLock = NULL

      writeLock = NULL;
      segmentInfos = NULL;
      directoryOwner = false;
      closeDirectory = false;
      stale = false;
      hasChanges = false;
  }

     
   IndexReader::IndexReader(Directory* directory, SegmentInfos* segmentInfos, bool closeDirectory) {
    this->directory = directory;
    this->segmentInfos = segmentInfos;
    directoryOwner = true;
    this->closeDirectory = closeDirectory;
    stale = false;
    hasChanges = false;
    writeLock = NULL;
  }

  IndexReader::~IndexReader(){
  //Func - Destructor
  //       Destroys the instance and releases the writeLock if needed
  //Pre  - true
  //Post - The instance has been destroyed if pre(writeLock) exists is has been released
      if (writeLock != NULL) {
		  //release writeLock
          writeLock->release();
		  _CLDELETE(writeLock);
      }
	  _CLDELETE(segmentInfos);
  }

  IndexReader* IndexReader::open(const char* path){
  //Func - Static method.
  //       Returns an IndexReader reading the index in an FSDirectory in the named path. 
  //Pre  - path != NULL and contains the path of the index for which an IndexReader must be 
  //       instantiated
  //       closeDir indicates if the directory needs to be closed
  //Post - An IndexReader has been returned that reads tnhe index located at path

	  CND_PRECONDITION(path != NULL, "path is NULL");

     IndexReader* reader = open(FSDirectory::getDirectory(path,false),true);
     //because fsdirectory will now have a refcount of 1 more than
     //if the reader had been opened with a directory object,
     //we need to do a refdec
     //reader->getDirectory()->refDec(); //todo: check this
     return reader;
  }

  IndexReader* IndexReader::open( Directory* directory, bool closeDirectory){
  //Func - Static method.
  //       Returns an IndexReader reading the index in an FSDirectory in the named path. 
  //Pre  - directory represents a directory 
  //       closeDir indicates if the directory needs to be closed
  //Post - An IndexReader has been returned that reads the index located at directory

	  // in- & inter-process sync
      SCOPED_LOCK_MUTEX(directory->DIR_OBJ); 
      
     IndexReader* ret = NULL;     

#ifndef CLUCENE_LITE
	  LuceneLock* lock = directory->makeLock("commit.lock");
#endif
	  //Instantiate an IndexReaderLockWith which can produce an IndexReader
      IndexReaderLockWith with(lock,directory,closeDirectory);

	  try{
	  //Create an IndexReader reading the index
		ret = (IndexReader*)with.run();
	  }_CLFINALLY(
        _CLDELETE( lock );
	  );

	   CND_CONDITION(ret != NULL,"ret is NULL");
	   //return reference 
       return ret;
  }

  void* IndexReader::IndexReaderLockWith::doBody() {
  //Func - Reads the segmentinfo file and depending on the number of segments found
  //       it returns a SegmentsReader or a SegmentReader
  //Pre  - directory != NULL
  //Post - Depending on the number of Segments present in directory this method
  //       returns an empty SegmentsReader when there are no segments, a SegmentReader when
  //       directory contains 1 segment and a nonempty SegmentsReader when directory
  //       contains multiple segements

	   CND_PRECONDITION(directory != NULL, "directory is NULL");

	   //Instantiate SegmentInfos
       SegmentInfos* infos = _CLNEW SegmentInfos;
	   try{
			//Have SegmentInfos read the segments file in directory
			infos->read(directory);
	   }catch(...){
	        //make sure infos is cleaned up
			_CLDELETE(infos);
			throw;
	   }

       // If there is at least one segment (if infos.size() >= 1), the last
       // SegmentReader object will close the directory when the SegmentReader
       // object itself is closed (see SegmentReader::doClose).
       // If there are no segments, there will be no "last SegmentReader object"
       // to fulfill this responsibility, so we need to explicitly close the
       // directory in the segmentsreader.close
       
	   //Count the number segments in the directory
	   const uint32_t nSegs = infos->size();

       if (nSegs == 1 ) {
			// index is optimized 
            return _CLNEW SegmentReader(infos, infos->info(0), closeDirectory);
	    }else{
			//Instantiate an array of pointers to SegmentReaders of size nSegs (The number of segments in the index)
			IndexReader** readers = NULL;

			if (nSegs > 0){
				uint32_t infosize=infos->size();
				readers = _CL_NEWARRAY(IndexReader*,infosize+1);
				for (uint32_t i = 0; i < infosize; i++) {
					//Instantiate a SegementReader responsible for reading the i-th segment and store it in
					//the readers array
					readers[i] = _CLNEW SegmentReader(infos->info(i));
				}
				readers[infosize] = NULL;
			}

			//return an instance of SegmentsReader which is a reader that manages all Segments
			MultiReader* ret = _CLNEW MultiReader(directory, infos, closeDirectory, readers);
			return ret;
        }// end if
  }

  int64_t IndexReader::lastModified(const char* directory) {
  //Func - Static method
  //       Returns the time the index in the named directory was last modified. 
  //Pre  - directory != NULL and contains the path name of the directory to check
  //Post - The last modified time of the index has been returned

      CND_PRECONDITION(directory != NULL, "directory is NULL");

	  return FSDirectory::fileModified(directory,"segments");
  }

  int64_t IndexReader::getCurrentVersion(Directory* directory) {
     return SegmentInfos::readCurrentVersion(directory);
  }


   int64_t IndexReader::getCurrentVersion(const char* directory){
      Directory* dir = FSDirectory::getDirectory(directory, false);
      int64_t version = getCurrentVersion(dir);
	  dir->close();
      _CLDECDELETE(dir);
      return version;
   }
    
  int64_t IndexReader::lastModified(const Directory* directory) {
  //Func - Static method
  //       Returns the time the index in this directory was last modified. 
  //Pre  - directory contains a valid reference
  //Post - The last modified time of the index has been returned

      return directory->fileModified("segments");
  }


  bool IndexReader::indexExists(const char* directory){
  //Func - Static method
  //       Checks if an index exists in the named directory
  //Pre  - directory != NULL
  //Post - Returns true if an index exists at the specified directory->
  //       If the directory does not exist or if there is no index in it.
  //       false is returned.

       CND_PRECONDITION(directory != NULL, "directory is NULL");

	   //Create a buffer of length CL_MAXDIR
       char f[CL_MAX_DIR];
	   //Copy the directory string to the buffer
       strcpy(f,directory);
	   //Cat the name of the segments to buffer
       strcat(f, "/segments");
	   //Check if the segments file exists
       return Misc::dir_Exists(f);
  }
    

  void IndexReader::setNorm(int32_t doc, const TCHAR* field, uint8_t value){
    SCOPED_LOCK_MUTEX(lockSetNorm);
    if(directoryOwner)
      aquireWriteLock();
    doSetNorm(doc, field, value);
    hasChanges = true;
  }

 void IndexReader::aquireWriteLock() {
    if (stale)
      _CLTHROWA(CL_ERR_IO,"IndexReader out of date and no longer valid for delete, undelete, or setNorm operations");

    if (writeLock == NULL) {
      LuceneLock* writeLock = directory->makeLock("write.lock");
      if (!writeLock->obtain(LUCENE_WRITE_LOCK_TIMEOUT)) // obtain write lock
       _CLTHROWA(CL_ERR_IO,"Index locked for write"); // + writeLock
      this->writeLock = writeLock;

      // we have to check whether index has changed since this reader was opened.
      // if so, this reader is no longer valid for deletion
      if (SegmentInfos::readCurrentVersion(directory) > segmentInfos->getVersion()) {
        stale = true;
        this->writeLock->release();
        _CLDELETE(this->writeLock);
        _CLTHROWA(CL_ERR_IO,"IndexReader out of date and no longer valid for delete, undelete, or setNorm operations");
      }
    }
  }
  

  void IndexReader::setNorm(int32_t doc, const TCHAR* field, float_t value){
     setNorm(doc, field, CL_NS(search)::Similarity::encodeNorm(value));
  }

  bool IndexReader::indexExists(const Directory* directory){
  //Func - Static method
  //       Checks if an index exists in the directory
  //Pre  - directory is a valid reference
  //Post - Returns true if an index exists at the specified directory->
  //       If the directory does not exist or if there is no index in it.
  //       false is returned.

      return directory->fileExists("segments");
  }

  TermDocs* IndexReader::termDocs(Term* term) const {
  //Func - Returns an enumeration of all the documents which contain
  //       term. For each document, the document number, the frequency of
  //       the term in that document is also provided, for use in search scoring.
  //       Thus, this method implements the mapping: 
  //
  //       Term => <docNum, freq>*
  //	   The enumeration is ordered by document number.  Each document number
  //       is greater than all that precede it in the enumeration. 
  //Pre  - term != NULL
  //Post - A reference to TermDocs containing an enumeration of all found documents
  //       has been returned

      CND_PRECONDITION(term != NULL, "term is NULL");

      //Reference an instantiated TermDocs instance
      TermDocs* _termDocs = termDocs();
      //Seek all documents containing term
      _termDocs->seek(term);
      //return the enumaration
      return _termDocs;
  }

  TermPositions* IndexReader::termPositions(Term* term) {
  //Func - Returns an enumeration of all the documents which contain  term. For each 
  //       document, in addition to the document number and frequency of the term in 
  //       that document, a list of all of the ordinal positions of the term in the document 
  //       is available.  Thus, this method implements the mapping:
  //
  //       Term => <docNum, freq,<pos 1, pos 2, ...pos freq-1>>*
  //
  //       This positional information faciliates phrase and proximity searching.
  //       The enumeration is ordered by document number.  Each document number is greater than 
  //       all that precede it in the enumeration. 
  //Pre  - term != NULL
  //Post - A reference to TermPositions containing an enumeration of all found documents
  //       has been returned

      CND_PRECONDITION(term != NULL, "term is NULL");

      //Reference an instantiated termPositions instance
      TermPositions* _termPositions = termPositions();
	  //Seek all documents containing term
      _termPositions->seek(term);
	  //return the enumeration
      return _termPositions;
  }

#ifndef CLUCENE_LITE
  void IndexReader::deleteDoc(const int32_t docNum) {
  //Func - Deletes the document numbered docNum.  Once a document is deleted it will not appear 
  //       in TermDocs or TermPostitions enumerations. Attempts to read its field with the document 
  //       method will result in an error.  The presence of this document may still be reflected in 
  //       the docFreq statistic, though this will be corrected eventually as the index is further modified.  
  //Pre  - docNum >= 0
  //Post - If successful the document identified by docNum has been deleted. If no writelock
  //       could be obtained an exception has been thrown stating that the index was locked or has no write access

	  SCOPED_LOCK_MUTEX(Delete_LOCK);

     CND_PRECONDITION(docNum >= 0, "docNum is negative");

      if (directoryOwner)
		  aquireWriteLock();

	  //Have the document identified by docNum deleted
      doDelete(docNum);
      hasChanges = true;
  }

  /**
   * Commit changes resulting from delete, undeleteAll, or setNorm operations
   * 
   * @throws IOException
   */
   void IndexReader::commit(){
    SCOPED_LOCK_MUTEX(commitLock);
    if(hasChanges){
      if(directoryOwner){
        LOCK_MUTEX(directory->DIR_OBJ);      // in- & inter-process sync

        LuceneLock* commitLock = directory->makeLock("commit.lock");
        IndexReaderCommitLockWith cl(commitLock,this);
        cl.run();
		_CLDELETE(commitLock);

        UNLOCK_MUTEX(directory->DIR_OBJ);
        if (writeLock != NULL) {
          writeLock->release();  // release write lock
          _CLDELETE(writeLock);
        }
      }else
        doCommit();
    }
    hasChanges = false;
  }


  void IndexReader::undeleteAll(){
     SCOPED_LOCK_MUTEX(lockUndeleteAll);
    if(directoryOwner)
      aquireWriteLock();
    doUndeleteAll();
    hasChanges = true;
  }

  int32_t IndexReader::deleteTerm(Term* term) {
  //Func - Deletes all documents containing term. This is useful if one uses a 
  //       document field to hold a unique ID string for the document.  Then to delete such  
  //       a document, one merely constructs a term with the appropriate field and the unique 
  //       ID string as its text and passes it to this method.  
  //Pre  - term != NULL
  //Post - All documents containing term have been deleted. The number of deleted documents
  //       has been returned

      CND_PRECONDITION(term != NULL, "term is NULL");

	  //Search for the documents contain term
      TermDocs* docs = termDocs(term);

	  //Check if documents have been found
	  if ( docs == NULL ){
          return 0;
	      }
    
	  //initialize
	  int32_t Counter = 0;
      try {
		  //iterate through the found documents
          while (docs->next()) {
			  //Delete the document
              deleteDoc(docs->doc());
              Counter++;
              }
          }
	  _CLFINALLY(
		  //Close the enumeration
          docs->close();
          );

    //Delete the enumeration of found documents
    _CLVDELETE( docs ); //todo: not a clucene object... should be

	//Return the number of deleted documents
    return Counter;
  }
#endif //ifndef CLUCENE_LITE

  void IndexReader::close() {
  //Func - Closes files associated with this index and also saves any new deletions to disk.
  //       No other methods should be called after this has been called.
  //Pre  - true
  //Post - All files associated with this index have been deleted and new deletions have been 
  //       saved to disk
    SCOPED_LOCK_MUTEX(close_LOCK);
    //directory->refDec();

    commit();
    doClose();

	if(closeDirectory){
      directory->close();
	  _CLDECDELETE(directory);
	}
  }
   
  bool IndexReader::isLocked(Directory* directory) {
  //Func - Static method 
  //       Checks if the index in the directory is currently locked.
  //Pre  - directory is a valid reference to a directory to check for a lock
  //Post - Returns true if the index in the named directory is locked otherwise false

	  //Check the existence of the file write.lock and return true when it does and false
	  //when it doesn't
     LuceneLock* l1 = directory->makeLock("write.lock");
     LuceneLock* l2 = directory->makeLock("commit.lock");

	 bool ret = l1->isLocked() || l2->isLocked();

     _CLDELETE(l1);
     _CLDELETE(l2);
     return ret;
  }

  bool IndexReader::isLocked(const char* directory) {
  //Func - Static method 
  //       Checks if the index in the named directory is currently locked.
  //Pre  - directory != NULL and contains the directory to check for a lock
  //Post - Returns true if the index in the named directory is locked otherwise false

      CND_PRECONDITION(directory != NULL, "directory is NULL");

	  //Create a buffer of length CL_MAXDIR
      char f[CL_MAX_DIR];
	  //Copy the directory string to the buffer
      strcpy(f,directory);
	  //Cat the name of the write.lock file to buffer
      strcat ( f,"/write.lock" );

      Directory* dir = FSDirectory::getDirectory(directory,false);
      bool ret = isLocked(dir);
	  dir->close();
      _CLDECDELETE(dir);

	  return ret;
  }

#ifndef CLUCENE_LITE

  void IndexReader::unlock(const char* path){
	  FSDirectory* dir = FSDirectory::getDirectory(path,false);
	  unlock(dir);
	  dir->close();
	  _CLDECDELETE(dir);
  }
  void IndexReader::unlock(Directory* directory){
  //Func - Static method
  //       Forcibly unlocks the index in the named directory->
  //       Caution: this should only be used by failure recovery code,
  //       when it is known that no other process nor thread is in fact
  //       currently accessing this index.
  //Pre  - directory is a valid reference to a directory 
  //Post - The directory has been forcibly unlocked
      LuceneLock* lock;

	  lock = directory->makeLock("write.lock");
      lock->release();
      _CLDELETE(lock);

      lock = directory->makeLock("commit.lock");
      lock->release();
      _CLDELETE(lock);
  }




	//Constructor	
    IndexReader::IndexReaderLockWith::IndexReaderLockWith(CL_NS(store)::LuceneLock* lock, CL_NS(store)::Directory* dir, const bool closeDir):
		CL_NS(store)::LuceneLockWith(lock,LUCENE_COMMIT_LOCK_TIMEOUT),
		closeDirectory(closeDir)
	{
		this->directory = dir;
	}	
	//Constructor	
	IndexReader::IndexReaderCommitLockWith::IndexReaderCommitLockWith( CL_NS(store)::LuceneLock* lock, IndexReader* r ):
		CL_NS(store)::LuceneLockWith(lock,LUCENE_COMMIT_LOCK_TIMEOUT),
		reader(r)
	{
	}
	void* IndexReader::IndexReaderCommitLockWith::doBody(){
		reader->doCommit();
		reader->segmentInfos->write(reader->getDirectory());
		return NULL;
	}
#endif //CLUCENE_LITE

CL_NS_END
