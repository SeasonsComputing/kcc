#include "CLucene/StdHeader.h"
#ifndef CLUCENE_LITE
#include "FieldsWriter.h"

#include "CLucene/util/VoidMap.h"
#include "CLucene/util/Reader.h"
#include "CLucene/util/Misc.h"
#include "CLucene/store/Directory.h"
#include "CLucene/store/OutputStream.h"
#include "CLucene/document/Document.h"
#include "CLucene/document/Field.h"
#include "FieldInfos.h"

CL_NS_USE(store)
CL_NS_USE(util)
CL_NS_USE(document)
CL_NS_DEF(index)
	
	FieldsWriter::FieldsWriter(Directory* d, const char* segment, FieldInfos* fn):
		fieldInfos(fn)
	{
	//Func - Constructor
	//Pre  - d contains a valid reference to a directory
	//       segment != NULL and contains the name of the segment
	//Post - fn contains a valid reference toa a FieldInfos

		CND_PRECONDITION(segment != NULL,"segment is NULL");

		const char* buf = Misc::segmentname(segment,".fdt");
        fieldsStream = d->createFile ( buf );
        _CLDELETE_CaARRAY( buf );
        
		buf = Misc::segmentname(segment,".fdx");
        indexStream = d->createFile( buf );
        _CLDELETE_CaARRAY( buf );
          
		CND_CONDITION(indexStream != NULL,"indexStream is NULL");
    }

	FieldsWriter::~FieldsWriter(){
	//Func - Destructor
	//Pre  - true
	//Post - Instance has been destroyed

		close();
	}

	void FieldsWriter::close() {
	//Func - Closes all streams and frees all resources
	//Pre  - true
	//Post - All streams have been closed all resources have been freed

		//Check if fieldsStream is valid
		if (fieldsStream){
			//Close fieldsStream
			fieldsStream->close();
			_CLDELETE( fieldsStream );
			}

		//Check if indexStream is valid
		if (indexStream){
			//Close indexStream
			indexStream->close();
			_CLDELETE( indexStream );
			}
	}

	void FieldsWriter::addDocument(Document* doc) {
	//Func - Adds a document
	//Pre  - doc contains a valid reference to a Document
	//       indexStream != NULL
	//       fieldsStream != NULL
	//Post - The document doc has been added

		CND_PRECONDITION(indexStream != NULL,"indexStream is NULL");
		CND_PRECONDITION(fieldsStream != NULL,"fieldsStream is NULL");

		indexStream->writeLong(fieldsStream->getFilePointer());

		int32_t storedCount = 0;
		DocumentFieldEnumeration* fields = doc->fields();
		while (fields->hasMoreElements()) {
			Field* field = fields->nextElement();
			if (field->isStored())
				storedCount++;
		}
		_CLDELETE(fields);
		fieldsStream->writeVInt(storedCount);

		fields = doc->fields();
		while (fields->hasMoreElements()) {
			Field* field = fields->nextElement();
			if (field->isStored()) {
				fieldsStream->writeVInt(fieldInfos->fieldNumber(field->name()));

				uint8_t bits = 0;
				if (field->isTokenized())
					bits |= 1;
				//FEATURE: can this data be compressed? A bit could be set to compress or not compress???
				fieldsStream->writeByte(bits);

				//FEATURE: this problem in Java Lucene too, if using Reader, data is not stored.
				if ( field->stringValue() == NULL ){
					Reader* r = field->readerValue();
					// Though Reader::position returns int64_t, we assume that
					// a single field won't exceed int32_t's capacity.
					int32_t rp = static_cast<int32_t>(r->position());
					r->seek(0);
					// Though Reader::available returns int64_t, we assume that
					// a single field won't exceed int32_t's capacity.
					int32_t rl = static_cast<int32_t>(r->available());

					TCHAR* rv = _CL_NEWARRAY(TCHAR,rl);
					r->read(rv,0,rl);
					r->seek(rp); //reset position

					//simulate writeString
					fieldsStream->writeVInt(rl);
					fieldsStream->writeChars(rv,0,rl);
					_CLDELETE_ARRAY(rv);
				}else
					fieldsStream->writeString(field->stringValue(),_tcslen(field->stringValue()));
			}
		}
		_CLDELETE(fields);
	}

CL_NS_END
#endif
