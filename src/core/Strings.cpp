/*
 * Kuumba C++ Core
 *
 * $Id: Strings.cpp 21801 2008-01-09 22:01:15Z tvk $
 */
#include <inc/core/Core.h>

#if defined(KCC_WINDOWS)
#   define kcc_strtok(_s,_sep,_lasts) (*(_lasts)=strtok((_s),(_sep)))
#elif defined(KCC_LINUX)
#   define kcc_strtok strtok_r
#endif

#define KCC_FILE "Strings"

namespace kcc
{
    // Constants
    static const String   k_empty  ("");
    static const StringRC k_emptyRC(k_empty);
    static const String   k_pipe   ("|");
    
    // Constants: regex's
    static const String   k_rx_word       ("\\w");
    static const String   k_rx_split      ("\\s+");
    static const String   k_rx_decimal    ("^\\d$");
    static const String   k_rx_fraction   ("^\\d.*\\d$");
    static const String   k_rx_reserved   ("(\\/|\\.|\\*|\\+|\\?|\\||\\(|\\)|\\[|\\]|\\{|\\}|\\\\)");
    static const String   k_rx_replace    ("\\\\$1");
    static const String   k_rx_ws_all     ("\\s");
    static const String   k_rx_ws_reduce  ("[ ]{2,}");
    static const String   k_rx_ws_trim    ("^[ ]*|[ ]*$");
    static const String   k_rx_ws_replace (" ");

    // Constants: Select
    static const Char     k_selectOp       = ':';
    static const Char     k_selectEqual    = '@';
    static const Char     k_selectNotEq    = '!';
    static const Char     k_selectBegins   = '^';
    static const Char     k_selectEnds     = '$';
    static const Char     k_selectContains = '~';
    static const Char     k_selectEnum     = '|';

    // Constants: Matches
    static const String   k_enumBoolTerms ("true|false");
    static const String   k_matchBoolean  ("{boolean}");
    static const String   k_matchBoolTrue ("{true}");
    static const String   k_matchBoolFalse("{false}");
    static const String   k_matchISODate  ("{isodate}");
    static const String   k_matchNumber   ("{number}");
    static const String   k_matchRX       ("{rx:");
    static const String   k_matchRXNot    ("{rx!");

    // Char stack buffer size
    const std::size_t SZ        = 1024;
    const std::size_t SZ_PRINTF = SZ*8;
    const std::size_t SZ_IOREAD = SZ*4;

    //
    // lower implementation
    //

    struct lower
    {
        const std::locale& m_locale;
        lower(const std::locale& l) : m_locale(l) {}
        Char operator() (Char c) const { return std::tolower(c, m_locale); }
    };

    //
    // upper Implementation
    //

    struct upper
    {
        const std::locale& m_locale;
        upper(const std::locale& l) : m_locale(l) {}
        Char operator() (Char c) const { return std::toupper(c, m_locale); }
    };

    // k_isalnum: check alpha and '_' and numeric
    inline bool k_isalnum(const Char& c, const std::locale& loc)
    {
        return std::isalpha(c, loc) || c == '_' || (c >= '0' && c <= '9');
    }

    //
    // Strings Implementation
    //

    // empty: return the empty string
    const String&   Strings::empty()   { return k_empty;   }
    const StringRC& Strings::emptyRC() { return k_emptyRC; }

    // toLower: convert strint to lower-case
    String& Strings::toLower(String& in)
    {
        std::locale loc;
        std::transform(in.begin(), in.end(), in.begin(), lower(loc));
        return in;
    }

    // toLower: convert char to lower-case
    Char Strings::toLower(const Char& in)
    {
        std::locale loc;
        return std::tolower(in, loc);
    }

    // toLower: convert char to lower-case preserving input
    String Strings::toLower(const String& in)
    {
        String s(in);
        return Strings::toLower(s);
    }

    // toUpper: convert string to upper-case
    String& Strings::toUpper(String& in)
    {
        std::locale loc;
        std::transform(in.begin(), in.end(), in.begin(), upper(loc));
        return in;
    }

