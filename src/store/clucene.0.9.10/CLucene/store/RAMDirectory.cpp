#include "CLucene/StdHeader.h"
#include "RAMDirectory.h"

#include "Lock.h"
#include "Directory.h"
#include "FSDirectory.h"
#include "CLucene/util/VoidMap.h"
#include "CLucene/util/Misc.h"
#include "CLucene/debug/condition.h"

CL_NS_USE(util)
CL_NS_DEF(store)

  RAMFile::RAMFile()
  {
     length = 0;
     lastModified = Misc::currentTimeMillis();
  }
  RAMFile::~RAMFile(){
  }



#ifndef CLUCENE_LITE
  RAMDirectory::RAMLock::RAMLock(const char* name, RAMDirectory* dir):
    fname ( STRDUP_AtoA(name) ),
    directory(dir)
  {
  }
  RAMDirectory::RAMLock::~RAMLock()
  {
    _CLDELETE_LCaARRAY( fname );
  }
  TCHAR* RAMDirectory::RAMLock::toString(){
	  return STRDUP_TtoT(_T("LockFile@RAM"));
  }
  bool RAMDirectory::RAMLock::isLocked() {
   return directory->fileExists(fname);
  }
  bool RAMDirectory::RAMLock::obtain(){
    SCOPED_LOCK_MUTEX(directory->files_mutex);
    if (!directory->fileExists(fname)) {
        OutputStream* tmp = directory->createFile(fname);
        tmp->close();
        _CLDELETE(tmp);

      return true;
    }
    return false;
  }

  void RAMDirectory::RAMLock::release(){
    directory->deleteFile(fname);
  }


  RAMOutputStream::~RAMOutputStream(){
     if ( deleteFile )
        _CLDELETE(file);
  }
  RAMOutputStream::RAMOutputStream(RAMFile* f):file(f) {
    pointer = 0;
    deleteFile = false;
  }
  
  RAMOutputStream::RAMOutputStream():
     file(_CLNEW RAMFile)
  {
     pointer = 0;
     deleteFile = true;
  }

  void RAMOutputStream::writeTo(OutputStream* out){
    flush();
    int64_t end = file->length;
    int64_t pos = 0;
    int32_t buffer = 0;
    while (pos < end) {
      int32_t length = LUCENE_STREAM_BUFFER_SIZE;
      int64_t nextPos = pos + length;
      if (nextPos > end) {                        // at the last buffer
        length = (int32_t)(end - pos);
      }
      out->writeBytes((uint8_t*)file->buffers[buffer++], length);
      pos = nextPos;
    }
  }

  void RAMOutputStream::reset(){
	seek(0);
    file->length = 0;
  }

  void RAMOutputStream::flushBuffer(const uint8_t* src, const int32_t len) {
    uint32_t bufferNumber = pointer/LUCENE_STREAM_BUFFER_SIZE;
    int32_t bufferOffset = pointer%LUCENE_STREAM_BUFFER_SIZE;
    int32_t bytesInBuffer = LUCENE_STREAM_BUFFER_SIZE - bufferOffset;
    int32_t bytesToCopy = bytesInBuffer >= len ? len : bytesInBuffer;

		if (bufferNumber == file->buffers.size()){
		  	uint8_t* tmp = _CL_NEWARRAY(uint8_t,LUCENE_STREAM_BUFFER_SIZE);
	      file->buffers.push_back( tmp );
		}

    uint8_t* buffer = file->buffers[bufferNumber];
    _arrays.arraycopy(src, 0, buffer, bufferOffset, bytesToCopy);

    if (bytesToCopy < len) {			  // not all in one buffer
      int32_t srcOffset = bytesToCopy;
      bytesToCopy = len - bytesToCopy;		  // remaining bytes
      bufferNumber++;
	  if (bufferNumber == file->buffers.size()){
	    uint8_t* tmp = _CL_NEWARRAY(uint8_t,LUCENE_STREAM_BUFFER_SIZE);
        file->buffers.push_back(tmp);
	  }
      buffer = file->buffers[bufferNumber];
      _arrays.arraycopy(src, srcOffset, buffer, 0, bytesToCopy);
    }
    pointer += len;
    if (pointer > file->length)
      file->length = pointer;

    file->lastModified = Misc::currentTimeMillis();
  }

  void RAMOutputStream::close() {
    OutputStream::close();
  }

  /** Random-at methods */
  void RAMOutputStream::seek(const int64_t pos){
    OutputStream::seek(pos);
    pointer = (int32_t)pos;
  }
  int64_t RAMOutputStream::Length() {
    return file->length;
  }
