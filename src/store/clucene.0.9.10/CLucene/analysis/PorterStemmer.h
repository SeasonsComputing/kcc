/* This is the Porter stemming algorithm, originally written by Martin Porter.
   It may be regarded as cononical, in that it follows the
   algorithm presented in

   Porter, 1980, An algorithm for suffix stripping, Program, Vol. 14,
   no. 3, pp 130-137,

   See also http://www.tartarus.org/~martin/PorterStemmer

   Modified by "Hemant Muthiyan"
   email: hemant_muthiyan@yahoo.co.in

*/

#include "CLucene/StdHeader.h"

CL_NS_DEF(analysis)

class PorterStemmer
{
private:
	TCHAR *b;
    size_t i,    /* offset into b */
    j, k, k0;
	bool dirty;
    //private static final int32_t EXTRA = 1;



  /* cons(i) is true <=> b[i] is a consonant. */

private:
	bool cons(size_t i) {
		switch (b[i]) {
			case 'a': case 'e': case 'i': case 'o': case 'u':
			return false;
			case 'y':
			return (i==k0) ? true : !cons(i-1);
			default:
			return true;
		}
	}

  /* m() measures the number of consonant sequences between k0 and j. if c is
     a consonant sequence and v a vowel sequence, and <..> indicates arbitrary
     presence,

          <c><v>       gives 0
          <c>vc<v>     gives 1
          <c>vcvc<v>   gives 2
          <c>vcvcvc<v> gives 3
          ....
  */

   int32_t m() {
    int32_t n = 0;
    size_t i = k0;
    while(true) {
      if (i > j)
        return n;
      if (! cons(i))
        break;
      i++;
    }
    i++;
    while(true) {
      while(true) {
        if (i > j)
          return n;
        if (cons(i))
          break;
        i++;
      }
      i++;
      n++;
      while(true) {
        if (i > j)
          return n;
        if (! cons(i))
          break;
        i++;
      }
      i++;
    }
  }

  /* vowelinstem() is true <=> k0,...j contains a vowel */

   bool vowelinstem() {
    for (size_t i = k0; i <= j; i++)
      if (! cons(i))
        return true;
    return false;
  }

  /* doublec(j) is true <=> j,(j-1) contain a double consonant. */
   bool doublec(size_t j) {
    if (j < k0+1)
      return false;
    if (b[j] != b[j-1])
      return false;
    return cons(j);
  }

  /* cvc(i) is true <=> i-2,i-1,i has the form consonant - vowel - consonant
     and also if the second c is not w,x or y. this is used when trying to
     restore an e at the end of a short word. e.g.

          cav(e), lov(e), hop(e), crim(e), but
          snow, box, tray.

  */
   bool cvc(size_t i) {
    if (i < k0+2 || !cons(i) || cons(i-1) || !cons(i-2))
      return false;
    else {
      int32_t ch = b[i];
      if (ch == 'w' || ch == 'x' || ch == 'y') return false;
    }
    return true;
  }

  bool ends(TCHAR *s) {
	size_t l = _tcslen(s);
    size_t o = k-l+1;
    if (o < k0)
      return false;
    for (size_t i = 0; i < l; i++)
      if (b[o+i] != s[i])
        return false;
    j = k-l;
    return true;
  }

  /* setto(s) sets (j+1),...k to the characters in the string s, readjusting
     k. */

  void setto(TCHAR *s) {
    size_t l = _tcslen(s);
    size_t o = j+1;
    for (size_t i = 0; i < l; i++)
      b[o+i] = s[i];
    k = j+l;
    dirty = true;
  }

  /* r(s) is used further down. */

  void r(TCHAR *s) { if (m() > 0) setto(s); }

  /* step1() gets rid of plurals and -ed or -ing. e.g.

           caresses  ->  caress
           ponies    ->  poni
           ties      ->  ti
           caress    ->  caress
           cats      ->  cat

           feed      ->  feed
           agreed    ->  agree
           disabled  ->  disable

           matting   ->  mat
           mating    ->  mate
           meeting   ->  meet
           milling   ->  mill
           messing   ->  mess

           meetings  ->  meet

  */

  void step1() {
    if (b[k] == _T('s')) {
      if (ends(_T("sses"))) k -= 2;
      else if (ends(_T("ies"))) setto(_T("i"));
      else if (b[k-1] != _T('s')) k--;
    }
    if (ends(_T("eed"))) {
      if (m() > 0)
        k--;
    }
    else if ((ends(_T("ed")) || ends(_T("ing"))) && vowelinstem()) {
      k = j;
      if (ends(_T("at"))) setto(_T("ate"));
      else if (ends(_T("bl"))) setto(_T("ble"));
      else if (ends(_T("iz"))) setto(_T("ize"));
      else if (doublec(k)) {
        int32_t ch = b[k--];
        if (ch == _T('l') || ch == _T('s') || ch == _T('z'))
          k++;
      }
      else if (m() == 1 && cvc(k))
        setto(_T("e"));
    }
  }

  /* step2() turns terminal y to i when there is another vowel in the stem. */

  void step2() {
    if (ends(_T("y")) && vowelinstem()) {
      b[k] = 'i';
      dirty = true;
    }
  }

  /* step3() maps double suffices to single ones. so -ization ( = -ize plus
     -ation) maps to -ize etc. note that the string before the suffix must give
     m() > 0. */

