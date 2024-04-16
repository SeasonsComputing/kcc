#include "CLucene/StdHeader.h"

typedef unsigned long  gunichar;
typedef unsigned short guint16;
typedef          short gint16;
typedef          char  gchar;
typedef unsigned char  guchar;


// This code is derived from glib 2.4 which is also covered by the LGPL

/* These are the possible character classifications.
 * See http://www.unicode.org/Public/UNIDATA/UnicodeData.html
 */
typedef enum
{
  G_UNICODE_CONTROL,
  G_UNICODE_FORMAT,
  G_UNICODE_UNASSIGNED,
  G_UNICODE_PRIVATE_USE,
  G_UNICODE_SURROGATE,
  G_UNICODE_LOWERCASE_LETTER,
  G_UNICODE_MODIFIER_LETTER,
  G_UNICODE_OTHER_LETTER,
  G_UNICODE_TITLECASE_LETTER,
  G_UNICODE_UPPERCASE_LETTER,
  G_UNICODE_COMBINING_MARK,
  G_UNICODE_ENCLOSING_MARK,
  G_UNICODE_NON_SPACING_MARK,
  G_UNICODE_DECIMAL_NUMBER,
  G_UNICODE_LETTER_NUMBER,
  G_UNICODE_OTHER_NUMBER,
  G_UNICODE_CONNECT_PUNCTUATION,
  G_UNICODE_DASH_PUNCTUATION,
  G_UNICODE_CLOSE_PUNCTUATION,
  G_UNICODE_FINAL_PUNCTUATION,
  G_UNICODE_INITIAL_PUNCTUATION,
  G_UNICODE_OTHER_PUNCTUATION,
  G_UNICODE_OPEN_PUNCTUATION,
  G_UNICODE_CURRENCY_SYMBOL,
  G_UNICODE_MODIFIER_SYMBOL,
  G_UNICODE_MATH_SYMBOL,
  G_UNICODE_OTHER_SYMBOL,
  G_UNICODE_LINE_SEPARATOR,
  G_UNICODE_PARAGRAPH_SEPARATOR,
  G_UNICODE_SPACE_SEPARATOR
} GUnicodeType;


#include "gunichartables.h"

#define UTF8_COMPUTE(Char, Mask, Len)					      \
  if (Char < 128)							      \
    {									      \
      Len = 1;								      \
      Mask = 0x7f;							      \
    }									      \
  else if ((Char & 0xe0) == 0xc0)					      \
    {									      \
      Len = 2;								      \
      Mask = 0x1f;							      \
    }									      \
  else if ((Char & 0xf0) == 0xe0)					      \
    {									      \
      Len = 3;								      \
      Mask = 0x0f;							      \
    }									      \
  else if ((Char & 0xf8) == 0xf0)					      \
    {									      \
      Len = 4;								      \
      Mask = 0x07;							      \
    }									      \
  else if ((Char & 0xfc) == 0xf8)					      \
    {									      \
      Len = 5;								      \
      Mask = 0x03;							      \
    }									      \
  else if ((Char & 0xfe) == 0xfc)					      \
    {									      \
      Len = 6;								      \
      Mask = 0x01;							      \
    }									      \
  else									      \
    Len = -1;

#define UTF8_LENGTH(Char)              \
  ((Char) < 0x80 ? 1 :                 \
   ((Char) < 0x800 ? 2 :               \
    ((Char) < 0x10000 ? 3 :            \
     ((Char) < 0x200000 ? 4 :          \
      ((Char) < 0x4000000 ? 5 : 6)))))


#define UTF8_GET(Result, Chars, Count, Mask, Len)			      \
  (Result) = (Chars)[0] & (Mask);					      \
  for ((Count) = 1; (Count) < (Len); ++(Count))				      \
    {									      \
      if (((Chars)[(Count)] & 0xc0) != 0x80)				      \
  {								      \
    (Result) = -1;						      \
    break;							      \
  }								      \
      (Result) <<= 6;							      \
      (Result) |= ((Chars)[(Count)] & 0x3f);				      \
    }

