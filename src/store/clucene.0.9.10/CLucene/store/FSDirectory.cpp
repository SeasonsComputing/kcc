#include "CLucene/StdHeader.h"
#include "FSDirectory.h"
#include "CLucene/util/Misc.h"
#include "CLucene/util/MD5Digester.h"
#include "CLucene/debug/condition.h"

#include "CLucene/util/dirent.h" //if we have dirent, then the native one will be used

CL_NS_DEF(store)
CL_NS_USE(util)

   /** This cache of directories ensures that there is a unique Directory
   * instance per path, so that synchronization on the Directory can be used to
   * synchronize access between readers and writers.
   */
	static CL_NS(util)::CLHashMap<const char*,FSDirectory*,CL_NS(util)::Compare::Char> DIRECTORIES(false,false);
	DEFINE_MUTEX(DIRECTORIES_MUTEX);
	
	FSDirectory::FSIndexInput::FSIndexInput(const char* path) {
	//Func - Constructor.
	//       Opens the file named path
	//Pre  - path != NULL
	//Post - if the file could not be opened  an exception is thrown.

	  CND_PRECONDITION(path != NULL, "path is NULL");

	  //Keep in mind that this instance is not a clone
	  isClone = false;
	  
	  strcpy(this->path,path);

	  //Open the file
	  fhandle  = _open(path, O_BINARY | O_RDONLY | O_RANDOM, _S_IREAD );
	  
	  //Check if a valid handle was retrieved
	  if (fhandle < 0){
		int err = errno;
        if ( err == ENOENT )
		    _CLTHROWA(CL_ERR_IO, "File does not exist");
        else if ( err == EACCES )
            _CLTHROWA(CL_ERR_IO, "File Access denied");
        else if ( err == EMFILE )
            _CLTHROWA(CL_ERR_IO, "Too many open files");
	  }

	  //Store the file length
	  _length = fileSize(fhandle);
  }

  FSDirectory::FSIndexInput::FSIndexInput(FSIndexInput& clone): BufferedIndexInput(clone){
  //Func - Constructor
  //       Uses clone for its initialization
  //Pre  - clone is a valide instance of FSIndexInput
  //Post - The instance has been created and initialized by clone

	  //note: todo: i have changed the way cloning works. cloning meant that
	  //every read had to check the file position. this is a bit of an expensive
	  //operation, and was slowing down reading. the tradeoff now is that we
	  //have more open file handles. if people start having issues with filehandles
	  //we might have to revisit this.
	  strcpy(path,clone.path);

	  fhandle = _open(path, O_BINARY | O_RDONLY | O_RANDOM, _S_IREAD );
	  //Check if a valid handle was retrieved
	  if (fhandle < 0){
		int err = errno;
        if ( err == ENOENT )
		    _CLTHROWA(CL_ERR_IO, "File does not exist");
        else if ( err == EACCES )
            _CLTHROWA(CL_ERR_IO, "File Access denied");
        else if ( err == EMFILE )
            _CLTHROWA(CL_ERR_IO, "Too many open files");
		else
			_CLTHROWA(CL_ERR_IO, "File IO Error");
	  }
	  
	  //seek to the same location
	  int64_t fpos = fileTell(clone.fhandle);
	  if (fpos == -1 )
		_CLTHROWA( CL_ERR_IO, "File IO tell error");

	  if ( fileSeek(fhandle,fpos,SEEK_SET) == -1 )
		_CLTHROWA(CL_ERR_IO,"File IO Seek error");

	  //clone the file length
	  _length  = clone._length;
	  //Keep in mind that this instance is a clone
	  isClone = true;
  }

  FSDirectory::FSIndexInput::~FSIndexInput(){
  //Func - Destructor
  //Pre  - True
  //Post - The file for which this instance is responsible has been closed.
  //       The instance has been destroyed

	  close();
  }

  InputStream* FSDirectory::FSIndexInput::clone()
  {
    return _CLNEW FSDirectory::FSIndexInput(*this);
  }
  void FSDirectory::FSIndexInput::close()  {
    InputStream::close();
    if ( fhandle >= 0 ){
      if ( _close(fhandle) != 0 )
        _CLTHROWA(CL_ERR_IO, "File IO Close error");
      else
        fhandle = -1; //bvk: setting to 0 doesnt really indicate that it is closed
    }
  }

  void FSDirectory::FSIndexInput::seekInternal(const int64_t position)  {
	CND_PRECONDITION(position>=0 &&position<this->_length,"Seeking out of range")

	CND_PRECONDITION(fhandle>=0,"file is not open");
  	if ( fileSeek(fhandle,position,SEEK_SET) == -1 ){
		  _CLTHROWA(CL_ERR_IO, "File IO Seek error");
	}
  }

