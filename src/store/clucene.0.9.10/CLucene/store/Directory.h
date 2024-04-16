#ifndef _lucene_store_Directory
#define _lucene_store_Directory

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#include "CLucene/store/Lock.h"
#include "CLucene/util/VoidList.h"
#include "CLucene/util/Misc.h"

#include "InputStream.h"
#include "OutputStream.h"

CL_NS_DEF(store)

   /** A Directory is a flat list of files.  Files may be written once, when they
   * are created.  Once a file is created it may only be opened for read, or
   * deleted.  Random access is permitted both when reading and writing.
   *
   * <p> Direct i/o is not used directly, but rather all i/o is
   * through this API.  This permits things such as: <ul>
   * <li> implementation of RAM-based indices;
   * <li> implementation indices stored in a database, via a database;
   * <li> implementation of an index as a single file;
   * </ul>
   *
   * @author Ben van Klinken
   */
	class Directory: LUCENE_REFBASE {
	private:
    DEFINE_MUTEX(REFINC_LOCK_MUTEX);
	protected:
		Directory(){
		}
	public:
		DEFINE_MUTEX (DIR_OBJ);
	   
		virtual ~Directory(){ };

		// Returns an null terminated array of strings, one for each file in the directory. 
		virtual char** list() const = 0;
		       
		// Returns true iff a file with the given name exists. 
		virtual bool fileExists(const char* name) const = 0;

		// Returns the time the named file was last modified. 
		virtual int64_t fileModified(const char* name) const = 0;

		// Returns the length of a file in the directory. 
		virtual int64_t fileLength(const char* name) const = 0;

		// Returns a stream reading an existing file. 
		virtual InputStream* openFile(const char* name) = 0;

#ifndef CLUCENE_LITE
		
		/// Set the modified time of an existing file to now. */
		virtual void touchFile(const char* name) = 0;

		// Removes an existing file in the directory. 
		virtual void deleteFile(const char* name, const bool throwError = true) = 0;

		// Renames an existing file in the directory.
		//	If a file already exists with the new name, then it is replaced.
		//	This replacement should be atomic. 
		virtual void renameFile(const char* from, const char* to) = 0;

		// Creates a new, empty file in the directory with the given name.
		//	Returns a stream writing this file. 
		virtual OutputStream* createFile(const char* name) = 0;

		// Construct a {@link Lock}.
		// @param name the name of the lock file
		virtual LuceneLock* makeLock(const char* name) = 0;
#endif //CLUCENE_LITE

		// Closes the store. 
		virtual void close() = 0;
		
	};
CL_NS_END
#endif