#define ATTR_TABLE(Page) (((Page) <= G_UNICODE_LAST_PAGE_PART1) \
                          ? attr_table_part1[Page] \
                          : attr_table_part2[(Page) - 0xe00])

#define ATTTABLE(Page, Char) \
  ((ATTR_TABLE(Page) == G_UNICODE_MAX_TABLE_INDEX) ? 0 : (attr_data[ATTR_TABLE(Page)][Char]))


#define TTYPE_PART1(Page, Char) \
  ((type_table_part1[Page] >= G_UNICODE_MAX_TABLE_INDEX) \
   ? (type_table_part1[Page] - G_UNICODE_MAX_TABLE_INDEX) \
   : (type_data[type_table_part1[Page]][Char]))

#define TTYPE_PART2(Page, Char) \
  ((type_table_part2[Page] >= G_UNICODE_MAX_TABLE_INDEX) \
   ? (type_table_part2[Page] - G_UNICODE_MAX_TABLE_INDEX) \
   : (type_data[type_table_part2[Page]][Char]))

#define TYPE(Char) \
  (((Char) <= G_UNICODE_LAST_CHAR_PART1) \
   ? TTYPE_PART1 ((Char) >> 8, (Char) & 0xff) \
   : (((Char) >= 0xe0000 && (Char) <= G_UNICODE_LAST_CHAR) \
      ? TTYPE_PART2 (((Char) - 0xe0000) >> 8, (Char) & 0xff) \
      : G_UNICODE_UNASSIGNED))

/* Count the number of elements in an array. The array must be defined
 * as such; using this with a dynamically allocated array will give
 * incorrect results.
 */
#define G_N_ELEMENTS(arr)		(sizeof (arr) / sizeof ((arr)[0]))




#if defined(LUCENE_USE_INTERNAL_CHAR_FUNCTIONS)
#ifdef _LUCENE_PRAGMA_WARNINGS
 #pragma message ("===== Using internal character function =====")
#else
 #warning "===== Using internal character function ====="
#endif

bool cl_isletter(TCHAR ch)
{
	gunichar c=ch;
    int t = TYPE ((gunichar)c);
    switch(t)
    {
      case G_UNICODE_LOWERCASE_LETTER: return true;
      case G_UNICODE_TITLECASE_LETTER: return true;
      case G_UNICODE_UPPERCASE_LETTER: return true;
      case G_UNICODE_MODIFIER_LETTER: return true;
      case G_UNICODE_OTHER_LETTER: return true;
      default: return false;
    }
}

bool cl_isalnum(TCHAR ch)
{
	gunichar c=ch;
    int t = TYPE (c);
    switch(t)
    {
      case G_UNICODE_LOWERCASE_LETTER: return true;
      case G_UNICODE_TITLECASE_LETTER: return true;
      case G_UNICODE_UPPERCASE_LETTER: return true;
      case G_UNICODE_MODIFIER_LETTER: return true;
      case G_UNICODE_OTHER_LETTER: return true;
      case G_UNICODE_DECIMAL_NUMBER: return true;
      case G_UNICODE_LETTER_NUMBER: return true;
      case G_UNICODE_OTHER_NUMBER: return true;
      default: return false;
    }
}

bool cl_isdigit(TCHAR ch)
{
	gunichar c=ch;
    int t = TYPE ((gunichar)c);
    switch(t)
    {
      case G_UNICODE_DECIMAL_NUMBER: return true;
      case G_UNICODE_LETTER_NUMBER: return true;
      case G_UNICODE_OTHER_NUMBER: return true;
      default: return false;
    }
}

/**
 * cl_isspace:
 * @c: a Unicode character
 *
 * Determines whether a character is a space, tab, or line separator
 * (newline, carriage return, etc.).  Given some UTF-8 text, obtain a
 * character value with lucene_utf8towc().
 *
 * (Note: don't use this to do word breaking; you have to use
 * Pango or equivalent to get word breaking right, the algorithm
 * is fairly complex.)
 *
 * Return value: %TRUE if @c is a punctuation character
 **/
