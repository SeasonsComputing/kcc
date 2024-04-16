/*
 * Kuumba C++ Core
 *
 * $Id: UUID.h 22625 2008-03-09 22:51:49Z tvk $
 */
#ifndef UUID_h
#define UUID_h

namespace kcc
{
    /**
     * UUID digest
     *
     * @author Ted V. Kremer
     */
    class KCC_CORE_EXPORT UUIDDigest
    {
    public:
        /** Constuct a digest */
        UUIDDigest() { clear(); }
        UUIDDigest(const String& hex) { *this = UUIDDigest::parse(hex); }

        /**
         * Accessor to hex string of digest
         * @param hex out param of uuid digest as hex
         * @return uuid digest as hex
         */
        void   digest(String& hex) const;
        String digest() const;

        /** Utility */
        inline int compare(const UUIDDigest& rhs) const { return digest().compare(rhs.digest()); }
        
        /** Factory */
        static UUIDDigest parse(const String& hex);

    private:
        // Attributes
        friend class UUID;
        unsigned char m_hash[16];
        void clear();
    };
    inline bool operator <  (const UUIDDigest& lhs, const UUIDDigest& rhs) { return lhs.compare(rhs) <  0; }
    inline bool operator == (const UUIDDigest& lhs, const UUIDDigest& rhs) { return lhs.compare(rhs) == 0; }
    inline std::ostream& operator << (std::ostream& out, const UUIDDigest& dig) { out << dig.digest(); return out; }

    /**
     * UUID algorithm
     *
     * @author Ted V. Kremer
     */
    class KCC_CORE_EXPORT UUID
    {
    public:
        /**
         * Generate a UUID
         * @param digest out param of newly created digest 
         * @param clear clear digest prior to generate
         * @return new uuid
         */
        static void       generate(UUIDDigest& digest, bool clear = true);
        static UUIDDigest generate();

    private:
        UUID();
    };
}

#endif // UUID_h
