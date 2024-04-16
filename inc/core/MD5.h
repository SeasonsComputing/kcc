/*
 * Kuumba C++ Core
 *
 * $Id: MD5.h 22625 2008-03-09 22:51:49Z tvk $
 */
#ifndef MD5_h
#define MD5_h

namespace kcc
{
    /**
     * MD5 digest
     *
     * @author Ted V. Kremer
     */
    class KCC_CORE_EXPORT MD5Digest
    {
    public:
        /** Constuct a digest */
        MD5Digest() { clear(); }
        MD5Digest(const String& hex) { *this = MD5Digest::parse(hex); }

        /**
         * Accessor to hex string of digest
         * @param hex out param of md5 digest as hex
         * @return md5 digest as hex
         */
        void   digest(String& hex) const;
        String digest() const;

        /** Utility */
        inline int compare(const MD5Digest& rhs) const { return digest().compare(rhs.digest()); }
        
        /** Factory */
        static MD5Digest parse(const String& hex);

    private:
        // Attributes
        friend class MD5;
        unsigned char m_hash[16];
        void clear();
    };
    inline bool operator <  (const MD5Digest& lhs, const MD5Digest& rhs) { return lhs.compare(rhs) <  0; }
    inline bool operator == (const MD5Digest& lhs, const MD5Digest& rhs) { return lhs.compare(rhs) == 0; }
    inline std::ostream& operator << (std::ostream& out, const MD5Digest& dig) { out << dig.digest(); return out; }

    /**
     * MD5 algorithm
     *
     * @author Ted V. Kremer
     */
    class KCC_CORE_EXPORT MD5
    {
    public:
        /**
         * Construct MD5 algorithm
         * @param digest where to store hash digest into
         */
        MD5(MD5Digest& digest);
        ~MD5();

        /**
         * Hash buffer into MD5 digest
         * @param buf bytes to hash
         * @param sz size of bytes
         */
        void hash(const char* buf, int sz);

        /** 
         * Complete hash algorithm 
         * @return hased digest from constructor
         */
        MD5Digest& complete();

        /**
         * Utility to hash text into MD5 digest
         * @param text what to hash
         * @param hex out param of md5 digest as hex
         * @param clear clear digest prior to hash
         * @return md5 digest as hex
         */
        static void   hash(const String& text, MD5Digest& digest, bool clear = true);
        static void   hash(const String& text, String& hex);
        static String hash(const String& text);

    private:
        MD5(const MD5&);
        MD5& operator = (const MD5&);
    
        // Attributes
        struct MD5Impl;
        MD5Digest& m_digest;
        MD5Impl*   m_impl; 
    };
}

#endif // MD5_h