/** InputStream methods */
void FSDirectory::FSIndexInput::readInternal(uint8_t* b, int32_t offset, const int32_t len) {
   CND_PRECONDITION(fhandle>=0,"file is not open");
   SCOPED_LOCK_MUTEX(file_mutex);
     /*int64_t position = getFilePointer();

	  if (position != fileTell(fhandle) ) {
		if ( fileSeek(fhandle,position,SEEK_SET) == -1 ){
		  //UNLOCK_MUTEX(file_mutex);
		  _CLTHROWA( "File IO Seek error");
		}
	  }
	  //TODO check that this is right. Code is fairly different than
	  //the java code.*/
	  
	  bufferLength = _read(fhandle,b+offset,len); // 2004.10.31:SF 1037836
	  if (bufferLength == 0){
			_CLTHROWA(CL_ERR_IO, "read past EOF");
	  }
	  if (bufferLength == -1){
			_CLTHROWA(CL_ERR_IO, "read error");
	  }
  }

#ifndef CLUCENE_LITE
  FSDirectory::FSIndexOutput::FSIndexOutput(const char* path){
	//O_BINARY - Opens file in binary (untranslated) mode
	//O_CREAT - Creates and opens new file for writing. Has no effect if file specified by filename exists
	//O_RANDOM - Specifies that caching is optimized for, but not restricted to, random access from disk.
	//O_WRONLY - Opens file for writing only;
	if ( Misc::dir_Exists(path) )
	  fhandle = _open( path, O_BINARY | O_RDWR | O_RANDOM | O_TRUNC, _S_IREAD | _S_IWRITE);
	else // added by JBP
	  fhandle = _open( path, O_BINARY | O_RDWR | O_RANDOM | O_CREAT, _S_IREAD | _S_IWRITE);

	if ( fhandle < 0 ){
        int err = errno;
        if ( err == ENOENT )
    	    _CLTHROWA(CL_ERR_IO, "File does not exist");
        else if ( err == EACCES )
            _CLTHROWA(CL_ERR_IO, "File Access denied");
        else if ( err == EMFILE )
            _CLTHROWA(CL_ERR_IO, "Too many open files");
    }
  }
  FSDirectory::FSIndexOutput::~FSIndexOutput(){
	if ( fhandle >= 0 ){
	  try {
        close();
	  }catch(...){ //todo: only catch IO Err???
	  }
	}
  }

  /** output methods: */
  void FSDirectory::FSIndexOutput::flushBuffer(const uint8_t* b, const int32_t size) {
	  CND_PRECONDITION(fhandle>=0,"file is not open");
      if ( size > 0 && _write(fhandle,b,size) != size )
        _CLTHROWA(CL_ERR_IO, "File IO Write error");
  }
  void FSDirectory::FSIndexOutput::close() {
    try{
      OutputStream::close();
    }catch(...){} //todo: only catch IO Err???

    if ( _close(fhandle) != 0 )
      _CLTHROWA(CL_ERR_IO, "File IO Close error");
    else
      fhandle = -1; //-1 now indicates closed
  }

  void FSDirectory::FSIndexOutput::seek(const int64_t pos) {
    CND_PRECONDITION(fhandle>=0,"file is not open");
    OutputStream::seek(pos);
	int64_t ret = fileSeek(fhandle,pos,SEEK_SET);
	if ( ret != pos ){
      _CLTHROWA(CL_ERR_IO, "File IO Seek error");
	}
  }
  int64_t FSDirectory::FSIndexOutput::Length(){
	  CND_PRECONDITION(fhandle>=0,"file is not open");
	  return fileSize(fhandle);
  }
