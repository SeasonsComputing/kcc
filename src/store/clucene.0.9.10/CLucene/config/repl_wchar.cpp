#include "CLucene/StdHeader.h"
#include "CLucene/util/Misc.h"

#if defined(LUCENE_USE_INTERNAL_CHAR_FUNCTIONS) || (defined(_ASCII) && !defined(_CL_HAVE_STRLWR)) || (defined(_UCS2) && !defined(_CL_HAVE_WCSLWR))
#ifdef _LUCENE_PRAGMA_WARNINGS
 #pragma message ("===== Using internal tcslwr function =====")
#else
 #warning "===== Using internal tcslwr function ====="
#endif
TCHAR* lucene_tcslwr( TCHAR* str )
{
    TCHAR* ret = str;
    for ( ; *str; str++) *str = _totlower(*str);
    return ret;
}
#endif

#if (defined(_ASCII) && !defined(_CL_HAVE_LLTOA)) || (defined(_UCS2) && !defined(_CL_HAVE_LLTOW))
#ifdef _LUCENE_PRAGMA_WARNINGS
 #pragma message ("===== Using internal _i64tot function =====")
#else
 #warning "===== Using internal _i64tot function ====="
#endif


TCHAR* lucene_i64tot(
    int64_t value, /* [I] Value to be converted */
    TCHAR* str,      /* [O] Destination for the converted value */
    int radix)      /* [I] Number base for conversion */
{
    uint64_t val;
    int negative;
    TCHAR buffer[65];
    TCHAR* pos;
    int digit;

	if (value < 0 && radix == 10) {
	negative = 1;
		val = -value;
	} else {
	negative = 0;
		val = value;
	} /* if */

	pos = &buffer[64];
	*pos = '\0';

	do {
	digit = val % radix;
	val = val / radix;
	if (digit < 10) {
		*--pos = '0' + digit;
	} else {
		*--pos = 'a' + digit - 10;
	} /* if */
	} while (val != 0L);

	if (negative) {
	*--pos = '-';
	} /* if */

    //memcpy(str, pos, &buffer[64] - pos + 1);
	_tcsncpy(str,pos,&buffer[64] - pos + 1); //needed for unicode to work
    return str;
}
#endif //HAVE_LLTOA


#if (defined(_ASCII) && !defined(_CL_HAVE_STRTOLL)) || (defined(_UCS2) && !defined(_CL_HAVE_WCSTOLL))
#pragma message("===== Using internal tcstoi64 function =====")

int64_t lucene_tcstoi64(const TCHAR* str, TCHAR**end, int radix){
	#define LUCENE_TCSTOI64_RADIX(x,r) ((x>=_T('0') && x<=_T('9'))?x-_T('0'):((x>=_T('a') && x<=_T('z'))?x-_T('a')+10:((x>=_T('A') && x<=_T('Z'))?x-_T('A')+10:1000)))

	if (radix < 2 || radix > 36) 
		return 0;

	/* Skip white space.  */
	while (_istspace (*str))
		++str;

	int sign=1;
	if ( str[0] == _T('+') )
		str++;
	else if ( str[0] == _T('-') ){
		sign = -1;
		str++;
	}
		
  *end=(TCHAR*)str;
  long r = -1;
  while ( (r=LUCENE_TCSTOI64_RADIX(*end[0],radix)) >=0 && r<radix )
      (*end)++;

  TCHAR* p = (*end)-1;
  int64_t ret = 0;
  int pos = 0;
  for ( ;p>=str;p-- ){
      int i=LUCENE_TCSTOI64_RADIX(p[0],radix);
      if ( pos == 0 )
          ret=i;
      else
          ret += (int64_t)pow((float_t)radix,(float_t)pos) * i; //todo: might be quicker with a different pow overload

      pos++;
  }
  return sign*ret;
}
#endif //HAVE_STRTOLL

#if defined(LUCENE_USE_INTERNAL_CHAR_FUNCTIONS) || (defined(_UCS2) && !defined(_CL_HAVE_WCSCASECMP)) || (defined(_ASCII) && !defined(_CL_HAVE_STRCASECMP))
#ifdef _LUCENE_PRAGMA_WARNINGS
 #pragma message ("===== Using internal tcscasecmp function =====")
#else
 #warning "===== Using internal tcscasecmp function ====="
#endif
int lucene_tcscasecmp(const TCHAR * dst, const TCHAR * src){
    TCHAR f,l;
    
    do{
        f = _totlower( (*(dst++)) );
        l = _totlower( (*(src++)) );
    } while ( (f) && (f == l) );
    
    return (int)(f - l);
}
#endif

#if defined(_UCS2) && !defined(_CL_HAVE_WCSTOD)
#ifdef _LUCENE_PRAGMA_WARNINGS
 #pragma message ("===== Using internal tcstod function =====")
#else
 #warning "===== Using internal tcstod function ====="
#endif
    double lucene_tcstod(const TCHAR *value, TCHAR **end){
        int32_t len = _tcslen(value)+1;
        char* avalue=_CL_NEWARRAY(char,len);
        char* aend=NULL;
        STRCPY_TtoA(avalue,value,len);
        
        double ret = strtod(avalue,&aend);
        *end=(TCHAR*)value+(aend-avalue);
        _CLDELETE_CARRAY(avalue);

        return ret;
    }
#endif



#if defined(_UCS2) && ( !defined(_CL_HAVE_SNWPRINTF) || !defined(_CL_HAVE_WPRINTF) || !defined(_CL_HAVE_VSNWPRINTF) )

#define XWSN_WRITE_CHAR(val) if (consout) lucene_xwsnprintf_consout(val); else *str++=val;
void lucene_xwsnprintf_consout(TCHAR val){
  char buf[7];
	int len = wctomb(buf,val); 
	for ( int i=0;i<len;i++)
		fputc(buf[i],stdout);
}

