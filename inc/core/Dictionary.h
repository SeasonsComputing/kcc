/*
 * Kuumba C++ Core
 *
 * $Id: Dictionary.h 22625 2008-03-09 22:51:49Z tvk $
 */
#ifndef Dictionary_h
#define Dictionary_h

namespace kcc
{
    /** Finder Utility Types */
    typedef std::pair<StringMultimap::iterator, StringMultimap::iterator>             DictionaryFinder;
    typedef std::pair<StringMultimap::const_iterator, StringMultimap::const_iterator> DictionaryFinderConst;

    /** 
     * Dictionary (multi-map) ADT
     *
     * @author Ted V. Kremer
     */
    class KCC_CORE_EXPORT Dictionary : public StringMultimap
    {
    public:
        /**
         * Construct dictionary, empty or from existing dictionary
         */
        Dictionary() {}
        Dictionary(const Dictionary& dict) : StringMultimap(dict.begin(), dict.end()) {}
        Dictionary(Dictionary::const_iterator begin, Dictionary::const_iterator end) : StringMultimap(begin, end) {}
        Dictionary& operator = (const Dictionary& rhs) { if (this != &rhs) { StringMultimap::operator = (rhs); } return *this; }

        /**
         * Convenience insert operator
         * @param key key to add
         * @return reference to key value
         */
        inline String& operator () (const String& key) 
        { 
            return insert(std::make_pair(key, Strings::empty()))->second; 
        }

        /** 
         * Convenience finder operator
         * @param key to find
         * @return value or empty if not exists
         */
        inline const String& operator [] (const String& key) const
        {
            Dictionary::const_iterator i = find(key);
            return i == end() ? Strings::empty() : i->second;
        }

        /**
         * Query existence of key
         * @param key to find
         * @return true if exists
         */
        inline bool exists(const String& key) const { return find(key) != end(); }

        /** 
         * Convenience finder to find set of values
         * @param key to find
         * @return property finder with bounds (first=lower, second=upper)
         */
        inline DictionaryFinder      finder(const String& key)       { return equal_range(key); }
        inline DictionaryFinderConst finder(const String& key) const { return equal_range(key); }

        /**
         * Empty dictionary (GOF: Singleton, Null-Object)
         * @return empty dictionary
         */
        static const Dictionary& empty();
    };
}

#endif // Dictionary_h