    // toUpper: convert char to upper-case
    Char Strings::toUpper(const Char& in)
    {
        std::locale loc;
        return std::toupper(in, loc);
    }

    // toUpper: convert char to upper-case preserving input
    String Strings::toUpper(const String& in)
    {
        String s(in);
        return Strings::toUpper(s);
    }

    // trim: trim node non-alpha's
    String& Strings::trim(String& in)
    {
        std::locale loc;
        String::size_type sz = in.length();
        String::size_type l;
        for (l = 0; l < sz; l++)
        {
            if (k_isalnum(in[l], loc)) break;
        }
        if (l >= sz)
        {
            in.clear();
            return in;
        }
        String::size_type r;
        for (r = sz - 1; r >= 0; r--)
        {
            if (k_isalnum(in[r], loc)) break;
        }
        if (l <= r)
        {
            in.erase(r+1, sz);
            in.erase(0, l);
        }
        return in;
    }

    // trim: trim non-alphas preserving in
    String Strings::trim(const String& in)
    {
        String s(in);
        return Strings::trim(s);
    }

    // trimws: trim white space
    String& Strings::trimws(String& in)
    {
        std::locale loc;
        String::size_type sz = in.length();
        String::size_type l;
        for (l = 0; l < sz; l++)
        {
            if (!std::isspace(in[l], loc)) break;
        }
        if (l >= sz)
        {
            in.clear();
            return in;
        }
        String::size_type r;
        for (r = sz - 1; r >= 0; r--)
        {
            if (!std::isspace(in[r], loc)) break;
        }
        if (l <= r)
        {
            in.erase(r+1, sz);
            in.erase(0, l);
        }
        return in;
    }
    
    // cleanws: replace ws meta-chars into space, and l-trim & r-trim ws    
    String Strings::cleanws(const String& t) throw (Exception)
    {
        String clean(t);
        return Strings::cleanws(clean);
    }

    // cleanws: replace ws meta-chars into space, and l-trim & r-trim ws    
    String& Strings::cleanws(String& t) throw (Exception)
    {
        Log::Scope scope(KCC_FILE, "cleanws");
        IRegex* rx = Core::regex();
        rx->replace(t, k_rx_ws_all,    k_rx_ws_replace);
        rx->replace(t, k_rx_ws_reduce, k_rx_ws_replace);
        rx->replace(t, k_rx_ws_trim,   k_empty);
        return t;
    }
    
    // isws: is string all white space
    bool Strings::isws(const String& in)
    {
        std::locale loc;
        String::size_type sz = in.length();
        for (String::size_type l = 0; l < sz; l++)
        {
            if (!std::isspace(in[l], loc)) return false;
        }
        return true;
    }

    // trimws: trim white space preserving in
    String Strings::trimws(const String& in)
    {
        String s(in);
        return Strings::trimws(s);
    }

    // xmlEncode: encode xml entity references
    String Strings::xmlEncode(const String& in)
    {
        String s;
        String::size_type sz = in.length();
        for (String::size_type i = 0; i < sz; i++)
        {
            Char c = in[i];
            if      (c == '<')  s += "&lt;";
            else if (c == '&')  s += "&amp;";
            else if (c == '>')  s += "&gt;";
            else if (c == '\"') s += "&quot;";
            else if (c == '\'') s += "&apos;";
            else if (c == 9 || c == 10 || c == 13 || c >= 127) s += Strings::printf("&#%d;", (int)(unsigned char)c);
            else if (c < 32)    s += " "; // invalid XML, throw exception ?
            else                s += c;
        }
        return s;
    }

