#include "CLucene/StdHeader.h"
#ifndef CLUCENE_LITE
#include "OutputStream.h"

#include "CLucene/util/Arrays.h"

CL_NS_USE(util)
CL_NS_DEF(store)


  OutputStream::OutputStream()
  {
    buffer = _CL_NEWARRAY(uint8_t, LUCENE_STREAM_BUFFER_SIZE ); //todo: this could be allocated earlier
    bufferStart = 0;
    bufferPosition = 0;
  }

  OutputStream::~OutputStream(){
  }

  bool OutputStream::isClosed() const { /* DSR:PROPOSED */
    return buffer == NULL;
  }

  void OutputStream::close(){
    flush();
    _CLDELETE_ARRAY( buffer );

    bufferStart = 0;
    bufferPosition = 0;
  }

  void OutputStream::writeByte(const uint8_t b) {
    if (bufferPosition >= LUCENE_STREAM_BUFFER_SIZE)
      flush();
    buffer[bufferPosition++] = b;
  }

  void OutputStream::writeBytes(const uint8_t* b, const int32_t length) {
    if ( length < 0 )
      _CLTHROWA(CL_ERR_IllegalArgument, "IO Argument Error. Value must be a positive value.");
    for (int32_t i = 0; i < length; i++)
      writeByte(b[i]);
  }

  void OutputStream::writeInt(const int32_t i) {
    writeByte((uint8_t)(i >> 24));
    writeByte((uint8_t)(i >> 16));
    writeByte((uint8_t)(i >>  8));
    writeByte((uint8_t) i);
  }

  void OutputStream::writeVInt(const int32_t vi) {
	uint32_t i = vi;
    while ((i & ~0x7F) != 0) {
      writeByte((uint8_t)((i & 0x7f) | 0x80));
      i >>= 7; //doing unsigned shift
    }
    writeByte( (uint8_t)i );
  }

  void OutputStream::writeLong(const int64_t i) {
    writeInt((int32_t) (i >> 32));
    writeInt((int32_t) i);
  }

  void OutputStream::writeVLong(const int64_t vi) {
	uint64_t i = vi;
    while ((i & ~0x7F) != 0) {
      writeByte((uint8_t)((i & 0x7f) | 0x80));
      i >>= 7; //doing unsigned shift
    }
    writeByte((uint8_t)i);
  }

  void OutputStream::writeString(const TCHAR* s, const int32_t length ) {
    //int32_t length = 0;
    //if ( s != NULL ) 
	 //	length = _tcslen(s);
	 writeVInt(length);
    writeChars(s, 0, length);
  }

  void OutputStream::writeChars(const TCHAR* s, const int32_t start, const int32_t length){
    if ( length < 0 || start < 0 )
      _CLTHROWA(CL_ERR_IllegalArgument, "IO Argument Error. Value must be a positive value.");

    const int32_t end = start + length;
    for (int32_t i = start; i < end; i++) {
        const int32_t code = (int32_t)s[i];
        if (code >= 0x01 && code <= 0x7F)
			writeByte((uint8_t)code);
        else if (((code >= 0x80) && (code <= 0x7FF)) || code == 0) {
			writeByte((uint8_t)(0xC0 | (code >> 6)));
			writeByte((uint8_t)(0x80 | (code & 0x3F)));
        } else {
			writeByte((uint8_t)(0xE0 | (((uint32_t)code) >> 12))); //unsigned shift
			writeByte((uint8_t)(0x80 | ((code >> 6) & 0x3F)));
			writeByte((uint8_t)(0x80 | (code & 0x3F)));
        }
    }
  }


  int64_t OutputStream::getFilePointer() {
    return bufferStart + bufferPosition;
  }

  void OutputStream::seek(const int64_t pos) {
    flush();
    bufferStart = pos;
  }

  void OutputStream::flush() {
    flushBuffer(buffer, bufferPosition);
    bufferStart += bufferPosition;
    bufferPosition = 0;
  }

CL_NS_END
#endif
