/*
 * Kuumba C++ Core
 *
 * $Id: MD5.cpp 23364 2008-04-30 23:30:47Z tvk $
 */
#include <inc/core/Core.h>

#define KCC_FILE "MD5"

namespace kcc
{
    //
    // Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
    // rights reserved.
    //
    // License to copy and use this software is granted provided that it
    // is identified as the "RSA Data Security, Inc. MD5 Message-Digest
    // Algorithm" in all material mentioning or referencing this software
    // or this function.
    // License is also granted to make and use derivative works provided
    // that such works are identified as "derived from the RSA Data
    // Security, Inc. MD5 Message-Digest Algorithm" in all material
    // mentioning or referencing the derived work.
    // RSA Data Security, Inc. makes no representations concerning either
    // the merchantability of this software or the suitability of this
    // software for any particular purpose. It is provided "as is"
    // without express or implied warranty of any kind.
    // These notices must be retained in any copies of any part of this
    // documentation and/or software.
    //
    struct MD5::MD5Impl
    {
        MD5Impl(unsigned char* digest);
        void update(const unsigned char*, unsigned int);
        void finalize();
        void transform(const unsigned char*);
        void encode(unsigned char*, const unsigned int*, unsigned int);
        void decode(unsigned int*, const unsigned char*, unsigned int);
        unsigned char* m_digest;
	    unsigned int   m_state[4];
	    unsigned int   m_count[2];
	    unsigned char  m_buffer[64];
    };