    // xmlDecode: decode xml entity references
    String Strings::xmlDecode(const String& in)
    {
        String s;
        String::size_type sz = in.length();
        for (String::size_type i = 0; i < sz; i++)
        {
            Char c = in[i];
            if (c == '&')
            {
                String::size_type eoc = in.find_first_of(';', i);
                if (eoc == String::npos)
                {
                    // TODO: invalid XML, throw exception ?
                    s += c;
                }
                else
                {
                    String::size_type cnt = eoc-i;
                    if      (in.substr(i, 4) == "&lt;")   s += '<'; 
                    else if (in.substr(i, 5) == "&amp;")  s += '&'; 
                    else if (in.substr(i, 4) == "&gt;")   s += '>'; 
                    else if (in.substr(i, 6) == "&quot;") s += '\"';
                    else if (in.substr(i, 6) == "&apos;") s += '\'';
                    else if (in.substr(i, 2) == "&#")
                    {
                        int parse;
                        if (in[i+2] == 'x' || in[i+2] == 'X') 
                            std::sscanf(in.substr(i+3, cnt-3).c_str(), "%x", &parse);
                        else                                  
                            std::sscanf(in.substr(i+2, cnt-2).c_str(), "%d", &parse);
                        s += (Char) parse;
                    }
                    else
                    {
                        // TODO: invalid XML, throw exception ?
                        s += in.substr(i, cnt);
                    }                
                    i += cnt;
                }
            }
            else
            {
                s += c;
            }
        }
        return s;
    }

    // isSpace: determine if char is space
    bool Strings::isSpace(const Char& in)
    {
        std::locale loc;
        return std::isspace(in, loc);
    }

    // isAlpha: determine if char is alpha
    bool Strings::isAlpha(const Char& in)
    {
        std::locale loc;
        return std::isalpha(in, loc);
    }

    // isDigit: determine if char is a digit
    bool Strings::isDigit(const Char& in)
    {
        std::locale loc;
        return std::isdigit(in, loc);
    }

    // isAlnum: determine if char is a alphabetic or numeric
    bool Strings::isAlnum(const Char& in)
    {
        std::locale loc;
        return std::isalnum(in, loc);
    }

    // isLower: determine if char is lowercase
    bool Strings::isLower(const Char& in)
    {
        std::locale loc;
        return std::islower(in, loc);
    }

    // isUpper: determine if char is uppercase
    bool Strings::isUpper(const Char& in)
    {
        std::locale loc;
        return std::isupper(in, loc);
    }

    // printf: printf style into String
    String Strings::printf(const Char *format, ...)
    {
        Char buf[SZ_PRINTF];
        va_list arg;
        va_start(arg, format);
        kcc_vsprintf(buf, SZ_PRINTF, format, arg);
        va_end(arg);
        buf[SZ_PRINTF-1] = 0;
        return buf;
    }

    // tokenize: tokenize string
    void Strings::tokenize(const String& s, const String& seps, StringVector& tokens, bool clear)
    {
        if (clear) tokens.clear();
        Char* search = new Char[s.length()+1];
        std::strcpy(search, s.c_str());
        Char* next  = NULL;
        Char* token = kcc_strtok(search, seps.c_str(), &next);
        while (NULL != token)
        {
            String t(token);
            Strings::trimws(t);
            tokens.push_back(t);
            token = kcc_strtok(NULL, seps.c_str(), &next);
        }
        delete [] search;
    }