bool cl_isspace (TCHAR ch)
{
  gunichar c=ch;
  switch (c)
    {
      /* special-case these since Unicode thinks they are not spaces */
    case '\t':
    case '\n':
    case '\r':
    case '\f':
      return true;
      break;

    default:
      {
        int t = TYPE ((gunichar)c);
        return (t == G_UNICODE_SPACE_SEPARATOR || t == G_UNICODE_LINE_SEPARATOR
                || t == G_UNICODE_PARAGRAPH_SEPARATOR);
      }
      break;
    }
}



/**
 * cl_tolower:
 * @c: a Unicode character.
 *
 * Converts a character to lower case.
 *
 * Return value: the result of converting @c to lower case.
 *               If @c is not an upperlower or titlecase character,
 *               or has no lowercase equivalent @c is returned unchanged.
 **/
TCHAR cl_tolower (TCHAR ch)
{
  gunichar c=ch;
  int t = TYPE ((gunichar)c);
  if (t == G_UNICODE_UPPERCASE_LETTER)
  {
      gunichar val = ATTTABLE (c >> 8, c & 0xff);
      if (val >= 0x1000000)
      {
        const gchar *p = special_case_table + val - 0x1000000;
        int len=0;
		wchar_t ret=0;
		lucene_utf8towc(&ret,p,6);
#ifdef _UCS2
		  return ret;
#else
        if ( ret > 0x80 )
           return LUCENE_OOR_CHAR;
        else
           return (char)ret;
#endif
        //return cl_utf8_get_char (p, &len);
      }else
        return val ? val : c;
  }else if (t == G_UNICODE_TITLECASE_LETTER){
      unsigned int i;
      for (i = 0; i < G_N_ELEMENTS (title_table); ++i)
      {
        if (title_table[i][0] == c)
          return title_table[i][2];
      }
  }
  return c;
}

/**
 * cl_toupper:
 * @c: a Unicode character
 * 
 * Converts a character to uppercase.
 * 
 * Return value: the result of converting @c to uppercase.
 *               If @c is not an lowercase or titlecase character,
 *               or has no upper case equivalent @c is returned unchanged.
 **/
TCHAR cl_toupper (TCHAR ch)
{
  gunichar c=ch;
  int t = TYPE (c);
  if (t == G_UNICODE_LOWERCASE_LETTER)
    {
      gunichar val = ATTTABLE (c >> 8, c & 0xff);
      if (val >= 0x1000000)
	{
	  const gchar *p = special_case_table + val - 0x1000000;
	  
	  wchar_t ret=0;
	  lucene_utf8towc(&ret,p,6);
#ifdef _UCS2
	  return ret;
#else
     if ( ret > 0x80 )
        return LUCENE_OOR_CHAR;
     else
        return (char)ret;
#endif
	  //return lucene_utf8towc (p);
	}
      else
	return val ? val : c;
    }
  else if (t == G_UNICODE_TITLECASE_LETTER)
    {
      unsigned int i;
      for (i = 0; i < G_N_ELEMENTS (title_table); ++i)
	{
	  if (title_table[i][0] == c)
	    return title_table[i][1];
	}
    }
  return c;
}



/**
 * cl_tcasefold:
 * @str: a unicode string
 * @len: length of @str, in bytes, or -1 if @str is nul-terminated.
 *
 * Converts a string into a form that is independent of case. The
 * result will not correspond to any particular case, but can be
 * compared for equality or ordered with the results of calling
 * cl_tcasefold() on other strings.
 *
 * Note that calling cl_tcasefold() followed by g_utf8_collate() is
 * only an approximation to the correct linguistic case insensitive
 * ordering, though it is a fairly good one. Getting this exactly
 * right would require a more sophisticated collation function that
 * takes case sensitivity into account. GLib does not currently
 * provide such a function.
 *
 * Return value: a newly allocated string, that is a
 *   case independent form of @str.
 **/
