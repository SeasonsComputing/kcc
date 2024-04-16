#include "CLucene/StdHeader.h"
#if defined(_UCS2) && !defined(_CL_HAVE_WCHAR_H)
#ifdef _LUCENE_PRAGMA_WARNINGS
 #pragma message ("===== Using all internal wchar.h replacement functions =====")
#else
 #warning "===== Using all internal wchar.h replacement functions ====="
#endif

//all of these functions will be needed, because we don't have a wchar.h header
//these functions come straight out of glibc

size_t lucene_wcscspn (const wchar_t *wcs, const wchar_t *reject)
{
  register size_t count = 0;

  while (*wcs != L'\0')
    if (_tcschr (reject, *wcs++) == NULL)
      ++count;
    else
      return count;

  return count;
}

int lucene_wcscmp (const wchar_t *s1, const wchar_t *s2)
{
  wchar_t c1, c2;

  do
    {
      c1 = (wchar_t) *s1++;
      c2 = (wchar_t) *s2++;
      if (c1 == L'\0')
	    return c1 - c2;
    }
  while (c1 == c2);

  return c1 - c2;
}

int lucene_wcsncmp (const wchar_t *s1, const wchar_t *s2, size_t len)
{
  wchar_t c1, c2;

  do
    {
      c1 = (wchar_t) *s1++;
      c2 = (wchar_t) *s2++;
      if (c1 == L'\0')
	    return c1 - c2;
    }
  while (c1 == c2);

  return c1 - c2;
}

size_t lucene_wcslen (const wchar_t *s)
{
  size_t len = 0;

  while (s[len] != L'\0')
    {
      if (s[++len] == L'\0')
	return len;
      if (s[++len] == L'\0')
	return len;
      if (s[++len] == L'\0')
	return len;
      ++len;
    }

  return len;
}

wchar_t *lucene_wcschr (const wchar_t *wcs, const wchar_t wc)
{
  do
    if (*wcs == wc)
      return (wchar_t *) wcs;
  while (*wcs++ != L'\0');

  return NULL;
}

wchar_t *lucene_wcscat (wchar_t *dest, const wchar_t *src)
{
  register wchar_t *s1 = dest;
  register const wchar_t *s2 = src;
  wchar_t c;

  /* Find the end of the string.  */
  do
    c = *s1++;
  while (c != L'\0');

  /* Make S1 point before the next character, so we can increment
     it while memory is read (wins on pipelined cpus).	*/
  s1 -= 2;

  do
    {
      c = *s2++;
      *++s1 = c;
    }
  while (c != L'\0');

  return dest;
}

wchar_t * lucene_wcsstr (const wchar_t *haystack, const wchar_t *needle)
{
  register wchar_t b, c;

  if ((b = *needle) != L'\0')
    {
      haystack--;				/* possible ANSI violation */
      do{
	    if ((c = *++haystack) == L'\0')
	    goto ret0;
      }while (c != b);

      if (!(c = *++needle))
	    goto foundneedle;
      ++needle;
      goto jin;

      for (;;)
      {
    	  register wchar_t a;
    	  register const wchar_t *rhaystack, *rneedle;
    
    	  do
    	  {
    	      if (!(a = *++haystack))
    		    goto ret0;
    	      if (a == b)
    		    break;
    	      if ((a = *++haystack) == L'\0')
    		    goto ret0;
            shloop:;
    	  }while (a != b);
    
          jin:  if (!(a = *++haystack))
    	            goto ret0;
    
    	  if (a != c)
    	    goto shloop;
    
    	  if (*(rhaystack = haystack-- + 1) == (a = *(rneedle = needle)))
    	    do
    	    {
        		if (a == L'\0')
        		  goto foundneedle;
        		if (*++rhaystack != (a = *++needle))
        		  break;
        		if (a == L'\0')
        		  goto foundneedle;
    	    }while (*++rhaystack == (a = *++needle));
    
    	  needle = rneedle;		  /* took the register-poor approach */
    
    	  if (a == L'\0')
    	    break;
    	}
    }
foundneedle:
  return (wchar_t*) haystack;
ret0:
  return NULL;
}

wchar_t * lucene_wcsncpy (wchar_t *dest, const wchar_t *src, size_t n)
{
  wchar_t c;
  wchar_t *const s = dest;

  --dest;

  if (n >= 4)
  {
    size_t n4 = n >> 2;

    for (;;)
	{
	  c = *src++;
	  *++dest = c;
	  if (c == L'\0')
	    break;
	  c = *src++;
	  *++dest = c;
	  if (c == L'\0')
	    break;
	  c = *src++;
	  *++dest = c;
	  if (c == L'\0')
	    break;
	  c = *src++;
	  *++dest = c;
	  if (c == L'\0')
	    break;
	  if (--n4 == 0)
	    goto last_chars;
	}
      n = n - (dest - s) - 1;
      if (n == 0)
	return s;
      goto zero_fill;
    }

 last_chars:
  n &= 3;
  if (n == 0)
    return s;

  do
    {
      c = *src++;
      *++dest = c;
      if (--n == 0)
	return s;
    }
  while (c != L'\0');

 zero_fill:
  do
    *++dest = L'\0';
  while (--n > 0);

  return s;
}

wchar_t * lucene_wcscpy ( wchar_t *dest, const wchar_t *src )
{
  wchar_t *wcp = (wchar_t *) src;
  int32_t c;
  const ptrdiff_t off = dest - src - 1;

  do{
      c = *wcp++;
      wcp[off] = c;
  }while (c != L'\0');

  return dest;
}

#endif