#endif //CLUCENE_LITE




	char* FSDirectory::LOCK_DIR=NULL;
	char* FSDirectory::getLockDir(){
	#ifdef LUCENE_LOCK_DIR
		LOCK_DIR = LUCENE_LOCK_DIR	
	#else	
		if ( LOCK_DIR == NULL ){
			#ifdef LUCENE_LOCK_DIR_ENV_1
				LOCK_DIR = getenv(LUCENE_LOCK_DIR_ENV_1);
			#endif			
			#ifdef LUCENE_LOCK_DIR_ENV_2
				if ( LOCK_DIR == NULL && getenv(LUCENE_LOCK_DIR_ENV_2) != NULL )
					LOCK_DIR = getenv(LUCENE_LOCK_DIR_ENV_2);
			#endif
			#ifdef LUCENE_LOCK_DIR_ENV_FALLBACK
				if ( LOCK_DIR == NULL )
					LOCK_DIR=LUCENE_LOCK_DIR_ENV_FALLBACK;
			#endif
		}
	#endif
		
		return LOCK_DIR;
	}

  FSDirectory::FSDirectory(const char* path, const bool createDir):
   Directory(),
   refCount(0),
   useMMap(true)
  {
    strcpy(directory,path);
    
	char* tmplockdir = getLockDir();
    if ( tmplockdir == NULL ) {
      strcpy(lockDir,directory);
    } else {
      strcpy(lockDir,tmplockdir);
    }

    if (createDir) {
      create();
    }

    if (!Misc::dir_Exists(directory)){
		char* err = _CL_NEWARRAY(char,19+strlen(path)+1); //19: len of " is not a directory"
		strcpy(err,path);
		strcat(err," is not a directory");
        _CLTHROWA(CL_ERR_IO, err );
    }
  }


  void FSDirectory::create(){
    SCOPED_LOCK_MUTEX(createLock);
	if ( !Misc::dir_Exists(directory) ){
       //todo: should construct directory using _mkdirs... have to write replacement
      if ( _mkdir(directory) == -1 ){
		  char* err = _CL_NEWARRAY(char,27+strlen(directory)+1); //27: len of "Couldn't create directory: "
		  strcpy(err,"Couldn't create directory: ");
		  strcat(err,directory);
		  _CLTHROWA_DEL(CL_ERR_IO, err );
      }
	}

	  //clear old files
    DIR* dir = opendir(directory);
    struct dirent* fl = readdir(dir);
    struct fileStat buf;

    char path[CL_MAX_DIR];
    while ( fl != NULL ){
      _snprintf(path,CL_MAX_DIR,"%s/%s",directory,fl->d_name);
      int32_t ret = fileStat(path,&buf);
      if ( ret==0 && !(buf.st_mode & S_IFDIR) ) {
        if ( (strcmp(fl->d_name, ".")) && (strcmp(fl->d_name, "..")) ) {
        if ( _unlink( path ) == -1 ) {
          closedir(dir);
          _CLTHROWA(CL_ERR_IO, "Couldn't delete file "); //todo: make richer error
        }
        }
      }
      fl = readdir(dir);
    }
    closedir(dir);

	char* lockPrefix = getLockPrefix(); // clear old locks
	int32_t lockPrefixLen = strlen(lockPrefix);
	
    dir = opendir(lockDir);
	fl = readdir(dir);
    while ( fl != NULL ){
		if (strncmp(fl->d_name,lockPrefix,lockPrefixLen)==0){
			_snprintf(path,CL_MAX_DIR,"%s/%s",lockDir,fl->d_name);
			if ( _unlink( path ) == -1 ) {
				closedir(dir);
				_CLDELETE_CaARRAY(lockPrefix);
				_CLTHROWA(CL_ERR_IO, "Couldn't delete file "); //todo: make richer error
			}
		}
      fl = readdir(dir);
    }
    closedir(dir);
    _CLDELETE_CaARRAY(lockPrefix);
  }

  void FSDirectory::priv_getFN(char* buffer, const char* name) const{
      buffer[0] = 0;
      strcpy(buffer,directory);
      strcat(buffer, PATH_DELIMITERA );
      strcat(buffer,name);
  }

  int32_t FSDirectory::priv_getStat(const char* name, struct fileStat* ret) const{
    char buffer[CL_MAX_DIR];
    priv_getFN(buffer,name);
    return fileStat( buffer, ret );
  }

  FSDirectory::~FSDirectory(){
  }

  char** FSDirectory::list() const{
    CND_PRECONDITION(directory[0]!=0,"directory is not open");
    DIR* dir = opendir(directory);
    
    struct dirent* fl = readdir(dir);
    struct fileStat buf;
    AStringArray names;

    char path[CL_MAX_DIR];
	strncpy(path,directory,CL_MAX_DIR);
    strcat(path,PATH_DELIMITERA);
    char* pathP = path + strlen(path);

    while ( fl != NULL ){
      strcpy(pathP,fl->d_name);
      fileStat(path,&buf);
      if ( buf.st_mode & S_IFDIR ) {
        names.put( STRDUP_AtoA(fl->d_name) );
      }
      fl = readdir(dir);
    }
    closedir(dir);

    int32_t size = names.size();
    char** list = _CL_NEWARRAY(char*,size+1);
    for ( int32_t i=0;i<size;i++ )
      list[i] = names[i];
    list[size]=NULL;
    return list;
  }

  bool FSDirectory::fileExists(const char* name) const {
	  CND_PRECONDITION(directory[0]!=0,"directory is not open");
    char fl[CL_MAX_DIR];
    priv_getFN(fl, name);
    return Misc::dir_Exists( fl );
  }

  const char* FSDirectory::getDirName() const{
    return directory;
  }

  //static
  FSDirectory* FSDirectory::getDirectory(const char* file, const bool create){
    FSDirectory* dir = NULL;
	LOCK_MUTEX(DIRECTORIES_MUTEX);
		dir = DIRECTORIES.get(file);
		if ( dir == NULL  ){
			dir = _CLNEW FSDirectory(file,create);
			DIRECTORIES.put( dir->directory, dir);
		}else if ( create ){
	    dir->create();
		}
	UNLOCK_MUTEX(DIRECTORIES_MUTEX);

	LOCK_MUTEX(dir->DIR_OBJ)
		dir->refCount++;
	UNLOCK_MUTEX(dir->DIR_OBJ)

    return _CL_POINTER(dir);
  }

  int64_t FSDirectory::fileModified(const char* name) const {
	CND_PRECONDITION(directory[0]!=0,"directory is not open");
    struct fileStat buf;
    if ( priv_getStat(name,&buf) == -1 )
      return 0;
    else
      return buf.st_mtime;
  }

  //static
  int64_t FSDirectory::fileModified(const char* dir, const char* name){
    struct fileStat buf;
    char buffer[CL_MAX_DIR];
	_snprintf(buffer,CL_MAX_DIR,"%s%s%s",dir,PATH_DELIMITERA,name);
    fileStat( buffer, &buf );
    return buf.st_mtime;
  }

  void FSDirectory::touchFile(const char* name){
	  CND_PRECONDITION(directory[0]!=0,"directory is not open");
    char buffer[CL_MAX_DIR];
    _snprintf(buffer,CL_MAX_DIR,"%s%s%s",directory,PATH_DELIMITERA,name);
	
	//todo: haven't checked this:
    int32_t r = _open(buffer, O_RDWR, _S_IWRITE);
	if ( r < 0 )
		_CLTHROWA(CL_ERR_IO,"IO Error while touching file");
	_close(r);
  }

  int64_t FSDirectory::fileLength(const char* name) const {
	  CND_PRECONDITION(directory[0]!=0,"directory is not open");
    struct fileStat buf;
    if ( priv_getStat(name, &buf) == -1 )
      return 0;
    else
      return buf.st_size;
  }


  InputStream* FSDirectory::openFile(const char* name) {
	  CND_PRECONDITION(directory[0]!=0,"directory is not open");
    char fl[CL_MAX_DIR];
    priv_getFN(fl, name);
#ifdef LUCENE_FS_MMAP
	//todo: do some tests here... like if the file
	//is >2gb, then some system cannot mmap the file
	//also windows could detect compressed ntfs files that cannot mmap'd?
	if ( useMMap && Misc::file_Size(fl) < LUCENE_INT32_MAX_SHOULDBE )
		return _CLNEW MMapInputStream( fl );
	else
		return _CLNEW FSIndexInput( fl );
#else
	return _CLNEW FSIndexInput( fl );
#endif
  }
		
  void FSDirectory::close(){
	  SCOPED_LOCK_MUTEX(closeLock);

	  CND_PRECONDITION(directory[0]!=0,"directory is not open");

      if (--refCount <= 1 ) {//refcount starts at 1
          LOCK_MUTEX(DIRECTORIES_MUTEX);
          Directory* dir = DIRECTORIES.get(getDirName());
          if(dir){
            DIRECTORIES.remove( getDirName() ); //this will be removed in ~FSDirectory
			  		_CLDECDELETE(dir);
          }
          UNLOCK_MUTEX(DIRECTORIES_MUTEX);
	   }
   }

   /**
   * So we can do some byte-to-hexchar conversion below
   */
	char HEX_DIGITS[] =
	{'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

	char* FSDirectory::getLockPrefix() {
		char dirName[CL_MAX_PATH]; // name to be hashed
		if ( _fullpath(dirName,directory,CL_MAX_PATH) == NULL ){
			_CLTHROWA(CL_ERR_Runtime,"Invalid directory path");
			return NULL;
		}
		int dirNameLen = strlen(dirName);

		//to make a compatible name with jlucene, we need to make some changes...
		if ( dirName[1] == ':' )
			dirName[0] = _totupper((char)dirName[0]);

		char* md5 = MD5String(dirName);

		char* ret=_CL_NEWARRAY(char,32+7+1); //32=2*16, 7=strlen("lucene-")
		strcpy(ret,"lucene-");
		strcat(ret,md5);
		
		_CLDELETE_CaARRAY(md5);

	    return ret; 
  }

#ifndef CLUCENE_LITE
  void FSDirectory::deleteFile(const char* name, const bool throwError)  {
	CND_PRECONDITION(directory[0]!=0,"directory is not open");
    char fl[CL_MAX_DIR];
    priv_getFN(fl, name);
	if ( _unlink(fl) == -1 && throwError){
      char buffer[200];
      _snprintf(buffer,200,"couldn't delete %s",name);
      _CLTHROWA(CL_ERR_IO, buffer );
    }
  }

  void FSDirectory::renameFile(const char* from, const char* to){
	  CND_PRECONDITION(directory[0]!=0,"directory is not open");
    SCOPED_LOCK_MUTEX(renameLock);
    char old[CL_MAX_DIR];
    priv_getFN(old, from);

    char nu[CL_MAX_DIR];
    priv_getFN(nu, to);

    /* This is not atomic.  If the program crashes between the call to
    delete() and the call to renameTo() then we're screwed, but I've
    been unable to figure out how else to do this... */

    if ( Misc::dir_Exists(nu) )
      if( _unlink(nu) != 0 ){
	    char* err = _CL_NEWARRAY(char,16+strlen(to)+1); //16: len of "couldn't delete "
		strcpy(err,"couldn't delete ");
		strcat(err,to);
        _CLTHROWA_DEL(CL_ERR_IO, err );
      }
    if ( _rename(old,nu) != 0 ){
       //todo: jlucene has some extra rename code - if the rename fails, it copies
       //the whole file to the new file... might want to implement that if renaming
       //fails on some platforms
        char buffer[200];
        strcpy(buffer,"couldn't rename ");
        strcat(buffer,from);
        strcat(buffer," to ");
        strcat(buffer,to);
      _CLTHROWA(CL_ERR_IO, buffer );
    }
  }

  OutputStream* FSDirectory::createFile(const char* name) {
	CND_PRECONDITION(directory[0]!=0,"directory is not open");
    char fl[CL_MAX_DIR];
    priv_getFN(fl, name);
    return _CLNEW FSIndexOutput( fl );
  }

  LuceneLock* FSDirectory::makeLock(const char* name) {
	CND_PRECONDITION(directory[0]!=0,"directory is not open");

	char* tmp = getLockPrefix();
	char* lockFile = _CL_NEWARRAY(char,strlen(tmp)+strlen(name)+2);
	strcpy(lockFile,tmp);
	strcat(lockFile,"-");
	strcat(lockFile,name);
	_CLDELETE_CaARRAY(tmp);

    // create a lock file
    LuceneLock* ret = _CLNEW FSLock( lockDir, lockFile );
	_CLDELETE_CaARRAY(lockFile);
	return ret;
  }



  FSDirectory::FSLock::FSLock ( const char* lockDir, const char* name )
  {
	  this->lockDir = STRDUP_AtoA(lockDir);
	  strcpy(lockFile,lockDir);
	  strcat(lockFile,PATH_DELIMITERA);
	  strcat(lockFile,name);
  }
  FSDirectory::FSLock::~FSLock(){
    _CLDELETE_LCaARRAY( lockDir );
  }
  TCHAR* FSDirectory::FSLock::toString(){
	  int lflen = strlen(lockFile);
	  TCHAR* ret = _CL_NEWARRAY(TCHAR,lflen+6);
	  _tcscpy(ret,_T("Lock@"));
	  STRCPY_AtoT(ret+5,lockFile,lflen+1);
	  return ret;
  }
  bool FSDirectory::FSLock::obtain() {
   if (LUCENE_DISABLE_LOCKS)
      return true;

	if ( Misc::dir_Exists(lockFile) )
	  return false;

	if ( !Misc::dir_Exists(lockDir) ){
       //todo: should construct directory using _mkdirs... have to write replacement
      if ( _mkdir(lockDir) == -1 ){
		  char* err = _CL_NEWARRAY(char,34+strlen(lockDir)+1); //34: len of "Couldn't create lock directory: "
		  strcpy(err,"Couldn't create lock directory: ");
		  strcat(err,lockDir);
		  _CLTHROWA_DEL(CL_ERR_IO, err );
      }
    }
    int32_t r = _open(lockFile,  O_RDWR | O_CREAT | O_RANDOM , _S_IREAD | _S_IWRITE); //must do this or file will be created Read only
	if ( r < 0 )
	  return false;
	else{
	  _close(r);
	  return true;
	}

  }
  bool FSDirectory::FSLock::isLocked(){
     if (LUCENE_DISABLE_LOCKS)
          return false;

     return Misc::dir_Exists(lockFile);
  }
  void FSDirectory::FSLock::release() {
    if (LUCENE_DISABLE_LOCKS)
          return;
    _unlink( lockFile );
  }
#endif // CLUCENE_LITE

CL_NS_END
