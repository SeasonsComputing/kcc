#include "CLucene/StdHeader.h"
#include "FastCharStream.h"

#include "CLucene/util/Arrays.h"
#include "CLucene/util/Reader.h"

CL_NS_DEF(util)

  FastCharStream::FastCharStream(Reader* reader):
	input(reader),
	col(1),
	line(1)
  {
  }

  TCHAR FastCharStream::GetNext()	{
    if (Eos())
    {
      _CLTHROWA(CL_ERR_IO,"warning : FileReader.GetNext : Read TCHAR over EOS.");
    }
    TCHAR ch = input->readChar();
    col = static_cast<int32_t>(input->position())+1;

    if(ch == '\n')
    {
      line++;
      col = 1;
    }

	return ch;
  }

  void FastCharStream::UnGet(){
    if ( input->position() == 0 )
      _CLTHROWA(CL_ERR_IO,"error : FileReader.UnGet : ungetted first TCHAR");
	input->seek(input->position()-1);
  }

  TCHAR FastCharStream::Peek(){
    try{
      return input->peek();
    }catch(...){} //todo: only catch IO Err???
    return 0;
  }


  bool FastCharStream::Eos()	{
		return input->available()==0;
  }

  int32_t FastCharStream::Column() {
		return col;
  }

  int32_t FastCharStream::Line() {
		return line;
  }
CL_NS_END