    static unsigned char PADDING[64] =
    {
        0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    inline unsigned int rotate_left(unsigned int x, unsigned int n) { return((x << n) | (x >>(32-n))); }
    inline unsigned int F(unsigned int x, unsigned int y, unsigned int z) { return((x & y) | (~x & z)); }
    inline unsigned int G(unsigned int x, unsigned int y, unsigned int z) { return((x & z) | (y & ~z)); }
    inline unsigned int H(unsigned int x, unsigned int y, unsigned int z) { return(x ^ y ^ z); }
    inline unsigned int I(unsigned int x, unsigned int y, unsigned int z) { return(y ^(x | ~z)); }
    inline void FF(unsigned int& a, unsigned int b, unsigned int c, unsigned int d, unsigned int x, unsigned int s, unsigned int ac)
        { a += F(b, c, d) + x + ac; a = rotate_left(a, s); a += b; }
    inline void GG(unsigned int& a, unsigned int b, unsigned int c, unsigned int d, unsigned int x, unsigned int s, unsigned int ac)
        { a += G(b, c, d) + x + ac; a = rotate_left(a, s); a += b; }
    inline void HH(unsigned int& a, unsigned int b, unsigned int c, unsigned int d, unsigned int x, unsigned int s, unsigned int ac)
        { a += H(b, c, d) + x + ac; a = rotate_left(a, s); a += b; }
    inline void	II(unsigned int& a, unsigned int b, unsigned int c, unsigned int d, unsigned int x, unsigned int s, unsigned int ac)
        { a += I(b, c, d) + x + ac; a = rotate_left(a, s); a += b; }

    MD5::MD5Impl::MD5Impl(unsigned char* digest) : m_digest(digest)
    {
        std::memset(m_count, 0, 2 * sizeof(unsigned int));
        m_state[0] = 0x67452301;
        m_state[1] = 0xefcdab89;
        m_state[2] = 0x98badcfe;
        m_state[3] = 0x10325476;
    }

    void MD5::MD5Impl::update(const unsigned char* chInput, unsigned int nInputLen)
    {
        unsigned int i, index, partLen;
        // Compute number of bytes mod 64
        index = (unsigned int)((m_count[0] >> 3) & 0x3F);
        // Update number of bits
        if ((m_count[0] += (nInputLen << 3)) <(nInputLen << 3))
        m_count[1]++;
        m_count[1] += (nInputLen >> 29);
        partLen = 64 - index;
        // Transform as many times as possible.
        if (nInputLen >= partLen)
        {
            std::memcpy(&m_buffer[index], chInput, partLen);
            transform(m_buffer);
            for (i = partLen; i + 63 < nInputLen; i += 64) transform(&chInput[i]);
            index = 0;
        }
        else
        {
            i = 0;
        }
        // Buffer remaining input
        std::memcpy(&m_buffer[index], &chInput[i], nInputLen-i);
    }

    void MD5::MD5Impl::finalize()
    {
        unsigned char bits[8];
        unsigned int index, padLen;
        // Save number of bits
        encode(bits, m_count, 8);
        // Pad out to 56 mod 64
        index = (unsigned int)((m_count[0] >> 3) & 0x3f);
        padLen = (index < 56) ?(56 - index) :(120 - index);
        update(PADDING, padLen);
        // Append length(before padding)
        update(bits, 8);
        // Store state in digest
        encode(m_digest, m_state, 16);
        std::memset(m_count, 0, 2 * sizeof(unsigned int));
        std::memset(m_state, 0, 4 * sizeof(unsigned int));
        std::memset(m_buffer,0, 64 * sizeof(unsigned char));
    }

    #define S11 7
    #define S12 12
    #define S13 17
    #define S14 22
    #define S21 5
    #define S22 9
    #define S23 14
    #define S24 20
    #define S31 4
    #define S32 11
    #define S33 16
    #define S34 23
    #define S41 6
    #define S42 10
    #define S43 15
    #define S44 21
    void MD5::MD5Impl::transform(const unsigned char* block)
    {
        unsigned int a = m_state[0], b = m_state[1], c = m_state[2], d = m_state[3], x[16];
        decode(x, block, 64);
        // Round 1
        FF(a, b, c, d, x[ 0], S11, 0xd76aa478);
        FF(d, a, b, c, x[ 1], S12, 0xe8c7b756);
        FF(c, d, a, b, x[ 2], S13, 0x242070db);
        FF(b, c, d, a, x[ 3], S14, 0xc1bdceee);
        FF(a, b, c, d, x[ 4], S11, 0xf57c0faf);
        FF(d, a, b, c, x[ 5], S12, 0x4787c62a);
        FF(c, d, a, b, x[ 6], S13, 0xa8304613);
        FF(b, c, d, a, x[ 7], S14, 0xfd469501);
        FF(a, b, c, d, x[ 8], S11, 0x698098d8);
        FF(d, a, b, c, x[ 9], S12, 0x8b44f7af);
        FF(c, d, a, b, x[10], S13, 0xffff5bb1);
        FF(b, c, d, a, x[11], S14, 0x895cd7be);
        FF(a, b, c, d, x[12], S11, 0x6b901122);
        FF(d, a, b, c, x[13], S12, 0xfd987193);
        FF(c, d, a, b, x[14], S13, 0xa679438e);
        FF(b, c, d, a, x[15], S14, 0x49b40821);
        // Round 2
        GG(a, b, c, d, x[ 1], S21, 0xf61e2562);
        GG(d, a, b, c, x[ 6], S22, 0xc040b340);
        GG(c, d, a, b, x[11], S23, 0x265e5a51);
        GG(b, c, d, a, x[ 0], S24, 0xe9b6c7aa);
        GG(a, b, c, d, x[ 5], S21, 0xd62f105d);
        GG(d, a, b, c, x[10], S22,  0x2441453);
        GG(c, d, a, b, x[15], S23, 0xd8a1e681);
        GG(b, c, d, a, x[ 4], S24, 0xe7d3fbc8);
        GG(a, b, c, d, x[ 9], S21, 0x21e1cde6);
        GG(d, a, b, c, x[14], S22, 0xc33707d6);
        GG(c, d, a, b, x[ 3], S23, 0xf4d50d87);
        GG(b, c, d, a, x[ 8], S24, 0x455a14ed);
        GG(a, b, c, d, x[13], S21, 0xa9e3e905);
        GG(d, a, b, c, x[ 2], S22, 0xfcefa3f8);
        GG(c, d, a, b, x[ 7], S23, 0x676f02d9);
        GG(b, c, d, a, x[12], S24, 0x8d2a4c8a);
        // Round 3
        HH(a, b, c, d, x[ 5], S31, 0xfffa3942);
        HH(d, a, b, c, x[ 8], S32, 0x8771f681);
        HH(c, d, a, b, x[11], S33, 0x6d9d6122);
        HH(b, c, d, a, x[14], S34, 0xfde5380c);
        HH(a, b, c, d, x[ 1], S31, 0xa4beea44);
        HH(d, a, b, c, x[ 4], S32, 0x4bdecfa9);
        HH(c, d, a, b, x[ 7], S33, 0xf6bb4b60);
        HH(b, c, d, a, x[10], S34, 0xbebfbc70);
        HH(a, b, c, d, x[13], S31, 0x289b7ec6);
        HH(d, a, b, c, x[ 0], S32, 0xeaa127fa);
        HH(c, d, a, b, x[ 3], S33, 0xd4ef3085);
        HH(b, c, d, a, x[ 6], S34,  0x4881d05);
        HH(a, b, c, d, x[ 9], S31, 0xd9d4d039);
        HH(d, a, b, c, x[12], S32, 0xe6db99e5);
        HH(c, d, a, b, x[15], S33, 0x1fa27cf8);
        HH(b, c, d, a, x[ 2], S34, 0xc4ac5665);
        // Round 4
        II(a, b, c, d, x[ 0], S41, 0xf4292244);
        II(d, a, b, c, x[ 7], S42, 0x432aff97);
        II(c, d, a, b, x[14], S43, 0xab9423a7);
        II(b, c, d, a, x[ 5], S44, 0xfc93a039);
        II(a, b, c, d, x[12], S41, 0x655b59c3);
        II(d, a, b, c, x[ 3], S42, 0x8f0ccc92);
        II(c, d, a, b, x[10], S43, 0xffeff47d);
        II(b, c, d, a, x[ 1], S44, 0x85845dd1);
        II(a, b, c, d, x[ 8], S41, 0x6fa87e4f);
        II(d, a, b, c, x[15], S42, 0xfe2ce6e0);
        II(c, d, a, b, x[ 6], S43, 0xa3014314);
        II(b, c, d, a, x[13], S44, 0x4e0811a1);
        II(a, b, c, d, x[ 4], S41, 0xf7537e82);
        II(d, a, b, c, x[11], S42, 0xbd3af235);
        II(c, d, a, b, x[ 2], S43, 0x2ad7d2bb);
        II(b, c, d, a, x[ 9], S44, 0xeb86d391);
        m_state[0] += a;
        m_state[1] += b;
        m_state[2] += c;
        m_state[3] += d;
        std::memset(x, 0, sizeof(x));
    }

    void MD5::MD5Impl::encode(unsigned char* dest, const unsigned int* src, unsigned int nLength)
    {
        unsigned int i, j;
        for(i = 0, j = 0; j < nLength; i++, j += 4)
        {
            dest[j]   = (unsigned char)(src[i] & 0xff);
            dest[j+1] = (unsigned char)((src[i] >> 8) & 0xff);
            dest[j+2] = (unsigned char)((src[i] >> 16) & 0xff);
            dest[j+3] = (unsigned char)((src[i] >> 24) & 0xff);
        }
    }

    void MD5::MD5Impl::decode(unsigned int* dest, const unsigned char* src, unsigned int nLength)
    {
        unsigned int i, j;
        for(i = 0, j = 0; j < nLength; i++, j += 4)
            dest[i] = 
                ((unsigned int)src[j]) |
                (((unsigned int)src[j+1])<<8) |
                (((unsigned int)src[j+2])<<16) |
                (((unsigned int)src[j+3])<<24);
    }

    //
    // MD5Digest implementation
    //

    // clear: reset digest
    inline void MD5Digest::clear() { std::memset(m_hash, 0, sizeof(m_hash)); }

    // digest: lazy conversion of digest to hex
    void MD5Digest::digest(String& hex) const
    {
        hex.clear();
        hex.reserve(sizeof(m_hash)*2);
        for (std::size_t i = 0; i < sizeof(m_hash); i++) hex += Strings::printf("%02x", m_hash[i]);
    }
    String MD5Digest::digest() const
    {
        String hex;
        digest(hex);
        return hex;
    }
    
    // parse: parse hex into digest
    MD5Digest MD5Digest::parse(const String& hex)
    {
        MD5Digest digest;
        for (std::size_t i = 0; i < sizeof(digest.m_hash); i++) 
        {
            int code = 0;
            std::sscanf(hex.c_str() + (i*2), "%02x", &code);
            digest.m_hash[i] = (unsigned char)code;
        }
        return digest;
    }
    
    //
    // MD5 implementation
    //

    // MD5 ctor/dtor
    MD5::MD5(MD5Digest& digest) : m_digest(digest), m_impl(new MD5Impl(m_digest.m_hash)) {}
    MD5::~MD5() { complete(); }

    // hash: hash buffer
    void MD5::hash(const char* buf, int size) 
    {
        KCC_ASSERT(m_impl != NULL, KCC_FILE, "hash", "MD5 already completed");
        m_impl->update((const unsigned char*)buf, (unsigned int)size); 
    }

    // complete: finalize hash
    MD5Digest& MD5::complete() 
    {
        if (m_impl != NULL)
        { 
            m_impl->finalize(); 
            delete m_impl;
            m_impl = NULL;
        }
        return m_digest;
    }

    // hash: helper to hash text
    void MD5::hash(const String& text, MD5Digest& d, bool clear)
    {
        if (clear) d.clear();
        MD5 md5(d);
        md5.hash(text.c_str(), text.size());
        md5.complete();
    }
    String MD5::hash(const String& text)
    {
        MD5Digest d;
        hash(text, d, false);
        return d.digest();
    }
    void MD5::hash(const String& text, String& hex)
    {
        MD5Digest d;
        hash(text, d, false);
        d.digest(hex);
    }
}
