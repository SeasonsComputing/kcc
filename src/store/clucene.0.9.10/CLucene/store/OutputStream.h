#ifndef _lucene_store_OutputStream_
#define _lucene_store_OutputStream_
#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#ifndef CLUCENE_LITE

CL_NS_DEF(store)
  /** Abstract class for output to a file in a Directory.  A random-access output
   * stream.  Used for all Lucene index output operations.
   * @see Directory
   * @see InputStream
   */
	class OutputStream:LUCENE_BASE{
  private:
    uint8_t* buffer;
    int64_t bufferStart;			  // position in file of buffer
    int32_t bufferPosition;		  // position in buffer

  public:
    OutputStream();
    //OutputStream(OutputStream& clone);
    virtual ~OutputStream();

    //virtual OutputStream* clone()=0;

	  /** Writes a single byte.
	   * @see InputStream#readByte()
	   */
    void writeByte(const uint8_t b);

	  /** Writes an array of bytes.
	   * @param b the bytes to write
	   * @param length the number of bytes to write
	   * @see InputStream#readBytes(byte[],int32_t,int32_t)
	   */
    void writeBytes(const uint8_t* b, const int32_t length);

	  /** Writes an int as four bytes.
	   * @see InputStream#readInt()
	   */
    void writeInt(const int32_t i);
	
	   /** Writes an int in a variable-length format.  Writes between one and
	   * five bytes.  Smaller values take fewer bytes.  Negative numbers are not
	   * supported.
	   * @see InputStream#readVInt()
	   */
    void writeVInt(const int32_t vi);

   /** Writes a long as eight bytes.
   * @see InputStream#readLong()
   */
    void writeLong(const int64_t i);

	  /** Writes an long in a variable-length format.  Writes between one and five
	   * bytes.  Smaller values take fewer bytes.  Negative numbers are not
	   * supported.
	   * @see InputStream#readVLong()
	   */
    void writeVLong(const int64_t vi);

	  /** Writes a string.
	   * @see InputStream#readString()
	   */
    void writeString(const TCHAR* s, const int32_t length);

   /** Writes a sequence of UTF-8 encoded characters from a string.
   * @param s the source of the characters
   * @param start the first character in the sequence
   * @param length the number of characters in the sequence
   * @see InputStream#readChars(char[],int32_t,int32_t)
   */
    void writeChars(const TCHAR* s, const int32_t start, const int32_t length);

    bool isClosed() const; /* DSR:PROPOSED */

     /** Closes this stream to further operations. */
    virtual void close();

    /** Returns the current position in this file, where the next write will
   * occur.
   * @see #seek(long)
   */
    int64_t getFilePointer();

   /** Sets current position in this file, where the next write will occur.
   * @see #getFilePointer()
   */
    virtual void seek(const int64_t pos);

    /** The number of bytes in the file. */
    virtual int64_t Length() = 0;

  protected:
  /** Forces any buffered output to be written. */
    void flush();

    /** Expert: implements buffer write.  Writes bytes at the current position in
   * the output.
   * @param b the bytes to write
   * @param len the number of bytes to write
   */
    virtual void flushBuffer(const uint8_t* b, const int32_t len) = 0;
  };
CL_NS_END
#endif
#endif
