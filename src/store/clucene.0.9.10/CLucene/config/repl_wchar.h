#ifndef _lucene_repl_wchar_h
#define _lucene_repl_wchar_h

#if defined(_UCS2) && !defined(_CL_HAVE_WCHAR_H)
    //all of these functions will be needed, because we don't have a wchar.h header
    
    #undef _tcscpy 
    #undef _tcsncpy
    #undef _tcscat
    #undef _tcschr
    #undef _tcsstr
    #undef _tcslen
    #undef _tcscmp
    #undef _tcsncmp
    #undef _tcscspn
    
    #define _tcscpy lucene_wcscpy //copy a string to another string
    #define _tcsncpy lucene_wcsncpy //copy a specified amount of one string to another string.
    #define _tcscat lucene_wcscat //copy a string onto the end of the other string
    #define _tcschr lucene_wcschr //find location of one character
    #define _tcsstr lucene_wcsstr //find location of a string
    #define _tcslen lucene_wcslen //get length of a string
    #define _tcscmp lucene_wcscmp //case sensitive compare two strings
    #define _tcsncmp lucene_wcsncmp //case sensitive compare two strings of a specified length
    #define _tcscspn lucene_wcscspn //Return the length of the maximum initial segment
                                    //of WCS which contains only wide-characters not in REJECT.
    
		/* Copy SRC to DEST.  */
    wchar_t * lucene_wcscpy ( wchar_t *dest, const wchar_t *src );
		/* Copy no more than N wide-characters of SRC to DEST.	*/
    wchar_t * lucene_wcsncpy (wchar_t *dest, const wchar_t *src, size_t n);
		/* Append SRC on the end of DEST.  */
    wchar_t *lucene_wcscat (wchar_t *dest, const wchar_t *src);
		/* Find the first occurrence of WC in WCS.  */
    wchar_t *lucene_wcschr (const wchar_t *wcs, const wchar_t wc);
		/* find needle in haystack */
    wchar_t * lucene_wcsstr (const wchar_t *haystack, const wchar_t *needle);
		/* Return length of string S.  */
    size_t lucene_wcslen (const wchar_t *s);
    /* Compare the first len characters of S1 and S2, returning less than, 
    	 equal to or greater than zero if S1 is lexicographically less than,
		   equal to or greater than S2.	 */
		int lucene_wcsncmp (const wchar_t *s1, const wchar_t *s2, size_t len);
    /* Compare S1 and S2, returning less than, equal to or
		   greater than zero if S1 is lexicographically less than,
		   equal to or greater than S2.	 */
		int lucene_wcscmp (const wchar_t *s1, const wchar_t *s2);
		/* Return the length of the maximum initial segment
		   of WCS which contains only wide-characters not in REJECT.  */
    size_t lucene_wcscspn (const wchar_t *wcs, const wchar_t *reject);
#endif

//formatting functions
#if defined(_UCS2) && !defined(_CL_HAVE_SNWPRINTF)
    #undef _sntprintf
    #define _sntprintf lucene_snwprintf
    int lucene_snwprintf(wchar_t *, size_t, const wchar_t *, ...);
#endif
#if defined(_UCS2) && !defined(_CL_HAVE_WPRINTF)
    #undef _tprintf
    #define _tprintf lucene_wprintf
    int lucene_wprintf(const wchar_t *, ...);
#endif
#if defined(_UCS2) && !defined(_CL_HAVE_VSNWPRINTF)
    #undef _vsntprintf
    #define _vsntprintf lucene_vsnwprintf
    int lucene_vsnwprintf(TCHAR * strbuf, size_t count, const wchar_t * format, va_list& ap);
#endif

//string function replacements
#if defined(LUCENE_USE_INTERNAL_CHAR_FUNCTIONS) || (defined(_UCS2) && !defined(_CL_HAVE_WCSCASECMP)) || (defined(_ASCII) && !defined(_CL_HAVE_STRCASECMP))
    int lucene_tcscasecmp(const TCHAR *, const TCHAR *);
    #undef _tcsicmp
    #define _tcsicmp lucene_tcscasecmp
#endif
#if defined(LUCENE_USE_INTERNAL_CHAR_FUNCTIONS) ||(defined(_ASCII) && !defined(_CL_HAVE_STRLWR)) || (defined(_UCS2) && !defined(_CL_HAVE_WCSLWR))
    TCHAR* lucene_tcslwr( TCHAR* str );
    #undef _tcslwr
    #define _tcslwr lucene_tcslwr
#endif

//conversion functions
#if (defined(_ASCII) && !defined(_CL_HAVE_LLTOA)) || (defined(_UCS2) && !defined(_CL_HAVE_LLTOW))
    TCHAR* lucene_i64tot( int64_t value, TCHAR* str, int radix);
    #undef _i64tot
    
    /*********************************************************************
		 *      _i64tot   (NTDLL.@)
		 *
		 * Converts a large integer to a string.
		 *
		 * RETURNS
		 *  Always returns str.
		 *
		 * NOTES
		 *  Converts value to a '\0' terminated string which is copied to str.
		 *  Does not check if radix is in the range of 2 to 36.
		 *  If str is NULL it crashes, as the native function does.
		 *
		 * DIFFERENCES
		 * - The native DLL converts negative values (for base 10) wrong:
		 *                     -1 is converted to -18446744073709551615
		 *                     -2 is converted to -18446744073709551614
		 *   -9223372036854775807 is converted to  -9223372036854775809
		 *   -9223372036854775808 is converted to  -9223372036854775808
		 *   The native msvcrt _i64toa function and our ntdll _i64toa function
		 *   do not have this bug.
		 */
    #define _i64tot lucene_i64tot
#endif
#if (defined(_ASCII) && !defined(_CL_HAVE_STRTOLL)) || (defined(_UCS2) && !defined(_CL_HAVE_WCSTOLL))
    int64_t lucene_tcstoi64(const TCHAR* str, TCHAR**end,const int radix);
    #undef _tcstoi64
    #define _tcstoi64 lucene_tcstoi64
#endif
#if defined(_UCS2) && !defined(_CL_HAVE_WCSTOD)
    double lucene_tcstod(const TCHAR *value, TCHAR **end);
    #undef _tcstod
    #define _tcstod lucene_tcstod
#endif

#endif
