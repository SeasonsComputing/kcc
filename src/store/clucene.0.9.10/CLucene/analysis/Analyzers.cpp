#include "CLucene/StdHeader.h"
#include "Analyzers.h"

CL_NS_USE(util)
CL_NS_DEF(analysis)
		
CharTokenizer::CharTokenizer(Reader* in) :
			//input(in), ; input is in tokenizer base class (bug fix thanks to Andy Osipienko)
			offset(0),
			bufferIndex(0),
			dataLen(0)

{
	input = in;
}

TCHAR CharTokenizer::normalize(const TCHAR c) const 
{ 
	return c; 
}
bool CharTokenizer::next(Token* token){
	int32_t length = 0;
	int32_t start = offset;
	while (true) {
		TCHAR c;
		offset++;
		if (bufferIndex >= dataLen) {
			dataLen = input->read(ioBuffer,0,LUCENE_IO_BUFFER_SIZE);
			bufferIndex = 0;
		}
		if (dataLen <= 0 ) {
			if (length > 0)
				break;
			else
				return false;
		}else
			c = ioBuffer[bufferIndex++];
		if (isTokenChar(c)) {                       // if it's a token TCHAR

			if (length == 0)			  // start of token
				start = offset-1;

			buffer[length++] = normalize(c);          // buffer it, normalized

			if (length == LUCENE_MAX_WORD_LEN)		  // buffer overflow!
				break;

		} else if (length > 0)			  // at non-Letter w/ chars
			break;					  // return 'em

	}
	buffer[length]=0;
	token->set( buffer, start, start+length);
	return true;
}

bool LetterTokenizer::isTokenChar(const TCHAR c) const {
	return _istalpha(c)!=0;
}


TCHAR LowerCaseTokenizer::normalize(const TCHAR chr) const {
	return _totlower(chr);
}

bool WhitespaceTokenizer::isTokenChar(const TCHAR c)  const{
	return _istspace(c)==0; //(return true if NOT a space)
}

TokenStream* WhitespaceAnalyzer::tokenStream(const TCHAR* fieldName, Reader* reader) {
	return _CLNEW WhitespaceTokenizer(reader);
}

TokenStream* SimpleAnalyzer::tokenStream(const TCHAR* fieldName, Reader* reader) {
	return _CLNEW LowerCaseTokenizer(reader);
}

bool PorterStemmerFilter::next(Token* t)
{
	if ( input->next(t) )
	{		
		lucene::analysis::PorterStemmer s(t->_termText);
		if(s.stem())
		{
			_tcsncpy(t->_termText,t->termText(),s.getResultLength());
			t->_termText[s.getResultLength()] = '\0';
		}
		return true;
	}else
		return false;
}

bool LowerCaseFilter::next(Token* t){
	if (!input->next(t))
		return false;
 	stringCaseFold( t->_termText );
	return true;
}

StopFilter::StopFilter(TokenStream* in, bool deleteTokenStream, TCHAR** stopWords):
	TokenFilter(in, deleteTokenStream),
	table(_CLNEW CLSetList<TCHAR*>(false))
{
	fillStopTable( table,stopWords );
}

void StopFilter::fillStopTable(CLSetList<TCHAR*>* stopTable,
								  TCHAR** stopWords) {
	for (int32_t i = 0; stopWords[i]!=NULL; i++)
		stopTable->insert(stopWords[i]);
}

bool StopFilter::next(Token* token) {
	// return the first non-stop word found
	for (; input->next(token); ){
		if (table->find(token->_termText)==table->end()){
			return true;
		}//else
		//	_CLDELETE( token );
	}

	// reached EOS -- return nothing
	return false;
}

StopAnalyzer::StopAnalyzer():stopTable(false)
{
	StopFilter::fillStopTable(&stopTable,ENGLISH_STOP_WORDS);
}
StopAnalyzer::~StopAnalyzer()
{
}
StopAnalyzer::StopAnalyzer( TCHAR** stopWords) {
	StopFilter::fillStopTable(&stopTable,stopWords);
}
TokenStream* StopAnalyzer::tokenStream(const TCHAR* fieldName, Reader* reader) {
	return _CLNEW StopFilter(_CLNEW LowerCaseTokenizer(reader),true, &stopTable);
}

TCHAR* StopAnalyzer::ENGLISH_STOP_WORDS[]  = 
{
	_T("a"), _T("an"), _T("and"), _T("are"), _T("as"), _T("at"), _T("be"), _T("but"), _T("by"),
	_T("for"), _T("if"), _T("in"), _T("into"), _T("is"), _T("it"),
	_T("no"), _T("not"), _T("of"), _T("on"), _T("or"), _T("s"), _T("such"),
	_T("t"), _T("that"), _T("the"), _T("their"), _T("then"), _T("there"), _T("these"),
	_T("they"), _T("this"), _T("to"), _T("was"), _T("will"), _T("with"), NULL
};

PerFieldAnalyzerWrapper::PerFieldAnalyzerWrapper(Analyzer* defaultAnalyzer):
    analyzerMap(true,true)
{
    this->defaultAnalyzer = defaultAnalyzer;
}
PerFieldAnalyzerWrapper::~PerFieldAnalyzerWrapper(){
    analyzerMap.clear();
    _CLDELETE(defaultAnalyzer);
}

void PerFieldAnalyzerWrapper::addAnalyzer(const TCHAR* fieldName, Analyzer* analyzer) {
    analyzerMap.put(STRDUP_TtoT(fieldName), analyzer);
}

TokenStream* PerFieldAnalyzerWrapper::tokenStream(const TCHAR* fieldName, Reader* reader) {
    Analyzer* analyzer = analyzerMap.get(fieldName);
    if (analyzer == NULL) {
      analyzer = defaultAnalyzer;
    }
    
    return analyzer->tokenStream(fieldName, reader);
}

CL_NS_END
