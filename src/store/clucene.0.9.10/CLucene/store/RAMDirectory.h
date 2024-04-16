#ifndef _lucene_store_RAMDirectory_
#define _lucene_store_RAMDirectory_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#include "Lock.h"
#include "Directory.h"
#include "CLucene/util/VoidMap.h"
#include "CLucene/util/Arrays.h"

CL_NS_DEF(store)

	class RAMFile:LUCENE_BASE {
	public:
		CL_NS(util)::CLVector<uint8_t*,CL_NS(util)::Deletor::Array<uint8_t> > buffers;
		int64_t length;
		int64_t lastModified;

        #ifdef _DEBUG
		const char* filename;
        #endif

		RAMFile();
		~RAMFile();
	};

    #ifndef CLUCENE_LITE
	class RAMOutputStream: public OutputStream {
	protected:
		RAMFile* file;
		int32_t pointer;
		CL_NS(util)::Arrays::Byte _arrays;
		bool deleteFile;
	public:

		RAMOutputStream(RAMFile* f);
      RAMOutputStream();
  	    /** Construct an empty output buffer. */
		virtual ~RAMOutputStream();

		// output methods: 
		void flushBuffer(const uint8_t* src, const int32_t len);

		virtual void close();

		// Random-at methods
		virtual void seek(const int64_t pos);
		int64_t Length();
        /** Resets this to an empty buffer. */
        void reset();
  	    /** Copy the current contents of this buffer to the named output. */
        void writeTo(OutputStream* output);
	};
    #endif

	class RAMInputStream:public BufferedIndexInput {
	private:
		RAMFile* file;
		int32_t pointer;
		RAMInputStream(RAMInputStream& clone);

	public:
		RAMInputStream(RAMFile* f);
		~RAMInputStream();
		InputStream* clone();

     /** InputStream methods */
	  /** DSR:CL_BUG:
	  ** The CLucene 0.8.11 implementation of readInternal caused invalid memory
	  ** ops if fewer than the requested number of bytes were available and the
	  ** difference between requested and available happened to cross a buffer
	  ** boundary.
	  ** A bufferNumber would then be generated for a buffer that did not exist,
	  ** so an invalid buffer pointer was fetched from beyond the end of the
	  ** file->buffers vector, and an attempt was made to read bytes from the
	  ** bad buffer pointer.  Often this resulted in reading uninitialized memory;
	  ** it only segfaulted occasionally.
	  ** I replaced the dysfunctional CLucene 0.8.11 impl with a fixed version. */
		void readInternal(uint8_t *dest, const int32_t idestOffset, const int32_t len);

		void close();

        /** Random-at methods */
		void seekInternal(const int64_t pos);
	};


   /**
   * A memory-resident {@link Directory} implementation.
   *
   * @version $Id: RAMDirectory.h 15199 2007-03-09 17:57:17Z tvk $
   */
	class RAMDirectory:public Directory{

		#ifndef CLUCENE_LITE
		class RAMLock: public LuceneLock{
		private:
			RAMDirectory* directory;
			const char* fname;
		public:
			RAMLock(const char* name, RAMDirectory* dir);
			virtual ~RAMLock();
			bool obtain();
			void release();
			bool isLocked();
			virtual TCHAR* toString();
		};
		#endif
		

		typedef CL_NS(util)::CLHashMap<const char*,RAMFile*, 
				CL_NS(util)::Compare::Char,CL_NS(util)::Deletor::acArray ,
				CL_NS(util)::Deletor::Object<RAMFile> > FileMap;
	protected:
		
  /**
   * Creates a new <code>RAMDirectory</code> instance from a different
   * <code>Directory</code> implementation.  This can be used to load
   * a disk-based index into memory.
   * <P>
   * This should be used only with indices that can fit into memory.
   *
   * @param dir a <code>Directory</code> value
   * @exception IOException if an error occurs
   */
	  void _copyFromDir(Directory* dir, bool closeDir);
	  FileMap files;
	public:
	  DEFINE_MUTEX(files_mutex);

		/// Returns a null terminated array of strings, one for each file in the directory. 
		char** list() const;

      /** Constructs an empty {@link Directory}. */
		RAMDirectory();
		
	  ///Destructor - only call this if you are sure the directory
	  ///is not being used anymore. Otherwise use the ref-counting
	  ///facilities of dir->close
		virtual ~RAMDirectory();
		RAMDirectory(Directory* dir);
		
	  /**
	   * Creates a new <code>RAMDirectory</code> instance from the {@link FSDirectory}.
	   *
	   * @param dir a <code>String</code> specifying the full index directory path
	   */
		RAMDirectory(const char* dir);
		       
		/// Returns true iff the named file exists in this directory. 
		bool fileExists(const char* name) const;

		/// Returns the time the named file was last modified. 
		int64_t fileModified(const char* name) const;

		/// Returns the length in bytes of a file in the directory. 
		int64_t fileLength(const char* name) const;

#ifndef CLUCENE_LITE
		/// Removes an existing file in the directory. 
		virtual void renameFile(const char* from, const char* to);

		/// Removes an existing file in the directory. 
		virtual void deleteFile(const char* name, const bool throwError = true);

		/** Set the modified time of an existing file to now. */
		void touchFile(const char* name);

		/// Creates a new, empty file in the directory with the given name.
		///	Returns a stream writing this file. 
		virtual OutputStream* createFile(const char* name);

		/// Construct a {@link Lock}.
		/// @param name the name of the lock file
		LuceneLock* makeLock(const char* name);
#endif

		/// Returns a stream reading an existing file. 
		InputStream* openFile(const char* name);

		virtual void close();

	};
CL_NS_END
#endif