TCHAR cl_tcasefold(const TCHAR ch){
    int start = 0;
    int end = G_N_ELEMENTS (casefold_table);
    
    if (ch >= casefold_table[start].ch &&
        ch <= casefold_table[end - 1].ch)
    {
        while (1)
        {
            int half = (start + end) / 2;
            if (ch == casefold_table[half].ch)
            {
				   wchar_t ret=0;
				   lucene_utf8towc(&ret,casefold_table[half].data,6);

               #ifdef _UCS2
		               return ret;
               #else
                     if ( ret > 0x80 )
                        return LUCENE_OOR_CHAR;
                     else
                        return (char)ret;
               #endif
            }else if (half == start){
                break;
            }else if (ch > casefold_table[half].ch){
                start = half;
            }else{
                end = half;
            }
        }
    }
    return cl_tolower(ch);
    
}
TCHAR* cl_tcscasefold( TCHAR * str, int len ) //len default is -1
{
    TCHAR *p = str;
    while ((len < 0 || p < str + len) && *p)
    {
        *p = cl_tcasefold(*p);
		p++;
    }
    return str;
}

int cl_tcscasefoldcmp(const TCHAR * dst, const TCHAR * src){
    TCHAR f,l;
    
    do{
        f = cl_tcasefold( (*(dst++)) );
        l = cl_tcasefold( (*(src++)) );
    } while ( (f) && (f == l) );
    
    return (int)(f - l);
}
#endif

/**
 * lucene_wctoutf8:
 * @c: a ISO10646 character code
 * @outbuf: output buffer, must have at least 6 bytes of space.
 *       If %NULL, the length will be computed and returned
 *       and nothing will be written to @outbuf.
 *
 * Converts a single character to UTF-8.
 *
 * Return value: number of bytes written
 **/
size_t	lucene_wctoutf8(char * outbuf, const wchar_t ch)
{
  gunichar c = ch;
  guchar len = 0;
  int first;
  int i;

  if (c < 0x80)
    {
      first = 0;
      len = 1;
    }
  else if (c < 0x800)
    {
      first = 0xc0;
      len = 2;
    }
  else if (c < 0x10000)
    {
      first = 0xe0;
      len = 3;
    }
   else if (c < 0x200000)
    {
      first = 0xf0;
      len = 4;
    }
  else if (c < 0x4000000)
    {
      first = 0xf8;
      len = 5;
    }
  else
    {
      first = 0xfc;
      len = 6;
    }

  if (outbuf)
  {
	for (i = len - 1; i > 0; --i)
	{
		outbuf[i] = (char)((c & 0x3f) | 0x80);
		c >>= 6;
	}
	outbuf[0] = c | first;
  }

  return len;
}


/**
 * lucene_utf8towc:
 * @p: a pointer to Unicode character encoded as UTF-8
 *
 * Converts a sequence of bytes encoded as UTF-8 to a Unicode character.
 * If @p does not point to a valid UTF-8 encoded character, results are
 * undefined. If you are not sure that the bytes are complete
 * valid Unicode characters, you should use lucene_utf8towc_validated()
 * instead.
 *
 * Return value: the resulting character
 **/
size_t lucene_utf8towc(wchar_t *pwc, const char *p, size_t n)
{
  int i, mask = 0;
  int result;
  unsigned char c = (unsigned char) *p;
  int len=0;

  UTF8_COMPUTE (c, mask, len);
  if (len == -1)
    return 0;
  UTF8_GET (result, p, i, mask, len);

  *pwc = result;
  return len;
}

size_t lucene_wcstoutf8(char * result, const wchar_t * str, size_t result_length){
  char *p=result;
  int i = 0;

  while (p < result + result_length-1 && str[i] != 0)
    p += lucene_wctoutf8(p,str[i++]);

  *p = '\0';

  return p-result; //todo: minus 1???
}

size_t lucene_utf8towcs(wchar_t * result, const char * str, size_t result_length){
  char *sp = (char*)str;
  wchar_t *rp = result;
  int i = 0;

  while (rp < result + result_length && *sp!=0){
    size_t r = lucene_utf8towc(rp,sp,6);
	if ( r == -1 )
		return 0;
	sp += r;
	rp++;
  }

  if ( sp-str < result_length )
	*rp = '\0';

  size_t ret = sp-str;
  return ret;
}