//this code is based on Wine Unicode string manipulation functions
int lucene_xwsnprintf(bool consout, TCHAR *strbuf, size_t len, const TCHAR *format, va_list& valist)
{
 unsigned int written = 0;
 const TCHAR *iter = format;
 char bufa[256], fmtbufa[64], *fmta;
 TCHAR* str=strbuf;

 while (*iter)
 {
     while (*iter && *iter != '%')
     {
         if (written++ >= len)
             return -1;
		 XWSN_WRITE_CHAR(*iter++);
     }
     if (*iter == '%')
     {
         if (iter[1] == '%')
         {
             if (written++ >= len)
                 return -1;
			 XWSN_WRITE_CHAR('%');
             //*str++ = '%'; /* "%%"->'%' */
             iter += 2;
             continue;
         }

         fmta = fmtbufa;
         *fmta++ = *iter++;
         while (//*iter == '' ||
                *iter == '+' ||
                *iter == '-' ||
                *iter == ' ' ||
                *iter == '*' ||
                *iter == '#')
         {
             if (*iter == '*')
             {
                 char *buffiter = bufa;
                 int fieldlen = va_arg(valist, int);
                 sprintf(buffiter, "%d", fieldlen);
                 while (*buffiter)
                     *fmta++ = *buffiter++;
             }
             else
                 *fmta++ = *iter;
             iter++;
         }

         while (_istdigit(*iter))
             *fmta++ = *iter++;

         if (*iter == '.')
         {
             *fmta++ = *iter++;
             if (*iter == '*')
             {
                 char *buffiter = bufa;
                 int fieldlen = va_arg(valist, int);
                 sprintf(buffiter, "%d", fieldlen);
                 while (*buffiter)
                     *fmta++ = *buffiter++;
             }
             else
                 while (_istdigit(*iter))
                     *fmta++ = *iter++;
         }
         if (*iter == 'h' || *iter == 'l')
             *fmta++ = *iter++;

         switch (*iter)
         {
         case 's':
         {
             static const TCHAR none[] = { '(','n','u','l','l',')',0 };
             const TCHAR *wstr = va_arg(valist, const TCHAR *);
             const TCHAR *striter = wstr ? wstr : none;
             while (*striter)
             {
                 if (written++ >= len)
                     return -1;
				 XWSN_WRITE_CHAR(*striter++);
                 //*str++ = *striter++;
             }
             iter++;
             break;
         }

         case 'c':
             if (written++ >= len)
                 return -1;
			 XWSN_WRITE_CHAR((TCHAR)va_arg(valist, int));
             //*str++ = (TCHAR)va_arg(valist, int);

             iter++;
             break;

         default:
         {
             /* For non wc types, use system sprintf and append to wide char output */
             /* FIXME: for unrecognised types, should ignore % when printing */
             char *bufaiter = bufa;
             if (*iter == 'p')
                 sprintf(bufaiter, "%08lX", va_arg(valist, long));
             else
             {
                 *fmta++ = *iter;
                 *fmta = '\0';
                 if (*iter == 'a' || *iter == 'A' ||
                     *iter == 'e' || *iter == 'E' ||
                     *iter == 'f' || *iter == 'F' || 
                     *iter == 'g' || *iter == 'G')
                     sprintf(bufaiter, fmtbufa, va_arg(valist, double));
                 else
                 {
                     /* FIXME: On 32 bit systems this doesn't handle int 64's.
                      *        on 64 bit systems this doesn't work for 32 bit types
                      */
                     sprintf(bufaiter, fmtbufa, va_arg(valist, void *));
                 }
             }
             while (*bufaiter)
             {
                 if (written++ >= len)
                     return -1;
				 
				 XWSN_WRITE_CHAR(*bufaiter++);
                 //*str++ = *bufaiter++;
             }
             iter++;
             break;
         }
         }
     }
 }
 if (written >= len)
     return -1;
 if ( !consout ){
    XWSN_WRITE_CHAR(0);
 }
 return (int)written;
}
#endif


#if defined(_UCS2) && !defined(_CL_HAVE_SNWPRINTF)
#ifdef _LUCENE_PRAGMA_WARNINGS
 #pragma message ("===== Using internal snwprintf function =====")
#else
 #warning "===== Using internal snwprintf function ====="
#endif
int lucene_snwprintf(TCHAR * strbuf, size_t count, const wchar_t * format, ...){
    va_list ap;
    va_start(ap, format);
    int ret = lucene_xwsnprintf(false,strbuf,count,format,ap);
    va_end(ap);
    return ret;
}
#endif

#if defined(_UCS2) && !defined(_CL_HAVE_WPRINTF)
#ifdef _LUCENE_PRAGMA_WARNINGS
 #pragma message ("===== Using internal wprintf function =====")
#else
 #warning "===== Using internal wprintf function ====="
#endif
int lucene_wprintf(const wchar_t * format, ...){
    va_list ap;
    va_start(ap, format);
    int ret = lucene_xwsnprintf(true,NULL,INT_MAX,format,ap);
    va_end(ap);
    return ret;
}
#endif

#if defined(_UCS2) && !defined(_CL_HAVE_VSNWPRINTF)
#ifdef _LUCENE_PRAGMA_WARNINGS
 #pragma message ("===== Using internal vsnwprintf function =====")
#else
 #warning "===== Using internal vsnwprintf function ====="
#endif
int lucene_vsnwprintf(TCHAR * strbuf, size_t count, const wchar_t * format, va_list& ap){
    int ret = lucene_xwsnprintf(false,strbuf,INT_MAX,format,ap);
    return ret;
}
#endif