    // csvLine: read a csv line from stream
    bool Strings::csv(std::istream& in, StringVector& tokens, Char sep, bool clear)
    {
        if (clear) tokens.clear();
        if (in.eof()) return false;
        String col;
        short quote = 0;
        Char c = 0;
        in.read(&c, 1);
        while (!in.eof())
        {
            if (c == '\xa' || c == '\xd')
            {
                Char peek = in.peek();
                if (c == '\xd' && peek == '\xa') in.read(&peek, 1); // linux eol
                
                // end of line break; otherwise embedded
                if (!quote) 
                {
                    // add column
                    tokens.push_back(col);
                    break;
                }
                
                // embedded eol
                col += c;
                if (peek == '\xa') col += peek; // linux eol
            }
			else if (c == '\"')
			{
				if (!quote)
				{
				    // begin quote
					quote++;
				}
				else	
				{
					// end quote or embedded quote (embedded quotes are "" inside the quoted string)
					if (quote >= 2)
					{
						quote--;
					}
                    else
                    {
					    // peek ahead at next char for embedded quote
					    Char peek = in.peek();
					    if (peek == '\"')
					    {
						    // embedded quote
						    quote++;
                            col += c; // build column
					    }
					    else
					    {
						    // end quote (TODO: check for balanced quotes here; quote--)
						    quote = 0;
					    }
					}
				}
            }
			else if (!quote && c == sep)
            {
                // add column
                tokens.push_back(col);
                col.clear();
            }
            else
            {
                col += c; // build column
            }
            
            // next char
            in.read(&c, 1);
            
            // add column
            if (in.eof()) tokens.push_back(col);
        }
        return !tokens.empty();
    }
    
    // csv: tokenize CSV string
    void Strings::csv(const String& s, StringVector& tokens, Char sep, bool clear)
    {
        StringStream in(s);
        csv(in, tokens, sep, clear);
    }

    // parse*: convert string to scalar
    double Strings::parseFraction(const String& v) { return std::atof(v.c_str()); }
    long   Strings::parseInteger(const String& v)  { return std::atol(v.c_str()); }

    // join: join strings with separator
    void Strings::join(
        const String& sep,
        StringVector::const_iterator b,
        StringVector::const_iterator e,
        String& out)
    {
        out.clear();
        out.reserve((e-b)*(sep.length()+32));
        for (StringVector::const_iterator i = b; i != e; i++)
        {
            if (i != b) out += sep;
            out += *i;
        }
    }

    // join: join strings with separator
    String Strings::join(
        const String& sep,
        StringVector::const_iterator b,
        StringVector::const_iterator e)
    {
        String out;
        join(sep, b, e, out);
        return out;
    }

    // loadText: load text file into string
    bool Strings::loadText(const String& path, String& text, bool clear)
    {
        Platform::File f;
        FILE* in = NULL;
        if (!Platform::fsFile(path, f) || (in = std::fopen(path.c_str(), "rt")) == NULL) return false;
        Char buf[SZ_IOREAD];
        if (clear) text.clear();
        text.reserve(f.size);
        while (!feof(in))
        {
            std::size_t read = std::fread(buf, sizeof(Char), SZ_IOREAD, in);
            text.append(buf, read);
        }
        std::fclose(in);
        return true;
    }
    
    // k_oneOf: search for value in pipe separated list
    inline bool k_oneOf(const String& value, const String& expr, String::size_type off = 0)
    {
        const Char sep = k_pipe[0];
        const String::size_type valsz = value.length();
        String::size_type start = off;
        String::size_type end   = expr.find_first_of(sep, start);
        while (end != String::npos)
        {
            if (expr.find(value, start) == start && valsz == end-start) return true;
            start = end + 1;
            end = expr.find_first_of(sep, start);
        }
        // last enum (no trailing pipe) 
        return expr.find(value, start) == start && valsz == expr.length()-start;
    }

    // select: match value selection to expression
    bool Strings::select(const String& value, const String& expr, SelectDefault def)
    {
        const String::size_type opsz = 2;
        String::size_type exsz = 0;
        if ((exsz = expr.length()) > opsz && expr[1] == k_selectOp)
        {
            // operator
            const Char*             text = expr.c_str() + opsz;
            const String::size_type txsz = exsz - opsz;
            const Char op = expr[0];
            if      (op == k_selectEqual)    return value == text;
            else if (op == k_selectNotEq)    return value != text;
            else if (op == k_selectBegins)   return value.find(text) == 0;
            else if (op == k_selectEnds)     return value.rfind(text) == value.length()-txsz;
            else if (op == k_selectContains) return value.find(text) != String::npos;
            else if (op == k_selectEnum)     return k_oneOf(value, expr, opsz);
            else                             throw std::logic_error("invalid Strings::select() operator");
        }
        else
        {
            // default
            switch (def)
            {
            case SD_EQUAL:    return value == expr;
            case SD_BEGINS:   return value.find(expr) == 0;
            case SD_CONTAINS: return value.find(expr) != String::npos;
            case SD_ENUM:     return k_oneOf(value, expr);
            default:          throw std::logic_error("invalid Strings::select() default");
            };
        }
        return false;
    }

