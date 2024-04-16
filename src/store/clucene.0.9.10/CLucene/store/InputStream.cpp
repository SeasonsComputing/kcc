#include "CLucene/StdHeader.h"

#include "InputStream.h"
#include "CLucene/util/Arrays.h"

CL_NS_USE(util)
CL_NS_DEF(store)

	IndexInput::IndexInput():
		_length(0)
	{
	}
	IndexInput::IndexInput(const IndexInput& clone):
		_length(clone._length)
	{
	}
	
  int64_t IndexInput::length() {
    return _length;
  }


  InputStream::~InputStream(){
  }
  void InputStream::close(){
  }
  int32_t InputStream::readInt() {
    uint8_t b1 = readByte();
    uint8_t b2 = readByte();
    uint8_t b3 = readByte();
    uint8_t b4 = readByte();
    return ((b1 & 0xFF) << 24) | ((b2 & 0xFF) << 16) | ((b3 & 0xFF) <<  8)
         | (b4 & 0xFF);
  }

  int32_t InputStream::readVInt() {
    uint8_t b = readByte();
    int32_t i = b & 0x7F;
    for (int32_t shift = 7; (b & 0x80) != 0; shift += 7) {
      b = readByte();
      i |= (b & 0x7F) << shift;
    }
    return i;
  }

  int64_t InputStream::readLong() {
    int32_t i1 = readInt();
    int32_t i2 = readInt();
    return (((int64_t)i1) << 32) | (i2 & 0xFFFFFFFFL);
  }

  int64_t InputStream::readVLong() {
    uint8_t b = readByte();
    int64_t i = b & 0x7F;
    for (int32_t shift = 7; (b & 0x80) != 0; shift += 7) {
      b = readByte();
      i |= (b & 0x7FL) << shift;
    }
    return i;
  }

  int32_t InputStream::readString(TCHAR* buffer, const int32_t maxLength){
    int32_t len = readVInt();
	int32_t ml=maxLength-1;
    if ( len >= ml ){
      readChars(buffer, 0, ml);
      buffer[ml] = 0;
      //we have to finish reading all the data for this string!
      if ( len-ml > 0 )
         seek(getFilePointer()+(len-ml));
      return ml;
    }else{
      readChars(buffer, 0, len);
      buffer[len] = 0;
      return len;
    }
  }

   TCHAR* InputStream::readString(const bool unique){
    int32_t len = readVInt();
      
   if ( len == 0){
      if ( unique )
         return stringDuplicate(LUCENE_BLANK_STRING);
      else
         return LUCENE_BLANK_STRING;
   }

    TCHAR* ret = _CL_NEWARRAY(TCHAR,len+1);
    readChars(ret, 0, len);
    ret[len] = 0;

    return ret;
  }

  void InputStream::readChars( TCHAR* buffer, const int32_t start, const int32_t len) {
    const int32_t end = start + len;
    for (int32_t i = start; i < end; i++) {
      TCHAR b = readByte();
      if ((b & 0x80) == 0) {
        b = (b & 0x7F);
      } else if ((b & 0xE0) != 0xE0) {
        b = (((b & 0x1F) << 6)
          | (readByte() & 0x3F));
      } else {
          uint8_t b2 = readByte();
          uint8_t b3 = readByte();
		  b = (((b & 0x0F) << 12)
              | ((b2 & 0x3F) << 6)
              | (b3 & 0x3F));
      }
      buffer[i] = (TCHAR)b;
	}
  }





	BufferedIndexInput::BufferedIndexInput():
		bufferStart(0),
		bufferLength(0),
		bufferPosition(0),
		buffer(NULL)
  {
  }

  BufferedIndexInput::BufferedIndexInput(const BufferedIndexInput& clone):
    bufferStart(clone.bufferStart),
    bufferLength(clone.bufferLength),
    bufferPosition(clone.bufferPosition),
    buffer(NULL)
  {
    /* DSR: Does the fact that sometime clone.buffer is not NULL even when
    ** clone.bufferLength is zero indicate memory corruption/leakage?
    **   if ( clone.buffer != NULL) { */
    if (clone.bufferLength != 0 && clone.buffer != NULL) {
      buffer = _CL_NEWARRAY(uint8_t,bufferLength);
      _arrays.arraycopy( clone.buffer, 0, buffer, 0, bufferLength);
    }
  }

  uint8_t BufferedIndexInput::readByte() {
    if (bufferPosition >= bufferLength)
      refill();

    return buffer[bufferPosition++];
  }
 
  void BufferedIndexInput::readBytes(uint8_t* b, const int32_t offset, const int32_t len){
    if (len < LUCENE_STREAM_BUFFER_SIZE) {
      for (int32_t i = 0; i < len; i++)		  // read byte-by-byte
        b[i + offset] = (uint8_t)readByte();
    } else {					  // read all-at-once
      int64_t start = getFilePointer();
      seekInternal(start);
      readInternal(b, offset, len);

      bufferStart = start + len;		  // adjust stream variables
      bufferPosition = 0;
      bufferLength = 0;				  // trigger refill() on read
    }
  }

  int64_t BufferedIndexInput::getFilePointer() {
    return bufferStart + bufferPosition;
  }

  void BufferedIndexInput::seek(const int64_t pos) {
    if ( pos < 0 )
      _CLTHROWA(CL_ERR_IO, "IO Argument Error. Value must be a positive value.");
    if (pos >= bufferStart && pos < (bufferStart + bufferLength))
      bufferPosition = (int32_t)(pos - bufferStart);  // seek within buffer
    else {
      bufferStart = pos;
      bufferPosition = 0;
      bufferLength = 0;				  // trigger refill() on read()
      seekInternal(pos);
    }
  }
  void BufferedIndexInput::close(){
      if ( buffer != NULL ) {
      _CLDELETE_ARRAY(buffer);
      }

    bufferLength = 0;
    bufferPosition = 0;
    bufferStart = 0;
  }


  BufferedIndexInput::~BufferedIndexInput(){
    close();
  }

  void BufferedIndexInput::refill() {
    int64_t start = bufferStart + bufferPosition;
    int64_t end = start + LUCENE_STREAM_BUFFER_SIZE;
    if (end > _length)				  // don't read past EOF
      end = _length;
    bufferLength = (int32_t)(end - start);
    if (bufferLength == 0)
      _CLTHROWA(CL_ERR_IO, "InputStream read past EOF");

    if (buffer == NULL){
      buffer = _CL_NEWARRAY(uint8_t,LUCENE_STREAM_BUFFER_SIZE);		  // allocate buffer lazily
      //bufferLength = LUCENE_STREAM_BUFFER_SIZE;
    }
    readInternal(buffer, 0, bufferLength);


    bufferStart = start;
    bufferPosition = 0;
  }

CL_NS_END

