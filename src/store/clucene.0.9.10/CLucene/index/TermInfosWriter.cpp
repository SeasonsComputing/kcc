#include "CLucene/StdHeader.h"
#ifndef CLUCENE_LITE
#include "TermInfosWriter.h"

#include "CLucene/store/Directory.h"
#include "CLucene/util/Misc.h"
#include "FieldInfos.h"
#include "Term.h"
#include "TermInfo.h"

CL_NS_USE(util)
CL_NS_USE(store)
CL_NS_DEF(index)

	TermInfosWriter::TermInfosWriter(Directory* directory, const char* segment, FieldInfos* fis):
        fieldInfos(fis){
    //Func - Constructor
	//Pre  - directory contains a valid reference to a Directory
    //       segment != NULL
	//       fis contains a valid reference to a reference FieldInfos
	//Post - The instance has been created

        CND_PRECONDITION(segment != NULL, "segment is NULL");
		//Initialize instance
        initialise(directory,segment,false);

		other = _CLNEW TermInfosWriter(directory, segment,fieldInfos, true);

		CND_CONDITION(other != NULL, "other is NULL");

		other->other = this;
	}

    TermInfosWriter::TermInfosWriter(Directory* directory, const char* segment, FieldInfos* fis, bool isIndex):
	    fieldInfos(fis){
    //Func - Constructor
	//Pre  - directory contains a valid reference to a Directory
    //       segment != NULL
	//       fis contains a valid reference to a reference FieldInfos
	//       isIndex is true or false
	//Post - The instance has been created

        CND_PRECONDITION(segment != NULL, "segment is NULL");
        initialise(directory,segment,isIndex);
    }

    void TermInfosWriter::initialise(Directory* directory, const char* segment, bool IsIndex){
    //Func - Helps constructors to initialize Instance
	//Pre  - directory contains a valid reference to a Directory
    //       segment != NULL
	//       fis contains a valid reference to a reference FieldInfos
	//Post - The instance has been initialized

		lastTerm = _CLNEW Term( LUCENE_BLANK_STRING, LUCENE_BLANK_STRING);

		CND_CONDITION(lastTerm != NULL, "Could not allocate memory for lastTerm");
    
		lastTi  = _CLNEW TermInfo();

        CND_CONDITION(lastTi != NULL, "Could not allocate memory for lastTi");

		lastIndexPointer = 0;
		size             = 0;
		isIndex          = IsIndex;
		indexInterval = LUCENE_DEFAULT_WRITER_INDEX_INTERVAL;
		skipInterval = LUCENE_DEFAULT_TERMDOCS_SKIP_INTERVAL;

	    const char* buf = Misc::segmentname(segment, (isIndex ? ".tii" : ".tis"));
	    output = directory->createFile( buf );
	    _CLDELETE_CaARRAY(buf);

      output->writeInt(FORMAT);                      // write format
      output->writeLong(0);                          // leave space for size
      output->writeInt(indexInterval);// write indexInterval
      output->writeInt(skipInterval); // write skipInterval

        //Set other to NULL by Default
        other = NULL;
    }

	TermInfosWriter::~TermInfosWriter(){
    //Func - Destructor
	//Pre  - true
	//Post - de instance has been destroyed

		close();
	}
	
	void TermInfosWriter::add(Term* term, const TermInfo* ti) {
	//Func - Writes a Term and TermInfo to the outputstream
	//Pre  - Term must be lexicographically greater than all previous Terms added.
    //       Pointers of TermInfo ti (freqPointer and proxPointer) must be positive and greater than all previous.

		CND_PRECONDITION(isIndex || (!isIndex  && term->compareTo(lastTerm) > 0),"term out of order");
		CND_PRECONDITION(ti->freqPointer >= lastTi->freqPointer,"freqPointer out of order");
		CND_PRECONDITION(ti->proxPointer >= lastTi->proxPointer,"proxPointer out of order");

		if (!isIndex && size % indexInterval == 0){
            //add an index term
			other->add(lastTerm, lastTi);
		}

		//write term
		writeTerm(term);				      
		// write doc freq
		output->writeVInt(ti->docFreq);		  
		//write pointers
		output->writeVLong(ti->freqPointer - lastTi->freqPointer);
		output->writeVLong(ti->proxPointer - lastTi->proxPointer);
		if (ti->docFreq >= skipInterval) {
			output->writeVInt(ti->skipOffset);
		}

		if (isIndex){
			output->writeVLong(other->output->getFilePointer() - lastIndexPointer);
			lastIndexPointer = other->output->getFilePointer(); // write pointer
		}

		lastTi->set(ti);
		size++;
	}

	void TermInfosWriter::close() {
    //Func - Closes the TermInfosWriter
	//Pre  - true
	//Post - The TermInfosWriter has been closed
    
		if (output){
			//write size at start
		    output->seek(4);          // write size after format
		    output->writeLong(size);
		    output->close();
		   _CLDELETE(output);

		   if (!isIndex){
			   if(other){
			      other->close();
			      _CLDELETE( other );
			      }
		      }
              _CLDECDELETE(lastTerm);

		      _CLDELETE(lastTi);
		   }
	}

	void TermInfosWriter::writeTerm(Term* term) {
    int32_t start = Misc::stringDifference(lastTerm->text(),lastTerm->textLength(), 
		term->text(),term->textLength());
		int32_t length = term->textLength() - start;
	 
		output->writeVInt(start);			  // write shared prefix length
		output->writeVInt(length);			  // write delta length
		output->writeChars(term->text(), start, length);  // write delta chars

		int32_t fieldnum = fieldInfos->fieldNumber(term->field());
		CND_PRECONDITION(fieldnum>=-1&&fieldnum<fieldInfos->size(),"Fieldnum is out of range");
		output->writeVInt(fieldnum); // write field num

		if ( lastTerm->__cl_refcount == 1 ){
			lastTerm->set(term->field(),term->text(),false);
		}else{
			_CLDECDELETE(lastTerm); //todo: reuse term?
			lastTerm = _CL_POINTER(term);
		}
	}


CL_NS_END
#endif
