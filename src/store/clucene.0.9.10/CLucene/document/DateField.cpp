#include "CLucene/StdHeader.h"

#include "DateField.h"
#include "CLucene/util/Misc.h"
CL_NS_USE(util)
CL_NS_DEF(document)

	DateField::~DateField(){
	}
	
  TCHAR* DateField::timeToString(const int64_t time) {
    if (time < 0)
      _CLTHROWA (CL_ERR_IllegalArgument,"time too early");
    /* DSR:2004.10.28: */
    /* Check for too-late time here rather than after the _i64tot call
    ** so as to avoid buffer overflow potential. */
    if (time > DATEFIELD_DATE_MAX)
      _CLTHROWA (CL_ERR_IllegalArgument, "time too late (past DATEFIELD_DATE_MAX");

	 TCHAR* buf = _CL_NEWARRAY(TCHAR,DATEFIELD_DATE_LEN + 1);
    _i64tot(time, buf, 36);
    int32_t bufLen = _tcslen(buf);

    CND_PRECONDITION (bufLen <= DATEFIELD_DATE_LEN, "timeToString length is greater than 9");

    /* Supply leading zeroes if necessary. */
    if (bufLen < DATEFIELD_DATE_LEN) {
      const int32_t nMissingZeroes = DATEFIELD_DATE_LEN - bufLen;
      /* Move buffer contents forward to make room for leading zeroes. */
      for (int32_t i = DATEFIELD_DATE_LEN - 1; i >= nMissingZeroes; i--)
        buf[i] = buf[i - nMissingZeroes];
      
	  {// MSVC6 scoping fix
	  /* Insert leading zeroes. */
      for (int32_t i = 0; i < nMissingZeroes; i++)
        buf[i] = '0';
	  }

      buf[DATEFIELD_DATE_LEN] = 0;
    }

    CND_PRECONDITION (_tcslen(buf) == DATEFIELD_DATE_LEN, "timeToString return is not equal to 9");

    return buf;
  }

  int64_t DateField::stringToTime(const TCHAR* time) {
    TCHAR* end;
    return _tcstoi64(time, &end, 36);
  }

CL_NS_END
