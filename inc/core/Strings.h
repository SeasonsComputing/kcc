/*
 * Kuumba C++ Core
 *
 * $Id: Strings.h 21801 2008-01-09 22:01:15Z tvk $
 */
#ifndef Strings_h
#define Strings_h

namespace kcc
{
    /**
     * String Utilities
     *
     * @author Ted V. Kremer
     */
    class KCC_CORE_EXPORT Strings
    {
    public:
        /** 
         * Empty string accessors (GOF: Singleton, Null-Object)
         */
        static const String&   empty();
        static const StringRC& emptyRC();

        /**
         * Convert string to lower case
         * @param in string to convert
         * @return reference to convert string (in param)
         */
        static String& toLower(String& in);
        static String  toLower(const String& in);
        static Char    toLower(const Char& in);

        /**
         * Convert string to upper case
         * @param in string to convert
         * @return reference to convert string (in param)
         */
        static String& toUpper(String& in);
        static String  toUpper(const String& in);
        static Char    toUpper(const Char& in);

        /**
         * Trim trailing and leading non-word (std::isalpha && '_')
         * @param in string to trim
         * @return reference to trimmed string (in param)
         */
        static String& trim(String& in);
        static String  trim(const String& in);

        /**
         * Trim trailing and leading whitespace (std::isspace)
         * @param in string to trim
         * @return reference to trimmed string (in param)
         */
        static String& trimws(String& in);
        static String  trimws(const String& in);
        static bool    isws  (const String& in); // is entire string ws

        /**
         * Trim trailing and leading whitespace and convert all non-space ws to space
         * @see Regex
         * @param in string to clean
         * @return reference to clean string (in param)
         */
        static String& cleanws(String& in)       throw (Exception);
        static String  cleanws(const String& in) throw (Exception);

        /**
         * Encode/decode XML entrity references (&lt; &amp; &gt; &quot; &apos;)
         * @param in string to decode
         * @return decoded string
         */
        static String xmlEncode(const String& in);
        static String xmlDecode(const String& in);

        /**
         * Determine type of character
         * @param in character to inspect
         * @return true if character is of appropriate type
         */
        static bool isSpace(const Char& in);
        static bool isAlpha(const Char& in);
    	static bool isDigit(const Char& in);
        static bool isAlnum(const Char& in);
        static bool isLower(const Char& in);
        static bool isUpper(const Char& in);

        /**
         * printf into String
         * @param format format string
         * @param parms format parms (see printf)
         */
        static String printf(const Char *format, ...);

        /**
         * Tokenizes a string into a vector of trim'd tokens
         * @param s string to tokenize                e.g. " foo =  bar"
         * @param seps string of seperator characters e.g. ";,="
         * @param tokens out collection of tokens     e.g. { "foo", "bar" }
         * @param clear clear out-param (as apposed to append)
         */
        static void tokenize(const String& s, const String& seps, StringVector& tokens, bool clear = true);

        /**
         * Tokenizes a CSV string into a vector of tokens (RFC 4180)
         * @param in stream to read from (required for multi-line fields -- embedded CRLF)
         * @param line string to tokenize             e.g. "foo,bar"
         * @param tokens out collection of tokens     e.g. { "foo", "bar" }
         * @param sep separator to use (default is ",")
         * @param clear clear out-param (as apposed to append)
         * @return true if not end of stream
         */
        static bool csv(std::istream& in, StringVector& tokens, Char sep = ',', bool clear = true);
        static void csv(const String& line, StringVector& tokens, Char sep = ',', bool clear = true);

        /**
         * Convert string to scalar
         * @param v string to convert
         * @return converted string (return 0 if invalid)
         */
        static double parseFraction(const String& v);
        static long   parseInteger (const String& v);

        /**
         * Join string vector into a single string
         * @param sep separator to join with
         * @param b vector begin position
         * @param e vector end position
         * @param out string containing joined text (cleared on entry)
         * @return joined text
         */
        static void join(
            const String& sep,
            StringVector::const_iterator b,
            StringVector::const_iterator e,
            String& out);
        static String join(
            const String& sep,
            StringVector::const_iterator b,
            StringVector::const_iterator e);

        /**
         * Load text file into string
         * @param path path to file to read
         * @param text out param of text contents
         * @param clear clear out-param (as apposed to append)
         * @return false if file not found
         */
        static bool loadText(const String& path, String& text, bool clear = true);

        /**
         * Match value selection to expression
         * @param value value to match
         * @param expr expression to select against
         *             "@:text"         :     equal: value == 'text'
         *             "!:text"         : different: value != 'text'
         *             "^:text"         :    begins: value begins-with 'text'
         *             "$:text"         :      ends: value ends-with 'text'
         *             "~:text"         :  contains: value contains 'text'
         *             "|:text1|text2"  :      enum: value one-of text1 OR text2 OR ...
         *                                           NOTE: does NOT escape; consumer MUST encode
         *                                                 pipes for enumerations
         * @param def default operation when prefix not specified in expression
         * @return true if selection matches
         */
        enum SelectDefault { SD_EQUAL, SD_BEGINS, SD_CONTAINS, SD_ENUM };
        static bool select(const String& value, const String& expr, SelectDefault sd);

        /**
         * Match value to expression
         * @see Regex
         * @param value value to match
         * @param expr expression to match against
         *             "{boolean}"      : true|false
         *             "{isodate}"      : yyyy-MM-dd hh:mm:ss
         *             "{number}"       : 5150 or 5150.5150
         *             "{rx:expr}"      : value ~= expr == true
         *             "{rx!expr}"      : value ~= expr != true
         *             (select)         : Strings::select(value, expr, sd) == true
         * @param sd default operation when prefix not specified in expression
         * @return true if matched
         * @throw Exception if error matching
         */
        static bool match(const String& value, const String& expr, SelectDefault sd = Strings::SD_ENUM) throw (Exception);

        /**
         * Tokenize and filter text.
         * @see Regex
         * @param text text to tokenize
         * @param tokens out-param of filtered tokens
         * @param stoplist terms to filter our
         * @param ops token option flags
         * @param clear clear token vector prior to tokenizing
         * @throw Exception if error tokenizing
         */
        enum TokenFlags
        {
            TF_NONE      = 0,  // just tokenize
            TF_TRIM      = 1,  // trim tokens (Strings::trim())
            TF_STOPLIST  = 2,  // remove tokens found in stoplist
            TF_LOWERCASE = 4,  // convert tokens to lowercase (Strings::toLower())
            TF_BLANKS    = 8,  // remove blank terms
            TF_DECIMALS  = 16, // remove decimal tokens
            TF_FRACTIONS = 32, // remove fraction tokens
            TF_ALL       = TF_TRIM|TF_STOPLIST|TF_LOWERCASE|TF_BLANKS|TF_DECIMALS|TF_FRACTIONS
        };
        typedef unsigned int TokenOptions;
        static void tokenize(
            const String& text, 
            StringVector& tokens, 
            const StringSet& stoplist, 
            TokenOptions ops = Strings::TF_ALL,
            bool clear = true) throw (Exception);
            
        /**
         * Escape regex expression
         * @see Regex
         * @param expr expression to escape
         * @param tokens out-param of filtered tokens
         * @throw Exception if error tokenizing
         */
        static String rxEscape(const String& expr)   throw (Exception);
        static void   rxEscape(String& expr)         throw (Exception);
        static void   rxEscape(StringVector& tokens) throw (Exception);

    private:
        Strings();
    };
}

#endif // Strings_h
