#include "CLucene/StdHeader.h"
#include "FieldInfos.h"

#include "CLucene/store/Directory.h"
#include "CLucene/document/Document.h"
#include "CLucene/document/Field.h"
#include "CLucene/util/VoidMap.h"
#include "CLucene/util/Misc.h"
#include "FieldInfo.h"

CL_NS_USE(store)
CL_NS_USE(document)
CL_NS_DEF(index)


	FieldInfos::FieldInfos():byName(false,false),byNumber(true) {
	}

	FieldInfos::~FieldInfos(){
		byName.clear();
		byNumber.clear();
	}

	FieldInfos::FieldInfos(Directory* d, const char* name):
		byName(false,false),byNumber(true) 
	{
		InputStream* input = d->openFile(name);
		try {
			read(input);
		} _CLFINALLY (
		    input->close();
		    _CLDELETE(input);
		);
	}

#ifndef CLUCENE_LITE
	void FieldInfos::add(Document* doc) {
		DocumentFieldEnumeration* fields  = doc->fields();
		while (fields->hasMoreElements()) {
			Field* field = fields->nextElement();
			add(field->name(), field->isIndexed(), field->isTermVectorStored());
		}
		_CLDELETE(fields);
	}

	void FieldInfos::add( const TCHAR* name, const bool isIndexed, const bool storeTermVector) {
		FieldInfo* fi = fieldInfo(name);
      if (fi == NULL) {
         addInternal(name, isIndexed, storeTermVector); //todo: check this interning
      } else {
         if (fi->isIndexed != isIndexed) {
            fi->isIndexed = true;                      // once indexed, always index
         }
         if (fi->storeTermVector != storeTermVector) {
            fi->storeTermVector = true;                // once vector, always vector
         }
      }
	}

   void FieldInfos::add(const TCHAR** names,const bool isIndexed, const bool storeTermVectors) {
	  int32_t i=0;      
	  while ( names[i] != NULL ){
         add(names[i], isIndexed, storeTermVectors);
		 i++;
      }
   }

#endif // CLUCENE_LITE

	int32_t FieldInfos::fieldNumber(const TCHAR* fieldName)const {
		FieldInfo* fi = fieldInfo(fieldName);
		if ( fi != NULL )
			return fi->number;
		else
			return -1;
	}


	FieldInfo* FieldInfos::fieldInfo(const TCHAR* fieldName) const {
		FieldInfo* ret = byName.get(fieldName);
		return ret;
	}
	const TCHAR* FieldInfos::fieldName(const int32_t fieldNumber)const {
		//jlucene no longer throws an error if the field name is invalid.
        //CND_PRECONDITION(fieldNumber>=0 && fieldNumber<byNumber.size(),"fieldNumber out of range")
		FieldInfo* fi = fieldInfo(fieldNumber);
		//if ( fi == NULL )
		//	_CLTHROWA(CL_ERR_NullPointer,"Field name does not exist");
        if ( fi == NULL )
            return LUCENE_BLANK_STRING;
		return fi->name;
	}

	FieldInfo* FieldInfos::fieldInfo(const int32_t fieldNumber) const {
		//jlucene no longer throws an error if the field name is invalid.
        //CND_PRECONDITION(fieldNumber>=0 && fieldNumber<byNumber.size(),"fieldNumber out of range")
		if ( fieldNumber < 0 || fieldNumber >= byNumber.size() )
            return NULL;
        return byNumber[fieldNumber];
	}

	int32_t FieldInfos::size()const {
		return byNumber.size();
	}

#ifndef CLUCENE_LITE
	void FieldInfos::write(Directory* d, const char* name) {
		OutputStream* output = d->createFile(name);
		try {
			write(output);
		} _CLFINALLY (
		    output->close();
		    _CLDELETE(output);
		);
	}

	void FieldInfos::write(OutputStream* output) {
		output->writeVInt(size());
		for (int32_t i = 0; i < size(); i++) {
			FieldInfo* fi = fieldInfo(i);
			uint8_t bits = 0x0;
			if (fi->isIndexed) bits |= 0x1;
			if (fi->storeTermVector) bits |= 0x2;

			output->writeString(fi->name,_tcslen(fi->name));
			output->writeByte(bits);
		}
	}
#endif //CLUCENE_LITE

	void FieldInfos::read(InputStream* input) {
		int32_t size = input->readVInt();
        TCHAR name[LUCENE_MAX_FIELD_LEN+1]; //todo: we make this very big, so that a jlucene field bigger than LUCENE_MAX_FIELD_LEN won't corrupt the buffer
		for (int32_t i = 0; i < size; i++){
			input->readString(name,LUCENE_MAX_FIELD_LEN);
			uint8_t bits = input->readByte();
			bool isIndexed = (bits & 0x1) != 0;
			bool storeTermVector = (bits & 0x2) != 0;

			addInternal(name, isIndexed, storeTermVector);
		}
	}
	void FieldInfos::addInternal( const TCHAR* name, const bool isIndexed, const bool storeTermVector) {
		FieldInfo* fi = _CLNEW FieldInfo(name, isIndexed, byNumber.size(), storeTermVector);
		byNumber.push_back(fi);
		byName.put( fi->name, fi);
	}

   bool FieldInfos::hasVectors() const{
    for (int32_t i = 0; i < size(); i++) {
       if (fieldInfo(i)->storeTermVector)
          return true;
    }
    return false;
  }
CL_NS_END
