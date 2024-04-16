#ifndef _lucene_util_Reader_
#define _lucene_util_Reader_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

CL_NS_DEF(util)

//todo: create buffered reader, split buffereing into this class
//move encoding down lower, so different types of readers can use the encoding
//i think there are a lot of slow parts to this code... look into that

  class Reader:LUCENE_BASE{
  protected:
		DEFINE_MUTEX(THIS_LOCK);
  public:
    virtual void close() = 0;
    virtual int32_t read(TCHAR* b, const int64_t start, const int32_t length) = 0;
		/* the available value may be greater than the actual value if the encoding
		 * is a variable one (such as utf8 or unicode) */
    virtual int64_t available () = 0;
    virtual TCHAR readChar() = 0;
    virtual TCHAR peek() = 0;
    virtual int64_t position() = 0;
    virtual void seek(int64_t position) = 0;
    virtual ~Reader(){
    }
	
    LUCENE_STATIC_CONSTANT(int32_t, ENCODING_ID_ASCII=1);
	LUCENE_STATIC_CONSTANT(int32_t, ENCODING_ID_UTF8=2);
	LUCENE_STATIC_CONSTANT(int32_t, ENCODING_ID_UCS2_LITTLE=3);
	LUCENE_STATIC_CONSTANT(int32_t, ENCODING_ID_UCS2=3);
	
    static const char* ENCODING_ASCII;
	static const char* ENCODING_UTF8;
	static const char* ENCODING_UCS2_LITTLE;
	static const char* ENCODING_UCS2;
  };

  class StringReader:public Reader{
private:
    TCHAR* data;
    uint32_t pt;
    size_t len;
  public:
    StringReader ( const TCHAR* value );
    StringReader ( const TCHAR* value, const size_t length );
    ~StringReader();

    int64_t available ();
    void seek(int64_t position);
	int64_t position();
    void close();
    TCHAR readChar();
	int32_t read(TCHAR* b, const int64_t start, const int32_t length);
    TCHAR peek();
  };

  #define FILEREADER_CACHE 64 //sive of cache size
  #define FILEREADER_CACHEBUFFER 6 //when reading, and only this value is left, 
								   //then refill is called again.
  #define FILEREADER_CACHEREMAINDER 6 //when reading, and only this value is left, 
								   //then refill is called again.
  class FileReader:public Reader{
  protected:
    uint8_t* cache;
    uint8_t* cacheremainder; //a temporary cache remainder is copied into this before cache is re-read. 
								//if the cache goes over a encoding boundary, there may be 1 or 2 character

	uint8_t filled; //whether the cache has been filled yet
	int32_t cachepos; //current position inside the cache
	int32_t cachelen; //length of the cache buffer (can be different to the value specified if the scheme requires it
	int32_t cachelenused; //length of the cache that is actually used
	int32_t cachebuff; //length of the cacheremainderbuffer
	int32_t cachestart; //this indicates the position of the start of the new cache
					  //within the cache. if refill is called, the cache that has not
					  //been read will be moved to the beginning, so the cache
					  //is not always all new
	int64_t cachefpos; //the fpos of the start of the cache - takes account the
					 //cache start issue

	int64_t filelen;
	int32_t fhandle;
	int64_t fpos;
	
	int32_t encoding_id;

	void refill();
  public:
    FileReader ( const char* fname, const char* encoding_scheme=NULL,const int32_t cachelen=FILEREADER_CACHE, const int32_t cachebuff=FILEREADER_CACHEBUFFER );
    ~FileReader();

    TCHAR readChar();
	 int32_t read(TCHAR* b, const int64_t start, const int32_t length);
    TCHAR peek();
    int64_t available();
    void close();
    int64_t position();
    void seek(int64_t position);
	
  };

  class FileReader_ThreadUnsafe:public FileReader{ 
  public:
     FileReader_ThreadUnsafe ( const char* fname, const char* encoding_scheme=NULL,const int32_t cachelen=FILEREADER_CACHE, const int32_t cachebuff=FILEREADER_CACHEBUFFER );
    
    TCHAR readChar();
	 int32_t read(TCHAR* b, const int64_t start, const int32_t length);
    TCHAR peek();
    int64_t available();
    int64_t position();
    void seek(int64_t position);
  };
  
CL_NS_END
#endif