#endif // CLUCENE_LITE




  RAMInputStream::RAMInputStream(RAMFile* f):file(f) {
    pointer = 0;
    _length = f->length;
  }
  RAMInputStream::RAMInputStream(RAMInputStream& clone):
    file(clone.file),
    BufferedIndexInput(clone)
  {
    pointer = clone.pointer;
    _length = clone._length;
  }
  RAMInputStream::~RAMInputStream(){
      close();
  }
  InputStream* RAMInputStream::clone()
  {
    RAMInputStream* ret = _CLNEW RAMInputStream(*this);
    return ret;
  }

  void RAMInputStream::readInternal(uint8_t* dest, int32_t destOffset, const int32_t len) {
    const int64_t bytesAvailable = file->length - pointer;
    int64_t remainder = len <= bytesAvailable ? len : bytesAvailable;
    int32_t start = pointer;
    while (remainder != 0) {
      int32_t bufferNumber = start / LUCENE_STREAM_BUFFER_SIZE;
      int32_t bufferOffset = start % LUCENE_STREAM_BUFFER_SIZE;
      int32_t bytesInBuffer = LUCENE_STREAM_BUFFER_SIZE - bufferOffset;

	  /* The buffer's entire length (bufferLength) is defined by InputStream.h
      ** as int32_t, so obviously the number of bytes in a given segment of the
      ** buffer won't exceed the the capacity of int32_t.  Therefore, the
      ** int64_t->int32_t cast on the next line is safe. */
      int32_t bytesToCopy = bytesInBuffer >= remainder ? static_cast<int32_t>(remainder) : bytesInBuffer;
      uint8_t* buffer = (uint8_t*)file->buffers[bufferNumber];

      _arrays.arraycopy(buffer, bufferOffset, dest, destOffset, bytesToCopy);

      destOffset += bytesToCopy;
      start += bytesToCopy;
      remainder -= bytesToCopy;
      pointer += bytesToCopy;
    }
  }

  void RAMInputStream::close() {
    InputStream::close();
  }

  void RAMInputStream::seekInternal(const int64_t pos) {
	  CND_PRECONDITION(pos>=0 &&pos<this->_length,"Seeking out of range")
    pointer = (int32_t)pos;
  }






  char** RAMDirectory::list() const{
    int32_t size = 0;
    char** list = _CL_NEWARRAY(char*,files.size()+1);

	FileMap::const_iterator itr = files.begin();
    while (itr != files.end()){
	  list[size] = lucenestrdup((char*)itr->first CL_FILELINE);
      itr++;
      size++;
    }
    list[size]=NULL;
    return list;
  }

  RAMDirectory::RAMDirectory():
   Directory(),files(true,true)
  {
  }
  
  RAMDirectory::~RAMDirectory(){
   //todo: should call close directory?
  }

  void RAMDirectory::_copyFromDir(Directory* dir, bool closeDir)
  {
    char** files = dir->list();
    int i=0;
    while ( files[i] != NULL ){
      // make place on ram disk
      OutputStream* os = createFile(files[i]);
      // read current file
      InputStream* is = dir->openFile(files[i]);
      // and copy to ram disk
      //todo: this could be a problem when copying from big indexes... but what's 
      //the chance that someone's going to copy more than 2gb from disk into RAM...
      //actually, it is possible...
      int32_t len = is->length();
      uint8_t* buf = _CL_NEWARRAY(uint8_t,len);
      is->readBytes(buf, 0, len);
      os->writeBytes(buf, len);
      _CLDELETE_ARRAY(buf);

      // graceful cleanup
      is->close();
      _CLDELETE(is);
      os->close();
      _CLDELETE(os);

      _CLDELETE_LCaARRAY(files[i]);
      i++;
    }
    _CLDELETE_ARRAY(files);

    if (closeDir)
       dir->close();
  }
  RAMDirectory::RAMDirectory(Directory* dir):
   Directory(),files(true,true)
  {
    _copyFromDir(dir,false);
    
  }
  
   RAMDirectory::RAMDirectory(const char* dir):
      Directory(),files(true,true)
   {
      Directory* fsdir = FSDirectory::getDirectory(dir,false);
      try{
         _copyFromDir(fsdir,false);
      }_CLFINALLY(fsdir->close());

   }

  bool RAMDirectory::fileExists(const char* name) const {
    return files.exists(name);
  }

  int64_t RAMDirectory::fileModified(const char* name) const {
	  const RAMFile* f = files.get(name);
	  return f->lastModified;
  }

  int64_t RAMDirectory::fileLength(const char* name) const{
	  RAMFile* f = files.get(name);
      return f->length;
  }


  InputStream* RAMDirectory::openFile(const char* name) {
    RAMFile* file = files.get(name);
    if (file == NULL) { /* DSR:PROPOSED: Better error checking. */
      _CLTHROWA(CL_ERR_IO,"[RAMDirectory::open] The requested file does not exist.");
    }
    return _CLNEW RAMInputStream( file );
  }

  void RAMDirectory::close(){
      files.clear();
  }

