#include "CLucene/StdHeader.h"
#include "Reader.h"

CL_NS_DEF(util)

const char* Reader::ENCODING_ASCII="ASCII";
const char* Reader::ENCODING_UTF8="UTF8";
const char* Reader::ENCODING_UCS2_LITTLE="UCS2_LITTLE";
const char* Reader::ENCODING_UCS2="UCS2";

   ///Internal Reader function. Converts the string representation of a character
   ///encoding scheme to the internal integer representation
	int32_t SchemeToId(const char* encoding_scheme){
		if ( strcmp(encoding_scheme,Reader::ENCODING_ASCII) == 0 )
			return Reader::ENCODING_ID_ASCII;
		else if ( strcmp(encoding_scheme,Reader::ENCODING_UTF8) == 0 )
			return Reader::ENCODING_ID_UTF8;
		else if ( strcmp(encoding_scheme,Reader::ENCODING_UCS2) == 0 )
			return Reader::ENCODING_ID_UCS2;
		else if ( strcmp(encoding_scheme,Reader::ENCODING_UCS2_LITTLE) == 0 )
			return Reader::ENCODING_ID_UCS2_LITTLE;
		else
			_CLTHROWA(CL_ERR_Runtime,"Unknown encoding type");
	}
  
	///Internal Reader function. Takes an array of encoded bytes and converts it to characters
	int32_t _decode_buffer(TCHAR* outbuffer,int32_t outpos, int32_t count,      
						int32_t encoding_id, //bytes are encoded with this type 
						uint8_t* inbuffer,int32_t& inpos, int32_t inlength){
		
		if ( encoding_id == Reader::ENCODING_ID_ASCII ){
			int32_t origpos = outpos;
			int32_t inavail = inlength - inpos;
			int32_t outavail = count;
			if (outavail > inavail)
				outavail = inavail;

			while (--outavail >= 0)
			{
				int32_t c = inbuffer[inpos++];
				outbuffer[outpos++] = ((c > 0x7f) ? LUCENE_OOR_CHAR : (TCHAR)c);
			}
			return outpos - origpos;

		}else if ( encoding_id == Reader::ENCODING_ID_UCS2_LITTLE ){
			int32_t origoutpos = outpos;

			while(outpos<count+origoutpos){
				uint8_t c1 = inbuffer[inpos] & 0xFF;

				if ( inpos +1 >= inlength )
					break; //character goes over cache boundary

				uint8_t c2 = inbuffer[inpos+1] & 0xFF;
				inpos+=2;

				unsigned short c = c1 | (c2<<8);
#ifdef _UCS2
				outbuffer[outpos++] = (TCHAR) c;
#else
				outbuffer[outpos++] = ((c > 0x7f) ? LUCENE_OOR_CHAR : (TCHAR)c);
#endif
			}
			return outpos-origoutpos;

		}else if ( encoding_id == Reader::ENCODING_ID_UTF8 ){
			int32_t origoutpos = outpos;

			while(outpos<count+origoutpos){
				uint8_t c1 = inbuffer[inpos];
				int32_t cr = 0;
				if ( (c1 & 0x80) == 0 ){
					cr = (c1 & 0x7F);
					inpos++;

				} else if ( (c1 & 0xE0) != 0xE0){
					if ( inpos +1 >= inlength )
						break; //character goes over cache boundary

					cr = (((c1 & 0x1F) << 6)
											| (inbuffer[inpos+1] & 0x3F));
					inpos+=2;

				} else {
					if ( inpos +2 >= inlength )
						break; //character goes over cache boundary
					
					cr = (((c1 & 0x0F) << 12)
										  | ((inbuffer[inpos+1] & 0x3F) << 6 )
										  | (inbuffer[inpos+2] & 0x3F));
					inpos+=3;
				}
#ifdef _UCS2
				outbuffer[outpos++] = (TCHAR) cr;
#else
				outbuffer[outpos++] = ((cr > 0x7f) ? LUCENE_OOR_CHAR : (TCHAR)cr);
#endif
			}
			return outpos-origoutpos;

		}else
			_CLTHROWA(CL_ERR_Runtime,"Unknown encoding type");
	}



	FileReader::FileReader(const char* fname, const char* _encoding_scheme,const int32_t cachelen, const int32_t cachebuff)
	{
        char* encoding_scheme=(char*)_encoding_scheme;
        if ( encoding_scheme == NULL )
            this->encoding_id=SchemeToId(PLATFORM_DEFAULT_READER_ENCODING);
		else
			this->encoding_id=SchemeToId(encoding_scheme);

		this->fhandle = _open(fname, O_BINARY | O_RDONLY | O_RANDOM, _S_IREAD );
        if ( this->fhandle < 0  ){
            int err = errno;
            if ( err == ENOENT )
			    _CLTHROWA(CL_ERR_IO, "File does not exist");
            else if ( err == EACCES )
                _CLTHROWA(CL_ERR_IO, "File Access denied");
            else if ( err == EMFILE )
                _CLTHROWA(CL_ERR_IO, "Too many open files");
        }

        //todo: this should be clearer to developers... important
        //only multi-byte encodings need a cache buffer
		if ( this->encoding_id == Reader::ENCODING_ID_UTF8 ){
		   if ( cachebuff + 3 >= cachelen )
			   _CLTHROWA(CL_ERR_Runtime, "Cache must be more than 3 larger than the buffer size") ;
			 this->cachebuff = cachebuff;
		}else{
			 this->cachebuff=0;
        }
        filelen = fileSize(fhandle);
		this->fpos = 0;

		this->cachepos = 0;
		this->cachefpos = 0;
		this->cachelenused = 0;
		this->cachelen = cachelen;
		this->cachestart = 0;
		this->filled = 0;

		this->cache = _CL_NEWARRAY(uint8_t,this->cachelen);
		
		this->cacheremainder = NULL;
		if ( this->cachebuff > 0 )
			this->cacheremainder = _CL_NEWARRAY(uint8_t,this->cachebuff);
	}
	FileReader::~FileReader(){
		close();
	}
	void FileReader::close(){
		SCOPED_LOCK_MUTEX(THIS_LOCK);
		if ( fhandle != 0 )
			_close(fhandle);
		fhandle = 0;
		filelen=0;
		fpos=0;
		encoding_id=0;
		filled = 0;

		cachelenused=0;
		cachepos=0;
		cachelen=0;
		cachefpos=0;
		_CLDELETE_ARRAY(cache);
		_CLDELETE_ARRAY(cacheremainder);
	}

	void FileReader::refill(){
		if ( filled && this->cachebuff>0 ){
			//make sure that the remaining cache can fit into the cacheremainder array
			if ( cachelenused-cachepos > this->cachebuff )
				_CLTHROWA(CL_ERR_Runtime,"Refill called when cache position is not near the end of the cache");

			cachestart=cachelenused-cachepos;
			CND_PRECONDITION(cachestart>=0,"cachestart<0");
			for ( int32_t i=0;i<cachestart;i++ )
				cacheremainder[i]=cache[cachepos++];

			{//MSVC 6 scope fix
			for ( int32_t i=0;i<cachestart;i++ )
				cache[i]=cacheremainder[i];
			}
		}

		//copy the length of the cache minus the amount of cache
		//already used in the cacheremainder
		uint8_t* tmp = cache;
		tmp += cachestart;
		cachelenused = _read(fhandle,tmp,cachelen-cachestart);
		cachefpos = fpos - cachestart;
		fpos += cachelenused;
		cachelenused += cachestart;
		cachepos = 0;
		filled = 1;
	}
	int32_t FileReader::read(TCHAR* buf, const int64_t start, const int32_t length){
		SCOPED_LOCK_MUTEX(THIS_LOCK);
		int32_t rd = 0;
		//if you are within cachebuff of the end of the cache 
		//reread cache and set pointer to start of cache

		while ( rd < length && cachefpos+cachepos < filelen ){
			if (!filled || cachelenused - cachepos <= cachebuff)
				refill();
			int32_t t = _decode_buffer(buf,rd,length-rd,encoding_id,cache,cachepos,cachelenused);
			if ( t == -1 )
				_CLTHROWA(CL_ERR_IO,"File IO read error");
			if ( t == 0 )
				break;
			rd+=t;
		}

		return rd;
 	}

	int64_t FileReader::available(){
		SCOPED_LOCK_MUTEX(THIS_LOCK);
		if ( !filled )
			refill();
		int64_t ret = filelen - (cachefpos+cachepos);
		if ( ret<0 )
			return 0;
		else
			return ret;
	}
	TCHAR FileReader::readChar(){
		SCOPED_LOCK_MUTEX(THIS_LOCK);
		if (!filled || (cachelenused>0 && cachelenused - cachepos <= cachebuff) )
			refill();

		TCHAR tmp[2];
		if ( cachelenused == 0 )
			return 0;

		//won't need to refill, since there is a buffer between
		int32_t rd=_decode_buffer(tmp,0,1,encoding_id,cache,cachepos,cachelenused);
		if ( rd == 1 )
			return tmp[0];
		else
			return 0;
	}
	TCHAR FileReader::peek(){
		SCOPED_LOCK_MUTEX(THIS_LOCK);
		if ( !filled )
			refill();
		int32_t oldcachepos = cachepos;
		TCHAR tmp[2];
		//won't need to refill, since there is a buffer between
		int32_t rd=_decode_buffer(tmp,0,1,encoding_id,cache,cachepos,cachelenused);
		cachepos = oldcachepos;
		if ( rd == 1 )
			return tmp[0];
		else
			return 0;
	}
	int64_t FileReader::position(){
		SCOPED_LOCK_MUTEX(THIS_LOCK);
		if ( !filled )
			refill();
		return cachefpos+cachepos;
	}
	void FileReader::seek(int64_t position){
		SCOPED_LOCK_MUTEX(THIS_LOCK);
		if ( !filled )
			refill();
		if ( position < cachefpos ||
			position > cachefpos+cachelenused ){
			//must refill

			if ( fileSeek(fhandle,position,SEEK_SET) == -1 ){
				_CLTHROWA(CL_ERR_IO, "File IO Seek error");
			}
			fpos = position;
		    filled = 0;

			cachestart=0;
			cachelenused=0;
			cachepos=0;
			cachefpos=0;
		}else{
			cachepos = position-cachefpos;
		}
	}



	StringReader::StringReader ( const TCHAR* value, const size_t length ):
		len(length)
	{
		this->data = _CL_NEWARRAY(TCHAR,len+1);
		STRCPY_TtoT(data,value,len);
		this->pt = 0;
	}
	StringReader::StringReader( const TCHAR* value ){
		this->len = _tcslen(value);
		this->data = _CL_NEWARRAY(TCHAR,len+1);
		STRCPY_TtoT(data,value,len);
		this->pt = 0;
	}
	StringReader::~StringReader(){
		close();
	}

	int64_t StringReader::available(){
		return len-pt;
	}
	int32_t StringReader::read ( TCHAR* buf, const int64_t start, const int32_t length ){
		if ( pt >= len )
			return -1;
		int32_t rd = 0;
		while ( pt < len && rd < length ){
			buf[start+rd] = data[pt];
			rd ++;
			pt ++;
		}
		return rd;
	}
	TCHAR StringReader::readChar(){
		if ( pt>=len )
		{
			if (pt==len)
				return 0;
			//printf("StringReader throwing EOF %d/%d\n", pt, len);fflush(NULL); //todo: some printf debug code here...
			throw "String reader EOF";
		}
		TCHAR ret = data[pt];
		pt++;
		return ret;
	}
	
	TCHAR StringReader::peek(){
		if ( pt>=len )
		{
			if (pt==len)
				return 0;
			printf("StringReader throwing EOF %d/%d\n", pt, len);fflush(NULL);
			throw "String reader EOF";
		}
		return data[pt];
	}
	void StringReader::close(){
		 if (data != NULL )      
			_CLDELETE_CARRAY(data);
	}
	int64_t StringReader::position(){
		return pt;
	}
	void StringReader::seek(int64_t position){
		if (position > LUCENE_MAX_FILELENGTH ) {
  	     _CLTHROWA(CL_ERR_IO,"position parameter to StringReader::seek exceeds theoretical"
  	       " capacity of StringReader's internal buffer."
  	     );
  	    }
		pt=position;
	}





   FileReader_ThreadUnsafe::FileReader_ThreadUnsafe ( const char* fname, const char* encoding_scheme,const int32_t cachelen, const int32_t cachebuff ):
      FileReader(fname,encoding_scheme,cachelen,cachebuff)
   {
   }
    
   int32_t FileReader_ThreadUnsafe::read(TCHAR* buf, const int64_t start, const int32_t length){
		int32_t rd = 0;
		//if you are within cachebuff of the end of the cache 
		//reread cache and set pointer to start of cache

		while ( rd < length && cachefpos+cachepos < filelen ){
			if (!filled || cachelenused - cachepos <= cachebuff)
				refill();
			int32_t t = _decode_buffer(buf,rd,length-rd,encoding_id,cache,cachepos,cachelenused);
			if ( t == -1 )
				_CLTHROWA(CL_ERR_IO,"File IO read error");
			if ( t == 0 )
				break;
			rd+=t;
		}

		return rd;
 	}

	int64_t FileReader_ThreadUnsafe::available(){
		if ( !filled )
			refill();
		int64_t ret = filelen - (cachefpos+cachepos);
		if ( ret<0 )
			return 0;
		else
			return ret;
	}
	TCHAR FileReader_ThreadUnsafe::readChar(){
		if (!filled || cachelenused - cachepos <= cachebuff)
			refill();

		TCHAR tmp[2];
		//won't need to refill, since there is a buffer between
		int32_t rd=_decode_buffer(tmp,0,1,encoding_id,cache,cachepos,cachelenused);
		if ( rd == 1 )
			return tmp[0];
		else
			return 0;
	}
	TCHAR FileReader_ThreadUnsafe::peek(){
		if ( !filled )
			refill();
		int32_t oldcachepos = cachepos;
		TCHAR tmp[2];
		//won't need to refill, since there is a buffer between
		int32_t rd=_decode_buffer(tmp,0,1,encoding_id,cache,cachepos,cachelenused);
		cachepos = oldcachepos;
		if ( rd == 1 )
			return tmp[0];
		else
			return 0;
	}
	int64_t FileReader_ThreadUnsafe::position(){
		if ( !filled )
			refill();
		return cachefpos+cachepos;
	}
	void FileReader_ThreadUnsafe::seek(int64_t position){
		if ( !filled )
			refill();
		if ( position < cachefpos ||
			position > cachefpos+cachelenused ){
			//must refill

			if ( fileSeek(fhandle,position,SEEK_SET) == -1 ){
				_CLTHROWA(CL_ERR_IO, "File IO Seek error");
			}
			fpos = position;
		    filled = 0;

			cachestart=0;
			cachelenused=0;
			cachepos=0;
			cachefpos=0;
		}else{
			cachepos = position-cachefpos;
		}
	}

CL_NS_END