    // match: match value to expression
    bool Strings::match(const String& value, const String& expr, SelectDefault sd) throw (Exception)
    {
        Log::Scope scope(KCC_FILE, "match");
        bool match = false;
        if (expr.find(k_matchBoolean) == 0)
        {
            match = Strings::select(value, k_enumBoolTerms, Strings::SD_ENUM);
        }
        else if (expr.find(k_matchISODate) == 0)
        {
            match = ISODate::valid(value);
        }
        else if (expr.find(k_matchNumber) == 0)
        {
            match = 
                Core::regex()->match(value, k_rx_decimal) || 
                Core::regex()->match(value, k_rx_fraction);
        }
        else if (expr.find(k_matchRX) == 0 || expr.find(k_matchRXNot) == 0)
        {
            String::size_type esz = expr.length();
            String::size_type rsz = k_matchRX.length(); // RX & RXNot have eq lengh
            String::size_type cnt = esz - rsz - 1;
            if (cnt <= 0) throw std::logic_error("invalid rx expression");
            match = Core::regex()->match(value, expr.substr(rsz, cnt));
            if (expr.find(k_matchRXNot) == 0) match = !match; // negate
        }
        else
        {
            match = Strings::select(value, expr, sd);
        }
	    return match;
    }

    // tokenize: tokenize and filter text
    void Strings::tokenize(
        const String& text, 
        StringVector& tokens, 
        const StringSet& stoplist, 
        TokenOptions ops,
        bool clear) throw (Exception)
    {
        Log::Scope scope(KCC_FILE, "tokenize");
        IRegex* rx = Core::regex();
        StringVector split;
        split.reserve(64);
        rx->split(text, k_rx_split, split);
        if (clear) tokens.clear();
        tokens.reserve(split.size());
        for (StringVector::size_type i = 0; i < split.size(); i++)
        {
            String& c = split[i];
            if ((ops & Strings::TF_TRIM)      != 0) Strings::trim(c);
            if ((ops & Strings::TF_LOWERCASE) != 0) Strings::toLower(c);
            if ((ops & Strings::TF_BLANKS)    != 0 && !rx->match(c, k_rx_word))           continue;
            if ((ops & Strings::TF_STOPLIST)  != 0 && stoplist.find(c) != stoplist.end()) continue;
            if ((ops & Strings::TF_DECIMALS)  != 0 && rx->match(c, k_rx_decimal))         continue;
            if ((ops & Strings::TF_FRACTIONS) != 0 && rx->match(c, k_rx_fraction))        continue;
            tokens.push_back(c);
        }
    }

    // rxEscape: escape regex expression    
    String Strings::rxEscape(const String& expr) throw (Exception)
    {
        String esc(expr);
        Strings::rxEscape(esc);
        return esc;
    }

    // rxEscape: escape regex expression    
    void Strings::rxEscape(String& expr) throw (Exception)
    {
        Log::Scope scope(KCC_FILE, "rxEscape");
        Core::regex()->replace(expr, k_rx_reserved, k_rx_replace);
    }
    
    // rxEscape: escape regex tokens
    void Strings::rxEscape(StringVector& tokens) throw (Exception)
    {
        StringVector escaped;
        escaped.reserve(tokens.size());
        for (StringVector::const_iterator i = tokens.begin(); i != tokens.end(); i++)
        {
            // remove empty tokens
            if (!i->empty()) escaped.push_back(Strings::rxEscape(*i));
        }
        tokens.assign(escaped.begin(), escaped.end());
    }
}
