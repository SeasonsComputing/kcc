#ifndef _lucene_store_FSDirectory_
#define _lucene_store_FSDirectory_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#include "Directory.h"
#include "Lock.h"
#include "CLucene/util/VoidMap.h"
#include "CLucene/util/StringBuffer.h"

#if defined(LUCENE_FS_MMAP) && !defined(_CLCOMPILER_MSVC)
 //todo: do some unix support stuff
 #undef LUCENE_FS_MMAP
#endif

   CL_NS_DEF(store)

   /**
   * Straightforward implementation of {@link Directory} as a directory of files.
   * <p>If the system property 'disableLuceneLocks' has the String value of
   * "true", lock creation will be disabled.
   *
   * @see Directory
   * @author Doug Cutting
   */
	class FSDirectory:public Directory{
#ifndef LUCENE_HIDE_INTERNAL
	public:
		#ifndef CLUCENE_LITE
		class FSLock: public LuceneLock{
		public:
			// const char* fname;
			char lockFile[CL_MAX_PATH];
			char* lockDir;
			FSLock ( const char* lockDir, const char* name );
			~FSLock();
			bool obtain();
			void release();
			bool isLocked();
			TCHAR* toString();
		};
		#endif // CLUCENE_LITE

#if defined(LUCENE_FS_MMAP)
		class MMapInputStream: public IndexInput{
			uint8_t* data;
			uint8_t* pointer;
			HANDLE handle;
			HANDLE fhandle;
    		bool isClone;

			MMapInputStream(const MMapInputStream& clone);
		public:
    		MMapInputStream(const char* path);
			~MMapInputStream();
			InputStream* clone();

			uint8_t readByte();
			void readBytes(uint8_t* b, const int32_t offset, const int32_t len);
			virtual void close();
			int64_t getFilePointer();
			void seek(const int64_t pos);
		};
#endif

    	class FSIndexInput:public BufferedIndexInput {
    		int32_t fhandle;
    	protected:
    		FSIndexInput(FSIndexInput& clone);
    		DEFINE_MUTEX(file_mutex);
    		bool isClone;
    		char path[CL_MAX_DIR]; //todo: this is only used for cloning, better to get information from the fhandle
    	public:
    		FSIndexInput(const char* path);
    		~FSIndexInput();
    
    		InputStream* clone();
    		void close();
    	protected:
    		// Random-access methods 
    		void seekInternal(const int64_t position);
    		// InputStream methods 
    		void readInternal(uint8_t* b, const int32_t offset, const int32_t len);
    	};

    #ifndef CLUCENE_LITE
    	class FSIndexOutput: public OutputStream {
    	private:
    		int32_t fhandle;
    	public:
    		FSIndexOutput(const char* path);
    		~FSIndexOutput();

    		// output methods: 
    		void flushBuffer(const uint8_t* b, const int32_t size);
    		void close();
    
    		// Random-access methods 
    		void seek(const int64_t pos);
    		int64_t Length();
      };
    #endif // CLUCENE_LITE
#endif //LUCENE_HIDE_INTERNAL


	protected:
		FSDirectory(const char* path, const bool createDir);
	private:
		char directory[CL_MAX_DIR];
		int refCount;
		void create();
		
		static char* LOCK_DIR;
		static char* getLockDir();
		char lockDir[CL_MAX_PATH];
		char* getLockPrefix();

		void priv_getFN(char* buffer, const char* name) const;
		int32_t priv_getStat(const char* name, struct fileStat* ret) const;
		
		DEFINE_MUTEX (createLock);
		DEFINE_MUTEX(closeLock);
		DEFINE_MUTEX(renameLock);

		bool useMMap;
	public:
	  ///Destructor - only call this if you are sure the directory
	  ///is not being used anymore. Otherwise use the ref-counting
	  ///facilities of _CLDECDELETE
		~FSDirectory();

		/// Returns a null terminated array of strings, one for each file in the directory. 
		char** list() const;

		/// Returns true iff a file with the given name exists. 
		bool fileExists(const char* name) const;

      /// Returns the text name of the directory
		const char* getDirName() const; ///<returns reference
		

		/**
         Returns the directory instance for the named location.
		   
         Do not delete this instance, only use close, otherwise other instances
         will lose this instance.

		   <p>Directories are cached, so that, for a given canonical path, the same
		   FSDirectory instance will always be returned.  This permits
		   synchronization on directories.
		   
		   @param file the path to the directory.
		   @param create if true, create, or erase any existing contents.
		   @return the FSDirectory for the named file.
        */
		static FSDirectory* getDirectory(const char* file, const bool create);

		/// Returns the time the named file was last modified.
		int64_t fileModified(const char* name) const;
		       
		//static
		/// Returns the time the named file was last modified.
		static int64_t fileModified(const char* dir, const char* name);

		//static
		/// Returns the length in bytes of a file in the directory. 
		int64_t fileLength(const char* name) const;

		/// Returns a stream reading an existing file. 
		InputStream* openFile(const char* name);

#ifndef CLUCENE_LITE
		/// Removes an existing file in the directory. 
		void deleteFile(const char* name, const bool throwError=true);

		/// Renames an existing file in the directory. 
		void renameFile(const char* from, const char* to);

      /** Set the modified time of an existing file to now. */
      void touchFile(const char* name);

		/// Creates a new, empty file in the directory with the given name.
		///	Returns a stream writing this file. 
		OutputStream* createFile(const char* name);

		/// Construct a {@link Lock}.
		/// @param name the name of the lock file
		LuceneLock* makeLock(const char* name);
#endif // CLUCENE_LITE

		  ///Decrease the ref-count to the directory by one. If
		  ///the object is no longer needed, then the object is
		  ///removed from the directory pool. 
      void close();

	  /** 
	  * If MMap is available, this can disable use of
	  * mmap reading.
	  */
	  void setUseMMap(bool value){ useMMap = value; }
	  /** 
	  * Gets whether the directory is using MMap for inputstreams.
	  */
	  bool getUseMMap(){ return useMMap; }
  };

CL_NS_END
#endif
