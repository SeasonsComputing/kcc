#ifndef _lucene_index_FieldsWriter_
#define _lucene_index_FieldsWriter_
#ifndef CLUCENE_LITE

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#include "CLucene/util/VoidMap.h"
#include "CLucene/store/Directory.h"
#include "CLucene/store/OutputStream.h"
#include "CLucene/document/Document.h"
#include "CLucene/document/Field.h"
#include "FieldInfos.h"

CL_NS_DEF(index)
	class FieldsWriter :LUCENE_BASE{
	private:
		FieldInfos* fieldInfos;
		CL_NS(store)::OutputStream* fieldsStream;
		CL_NS(store)::OutputStream* indexStream;

	public:
		FieldsWriter(CL_NS(store)::Directory* d, const char* segment, FieldInfos* fn);
		~FieldsWriter();

		void close();

		void addDocument(CL_NS(document)::Document* doc);
	};
CL_NS_END
#endif
#endif