#ifndef CLUCENE_LITE

  void RAMDirectory::deleteFile(const char* name, const bool throwError) {
    files.remove(name);
  }

  void RAMDirectory::renameFile(const char* from, const char* to) {
	FileMap::iterator itr = files.find(from);

    /* DSR:CL_BUG_LEAK:
    ** If a file named $to already existed, its old value was leaked.
    ** My inclination would be to prevent this implicit deletion with an
    ** exception, but it happens routinely in CLucene's internals (e.g., during
    ** IndexWriter.addIndexes with the file named 'segments'). */
    if (files.exists(to)) {
      files.remove(to);
    }
		RAMFile* file = itr->second;
    files.removeitr(itr,false,true);
    files.put(STRDUP_AtoA(to), file);
  }

  
  void RAMDirectory::touchFile(const char* name) {
    RAMFile* file = files.get(name);
    int64_t ts1 = Misc::currentTimeMillis();
    int64_t ts2 = ts1;
    do {
        _sleep(1);
        ts2 = Misc::currentTimeMillis();
    } while(ts1 == ts2);
    //todo: what's the point of this loop??? to ensure that time is not the same?

    file->lastModified = ts2;
  }

  OutputStream* RAMDirectory::createFile(const char* name) {
    /* Check the $files VoidMap to see if there was a previous file named
    ** $name.  If so, delete the old RAMFile object, but reuse the existing
    ** char buffer ($n) that holds the filename.  If not, duplicate the
    ** supplied filename buffer ($name) and pass ownership of that memory ($n)
    ** to $files. */
    const char* n = files.getKey(name);
    if (n != NULL) {
	   RAMFile* rf = files.get(name);
      _CLDELETE(rf);
    } else {
      n = STRDUP_AtoA(name);
    }

    RAMFile* file = _CLNEW RAMFile();
    #ifdef _DEBUG
      file->filename = n;
    #endif
    files[n] = file;

    OutputStream* ret = _CLNEW RAMOutputStream(file);
    return ret;
  }

  LuceneLock* RAMDirectory::makeLock(const char* name) {
    return _CLNEW RAMLock(name,this);
  }
#endif //CLUCENE_LITE

CL_NS_END
