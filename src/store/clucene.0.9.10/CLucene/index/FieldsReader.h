#ifndef _lucene_index_FieldsReader_
#define _lucene_index_FieldsReader_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#include "CLucene/util/VoidMap.h"
#include "CLucene/store/Directory.h"
#include "CLucene/document/Document.h"
#include "CLucene/document/Field.h"
#include "FieldInfos.h"

CL_NS_DEF(index)

   /**
   * Class responsible for access to stored document fields.
   *
   * It uses &lt;segment&gt;.fdt and &lt;segment&gt;.fdx; files.
   *
   * @version $Id: FieldsReader.h 15199 2007-03-09 17:57:17Z tvk $
   */
	class FieldsReader :LUCENE_BASE{
	private:
		const FieldInfos* fieldInfos;
		CL_NS(store)::InputStream* fieldsStream;
		CL_NS(store)::InputStream* indexStream;
		int32_t _size;
	public:
		FieldsReader(CL_NS(store)::Directory* d, const char* segment, FieldInfos* fn);
		~FieldsReader();

		void close();

		int32_t size();

		CL_NS(document)::Document* doc(int32_t n);
	};
CL_NS_END
#endif