  void step3() {
    if (k == k0) return; /* For Bug 1 */
    switch (b[k-1]) {
    case 'a':
      if (ends(_T("ational"))) { r(_T("ate")); break; }
      if (ends(_T("tional"))) { r(_T("tion")); break; }
      break;
    case 'c':
      if (ends(_T("enci"))) { r(_T("ence")); break; }
      if (ends(_T("anci"))) { r(_T("ance")); break; }
      break;
    case 'e':
      if (ends(_T("izer"))) { r(_T("ize")); break; }
      break;
    case 'l':
      if (ends(_T("bli"))) { r(_T("ble")); break; }
      if (ends(_T("alli"))) { r(_T("al")); break; }
      if (ends(_T("entli"))) { r(_T("ent")); break; }
      if (ends(_T("eli"))) { r(_T("e")); break; }
      if (ends(_T("ousli"))) { r(_T("ous")); break; }
      break;
    case 'o':
      if (ends(_T("ization"))) { r(_T("ize")); break; }
      if (ends(_T("ation"))) { r(_T("ate")); break; }
      if (ends(_T("ator"))) { r(_T("ate")); break; }
      break;
    case 's':
      if (ends(_T("alism"))) { r(_T("al")); break; }
      if (ends(_T("iveness"))) { r(_T("ive")); break; }
      if (ends(_T("fulness"))) { r(_T("ful")); break; }
      if (ends(_T("ousness"))) { r(_T("ous")); break; }
      break;
    case 't':
      if (ends(_T("aliti"))) { r(_T("al")); break; }
      if (ends(_T("iviti"))) { r(_T("ive")); break; }
      if (ends(_T("biliti"))) { r(_T("ble")); break; }
      break;
    case 'g':
      if (ends(_T("logi"))) { r(_T("log")); break; }
    }
  }

  /* step4() deals with -ic-, -full, -ness etc. similar strategy to step3. */

  void step4() {
    switch (b[k]) {
    case 'e':
      if (ends(_T("icate"))) { r(_T("ic")); break; }
      if (ends(_T("ative"))) { r(LUCENE_BLANK_STRING); break; }
      if (ends(_T("alize"))) { r(_T("al")); break; }
      break;
    case 'i':
      if (ends(_T("iciti"))) { r(_T("ic")); break; }
      break;
    case 'l':
      if (ends(_T("ical"))) { r(_T("ic")); break; }
      if (ends(_T("ful"))) { r(LUCENE_BLANK_STRING); break; }
      break;
    case 's':
      if (ends(_T("ness"))) { r(LUCENE_BLANK_STRING); break; }
      break;
    }
  }

  /* step5() takes off -ant, -ence etc., in context <c>vcvc<v>. */

  void step5() {
    if (k == k0) return; /* for Bug 1 */
    switch (b[k-1]) {
    case 'a':
      if (ends(_T("al"))) break;
      return;
    case 'c':
      if (ends(_T("ance"))) break;
      if (ends(_T("ence"))) break;
      return;
    case 'e':
      if (ends(_T("er"))) break; return;
    case 'i':
      if (ends(_T("ic"))) break; return;
    case 'l':
      if (ends(_T("able"))) break;
      if (ends(_T("ible"))) break; return;
    case 'n':
      if (ends(_T("ant"))) break;
      if (ends(_T("ement"))) break;
      if (ends(_T("ment"))) break;
      /* element etc. not stripped before the m */
      if (ends(_T("ent"))) break;
      return;
    case 'o':
      if (ends(_T("ion")) && j >= 0 && (b[j] == 's' || b[j] == 't')) break;
      /* j >= 0 fixes Bug 2 */
      if (ends(_T("ou"))) break;
      return;
      /* takes care of -ous */
    case 's':
      if (ends(_T("ism"))) break;
      return;
    case 't':
      if (ends(_T("ate"))) break;
      if (ends(_T("iti"))) break;
      return;
    case 'u':
      if (ends(_T("ous"))) break;
      return;
    case 'v':
      if (ends(_T("ive"))) break;
      return;
    case 'z':
      if (ends(_T("ize"))) break;
      return;
    default:
      return;
    }
    if (m() > 1)
      k = j;
  }

  /* step6() removes a final -e if m() > 1. */

  void step6() {
    j = k;
    if (b[k] == 'e') {
      int32_t a = m();
      if (a > 1 || a == 1 && !cvc(k-1))
        k--;
    }
    if (b[k] == 'l' && doublec(k) && m() > 1)
      k--;
  }




 public:

 	PorterStemmer(TCHAR *Text) {
     b = Text;
     i = _tcslen(b);
 	dirty = false;
   }

   ~PorterStemmer(){
 		b = NULL;
 	}


   /**
    * Returns the length of the word resulting from the stemming process.
    */
   int32_t getResultLength() { return i; }

	 bool stem() {
    //i = strlen(b);
		 k = i -1;
    k0 = 0;
    if (k > k0+1) {
      step1(); step2(); step3(); step4(); step5(); step6();
    }
    // Also, a word is considered dirty if we lopped off letters
    // Thanks to Ifigenia Vairelles for pointing this out.
    if (i != k+1)
      dirty = true;
    i = k+1;
    return dirty;
  }

  /**
   * Returns a reference to a character buffer containing the results of
   * the stemming process.  You also need to consult getResultLength()
   * to determine the length of the result.
   */
  const TCHAR* getResultBuffer() { return b; }

};
CL_NS_END
