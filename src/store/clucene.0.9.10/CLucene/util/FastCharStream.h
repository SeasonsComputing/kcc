#ifndef _lucene_util_FastCharStream_
#define _lucene_util_FastCharStream_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#include "CLucene/util/Arrays.h"
#include "CLucene/util/Reader.h"

CL_NS_DEF(util)

	/// Ported implementation of the FastCharStream class LUCENE_EXPORT.
	class FastCharStream:LUCENE_BASE
	{
		int32_t col;
		int32_t line;
	
	public:
		Reader* input;

		/// Initializes a new instance of the FastCharStream class LUCENE_EXPORT.
		FastCharStream(Reader* reader);
		
		/// Returns the next TCHAR from the stream.
		TCHAR GetNext();

		void UnGet();
		
		/// Returns the current top TCHAR from the input stream without removing it.
		TCHAR Peek();
		
		
		/// Returns <b>True</b> if the end of stream was reached.
		bool Eos()	;

		/// Gets the current column.
		int32_t Column();

		/// Gets the current line.
		int32_t Line();
	};
CL_NS_END
#endif
