#include "CLucene/StdHeader.h"
#ifndef _lucene_store_TransactionalRAMDirectory_
#define _lucene_store_TransactionalRAMDirectory_

#include "RAMDirectory.h"
#include "CLucene/util/VoidList.h"
CL_NS_DEF(store)

  /***
  This transactional in-memory Directory was created to address a specific
  situation, and was deliberately pared down to the simplest viable
  implementation.  For the sake of simplicity, this implementation imposes
  restrictions on what operations can be performed in the directory while a
  transaction is in progress (documented in TransactionalRAMDirectory.cpp).

  Because the Lucene Directory interface itself is rather simplistic, it
  would not be difficult to expand TransactionalRAMDirectory so that it
  provided fully general transactionality.  However, the developer of this
  original implementation was of the opinion that the last thing CLucene
  needs is gratuitous features that exceed their required complexity and
  haven't been rigorously tested.
  */
  class TransactionalRAMDirectory: public RAMDirectory {
  private:
	typedef CL_NS(util)::CLSet<const char*, void*, CL_NS(util)::Compare::Char> FilenameSet;
    FilenameSet filesToRemoveOnAbort;

	typedef CL_NS(util)::CLSet<const char*, RAMFile*, CL_NS(util)::Compare::Char,CL_NS(util)::Deletor::acArray,CL_NS(util)::Deletor::Object<RAMFile> > FileMap;
    FileMap filesToRestoreOnAbort;

    bool transOpen;

    void transResolved();
    bool archiveOrigFileIfNecessary(const char* name);
    void unarchiveOrigFile(const char* name);

  public:
    TransactionalRAMDirectory();
    ~TransactionalRAMDirectory();

    bool transIsOpen();
    void transStart();
    void transCommit();
    void transAbort();

    // Constrained operations:
    void deleteFile(const char* name, const bool throwError = true);
    void renameFile(const char* from, const char* to);
    OutputStream* createFile(char* name);

    void close();
  };

CL_NS_END
#endif // ndef _lucene_store_TransactionalRAMDirectory_
