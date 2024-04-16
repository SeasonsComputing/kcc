#include "stdafx.h"

#include "CLucene.h"
#include <iostream>

using namespace std;
using namespace lucene::index;
using namespace lucene::util;
using namespace lucene::store;
using namespace lucene::document;

void DeleteFiles(const char* dir) {
	IndexReader* reader = IndexReader::open(dir);

	int32_t count = 0;
	for (int32_t i = 0; i < reader->maxDoc(); i++){
		reader->deleteDoc (i);
		count ++;
	}
	printf("Deleted %d files\n", count);
	reader->close();
	_CLDELETE(reader);

	//OPTIMIZE
	if ( IndexReader::indexExists(dir) ){
		lucene::analysis::SimpleAnalyzer an;
		if ( IndexReader::isLocked(dir) ){
			printf("Index was locked... unlocking it.\n");
			IndexReader::unlock(dir);
		}

		IndexWriter* writer = _CLNEW IndexWriter( dir, &an, false);
		writer->optimize();
		_CLDELETE(writer);
	}
}
