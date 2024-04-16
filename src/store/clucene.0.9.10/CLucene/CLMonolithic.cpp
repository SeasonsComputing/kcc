/*
* this is a monolithic file that can be used to compile clucene using one source file.
* it simplifies some build processes by avoiding static & dynamic compalation pitfalls.
* 
* note: when creating a project add either this file, or all the other .cpp files, not both!
*/
#include "CLucene/StdHeader.cpp"
#include "CLucene/analysis/Analyzers.cpp"
#include "CLucene/analysis/AnalysisHeader.cpp"
#include "CLucene/analysis/standard/StandardAnalyzer.cpp"
#include "CLucene/analysis/standard/StandardFilter.cpp"
#include "CLucene/analysis/standard/StandardTokenizer.cpp"
#include "CLucene/config/gunichartables.cpp"
#include "CLucene/config/repl_wcs.cpp"
#include "CLucene/config/repl_wchar.cpp"
#include "CLucene/debug/condition.cpp"
#include "CLucene/debug/error.cpp"
#include "CLucene/debug/mempool.cpp"
#include "CLucene/debug/memtracking.cpp"
#include "CLucene/document/DateField.cpp"
#include "CLucene/document/Document.cpp"
#include "CLucene/document/Field.cpp"
#include "CLucene/index/CompoundFile.cpp"
#include "CLucene/index/DocumentWriter.cpp"
#include "CLucene/index/FieldInfos.cpp"
#include "CLucene/index/FieldsReader.cpp"
#include "CLucene/index/FieldsWriter.cpp"
#include "CLucene/index/IndexWriter.cpp"
#include "CLucene/index/IndexReader.cpp"
#include "CLucene/index/MultiReader.cpp"
#include "CLucene/index/SegmentInfos.cpp"
#include "CLucene/index/SegmentMergeInfo.cpp"
#include "CLucene/index/SegmentMergeQueue.cpp"
#include "CLucene/index/SegmentMerger.cpp"
#include "CLucene/index/SegmentReader.cpp"
#include "CLucene/index/SegmentTermDocs.cpp"
#include "CLucene/index/SegmentTermEnum.cpp"
#include "CLucene/index/SegmentTermPositions.cpp"
#include "CLucene/index/SegmentTermVector.cpp"
#include "CLucene/index/Term.cpp"
#include "CLucene/index/TermInfo.cpp"
#include "CLucene/index/TermInfosReader.cpp"
#include "CLucene/index/TermInfosWriter.cpp"
#include "CLucene/index/TermVectorReader.cpp"
#include "CLucene/index/TermVectorWriter.cpp"
#include "CLucene/queryParser/Lexer.cpp"
#include "CLucene/queryParser/MultiFieldQueryParser.cpp"
#include "CLucene/queryParser/QueryParser.cpp"
#include "CLucene/queryParser/QueryParserBase.cpp"
#include "CLucene/queryParser/QueryToken.cpp"
#include "CLucene/queryParser/TokenList.cpp"
#include "CLucene/search/BooleanQuery.cpp"
#include "CLucene/search/BooleanScorer.cpp"
#include "CLucene/search/DateFilter.cpp"
#include "CLucene/search/ConjunctionScorer.cpp"
#include "CLucene/search/ExactPhraseScorer.cpp"
#include "CLucene/search/Explanation.cpp"
#include "CLucene/search/FieldCache.cpp"
#include "CLucene/search/FieldCacheImpl.cpp"
#include "CLucene/search/FieldDocSortedHitQueue.cpp"
#include "CLucene/search/FieldSortedHitQueue.cpp"
#include "CLucene/search/FilteredTermEnum.cpp"
#include "CLucene/search/FuzzyQuery.cpp"
#include "CLucene/search/Hits.cpp"
#include "CLucene/search/HitQueue.cpp"
#include "CLucene/search/IndexSearcher.cpp"
#include "CLucene/search/MultiSearcher.cpp"
#include "CLucene/search/MultiTermQuery.cpp"
#include "CLucene/search/PhrasePositions.cpp"
#include "CLucene/search/PhraseQuery.cpp"
#include "CLucene/search/PhraseScorer.cpp"
#include "CLucene/search/PrefixQuery.cpp"
#include "CLucene/search/RangeQuery.cpp"
#include "CLucene/search/SearchHeader.cpp"
#include "CLucene/search/Similarity.cpp"
#include "CLucene/search/SloppyPhraseScorer.cpp"
#include "CLucene/search/Sort.cpp"
#include "CLucene/search/TermQuery.cpp"
#include "CLucene/search/TermScorer.cpp"
#include "CLucene/search/WildcardQuery.cpp"
#include "CLucene/search/WildcardTermEnum.cpp"
#include "CLucene/store/FSDirectory.cpp"
#include "CLucene/store/InputStream.cpp"
#include "CLucene/store/Lock.cpp"
#include "CLucene/store/OutputStream.cpp"
#include "CLucene/store/RAMDirectory.cpp"
#include "CLucene/store/TransactionalRAMDirectory.cpp"
#include "CLucene/util/Arrays.cpp"
#include "CLucene/util/BitVector.cpp"
#include "CLucene/util/FastCharStream.cpp"
#include "CLucene/util/MD5Digester.cpp"
#include "CLucene/util/Misc.cpp"
#include "CLucene/util/Reader.cpp"
#include "CLucene/util/StringBuffer.cpp"
#include "CLucene/util/StringIntern.cpp"
#include "CLucene/util/dirent.cpp"
